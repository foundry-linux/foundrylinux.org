#!/usr/bin/env bash
# Full Phase 1 bootstrap: push foundry-apt to GitHub, generate GPG key,
# wire GitHub secrets, provision R2 bucket, configure DNS, upload public key.
# Steps 1b–9 — run once from the linuxfoundry.org repo root.
#
# Usage:
#   bash scripts/bootstrap.sh [--dry-run] [-h]
#
# Step 1b requires a Cloudflare operator token. If CF_API_TOKEN is not already
# exported, the script shows instructions to create it and prompts for the value.
# CF_ACCOUNT_ID and CF_ZONE_ID are fetched automatically via the API.
#
# Prerequisites:
#   gpg (gnupg2), shred, curl, jq
#   aws CLI (S3-compat client for R2 upload — no AWS account needed)
#   gh CLI (gh auth login)
#
# After this script: Step 10 — push the first release tag from your
# foundry-apt checkout to trigger the publish workflow.

set -euo pipefail

REPO_ROOT="$(cd "$(dirname "$0")/.." && pwd)"

# ── config ───────────────────────────────────────────────────────────────────

GH_ORG="foundry-linux"
PKG_NAME="foundry-apt"
GH_REPO="${GH_ORG}/${PKG_NAME}"
SRC_DIR="${REPO_ROOT}/${PKG_NAME}"
REPO_DESC="Foundry Linux signed APT repo and metapackages"

KEY_NAME="Foundry Linux Packages"
KEY_EMAIL="packages@foundrylinux.org"
KEY_BITS=4096
KEY_EXPIRY="2y"
PUB_KEY="/tmp/foundry-packages.pub.gpg"
SEC_KEY="/tmp/foundry-packages.sec.gpg"

R2_BUCKET="foundry-apt"
R2_TOKEN_NAME="foundry-apt-ci"
CUSTOM_DOMAIN="apt.foundrylinux.org"
DNS_CNAME="apt"
CF_OPERATOR_TOKEN_NAME="foundry-linux-operator"
CF_ZONE_NAME="foundrylinux.org"

DRY_RUN=false

# Temp paths — all cleaned up on exit
WORK_DIR=""
BATCH_FILE=""

# ── helpers ──────────────────────────────────────────────────────────────────

info() { echo "  [info]  $*"; }
ok()   { echo "  [ok]    $*"; }
warn() { echo "  [warn]  $*" >&2; }
err()  { echo "  [error] $*" >&2; }
die()  { err "$*"; exit 1; }

usage() {
    sed -n '2,14p' "$0" | sed 's/^# //'
    exit 0
}

cleanup() {
    [[ -n "${WORK_DIR}"   && -d "${WORK_DIR}"   ]] && rm -rf "${WORK_DIR}"
    [[ -n "${BATCH_FILE}" && -f "${BATCH_FILE}" ]] && rm -f  "${BATCH_FILE}"
}
trap cleanup EXIT

