#!/usr/bin/env bash
# Vendored-upstream build for LDtk (deepnight/ldtk), MIT.
#
# Upstream ships no source tarball and no .deb — the Linux release is
# `ubuntu-distribution.zip` containing a single AppImage (an Electron bundle).
# We sha256-pin the zip, extract the AppImage's AppDir at build time, and ship
# that tree to /opt/ldtk with a /usr/bin/ldtk launcher. No runtime FUSE.
#
# Bump: edit UPSTREAM_VERSION + SHA256 (re-pin with the curl|sha256sum below),
# add a debian/changelog entry, then `task build`.
set -euo pipefail

UPSTREAM_VERSION="1.5.3"
SHA256="8bb1c870ab35d2cadfbf08a119d3049e7986a2a80558d2610babc67fcd502566"
ZIP_URL="https://github.com/deepnight/ldtk/releases/download/v${UPSTREAM_VERSION}/ubuntu-distribution.zip"

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
DIST_DIR="$REPO_ROOT/dist"
mkdir -p "$DIST_DIR"

# Build-Depends (CI runners are non-root w/ sudo; containers are root).
if command -v apt-get >/dev/null; then
    # shellcheck disable=SC2015  # root path runs apt-get; non-root falls back to sudo
    _apt() { [[ $EUID -eq 0 ]] && apt-get "$@" || sudo apt-get "$@"; }
    _apt install -y --no-install-recommends unzip ca-certificates curl >/dev/null 2>&1 || true
fi

WORK="$(mktemp -d -t ldtk-XXXXXX)"
trap 'rm -rf "$WORK"' EXIT
cd "$WORK"

echo "=== fetch + verify ldtk ${UPSTREAM_VERSION} ==="
curl -fsSL -o ldtk.zip "$ZIP_URL"
echo "${SHA256}  ldtk.zip" | sha256sum -c -

echo "=== extract AppImage AppDir ==="
unzip -q ldtk.zip
APPIMAGE="$(ls -- *.AppImage)"
chmod +x "$APPIMAGE"
# --appimage-extract is FUSE-free (unpacks the embedded squashfs, no mount).
"./$APPIMAGE" --appimage-extract >/dev/null

SRC="$WORK/ldtk-${UPSTREAM_VERSION}"
mv squashfs-root "$SRC"
cp -a "$SCRIPT_DIR/debian" "$SRC/debian"

echo "=== dpkg-buildpackage (binary-only) ==="
( cd "$SRC" && dpkg-buildpackage -us -uc -b )

mv "$WORK"/ldtk_*.deb "$DIST_DIR/"
echo "=== built: $(ls "$DIST_DIR"/ldtk_*.deb) ==="
