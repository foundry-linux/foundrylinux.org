#!/usr/bin/env bash
set -euo pipefail

for arg in "$@"; do
    case "$arg" in
        -h|--help)
            cat <<'EOF'
Build the official Electron 42 runtime as a versioned Debian package.

Usage: build.sh [-h|--help]

Environment overrides (must be changed together):
  ELECTRON_VERSION  upstream 42.x version (default: 42.3.0)
  ELECTRON_SHA256   sha256 of the official Linux x64 zip

Output: dist/electron-runtime-42_<DEB_VERSION>_amd64.deb and source artifacts
EOF
            exit 0
            ;;
        *)
            echo "ERROR: unknown argument: $arg" >&2
            exit 2
            ;;
    esac
done

UPSTREAM_VERSION="${ELECTRON_VERSION:-42.9.3}"
SHA256="${ELECTRON_SHA256:-46fc1cd5d70de57c372fbc0f36870c4c4d80b127a0d452d80bd577c5a7d39b7d}"
[[ "$UPSTREAM_VERSION" == 42.* ]] || {
    echo "ERROR: electron-runtime-42 only accepts Electron 42.x, got $UPSTREAM_VERSION" >&2
    exit 2
}
URL="https://github.com/electron/electron/releases/download/v${UPSTREAM_VERSION}/electron-v${UPSTREAM_VERSION}-linux-x64.zip"

cd "$(dirname "$0")"
PKG_DIR="$(pwd)"
REPO_ROOT="$(cd ../.. && pwd)"
WORKDIR=$(mktemp -d -t electron-runtime-42-build-XXXXXX)
trap 'rm -rf "$WORKDIR"' EXIT
SRC_DIR="$WORKDIR/electron-runtime-42-${UPSTREAM_VERSION}"
mkdir -p "$SRC_DIR" "$REPO_ROOT/dist"

echo "=== Fetching Electron ${UPSTREAM_VERSION} ==="
curl -fsSL -o "$WORKDIR/electron.zip" "$URL"
echo "$SHA256  $WORKDIR/electron.zip" | sha256sum -c -
unzip -q "$WORKDIR/electron.zip" -d "$SRC_DIR"
test -x "$SRC_DIR/electron"
test -f "$SRC_DIR/resources/default_app.asar"
cp -a "$PKG_DIR/debian" "$SRC_DIR/"

# Publishable repositories need source artifacts and a Sources index even
# though this package deliberately redistributes upstream's official binary.
if [[ -f "$REPO_ROOT/scripts/lib-source-build.sh" ]]; then
    # REPO_ROOT is resolved above.
    # shellcheck disable=SC1091
    source "$REPO_ROOT/scripts/lib-source-build.sh"
    emit_source_package "$SRC_DIR" "$WORKDIR" "$REPO_ROOT" || true
fi

if command -v apt-get >/dev/null; then
    _apt() { if [[ $EUID -eq 0 ]]; then apt-get "$@"; else sudo apt-get "$@"; fi; }
    _apt install -y --no-install-recommends unzip libasound2t64 libatk-bridge2.0-0t64 \
        libatk1.0-0t64 libatspi2.0-0t64 libcairo2 libcups2t64 libdbus-1-3 \
        libexpat1 libgbm1 libglib2.0-0t64 libgtk-3-0t64 libnspr4 libnss3 \
        libpango-1.0-0 libudev1 libx11-6 libxcb1 libxcomposite1 libxdamage1 \
        libxext6 libxfixes3 libxkbcommon0 libxrandr2
fi

( cd "$SRC_DIR" && dpkg-buildpackage -us -uc -b )
DEB_VERSION=$(dpkg-parsechangelog -l "$PKG_DIR/debian/changelog" -SVersion)
DEB="$WORKDIR/electron-runtime-42_${DEB_VERSION}_amd64.deb"
test -f "$DEB"
mv "$DEB" "$REPO_ROOT/dist/"
echo "OK   $REPO_ROOT/dist/$(basename "$DEB")"
