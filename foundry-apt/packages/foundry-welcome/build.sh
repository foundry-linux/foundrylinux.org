#!/usr/bin/env bash
# Build foundry-welcome as a Debian .deb via dpkg-buildpackage.
#
# foundry-welcome is a native Qt6/QML application (our own source, no upstream
# tarball).  It must be built on the target Ubuntu release so dpkg-shlibdeps
# captures the correct Qt6 soname version.
#
# Build deps installed by this script:
#   cmake qt6-base-dev qt6-declarative-dev debhelper-compat
#   extra-cmake-modules qt6-declarative-private-dev

set -euo pipefail

for arg in "$@"; do
    case "$arg" in
        -h|--help)
            cat <<'EOF'
Build foundry-welcome as a Debian .deb via dpkg-buildpackage.

Usage: build.sh [-h|--help]

Output: $REPO_ROOT/dist/foundry-welcome_<VERSION>_amd64.deb
EOF
            exit 0
            ;;
    esac
done

cd "$(dirname "$0")"
PKG_DIR="$(pwd)"
NAME="foundry-welcome"
REPO_ROOT="$(cd ../.. && pwd)"
mkdir -p "$REPO_ROOT/dist"

echo "=== Installing Build-Depends ==="
if command -v apt-get >/dev/null; then
    _apt() { if [[ $EUID -eq 0 ]]; then apt-get "$@"; else sudo apt-get "$@"; fi; }
    _apt install -y --no-install-recommends \
        build-essential debhelper cmake \
        qt6-base-dev qt6-declarative-dev \
        libgl-dev libvulkan-dev
fi

WORKDIR=$(mktemp -d -t "${NAME}-build-XXXXXX")
# shellcheck disable=SC2064  # expand $WORKDIR now so the trap captures the value
trap "rm -rf '$WORKDIR'" EXIT

DEB_VERSION=$(awk 'NR==1 {match($0, /\(([^)]+)\)/); print substr($0, RSTART+1, RLENGTH-2)}' "$PKG_DIR/debian/changelog")
SRC_DIR="$WORKDIR/${NAME}-${DEB_VERSION}"
mkdir -p "$SRC_DIR"

echo "=== Copying source tree ==="
cp -a "$PKG_DIR/." "$SRC_DIR/"

echo "=== dpkg-buildpackage -us -uc -b ==="
# Emit the source package (.dsc + tarballs) -> dist/ for the apt Sources index
# (best-effort; never blocks the binary build below). See scripts/lib-source-build.sh.
if [[ -f "$REPO_ROOT/scripts/lib-source-build.sh" ]]; then
    # shellcheck source=/dev/null
    source "$REPO_ROOT/scripts/lib-source-build.sh"
    emit_source_package "$SRC_DIR" "$WORKDIR" "$REPO_ROOT" || true
fi
( cd "$SRC_DIR" && dpkg-buildpackage -us -uc -b )

ARCH=$(dpkg --print-architecture)
DEB="$WORKDIR/${NAME}_${DEB_VERSION}_${ARCH}.deb"
[[ -f "$DEB" ]] || { echo "ERROR: expected .deb not found: $DEB" >&2; ls -la "$WORKDIR"; exit 1; }

mv "$DEB" "$REPO_ROOT/dist/"
OUT="$REPO_ROOT/dist/$(basename "$DEB")"
echo "OK   $OUT  ($(stat -c%s "$OUT") bytes)"
