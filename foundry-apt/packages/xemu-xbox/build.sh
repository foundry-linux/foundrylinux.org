#!/usr/bin/env bash
# Vendored-upstream Debian package build wrapper for xemu (original Xbox emulator).
#
# xemu is a QEMU fork. The plain GitHub tag archive is NOT buildable: xemu keeps
# its meson subprojects (imgui, implot, glslang, SPIRV-Reflect, VulkanMemoryAllocator,
# SDL3, berkeley-softfloat, ...) as git submodules, which a tag tarball omits. The
# upstream release asset xemu-<VERSION>.tar.zst vendors all of them, so that is
# what we fetch. It also carries an XEMU_VERSION file, which upstream's
# scripts/xemu-version.sh falls back to when no .git directory is present — so the
# built binary reports the correct version without a git checkout.
#
# This script:
#   1. Downloads the upstream release source archive (sha256-verified)
#   2. Extracts it to xemu-<VERSION>/ (dpkg-buildpackage expects <srcname>-<upstream>/)
#   3. Copies our debian/ tree into that directory
#   4. Runs dpkg-buildpackage -us -uc -b (binary-only)
#   5. Moves the resulting .deb into $REPO_ROOT/dist/
#
# To bump the upstream version: change UPSTREAM_VERSION + SHA256 below, add a
# debian/changelog entry via `dch -v <NEW>-1foundry1 -D resolute`, re-pin the
# sha256 with:
#   curl -fsSL <new-url> | sha256sum

set -euo pipefail

for arg in "$@"; do
    case "$arg" in
        -h|--help)
            cat <<EOF
Build xemu as a Debian .deb via dpkg-buildpackage.

Usage: build.sh [-h|--help]

Environment overrides (rarely needed):
  XEMU_VERSION  upstream version (default: 0.8.136)
  XEMU_SHA256   sha256 of the upstream .tar.zst source archive (must match VERSION)

Note: this is a QEMU fork and the build is correspondingly heavy — expect on the
order of 10-20 minutes and several GB of scratch space.

Output: \$REPO_ROOT/dist/xemu_<DEB_VERSION>_amd64.deb
EOF
            exit 0
            ;;
    esac
done

UPSTREAM_VERSION="${XEMU_VERSION:-0.8.136}"
SHA256="${XEMU_SHA256:-92a09305a87d7dabaecc335fa54ca888117aab10fe22af6ad73fc0d3cdd20c1f}"
UPSTREAM_URL="https://github.com/xemu-project/xemu/releases/download/v${UPSTREAM_VERSION}/xemu-${UPSTREAM_VERSION}.tar.zst"

cd "$(dirname "$0")"
PKG_DIR="$(pwd)"
NAME="xemu"
REPO_ROOT="$(cd ../.. && pwd)"
mkdir -p "$REPO_ROOT/dist"

if ! curl -fsI -o /dev/null https://github.com/; then
    echo "ERROR: cannot reach github.com — skipping $NAME build" >&2
    exit 1
fi

WORKDIR=$(mktemp -d -t "${NAME}-build-XXXXXX")
# shellcheck disable=SC2064
trap "rm -rf '$WORKDIR'" EXIT

echo "=== Installing Build-Depends ==="
if command -v apt-get >/dev/null; then
    _apt() { if [[ $EUID -eq 0 ]]; then apt-get "$@"; else sudo apt-get "$@"; fi; }
    _apt install -y --no-install-recommends \
        zstd meson ninja-build cmake pkgconf python3 python3-venv python3-tomli \
        python3-yaml libepoxy-dev libpixman-1-dev libsdl2-dev \
        libsamplerate0-dev libglib2.0-dev zlib1g-dev libssl-dev libpcap-dev \
        libslirp-dev libvulkan-dev libusb-1.0-0-dev libcurl4-gnutls-dev \
        libpipewire-0.3-dev
fi

echo "=== Fetching $UPSTREAM_URL ==="
TARBALL="$WORKDIR/${NAME}-${UPSTREAM_VERSION}.tar.zst"
curl -fsSL -o "$TARBALL" "$UPSTREAM_URL"

echo "=== Verifying sha256 ==="
echo "$SHA256  $TARBALL" | sha256sum -c -

echo "=== Extracting ==="
tar --use-compress-program=unzstd -xf "$TARBALL" -C "$WORKDIR"
SRC_DIR="$WORKDIR/${NAME}-${UPSTREAM_VERSION}"
[[ -f "$SRC_DIR/build.sh" ]] || {
    echo "ERROR: expected upstream build.sh in $SRC_DIR" >&2; ls -la "$WORKDIR"; exit 1; }

echo "=== Copying debian/ tree into source ==="
# xemu vendors its OWN debian/ directory, inherited from Debian's QEMU packaging
# and long stale (changelog frozen at 0.5.2/2021, debhelper compat 11,
# Standards-Version 3.9.8, d/watch tracking download.qemu.org). It must be
# removed rather than copied over: `cp -a` merges into an existing directory, so
# upstream's debian/compat would survive and collide with our
# debhelper-compat (= 13), failing `debian/rules clean` with
# "compat level specified both in debian/compat and in debian/control".
rm -rf "$SRC_DIR/debian"
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

DEB_VERSION=$(sed -n '1s/.*(\(.*\)).*/\1/p' "$PKG_DIR/debian/changelog")
# Architecture is fixed: upstream supports x86_64 Linux (Architecture: amd64).
DEB="$WORKDIR/${NAME}_${DEB_VERSION}_amd64.deb"
[[ -f "$DEB" ]] || { echo "ERROR: expected .deb not found: $DEB" >&2; ls -la "$WORKDIR"; exit 1; }

mv "$DEB" "$REPO_ROOT/dist/"
OUT="$REPO_ROOT/dist/$(basename "$DEB")"
echo "OK   $OUT  ($(stat -c%s "$OUT") bytes)"
