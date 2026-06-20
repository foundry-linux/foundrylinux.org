#!/usr/bin/env bash
# Vendored-upstream Debian package build wrapper for ruff.
#
# Ruff is a Rust binary distributed as a pre-built wheel — there is no
# source compilation step. This script:
#
#   1. Downloads the upstream manylinux_2_17 x86_64 wheel (sha256-verified)
#   2. Unzips it into a source tree
#   3. Copies our debian/ tree in
#   4. Runs dpkg-buildpackage -us -uc -b
#   5. Moves the resulting .deb into $REPO_ROOT/dist/
#
# To bump the upstream version: change UPSTREAM_VERSION, SHA256, and
# UPSTREAM_URL below, add a debian/changelog entry, and update
# debian/rules UPSTREAM variable to match.

set -euo pipefail

for arg in "$@"; do
    case "$arg" in
        -h|--help)
            cat <<EOF
Build ruff as a Debian .deb via dpkg-buildpackage.

Usage: build.sh [-h|--help]

Environment overrides (rarely needed):
  RUFF_VERSION  upstream version (default: 0.15.15)
  RUFF_SHA256   sha256 of the upstream wheel (must match VERSION)

Output: \$REPO_ROOT/dist/ruff_<DEB_VERSION>_amd64.deb
EOF
            exit 0
            ;;
    esac
done

UPSTREAM_VERSION="${RUFF_VERSION:-0.15.15}"
SHA256="${RUFF_SHA256:-48decfa11d740de4889de623be1463308346312f2409a56e24aa280c86162dc4}"
UPSTREAM_URL="https://files.pythonhosted.org/packages/fe/1c/e6e5e568f22be4fb05d6244234aba384c06b451252453b821e1a529263cf/ruff-${UPSTREAM_VERSION}-py3-none-manylinux_2_17_x86_64.manylinux2014_x86_64.whl"

cd "$(dirname "$0")"
PKG_DIR="$(pwd)"
NAME="ruff"
REPO_ROOT="$(cd ../.. && pwd)"
mkdir -p "$REPO_ROOT/dist"

if ! curl -fsI -o /dev/null https://files.pythonhosted.org/; then
    echo "ERROR: cannot reach files.pythonhosted.org — skipping $NAME build" >&2
    exit 1
fi

WORKDIR=$(mktemp -d -t "${NAME}-build-XXXXXX")
# shellcheck disable=SC2064
trap "rm -rf '$WORKDIR'" EXIT

echo "=== Fetching $UPSTREAM_URL ==="
WHEEL="$WORKDIR/ruff-${UPSTREAM_VERSION}.whl"
curl -fsSL -o "$WHEEL" "$UPSTREAM_URL"

echo "=== Verifying sha256 ==="
echo "$SHA256  $WHEEL" | sha256sum -c -

echo "=== Extracting wheel ==="
# Wheel is a zip; unzip flat into versioned source dir
SRC_DIR="$WORKDIR/${NAME}-${UPSTREAM_VERSION}"
mkdir "$SRC_DIR"
unzip -q "$WHEEL" -d "$SRC_DIR/"

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
ARCH=$(dpkg --print-architecture)
DEB="$WORKDIR/${NAME}_${DEB_VERSION}_${ARCH}.deb"
[[ -f "$DEB" ]] || { echo "ERROR: expected .deb not found: $DEB" >&2; ls -la "$WORKDIR"; exit 1; }

mv "$DEB" "$REPO_ROOT/dist/"
OUT="$REPO_ROOT/dist/$(basename "$DEB")"
echo "OK   $OUT  ($(stat -c%s "$OUT") bytes)"
