#!/usr/bin/env bash
#
# Vendored-upstream Debian package build wrapper for x-emulators (LGB's Xemu).
# Generated via the /package skill (~/.claude/skills/package/). Entry point that
# foundry-apt's build-all.sh invokes:
#
#   1. Fetches the upstream source tarball (sha256-verified)
#   2. Installs Build-Depends
#   3. Copies our debian/ tree into the upstream tree
#   4. Runs dpkg-buildpackage -us -uc -b
#   5. Moves the resulting .deb into $REPO_ROOT/dist/
#
# Upstream publishes NO tags and NO releases — master is the only ref. So we pin
# a commit and use a git-snapshot Debian version (0~gitYYYYMMDD.<shortsha>),
# which sorts below any future real release.
#
# To bump: change UPSTREAM_COMMIT + COMMIT_DATE + SHA256 below, add a
# debian/changelog entry via
#   dch -v 0~git<YYYYMMDD>.<shortsha>-1foundry1 -D resolute
# and re-pin with:
#   curl -fsSL https://github.com/lgblgblgb/xemu/archive/<sha>.tar.gz | sha256sum
#
# NOTE: the package is deliberately NOT called "xemu" — that name belongs to the
# unrelated original Xbox emulator (https://xemu.app), packaged separately here.

set -euo pipefail

for arg in "$@"; do
    case "$arg" in
        -h|--help)
            cat <<EOF
Build x-emulators (LGB's Xemu suite) as a Debian .deb via dpkg-buildpackage.

Usage: build.sh [-h|--help]

Environment overrides (rarely needed):
  XEMU_LGB_COMMIT   upstream git commit (default: 40dfef0d1d5f56be2469492715c12bdb32c75b67)
  XEMU_LGB_SHA256   sha256 of the upstream tarball (must match COMMIT)

Output: \$REPO_ROOT/dist/x-emulators_<DEB_VERSION>_<arch>.deb
EOF
            exit 0
            ;;
    esac
done

UPSTREAM_COMMIT="${XEMU_LGB_COMMIT:-40dfef0d1d5f56be2469492715c12bdb32c75b67}"
SHA256="${XEMU_LGB_SHA256:-58549a7224b84be093336c471f45c5d483fb4b9a57ad27bb9ed1d10a5423cac9}"
UPSTREAM_VERSION="0~git20260129.${UPSTREAM_COMMIT:0:8}"
UPSTREAM_URL="https://github.com/lgblgblgb/xemu/archive/${UPSTREAM_COMMIT}.tar.gz"

cd "$(dirname "$0")"
PKG_DIR="$(pwd)"
NAME="x-emulators"
REPO_ROOT="$(cd ../.. && pwd)"
mkdir -p "$REPO_ROOT/dist"

if ! curl -fsI -o /dev/null https://github.com/; then
    echo "ERROR: cannot reach github.com — skipping $NAME build" >&2
    exit 1
fi

WORKDIR=$(mktemp -d -t "${NAME}-build-XXXXXX")
# shellcheck disable=SC2064  # expand $WORKDIR now so the trap captures the value
trap "rm -rf '$WORKDIR'" EXIT

if command -v apt-get >/dev/null; then
    _apt() { if [[ $EUID -eq 0 ]]; then apt-get "$@"; else sudo apt-get "$@"; fi; }
    _apt install -y --no-install-recommends \
        pkgconf libsdl2-dev libgtk-3-dev libreadline-dev zlib1g-dev
fi

echo "=== Fetching $UPSTREAM_URL ==="
ORIG_TARBALL="$WORKDIR/${NAME}_${UPSTREAM_VERSION}.orig.tar.gz"
curl -fsSL -o "$ORIG_TARBALL" "$UPSTREAM_URL"

echo "=== Verifying sha256 ==="
echo "$SHA256  $ORIG_TARBALL" | sha256sum -c -

echo "=== Extracting ==="
# GitHub's commit archive top-dir is xemu-<full-sha>, not <name>-<version>.
tar -xzf "$ORIG_TARBALL" -C "$WORKDIR"
SRC_DIR="$WORKDIR/${NAME}-${UPSTREAM_VERSION}"
mv "$WORKDIR/xemu-${UPSTREAM_COMMIT}" "$SRC_DIR"

cp -a "$PKG_DIR/debian" "$SRC_DIR/"

echo "=== Building ==="
( cd "$SRC_DIR" && dpkg-buildpackage -us -uc -b )

DEB_VERSION=$(sed -n '1s/.*(\(.*\)).*/\1/p' "$PKG_DIR/debian/changelog")
ARCH=$(dpkg --print-architecture)
DEB="$WORKDIR/${NAME}_${DEB_VERSION}_${ARCH}.deb"
[[ -f "$DEB" ]] || { echo "ERROR: expected $DEB not found" >&2; ls -la "$WORKDIR" >&2; exit 1; }
mv "$DEB" "$REPO_ROOT/dist/"
echo "=== Built $REPO_ROOT/dist/$(basename "$DEB") ==="

# Emit the source package (.dsc + tarballs) -> dist/ for the apt Sources index
# (best-effort; never blocks the binary build above). See scripts/lib-source-build.sh.
if [[ -f "$REPO_ROOT/scripts/lib-source-build.sh" ]]; then
    # shellcheck source=/dev/null
    . "$REPO_ROOT/scripts/lib-source-build.sh"
    emit_source_package "$SRC_DIR" "$WORKDIR" "$REPO_ROOT" || true
fi
