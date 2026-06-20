#!/usr/bin/env bash
# prune-dist.sh — keep dist/ to exactly what the current sources produce:
#   1. drop ORPHANS — .debs whose package name is no longer produced by any
#      packages/*/debian/control (renamed or removed packages). Without this, a
#      restored CI build cache would keep republishing the old name forever
#      (e.g. after asar-snes → asar-snes-assembler). The old no-cache CI dropped
#      these for free by starting from an empty dist/.
#   2. drop SUPERSEDED versions — keep only the newest .deb per package, so a
#      version bump doesn't leave the previous build behind.
# Run in-container right after build-all (it reads the source tree + the .debs).
set -euo pipefail

if [[ "${1:-}" == "-h" || "${1:-}" == "--help" ]]; then
    echo "Usage: prune-dist.sh [--dry-run|-n]"
    echo "Drop dist/ .debs that are orphaned (no current source) or superseded"
    echo "by a newer version of the same package."
    exit 0
fi

DRY_RUN=false
[[ "${1:-}" == "--dry-run" || "${1:-}" == "-n" ]] && DRY_RUN=true

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
DIST="$REPO_ROOT/dist"
PACKAGES_DIR="$REPO_ROOT/packages"

shopt -s nullglob
debs=("$DIST"/*.deb)
if (( ${#debs[@]} == 0 )); then
    echo "prune-dist: no .debs in dist/ — nothing to do"
    exit 0
fi

# Every binary package name the current sources can produce (multi-binary
# sources like pvsneslib list several Package: stanzas; metapackages list one).
declare -A valid
while read -r name; do
    [[ -n "$name" ]] && valid["$name"]=1
done < <(grep -rh '^Package:' "$PACKAGES_DIR"/*/debian/control 2>/dev/null | awk '{print $2}' | sort -u)

if (( ${#valid[@]} == 0 )); then
    echo "prune-dist: refusing to prune — found no Package: names under $PACKAGES_DIR (sanity guard)" >&2
    exit 1
fi

# Pass 1: newest version per still-valid package.
declare -A best_ver best_file
for deb in "${debs[@]}"; do
    name="$(dpkg-deb -f "$deb" Package)"
    [[ -n "${valid[$name]:-}" ]] || continue          # orphan — handled in pass 2
    ver="$(dpkg-deb -f "$deb" Version)"
    if [[ -z "${best_ver[$name]:-}" ]] || dpkg --compare-versions "$ver" gt "${best_ver[$name]}"; then
        best_ver[$name]="$ver"
        best_file[$name]="$deb"
    fi
done

# Pass 2: drop orphans + superseded.
orphans=0
pruned=0
for deb in "${debs[@]}"; do
    name="$(dpkg-deb -f "$deb" Package)"
    if [[ -z "${valid[$name]:-}" ]]; then
        echo "drop  $(basename "$deb")  (renamed/removed — no current source for '$name')"
        $DRY_RUN || rm -f "$deb"
        orphans=$((orphans + 1))
    elif [[ "$deb" != "${best_file[$name]}" ]]; then
        echo "prune $(basename "$deb")  (superseded by ${name} ${best_ver[$name]})"
        $DRY_RUN || rm -f "$deb"
        pruned=$((pruned + 1))
    fi
done

# Pass 3: source packages (.dsc + tarballs) — the repo now publishes a Sources index,
# so superseded/orphaned source artifacts must be pruned the same way as .debs, or
# they'd linger and the Sources index would list stale versions. Prune by SOURCE name
# (differs from binary names for multi-binary sources like pvsneslib); a tarball is
# kept iff a surviving .dsc still references it (orig tarballs are shared across
# revisions of the same upstream version, so never prune by version alone).
src_orphans=0
src_pruned=0
dscs=("$DIST"/*.dsc)
if (( ${#dscs[@]} > 0 )); then
    declare -A valid_src
    while read -r s; do [[ -n "$s" ]] && valid_src["$s"]=1; done \
        < <(grep -rh '^Source:' "$PACKAGES_DIR"/*/debian/control 2>/dev/null | awk '{print $2}' | sort -u)

    declare -A best_dsc_ver best_dsc_file
    for dsc in "${dscs[@]}"; do
        s="$(awk '/^Source:/{print $2; exit}' "$dsc")"
        v="$(awk '/^Version:/{print $2; exit}' "$dsc")"
        [[ -n "${valid_src[$s]:-}" ]] || continue
        if [[ -z "${best_dsc_ver[$s]:-}" ]] || dpkg --compare-versions "$v" gt "${best_dsc_ver[$s]}"; then
            best_dsc_ver[$s]="$v"; best_dsc_file[$s]="$dsc"
        fi
    done

    # Files referenced by every surviving .dsc (the .dsc itself + its tarballs).
    declare -A keep_file
    for s in "${!best_dsc_file[@]}"; do
        d="${best_dsc_file[$s]}"
        keep_file["$(basename "$d")"]=1
        while read -r fn; do [[ -n "$fn" ]] && keep_file["$fn"]=1; done \
            < <(awk '/^(Files|Checksums-Sha256|Checksums-Sha1):/{f=1;next} /^[^ ]/{f=0} f{print $NF}' "$d")
    done

    for dsc in "${dscs[@]}"; do
        s="$(awk '/^Source:/{print $2; exit}' "$dsc")"
        if [[ -z "${valid_src[$s]:-}" ]]; then
            echo "drop  $(basename "$dsc")  (renamed/removed — no current source for '$s')"
            $DRY_RUN || rm -f "$dsc"; src_orphans=$((src_orphans + 1))
        elif [[ "$dsc" != "${best_dsc_file[$s]}" ]]; then
            echo "prune $(basename "$dsc")  (superseded by ${s} ${best_dsc_ver[$s]})"
            $DRY_RUN || rm -f "$dsc"; src_pruned=$((src_pruned + 1))
        fi
    done

    for tarball in "$DIST"/*.tar.*; do
        [[ -f "$tarball" ]] || continue
        [[ -n "${keep_file["$(basename "$tarball")"]:-}" ]] && continue
        echo "prune $(basename "$tarball")  (source tarball no longer referenced by a kept .dsc)"
        $DRY_RUN || rm -f "$tarball"; src_pruned=$((src_pruned + 1))
    done
fi

suffix=""
$DRY_RUN && suffix=" (dry-run; nothing removed)"
echo "prune-dist: kept ${#best_ver[@]} package(s); removed ${orphans} orphan(s) + ${pruned} superseded .deb(s), ${src_orphans} orphan + ${src_pruned} superseded source artifact(s)${suffix}"
