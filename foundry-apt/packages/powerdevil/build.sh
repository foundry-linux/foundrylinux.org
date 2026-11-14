#!/usr/bin/env bash
# Rebuild Ubuntu's PowerDevil package with the upstream battery recovery fixes.
set -euo pipefail
case "${1:-}" in
    -h|--help) echo 'Build PowerDevil for Foundry in an Ubuntu 26.04 container; output: dist/'; exit 0 ;;
    '') ;;
    *) echo 'Usage: build.sh [--help]' >&2; exit 2 ;;
esac
[[ -f /.dockerenv || -f /run/.containerenv ]] || {
    echo 'Build this package inside an Ubuntu 26.04 container.' >&2; exit 1;
}
# shellcheck source=/dev/null
source /etc/os-release
[[ $ID == ubuntu && $VERSION_ID == 26.04 ]] || {
    echo 'Ubuntu 26.04 is required for the target library ABI.' >&2; exit 1;
}

UPSTREAM_VERSION="6.6.6"
SHA256="18f2e1aa3b49848d9ba3ad38f1097090424edb3b17a474d2da3b81fb2e083e17"
UPSTREAM_URL="https://archive.ubuntu.com/ubuntu/pool/universe/p/powerdevil/powerdevil_${UPSTREAM_VERSION}.orig.tar.xz"
PKG_DIR=$(cd "$(dirname "$0")" && pwd)
REPO_ROOT=$(cd "$PKG_DIR/../.." && pwd)
export DEBIAN_FRONTEND=noninteractive
apt-get install -y --no-install-recommends build-essential dpkg-dev ca-certificates curl xz-utils
apt-get build-dep -y --no-install-recommends "$PKG_DIR"

BUILD_DIR=$(mktemp -d -t powerdevil-build-XXXXXX)
trap 'rm -rf "$BUILD_DIR"' EXIT
ORIG="$BUILD_DIR/powerdevil_${UPSTREAM_VERSION}.orig.tar.xz"
curl -fsSL --retry 3 "$UPSTREAM_URL" -o "$ORIG"
echo "$SHA256  $ORIG" | sha256sum -c -
tar -xJf "$ORIG" -C "$BUILD_DIR"
SOURCE_DIR="$BUILD_DIR/powerdevil-${UPSTREAM_VERSION}"
cp -a "$PKG_DIR/debian" "$SOURCE_DIR/"
# Bound parallelism on developer laptops; CI may explicitly increase it.
export DEB_BUILD_OPTIONS="${DEB_BUILD_OPTIONS:-parallel=2}"
(
    cd "$SOURCE_DIR"
    dpkg-buildpackage -us -uc -b
    dpkg-buildpackage -us -uc -S -d
)

mkdir -p "$REPO_ROOT/dist"
shopt -s nullglob
artifacts=("$BUILD_DIR"/*.deb "$BUILD_DIR"/*.dsc "$BUILD_DIR"/*.debian.tar.* "$ORIG")
for artifact in "${artifacts[@]}"; do
    cp "$artifact" "$REPO_ROOT/dist/"
done
echo "PowerDevil binary and source packages copied to $REPO_ROOT/dist"
