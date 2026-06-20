#!/usr/bin/env bash
# Vendored-upstream Debian package build wrapper for asar-snes-assembler.
#
# asar is a SNES 65816/SPC700/SuperFX cross-assembler by RPGHacker (Alcaro).
# This script:
#
#   1. Downloads the upstream source tarball (sha256-verified)
#   2. Extracts to a tmpdir
#   3. Copies our debian/ tree in
#   4. Runs dpkg-buildpackage -us -uc -b
#   5. Moves the resulting .deb into $REPO_ROOT/dist/
#
# To bump the upstream version: change UPSTREAM_VERSION + SHA256 below,
# add a debian/changelog entry, and re-pin SHA256 with:
#   curl -fsSL https://api.github.com/repos/RPGHacker/asar/tarball/v<VER> | sha256sum

set -euo pipefail

for arg in "$@"; do
    case "$arg" in
        -h|--help)
            cat <<EOF
Build asar-snes-assembler as a Debian .deb via dpkg-buildpackage.

Usage: build.sh [-h|--help]

Environment overrides:
  ASAR_SNES_VERSION  upstream version (default: 1.91)
  ASAR_SNES_SHA256   sha256 of the GitHub tarball (must match VERSION)

Output: \$REPO_ROOT/dist/asar-snes-assembler_<DEB_VERSION>_<arch>.deb
EOF
            exit 0
            ;;
    esac
done

UPSTREAM_VERSION="${ASAR_SNES_VERSION:-1.91}"
SHA256="${ASAR_SNES_SHA256:-b063ffff2e21bb9ef1e0de37557809b57ef8ee79908adf5a64ddae40be6e7c1d}"
UPSTREAM_URL="https://api.github.com/repos/RPGHacker/asar/tarball/v${UPSTREAM_VERSION}"

cd "$(dirname "$0")"
PKG_DIR="$(pwd)"
NAME="asar-snes-assembler"
REPO_ROOT="$(cd ../.. && pwd)"
mkdir -p "$REPO_ROOT/dist"

echo "=== Installing build dependencies ==="
if command -v apt-get >/dev/null; then
    _apt() { if [[ $EUID -eq 0 ]]; then apt-get "$@"; else sudo apt-get "$@"; fi; }
    _apt install -y --no-install-recommends cmake
fi

WORKDIR=$(mktemp -d -t "${NAME}-build-XXXXXX")
# shellcheck disable=SC2064
trap "rm -rf '$WORKDIR'" EXIT

echo "=== Fetching upstream tarball ==="
ORIG_TARBALL="$WORKDIR/${NAME}_${UPSTREAM_VERSION}.orig.tar.gz"
curl -fsSL -o "$ORIG_TARBALL" "$UPSTREAM_URL"

echo "=== Verifying sha256 ==="
echo "$SHA256  $ORIG_TARBALL" | sha256sum -c -

echo "=== Extracting ==="
# GitHub tarballs extract to RPGHacker-asar-<commit>/ — rename to <name>-<version>/
tar -xzf "$ORIG_TARBALL" -C "$WORKDIR"
EXTRACTED=$(find "$WORKDIR" -maxdepth 1 -name "RPGHacker-asar-*" -type d | head -1)
SRC_DIR="$WORKDIR/${NAME}-${UPSTREAM_VERSION}"
mv "$EXTRACTED" "$SRC_DIR"

echo "=== Copying debian/ tree into source ==="
cp -a "$PKG_DIR/debian" "$SRC_DIR/"

echo "=== dpkg-buildpackage -us -uc -b ==="
( cd "$SRC_DIR" && dpkg-buildpackage -us -uc -b )

DEB_VERSION=$(sed -n '1s/.*(\(.*\)).*/\1/p' "$PKG_DIR/debian/changelog")
ARCH=$(dpkg --print-architecture)
DEB="$WORKDIR/${NAME}_${DEB_VERSION}_${ARCH}.deb"
[[ -f "$DEB" ]] || { echo "ERROR: expected .deb not found: $DEB" >&2; ls -la "$WORKDIR"; exit 1; }

mv "$DEB" "$REPO_ROOT/dist/"
OUT="$REPO_ROOT/dist/$(basename "$DEB")"
echo "OK   $OUT  ($(stat -c%s "$OUT") bytes)"
