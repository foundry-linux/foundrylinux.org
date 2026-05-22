#!/usr/bin/env bash
# Build a Foundry Linux ISO using live-build inside an ubuntu:26.04 container.
#
# Usage:
#   EDITION=anvil bash scripts/build-iso.sh
#   EDITION=atelier bash scripts/build-iso.sh
#
# Output: dist/foundry-<edition>-1.0-amd64.iso

set -euo pipefail

EDITION="${EDITION:?EDITION env var required: anvil or atelier}"
case "$EDITION" in
  anvil|atelier) ;;
  *) echo "EDITION must be one of: anvil, atelier" >&2; exit 1 ;;
esac

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
DIST_DIR="$REPO_ROOT/dist"

mkdir -p "$DIST_DIR"

echo "=== Fetching apt signing keys ==="
curl -fsSL https://apt.foundrylinux.org/key.gpg \
  | gpg --dearmor > "$REPO_ROOT/config/archives/foundry.key.chroot"
curl -fsSL https://apt.worldfoundry.org/key.gpg \
  | gpg --dearmor > "$REPO_ROOT/config/archives/worldfoundry.key.chroot"

echo "=== Building foundry-${EDITION} ISO (inside ubuntu:26.04 container) ==="
docker run --rm \
  --privileged \
  -e EDITION="$EDITION" \
  -v "$REPO_ROOT:/work" \
  -w /work \
  ubuntu:26.04 \
  bash -c '
    set -euo pipefail
    apt-get update -qq
    apt-get install -y --no-install-recommends \
      live-build curl gpg apt-utils ca-certificates \
      xorriso isolinux grub-efi-amd64-bin grub-pc-bin mtools dosfstools
    lb clean --purge 2>/dev/null || true
    bash config/auto/config
    lb build
  '

# live-build writes the ISO to the working directory as live-image-amd64.hybrid.iso
ISO_SRC="$REPO_ROOT/live-image-amd64.hybrid.iso"
ISO_DST="$DIST_DIR/foundry-${EDITION}-1.0-amd64.iso"

if [[ -f "$ISO_SRC" ]]; then
  mv "$ISO_SRC" "$ISO_DST"
  echo "=== ISO ready: $ISO_DST ($(du -h "$ISO_DST" | cut -f1)) ==="
else
  echo "ERROR: expected $ISO_SRC not found after lb build" >&2
  exit 1
fi
