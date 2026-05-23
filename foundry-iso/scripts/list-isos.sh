#!/usr/bin/env bash
# List ISOs in R2 and local dist/ with box-table output.

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

_gb() { awk -v b="$1" 'BEGIN { if (b>=1073741824) printf "%.1fG", b/1073741824; else if (b>=1048576) printf "%.0fM", b/1048576; else printf "%dB", b }'; }

files=$(rclone lsl "r2:${BUCKET}/" 2>/dev/null | grep -v 'latest' | sort -k4 || true)

# ── R2 table ─────────────────────────────────────────────────────────────────
printf '\n┌─────────────────┬────────────┬───────┬────────┬────────┬────────┐\n'
printf '│ %-15s │ %-10s │ %-5s │ %-6s │ %-6s │ %-6s │\n' "edition" "date" "iso" "ova" "vmdk" "qcow2"
printf '├─────────────────┼────────────┼───────┼────────┼────────┼────────┤\n'

any_r2=0
for edition in anvil atelier; do
  iso_line=$(echo "$files" | grep -E "foundry-${edition}-[0-9]+\.[0-9]+-amd64\.iso$" | head -1 || true)
  [[ -z "$iso_line" ]] && continue
  any_r2=1
  version=$(echo "$iso_line" | awk '{print $4}' | sed "s/foundry-${edition}-//;s/-amd64\.iso//")
  date=$(echo "$iso_line"  | awk '{print $2}')
  iso_bytes=$(echo "$iso_line" | awk '{print $1}')
  iso_sz=$(_gb "$iso_bytes")
  ova_sz=""; vmdk_sz=""; qcow2_sz=""
  # ISOs over 8 GB are too large for practical VM images
  if (( iso_bytes > 8589934592 )); then
    ova_sz="-"; vmdk_sz="-"; qcow2_sz="-"
  else
    for fmt in ova vmdk qcow2; do
      b=$(echo "$files" | grep -E "foundry-${edition}-${version}-amd64\.${fmt}$" | awk '{print $1}' | head -1 || true)
      [[ -n "$b" ]] && eval "${fmt}_sz=\$(_gb \"\$b\")"
    done
  fi
  printf '│ %-15s │ %-10s │ %-5s │ %-6s │ %-6s │ %-6s │\n' \
    "${edition}-${version}" "$date" "$iso_sz" "$ova_sz" "$vmdk_sz" "$qcow2_sz"
done
if [[ $any_r2 -eq 0 ]]; then
  printf '│ %-63s │\n' "(no versioned ISOs found)"
fi
printf '└─────────────────┴────────────┴───────┴────────┴────────┴────────┘\n'

# ── Local table ───────────────────────────────────────────────────────────────
printf '\n┌──────────────────────────────────────────────┬──────┐\n'
printf '│ %-44s │ %-4s │\n' "local: dist/" "size"
printf '├──────────────────────────────────────────────┼──────┤\n'

any_local=0
for f in "$DIST_DIR"/foundry-*.{iso,ova,vmdk,qcow2}; do
  [[ -f "$f" ]] || continue
  any_local=1
  printf '│ %-44s │ %-4s │\n' "$(basename "$f")" "$(du -h "$f" | cut -f1)"
done
if [[ $any_local -eq 0 ]]; then
  printf '│ %-44s │ %-4s │\n' "(empty)" ""
fi
printf '└──────────────────────────────────────────────┴──────┘\n'
