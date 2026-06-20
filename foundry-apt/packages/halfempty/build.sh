#!/usr/bin/env bash
# Vendored-upstream Debian package build wrapper for halfempty.
#
#   1. Downloads the upstream GitHub release tarball (sha256-verified)
#   2. Extracts to a tmpdir
#   3. Copies our debian/ tree into the upstream tree
#   4. Runs dpkg-buildpackage -us -uc -b
#   5. Moves the resulting .deb into $REPO_ROOT/dist/
#
# To bump the upstream version: change UPSTREAM_VERSION + SHA256 below, add a
# debian/changelog entry via `dch -v <NEW>-1foundry1 -D resolute "..."`,
# re-pin the sha256 with:
#   curl -fsSL <new-github-tarball-url> | sha256sum

set -euo pipefail

for arg in "$@"; do
    case "$arg" in
        -h|--help)
            cat <<EOF
Build halfempty as a Debian .deb via dpkg-buildpackage.

Usage: build.sh [-h|--help]

Environment overrides (rarely needed):
  HALFEMPTY_VERSION  upstream version (default: 0.40)
  HALFEMPTY_SHA256   sha256 of the upstream tarball (must match VERSION)

Output: \$REPO_ROOT/dist/halfempty_<DEB_VERSION>_<arch>.deb
EOF
            exit 0
            ;;
    esac
done

UPSTREAM_VERSION="${HALFEMPTY_VERSION:-0.40}"
SHA256="${HALFEMPTY_SHA256:-6013c6e37c9dc727bb9066ee15ededbb59a6e97c74f4879f6ea00aac5fb94fa5}"
UPSTREAM_URL="https://github.com/googleprojectzero/halfempty/archive/refs/tags/v${UPSTREAM_VERSION}.tar.gz"

cd "$(dirname "$0")"
PKG_DIR="$(pwd)"
NAME="halfempty"
REPO_ROOT="$(cd ../.. && pwd)"
ARCH="$(dpkg --print-architecture)"
mkdir -p "$REPO_ROOT/dist"

if ! curl -fsI -o /dev/null https://github.com/; then
    echo "ERROR: cannot reach github.com — skipping $NAME build" >&2
    exit 1
fi

WORKDIR=$(mktemp -d -t "${NAME}-build-XXXXXX")
# shellcheck disable=SC2064
trap "rm -rf '$WORKDIR'" EXIT

echo "=== Fetching $UPSTREAM_URL ==="
ORIG_TARBALL="$WORKDIR/${NAME}_${UPSTREAM_VERSION}.orig.tar.gz"
curl -fsSL -o "$ORIG_TARBALL" "$UPSTREAM_URL"

echo "=== Verifying sha256 ==="
echo "$SHA256  $ORIG_TARBALL" | sha256sum -c -

echo "=== Extracting ==="
tar -xzf "$ORIG_TARBALL" -C "$WORKDIR"
# GitHub archive of tag v0.40 extracts to halfempty-0.40/ == <NAME>-<VERSION>/
SRC_DIR="$WORKDIR/${NAME}-${UPSTREAM_VERSION}"
[[ -d "$SRC_DIR" ]] || { echo "ERROR: expected source dir $SRC_DIR not found" >&2; ls -la "$WORKDIR"; exit 1; }

echo "=== Copying debian/ tree into source ==="
cp -a "$PKG_DIR/debian" "$SRC_DIR/"

echo "=== Installing Build-Depends ==="
if command -v apt-get >/dev/null; then
    _apt() { if [[ $EUID -eq 0 ]]; then apt-get "$@"; else sudo apt-get "$@"; fi; }
    _apt install -y --no-install-recommends \
        pkg-config libglib2.0-dev bsdextrautils
fi

echo "=== dpkg-buildpackage -us -uc -b ==="
( cd "$SRC_DIR" && dpkg-buildpackage -us -uc -b )

DEB_VERSION=$(sed -n '1s/.*(\(.*\)).*/\1/p' "$PKG_DIR/debian/changelog")
DEB="$WORKDIR/${NAME}_${DEB_VERSION}_${ARCH}.deb"
[[ -f "$DEB" ]] || { echo "ERROR: expected .deb not found: $DEB" >&2; ls -la "$WORKDIR"; exit 1; }

mv "$DEB" "$REPO_ROOT/dist/"
OUT="$REPO_ROOT/dist/$(basename "$DEB")"
echo "OK   $OUT  ($(stat -c%s "$OUT") bytes)"
