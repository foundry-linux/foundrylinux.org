#!/usr/bin/env bash
# One-time setup: create foundry-iso R2 bucket, attach iso.foundrylinux.org,
# fetch the shared GPG signing key from foundry-secrets, and wire all
# GitHub Actions secrets on foundry-linux/foundry-iso.
#
# Usage:
#   bash scripts/bootstrap-r2.sh [--dry-run]
#
# If CF_API_TOKEN is not already exported, the script prompts you to roll
# the 'foundry-linux-operator' Cloudflare token and paste the new value.
# Credentials are cached in .foundry/bootstrap.env (project root, gitignored).

set -euo pipefail

REPO_ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
BOOTSTRAP_CACHE="${REPO_ROOT}/.foundry/bootstrap.env"
mkdir -p "${REPO_ROOT}/.foundry"
GH_REPO="foundry-linux/foundry-iso"
BUCKET="foundry-iso"
SECRETS_BUCKET="foundry-secrets"
R2_TOKEN_NAME="foundry-iso-ci"
CUSTOM_DOMAIN="iso.foundrylinux.org"
DNS_CNAME="iso"
CF_ZONE_NAME="foundrylinux.org"

DRY_RUN=false
for arg in "$@"; do
  case "$arg" in
    --dry-run) DRY_RUN=true ;;
    -h|--help)
      sed -n '2,11p' "$0" | sed 's/^# //'
      exit 0
      ;;
    *) echo "Unknown argument: $arg" >&2; exit 1 ;;
  esac
done

info() { echo "  [info]  $*"; }
ok()   { echo "  [ok]    $*"; }
err()  { echo "  [error] $*" >&2; }
die()  { err "$*"; exit 1; }

cache_set() {
  local key="$1" val="$2"
  { grep -v "^${key}=" "$BOOTSTRAP_CACHE" 2>/dev/null || true
    printf '%s=%q\n' "$key" "$val"
  } > "${BOOTSTRAP_CACHE}.tmp" && mv "${BOOTSTRAP_CACHE}.tmp" "$BOOTSTRAP_CACHE"
  chmod 600 "$BOOTSTRAP_CACHE"
}

cf_api() {
  local method="$1" path="$2"
  shift 2
  curl -fsSL -X "$method" \
    "https://api.cloudflare.com/client/v4${path}" \
    -H "Authorization: Bearer ${CF_API_TOKEN:-}" \
    -H "Content-Type: application/json" \
    "$@"
}

r2_get_secret() {
  local name="$1"
  curl -fsSL -X GET \
    "https://api.cloudflare.com/client/v4/accounts/${CF_ACCOUNT_ID}/r2/buckets/${SECRETS_BUCKET}/objects/${name}" \
    -H "Authorization: Bearer ${CF_API_TOKEN}"
}

# ── preflight ────────────────────────────────────────────────────────────────

command -v curl &>/dev/null || die "curl not found"
command -v jq   &>/dev/null || die "jq not found"
command -v gh   &>/dev/null || die "gh CLI not found"

if ! $DRY_RUN; then
  gh auth status &>/dev/null || die "gh not authenticated — run: gh auth login"
fi

# ── load cache ───────────────────────────────────────────────────────────────

if [[ -f "$BOOTSTRAP_CACHE" ]]; then
  # shellcheck source=/dev/null
  source "$BOOTSTRAP_CACHE"
  info "Loaded cached credentials from $BOOTSTRAP_CACHE"
fi

# ── step 1: CF_API_TOKEN ─────────────────────────────────────────────────────

echo ""
info "Bootstrap: $GH_REPO → R2/$BUCKET → $CUSTOM_DOMAIN"
echo ""

if $DRY_RUN; then
  CF_API_TOKEN="${CF_API_TOKEN:-DRY_RUN_TOKEN}"
  CF_ACCOUNT_ID="${CF_ACCOUNT_ID:-DRY_RUN_ACCOUNT}"
  CF_ZONE_ID="${CF_ZONE_ID:-DRY_RUN_ZONE}"
  R2_DEV_HOSTNAME="pub-dry-run.r2.dev"
