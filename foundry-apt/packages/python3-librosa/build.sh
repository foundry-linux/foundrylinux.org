#!/usr/bin/env bash
# Vendored-upstream Debian package build wrapper for python3-librosa.
#
#   1. Downloads the upstream PyPI sdist (sha256-verified)
#   2. Extracts to a tmpdir
#   3. Copies our debian/ tree into the upstream tree
#   4. Runs dpkg-buildpackage -us -uc -b
#   5. Moves the resulting .deb into $REPO_ROOT/dist/
#
# To bump the upstream version: change UPSTREAM_VERSION + SHA256 below, add
# a debian/changelog entry via `dch -v <NEW>-1foundry1 -D resolute "..."`,
# re-pin the sha256 with:
#   curl -fsSL <new-pypi-url> | sha256sum

set -euo pipefail

for arg in "$@"; do
    case "$arg" in
        -h|--help)
            cat <<EOF
Build python3-librosa as a Debian .deb via dpkg-buildpackage.

Usage: build.sh [-h|--help]

Environment overrides (rarely needed):
  LIBROSA_VERSION  upstream version (default: 0.11.0)
  LIBROSA_SHA256   sha256 of the upstream sdist (must match VERSION)

Output: \$REPO_ROOT/dist/python3-librosa_<DEB_VERSION>_all.deb
EOF
            exit 0
            ;;
    esac
done

UPSTREAM_VERSION="${LIBROSA_VERSION:-0.11.0}"
SHA256="${LIBROSA_SHA256:-f5ed951ca189b375bbe2e33b2abd7e040ceeee302b9bbaeeffdfddb8d0ace908}"
UPSTREAM_URL="https://files.pythonhosted.org/packages/64/36/360b5aafa0238e29758729e9486c6ed92a6f37fa403b7875e06c115cdf4a/librosa-${UPSTREAM_VERSION}.tar.gz"

cd "$(dirname "$0")"
PKG_DIR="$(pwd)"
NAME="python3-librosa"
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
ORIG_TARBALL="$WORKDIR/librosa_${UPSTREAM_VERSION}.orig.tar.gz"
curl -fsSL -o "$ORIG_TARBALL" "$UPSTREAM_URL"

echo "=== Verifying sha256 ==="
echo "$SHA256  $ORIG_TARBALL" | sha256sum -c -

echo "=== Extracting ==="
tar -xzf "$ORIG_TARBALL" -C "$WORKDIR"
SRC_DIR="$WORKDIR/${NAME}-${UPSTREAM_VERSION}"
mv "$WORKDIR/librosa-${UPSTREAM_VERSION}" "$SRC_DIR"

echo "=== Copying debian/ tree into source ==="
cp -a "$PKG_DIR/debian" "$SRC_DIR/"

echo "=== Installing Build-Depends ==="
if command -v apt-get >/dev/null; then
    _apt() { if [[ $EUID -eq 0 ]]; then apt-get "$@"; else sudo apt-get "$@"; fi; }
    _apt install -y --no-install-recommends \
        dh-python python3-all python3-setuptools
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
DEB="$WORKDIR/${NAME}_${DEB_VERSION}_all.deb"
[[ -f "$DEB" ]] || { echo "ERROR: expected .deb not found: $DEB" >&2; ls -la "$WORKDIR"; exit 1; }

mv "$DEB" "$REPO_ROOT/dist/"
OUT="$REPO_ROOT/dist/$(basename "$DEB")"
echo "OK   $OUT  ($(stat -c%s "$OUT") bytes)"
