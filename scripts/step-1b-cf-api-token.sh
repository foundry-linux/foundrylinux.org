#!/usr/bin/env bash
# Create a scoped Cloudflare operator token for the foundry-apt bootstrap.
# Phase 1 bootstrap Step 1b — run once, produces CF_API_TOKEN used by
# step-3-9-bootstrap.sh.
#
# Usage:
#   bash scripts/step-1b-cf-api-token.sh [--dry-run] [-h]
#
# Required env vars (Global API Key — used once here, then never again):
#   CF_EMAIL          — Cloudflare account email
#   CF_GLOBAL_API_KEY — Cloudflare Global API Key
#                       (Dash → My Profile → API Tokens → Global API Key)
#
# Outputs: prints three export lines. Copy them into your shell before
# running step-3-9-bootstrap.sh.

set -euo pipefail

# ── config ───────────────────────────────────────────────────────────────────

TOKEN_NAME="foundry-linux-operator"
ZONE_NAME="foundrylinux.org"
DRY_RUN=false

# ── helpers ──────────────────────────────────────────────────────────────────

info() { echo "  [info]  $*"; }
ok()   { echo "  [ok]    $*"; }
warn() { echo "  [warn]  $*" >&2; }
err()  { echo "  [error] $*" >&2; }
die()  { err "$*"; exit 1; }

usage() {
    sed -n '2,11p' "$0" | sed 's/^# //'
    exit 0
}

# Cloudflare API using Global API Key auth
cf() {
    local method="$1" path="$2"
    shift 2
    curl -fsSL -X "$method" \
        "https://api.cloudflare.com/client/v4${path}" \
        -H "X-Auth-Email: ${CF_EMAIL}" \
        -H "X-Auth-Key: ${CF_GLOBAL_API_KEY}" \
        -H "Content-Type: application/json" \
        "$@"
}

# ── arg parse ────────────────────────────────────────────────────────────────

for arg in "$@"; do
    case "$arg" in
        -h|--help)  usage ;;
        --dry-run)  DRY_RUN=true ;;
        *)          die "Unknown argument: $arg" ;;
    esac
done

# ── preflight ────────────────────────────────────────────────────────────────

command -v curl &>/dev/null || die "curl not found"
command -v jq   &>/dev/null || die "jq not found"

if ! $DRY_RUN; then
    : "${CF_EMAIL:?CF_EMAIL is required}"
    : "${CF_GLOBAL_API_KEY:?CF_GLOBAL_API_KEY is required}"
    cf GET "/user" &>/dev/null || die "Cloudflare authentication failed — check CF_EMAIL and CF_GLOBAL_API_KEY"
fi

# ── idempotency check ─────────────────────────────────────────────────────────

if ! $DRY_RUN; then
    EXISTING=$(cf GET "/user/tokens" \
        | jq -r ".result[] | select(.name == \"${TOKEN_NAME}\") | .id" || true)
    if [[ -n "${EXISTING}" ]]; then
        warn "Token '${TOKEN_NAME}' already exists (ID: ${EXISTING})"
        warn "Token value is not retrievable after creation. Options:"
        warn "  1. Use the existing token if you still have it."
        warn "  2. Delete it at https://dash.cloudflare.com/profile/api-tokens and re-run."
        exit 1
    fi
fi

# ── discover account ID and zone ID ──────────────────────────────────────────

info "Looking up account ID"
if $DRY_RUN; then
    CF_ACCOUNT_ID="DRY_RUN_ACCOUNT_ID"
    echo "  [dry-run] GET /accounts"
else
    CF_ACCOUNT_ID=$(cf GET "/accounts?per_page=1" | jq -r '.result[0].id')
    [[ -n "${CF_ACCOUNT_ID}" ]] || die "Could not retrieve account ID"
    ok "Account ID: ${CF_ACCOUNT_ID}"
fi

info "Looking up zone ID for ${ZONE_NAME}"
if $DRY_RUN; then
    CF_ZONE_ID="DRY_RUN_ZONE_ID"
    echo "  [dry-run] GET /zones?name=${ZONE_NAME}"
else
    CF_ZONE_ID=$(cf GET "/zones?name=${ZONE_NAME}" | jq -r '.result[0].id')
    [[ -n "${CF_ZONE_ID}" && "${CF_ZONE_ID}" != "null" ]] \
        || die "Zone ${ZONE_NAME} not found — is it in your Cloudflare account?"
    ok "Zone ID: ${CF_ZONE_ID}"
