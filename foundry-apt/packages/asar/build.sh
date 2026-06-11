#!/usr/bin/env bash
# Vendored-upstream Debian package build wrapper for asar.
#
# @electron/asar is a pure-JavaScript npm package — there is no
# native compilation step. This script:
#
#   1. Downloads the upstream npm tarball (sha256-verified)
#   2. Extracts to a tmpdir
#   3. Runs npm install --omit=dev to populate node_modules/
#   4. Copies our debian/ tree in
#   5. Runs dpkg-buildpackage -us -uc -b
#   6. Moves the resulting .deb into $REPO_ROOT/dist/
#
# To bump the upstream version: change UPSTREAM_VERSION + SHA256 below,
# add a debian/changelog entry, and re-pin SHA256 with:
#   curl -fsSL https://registry.npmjs.org/@electron/asar/-/asar-<VER>.tgz | sha256sum

set -euo pipefail

for arg in "$@"; do
    case "$arg" in
        -h|--help)
            cat <<EOF
Build asar as a Debian .deb via dpkg-buildpackage.

Usage: build.sh [-h|--help]

Environment overrides:
  ASAR_VERSION  upstream version (default: 4.2.0)
  ASAR_SHA256   sha256 of the npm tarball (must match VERSION)

Output: \$REPO_ROOT/dist/asar_<DEB_VERSION>_all.deb
EOF
            exit 0
            ;;
    esac
done

UPSTREAM_VERSION="${ASAR_VERSION:-4.2.0}"
SHA256="${ASAR_SHA256:-9eaf6ea29e3b67be7dd52a575f6a74e3835dce57576fe7b2a31a70f7f19553bd}"
NPM_URL="https://registry.npmjs.org/@electron/asar/-/asar-${UPSTREAM_VERSION}.tgz"

cd "$(dirname "$0")"
PKG_DIR="$(pwd)"
NAME="asar"
REPO_ROOT="$(cd ../.. && pwd)"
mkdir -p "$REPO_ROOT/dist"

if ! curl -fsI -o /dev/null https://registry.npmjs.org/; then
    echo "ERROR: cannot reach registry.npmjs.org — skipping $NAME build" >&2
    exit 1
fi

WORKDIR=$(mktemp -d -t "${NAME}-build-XXXXXX")
# shellcheck disable=SC2064
trap "rm -rf '$WORKDIR'" EXIT

echo "=== Fetching $NPM_URL ==="
ORIG_TARBALL="$WORKDIR/${NAME}_${UPSTREAM_VERSION}.orig.tar.gz"
curl -fsSL -o "$ORIG_TARBALL" "$NPM_URL"

echo "=== Verifying sha256 ==="
echo "$SHA256  $ORIG_TARBALL" | sha256sum -c -

echo "=== Extracting ==="
# npm tarballs always extract to a top-level "package/" directory
tar -xzf "$ORIG_TARBALL" -C "$WORKDIR"
SRC_DIR="$WORKDIR/${NAME}-${UPSTREAM_VERSION}"
mv "$WORKDIR/package" "$SRC_DIR"

echo "=== Installing build dependencies ==="
if command -v apt-get >/dev/null; then
    _apt() { if [[ $EUID -eq 0 ]]; then apt-get "$@"; else sudo apt-get "$@"; fi; }
    _apt install -y --no-install-recommends nodejs npm debhelper dpkg-dev
fi

echo "=== Installing npm runtime dependencies ==="
( cd "$SRC_DIR" && npm install --omit=dev --ignore-scripts --legacy-peer-deps 2>&1 )

echo "=== Copying debian/ tree into source ==="
cp -a "$PKG_DIR/debian" "$SRC_DIR/"

echo "=== dpkg-buildpackage -us -uc -b ==="
( cd "$SRC_DIR" && dpkg-buildpackage -us -uc -b )

DEB_VERSION=$(sed -n '1s/.*(\(.*\)).*/\1/p' "$PKG_DIR/debian/changelog")
# Architecture: all — deb is always _all.deb regardless of host arch
DEB="$WORKDIR/${NAME}_${DEB_VERSION}_all.deb"
[[ -f "$DEB" ]] || { echo "ERROR: expected .deb not found: $DEB" >&2; ls -la "$WORKDIR"; exit 1; }

mv "$DEB" "$REPO_ROOT/dist/"
OUT="$REPO_ROOT/dist/$(basename "$DEB")"
echo "OK   $OUT  ($(stat -c%s "$OUT") bytes)"
