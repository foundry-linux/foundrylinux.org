#!/usr/bin/env bash
# Restore GitHub Actions secrets for foundry-linux/foundrylinux.org from the
# foundry-secrets R2 backup bucket.
#
# Run once after the monorepo migration — the original bootstrap set secrets
# on foundry-linux/foundry-apt; this script migrates them to the monorepo.
#
# Requires: CF_API_TOKEN in env or .foundry/bootstrap.env, gh CLI authenticated.
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
CACHE_FILE="$REPO_ROOT/.foundry/bootstrap.env"

if [[ "${1:-}" == "-h" || "${1:-}" == "--help" ]]; then
    echo "Usage: restore-gh-secrets.sh"
    echo "Reads GPG_PRIVATE_KEY + R2 creds from the foundry-secrets R2 bucket"
    echo "and sets them as GitHub Actions secrets on foundry-linux/foundrylinux.org."
    exit 0
fi

GH_REPO="foundry-linux/foundrylinux.org"
CF_ACCOUNT_ID="f7e1e6cd8b3414a6d2226152533c21f2"
SECRETS_BUCKET="foundry-linux-secrets"

# Load CF_API_TOKEN from cache if not already set.
if [[ -z "${CF_API_TOKEN:-}" && -f "$CACHE_FILE" ]]; then
    CF_API_TOKEN="$(grep '^CF_API_TOKEN=' "$CACHE_FILE" | cut -d= -f2-)"
fi
: "${CF_API_TOKEN:?CF_API_TOKEN is required (set it or run bootstrap.sh first)}"

r2_get_secret() {
    local name="$1"
    curl -fsSL \
        "https://api.cloudflare.com/client/v4/accounts/${CF_ACCOUNT_ID}/r2/buckets/${SECRETS_BUCKET}/objects/${name}" \
        -H "Authorization: Bearer ${CF_API_TOKEN}"
}

echo "=== Fetching secrets from R2 foundry-secrets bucket ==="
GPG_PRIVATE_KEY="$(r2_get_secret GPG_PRIVATE_KEY)"
R2_ACCESS_KEY_ID="$(r2_get_secret R2_ACCESS_KEY_ID)"
R2_SECRET_ACCESS_KEY="$(r2_get_secret R2_SECRET_ACCESS_KEY)"

# R2_ENDPOINT is deterministic: https://<account-id>.r2.cloudflarestorage.com
R2_ENDPOINT="https://${CF_ACCOUNT_ID}.r2.cloudflarestorage.com"

echo "=== Setting GitHub Actions secrets on ${GH_REPO} ==="
gh secret set GPG_PRIVATE_KEY      --repo "$GH_REPO" --body "$GPG_PRIVATE_KEY"
echo "  ✓ GPG_PRIVATE_KEY"
gh secret set R2_ACCESS_KEY_ID     --repo "$GH_REPO" --body "$R2_ACCESS_KEY_ID"
echo "  ✓ R2_ACCESS_KEY_ID"
gh secret set R2_SECRET_ACCESS_KEY --repo "$GH_REPO" --body "$R2_SECRET_ACCESS_KEY"
echo "  ✓ R2_SECRET_ACCESS_KEY"
gh secret set R2_ENDPOINT          --repo "$GH_REPO" --body "$R2_ENDPOINT"
echo "  ✓ R2_ENDPOINT"

echo "=== Done. Re-trigger the publish workflow: ==="
echo "  gh workflow run foundry-apt-publish.yml --repo ${GH_REPO} --ref main"
