#!/usr/bin/env bash
# Prune versioned ISOs from R2 (never touches 'latest' pointers).
#
# Usage:
#   KEEP=2 bash scripts/prune-isos.sh              # keep 2 most recent per edition
#   BEFORE=2026-05-01 bash scripts/prune-isos.sh   # delete ISOs uploaded before date
#
# Dry-run by default. Set DRY_RUN=0 to actually delete.
#
# Sidecars (.sha256, .asc, .torrent) for each deleted ISO are also removed.

set -euo pipefail

KEEP="${KEEP:-}"
BEFORE="${BEFORE:-}"
DRY_RUN="${DRY_RUN:-1}"

if [[ -z "$KEEP" && -z "$BEFORE" ]]; then
  echo "ERROR: set KEEP=N or BEFORE=YYYY-MM-DD" >&2
  echo "  KEEP=2 bash scripts/prune-isos.sh" >&2
  echo "  BEFORE=2026-05-01 bash scripts/prune-isos.sh" >&2
  exit 1
fi

if [[ -n "$KEEP" && -n "$BEFORE" ]]; then
  echo "ERROR: set only one of KEEP or BEFORE, not both" >&2
  exit 1
fi

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

BUCKET="foundry-iso"

# List all objects, extract versioned ISOs (not 'latest'):
#   foundry-anvil-1.0-amd64.iso  ← versioned (keep/prune candidate)
#   foundry-anvil-latest-amd64.iso  ← latest pointer (never touch)
# rclone lsl output: <size> <date> <time> <name>
versioned_isos=$(rclone lsl "r2:${BUCKET}/" \
  | awk '{print $2, $4}' \
  | grep -E 'foundry-[a-z]+-[0-9]+\.[0-9]+-amd64\.iso$' \
  | sort || true)

if [[ -z "$versioned_isos" ]]; then
  echo "No versioned ISOs found in R2."
  exit 0
fi

# Build list of (date, basename) to prune
to_prune=()

if [[ -n "$KEEP" ]]; then
  # Group by edition, keep N most recent per edition, prune the rest
  for edition in anvil atelier; do
    edition_isos=$(echo "$versioned_isos" \
      | grep "foundry-${edition}-" \
      | sort -r || true)  # newest first
    count=0
    while IFS= read -r line; do
      [[ -z "$line" ]] && continue
      count=$(( count + 1 ))
      if (( count > KEEP )); then
        name=$(echo "$line" | awk '{print $2}')
        base="${name%.iso}"
        to_prune+=("$base")
      fi
    done <<< "$edition_isos"
  done
else
  # BEFORE: prune ISOs uploaded before this date
  while IFS= read -r line; do
    [[ -z "$line" ]] && continue
    date=$(echo "$line" | awk '{print $1}')
    name=$(echo "$line" | awk '{print $2}')
    if [[ "$date" < "$BEFORE" ]]; then
      base="${name%.iso}"
      to_prune+=("$base")
    fi
  done <<< "$versioned_isos"
fi

if (( ${#to_prune[@]} == 0 )); then
  echo "Nothing to prune."
  exit 0
fi

if [[ "$DRY_RUN" == "1" ]]; then
  echo "=== DRY RUN (set DRY_RUN=0 to actually delete) ==="
fi

for base in "${to_prune[@]}"; do
  echo "Pruning: ${base}.*"
  # Delete the .iso and all known sidecars
  for suffix in iso iso.sha256 iso.asc iso.torrent; do
    file="${base}.${suffix}"
    if rclone lsf "r2:${BUCKET}/${file}" &>/dev/null 2>&1; then
      if [[ "$DRY_RUN" == "1" ]]; then
        echo "  [dry-run] would delete: ${file}"
      else
        echo "  deleting: ${file}"
        rclone deletefile "r2:${BUCKET}/${file}"
      fi
    fi
  done
done

if [[ "$DRY_RUN" == "1" ]]; then
  echo ""
  echo "Re-run with DRY_RUN=0 to apply."
fi
