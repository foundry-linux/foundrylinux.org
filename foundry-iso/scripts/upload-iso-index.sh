#!/usr/bin/env bash
# Upload dist/index.html (and any companion static assets) to the
# foundry-iso R2 bucket so iso.foundrylinux.org/ serves the index page.
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

# Map bootstrap variable names → canonical upload names
R2_ACCOUNT_ID="${R2_ACCOUNT_ID:-${CF_ACCOUNT_ID:-}}"
R2_ACCESS_KEY_ID="${R2_ACCESS_KEY_ID:-${ISO_R2_KEY_ID:-}}"
R2_SECRET_ACCESS_KEY="${R2_SECRET_ACCESS_KEY:-${ISO_R2_SECRET:-}}"

[[ -n "$R2_ACCOUNT_ID"        ]] || { echo "ERROR: R2_ACCOUNT_ID / CF_ACCOUNT_ID not set — run bootstrap-r2.sh first" >&2; exit 1; }
[[ -n "$R2_ACCESS_KEY_ID"     ]] || { echo "ERROR: R2_ACCESS_KEY_ID / ISO_R2_KEY_ID not set — run bootstrap-r2.sh first" >&2; exit 1; }
[[ -n "$R2_SECRET_ACCESS_KEY" ]] || { echo "ERROR: R2_SECRET_ACCESS_KEY / ISO_R2_SECRET not set — run bootstrap-r2.sh first" >&2; exit 1; }

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
DIST_DIR="$SCRIPT_DIR/../dist"
BUCKET="foundry-iso"
ENDPOINT="https://${R2_ACCOUNT_ID}.r2.cloudflarestorage.com"

if [[ ! -f "$DIST_DIR/index.html" ]]; then
  echo "ERROR: $DIST_DIR/index.html not found — run generate-iso-index.sh first" >&2
  exit 1
fi

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

echo "=== Uploading index.html ==="
rclone copyto "$DIST_DIR/index.html" "r2:${BUCKET}/index.html" \
  --s3-no-check-bucket \
  --header-upload "Content-Type: text/html; charset=utf-8" \
  --progress

echo "=== Upload complete: https://iso.foundrylinux.org/ ==="
