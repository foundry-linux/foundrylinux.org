#!/usr/bin/env bash
# Vendored-upstream Debian package build wrapper for losslesscut.
#
# LosslessCut ships as a pre-built Electron application bundle, not a source
# tarball — building from source would need yarn 4, electron-vite,
# electron-builder and a network fetch of both Electron and a prebuilt ffmpeg,
# which is not viable inside this repo's CI container. This script therefore
# repackages upstream's official Linux build:
#
#   1. Downloads the upstream AppImage (sha256-verified)
#   2. Unpacks it with --appimage-extract (no FUSE, no root) and renames the
#      resulting squashfs-root/ to losslesscut-<VERSION>/, which is the
#      directory layout dpkg-buildpackage expects
#   3. Copies our debian/ tree into that directory
#   4. Runs dpkg-buildpackage -us -uc -b (binary-only)
#   5. Moves the resulting .deb into $REPO_ROOT/dist/
#
# Why the AppImage and not LosslessCut-linux-x64.tar.bz2: both carry the
# identical Electron + ffmpeg payload, but the AppImage additionally ships
# upstream's own 512x512 PNG icon and a desktop entry, which the tarball
# omits. Nothing mounts an AppImage at runtime — it is unpacked here at build
# time — so the package needs no FUSE support.
#
# To bump the upstream version: change UPSTREAM_VERSION and SHA256 below, add a
# debian/changelog entry via `dch -v <NEW>-1foundry1 -D resolute`, and re-pin
# the sha256 with:
#   curl -fsSL <new-url> | sha256sum
# GitHub also publishes the digest, which is a useful independent check:
#   gh api repos/mifi/lossless-cut/releases/tags/v<NEW> \
#     --jq '.assets[] | select(.name|test("AppImage")) | .digest'

set -euo pipefail

for arg in "$@"; do
    case "$arg" in
        -h|--help)
            cat <<EOF
Build losslesscut as a Debian .deb via dpkg-buildpackage.

Usage: build.sh [-h|--help]

Environment overrides (rarely needed):
  LOSSLESSCUT_VERSION  upstream version (default: 3.69.0)
  LOSSLESSCUT_SHA256   sha256 of the upstream AppImage (must match VERSION)

Output: \$REPO_ROOT/dist/losslesscut_<DEB_VERSION>_<arch>.deb
EOF
            exit 0
            ;;
    esac
done

UPSTREAM_VERSION="${LOSSLESSCUT_VERSION:-3.69.0}"
SHA256="${LOSSLESSCUT_SHA256:-179eaae27bffbe25a6549a4a714474126b57c288cefc307046fc9c6313a12676}"
UPSTREAM_URL="https://github.com/mifi/lossless-cut/releases/download/v${UPSTREAM_VERSION}/LosslessCut-linux-x86_64.AppImage"

cd "$(dirname "$0")"
PKG_DIR="$(pwd)"
NAME="losslesscut"
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
APPIMAGE="$WORKDIR/${NAME}_${UPSTREAM_VERSION}.AppImage"
curl -fsSL -o "$APPIMAGE" "$UPSTREAM_URL"

echo "=== Verifying sha256 ==="
echo "$SHA256  $APPIMAGE" | sha256sum -c -

echo "=== Installing Build-Depends ==="
if command -v apt-get >/dev/null; then
    _apt() { if [[ $EUID -eq 0 ]]; then apt-get "$@"; else sudo apt-get "$@"; fi; }
    # file(1) drives the +x restore in debian/rules; squashfs-tools is the
    # fallback unpack path below. The GUI runtime libraries are needed at
    # build time so dpkg-shlibdeps can resolve the Electron binary's NEEDED
    # sonames to real archive packages — without them it fails outright with
    # "cannot find library", not merely with a missing-info warning.
    _apt install -y --no-install-recommends file squashfs-tools patchelf \
        libasound2t64 libatk-bridge2.0-0t64 libatk1.0-0t64 libatspi2.0-0t64 \
        libcairo2 libcups2t64 libdbus-1-3 libexpat1 libgbm1 libglib2.0-0t64 \
        libgtk-3-0t64 libnspr4 libnss3 libpango-1.0-0 libudev1 libx11-6 \
        libxcb1 libxcomposite1 libxdamage1 libxext6 libxfixes3 libxkbcommon0 \
        libxrandr2
fi

echo "=== Unpacking AppImage ==="
chmod +x "$APPIMAGE"
SRC_DIR="$WORKDIR/${NAME}-${UPSTREAM_VERSION}"
if ( cd "$WORKDIR" && "$APPIMAGE" --appimage-extract >/dev/null 2>&1 ) \
   && [[ -d "$WORKDIR/squashfs-root" ]]; then
    mv "$WORKDIR/squashfs-root" "$SRC_DIR"
else
    # The AppImage runtime could not execute (unusual container, no binfmt).
    # Fall back to reading the appended squashfs directly at its offset.
    echo "--- --appimage-extract failed; falling back to unsquashfs ---"
    OFFSET=$("$APPIMAGE" --appimage-offset 2>/dev/null || true)
    if [[ -z "$OFFSET" ]]; then
        echo "ERROR: cannot determine squashfs offset in $APPIMAGE" >&2
        exit 1
    fi
    unsquashfs -q -o "$OFFSET" -d "$SRC_DIR" "$APPIMAGE"
fi

# Sanity-check the payload before we start building around it.
for required in losslesscut resources/ffmpeg resources/app.asar \
                usr/share/icons/hicolor/512x512/apps/losslesscut.png; do
    [[ -e "$SRC_DIR/$required" ]] || {
        echo "ERROR: unpacked AppImage is missing $required" >&2
        exit 1
    }
done

echo "=== Copying debian/ tree into source ==="
cp -a "$PKG_DIR/debian" "$SRC_DIR/"

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
DEB_VERSION=$(sed -n '1s/.*(\(.*\)).*/\1/p' "$PKG_DIR/debian/changelog")
DEB="$WORKDIR/${NAME}_${DEB_VERSION}_${ARCH}.deb"
[[ -f "$DEB" ]] || { echo "ERROR: expected .deb not found: $DEB" >&2; ls -la "$WORKDIR"; exit 1; }

mv "$DEB" "$REPO_ROOT/dist/"
OUT="$REPO_ROOT/dist/$(basename "$DEB")"
echo "OK   $OUT  ($(stat -c%s "$OUT") bytes)"
