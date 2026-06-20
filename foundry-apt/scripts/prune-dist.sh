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

suffix=""
$DRY_RUN && suffix=" (dry-run; nothing removed)"
echo "prune-dist: kept ${#best_ver[@]} package(s); removed ${orphans} orphan(s) + ${pruned} superseded .deb(s)${suffix}"
