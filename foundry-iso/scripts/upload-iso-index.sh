#!/usr/bin/env bash
# Upload dist/index.html (and any companion static assets) to the
# foundry-iso R2 bucket so iso.foundrylinux.org/ serves the index page.
#
# Required env vars:
#   R2_ACCOUNT_ID        Cloudflare account ID
#   R2_ACCESS_KEY_ID     R2 S3-compat access key
#   R2_SECRET_ACCESS_KEY R2 S3-compat secret key

set -euo pipefail

R2_ACCOUNT_ID="${R2_ACCOUNT_ID:?R2_ACCOUNT_ID required}"
R2_ACCESS_KEY_ID="${R2_ACCESS_KEY_ID:?R2_ACCESS_KEY_ID required}"
R2_SECRET_ACCESS_KEY="${R2_SECRET_ACCESS_KEY:?R2_SECRET_ACCESS_KEY required}"

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
DIST_DIR="$SCRIPT_DIR/../dist"
BUCKET="foundry-iso"
ENDPOINT="https://${R2_ACCOUNT_ID}.r2.cloudflarestorage.com"

if [[ ! -f "$DIST_DIR/index.html" ]]; then
  echo "ERROR: $DIST_DIR/index.html not found — run generate-iso-index.sh first" >&2
  exit 1
fi

if ! command -v rclone &>/dev/null; then
  echo "=== Installing rclone ==="
  curl -fsSL https://rclone.org/install.sh | bash
fi

export RCLONE_CONFIG_R2_TYPE=s3
export RCLONE_CONFIG_R2_PROVIDER=Cloudflare
export RCLONE_CONFIG_R2_ACCESS_KEY_ID="$R2_ACCESS_KEY_ID"
export RCLONE_CONFIG_R2_SECRET_ACCESS_KEY="$R2_SECRET_ACCESS_KEY"
export RCLONE_CONFIG_R2_ENDPOINT="$ENDPOINT"
export RCLONE_CONFIG_R2_ACL=public-read

echo "=== Uploading index.html ==="
rclone copyto "$DIST_DIR/index.html" "r2:${BUCKET}/index.html" \
  --header-upload "Content-Type: text/html; charset=utf-8" \
  --progress

echo "=== Upload complete: https://iso.foundrylinux.org/ ==="
