#!/usr/bin/env bash
# List ISOs in the R2 bucket and in local dist/.
#
# Usage:
#   bash scripts/list-isos.sh

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BOOTSTRAP_CACHE="$(cd "$SCRIPT_DIR/../.." && pwd)/.foundry/bootstrap.env"
[[ -f "$BOOTSTRAP_CACHE" ]] && source "$BOOTSTRAP_CACHE"

R2_ACCOUNT_ID="${R2_ACCOUNT_ID:-${CF_ACCOUNT_ID:-}}"
R2_ACCESS_KEY_ID="${R2_ACCESS_KEY_ID:-${ISO_R2_KEY_ID:-}}"
R2_SECRET_ACCESS_KEY="${R2_SECRET_ACCESS_KEY:-${ISO_R2_SECRET:-}}"
[[ -n "$R2_ACCOUNT_ID"        ]] || { echo "ERROR: R2_ACCOUNT_ID not set — run bootstrap-r2.sh first" >&2; exit 1; }
[[ -n "$R2_ACCESS_KEY_ID"     ]] || { echo "ERROR: R2_ACCESS_KEY_ID not set — run bootstrap-r2.sh first" >&2; exit 1; }
[[ -n "$R2_SECRET_ACCESS_KEY" ]] || { echo "ERROR: R2_SECRET_ACCESS_KEY not set — run bootstrap-r2.sh first" >&2; exit 1; }

export RCLONE_CONFIG_R2_TYPE=s3
export RCLONE_CONFIG_R2_PROVIDER=Cloudflare
export RCLONE_CONFIG_R2_ACCESS_KEY_ID="$R2_ACCESS_KEY_ID"
export RCLONE_CONFIG_R2_SECRET_ACCESS_KEY="$R2_SECRET_ACCESS_KEY"
export RCLONE_CONFIG_R2_ENDPOINT="https://${R2_ACCOUNT_ID}.r2.cloudflarestorage.com"
export RCLONE_CONFIG_R2_NO_CHECK_BUCKET=true

DIST_DIR="$SCRIPT_DIR/../dist"

echo "=== R2: foundry-iso ==="
rclone lsl r2:foundry-iso/ | sort -k4 | awk '
{
  size = $1
  date = $2
  time = substr($3, 1, 8)
  name = $4
  if (size >= 1073741824)
    printf "  %s  %s  %5.1f GB  %s\n", date, time, size/1073741824, name
  else if (size >= 1048576)
    printf "  %s  %s  %5.1f MB  %s\n", date, time, size/1048576, name
  else
    printf "  %s  %s  %5d  B  %s\n", date, time, size, name
}'

echo ""
echo "=== Local: dist/ ==="
if ls "$DIST_DIR"/ &>/dev/null; then
  ls -lh "$DIST_DIR"/ | tail -n +2
else
  echo "  (empty)"
fi
