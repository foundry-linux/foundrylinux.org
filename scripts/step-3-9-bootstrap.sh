#!/usr/bin/env bash
# GPG key, SSM, OIDC/IAM, R2 bucket, DNS, public-key upload, GitHub secrets.
# Phase 1 bootstrap Steps 3–9 — run once from any trusted machine.
#
# Usage:
#   bash scripts/step-3-9-bootstrap.sh [--dry-run] [-h]
#
# Required env vars (produced by step-1b-cf-api-token.sh):
#   CF_API_TOKEN   — Cloudflare operator token (R2 + DNS + user-token:edit)
#   CF_ACCOUNT_ID  — Cloudflare account ID
#   CF_ZONE_ID     — Cloudflare zone ID for foundrylinux.org
#
# AWS prerequisites:
#   aws CLI authenticated: sts:GetCallerIdentity, ssm:PutParameter,
#   iam:CreateOpenIDConnectProvider, iam:CreateRole, iam:PutRolePolicy,
#   iam:GetRole, iam:GetOpenIDConnectProvider
#
# Other prerequisites:
#   gpg (gnupg2), shred, curl, jq, gh CLI (gh auth login)
#
# After this script: Step 10 — push the first release tag from your
# foundry-apt checkout to trigger the publish workflow.

set -euo pipefail

# ── config ───────────────────────────────────────────────────────────────────

KEY_NAME="Foundry Linux Packages"
KEY_EMAIL="packages@foundrylinux.org"
KEY_BITS=4096
KEY_EXPIRY="2y"
SSM_PARAM="/foundry-apt/signing-key"
SSM_DESC="GPG signing key for foundry-apt CI"
PUB_KEY="/tmp/foundry-packages.pub.gpg"
SEC_KEY="/tmp/foundry-packages.sec.gpg"

OIDC_URL="https://token.actions.githubusercontent.com"
OIDC_THUMBPRINT="6938fd4d98bab03faadb97b34396831e3780aea1"
IAM_ROLE_NAME="foundry-apt-publish"
IAM_POLICY_NAME="foundry-apt-ssm-read"
GH_REPO="foundry-linux/foundry-apt"

R2_BUCKET="foundry-apt"
R2_TOKEN_NAME="foundry-apt-ci"
CUSTOM_DOMAIN="apt.foundrylinux.org"
DNS_CNAME="apt"

DRY_RUN=false

# Temp files — all cleaned up on exit
BATCH_FILE=""
TRUST_FILE=""
POLICY_FILE=""

# ── helpers ──────────────────────────────────────────────────────────────────

info() { echo "  [info]  $*"; }
ok()   { echo "  [ok]    $*"; }
warn() { echo "  [warn]  $*" >&2; }
err()  { echo "  [error] $*" >&2; }
die()  { err "$*"; exit 1; }

usage() {
    sed -n '2,13p' "$0" | sed 's/^# //'
    exit 0
}

cleanup() {
    [[ -n "${BATCH_FILE}" && -f "${BATCH_FILE}" ]] && rm -f "${BATCH_FILE}"
    [[ -n "${TRUST_FILE}"  && -f "${TRUST_FILE}"  ]] && rm -f "${TRUST_FILE}"
    [[ -n "${POLICY_FILE}" && -f "${POLICY_FILE}" ]] && rm -f "${POLICY_FILE}"
}
trap cleanup EXIT

# Cloudflare API using the operator token
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

command -v gpg   &>/dev/null || die "gpg not found — install gnupg2"
command -v aws   &>/dev/null || die "aws CLI not found"
command -v shred &>/dev/null || die "shred not found (install util-linux)"
command -v curl  &>/dev/null || die "curl not found"
command -v jq    &>/dev/null || die "jq not found"
command -v gh    &>/dev/null || die "gh CLI not found — https://cli.github.com"

if ! $DRY_RUN; then
    aws sts get-caller-identity &>/dev/null \
        || die "AWS credentials not configured — run: aws configure"
    gh auth status &>/dev/null \
        || die "gh not authenticated — run: gh auth login"
    : "${CF_API_TOKEN:?CF_API_TOKEN required — run step-1b-cf-api-token.sh first}"
    : "${CF_ACCOUNT_ID:?CF_ACCOUNT_ID required — run step-1b-cf-api-token.sh first}"
    : "${CF_ZONE_ID:?CF_ZONE_ID required — run step-1b-cf-api-token.sh first}"
fi

# Derive the AWS account ID once; used for OIDC ARN and as a fallback
if $DRY_RUN; then
    AWS_ACCOUNT_ID="111122223333"
else
    AWS_ACCOUNT_ID="$(aws sts get-caller-identity --query Account --output text)"
