#!/usr/bin/env bash
# Local-source Debian package build wrapper for drmon.
#
# drmon lives in the sibling drdevtools repository rather than an upstream
# URL, so this script assembles the source tree from the local checkout
# instead of fetching a tarball.
#
# This script is the entry point that foundry-apt's build-all.sh invokes:
#
#   1. Copies the drmon source tree from drdevtools
#   2. Copies libs/pclib headers alongside it
#   3. Patches CMakeLists.txt to use the local libs/ path
#   4. Copies our debian/ tree into the assembled source tree
#   5. Runs dpkg-buildpackage -us -uc -b
#   6. Moves the resulting .deb into $REPO_ROOT/dist/
#
# Build deps the CI runner needs:
#   build-essential debhelper dpkg-dev lintian cmake ninja-build libncurses-dev
#
# To bump the version: update version.h in drdevtools, add a debian/changelog
# entry via `dch -v <NEW> -D resolute "..."` from this directory.

set -euo pipefail

for arg in "$@"; do
    case "$arg" in
        -h|--help)
            cat <<EOF
Build drmon as a Debian .deb via dpkg-buildpackage.

Usage: build.sh [-h|--help]

Environment overrides:
  DRDEVTOOLS    path to the drdevtools repo checkout
                (default: \$HOME/SRC/drdevtools)

Output: \$REPO_ROOT/dist/drmon_<VERSION>_<arch>.deb
EOF
            exit 0
            ;;
    esac
done

DRDEVTOOLS="${DRDEVTOOLS:-$HOME/SRC/drdevtools}"
DRMON_SRC="$DRDEVTOOLS/devsys/tools/drmon"
LIBS_SRC="$DRDEVTOOLS/libs"

if [[ ! -d "$DRMON_SRC" ]]; then
    echo "ERROR: drmon source not found at $DRMON_SRC" >&2
    echo "  Set DRDEVTOOLS=/path/to/drdevtools to override." >&2
    exit 1
fi

cd "$(dirname "$0")"
PKG_DIR="$(pwd)"
NAME="drmon"
REPO_ROOT="$(cd ../.. && pwd)"
mkdir -p "$REPO_ROOT/dist"

DEB_VERSION=$(sed -n '1s/.*(\(.*\)).*/\1/p' "$PKG_DIR/debian/changelog")
echo "=== Building $NAME $DEB_VERSION ==="

WORKDIR=$(mktemp -d -t "${NAME}-build-XXXXXX")
# shellcheck disable=SC2064
trap "rm -rf '$WORKDIR'" EXIT

SRC_DIR="$WORKDIR/${NAME}-${DEB_VERSION}"
mkdir -p "$SRC_DIR"

echo "=== Copying drmon source from $DRMON_SRC ==="
cp -a "$DRMON_SRC/." "$SRC_DIR/"

echo "=== Copying libs/pclib headers ==="
mkdir -p "$SRC_DIR/libs"
cp -a "$LIBS_SRC/." "$SRC_DIR/libs/"

echo "=== Patching CMakeLists.txt: ../../../libs -> libs ==="
sed -i 's|\${CMAKE_CURRENT_SOURCE_DIR}/../../../libs|${CMAKE_CURRENT_SOURCE_DIR}/libs|g' \
    "$SRC_DIR/CMakeLists.txt"

echo "=== Copying debian/ tree ==="
cp -a "$PKG_DIR/debian" "$SRC_DIR/"

echo "=== Installing Build-Depends ==="
if command -v apt-get >/dev/null; then
    _apt() { [[ $EUID -eq 0 ]] && apt-get "$@" || sudo apt-get "$@"; }
    _apt install -y --no-install-recommends \
        cmake ninja-build libncurses-dev
fi

echo "=== dpkg-buildpackage -us -uc -b ==="
( cd "$SRC_DIR" && dpkg-buildpackage -us -uc -b )

ARCH=$(dpkg --print-architecture)
DEB="$WORKDIR/${NAME}_${DEB_VERSION}_${ARCH}.deb"
[[ -f "$DEB" ]] || { echo "ERROR: expected .deb not found: $DEB" >&2; ls -la "$WORKDIR"; exit 1; }

mv "$DEB" "$REPO_ROOT/dist/"
OUT="$REPO_ROOT/dist/$(basename "$DEB")"
echo "OK   $OUT  ($(stat -c%s "$OUT") bytes)"