else
  if [[ -z "${CF_API_TOKEN:-}" ]]; then
    echo "  Cloudflare operator token needed."
    echo "  Go to: https://dash.cloudflare.com/profile/api-tokens"
    echo "  Click '...' next to 'foundry-linux-operator' → Roll → confirm → copy the new value."
    echo "  (Cloudflare does not let you view existing token values — rolling is the only way.)"
    echo ""
    until [[ -n "${CF_API_TOKEN:-}" ]]; do
      read -rsp "  Paste token value (input hidden): " CF_API_TOKEN; echo
      [[ -z "${CF_API_TOKEN:-}" ]] && echo "  (cannot be blank — try again)"
    done
    export CF_API_TOKEN
    cache_set CF_API_TOKEN "$CF_API_TOKEN"
    ok "[1] CF_API_TOKEN saved to $BOOTSTRAP_CACHE"
  else
    ok "[1] CF_API_TOKEN already set"
  fi

  if [[ -z "${CF_ACCOUNT_ID:-}" ]]; then
    CF_ACCOUNT_ID=$(cf_api GET "/accounts?per_page=1" | jq -r '.result[0].id')
    [[ -n "$CF_ACCOUNT_ID" && "$CF_ACCOUNT_ID" != "null" ]] \
      || die "[1] Could not resolve account ID — check token permissions"
    export CF_ACCOUNT_ID
    cache_set CF_ACCOUNT_ID "$CF_ACCOUNT_ID"
  fi
  ok "[1] Account: $CF_ACCOUNT_ID"

  if [[ -z "${CF_ZONE_ID:-}" ]]; then
    CF_ZONE_ID=$(cf_api GET "/zones?name=${CF_ZONE_NAME}" | jq -r '.result[0].id')
    [[ -n "$CF_ZONE_ID" && "$CF_ZONE_ID" != "null" ]] \
      || die "[1] Zone $CF_ZONE_NAME not found"
    export CF_ZONE_ID
    cache_set CF_ZONE_ID "$CF_ZONE_ID"
  fi
  ok "[1] Zone: $CF_ZONE_ID"
fi

R2_ENDPOINT="https://${CF_ACCOUNT_ID}.r2.cloudflarestorage.com"

# ── step 2: create R2 bucket ─────────────────────────────────────────────────

info "[2] Creating R2 bucket: $BUCKET"
if $DRY_RUN; then
  echo "  [dry-run] POST /r2/buckets {name: $BUCKET}"
else
  RESP=$(curl -sS -X POST \
    "https://api.cloudflare.com/client/v4/accounts/${CF_ACCOUNT_ID}/r2/buckets" \
    -H "Authorization: Bearer ${CF_API_TOKEN}" \
    -H "Content-Type: application/json" \
    -d "$(jq -n --arg n "$BUCKET" '{name:$n,locationHint:"auto"}')")
  if echo "$RESP" | jq -e '.success == true' &>/dev/null; then
    ok "[2] Bucket '$BUCKET' created"
  elif echo "$RESP" | jq -r '.errors[].code' 2>/dev/null | grep -qE "10004|10006"; then
    ok "[2] Bucket '$BUCKET' already exists"
  else
    die "[2] Unexpected response: $(echo "$RESP" | jq -c '.errors')"
  fi
fi

# ── step 3: enable r2.dev subdomain ─────────────────────────────────────────

info "[3] Enabling r2.dev subdomain"
if $DRY_RUN; then
  echo "  [dry-run] PUT /r2/buckets/$BUCKET/domains/managed"
