#!/usr/bin/env bash
# Upload VM images (qcow2, VMDK, OVA) to Cloudflare R2 (foundry-iso bucket).
#
# Credentials are loaded from .foundry/bootstrap.env (written by bootstrap-r2.sh)
# or from env vars:
#   R2_ACCOUNT_ID        Cloudflare account ID  (or CF_ACCOUNT_ID)
#   R2_ACCESS_KEY_ID     R2 S3-compat access key (or ISO_R2_KEY_ID)
#   R2_SECRET_ACCESS_KEY R2 S3-compat secret key (or ISO_R2_SECRET)

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BOOTSTRAP_CACHE="$(cd "$SCRIPT_DIR/../.." && pwd)/.foundry/bootstrap.env"

if [[ -f "$BOOTSTRAP_CACHE" ]]; then
  # shellcheck source=/dev/null
  source "$BOOTSTRAP_CACHE"
fi

R2_ACCOUNT_ID="${R2_ACCOUNT_ID:-${CF_ACCOUNT_ID:-}}"
R2_ACCESS_KEY_ID="${R2_ACCESS_KEY_ID:-${ISO_R2_KEY_ID:-}}"
R2_SECRET_ACCESS_KEY="${R2_SECRET_ACCESS_KEY:-${ISO_R2_SECRET:-}}"

[[ -n "$R2_ACCOUNT_ID"        ]] || { echo "ERROR: R2_ACCOUNT_ID not set — run bootstrap-r2.sh first" >&2; exit 1; }
[[ -n "$R2_ACCESS_KEY_ID"     ]] || { echo "ERROR: R2_ACCESS_KEY_ID not set — run bootstrap-r2.sh first" >&2; exit 1; }
[[ -n "$R2_SECRET_ACCESS_KEY" ]] || { echo "ERROR: R2_SECRET_ACCESS_KEY not set — run bootstrap-r2.sh first" >&2; exit 1; }

DIST_DIR="$SCRIPT_DIR/../dist"
ISO_VERSION="$(cat "$SCRIPT_DIR/../VERSION")"
IMAGE_BASE="foundry-anvil-${ISO_VERSION}-amd64"
BUCKET="foundry-iso"
ENDPOINT="https://${R2_ACCOUNT_ID}.r2.cloudflarestorage.com"

for ext in qcow2 vmdk ova; do
  f="$DIST_DIR/${IMAGE_BASE}.${ext}"
  if [[ ! -f "$f" ]]; then
    echo "ERROR: $f not found — run build-vm-images.sh first" >&2
    exit 1
  fi
done

if ! command -v rclone &>/dev/null; then
  echo "=== Installing rclone to ~/.local/bin ==="
  mkdir -p "$HOME/.local/bin"
  RCLONE_TMP=$(mktemp -d)
  curl -fsSL "https://downloads.rclone.org/rclone-current-linux-amd64.zip" -o "$RCLONE_TMP/rclone.zip"
  unzip -q "$RCLONE_TMP/rclone.zip" -d "$RCLONE_TMP"
  cp "$RCLONE_TMP"/rclone-*/rclone "$HOME/.local/bin/rclone"
  chmod +x "$HOME/.local/bin/rclone"
  rm -rf "$RCLONE_TMP"
  export PATH="$HOME/.local/bin:$PATH"
fi

export RCLONE_CONFIG_R2_TYPE=s3
export RCLONE_CONFIG_R2_PROVIDER=Cloudflare
export RCLONE_CONFIG_R2_ACCESS_KEY_ID="$R2_ACCESS_KEY_ID"
export RCLONE_CONFIG_R2_SECRET_ACCESS_KEY="$R2_SECRET_ACCESS_KEY"
export RCLONE_CONFIG_R2_ENDPOINT="$ENDPOINT"
export RCLONE_CONFIG_R2_ACL=public-read
export RCLONE_CONFIG_R2_NO_CHECK_BUCKET=true

for ext in qcow2 vmdk ova; do
  VERSIONED="${IMAGE_BASE}.${ext}"
  LATEST="foundry-anvil-latest-amd64.${ext}"

  echo "=== Uploading $VERSIONED ==="
  rclone copyto "$DIST_DIR/${VERSIONED}"        "r2:${BUCKET}/${VERSIONED}"        --progress
  rclone copyto "$DIST_DIR/${VERSIONED}.sha256" "r2:${BUCKET}/${VERSIONED}.sha256" --progress

  echo "=== Updating latest pointer: $LATEST ==="
  rclone copyto "$DIST_DIR/${VERSIONED}"        "r2:${BUCKET}/${LATEST}"           --progress
  rclone copyto "$DIST_DIR/${VERSIONED}.sha256" "r2:${BUCKET}/${LATEST}.sha256"    --progress
done

echo "=== VM upload complete ==="
echo "  qcow2: https://iso.foundrylinux.org/foundry-anvil-latest-amd64.qcow2"
echo "  vmdk:  https://iso.foundrylinux.org/foundry-anvil-latest-amd64.vmdk"
echo "  ova:   https://iso.foundrylinux.org/foundry-anvil-latest-amd64.ova"
