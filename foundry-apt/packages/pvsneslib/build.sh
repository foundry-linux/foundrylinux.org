#!/usr/bin/env bash
# Vendored-upstream Debian package build wrapper for pvsneslib.
#
# PVSnesLib ships as a pre-built Linux zip (a self-contained SNES SDK:
# toolchain + C library + asset-conversion tools) — there is no compile step.
# This script:
#
#   1. Downloads the upstream pre-built Linux zip (sha256-verified)
#   2. Extracts; the zip's top dir is "pvsneslib/" — rename to pvsneslib-<VERSION>/
#      (dpkg-buildpackage expects <srcname>-<upstream>/ as the build directory)
#   3. Copies our debian/ tree into that directory
#   4. Runs dpkg-buildpackage -us -uc -b (binary-only) — produces THREE .debs:
#      pvsneslib-core_*_amd64.deb, pvsneslib-examples_*_all.deb, pvsneslib_*_all.deb
#   5. Moves all three into $REPO_ROOT/dist/
#
# To bump the upstream version: change UPSTREAM_VERSION + SHA256 + the zip
# filename in UPSTREAM_URL (it encodes the version as e.g. 450 for 4.5.0), add a
# debian/changelog entry via `dch -v <NEW>-1foundry1 -D resolute`, re-pin the
# sha256 with:
#   curl -fsSL <new-url> | sha256sum

set -euo pipefail

for arg in "$@"; do
    case "$arg" in
        -h|--help)
            cat <<EOF
Build pvsneslib (core + examples + meta) as Debian .debs via dpkg-buildpackage.

Usage: build.sh [-h|--help]

Environment overrides (rarely needed):
  PVSNESLIB_VERSION  upstream version (default: 4.5.0)
  PVSNESLIB_SHA256   sha256 of the upstream Linux zip (must match VERSION)

Output: \$REPO_ROOT/dist/pvsneslib{,-core,-examples}_<DEB_VERSION>_<arch>.deb
EOF
            exit 0
            ;;
    esac
done

UPSTREAM_VERSION="${PVSNESLIB_VERSION:-4.5.0}"
SHA256="${PVSNESLIB_SHA256:-b69ff32ada19895b7ebfe02a1e3c08a44c80bd9c8132de05f5c356f86264ce32}"
# The release asset filename encodes the version without dots (4.5.0 -> 450).
VERSION_NODOTS="${UPSTREAM_VERSION//./}"
UPSTREAM_URL="https://github.com/alekmaul/pvsneslib/releases/download/${UPSTREAM_VERSION}/pvsneslib_${VERSION_NODOTS}_64b_linux.zip"

cd "$(dirname "$0")"
PKG_DIR="$(pwd)"
NAME="pvsneslib"
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

echo "=== Installing Build-Depends ==="
if command -v apt-get >/dev/null; then
    _apt() { if [[ $EUID -eq 0 ]]; then apt-get "$@"; else sudo apt-get "$@"; fi; }
    _apt install -y --no-install-recommends unzip
fi

echo "=== Extracting ==="
unzip -q "$ZIP" -d "$WORKDIR"
# Upstream extracts to pvsneslib/ — rename to pvsneslib-4.5.0/ (dpkg convention)
EXTRACTED="$WORKDIR/pvsneslib"
SRC_DIR="$WORKDIR/${NAME}-${UPSTREAM_VERSION}"
[[ -d "$EXTRACTED" ]] || { echo "ERROR: expected $EXTRACTED after extract" >&2; ls "$WORKDIR"; exit 1; }
mv "$EXTRACTED" "$SRC_DIR"

echo "=== Copying debian/ tree into source ==="
cp -a "$PKG_DIR/debian" "$SRC_DIR/"

echo "=== dpkg-buildpackage -us -uc -b ==="
( cd "$SRC_DIR" && dpkg-buildpackage -us -uc -b )

DEB_VERSION=$(sed -n '1s/.*(\(.*\)).*/\1/p' "$PKG_DIR/debian/changelog")
ARCH=$(dpkg --print-architecture)

# Three artifacts: -core is arch-specific, -examples and the meta are arch:all.
moved=0
for DEB in \
    "$WORKDIR/${NAME}-core_${DEB_VERSION}_${ARCH}.deb" \
    "$WORKDIR/${NAME}-examples_${DEB_VERSION}_all.deb" \
    "$WORKDIR/${NAME}_${DEB_VERSION}_all.deb"; do
    [[ -f "$DEB" ]] || { echo "ERROR: expected .deb not found: $DEB" >&2; ls -la "$WORKDIR"; exit 1; }
    mv "$DEB" "$REPO_ROOT/dist/"
    OUT="$REPO_ROOT/dist/$(basename "$DEB")"
    echo "OK   $OUT  ($(stat -c%s "$OUT") bytes)"
    moved=$((moved + 1))
done
echo "=== Moved $moved .debs into dist/ ==="
