#!/usr/bin/env bash
# Vendored-upstream Debian package build wrapper for drmon.
#
# drmon lives in the public drdevtools repository (developer-resources-co/
# drdevtools) rather than shipping a standalone release tarball, so this
# script assembles the source tree from that repo. By default it fetches the
# sha256-pinned repo tarball at a fixed commit (so it builds reproducibly in
# CI, with no local checkout). Set DRDEVTOOLS=/path/to/checkout to build from
# a local working tree instead (e.g. when iterating on unreleased changes).
#
# This script is the entry point that foundry-apt's build-all.sh invokes:
#
#   1. Obtains the drdevtools tree (pinned tarball fetch, or local override)
#   2. Copies the drmon source tree + libs/pclib headers alongside it
#   3. Patches CMakeLists.txt to use the local libs/ path
#   4. Copies our debian/ tree into the assembled source tree
#   5. Runs dpkg-buildpackage -us -uc -b
#   6. Moves the resulting .deb into $REPO_ROOT/dist/
#
# Build deps the CI runner needs:
#   build-essential debhelper dpkg-dev lintian cmake ninja-build libncurses-dev
#
# To bump the version: re-pin DRDEVTOOLS_SHA + SHA256 below to a newer commit
# (re-pin with: curl -fsSL <archive-url> | sha256sum), add a debian/changelog
# entry via `dch -v <NEW> -D resolute "..."` from this directory.

set -euo pipefail

for arg in "$@"; do
    case "$arg" in
        -h|--help)
            cat <<EOF
Build drmon as a Debian .deb via dpkg-buildpackage.

Usage: build.sh [-h|--help]

By default fetches the sha256-pinned drdevtools tarball at commit
\${DRDEVTOOLS_SHA}. Environment overrides:
  DRDEVTOOLS    path to a local drdevtools checkout (skips the fetch)
  DRDEVTOOLS_SHA / DRDEVTOOLS_SHA256
                override the pinned commit + its tarball sha256

Output: \$REPO_ROOT/dist/drmon_<VERSION>_<arch>.deb
EOF
            exit 0
            ;;
    esac
done

# Pinned drdevtools commit + sha256 of its GitHub archive tarball.
DRDEVTOOLS_SHA="${DRDEVTOOLS_SHA:-61e1303ddfceb6b15a8a0d234fe798b578fe3725}"
SHA256="${DRDEVTOOLS_SHA256:-c8e90d57cd43f796f9a1fcb8002119950d3cf9953de183717fca0e7a5f6fa98d}"
TARBALL_URL="https://github.com/developer-resources-co/drdevtools/archive/${DRDEVTOOLS_SHA}.tar.gz"

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

# Obtain the drdevtools tree: local checkout if DRDEVTOOLS is set + present,
# otherwise fetch the sha256-pinned repo tarball.
if [[ -n "${DRDEVTOOLS:-}" && -d "$DRDEVTOOLS/devsys/tools/drmon" ]]; then
    echo "=== Using local drdevtools checkout at $DRDEVTOOLS ==="
    DRDEVTOOLS_ROOT="$DRDEVTOOLS"
else
    echo "=== Fetching $TARBALL_URL ==="
    if ! curl -fsI -o /dev/null https://github.com/; then
        echo "ERROR: cannot reach github.com — skipping $NAME build" >&2
        exit 1
    fi
    if command -v apt-get >/dev/null && ! command -v curl >/dev/null; then
        _apt0() { if [[ $EUID -eq 0 ]]; then apt-get "$@"; else sudo apt-get "$@"; fi; }
        _apt0 install -y --no-install-recommends curl ca-certificates
    fi
    curl -fsSL -o "$WORKDIR/drdevtools.tar.gz" "$TARBALL_URL"
    echo "=== Verifying sha256 ==="
    echo "$SHA256  $WORKDIR/drdevtools.tar.gz" | sha256sum -c -
    tar -xzf "$WORKDIR/drdevtools.tar.gz" -C "$WORKDIR"
    DRDEVTOOLS_ROOT="$WORKDIR/drdevtools-${DRDEVTOOLS_SHA}"
fi

DRMON_SRC="$DRDEVTOOLS_ROOT/devsys/tools/drmon"
LIBS_SRC="$DRDEVTOOLS_ROOT/libs"
[[ -d "$DRMON_SRC" ]] || { echo "ERROR: drmon source not found at $DRMON_SRC" >&2; exit 1; }

SRC_DIR="$WORKDIR/${NAME}-${DEB_VERSION}"
mkdir -p "$SRC_DIR"

echo "=== Copying drmon source from $DRMON_SRC ==="
cp -a "$DRMON_SRC/." "$SRC_DIR/"

echo "=== Copying libs/pclib headers ==="
mkdir -p "$SRC_DIR/libs"
cp -a "$LIBS_SRC/." "$SRC_DIR/libs/"

echo "=== Patching CMakeLists.txt: ../../../libs -> libs ==="
# shellcheck disable=SC2016  # ${CMAKE_CURRENT_SOURCE_DIR} is a literal CMake variable, must NOT expand in shell
sed -i 's|\${CMAKE_CURRENT_SOURCE_DIR}/../../../libs|${CMAKE_CURRENT_SOURCE_DIR}/libs|g' \
    "$SRC_DIR/CMakeLists.txt"

# Wire cppdap to the system jsoncpp. Upstream FetchContent-clones cppdap
# without its git submodules, so cppdap's bundled jsoncpp tree is empty and
# the Json::Value symbols never link. Tell cppdap to use the external jsoncpp
# package (provided by libcppdap-dev's sibling libjsoncpp-dev), and link
# jsoncpp_lib into the drmon-dap targets (drmon's own session.cpp uses it too).
echo "=== Patching CMakeLists.txt: use system jsoncpp for cppdap ==="
sed -i 's|^FetchContent_MakeAvailable(cppdap)|set(CPPDAP_USE_EXTERNAL_JSONCPP_PACKAGE ON CACHE BOOL "" FORCE)\nFetchContent_MakeAvailable(cppdap)\nfind_package(jsoncpp REQUIRED)|' \
    "$SRC_DIR/CMakeLists.txt"
# shellcheck disable=SC2016  # ${TARGET} is a literal CMake variable, must NOT expand in shell
sed -i 's|target_link_libraries(${TARGET} PRIVATE cppdap Threads::Threads)|target_link_libraries(${TARGET} PRIVATE cppdap jsoncpp_lib Threads::Threads)|' \
    "$SRC_DIR/CMakeLists.txt"

echo "=== Copying debian/ tree ==="
cp -a "$PKG_DIR/debian" "$SRC_DIR/"

echo "=== Installing Build-Depends ==="
if command -v apt-get >/dev/null; then
    _apt() { if [[ $EUID -eq 0 ]]; then apt-get "$@"; else sudo apt-get "$@"; fi; }
    _apt install -y --no-install-recommends \
        cmake ninja-build libncurses-dev libcppdap-dev libjsoncpp-dev
fi

echo "=== dpkg-buildpackage -us -uc -b ==="
( cd "$SRC_DIR" && dpkg-buildpackage -us -uc -b )

ARCH=$(dpkg --print-architecture)
DEB="$WORKDIR/${NAME}_${DEB_VERSION}_${ARCH}.deb"
[[ -f "$DEB" ]] || { echo "ERROR: expected .deb not found: $DEB" >&2; ls -la "$WORKDIR"; exit 1; }

mv "$DEB" "$REPO_ROOT/dist/"
OUT="$REPO_ROOT/dist/$(basename "$DEB")"
echo "OK   $OUT  ($(stat -c%s "$OUT") bytes)"
