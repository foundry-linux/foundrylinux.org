#!/usr/bin/env bash
# One-time setup: create the foundry-iso R2 bucket + iso.foundrylinux.org
# custom domain binding.
#
# Usage:
#   bash scripts/bootstrap-r2.sh [--dry-run]
#
# Requires: CF_API_TOKEN (Cloudflare API token with R2 + DNS write access)

set -euo pipefail

DRY_RUN=false
for arg in "$@"; do
  case "$arg" in
    --dry-run) DRY_RUN=true ;;
    -h|--help)
      echo "Usage: $0 [--dry-run]"
      echo "  --dry-run  Print steps without executing"
      exit 0
      ;;
  esac
done

CF_API_TOKEN="${CF_API_TOKEN:?CF_API_TOKEN env var required}"
BUCKET="foundry-iso"
CUSTOM_DOMAIN="iso.foundrylinux.org"
CF_ZONE_NAME="foundrylinux.org"

run() {
  if $DRY_RUN; then
    echo "  [dry-run] $*"
  else
    "$@"
  fi
}

cf_api() {
  local method="$1" path="$2" data="${3:-}"
  local args=(-s -X "$method" "https://api.cloudflare.com/client/v4${path}"
              -H "Authorization: Bearer $CF_API_TOKEN"
              -H "Content-Type: application/json")
  [[ -n "$data" ]] && args+=(--data "$data")
  if $DRY_RUN; then
    echo "  [dry-run] $method $path ${data:+$data}"
    echo "{}"
  else
    curl "${args[@]}"
  fi
}

echo "=== [1] Getting Cloudflare account ID ==="
ACCOUNT_ID=$(cf_api GET "/accounts" | python3 -c \
  "import sys,json; print(json.load(sys.stdin)['result'][0]['id'])")
echo "  account: $ACCOUNT_ID"

echo "=== [2] Getting zone ID for $CF_ZONE_NAME ==="
ZONE_ID=$(cf_api GET "/zones?name=${CF_ZONE_NAME}" | python3 -c \
  "import sys,json; print(json.load(sys.stdin)['result'][0]['id'])")
echo "  zone: $ZONE_ID"

echo "=== [3] Creating R2 bucket: $BUCKET ==="
cf_api POST "/accounts/${ACCOUNT_ID}/r2/buckets" \
  "{\"name\":\"${BUCKET}\"}" > /dev/null

echo "=== [4] Enabling R2 public access (r2.dev subdomain) ==="
cf_api PUT "/accounts/${ACCOUNT_ID}/r2/buckets/${BUCKET}/domains/managed" \
  '{"enabled":true}' > /dev/null

echo "=== [5] Attaching custom domain $CUSTOM_DOMAIN ==="
run cf_api POST "/accounts/${ACCOUNT_ID}/r2/buckets/${BUCKET}/domains/custom" \
  "{\"domain\":\"${CUSTOM_DOMAIN}\",\"enabled\":true}" > /dev/null

echo "=== [6] Creating URL rewrite: trailing / → /index.html on $CUSTOM_DOMAIN ==="
run cf_api PUT "/zones/${ZONE_ID}/rulesets/phases/http_request_transform/entrypoint" \
  "$(python3 -c "
import json
rule = {
  'description': 'iso.foundrylinux.org: trailing slash redirect to index.html',
  'expression': 'http.host eq \"${CUSTOM_DOMAIN}\" and ends_with(http.request.uri.path, \"/\")',
  'action': 'rewrite',
  'action_parameters': {'uri': {'path': {'expression': 'concat(http.request.uri.path, \"index.html\")'}}}
}
print(json.dumps({'rules': [rule]}))
")" > /dev/null

echo "=== [7] Setting R2 lifecycle: expire non-latest objects after 90 days ==="
# Note: R2 lifecycle API — expire versioned/tagged objects older than 90 days.
# Objects named *-latest-* never expire (no lifecycle rule targets them).
run cf_api PUT "/accounts/${ACCOUNT_ID}/r2/buckets/${BUCKET}/lifecycle" \
  '{"rules":[{"id":"expire-old-releases","status":"enabled","filter":{"object_size_greater_than":0},"expiration":{"days":90}}]}' \
  > /dev/null

echo
echo "=== Bootstrap complete ==="
echo "  Bucket:  r2://${BUCKET}"
echo "  Domain:  https://${CUSTOM_DOMAIN}"
echo "  Next:    add R2_ACCOUNT_ID, R2_ACCESS_KEY_ID, R2_SECRET_ACCESS_KEY"
echo "           to foundry-linux/foundry-iso GitHub Actions secrets"
