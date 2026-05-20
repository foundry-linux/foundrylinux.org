#!/usr/bin/env bash
# Shared build helper for the per-tool packages under apt/packages/<name>/.
#
# Each tool's build.sh exports a few env vars then exec's into here:
#
#   #!/usr/bin/env bash
#   set -euo pipefail
#   export NAME=iffdump
#   export EXTRA_CRATES="wf_hdump"
#   exec bash "$(dirname "$0")/../../scripts/build-rust-tool.sh"
#
# Recognised env vars:
#
#   NAME         (required) deb package name (e.g. iffdump)
#   BIN          binary name (defaults to $NAME)
#   SOURCE_DIR   wftools subdir to vendor (defaults to ${NAME}-rs)
#   EXTRA_CRATES space-separated sibling wftools/ crates to also vendor
#                (e.g. "wf_oad wf_hdump") so Cargo's `path = "../foo"` deps
#                resolve inside the orig tarball
#   WFTOOLS_SHA  upstream pinned SHA in github.com/wbniv/WorldFoundry
#                (default: the latest known-good for this repo)
#
# Flow:
#   1. curl the github codeload tarball at the pinned SHA
#   2. extract only wftools/${SOURCE_DIR}/*, wftools/COPYING,
#      and wftools/<c>/* for each c in EXTRA_CRATES
#   3. rename top-dir to <name>-<upstream-version>/
#   4. repack as <name>_<upstream-version>.orig.tar.gz
#   5. overlay the package's debian/ tree
#   6. dpkg-buildpackage -us -uc -b -d --no-sign
#   7. mv <name>_*.deb → $APT_ROOT/dist/
#
# Notes:
#   -d (in step 6) skips the Build-Depends check because rustc / cargo
#   come from rustup-installed /opt/rust in apt-builder, not via apt.

set -euo pipefail

for arg in "$@"; do
    case "$arg" in
        -h|--help)
            sed -n '2,32p' "$0" | sed 's/^# \{0,1\}//'
            exit 0
            ;;
    esac
done

NAME="${NAME:?must export NAME=<deb-package>}"
BIN="${BIN:-$NAME}"
SOURCE_DIR="${SOURCE_DIR:-${NAME}-rs}"
EXTRA_CRATES="${EXTRA_CRATES:-}"
EXTRA_FILES="${EXTRA_FILES:-}"     # individual file paths under wftools/
                                    # to also vendor, e.g. "wf_asset.py"

# Latest WorldFoundry SHA carrying wf_asset.py / wf_viewer/build.sh fixes.
# Bump this in lockstep with each tool's debian/changelog (the upstream
# version string `0.X.Y+git<short>` should match this SHA prefix).
UPSTREAM_OWNER="wbniv"
UPSTREAM_REPO="WorldFoundry"
UPSTREAM_SHA="${WFTOOLS_SHA:-0a19d26cd40859f44d4a7787047f69453abc8612}"
UPSTREAM_URL="https://codeload.github.com/${UPSTREAM_OWNER}/${UPSTREAM_REPO}/tar.gz/${UPSTREAM_SHA}"

# This helper lives at apt/scripts/build-rust-tool.sh. Its parent dir
# is apt/scripts/; one level up is the apt root (where packages/, dist/,
# Taskfile.yml live). We deliberately do NOT walk up to the project
# root, because the project root's dist/ belongs to the Astro site
# build — putting .debs there would pollute it.
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
APT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
PKG_DIR="$APT_ROOT/packages/$NAME"
test -d "$PKG_DIR" || { echo "ERROR: $PKG_DIR not found" >&2; exit 1; }
test -d "$PKG_DIR/debian" || { echo "ERROR: $PKG_DIR/debian missing" >&2; exit 1; }
mkdir -p "$APT_ROOT/dist"

DEB_VERSION=$(dpkg-parsechangelog -l "$PKG_DIR/debian/changelog" -SVersion)
UPSTREAM_VERSION="${DEB_VERSION%-*}"   # strip the "-1foundry1" Debian revision

WORKDIR=$(mktemp -d -t "${NAME}-build-XXXXXX")
# shellcheck disable=SC2064  # expand $WORKDIR now so the trap captures the value
trap "rm -rf '$WORKDIR'" EXIT

echo "=== Fetching $UPSTREAM_URL ==="
SRC_TARBALL="$WORKDIR/upstream.tar.gz"
curl -fsSL -o "$SRC_TARBALL" "$UPSTREAM_URL"

# Build the list of paths to extract from the tarball.
extract_paths=(
    "${UPSTREAM_REPO}-${UPSTREAM_SHA}/wftools/${SOURCE_DIR}/*"
    "${UPSTREAM_REPO}-${UPSTREAM_SHA}/wftools/COPYING"
)
for c in $EXTRA_CRATES; do
    extract_paths+=("${UPSTREAM_REPO}-${UPSTREAM_SHA}/wftools/${c}/*")
done
for f in $EXTRA_FILES; do
    # Individual file under wftools/ — used by pure-Python debs (wf-asset)
    # that want a single script alongside a sibling addon dir.
    extract_paths+=("${UPSTREAM_REPO}-${UPSTREAM_SHA}/wftools/${f}")
done

echo "=== Extracting upstream subset ==="
printf '  %s\n' "${extract_paths[@]}"
tar -xzf "$SRC_TARBALL" -C "$WORKDIR" --wildcards "${extract_paths[@]}"

# Rename top-dir to <name>-<version>/ (dpkg-buildpackage's expected form).
UPSTREAM_DIR="$WORKDIR/${UPSTREAM_REPO}-${UPSTREAM_SHA}"
SRC_DIR="$WORKDIR/${NAME}-${UPSTREAM_VERSION}"
mv "$UPSTREAM_DIR" "$SRC_DIR"

echo "=== Repacking as ${NAME}_${UPSTREAM_VERSION}.orig.tar.gz ==="
ORIG_TARBALL="$WORKDIR/${NAME}_${UPSTREAM_VERSION}.orig.tar.gz"
tar --owner=0 --group=0 --sort=name \
    --mtime="2026-01-01 00:00:00 UTC" \
    -czf "$ORIG_TARBALL" -C "$WORKDIR" "$(basename "$SRC_DIR")"

echo "=== Overlaying debian/ ==="
cp -a "$PKG_DIR/debian" "$SRC_DIR/"

echo "=== dpkg-buildpackage -us -uc -b -d ==="
( cd "$SRC_DIR" && dpkg-buildpackage -us -uc -b -d --no-sign )

ARCH=$(dpkg --print-architecture)
# Try both arch-specific (amd64/arm64) and arch-independent (all) deb names.
deb_amd64="$WORKDIR/${NAME}_${DEB_VERSION}_${ARCH}.deb"
deb_all="$WORKDIR/${NAME}_${DEB_VERSION}_all.deb"
for DEB in "$deb_amd64" "$deb_all"; do
    if [[ -f "$DEB" ]]; then
        mv "$DEB" "$APT_ROOT/dist/"
        OUT="$APT_ROOT/dist/$(basename "$DEB")"
        echo "OK   $OUT  ($(stat -c%s "$OUT") bytes)"
        exit 0
    fi
done

echo "ERROR: expected .deb not found in $WORKDIR" >&2
ls -la "$WORKDIR" >&2
exit 1
