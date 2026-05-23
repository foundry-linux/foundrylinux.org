#!/usr/bin/env bash
# List ISOs in R2 and local dist/ — one line per edition+version set.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BOOTSTRAP_CACHE="$(cd "$SCRIPT_DIR/../.." && pwd)/.foundry/bootstrap.env"
[[ -f "$BOOTSTRAP_CACHE" ]] && source "$BOOTSTRAP_CACHE"

R2_ACCOUNT_ID="${R2_ACCOUNT_ID:-${CF_ACCOUNT_ID:-}}"
R2_ACCESS_KEY_ID="${R2_ACCESS_KEY_ID:-${ISO_R2_KEY_ID:-}}"
R2_SECRET_ACCESS_KEY="${R2_SECRET_ACCESS_KEY:-${ISO_R2_SECRET:-}}"
[[ -n "$R2_ACCOUNT_ID"        ]] || { echo "ERROR: R2_ACCOUNT_ID not set" >&2; exit 1; }
[[ -n "$R2_ACCESS_KEY_ID"     ]] || { echo "ERROR: R2_ACCESS_KEY_ID not set" >&2; exit 1; }
[[ -n "$R2_SECRET_ACCESS_KEY" ]] || { echo "ERROR: R2_SECRET_ACCESS_KEY not set" >&2; exit 1; }

export RCLONE_CONFIG_R2_TYPE=s3
export RCLONE_CONFIG_R2_PROVIDER=Cloudflare
export RCLONE_CONFIG_R2_ACCESS_KEY_ID="$R2_ACCESS_KEY_ID"
export RCLONE_CONFIG_R2_SECRET_ACCESS_KEY="$R2_SECRET_ACCESS_KEY"
export RCLONE_CONFIG_R2_ENDPOINT="https://${R2_ACCOUNT_ID}.r2.cloudflarestorage.com"
export RCLONE_CONFIG_R2_NO_CHECK_BUCKET=true

DIST_DIR="$SCRIPT_DIR/../dist"
BUCKET="foundry-iso"

_gb() { awk -v b="$1" 'BEGIN { printf "%.1fG", b/1073741824 }'; }

files=$(rclone lsl "r2:${BUCKET}/" 2>/dev/null | grep -v 'latest' | sort -k4 || true)

echo "=== R2: foundry-iso ==="
for edition in anvil atelier; do
  iso_line=$(echo "$files" | grep -E "foundry-${edition}-[0-9]+\.[0-9]+-amd64\.iso$" | head -1 || true)
  [[ -z "$iso_line" ]] && continue
  version=$(echo "$iso_line" | awk '{print $4}' | sed "s/foundry-${edition}-//;s/-amd64\.iso//")
  date=$(echo "$iso_line" | awk '{print $2}')
  line=$(printf "  %-15s  %s" "${edition}-${version}" "$date")
  for fmt in iso ova vmdk qcow2; do
    sz=$(echo "$files" | grep -E "foundry-${edition}-${version}-amd64\.${fmt}$" | awk '{print $1}' | head -1 || true)
    [[ -n "$sz" ]] && line+="  ${fmt}:$(_gb "$sz")"
  done
  echo "$line"
done

echo ""
echo "=== Local: dist/ ==="
any=0
for f in "$DIST_DIR"/foundry-*.{iso,ova,vmdk,qcow2}; do
  [[ -f "$f" ]] || continue
  any=1
  printf "  %-45s  %s\n" "$(basename "$f")" "$(du -h "$f" | cut -f1)"
done
if [[ $any -eq 0 ]]; then echo "  (empty)"; fi