else
  R2_MANAGED=$(cf_api PUT \
    "/accounts/${CF_ACCOUNT_ID}/r2/buckets/${BUCKET}/domains/managed" \
    -d '{"enabled":true}' 2>/dev/null || true)
  R2_DEV_HOSTNAME=$(echo "$R2_MANAGED" \
    | jq -r '.result.domain // empty' \
    | grep -o '[a-zA-Z0-9-]*\.r2\.dev' | head -1 || true)
  if [[ -z "${R2_DEV_HOSTNAME:-}" ]]; then
    R2_DEV_HOSTNAME=$(cf_api GET \
      "/accounts/${CF_ACCOUNT_ID}/r2/buckets/${BUCKET}/domains/managed" \
      | jq -r '.result.domain // empty' \
      | grep -o '[a-zA-Z0-9-]*\.r2\.dev' | head -1 || true)
  fi
  [[ -n "${R2_DEV_HOSTNAME:-}" ]] \
    || die "[3] Could not get r2.dev hostname — check the Cloudflare dashboard"
  ok "[3] r2.dev: $R2_DEV_HOSTNAME"
fi

# ── step 4: DNS CNAME + custom domain ────────────────────────────────────────

info "[4] DNS CNAME: $DNS_CNAME.foundrylinux.org → $R2_DEV_HOSTNAME"
if $DRY_RUN; then
  echo "  [dry-run] POST /dns_records {type: CNAME, name: $DNS_CNAME}"
else
  CNAME_ID=$(cf_api GET \
    "/zones/${CF_ZONE_ID}/dns_records?type=CNAME&name=${DNS_CNAME}.foundrylinux.org" \
    | jq -r '.result[0].id // empty' || true)
  if [[ -n "$CNAME_ID" ]]; then
    ok "[4] CNAME already exists"
  else
    cf_api POST "/zones/${CF_ZONE_ID}/dns_records" -d "$(jq -n \
      --arg name    "$DNS_CNAME" \
      --arg content "$R2_DEV_HOSTNAME" \
      '{type:"CNAME",name:$name,content:$content,proxied:true,comment:"foundry-iso R2 bucket"}')" \
      >/dev/null
    ok "[4] CNAME created"
  fi

  DOMAIN_HTTP=$(curl -sS -o /dev/null -w "%{http_code}" -X POST \
    "https://api.cloudflare.com/client/v4/accounts/${CF_ACCOUNT_ID}/r2/buckets/${BUCKET}/domains/custom" \
    -H "Authorization: Bearer ${CF_API_TOKEN}" \
    -H "Content-Type: application/json" \
    -d "$(jq -n --arg d "$CUSTOM_DOMAIN" --arg z "$CF_ZONE_ID" \
          '{domain:$d,zoneId:$z,enabled:true}')")
  if [[ "$DOMAIN_HTTP" == "200" ]]; then
    ok "[4] Custom domain attached: $CUSTOM_DOMAIN"
  elif [[ "$DOMAIN_HTTP" == "409" ]]; then
    ok "[4] Custom domain already attached: $CUSTOM_DOMAIN"
  else
    die "[4] HTTP $DOMAIN_HTTP attaching custom domain"
  fi
fi

# ── step 5: URL rewrite / → /index.html ─────────────────────────────────────

info "[5] URL rewrite: $CUSTOM_DOMAIN/ → /index.html"
REWRITE_EXPR="(http.host eq \"${CUSTOM_DOMAIN}\" and http.request.uri.path eq \"/\")"
if $DRY_RUN; then
  echo "  [dry-run] PUT /zones/.../rulesets/phases/http_request_transform/entrypoint"
