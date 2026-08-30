#!/usr/bin/env bash
# Repackage the official draw.io Desktop .deb as an application-only package
# using Foundry's versioned Electron 42 runtime.
set -euo pipefail

for arg in "$@"; do
    case "$arg" in
        -h|--help)
            cat <<'EOF'
Build draw.io Desktop against the shared Electron 42 runtime.

Usage: build.sh [-h|--help]

Environment overrides (must be changed together):
  DRAWIO_VERSION  upstream version (default: 31.3.1)
  DRAWIO_SHA256   sha256 of the official amd64 .deb

Output: dist/drawio-desktop_<DEB_VERSION>_amd64.deb and source artifacts
EOF
            exit 0
            ;;
        *) echo "ERROR: unknown argument: $arg" >&2; exit 2 ;;
    esac
done

UPSTREAM_VERSION="${DRAWIO_VERSION:-31.3.1}"
SHA256="${DRAWIO_SHA256:-5314046cf4059e9484e876a6e5ba259502030c1cf185d63062c93e83d00a21ac}"
URL="https://github.com/jgraph/drawio-desktop/releases/download/v${UPSTREAM_VERSION}/drawio-amd64-${UPSTREAM_VERSION}.deb"

cd "$(dirname "$0")"
PKG_DIR=$(pwd)
REPO_ROOT=$(cd ../.. && pwd)
WORKDIR=$(mktemp -d -t drawio-desktop-build-XXXXXX)
trap 'rm -rf "$WORKDIR"' EXIT
SRC_DIR="$WORKDIR/drawio-desktop-${UPSTREAM_VERSION}"
mkdir -p "$SRC_DIR/upstream-root" "$REPO_ROOT/dist"

echo "=== Fetching draw.io Desktop ${UPSTREAM_VERSION} ==="
curl -fsSL -o "$WORKDIR/upstream.deb" "$URL"
echo "$SHA256  $WORKDIR/upstream.deb" | sha256sum -c -
dpkg-deb -x "$WORKDIR/upstream.deb" "$SRC_DIR/upstream-root"

for required in opt/drawio/resources/app.asar usr/share/applications/drawio.desktop \
                usr/share/mime/packages/drawio.xml; do
    test -f "$SRC_DIR/upstream-root/$required" || {
        echo "ERROR: upstream package is missing $required" >&2
        exit 1
    }
done

# Keep only the app payload from /opt/drawio. This also removes app-update.yml:
# APT owns updates for this build, and the generic runtime is packaged elsewhere.
mv "$SRC_DIR/upstream-root/opt/drawio/resources/app.asar" "$WORKDIR/app.asar"
rm -rf "$SRC_DIR/upstream-root/opt/drawio"
install -d "$SRC_DIR/upstream-root/opt/drawio/resources"
mv "$WORKDIR/app.asar" "$SRC_DIR/upstream-root/opt/drawio/resources/app.asar"
cp -a "$PKG_DIR/debian" "$SRC_DIR/"

if [[ -f "$REPO_ROOT/scripts/lib-source-build.sh" ]]; then
    # REPO_ROOT is resolved above.
    # shellcheck disable=SC1091
    source "$REPO_ROOT/scripts/lib-source-build.sh"
    emit_source_package "$SRC_DIR" "$WORKDIR" "$REPO_ROOT" || true
fi

( cd "$SRC_DIR" && dpkg-buildpackage -us -uc -b )
DEB_VERSION=$(dpkg-parsechangelog -l "$PKG_DIR/debian/changelog" -SVersion)
DEB="$WORKDIR/drawio-desktop_${DEB_VERSION}_amd64.deb"
test -f "$DEB"
mv "$DEB" "$REPO_ROOT/dist/"
echo "OK   $REPO_ROOT/dist/$(basename "$DEB")"
