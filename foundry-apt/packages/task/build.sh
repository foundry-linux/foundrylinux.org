#!/usr/bin/env bash
# Vendored-upstream Debian package build wrapper for task (go-task).
#
# task is a single static Go binary published as a per-OS/arch tarball on
# GitHub releases (task_linux_<arch>.tar.gz). This script:
#   1. Downloads the upstream tarball for the host arch (sha256-verified)
#   2. Extracts it into a versioned source tree
#   3. Copies our debian/ tree in
#   4. Runs dpkg-buildpackage -us -uc -b
#   5. Moves the resulting .deb into $REPO_ROOT/dist/
#
# Foundry vendors task so `Depends: task` (foundry-anvil/-core) resolves
# from apt.foundrylinux.org instead of go-task's third-party Cloudsmith
# repo, whose layout change (any-distro -> per-distro) silently 404'd
# installs. To bump: change UPSTREAM_VERSION + the SHA256_* lines (from the
# release's task_checksums.txt) and add a debian/changelog entry.

set -euo pipefail

for arg in "$@"; do
    case "$arg" in
        -h|--help) sed -n '2,16p' "$0"; exit 0 ;;
    esac
done

UPSTREAM_VERSION="${TASK_VERSION:-3.51.1}"
# sha256 of task_linux_<arch>.tar.gz for UPSTREAM_VERSION (task_checksums.txt)
SHA256_amd64="da7e92f0ff961ef2aae7cfecbad8d1fd2a08d7b09ba968673adf7ff389b243b5"
SHA256_arm64="49c58bb00eff2449a5553f3b3e694fc424e0dc04d5c669d8831126daee1000f8"

cd "$(dirname "$0")"
PKG_DIR="$(pwd)"
NAME="task"
REPO_ROOT="$(cd ../.. && pwd)"
mkdir -p "$REPO_ROOT/dist"

ARCH="$(dpkg --print-architecture)"
case "$ARCH" in
    amd64) SHA256="$SHA256_amd64" ;;
    arm64) SHA256="$SHA256_arm64" ;;
    *) echo "ERROR: unsupported arch '$ARCH' (task vendoring covers amd64/arm64)" >&2; exit 1 ;;
esac

UPSTREAM_URL="https://github.com/go-task/task/releases/download/v${UPSTREAM_VERSION}/task_linux_${ARCH}.tar.gz"

if ! curl -fsI -o /dev/null https://github.com/; then
    echo "ERROR: cannot reach github.com — skipping $NAME build" >&2
    exit 1
fi

WORKDIR=$(mktemp -d -t "${NAME}-build-XXXXXX")
# shellcheck disable=SC2064
trap "rm -rf '$WORKDIR'" EXIT

echo "=== Fetching $UPSTREAM_URL ==="
TGZ="$WORKDIR/${NAME}_${UPSTREAM_VERSION}.tar.gz"
curl -fsSL -o "$TGZ" "$UPSTREAM_URL"

echo "=== Verifying sha256 ($ARCH) ==="
echo "$SHA256  $TGZ" | sha256sum -c -

echo "=== Extracting ==="
SRC_DIR="$WORKDIR/${NAME}-${UPSTREAM_VERSION}"
mkdir "$SRC_DIR"
tar -xzf "$TGZ" -C "$SRC_DIR"

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

DEB_VERSION=$(sed -n '1s/.*(\(.*\)).*/\1/p' "$PKG_DIR/debian/changelog")
DEB="$WORKDIR/${NAME}_${DEB_VERSION}_${ARCH}.deb"
[[ -f "$DEB" ]] || { echo "ERROR: expected .deb not found: $DEB" >&2; ls -la "$WORKDIR"; exit 1; }

mv "$DEB" "$REPO_ROOT/dist/"
OUT="$REPO_ROOT/dist/$(basename "$DEB")"
echo "OK   $OUT  ($(stat -c%s "$OUT") bytes)"