cf_api() {
    local method="$1" path="$2"
    shift 2
    curl -fsSL -X "$method" \
        "https://api.cloudflare.com/client/v4${path}" \
        -H "Authorization: Bearer ${CF_API_TOKEN:-}" \
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

[[ -d "${SRC_DIR}" ]] || die "${PKG_NAME}/ not found under ${REPO_ROOT}"

command -v gpg   &>/dev/null || die "gpg not found — install gnupg2"
command -v shred &>/dev/null || die "shred not found (install util-linux)"
command -v curl  &>/dev/null || die "curl not found"
command -v jq    &>/dev/null || die "jq not found"
command -v aws   &>/dev/null || die "aws CLI not found (needed as S3-compat client for R2)"
command -v gh    &>/dev/null || die "gh CLI not found — https://cli.github.com"

if ! $DRY_RUN; then
    gh auth status &>/dev/null || die "gh not authenticated — run: gh auth login"
    if [[ -z "${CF_API_TOKEN:-}" ]]; then
        echo "  Cloudflare operator token needed. Create '${CF_OPERATOR_TOKEN_NAME}' first:"
        echo ""
        echo "  https://dash.cloudflare.com/profile/api-tokens"
        echo "  Click '+ Create Token', then 'Get started' next to 'Create Custom Token'"
        echo "  Name: ${CF_OPERATOR_TOKEN_NAME}"
        echo "  Permissions:"
        echo "    Account | Workers R2 Storage | Edit"
        echo "    Account | API Tokens         | Edit"
        echo "    Zone    | DNS                | Edit  (Specific zone: ${CF_ZONE_NAME})"
        echo "  Account Resources: Include → your account (default)"
        echo "  Zone Resources:    Include → Specific zone → ${CF_ZONE_NAME}"
        echo ""
        read -rsp "  Paste token value (input hidden): " CF_API_TOKEN
        echo
        export CF_API_TOKEN
    fi
fi

R2_ACCESS_KEY_ID=""
R2_SECRET_ACCESS_KEY=""
R2_DEV_HOSTNAME=""

echo ""
info "Bootstrap: Steps 1b–9 for ${GH_REPO}"
echo ""

# ════════════════════════════════════════════════════════════════════════════
# Step 1b — Resolve CF_ACCOUNT_ID and CF_ZONE_ID from the operator token
# ════════════════════════════════════════════════════════════════════════════

if $DRY_RUN; then
    CF_API_TOKEN="${CF_API_TOKEN:-DRY_RUN_TOKEN}"
    CF_ACCOUNT_ID="${CF_ACCOUNT_ID:-DRY_RUN_ACCOUNT_ID}"
    CF_ZONE_ID="${CF_ZONE_ID:-DRY_RUN_ZONE_ID}"
    echo "  [dry-run] GET /accounts → CF_ACCOUNT_ID"
    echo "  [dry-run] GET /zones?name=${CF_ZONE_NAME} → CF_ZONE_ID"
else
    if [[ -z "${CF_ACCOUNT_ID:-}" ]]; then
        CF_ACCOUNT_ID=$(cf_api GET "/accounts?per_page=1" | jq -r '.result[0].id')
        [[ -n "${CF_ACCOUNT_ID}" && "${CF_ACCOUNT_ID}" != "null" ]] \
            || die "[1b] Could not retrieve account ID — check CF_API_TOKEN permissions"
        ok "[1b] Account ID: ${CF_ACCOUNT_ID}"
        export CF_ACCOUNT_ID
    else
        ok "[1b] CF_ACCOUNT_ID already set: ${CF_ACCOUNT_ID}"
    fi

    if [[ -z "${CF_ZONE_ID:-}" ]]; then
        CF_ZONE_ID=$(cf_api GET "/zones?name=${CF_ZONE_NAME}" | jq -r '.result[0].id')
        [[ -n "${CF_ZONE_ID}" && "${CF_ZONE_ID}" != "null" ]] \
            || die "[1b] Zone ${CF_ZONE_NAME} not found — check CF_API_TOKEN has DNS:Edit for this zone"
        ok "[1b] Zone ID: ${CF_ZONE_ID}"
        export CF_ZONE_ID
    else
        ok "[1b] CF_ZONE_ID already set: ${CF_ZONE_ID}"
    fi
fi

R2_ENDPOINT="https://${CF_ACCOUNT_ID:-DRY_RUN}.r2.cloudflarestorage.com"

# ════════════════════════════════════════════════════════════════════════════
# Step 2b — Push foundry-apt/ to standalone GitHub repo
# ════════════════════════════════════════════════════════════════════════════

if ! $DRY_RUN && gh repo view "${GH_REPO}" &>/dev/null; then
    ok "[2b] ${GH_REPO} already exists on GitHub"
else
    info "[2b] Creating https://github.com/${GH_REPO}"
    WORK_DIR="/tmp/${PKG_NAME}-push"
    if $DRY_RUN; then
        echo "  [dry-run] cp -r ${SRC_DIR} ${WORK_DIR}"
        echo "  [dry-run] git init && git add . && git commit -m 'feat: initial ${PKG_NAME} import'"
        echo "  [dry-run] gh repo create ${GH_REPO} --public --source=. --push"
        echo "  [dry-run] gh repo edit ${GH_REPO} --enable-discussions"
    else
        rm -rf "${WORK_DIR}"
        cp -r "${SRC_DIR}" "${WORK_DIR}"
        git -C "${WORK_DIR}" init -q
        git -C "${WORK_DIR}" add .
        git -C "${WORK_DIR}" commit -q -m "feat: initial ${PKG_NAME} import"
        gh repo create "${GH_REPO}" \
            --public \
            --description "${REPO_DESC}" \
            --source="${WORK_DIR}" --remote=origin --push
        gh repo edit "${GH_REPO}" --enable-discussions
        ok "[2b] Repo created: https://github.com/${GH_REPO}"
    fi
fi

# ════════════════════════════════════════════════════════════════════════════
# Step 3 — Generate GPG signing key
# ════════════════════════════════════════════════════════════════════════════

if gpg --list-keys "${KEY_EMAIL}" &>/dev/null; then
    ok "[3] GPG key for ${KEY_EMAIL} already in keyring"
else
    info "[3] Generating ${KEY_BITS}-bit RSA signing key (${KEY_EMAIL}, expiry: ${KEY_EXPIRY})"
    BATCH_FILE="$(mktemp /tmp/gpg-batch-XXXXXX)"
    cat > "${BATCH_FILE}" <<EOF
%no-protection
Key-Type: RSA
Key-Usage: sign
Key-Length: ${KEY_BITS}
Name-Real: ${KEY_NAME}
Name-Email: ${KEY_EMAIL}
Expire-Date: ${KEY_EXPIRY}
%commit
EOF
    if $DRY_RUN; then
        echo "  [dry-run] gpg --batch --gen-key <batch-file>"
    else
        gpg --batch --gen-key "${BATCH_FILE}"
        ok "[3] GPG key generated"
    fi
fi

if $DRY_RUN; then
    echo "  [dry-run] gpg --armor --export ${KEY_EMAIL} > ${PUB_KEY}"
    echo "  [dry-run] gpg --armor --export-secret-keys ${KEY_EMAIL} > ${SEC_KEY}"
else
    gpg --armor --export "${KEY_EMAIL}" > "${PUB_KEY}"
    gpg --armor --export-secret-keys "${KEY_EMAIL}" > "${SEC_KEY}"
    chmod 600 "${SEC_KEY}"
fi

# ════════════════════════════════════════════════════════════════════════════
# Step 4 — Set GPG_PRIVATE_KEY GitHub secret, shred local private key
# ════════════════════════════════════════════════════════════════════════════

GPG_SECRET_EXISTS=false
if ! $DRY_RUN && gh secret list --repo "${GH_REPO}" 2>/dev/null | grep -q "^GPG_PRIVATE_KEY"; then
    GPG_SECRET_EXISTS=true
fi

if $GPG_SECRET_EXISTS; then
    ok "[4] GPG_PRIVATE_KEY secret already exists on ${GH_REPO}"
    [[ -f "${SEC_KEY}" ]] && shred -u "${SEC_KEY}"
else
    info "[4] Setting GPG_PRIVATE_KEY secret on ${GH_REPO}"
    if $DRY_RUN; then
        echo "  [dry-run] gh secret set GPG_PRIVATE_KEY --repo ${GH_REPO} --body <private-key>"
        echo "  [dry-run] shred -u ${SEC_KEY}"
    else
        gh secret set GPG_PRIVATE_KEY --repo "${GH_REPO}" --body "$(cat "${SEC_KEY}")"
        ok "[4] GPG_PRIVATE_KEY secret set"
        shred -u "${SEC_KEY}"
        ok "[4] Private key shredded"
    fi
fi

# ════════════════════════════════════════════════════════════════════════════
# Step 6 — Cloudflare R2 bucket + scoped CI token
# ════════════════════════════════════════════════════════════════════════════

if $DRY_RUN; then
    echo "  [dry-run] POST /accounts/.../r2/buckets {name: ${R2_BUCKET}}"
else
    BUCKET_RESPONSE=$(curl -sS -X POST \
        "https://api.cloudflare.com/client/v4/accounts/${CF_ACCOUNT_ID}/r2/buckets" \
        -H "Authorization: Bearer ${CF_API_TOKEN}" \
        -H "Content-Type: application/json" \
        -d "{\"name\":\"${R2_BUCKET}\",\"locationHint\":\"auto\"}")
    if echo "${BUCKET_RESPONSE}" | jq -e '.success == true' &>/dev/null; then
        ok "[6] R2 bucket '${R2_BUCKET}' created"
    elif echo "${BUCKET_RESPONSE}" | jq -r '.errors[].code' 2>/dev/null | grep -q "10006"; then
        ok "[6] R2 bucket '${R2_BUCKET}' already exists"
    else
        die "[6] Unexpected bucket response: $(echo "${BUCKET_RESPONSE}" | jq -c '.errors')"
    fi
fi

info "[6] Enabling r2.dev subdomain"
if $DRY_RUN; then
    R2_DEV_HOSTNAME="pub-dry-run.r2.dev"
    echo "  [dry-run] PUT /r2/buckets/${R2_BUCKET}/domains/managed {enabled: true}"
else
    R2_MANAGED=$(cf_api PUT \
        "/accounts/${CF_ACCOUNT_ID}/r2/buckets/${R2_BUCKET}/domains/managed" \
        -d '{"enabled":true}')
    R2_DEV_HOSTNAME="$(echo "${R2_MANAGED}" | jq -r '.result.domain // empty' \
        | grep -o '[a-zA-Z0-9-]*\.r2\.dev' | head -1 || true)"
    if [[ -z "${R2_DEV_HOSTNAME}" ]]; then
        R2_MANAGED_GET=$(cf_api GET \
            "/accounts/${CF_ACCOUNT_ID}/r2/buckets/${R2_BUCKET}/domains/managed")
        R2_DEV_HOSTNAME="$(echo "${R2_MANAGED_GET}" | jq -r '.result.domain // empty' \
            | grep -o '[a-zA-Z0-9-]*\.r2\.dev' | head -1 || true)"
    fi
    [[ -n "${R2_DEV_HOSTNAME}" ]] \
        || die "[6] Could not determine r2.dev hostname — check the Cloudflare dashboard"
    ok "[6] r2.dev hostname: ${R2_DEV_HOSTNAME}"
fi

EXISTING_TOKEN_ID=""
if ! $DRY_RUN; then
    EXISTING_TOKEN_ID=$(cf_api GET "/user/tokens" \
        | jq -r ".result[] | select(.name == \"${R2_TOKEN_NAME}\") | .id" || true)
fi

if [[ -n "${EXISTING_TOKEN_ID}" ]]; then
    warn "[6] R2 CI token '${R2_TOKEN_NAME}' already exists (ID: ${EXISTING_TOKEN_ID})"
    warn "    Secret value is gone. If GitHub secrets aren't set, delete this token at"
    warn "    https://dash.cloudflare.com/profile/api-tokens and re-run."
    R2_ACCESS_KEY_ID="${EXISTING_TOKEN_ID}"
    R2_SECRET_ACCESS_KEY=""
else
    info "[6] Creating scoped R2 CI token: ${R2_TOKEN_NAME}"
    if $DRY_RUN; then
        R2_WRITE_ID="DRY_RUN_R2_WRITE_ID"
        echo "  [dry-run] GET /user/tokens/permission_groups"
    else
        R2_WRITE_ID=$(cf_api GET "/user/tokens/permission_groups" \
            | jq -r '.result[] | select(.name | test("R2.*Storage.*Bucket.*Item.*Write"; "i")) | .id' \
            | head -1 || true)
        [[ -n "${R2_WRITE_ID}" ]] || die "[6] Could not find R2 write permission group ID"
    fi

    if $DRY_RUN; then
        echo "  [dry-run] POST /user/tokens {name: ${R2_TOKEN_NAME}}"
        R2_ACCESS_KEY_ID="DRY_RUN_KEY_ID"
        R2_SECRET_ACCESS_KEY="DRY_RUN_SECRET"
    else
        R2_TOKEN_RESPONSE=$(cf_api POST "/user/tokens" -d "{
            \"name\": \"${R2_TOKEN_NAME}\",
            \"policies\": [{
              \"effect\": \"allow\",
              \"resources\": { \"com.cloudflare.api.account.${CF_ACCOUNT_ID}\": \"*\" },
              \"permission_groups\": [{
                \"id\": \"${R2_WRITE_ID}\",
                \"name\": \"Workers R2 Storage Bucket Item Write\"
              }]
            }]
          }")
        R2_ACCESS_KEY_ID="$(echo "${R2_TOKEN_RESPONSE}" | jq -r '.result.id')"
        R2_SECRET_ACCESS_KEY="$(echo "${R2_TOKEN_RESPONSE}" | jq -r '.result.value')"
        [[ -n "${R2_ACCESS_KEY_ID}" && "${R2_ACCESS_KEY_ID}" != "null" ]] \
            || die "[6] R2 token creation failed: $(echo "${R2_TOKEN_RESPONSE}" | jq -c '.errors')"
        ok "[6] R2 CI token created (ID: ${R2_ACCESS_KEY_ID})"
    fi
