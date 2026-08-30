#!/usr/bin/env bash
# Audit every package build's pinned upstream payload for packaging maintained
# by upstream.  This deliberately examines the unmodified payload, before our
# debian/ directory is overlaid by build.sh.
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
APT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
CACHE_DIR="${UPSTREAM_AUDIT_CACHE:-$APT_ROOT/.cache/upstream-packaging-audit}"
BASELINE="$SCRIPT_DIR/upstream-packaging-baseline.tsv"
mkdir -p "$CACHE_DIR"

usage() {
    cat <<EOF
Usage: $(basename "$0") [--strict] [--inventory-only] [package ...]

Audit all vendored upstreams, or only the named packages. By default, findings
are compared with the committed baseline; --strict fails on any signal.
--inventory-only checks that the baseline and packages/*/build.sh cover the
same package set without fetching upstreams. Downloads are kept in:
  $CACHE_DIR

Exit status is 1 for baseline drift (or any signal with --strict), 0 when the
observed result matches, and 2 for an audit/fetch error.
EOF
}

packages=()
strict=0
inventory_only=0
for arg in "$@"; do
    case "$arg" in
        -h|--help) usage; exit 0 ;;
        --strict) strict=1 ;;
        --inventory-only) inventory_only=1 ;;
        --*) echo "ERROR: unknown option: $arg" >&2; usage >&2; exit 2 ;;
        *) packages+=("$arg") ;;
    esac
done
if ((${#packages[@]} == 0)); then
    while IFS= read -r build; do
        packages+=("$(basename "$(dirname "$build")")")
    done < <(find "$APT_ROOT/packages" -mindepth 2 -maxdepth 2 -name build.sh -print | sort)
fi

[[ -f $BASELINE ]] || { echo "ERROR: baseline not found: $BASELINE" >&2; exit 2; }
if ((inventory_only)); then
    actual=$(mktemp)
    expected=$(mktemp)
    trap 'rm -f "$actual" "$expected"' EXIT
    printf '%s\n' "${packages[@]}" | sort -u > "$actual"
    awk -F '\t' '!/^#/ && NF {print $1}' "$BASELINE" | sort -u > "$expected"
    if ! diff -u "$expected" "$actual"; then
        echo "ERROR: upstream packaging baseline inventory is stale" >&2
        exit 1
    fi
    echo "PASS: upstream packaging baseline covers all ${#packages[@]} package builds"
    exit 0
fi

# Read only the declarative, upper-case scalar assignments from a trusted
# build.sh. This reuses its pins without executing installs, builds, or traps.
load_build_vars() {
    local file=$1 line
    while IFS= read -r line; do
        [[ $line =~ ^(export[[:space:]]+)?(UPSTREAM_VERSION|UPSTREAM_COMMIT|UPSTREAM_DATE|UPSTREAM_SHA|EXPECTED_COMMIT|SHA256|EXPECTED_SHA256|ORIG_SHA256|DRDEVTOOLS_SHA|VERSION_NODOTS|UPSTREAM_OWNER|UPSTREAM_REPO|UPSTREAM_URL|TARBALL_URL|ZIP_URL|REPO_URL)= ]] || continue
        eval "$line"
    done < "$file"
}

fetch_spec() {
    local pkg=$1 build="$APT_ROOT/packages/$1/build.sh"
    unset UPSTREAM_URL TARBALL_URL ZIP_URL REPO_URL SHA256 EXPECTED_SHA256 \
        ORIG_SHA256 UPSTREAM_COMMIT EXPECTED_COMMIT UPSTREAM_SHA
    load_build_vars "$build"

    case "$pkg" in
        foundry-welcome) FETCH_KIND=local; FETCH_URL=-; FETCH_PIN=- ;;
        blender-asset-finder|blender-asset-finder-cli)
            # Their wrappers delegate the pinned fetch to build-rust-tool.sh.
            export DEB_VERSION=x
            load_build_vars "$APT_ROOT/scripts/build-rust-tool.sh"
            FETCH_KIND=archive; FETCH_URL=$UPSTREAM_URL; FETCH_PIN=$UPSTREAM_SHA ;;
        flycast) FETCH_KIND=archive; FETCH_URL="https://github.com/flyinghead/flycast/archive/${EXPECTED_COMMIT}.tar.gz"; FETCH_PIN=$EXPECTED_COMMIT ;;
        ppsspp) FETCH_KIND=archive; FETCH_URL="https://github.com/hrydgard/ppsspp/archive/${EXPECTED_COMMIT}.tar.gz"; FETCH_PIN=$EXPECTED_COMMIT ;;
        rpcs3) FETCH_KIND=archive; FETCH_URL="https://github.com/RPCS3/rpcs3/archive/${UPSTREAM_COMMIT}.tar.gz"; FETCH_PIN=$UPSTREAM_COMMIT ;;
        *)
            FETCH_KIND=archive
            FETCH_URL=${UPSTREAM_URL:-${TARBALL_URL:-${ZIP_URL:-}}}
            FETCH_PIN=${SHA256:-${EXPECTED_SHA256:-${ORIG_SHA256:-}}}
            ;;
    esac
    [[ $FETCH_KIND == local || (-n $FETCH_URL && -n $FETCH_PIN) ]] || {
        echo "ERROR: could not obtain pinned fetch from $build" >&2; return 1;
    }
}

extract_archive() {
    local archive=$1 dest=$2
    case "$archive" in
        *.zip|*.whl) unzip -q "$archive" -d "$dest" ;;
        *.AppImage)
            # A type-2 AppImage is an ELF runtime with a squashfs appended.
            # --appimage-extract unpacks it without FUSE and without root,
            # writing squashfs-root/ into the current directory.
            chmod +x "$archive"
            ( cd "$dest" && "$archive" --appimage-extract >/dev/null 2>&1 ) && return 0
            # Fall back to reading the appended squashfs at its offset if the
            # runtime cannot execute (unusual container, no binfmt).
            local offset
            offset=$("$archive" --appimage-offset 2>/dev/null || true)
            [[ -n $offset ]] || return 1
            unsquashfs -q -o "$offset" -d "$dest/squashfs-root" "$archive" >/dev/null
            ;;
        *) tar -xf "$archive" -C "$dest" ;;
    esac
}

join_paths() { sed "s#^$1/##" | sort -u | paste -sd ', ' -; }

errors=0
observed=$(mktemp)
expected=$(mktemp)
trap 'rm -f "$observed" "$expected"' EXIT
emit_result() {
    local pkg=$1 debian=$2 debbuild=$3 distrofiles=$4 refs=$5
    # shellcheck disable=SC2016 # Markdown backticks are literal, not shell syntax.
    printf '| `%s` | %s | %s | %s | %s |\n' "$pkg" "$debian" "$debbuild" "$distrofiles" "$refs"
    printf '%s\t%s\t%s\t%s\t%s\n' "$pkg" "$debian" "$debbuild" "$distrofiles" "$refs" >> "$observed"
}
# shellcheck disable=SC2016 # Markdown backticks are literal, not shell syntax.
printf '| Package | upstream `debian/` | own `.deb`/`.dsc` build | `PKGBUILD`/`.spec` | distro refs |\n'
printf '|---|---|---|---|---|\n'
for pkg in "${packages[@]}"; do
    build="$APT_ROOT/packages/$pkg/build.sh"
    if [[ ! -f $build ]]; then
        echo "ERROR: no package build.sh for $pkg" >&2; errors=1; continue
    fi
    if ! fetch_spec "$pkg"; then errors=1; continue; fi
    if [[ $FETCH_KIND == local ]]; then
        emit_result "$pkg" 'n/a (native package)' n/a n/a n/a
        continue
    fi

    suffix=${FETCH_URL%%\?*}; suffix=${suffix##*/}
    case "$suffix" in *.zip|*.whl|*.AppImage|*.tar.zst|*.tar.xz|*.tar.bz2|*.tgz|*.tar.gz) ;; *) suffix=upstream.tar.gz ;; esac
    # Key payloads by the build pin, not the Debian package name: two packages
    # may intentionally vendor different subsets of the same pinned upstream.
    archive="$CACHE_DIR/payload-${FETCH_PIN}-${suffix}"
    if [[ ! -f $archive ]]; then
        echo "Fetching $pkg from $FETCH_URL" >&2
        if ! curl -fL --retry 3 -o "$archive.part" "$FETCH_URL"; then
            rm -f "$archive.part"; errors=1; continue
        fi
        mv "$archive.part" "$archive"
    fi
    if [[ $FETCH_PIN =~ ^[0-9a-fA-F]{64}$ ]] && ! echo "$FETCH_PIN  $archive" | sha256sum -c - >/dev/null; then
        echo "ERROR: sha256 mismatch for cached $pkg payload" >&2; errors=1; continue
    fi

    tree="$CACHE_DIR/tree-$pkg-${FETCH_PIN:0:16}"
    if [[ ! -d $tree ]]; then
        mkdir -p "$tree.tmp"
        if ! extract_archive "$archive" "$tree.tmp"; then
            echo "ERROR: cannot extract $archive" >&2; rm -rf "$tree.tmp"; errors=1; continue
        fi
        mv "$tree.tmp" "$tree"
    fi

    debian=$(find "$tree" -type d -name debian -not -path '*/node_modules/*' -print0 \
        | while IFS= read -r -d '' dir; do
            [[ -f $dir/control || -f $dir/rules || -f $dir/changelog ]] && printf '%s\n' "$dir"
        done | join_paths "$tree")
    debbuild=$(find "$tree" -type f \( -iname '*deb*build*' -o -iname 'build*deb*' -o -iname '*mkdeb*' -o -iname '*.dsc' \) -print0 \
        | while IFS= read -r -d '' candidate; do
            [[ $candidate != *.dsc ]] || grep -qE '^Format:.*(native|quilt|source)' "$candidate" 2>/dev/null || continue
            printf '%s\n' "$candidate"
        done | join_paths "$tree")
    distrofiles=$(find "$tree" -type f \( -name PKGBUILD -o -iname '*.spec' \) -print | join_paths "$tree")
    refs=$(find "$tree" -type f \( -iname 'README*' -o -iname 'INSTALL*' -o -iname '*.md' -o -iname '*.rst' \) \
        -not -path '*/docs/transcripts/*' -size -2M -print0 \
        | while IFS= read -r -d '' doc; do
            grep -IlE 'launchpad\.net/(~|ubuntu/\+source/)|(^|[^[:alnum:]])PPA([^[:alnum:]]|$)|build\.opensuse\.org|copr\.fedorainfracloud\.org' "$doc" 2>/dev/null || true
        done | join_paths "$tree")
    emit_result "$pkg" "${debian:-no}" "${debbuild:-no}" "${distrofiles:-no}" "${refs:-no}"
done

((errors)) && exit 2
if ((strict)); then
    awk -F '\t' '{for (i=2; i<=5; i++) if ($i != "no" && $i != "n/a" && $i != "n/a (native package)") found=1} END {exit !found}' "$observed" && exit 1
    exit 0
fi
for pkg in "${packages[@]}"; do
    row=$(awk -F '\t' -v pkg="$pkg" '$1 == pkg {print; count++} END {if (count != 1) exit 1}' "$BASELINE") || {
        echo "ERROR: baseline must contain exactly one row for $pkg" >&2
        errors=1
        continue
    }
    printf '%s\n' "$row" >> "$expected"
done
((errors)) && exit 1
sort -o "$observed" "$observed"
sort -o "$expected" "$expected"
if ! diff -u "$expected" "$observed"; then
    echo "ERROR: upstream packaging findings differ from the committed baseline" >&2
    exit 1
fi
echo "PASS: upstream packaging findings match the committed baseline" >&2
exit 0