fi
OIDC_ARN="arn:aws:iam::${AWS_ACCOUNT_ID}:oidc-provider/token.actions.githubusercontent.com"
R2_ENDPOINT="https://${CF_ACCOUNT_ID}.r2.cloudflarestorage.com"

# Carry-forward state populated during the run
ROLE_ARN=""
R2_DEV_HOSTNAME=""
R2_ACCESS_KEY_ID=""
R2_SECRET_ACCESS_KEY=""

echo ""
info "Bootstrap: Steps 3–9 for foundry-linux/foundry-apt"
echo ""

# ════════════════════════════════════════════════════════════════════════════
# Step 3 — Generate GPG signing key
# ════════════════════════════════════════════════════════════════════════════

if gpg --list-keys "${KEY_EMAIL}" &>/dev/null; then
    ok "[3] GPG key for ${KEY_EMAIL} already in keyring — skipping generation"
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

# Export keys (always re-export in case /tmp was cleared between runs)
if $DRY_RUN; then
    echo "  [dry-run] gpg --armor --export ${KEY_EMAIL} > ${PUB_KEY}"
    echo "  [dry-run] gpg --armor --export-secret-keys ${KEY_EMAIL} > ${SEC_KEY}"
else
    gpg --armor --export "${KEY_EMAIL}" > "${PUB_KEY}"
    gpg --armor --export-secret-keys "${KEY_EMAIL}" > "${SEC_KEY}"
    chmod 600 "${SEC_KEY}"
fi

# ════════════════════════════════════════════════════════════════════════════
# Step 4 — Store private key in AWS SSM, shred local copy
# ════════════════════════════════════════════════════════════════════════════

if ! $DRY_RUN && aws ssm get-parameter --name "${SSM_PARAM}" &>/dev/null 2>&1; then
    ok "[4] SSM parameter ${SSM_PARAM} already exists — skipping upload"
    [[ -f "${SEC_KEY}" ]] && shred -u "${SEC_KEY}"
else
    info "[4] Uploading private key to SSM: ${SSM_PARAM}"
    if $DRY_RUN; then
        echo "  [dry-run] aws ssm put-parameter --name ${SSM_PARAM} --type SecureString ..."
    else
        aws ssm put-parameter \
            --name "${SSM_PARAM}" \
            --type SecureString \
            --value "$(cat "${SEC_KEY}")" \
            --description "${SSM_DESC}"
        ok "[4] Private key stored in SSM"
        shred -u "${SEC_KEY}"
        ok "[4] Private key shredded"
    fi
fi

# ════════════════════════════════════════════════════════════════════════════
# Step 5 — GitHub OIDC identity provider + IAM role
# ════════════════════════════════════════════════════════════════════════════

# OIDC provider
if ! $DRY_RUN && aws iam get-open-id-connect-provider \
        --open-id-connect-provider-arn "${OIDC_ARN}" &>/dev/null 2>&1; then
    ok "[5] GitHub OIDC provider already exists"
else
    info "[5] Registering GitHub OIDC identity provider"
    if $DRY_RUN; then
        echo "  [dry-run] aws iam create-open-id-connect-provider --url ${OIDC_URL} ..."
    else
        aws iam create-open-id-connect-provider \
            --url "${OIDC_URL}" \
            --client-id-list sts.amazonaws.com \
            --thumbprint-list "${OIDC_THUMBPRINT}" \
            2>/dev/null \
            || warn "[5] OIDC provider already exists (concurrent call), continuing"
        ok "[5] OIDC identity provider registered"
    fi
fi

# IAM role
if ! $DRY_RUN && aws iam get-role --role-name "${IAM_ROLE_NAME}" &>/dev/null 2>&1; then
    ok "[5] IAM role ${IAM_ROLE_NAME} already exists"
    ROLE_ARN="$(aws iam get-role --role-name "${IAM_ROLE_NAME}" --query Role.Arn --output text)"