fi

# ════════════════════════════════════════════════════════════════════════════
# Step 7 — DNS CNAME + attach custom domain to R2 bucket
# ════════════════════════════════════════════════════════════════════════════

CNAME_EXISTS=""
if ! $DRY_RUN; then
    CNAME_EXISTS=$(cf_api GET \
        "/zones/${CF_ZONE_ID}/dns/records?type=CNAME&name=${DNS_CNAME}.foundrylinux.org" \
        | jq -r '.result[0].id // empty' || true)
fi

if [[ -n "${CNAME_EXISTS}" ]]; then
    ok "[7] DNS CNAME ${DNS_CNAME}.foundrylinux.org already exists"
else
    info "[7] Creating DNS CNAME: ${DNS_CNAME}.foundrylinux.org → ${R2_DEV_HOSTNAME}"
    if $DRY_RUN; then
        echo "  [dry-run] POST /zones/.../dns/records {type: CNAME, name: ${DNS_CNAME}}"
    else
        cf_api POST "/zones/${CF_ZONE_ID}/dns/records" -d "{
            \"type\": \"CNAME\",
            \"name\": \"${DNS_CNAME}\",
            \"content\": \"${R2_DEV_HOSTNAME}\",
            \"proxied\": true,
            \"comment\": \"foundry-apt R2 bucket\"
          }" >/dev/null
        ok "[7] DNS CNAME created"
    fi
