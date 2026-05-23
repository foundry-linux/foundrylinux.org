#!/usr/bin/env bash
# Build VM images (qcow2, VMDK, OVA) from a Foundry Linux ISO.
# Only EDITION=anvil is supported (Atelier at 10+ GB makes impractical VM images).
#
# Usage:
#   bash scripts/build-vm-images.sh
#   EDITION=anvil bash scripts/build-vm-images.sh    # same thing
#
# Output (in dist/):
#   foundry-anvil-1.0-amd64.qcow2
#   foundry-anvil-1.0-amd64.vmdk
#   foundry-anvil-1.0-amd64.ova
#   ...plus .sha256 checksums for each

set -euo pipefail

EDITION="${EDITION:-anvil}"
if [[ "$EDITION" != "anvil" ]]; then
  echo "ERROR: VM images are only built for the anvil edition (not $EDITION)" >&2
  exit 1
fi

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
DIST_DIR="$REPO_ROOT/dist"
ISO_VERSION="$(cat "$SCRIPT_DIR/../VERSION")"
ISO="$DIST_DIR/foundry-${EDITION}-${ISO_VERSION}-amd64.iso"
IMAGE_BASE="foundry-${EDITION}-${ISO_VERSION}-amd64"

if [[ ! -f "$ISO" ]]; then
  echo "ERROR: $ISO not found — run build-iso.sh first" >&2
  exit 1
fi

echo "=== Building VM images from $ISO ==="

docker run --rm \
  --privileged \
  -e IMAGE_BASE="$IMAGE_BASE" \
  -v "$REPO_ROOT:/work" \
  -v /lib/modules:/lib/modules:ro \
  -w /work \
  ubuntu:26.04 \
  bash /work/scripts/vm-images-inner.sh

echo "=== Done: dist/ ==="
ls -lh "$DIST_DIR/foundry-${EDITION}-${ISO_VERSION}-amd64".{qcow2,vmdk,ova} 2>/dev/null || true