else
    info "[5] Creating IAM role: ${IAM_ROLE_NAME}"
    TRUST_FILE="$(mktemp /tmp/foundry-apt-trust-XXXXXX.json)"
    POLICY_FILE="$(mktemp /tmp/foundry-apt-policy-XXXXXX.json)"

    cat > "${TRUST_FILE}" <<EOF
{
  "Version": "2012-10-17",
  "Statement": [{
    "Effect": "Allow",
    "Principal": { "Federated": "${OIDC_ARN}" },
    "Action": "sts:AssumeRoleWithWebIdentity",
    "Condition": {
      "StringEquals": {
        "token.actions.githubusercontent.com:aud": "sts.amazonaws.com",
        "token.actions.githubusercontent.com:sub": "repo:${GH_REPO}:ref:refs/tags/v*"
      }
    }
  }]
}
EOF

    cat > "${POLICY_FILE}" <<'EOF'
{
  "Version": "2012-10-17",
  "Statement": [{
    "Effect": "Allow",
    "Action": ["ssm:GetParameter", "kms:Decrypt"],
    "Resource": "arn:aws:ssm:*:*:parameter/foundry-apt/*"
  }]
}
EOF

    if $DRY_RUN; then
        echo "  [dry-run] aws iam create-role --role-name ${IAM_ROLE_NAME} ..."
        echo "  [dry-run] aws iam put-role-policy --policy-name ${IAM_POLICY_NAME} ..."
        ROLE_ARN="arn:aws:iam::${AWS_ACCOUNT_ID}:role/${IAM_ROLE_NAME}"
    else
        aws iam create-role \
            --role-name "${IAM_ROLE_NAME}" \
            --assume-role-policy-document "file://${TRUST_FILE}" \
            --description "OIDC role — ${GH_REPO} tag-push CI only"
        aws iam put-role-policy \
            --role-name "${IAM_ROLE_NAME}" \
            --policy-name "${IAM_POLICY_NAME}" \
            --policy-document "file://${POLICY_FILE}"
        ROLE_ARN="$(aws iam get-role --role-name "${IAM_ROLE_NAME}" --query Role.Arn --output text)"
        ok "[5] IAM role created: ${ROLE_ARN}"
    fi
fi

# ════════════════════════════════════════════════════════════════════════════
# Step 6 — Cloudflare R2 bucket + scoped CI token
# ════════════════════════════════════════════════════════════════════════════

# Create bucket (idempotent: 409 on duplicate is fine)
if $DRY_RUN; then
    echo "  [dry-run] POST /accounts/${CF_ACCOUNT_ID}/r2/buckets {name: ${R2_BUCKET}}"
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
        die "[6] Unexpected bucket create response: $(echo "${BUCKET_RESPONSE}" | jq -c '.errors')"
    fi
fi

# Enable r2.dev subdomain; capture the public hostname
info "[6] Enabling r2.dev subdomain"
if $DRY_RUN; then
    R2_DEV_HOSTNAME="pub-dry-run.r2.dev"
    echo "  [dry-run] PUT /r2/buckets/${R2_BUCKET}/domains/managed {enabled: true}"
else
    R2_MANAGED=$(cf_api PUT \
        "/accounts/${CF_ACCOUNT_ID}/r2/buckets/${R2_BUCKET}/domains/managed" \
        -d '{"enabled":true}')
    R2_DEV_HOSTNAME="$(echo "${R2_MANAGED}" | jq -r '.result.domain // .result // empty' \
        | grep -o '[a-zA-Z0-9-]*\.r2\.dev' | head -1 || true)"
    if [[ -z "${R2_DEV_HOSTNAME}" ]]; then
        # Fall back to GET if the PUT response doesn't include the hostname
        R2_MANAGED_GET=$(cf_api GET \
            "/accounts/${CF_ACCOUNT_ID}/r2/buckets/${R2_BUCKET}/domains/managed")
        R2_DEV_HOSTNAME="$(echo "${R2_MANAGED_GET}" | jq -r '.result.domain // empty' \
            | grep -o '[a-zA-Z0-9-]*\.r2\.dev' | head -1 || true)"
    fi
    [[ -n "${R2_DEV_HOSTNAME}" ]] \
        || die "[6] Could not determine r2.dev hostname — check the Cloudflare dashboard"
    ok "[6] r2.dev hostname: ${R2_DEV_HOSTNAME}"
fi

# Create scoped R2 CI token (value shown once — capture it now)
EXISTING_TOKEN_ID=$(
    if $DRY_RUN; then echo ""; else
        cf_api GET "/user/tokens" \
            | jq -r ".result[] | select(.name == \"${R2_TOKEN_NAME}\") | .id" || true
    fi
)

if [[ -n "${EXISTING_TOKEN_ID}" ]]; then
    warn "[6] R2 CI token '${R2_TOKEN_NAME}' already exists (ID: ${EXISTING_TOKEN_ID})"
    warn "    The secret value is gone. If GitHub secrets aren't set, delete this"
    warn "    token at https://dash.cloudflare.com/profile/api-tokens and re-run."
    R2_ACCESS_KEY_ID="${EXISTING_TOKEN_ID}"
    R2_SECRET_ACCESS_KEY=""
else
    info "[6] Looking up R2 write permission group"
    if $DRY_RUN; then
        R2_WRITE_ID="DRY_RUN_R2_WRITE_ID"
    else
        R2_WRITE_ID=$(cf_api GET "/user/tokens/permission_groups" \
            | jq -r '.result[] | select(.name | test("R2.*Storage.*Bucket.*Item.*Write"; "i")) | .id' \
            | head -1 || true)
        [[ -n "${R2_WRITE_ID}" ]] \
            || die "[6] Could not find R2 write permission group ID"
    fi

    info "[6] Creating scoped R2 CI token: ${R2_TOKEN_NAME}"
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