fi

info "[7] Attaching custom domain ${CUSTOM_DOMAIN} to R2 bucket"
if $DRY_RUN; then
    echo "  [dry-run] PUT /r2/buckets/${R2_BUCKET}/domains/custom"
else
    cf_api PUT \
        "/accounts/${CF_ACCOUNT_ID}/r2/buckets/${R2_BUCKET}/domains/custom" \
        -d "{\"domains\":[{\"domain\":\"${CUSTOM_DOMAIN}\",\"enabled\":true}]}" >/dev/null
    ok "[7] Custom domain attached: ${CUSTOM_DOMAIN}"
fi

# ════════════════════════════════════════════════════════════════════════════
# Step 8 — Upload public signing key to R2, shred local copy
# ════════════════════════════════════════════════════════════════════════════

KEY_LIVE=false
if ! $DRY_RUN && curl -fsSL "https://${CUSTOM_DOMAIN}/key.gpg" \
        | gpg --show-keys &>/dev/null 2>&1; then
    KEY_LIVE=true
fi

if $KEY_LIVE; then
    ok "[8] key.gpg already reachable at https://${CUSTOM_DOMAIN}/key.gpg"
    [[ -f "${PUB_KEY}" ]] && shred -u "${PUB_KEY}"
else
    info "[8] Uploading public key → s3://${R2_BUCKET}/key.gpg"
    if $DRY_RUN; then
        echo "  [dry-run] aws s3 cp ${PUB_KEY} s3://${R2_BUCKET}/key.gpg --endpoint-url ${R2_ENDPOINT}"
        echo "  [dry-run] shred -u ${PUB_KEY}"
    else
        [[ -f "${PUB_KEY}" ]] \
            || die "[8] Public key missing — re-export: gpg --armor --export ${KEY_EMAIL} > ${PUB_KEY}"
        [[ -n "${R2_SECRET_ACCESS_KEY}" ]] \
            || die "[8] R2_SECRET_ACCESS_KEY empty — see warning above"
        AWS_ACCESS_KEY_ID="${R2_ACCESS_KEY_ID}" \
        AWS_SECRET_ACCESS_KEY="${R2_SECRET_ACCESS_KEY}" \
        aws s3 cp "${PUB_KEY}" \
            "s3://${R2_BUCKET}/key.gpg" \
            --endpoint-url "${R2_ENDPOINT}"
        ok "[8] Public key uploaded"
        shred -u "${PUB_KEY}"
        ok "[8] Public key shredded"
    fi
