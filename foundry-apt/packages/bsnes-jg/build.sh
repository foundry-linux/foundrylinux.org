#!/usr/bin/env bash
# Vendored-upstream Debian package build wrapper for bsnes-jg.
#
# Generated via the /package skill (~/.claude/skills/package/). Entry point that
# foundry-apt's build-all.sh invokes:
#
#   1. Fetches the upstream source tarball (sha256-verified)
#   2. Installs Build-Depends + The Jolly Good API headers (jg.h, build-only)
#   3. Copies our debian/ tree into the upstream tree
#   4. Runs dpkg-buildpackage -us -uc -b
#   5. Moves the resulting .deb into $REPO_ROOT/dist/
#
# Build deps the CI runner needs (publish.yml installs build-essential debhelper
# dpkg-dev lintian; this script installs the rest):
#   pkg-config libsdl2-dev libsamplerate0-dev
# (bsnes-jg 2.1.0's example frontend uses SDL2; master has since moved to SDL3.)
# Plus The Jolly Good API headers (gitlab.com/jgemu/jg) — not a Debian package,
# so this script make-installs them to /usr at build time. They are build-only
# (jg.h is an interface header; nothing from jg is shipped in the .deb). If
# foundry later packages more Jolly Good cores, factor jg.h into a shared
# `jollygood-api` build-dep and drop the fetch here.
#
# To bump: change *_VERSION + *_SHA256 below, add a debian/changelog entry via
#   dch -v <NEW>-1foundry1 -D resolute
# and re-pin with:
#   curl -fsSL https://gitlab.com/jgemu/bsnes/-/archive/<VER>/bsnes-<VER>.tar.gz | sha256sum

set -euo pipefail

for arg in "$@"; do
    case "$arg" in
        -h|--help)
            cat <<EOF
Build bsnes-jg as a Debian .deb via dpkg-buildpackage.

Usage: build.sh [-h|--help]

Environment overrides (rarely needed):
  BSNES_JG_VERSION   upstream version (default: 2.1.0)
  BSNES_JG_SHA256    sha256 of the upstream tarball (must match VERSION)
  JG_API_VERSION     Jolly Good API version (default: 1.0.0)
  JG_API_SHA256      sha256 of the jg-api tarball

Output: \$REPO_ROOT/dist/bsnes-jg_<DEB_VERSION>_<arch>.deb
EOF
            exit 0
            ;;
    esac
done

UPSTREAM_VERSION="${BSNES_JG_VERSION:-2.1.0}"
SHA256="${BSNES_JG_SHA256:-a8e0fd36711406198afe1110ddc6960c9d795f4ab73d0badd8878396ac3d0c42}"
UPSTREAM_URL="https://gitlab.com/jgemu/bsnes/-/archive/${UPSTREAM_VERSION}/bsnes-${UPSTREAM_VERSION}.tar.gz"

JG_API_VERSION="${JG_API_VERSION:-1.0.0}"
JG_API_SHA256="${JG_API_SHA256:-7b796430a158adeba6fe53c551868b268e0c9c54a8bedb84b3901a7c92ccad9e}"
JG_API_URL="https://gitlab.com/jgemu/jg/-/archive/${JG_API_VERSION}/jg-${JG_API_VERSION}.tar.gz"

cd "$(dirname "$0")"
PKG_DIR="$(pwd)"
NAME="bsnes-jg"
REPO_ROOT="$(cd ../.. && pwd)"
mkdir -p "$REPO_ROOT/dist"

if ! curl -fsI -o /dev/null https://gitlab.com/; then
    echo "ERROR: cannot reach gitlab.com — skipping $NAME build" >&2
    exit 1
fi

WORKDIR=$(mktemp -d -t "${NAME}-build-XXXXXX")
# shellcheck disable=SC2064  # expand $WORKDIR now so the trap captures the value
trap "rm -rf '$WORKDIR'" EXIT

run_root() { if [[ $EUID -eq 0 ]]; then "$@"; else sudo "$@"; fi; }

echo "=== Fetching $UPSTREAM_URL ==="
ORIG_TARBALL="$WORKDIR/${NAME}_${UPSTREAM_VERSION}.orig.tar.gz"
curl -fsSL -o "$ORIG_TARBALL" "$UPSTREAM_URL"

echo "=== Verifying sha256 ==="
echo "$SHA256  $ORIG_TARBALL" | sha256sum -c -

echo "=== Extracting ==="
tar -xzf "$ORIG_TARBALL" -C "$WORKDIR"
# GitLab's archive top-dir is the repo name (bsnes-<ver>), not the package name.
EXTRACTED="$WORKDIR/bsnes-${UPSTREAM_VERSION}"
SRC_DIR="$WORKDIR/${NAME}-${UPSTREAM_VERSION}"
[[ -d "$EXTRACTED" ]] || { echo "ERROR: expected $EXTRACTED after extract" >&2; ls "$WORKDIR"; exit 1; }
mv "$EXTRACTED" "$SRC_DIR"

echo "=== Installing Build-Depends ==="
if command -v apt-get >/dev/null; then
    run_root apt-get install -y --no-install-recommends \
        pkg-config libsdl2-dev libsamplerate0-dev
fi

echo "=== Installing The Jolly Good API headers (jg.h — build-only) ==="
JG_TARBALL="$WORKDIR/jg_${JG_API_VERSION}.tar.gz"
curl -fsSL -o "$JG_TARBALL" "$JG_API_URL"
echo "$JG_API_SHA256  $JG_TARBALL" | sha256sum -c -
tar -xzf "$JG_TARBALL" -C "$WORKDIR"
( cd "$WORKDIR/jg-${JG_API_VERSION}" && run_root make install PREFIX=/usr )

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