else
  PHASE_JSON=$(cf_api GET \
    "/zones/${CF_ZONE_ID}/rulesets/phases/http_request_transform/entrypoint" \
    2>/dev/null || echo '{}')
  RULESET_ID=$(echo "$PHASE_JSON" | jq -r '.result.id // empty' || true)
  EXISTING=$(echo "$PHASE_JSON" | jq -r \
    --arg expr "$REWRITE_EXPR" \
    '.result.rules[]? | select(.expression == $expr) | .id' || true)
  RULE_BODY=$(jq -n --arg expr "$REWRITE_EXPR" \
    '{action:"rewrite",action_parameters:{uri:{path:{value:"/index.html"}}},expression:$expr,enabled:true}')
  if [[ -n "$EXISTING" ]]; then
    ok "[5] URL rewrite rule already exists"
  elif [[ -z "$RULESET_ID" ]]; then
    cf_api PUT "/zones/${CF_ZONE_ID}/rulesets/phases/http_request_transform/entrypoint" \
      -d "$(jq -n --argjson rule "$RULE_BODY" '{name:"Zone Rewrite Rules",rules:[$rule]}')" >/dev/null
    ok "[5] URL rewrite rule created"
  else
    cf_api POST "/zones/${CF_ZONE_ID}/rulesets/${RULESET_ID}/rules" -d "$RULE_BODY" >/dev/null
    ok "[5] URL rewrite rule added"
  fi
fi

# ── step 6: lifecycle — expire objects after 90 days ────────────────────────

info "[6] Setting 90-day lifecycle on $BUCKET"
if $DRY_RUN; then
  echo "  [dry-run] PUT /r2/buckets/$BUCKET/lifecycle"
else
  LC_RESP=$(curl -sS -X PUT \
    "https://api.cloudflare.com/client/v4/accounts/${CF_ACCOUNT_ID}/r2/buckets/${BUCKET}/lifecycle" \
    -H "Authorization: Bearer ${CF_API_TOKEN}" \
    -H "Content-Type: application/json" \
    -d '{"rules":[{"id":"expire-old-releases","enabled":true,"prefix":"","expiration":{"days":90}}]}')
  if echo "$LC_RESP" | python3 -c "import sys,json; sys.exit(0 if json.load(sys.stdin).get('success') else 1)" 2>/dev/null; then
    ok "[6] Lifecycle rule set (90 days)"
  else
    echo "  [warn]  [6] Lifecycle rule failed (non-fatal): $(echo "$LC_RESP" | python3 -c "import sys,json; r=json.load(sys.stdin); print(r.get('errors',r))" 2>/dev/null || echo "$LC_RESP")"
  fi
fi

# ── step 7: R2 API token for CI (foundry-iso-ci) ────────────────────────────

ISO_R2_KEY_ID="${ISO_R2_KEY_ID:-}"
ISO_R2_SECRET="${ISO_R2_SECRET:-}"

if $DRY_RUN; then
  ISO_R2_KEY_ID="${ISO_R2_KEY_ID:-DRY_RUN_KEY_ID}"
  ISO_R2_SECRET="${ISO_R2_SECRET:-DRY_RUN_SECRET}"
elif [[ -z "$ISO_R2_KEY_ID" || -z "$ISO_R2_SECRET" ]]; then
  echo ""
  echo "  Create an R2 API token for CI at:"
  echo "  https://dash.cloudflare.com/${CF_ACCOUNT_ID}/r2/api-tokens"
  echo ""
  echo "  Click 'Create Account API token'"
  echo "    Token name:  $R2_TOKEN_NAME"
  echo "    Permissions: Object Read & Write"
  echo "    Bucket:      Apply to specific bucket → $BUCKET"
  echo ""
  echo "  Copy the S3-compatible credentials shown after creation:"
  echo ""
  until [[ -n "$ISO_R2_KEY_ID" ]]; do
    read -rsp "  Paste Access Key ID (input hidden): " ISO_R2_KEY_ID; echo
    [[ -z "$ISO_R2_KEY_ID" ]] && echo "  (cannot be blank)"
  done
  until [[ -n "$ISO_R2_SECRET" ]]; do
    read -rsp "  Paste Secret Access Key (input hidden): " ISO_R2_SECRET; echo
    [[ -z "$ISO_R2_SECRET" ]] && echo "  (cannot be blank)"
  done
  export ISO_R2_KEY_ID ISO_R2_SECRET
  cache_set ISO_R2_KEY_ID "$ISO_R2_KEY_ID"
  cache_set ISO_R2_SECRET "$ISO_R2_SECRET"
  ok "[7] R2 credentials captured"