fi

# ════════════════════════════════════════════════════════════════════════════
# Step 9 — Set GitHub Actions secrets
# ════════════════════════════════════════════════════════════════════════════

info "[9] Setting GitHub Actions secrets on ${GH_REPO}"

if $DRY_RUN; then
    echo "  [dry-run] gh secret set R2_ACCESS_KEY_ID     --repo ${GH_REPO}"
    echo "  [dry-run] gh secret set R2_SECRET_ACCESS_KEY --repo ${GH_REPO}"
    echo "  [dry-run] gh secret set R2_ENDPOINT          --repo ${GH_REPO}"
else
    [[ -n "${R2_SECRET_ACCESS_KEY}" ]] \
        || die "[9] R2_SECRET_ACCESS_KEY empty — see warning above"
    gh secret set R2_ACCESS_KEY_ID     --repo "${GH_REPO}" --body "${R2_ACCESS_KEY_ID}"
    gh secret set R2_SECRET_ACCESS_KEY --repo "${GH_REPO}" --body "${R2_SECRET_ACCESS_KEY}"
    gh secret set R2_ENDPOINT          --repo "${GH_REPO}" --body "${R2_ENDPOINT}"
    ok "[9] GitHub secrets set"
    gh secret list --repo "${GH_REPO}"
fi

# ════════════════════════════════════════════════════════════════════════════
# Done
# ════════════════════════════════════════════════════════════════════════════

echo ""
ok "Steps 1b–9 complete."
echo ""
info "Step 10 — push the first release tag to trigger CI:"
info "  gh repo clone ${GH_REPO} /tmp/foundry-apt-release"
info "  git -C /tmp/foundry-apt-release tag v0.0.1"
info "  git -C /tmp/foundry-apt-release push origin v0.0.1"
info "  # Watch: https://github.com/${GH_REPO}/actions"
