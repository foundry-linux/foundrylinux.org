#!/usr/bin/env bash
# Vendored-upstream Debian package build wrapper for ghidra.
#
# Ghidra ships as a pre-built zip (not a source tarball) — there is no
# compile step. This script:
#
#   1. Downloads the upstream pre-built zip (sha256-verified)
#   2. Extracts and renames to ghidra-<VERSION>/ (dpkg-buildpackage expects
#      <srcname>-<upstream>/ as the build directory)
#   3. Copies our debian/ tree into that directory
#   4. Runs dpkg-buildpackage -us -uc -b (binary-only)
#   5. Moves the resulting .deb into $REPO_ROOT/dist/
#
# To bump the upstream version: change UPSTREAM_VERSION, SHA256, and
# UPSTREAM_URL below (the zip filename includes a release date), add a
# debian/changelog entry via `dch -v <NEW>-1foundry1 -D resolute`, re-pin
# the sha256 with:
#   curl -fsSL <new-url> | sha256sum

set -euo pipefail

for arg in "$@"; do
    case "$arg" in
        -h|--help)
            cat <<EOF
Build ghidra as a Debian .deb via dpkg-buildpackage.

Usage: build.sh [-h|--help]

Environment overrides (rarely needed):
  GHIDRA_VERSION  upstream version (default: 12.1)
  GHIDRA_SHA256   sha256 of the upstream zip (must match VERSION)

Output: \$REPO_ROOT/dist/ghidra_<DEB_VERSION>_<arch>.deb
EOF
            exit 0
            ;;
    esac
done

UPSTREAM_VERSION="${GHIDRA_VERSION:-12.1}"
SHA256="${GHIDRA_SHA256:-aa5cbcbbf48f41ca185fce900e19592f1ade4cd5994eb6e0ede468dac8a6f302}"
# NOTE: the zip filename includes the release date; update UPSTREAM_URL on each bump.
UPSTREAM_URL="https://github.com/NationalSecurityAgency/ghidra/releases/download/Ghidra_${UPSTREAM_VERSION}_build/ghidra_${UPSTREAM_VERSION}_PUBLIC_20260513.zip"

cd "$(dirname "$0")"
PKG_DIR="$(pwd)"
NAME="ghidra"
REPO_ROOT="$(cd ../.. && pwd)"
mkdir -p "$REPO_ROOT/dist"

if ! curl -fsI -o /dev/null https://github.com/; then
    echo "ERROR: cannot reach github.com — skipping $NAME build" >&2
    exit 1
fi

WORKDIR=$(mktemp -d -t "${NAME}-build-XXXXXX")
# shellcheck disable=SC2064
trap "rm -rf '$WORKDIR'" EXIT

echo "=== Fetching $UPSTREAM_URL ==="
ZIP="$WORKDIR/${NAME}_${UPSTREAM_VERSION}.zip"
curl -fsSL -o "$ZIP" "$UPSTREAM_URL"

echo "=== Verifying sha256 ==="
echo "$SHA256  $ZIP" | sha256sum -c -

echo "=== Extracting ==="
unzip -q "$ZIP" -d "$WORKDIR"
# Upstream extracts to ghidra_12.1_PUBLIC/ — rename to ghidra-12.1/ (dpkg convention)
EXTRACTED="$WORKDIR/ghidra_${UPSTREAM_VERSION}_PUBLIC"
SRC_DIR="$WORKDIR/${NAME}-${UPSTREAM_VERSION}"
[[ -d "$EXTRACTED" ]] || { echo "ERROR: expected $EXTRACTED after extract" >&2; ls "$WORKDIR"; exit 1; }
mv "$EXTRACTED" "$SRC_DIR"

echo "=== Copying debian/ tree into source ==="
cp -a "$PKG_DIR/debian" "$SRC_DIR/"

echo "=== Installing Build-Depends ==="
if command -v apt-get >/dev/null; then
    _apt() { [[ $EUID -eq 0 ]] && apt-get "$@" || sudo apt-get "$@"; }
    _apt install -y --no-install-recommends unzip
fi

echo "=== dpkg-buildpackage -us -uc -b ==="
( cd "$SRC_DIR" && dpkg-buildpackage -us -uc -b )

ARCH=$(dpkg --print-architecture)
DEB_VERSION=$(sed -n '1s/.*(\(.*\)).*/\1/p' "$PKG_DIR/debian/changelog")
DEB="$WORKDIR/${NAME}_${DEB_VERSION}_${ARCH}.deb"
[[ -f "$DEB" ]] || { echo "ERROR: expected .deb not found: $DEB" >&2; ls -la "$WORKDIR"; exit 1; }

mv "$DEB" "$REPO_ROOT/dist/"
OUT="$REPO_ROOT/dist/$(basename "$DEB")"
echo "OK   $OUT  ($(stat -c%s "$OUT") bytes)"