else
  ok "[7] R2 credentials already in cache"
fi

# ── step 8: get shared GPG signing key ──────────────────────────────────────
# Key has no passphrase (generated by bootstrap.sh for CI use).

GPG_KEY_EMAIL="packages@foundrylinux.org"
GPG_PRIVATE_KEY="${GPG_PRIVATE_KEY:-}"

if $DRY_RUN; then
  GPG_PRIVATE_KEY="${GPG_PRIVATE_KEY:-DRY_RUN_GPG_KEY}"
elif [[ -z "$GPG_PRIVATE_KEY" ]]; then
  info "[8] Trying r2://${SECRETS_BUCKET}/GPG_PRIVATE_KEY"
  GPG_PRIVATE_KEY=$(r2_get_secret "GPG_PRIVATE_KEY" 2>/dev/null || true)
  if [[ -z "$GPG_PRIVATE_KEY" ]]; then
    info "[8] Not in r2://foundry-secrets — exporting from local GPG keyring"
    GPG_PRIVATE_KEY=$(gpg --armor --export-secret-keys "$GPG_KEY_EMAIL" 2>/dev/null || true)
    [[ -n "$GPG_PRIVATE_KEY" ]] \
      || die "[8] GPG key for $GPG_KEY_EMAIL not in local keyring. Re-run bootstrap.sh."
    ok "[8] Exported from local keyring; storing in r2://foundry-secrets for next time"
    printf '%s' "$GPG_PRIVATE_KEY" | curl -fsSL -X PUT \
      "https://api.cloudflare.com/client/v4/accounts/${CF_ACCOUNT_ID}/r2/buckets/${SECRETS_BUCKET}/objects/GPG_PRIVATE_KEY" \
      -H "Authorization: Bearer ${CF_API_TOKEN}" \
      -H "Content-Type: text/plain; charset=utf-8" \
      --data-binary @- >/dev/null
  fi
  export GPG_PRIVATE_KEY
  ok "[8] GPG_PRIVATE_KEY ready"
else
  ok "[8] GPG_PRIVATE_KEY already in environment"
fi

# ── step 9: set GitHub Actions secrets on foundry-linux/foundry-iso ─────────

info "[9] Setting GitHub Actions secrets on $GH_REPO"
if $DRY_RUN; then
  echo "  [dry-run] gh secret set GPG_PRIVATE_KEY      --repo $GH_REPO"
  echo "  [dry-run] gh secret set R2_ACCOUNT_ID        --repo $GH_REPO"
  echo "  [dry-run] gh secret set R2_ACCESS_KEY_ID  --repo $GH_REPO"
  echo "  [dry-run] gh secret set R2_SECRET_ACCESS_KEY --repo $GH_REPO"
  echo "  [dry-run] gh secret set R2_ENDPOINT       --repo $GH_REPO"
else
  gh secret set GPG_PRIVATE_KEY      --repo "$GH_REPO" --body "$GPG_PRIVATE_KEY"
  gh secret set R2_ACCOUNT_ID        --repo "$GH_REPO" --body "$CF_ACCOUNT_ID"
  gh secret set R2_ACCESS_KEY_ID     --repo "$GH_REPO" --body "$ISO_R2_KEY_ID"
  gh secret set R2_SECRET_ACCESS_KEY --repo "$GH_REPO" --body "$ISO_R2_SECRET"
  gh secret set R2_ENDPOINT          --repo "$GH_REPO" --body "$R2_ENDPOINT"
  ok "[9] Secrets set"
  gh secret list --repo "$GH_REPO"
fi

echo ""
ok "Bootstrap complete."
echo "  Bucket:  r2://$BUCKET"
echo "  Domain:  https://$CUSTOM_DOMAIN (DNS propagation may take 1–2 min)"
echo "  Next:    task iso-release TAG=v0.0.1"
echo ""
