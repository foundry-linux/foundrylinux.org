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
REPO_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
BUCKET="foundry-iso"

_gb() { awk -v b="$1" 'BEGIN { if (b>=1073741824) printf "%.1fG", b/1073741824; else if (b>=1048576) printf "%.0fM", b/1048576; else printf "%dB", b }'; }

files=$(rclone lsl "r2:${BUCKET}/" 2>/dev/null | grep -v 'latest' | sort -k4 || true)

# ── R2 table ─────────────────────────────────────────────────────────────────
printf '\n┌───────────────────────────────────────────────────────────────────────┐\n'
printf '│ %-69s │\n' "r2: ${BUCKET}/"
printf '├─────────────────┬──────────────────┬───────┬────────┬────────┬────────┤\n'
printf '│ %-15s │ %-16s │ %-5s │ %-6s │ %-6s │ %-6s │\n' "edition" "timestamp" "iso" "ova" "vmdk" "qcow2"
printf '├─────────────────┼──────────────────┼───────┼────────┼────────┼────────┤\n'

any_r2=0
for edition in anvil atelier; do
  iso_line=$(echo "$files" | grep -E "foundry-${edition}-[0-9]+\.[0-9]+-amd64\.iso$" | head -1 || true)
  [[ -z "$iso_line" ]] && continue
  any_r2=1
  version=$(echo "$iso_line" | awk '{print $4}' | sed "s/foundry-${edition}-//;s/-amd64\.iso//")
  ts="${iso_line}" ; ts="$(echo "$ts" | awk '{print $2, substr($3,1,5)}')"
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
  printf '│ %-15s │ %-16s │ %-5s │ %-6s │ %-6s │ %-6s │\n' \
    "${edition}-${version}" "$ts" "$iso_sz" "$ova_sz" "$vmdk_sz" "$qcow2_sz"
done
if [[ $any_r2 -eq 0 ]]; then
  printf '│ %-69s │\n' "(no versioned ISOs found)"
fi
printf '└─────────────────┴──────────────────┴───────┴────────┴────────┴────────┘\n'

# ── Local table ───────────────────────────────────────────────────────────────
DIST_LABEL="$(realpath --relative-to="$REPO_ROOT" "$DIST_DIR")/"
printf '\n┌───────────────────────────────────────────────────────────────────────┐\n'
printf '│ %-69s │\n' "local: ${DIST_LABEL}"
printf '├─────────────────┬──────────────────┬───────┬────────┬────────┬────────┤\n'
printf '│ %-15s │ %-16s │ %-5s │ %-6s │ %-6s │ %-6s │\n' "edition" "timestamp" "iso" "ova" "vmdk" "qcow2"
printf '├─────────────────┼──────────────────┼───────┼────────┼────────┼────────┤\n'

any_local=0
for edition in anvil atelier; do
  iso_f=$(ls "$DIST_DIR"/foundry-${edition}-*-amd64.iso 2>/dev/null | sort | tail -1 || true)
  [[ -z "$iso_f" || ! -f "$iso_f" ]] && continue
  any_local=1
  version=$(basename "$iso_f" | sed "s/foundry-${edition}-//;s/-amd64\.iso//")
  ts=$(stat -c '%y' "$iso_f" | cut -c1-16)
  iso_sz=$(du -h "$iso_f" | cut -f1)
  ova_sz="-"; vmdk_sz="-"; qcow2_sz="-"
  for fmt in ova vmdk qcow2; do
    vm_f="$DIST_DIR/foundry-${edition}-${version}-amd64.${fmt}"
    [[ -f "$vm_f" ]] && eval "${fmt}_sz=\$(du -h \"\$vm_f\" | cut -f1)"
  done
  printf '│ %-15s │ %-16s │ %-5s │ %-6s │ %-6s │ %-6s │\n' \
    "${edition}-${version}" "$ts" "$iso_sz" "$ova_sz" "$vmdk_sz" "$qcow2_sz"
done
if [[ $any_local -eq 0 ]]; then
  printf '│ %-69s │\n' "(no local builds)"
fi
printf '└─────────────────┴──────────────────┴───────┴────────┴────────┴────────┘\n'
