#!/usr/bin/env bash
# Vendored-upstream Debian package build wrapper for mesen2 (Mesen emulator).
#
# Mesen2 ships an official self-contained Linux x64 release zip (not a source
# tarball) — a .NET ahead-of-time-compiled, PublishSingleFile binary. There is
# no compile step (building from source needs the .NET 8 SDK + SDL2 + clang,
# per upstream COMPILING.md). This script:
#
#   1. Downloads the upstream pre-built zip (sha256-verified)
#   2. Extracts the single `Mesen` binary into mesen2-<VERSION>/ (dpkg-buildpackage
#      expects <srcname>-<upstream>/ as the build directory)
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
Build mesen2 as a Debian .deb via dpkg-buildpackage.

Usage: build.sh [-h|--help]

Environment overrides (rarely needed):
  MESEN2_VERSION  upstream version (default: 2.1.1)
  MESEN2_SHA256   sha256 of the upstream Linux x64 zip (must match VERSION)

Output: \$REPO_ROOT/dist/mesen2_<DEB_VERSION>_amd64.deb
EOF
            exit 0
            ;;
    esac
done

UPSTREAM_VERSION="${MESEN2_VERSION:-2.1.1}"
SHA256="${MESEN2_SHA256:-7a9947575cc198209f743fef83fb2b702b786ea705506bdf3f2aea01ab7c1ce9}"
UPSTREAM_URL="https://github.com/SourMesen/Mesen2/releases/download/${UPSTREAM_VERSION}/Mesen_${UPSTREAM_VERSION}_Linux_x64.zip"

cd "$(dirname "$0")"
PKG_DIR="$(pwd)"
NAME="mesen2"
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

echo "=== Extracting ==="
SRC_DIR="$WORKDIR/${NAME}-${UPSTREAM_VERSION}"
mkdir -p "$SRC_DIR"
# The zip contains a single file named `Mesen`.
unzip -q "$ZIP" -d "$SRC_DIR"
[[ -f "$SRC_DIR/Mesen" ]] || { echo "ERROR: expected Mesen binary after extract" >&2; ls -la "$SRC_DIR"; exit 1; }

echo "=== Copying debian/ tree into source ==="
cp -a "$PKG_DIR/debian" "$SRC_DIR/"

echo "=== Installing Build-Depends ==="
if command -v apt-get >/dev/null; then
    _apt() { if [[ $EUID -eq 0 ]]; then apt-get "$@"; else sudo apt-get "$@"; fi; }
    _apt install -y --no-install-recommends unzip
fi

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
# Architecture is fixed: the upstream zip is x64-only (Architecture: amd64).
DEB="$WORKDIR/${NAME}_${DEB_VERSION}_amd64.deb"
[[ -f "$DEB" ]] || { echo "ERROR: expected .deb not found: $DEB" >&2; ls -la "$WORKDIR"; exit 1; }

mv "$DEB" "$REPO_ROOT/dist/"
OUT="$REPO_ROOT/dist/$(basename "$DEB")"
echo "OK   $OUT  ($(stat -c%s "$OUT") bytes)"
