#!/usr/bin/env bash
# Upload a signed ISO + checksums to Cloudflare R2 (foundry-iso bucket).
#
# Required env vars:
#   EDITION              anvil | atelier
#   R2_ACCOUNT_ID        Cloudflare account ID
#   R2_ACCESS_KEY_ID     R2 S3-compat access key
#   R2_SECRET_ACCESS_KEY R2 S3-compat secret key

set -euo pipefail

EDITION="${EDITION:?EDITION env var required}"
R2_ACCOUNT_ID="${R2_ACCOUNT_ID:?R2_ACCOUNT_ID required}"
R2_ACCESS_KEY_ID="${R2_ACCESS_KEY_ID:?R2_ACCESS_KEY_ID required}"
R2_SECRET_ACCESS_KEY="${R2_SECRET_ACCESS_KEY:?R2_SECRET_ACCESS_KEY required}"

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
DIST_DIR="$SCRIPT_DIR/../dist"
ISO="$DIST_DIR/foundry-${EDITION}-1.0-amd64.iso"
BUCKET="foundry-iso"
ENDPOINT="https://${R2_ACCOUNT_ID}.r2.cloudflarestorage.com"

if [[ ! -f "$ISO" ]]; then
  echo "ERROR: $ISO not found" >&2
  exit 1
fi

if ! command -v rclone &>/dev/null; then
  echo "=== Installing rclone ==="
  curl -fsSL https://rclone.org/install.sh | bash
fi

# Configure rclone R2 remote on the fly (no config file needed).
export RCLONE_CONFIG_R2_TYPE=s3
export RCLONE_CONFIG_R2_PROVIDER=Cloudflare
export RCLONE_CONFIG_R2_ACCESS_KEY_ID="$R2_ACCESS_KEY_ID"
export RCLONE_CONFIG_R2_SECRET_ACCESS_KEY="$R2_SECRET_ACCESS_KEY"
export RCLONE_CONFIG_R2_ENDPOINT="$ENDPOINT"
export RCLONE_CONFIG_R2_ACL=public-read
export RCLONE_CONFIG_R2_NO_CHECK_BUCKET=true

VERSIONED="foundry-${EDITION}-1.0-amd64.iso"
LATEST="foundry-${EDITION}-latest-amd64.iso"

echo "=== Uploading $VERSIONED ==="
rclone copyto "$ISO"           "r2:${BUCKET}/${VERSIONED}"           --progress
rclone copyto "${ISO}.sha256"  "r2:${BUCKET}/${VERSIONED}.sha256"    --progress
rclone copyto "${ISO}.asc"     "r2:${BUCKET}/${VERSIONED}.asc"       --progress
rclone copyto "$DIST_DIR/manifest-${EDITION}.json" \
              "r2:${BUCKET}/manifest-${EDITION}.json" --progress

echo "=== Updating latest pointer ==="
rclone copyto "$ISO"           "r2:${BUCKET}/${LATEST}"              --progress
rclone copyto "${ISO}.sha256"  "r2:${BUCKET}/${LATEST}.sha256"       --progress
rclone copyto "${ISO}.asc"     "r2:${BUCKET}/${LATEST}.asc"          --progress

if [[ -f "${ISO}.torrent" ]]; then
  echo "=== Uploading torrent ==="
  rclone copyto "${ISO}.torrent" "r2:${BUCKET}/${VERSIONED}.torrent" --progress
  rclone copyto "${ISO}.torrent" "r2:${BUCKET}/${LATEST}.torrent"    --progress
fi

echo "=== Upload complete ==="
echo "  https://iso.foundrylinux.org/${LATEST}"