# CNAME record
CNAME_EXISTS=""
if ! $DRY_RUN; then
    CNAME_EXISTS=$(cf_api GET \
        "/zones/${CF_ZONE_ID}/dns/records?type=CNAME&name=${DNS_CNAME}.${ZONE_NAME:-foundrylinux.org}" \
        | jq -r '.result[0].id // empty' || true)
fi

if [[ -n "${CNAME_EXISTS}" ]]; then
    ok "[7] DNS CNAME ${DNS_CNAME}.foundrylinux.org already exists"
else
    info "[7] Creating DNS CNAME: ${DNS_CNAME}.foundrylinux.org → ${R2_DEV_HOSTNAME}"
    if $DRY_RUN; then
        echo "  [dry-run] POST /zones/.../dns/records {type: CNAME, name: ${DNS_CNAME}, content: ${R2_DEV_HOSTNAME}}"
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

# Attach custom domain
info "[7] Attaching custom domain ${CUSTOM_DOMAIN} to R2 bucket"
if $DRY_RUN; then
    echo "  [dry-run] PUT /r2/buckets/${R2_BUCKET}/domains/custom {domains: [{domain: ${CUSTOM_DOMAIN}}]}"
else
    cf_api PUT \
        "/accounts/${CF_ACCOUNT_ID}/r2/buckets/${R2_BUCKET}/domains/custom" \
        -d "{\"domains\":[{\"domain\":\"${CUSTOM_DOMAIN}\",\"enabled\":true}]}" >/dev/null
    ok "[7] Custom domain attached: ${CUSTOM_DOMAIN}"
fi

# ════════════════════════════════════════════════════════════════════════════
# Step 8 — Upload public signing key to R2, shred local copy
# ════════════════════════════════════════════════════════════════════════════

# Check if key.gpg is already live (covers re-run case)
KEY_LIVE=false
if ! $DRY_RUN && curl -fsSL "https://${CUSTOM_DOMAIN}/key.gpg" \
        | gpg --show-keys &>/dev/null 2>&1; then
    KEY_LIVE=true
fi

if $KEY_LIVE; then
    ok "[8] key.gpg already reachable at https://${CUSTOM_DOMAIN}/key.gpg — skipping upload"
    [[ -f "${PUB_KEY}" ]] && shred -u "${PUB_KEY}"
else
    info "[8] Uploading public key to s3://${R2_BUCKET}/key.gpg"
    if $DRY_RUN; then
        echo "  [dry-run] aws s3 cp ${PUB_KEY} s3://${R2_BUCKET}/key.gpg --endpoint-url ${R2_ENDPOINT}"
    else
        [[ -f "${PUB_KEY}" ]] \
            || die "[8] Public key not found at ${PUB_KEY} — re-export: gpg --armor --export ${KEY_EMAIL} > ${PUB_KEY}"
        [[ -n "${R2_SECRET_ACCESS_KEY}" ]] \
            || die "[8] R2_SECRET_ACCESS_KEY is empty — see the warning above about the existing CI token"
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
# Step 9 — Set GitHub Actions secrets on foundry-linux/foundry-apt
# ════════════════════════════════════════════════════════════════════════════

info "[9] Setting GitHub Actions secrets on ${GH_REPO}"

# Resolve ROLE_ARN if we didn't create it this run (idempotent re-run)
if [[ -z "${ROLE_ARN}" ]] && ! $DRY_RUN; then
    ROLE_ARN="$(aws iam get-role --role-name "${IAM_ROLE_NAME}" --query Role.Arn --output text)"
fi

if $DRY_RUN; then
    echo "  [dry-run] gh secret set AWS_ROLE_ARN         --repo ${GH_REPO}"
    echo "  [dry-run] gh secret set R2_ACCESS_KEY_ID     --repo ${GH_REPO}"
    echo "  [dry-run] gh secret set R2_SECRET_ACCESS_KEY --repo ${GH_REPO}"
    echo "  [dry-run] gh secret set R2_ENDPOINT          --repo ${GH_REPO}"
else
    [[ -n "${R2_SECRET_ACCESS_KEY}" ]] \
        || die "[9] R2_SECRET_ACCESS_KEY is empty — see warning above for fix"
    gh secret set AWS_ROLE_ARN         --repo "${GH_REPO}" --body "${ROLE_ARN}"
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
ok "Steps 3–9 complete."
echo ""
info "Step 10 — push the first release tag from your foundry-apt checkout:"
info "  gh repo clone ${GH_REPO} /tmp/foundry-apt-release"
info "  git -C /tmp/foundry-apt-release tag v0.0.1"
info "  git -C /tmp/foundry-apt-release push origin v0.0.1"
info "  # Watch: https://github.com/${GH_REPO}/actions"