fi

# ── discover permission group IDs ─────────────────────────────────────────────

info "Fetching permission groups"
if $DRY_RUN; then
    R2_WRITE_ID="DRY_RUN_R2_ID"
    DNS_WRITE_ID="DRY_RUN_DNS_ID"
    TOKEN_EDIT_ID="DRY_RUN_TOKEN_ID"
    echo "  [dry-run] GET /user/tokens/permission_groups"
else
    PERM_GROUPS=$(cf GET "/user/tokens/permission_groups")

    # R2 bucket + object write (name varies by Cloudflare API version)
    R2_WRITE_ID=$(echo "${PERM_GROUPS}" \
        | jq -r '.result[] | select(.name | test("R2.*Storage.*Bucket.*Item.*Write"; "i")) | .id' \
        | head -1 || true)
    if [[ -z "${R2_WRITE_ID}" ]]; then
        R2_WRITE_ID=$(echo "${PERM_GROUPS}" \
            | jq -r '.result[] | select(.name | test("R2"; "i")) | select(.name | test("Write|Edit"; "i")) | .id' \
            | head -1 || true)
    fi
    [[ -n "${R2_WRITE_ID}" ]] || die "Could not find R2 write permission group — check: cf GET /user/tokens/permission_groups"

    # Zone DNS write
    DNS_WRITE_ID=$(echo "${PERM_GROUPS}" \
        | jq -r '.result[] | select(.name | test("Zone.*DNS.*Write|DNS.*Write"; "i")) | .id' \
        | head -1 || true)
    [[ -n "${DNS_WRITE_ID}" ]] || die "Could not find Zone DNS Write permission group"

    # User API token edit (needed to create the narrow foundry-apt-ci token)
    TOKEN_EDIT_ID=$(echo "${PERM_GROUPS}" \
        | jq -r '.result[] | select(.name | test("User.*API.*Token.*Edit|API.*Token.*Edit"; "i")) | .id' \
        | head -1 || true)
    [[ -n "${TOKEN_EDIT_ID}" ]] || die "Could not find User API Tokens Edit permission group"

    ok "Permission groups: R2=${R2_WRITE_ID} DNS=${DNS_WRITE_ID} Tokens=${TOKEN_EDIT_ID}"
fi

# ── create scoped operator token ──────────────────────────────────────────────

info "Creating scoped operator token: ${TOKEN_NAME}"

TOKEN_BODY=$(cat <<EOF
{
  "name": "${TOKEN_NAME}",
  "policies": [
    {
      "effect": "allow",
      "resources": { "com.cloudflare.api.account.${CF_ACCOUNT_ID}": "*" },
      "permission_groups": [
        { "id": "${R2_WRITE_ID}" },
        { "id": "${TOKEN_EDIT_ID}" }
      ]
    },
    {
      "effect": "allow",
      "resources": { "com.cloudflare.api.account.zone.${CF_ZONE_ID}": "*" },
      "permission_groups": [
        { "id": "${DNS_WRITE_ID}" }
      ]
    }
  ]
}
EOF
)

if $DRY_RUN; then
    echo "  [dry-run] POST /user/tokens <body>"
    printf '%s\n' "${TOKEN_BODY}" | while IFS= read -r line; do echo "              ${line}"; done
    CF_API_TOKEN="DRY_RUN_TOKEN_VALUE"
else
    RESPONSE=$(cf POST "/user/tokens" -d "${TOKEN_BODY}")
    CF_API_TOKEN=$(echo "${RESPONSE}" | jq -r '.result.value')
    [[ -n "${CF_API_TOKEN}" && "${CF_API_TOKEN}" != "null" ]] \
        || die "Token creation failed: $(echo "${RESPONSE}" | jq -r '.errors')"
    ok "Token created"
fi

# ── print export commands ─────────────────────────────────────────────────────

echo ""
ok "Copy these into your shell before running step-3-9-bootstrap.sh:"
echo ""
echo "export CF_API_TOKEN='${CF_API_TOKEN}'"
echo "export CF_ACCOUNT_ID='${CF_ACCOUNT_ID}'"
echo "export CF_ZONE_ID='${CF_ZONE_ID}'"
echo ""
info "The CF_GLOBAL_API_KEY is no longer needed — do not store it."
info "Verify the token: curl -fsSL https://api.cloudflare.com/client/v4/user/tokens/verify -H 'Authorization: Bearer \${CF_API_TOKEN}'"
