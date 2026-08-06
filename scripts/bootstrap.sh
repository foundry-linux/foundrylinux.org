#!/usr/bin/env bash
# Full Phase 1 bootstrap: push foundry-apt to GitHub, generate GPG key,
# wire GitHub secrets, provision R2 bucket, configure DNS, upload public key.
# Steps 1b–9 — run once from the foundrylinux.org repo root.
#
# Usage:
#   task setup                              # preferred — wraps this script
#   task setup -- --dispatch-pat-only       # re-provision one credential
#   bash scripts/bootstrap.sh [--dry-run] [--dispatch-pat-only] [-h]
#
# --dispatch-pat-only is a *credential upgrade* on an already-bootstrapped repo:
# it prompts for FOUNDRYLINUX_DISPATCH_PAT, stores it in the private R2 secrets
# bucket, mirrors it to the wbniv/worldfoundry.org Actions secret, then exits
# without touching Steps 1c-9. Same shape as wald3n.com's --trello-only.
#
# Step 1b requires a Cloudflare operator token. If CF_API_TOKEN is not already
# exported, the script shows instructions to create it and prompts for the value.
# CF_ACCOUNT_ID and CF_ZONE_ID are fetched automatically via the API.
#
# Prerequisites:
#   gpg (gnupg2), shred, curl, jq
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
SECRETS_BUCKET="foundry-linux-secrets"
R2_TOKEN_NAME="foundry-apt-ci"
BOOTSTRAP_CACHE="${REPO_ROOT}/.foundry/bootstrap.env"
CUSTOM_DOMAIN="apt.foundrylinux.org"
DNS_CNAME="apt"
CF_OPERATOR_TOKEN_NAME="foundry-operator"
CF_ZONE_NAME="foundrylinux.org"

DRY_RUN=false
DISPATCH_PAT_ONLY=false

# The cross-repo dispatch PAT: apt.worldfoundry.org's publish uses it to ping
# foundrylinux.org so the packages page (which renders BOTH apt repos) rebuilds.
DISPATCH_PAT_NAME="FOUNDRYLINUX_DISPATCH_PAT"
DISPATCH_PAT_REPO="wbniv/worldfoundry.org"

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
    # Strip "# " from text lines and bare "#" from blank separator lines —
    # 's/^# //' alone leaves a stray '#' on every blank comment line.
    sed -n '2,21p' "$0" | sed 's/^#\( \|$\)//'
    exit 0
}

cleanup() {
    [[ -n "${WORK_DIR}"   && -d "${WORK_DIR}"   ]] && rm -rf "${WORK_DIR}"   || true
    [[ -n "${BATCH_FILE}" && -f "${BATCH_FILE}" ]] && rm -f  "${BATCH_FILE}" || true
}
trap cleanup EXIT

mkdir -p "$(dirname "$BOOTSTRAP_CACHE")"

cache_set() {
    local key="$1" val="$2"
    { grep -v "^${key}=" "$BOOTSTRAP_CACHE" 2>/dev/null || true
      printf '%s=%q\n' "$key" "$val"
    } > "${BOOTSTRAP_CACHE}.tmp" && mv "${BOOTSTRAP_CACHE}.tmp" "$BOOTSTRAP_CACHE"
    chmod 600 "$BOOTSTRAP_CACHE"
}

# Cloudflare API wrapper. On success, the response body goes to stdout exactly
# as before. On failure it reports the HTTP status and Cloudflare's own error
# messages to stderr, rather than dying as a bare "curl: (22) ... error: 400"
# with the diagnosis thrown away — the same swallowed-error-body problem the
# cross-repo dispatch had. Callers that only want a boolean still use
# `&>/dev/null`, which suppresses this too.
cf_api() {
    local method="$1" path="$2"
    shift 2
    local response status body
    response=$(curl -sS -w $'\n%{http_code}' -X "$method" \
        "https://api.cloudflare.com/client/v4${path}" \
        -H "Authorization: Bearer ${CF_API_TOKEN:-}" \
        -H "Content-Type: application/json" \
        "$@") || { err "cf_api: curl transport failure on ${method} ${path}"; return 1; }

    status=${response##*$'\n'}
    body=${response%$'\n'*}

    if [[ "$status" != 2* ]]; then
        err "Cloudflare API ${method} ${path} → HTTP ${status}"
        printf '%s' "$body" \
            | jq -r '.errors[]? | "  [\(.code)] \(.message)"' 2>/dev/null >&2 \
            || printf '  %s\n' "$body" >&2
        return 1
    fi

    printf '%s' "$body"
}

# Store a secret in the private foundry-linux-secrets R2 bucket.
# Usage: r2_put_secret <key-name> <value>
r2_put_secret() {
    local name="$1" value="$2"
    if $DRY_RUN; then
        echo "  [dry-run] PUT r2://${SECRETS_BUCKET}/${name}"
        return
    fi
    printf '%s' "${value}" | curl -fsSL -X PUT \
        "https://api.cloudflare.com/client/v4/accounts/${CF_ACCOUNT_ID}/r2/buckets/${SECRETS_BUCKET}/objects/${name}" \
        -H "Authorization: Bearer ${CF_API_TOKEN}" \
        -H "Content-Type: text/plain; charset=utf-8" \
        --data-binary @- \
        >/dev/null
}

# ── arg parse ────────────────────────────────────────────────────────────────

for arg in "$@"; do
    case "$arg" in
        -h|--help)             usage ;;
        --dry-run)             DRY_RUN=true ;;
        --dispatch-pat-only)   DISPATCH_PAT_ONLY=true ;;
        *)                     die "Unknown argument: $arg" ;;
    esac
done

# ── preflight ────────────────────────────────────────────────────────────────

if [[ -f "$BOOTSTRAP_CACHE" ]]; then
    # shellcheck source=/dev/null
    source "$BOOTSTRAP_CACHE"
    info "Loaded cached credentials from $BOOTSTRAP_CACHE"
fi

[[ -d "${SRC_DIR}" ]] || die "${PKG_NAME}/ not found under ${REPO_ROOT}"

command -v gpg   &>/dev/null || die "gpg not found — install gnupg2"
command -v shred &>/dev/null || die "shred not found (install util-linux)"
command -v curl  &>/dev/null || die "curl not found"
command -v jq    &>/dev/null || die "jq not found"
command -v gh    &>/dev/null || die "gh CLI not found — https://cli.github.com"

if ! $DRY_RUN; then
    gh auth status &>/dev/null || die "gh not authenticated — run: gh auth login"
    # --dispatch-pat-only needs Cloudflare ONLY for the optional R2 backup leg.
    # Never make installing a GitHub PAT block on minting a Cloudflare token:
    # use cached CF creds if they happen to be there, otherwise carry on.
    if [[ -z "${CF_API_TOKEN:-}" ]] && ! $DISPATCH_PAT_ONLY; then
        echo "  ┌─────────────────────────────────────────────────────────────┐"
        echo "  │  This prompt wants a CLOUDFLARE token — NOT a GitHub PAT.   │"
        echo "  └─────────────────────────────────────────────────────────────┘"
        echo ""
        echo "  Cloudflare operator token needed. Create '${CF_OPERATOR_TOKEN_NAME}' first:"
        echo ""
        echo "  https://dash.cloudflare.com/profile/api-tokens"
        echo "  Click '+ Create Token', then 'Get started' next to 'Create Custom Token'"
        echo "  Name: ${CF_OPERATOR_TOKEN_NAME}"
        echo "  Permissions:"
        echo "    Account | Workers R2 Storage | Edit"
        echo "    Account | Pages Write         |"
        echo "    Zone    | DNS                | Edit  (Specific zone: ${CF_ZONE_NAME})"
        echo "    Zone    | Transform Rules    | Edit  (Specific zone: ${CF_ZONE_NAME})"
        echo "  Account Resources: Include → select your account"
        echo "  Zone Resources:    Include → Specific zone → ${CF_ZONE_NAME}"
        echo ""
        # Validate BEFORE caching. Caching an unvalidated value is what let a
        # GitHub PAT pasted at this prompt (2026-08-05) persist into
        # .foundry/bootstrap.env and fail later with an opaque "curl: (22) 400",
        # having already been transmitted to Cloudflare in an Authorization
        # header. Reject the obvious wrong-token case by shape, then prove the
        # token works against /user/tokens/verify before it is written anywhere.
        while :; do
            read -rsp "  Paste CLOUDFLARE token value (input hidden): " CF_API_TOKEN </dev/tty; echo

            if [[ -z "${CF_API_TOKEN:-}" ]]; then
                warn "Token cannot be blank — try again."
                continue
            fi
            case "$CF_API_TOKEN" in
                github_pat_*|ghp_*|gho_*|ghu_*|ghs_*)
                    CF_API_TOKEN=""
                    err "That is a GITHUB token. This prompt wants a Cloudflare token."
                    err "Nothing was stored or sent. Try again with the '${CF_OPERATOR_TOKEN_NAME}' token."
                    continue ;;
            esac

            info "Verifying token against Cloudflare..."
            if cf_api GET "/user/tokens/verify" >/dev/null 2>&1; then
                ok "Token verified."
                break
            fi
            CF_API_TOKEN=""
            err "Cloudflare rejected that token. Check you copied the '${CF_OPERATOR_TOKEN_NAME}'"
            err "token from https://dash.cloudflare.com/profile/api-tokens — try again."
        done

        export CF_API_TOKEN
        cache_set CF_API_TOKEN "$CF_API_TOKEN"
    fi
fi

R2_ACCESS_KEY_ID="${R2_ACCESS_KEY_ID:-}"
R2_SECRET_ACCESS_KEY="${R2_SECRET_ACCESS_KEY:-}"
R2_DEV_HOSTNAME=""

echo ""
if $DISPATCH_PAT_ONLY; then
    info "Credential upgrade only — Steps 1b–9 will be skipped"
else
    info "Bootstrap: Steps 1b–9 for ${GH_REPO}"
fi
echo ""

# ════════════════════════════════════════════════════════════════════════════
# Credential upgrade — --dispatch-pat-only runs and exits HERE
# ════════════════════════════════════════════════════════════════════════════
# Deliberately ahead of Step 1b: resolving a Cloudflare account id is only
# needed for the optional R2 backup, and requiring a Cloudflare token in order
# to install a GitHub PAT is a coupling with no justification.
#
# On the R2 leg being optional here specifically: unlike the GPG signing key,
# this PAT is re-mintable in under a minute and expires on a schedule anyway, so
# a backup copy has almost no recovery value — you cannot restore your way out
# of an expired token. That reasoning does NOT generalise to the other secrets
# this script handles, which stay dual-homed.

if $DISPATCH_PAT_ONLY; then
    echo "  Create a fine-grained PAT at:"
    echo "  https://github.com/settings/personal-access-tokens/new"
    echo ""
    echo "    Token name:      ${DISPATCH_PAT_NAME} (${DISPATCH_PAT_REPO})"
    echo "                     — this is what the expiry-warning email shows, so name it"
    echo "                       after the secret and the repo that HOLDS it, not the target."
    echo "    Resource owner:  ${GH_ORG}          (the org — NOT wbniv)"
    echo "    Repository:      Only select repositories → ${GH_ORG}/foundrylinux.org"
    echo "    Permissions:     Contents: Read and write   (Metadata: Read is implicit)"
    echo "                     — NOT Actions; POST /dispatches is gated on Contents."
    echo "    Expiration:      1 year"
    echo ""

    if $DRY_RUN; then
        echo "  [dry-run] prompt for ${DISPATCH_PAT_NAME}"
        echo "  [dry-run] gh secret set ${DISPATCH_PAT_NAME} --repo ${DISPATCH_PAT_REPO}"
        echo "  [dry-run] R2 backup: only if CF_API_TOKEN + CF_ACCOUNT_ID are already cached"
        exit 0
    fi

    if [[ -n "${CF_API_TOKEN:-}" && -n "${CF_ACCOUNT_ID:-}" ]]; then
        # Cloudflare creds already cached — take the dual-homed path, which also
        # verifies the R2 copy by sha256 read-back.
        info "Cloudflare creds cached — storing in R2 and mirroring to GitHub."
        bash "${REPO_ROOT}/scripts/backup-secret.sh" \
            "${DISPATCH_PAT_NAME}" --gh-repo "${DISPATCH_PAT_REPO}" \
            || die "Failed to store ${DISPATCH_PAT_NAME}"
    else
        # No Cloudflare creds: set the GitHub secret and say plainly what was skipped.
        info "No Cloudflare creds cached — setting the GitHub secret only."
        echo ""
        PAT_VALUE=""
        while [[ -z "$PAT_VALUE" ]]; do
            read -rsp "  Paste GITHUB PAT value (input hidden): " PAT_VALUE </dev/tty; echo
            [[ -z "$PAT_VALUE" ]] && warn "Value cannot be blank — try again."
        done

        printf '%s' "$PAT_VALUE" \
            | gh secret set "${DISPATCH_PAT_NAME}" --repo "${DISPATCH_PAT_REPO}" \
            || { unset PAT_VALUE; die "gh secret set failed"; }
        unset PAT_VALUE
        ok "GitHub secret ${DISPATCH_PAT_NAME} set on ${DISPATCH_PAT_REPO}"
        warn "R2 backup skipped (no Cloudflare creds). To add it later:"
        warn "  task secret-set NAME=${DISPATCH_PAT_NAME} REPO=${DISPATCH_PAT_REPO}"
    fi

    echo ""
    ok "Done. Verify the leg fires:"
    echo "    gh workflow run notify-foundrylinux.yml --repo ${DISPATCH_PAT_REPO}"
    echo "    gh run list --repo ${DISPATCH_PAT_REPO} --workflow notify-foundrylinux.yml --limit 1"
    echo "    gh run list --repo ${GH_ORG}/foundrylinux.org --workflow site-deploy.yml --limit 1"
    echo "      → expect a run with event 'repository_dispatch'"
    exit 0
fi

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

    # Validate all required permissions upfront before any destructive steps
    info "[1b] Validating token permissions..."
    PERM_ERRORS=()
    cf_api GET "/accounts/${CF_ACCOUNT_ID}/r2/buckets?per_page=1" &>/dev/null \
        || PERM_ERRORS+=("  Account | Workers R2 Storage | Edit")
    cf_api GET "/zones/${CF_ZONE_ID}/dns_records?per_page=1" &>/dev/null \
        || PERM_ERRORS+=("  Zone    | DNS                | Edit  (zone: ${CF_ZONE_NAME})")
    cf_api GET "/zones/${CF_ZONE_ID}/rulesets" &>/dev/null \
        || PERM_ERRORS+=("  Zone    | Transform Rules    | Edit  (zone: ${CF_ZONE_NAME})")
    if [[ ${#PERM_ERRORS[@]} -gt 0 ]]; then
        err "[1b] Token is missing required permissions:"
        for e in "${PERM_ERRORS[@]}"; do err "$e"; done
        err ""
        err "  Edit your token at: https://dash.cloudflare.com/profile/api-tokens"
        die "[1b] Fix token permissions and re-run."
    fi
    ok "[1b] Token permissions verified"
fi

R2_ENDPOINT="https://${CF_ACCOUNT_ID:-DRY_RUN}.r2.cloudflarestorage.com"

# ════════════════════════════════════════════════════════════════════════════
# Step 1c — Create private secrets bucket and store operator token
# ════════════════════════════════════════════════════════════════════════════

info "[1c] Ensuring private secrets bucket '${SECRETS_BUCKET}' exists"
if $DRY_RUN; then
    echo "  [dry-run] POST /accounts/.../r2/buckets {name: ${SECRETS_BUCKET}}"
    echo "  [dry-run] PUT r2://${SECRETS_BUCKET}/CF_API_TOKEN"
else
    SEC_BUCKET_RESP=$(curl -sS -X POST \
        "https://api.cloudflare.com/client/v4/accounts/${CF_ACCOUNT_ID}/r2/buckets" \
        -H "Authorization: Bearer ${CF_API_TOKEN}" \
        -H "Content-Type: application/json" \
        -d "$(jq -n --arg n "$SECRETS_BUCKET" '{name:$n,locationHint:"auto"}')")
    if echo "${SEC_BUCKET_RESP}" | jq -e '.success == true' &>/dev/null; then
        ok "[1c] Secrets bucket '${SECRETS_BUCKET}' created (no public access)"
    elif echo "${SEC_BUCKET_RESP}" | jq -r '.errors[].code' 2>/dev/null | grep -qE "10004|10006"; then
        ok "[1c] Secrets bucket '${SECRETS_BUCKET}' already exists"
    else
        die "[1c] Unexpected bucket response: $(echo "${SEC_BUCKET_RESP}" | jq -c '.errors')"
    fi
    r2_put_secret "CF_API_TOKEN" "${CF_API_TOKEN}"
    ok "[1c] CF_API_TOKEN stored in r2://${SECRETS_BUCKET}/CF_API_TOKEN"
fi

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
    if ! $DRY_RUN && [[ -f "${SEC_KEY}" ]]; then
        r2_put_secret "GPG_PRIVATE_KEY" "$(cat "${SEC_KEY}")"
        ok "[4] GPG_PRIVATE_KEY stored in r2://${SECRETS_BUCKET}/GPG_PRIVATE_KEY"
        shred -u "${SEC_KEY}"
    fi
else
    info "[4] Setting GPG_PRIVATE_KEY secret on ${GH_REPO}"
    if $DRY_RUN; then
        echo "  [dry-run] gh secret set GPG_PRIVATE_KEY --repo ${GH_REPO} --body <private-key>"
        echo "  [dry-run] PUT r2://${SECRETS_BUCKET}/GPG_PRIVATE_KEY"
        echo "  [dry-run] shred -u ${SEC_KEY}"
    else
        gh secret set GPG_PRIVATE_KEY --repo "${GH_REPO}" --body "$(cat "${SEC_KEY}")"
        ok "[4] GPG_PRIVATE_KEY secret set"
        r2_put_secret "GPG_PRIVATE_KEY" "$(cat "${SEC_KEY}")"
        ok "[4] GPG_PRIVATE_KEY stored in r2://${SECRETS_BUCKET}/GPG_PRIVATE_KEY"
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
        -d "$(jq -n --arg n "$R2_BUCKET" '{name:$n,locationHint:"auto"}')")
    if echo "${BUCKET_RESPONSE}" | jq -e '.success == true' &>/dev/null; then
        ok "[6] R2 bucket '${R2_BUCKET}' created"
    elif echo "${BUCKET_RESPONSE}" | jq -r '.errors[].code' 2>/dev/null | grep -qE "10004|10006"; then
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
        -d '{"enabled":true}' 2>/dev/null || true)
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

info "[6] R2 CI credentials needed (R2 S3 tokens must be created in the R2 dashboard)"
if $DRY_RUN; then
    R2_ACCESS_KEY_ID="DRY_RUN_KEY_ID"
    R2_SECRET_ACCESS_KEY="DRY_RUN_SECRET"
elif [[ -z "${R2_ACCESS_KEY_ID:-}" || -z "${R2_SECRET_ACCESS_KEY:-}" ]]; then
    echo ""
    echo "  Create an R2 API token at:"
    echo "  https://dash.cloudflare.com/${CF_ACCOUNT_ID}/r2/api-tokens"
    echo ""
    echo "  Click 'Create Account API token' (not User — Account tokens survive org changes)"
    echo "    Token name:  ${R2_TOKEN_NAME}"
    echo "    Permissions: Object Read & Write"
    echo "    Bucket:      Apply to specific bucket → ${R2_BUCKET}"
    echo ""
    echo "  Under 'Use the following credentials for S3 clients':"
    until [[ -n "${R2_ACCESS_KEY_ID:-}" ]]; do
        read -rsp "  Paste Access Key ID (input hidden): " R2_ACCESS_KEY_ID; echo
        [[ -z "${R2_ACCESS_KEY_ID:-}" ]] && echo "  (cannot be blank — try again)"
    done
    until [[ -n "${R2_SECRET_ACCESS_KEY:-}" ]]; do
        read -rsp "  Paste Secret Access Key (input hidden): " R2_SECRET_ACCESS_KEY; echo
        [[ -z "${R2_SECRET_ACCESS_KEY:-}" ]] && echo "  (cannot be blank — try again)"
    done
    export R2_ACCESS_KEY_ID R2_SECRET_ACCESS_KEY
    cache_set R2_ACCESS_KEY_ID     "$R2_ACCESS_KEY_ID"
    cache_set R2_SECRET_ACCESS_KEY "$R2_SECRET_ACCESS_KEY"
    ok "[6] R2 credentials captured"
fi

if ! $DRY_RUN && [[ -n "${R2_ACCESS_KEY_ID}" && -n "${R2_SECRET_ACCESS_KEY}" ]]; then
    r2_put_secret "R2_ACCESS_KEY_ID"     "${R2_ACCESS_KEY_ID}"
    r2_put_secret "R2_SECRET_ACCESS_KEY" "${R2_SECRET_ACCESS_KEY}"
    ok "[6] R2 credentials stored in r2://${SECRETS_BUCKET}/R2_ACCESS_KEY_ID and R2_SECRET_ACCESS_KEY"
fi

# ════════════════════════════════════════════════════════════════════════════
# Step 7 — DNS CNAME + attach custom domain to R2 bucket
# ════════════════════════════════════════════════════════════════════════════

CNAME_EXISTS=""
if ! $DRY_RUN; then
    CNAME_EXISTS=$(cf_api GET \
        "/zones/${CF_ZONE_ID}/dns_records?type=CNAME&name=${DNS_CNAME}.foundrylinux.org" \
        | jq -r '.result[0].id // empty' || true)
fi

if [[ -n "${CNAME_EXISTS}" ]]; then
    ok "[7] DNS CNAME ${DNS_CNAME}.foundrylinux.org already exists"
else
    info "[7] Creating DNS CNAME: ${DNS_CNAME}.foundrylinux.org → ${R2_DEV_HOSTNAME}"
    if $DRY_RUN; then
        echo "  [dry-run] POST /zones/.../dns_records {type: CNAME, name: ${DNS_CNAME}}"
    else
        cf_api POST "/zones/${CF_ZONE_ID}/dns_records" -d "$(jq -n \
            --arg name    "$DNS_CNAME" \
            --arg content "$R2_DEV_HOSTNAME" \
            '{type:"CNAME",name:$name,content:$content,proxied:true,comment:"foundry-apt R2 bucket"}')" >/dev/null
        ok "[7] DNS CNAME created"
    fi
fi

info "[7] Attaching custom domain ${CUSTOM_DOMAIN} to R2 bucket"
if $DRY_RUN; then
    echo "  [dry-run] POST /r2/buckets/${R2_BUCKET}/domains/custom"
else
    DOMAIN_HTTP=$(curl -sS -o /dev/null -w "%{http_code}" -X POST \
        "https://api.cloudflare.com/client/v4/accounts/${CF_ACCOUNT_ID}/r2/buckets/${R2_BUCKET}/domains/custom" \
        -H "Authorization: Bearer ${CF_API_TOKEN}" \
        -H "Content-Type: application/json" \
        -d "$(jq -n --arg d "$CUSTOM_DOMAIN" --arg z "$CF_ZONE_ID" '{domain:$d,zoneId:$z,enabled:true}')")
    if [[ "${DOMAIN_HTTP}" == "200" ]]; then
        ok "[7] Custom domain attached: ${CUSTOM_DOMAIN}"
    elif [[ "${DOMAIN_HTTP}" == "409" ]]; then
        ok "[7] Custom domain already attached: ${CUSTOM_DOMAIN}"
    else
        die "[7] Unexpected HTTP ${DOMAIN_HTTP} attaching custom domain — check Cloudflare dashboard"
    fi
fi

# ════════════════════════════════════════════════════════════════════════════
# Step 7.5 — Cloudflare Redirect Rule: / → /index.html
# ════════════════════════════════════════════════════════════════════════════

info "[7.5] Creating URL rewrite rule: ${CUSTOM_DOMAIN}/ → /index.html"

# http_request_redirect is not available on the free plan; http_request_transform
# (URL rewrite) is — it serves index.html transparently without a 301 round-trip.
REWRITE_EXPR="(http.host eq \"${CUSTOM_DOMAIN}\" and http.request.uri.path eq \"/\")"

if $DRY_RUN; then
    echo "  [dry-run] PUT /zones/.../rulesets/phases/http_request_transform/entrypoint"
else
    PHASE_JSON=$(cf_api GET \
        "/zones/${CF_ZONE_ID}/rulesets/phases/http_request_transform/entrypoint" \
        2>/dev/null || echo '{}')
    RULESET_ID=$(echo "$PHASE_JSON" | jq -r '.result.id // empty' 2>/dev/null || true)
    EXISTING_RULE=$(echo "$PHASE_JSON" | jq -r \
        --arg expr "$REWRITE_EXPR" \
        '.result.rules[]? | select(.expression == $expr) | .id' 2>/dev/null || true)

    RULE_BODY=$(jq -n --arg expr "$REWRITE_EXPR" \
        '{action:"rewrite",action_parameters:{uri:{path:{value:"/index.html"}}},expression:$expr,enabled:true}')

    if [[ -n "$EXISTING_RULE" ]]; then
        ok "[7.5] URL rewrite rule already exists (id: ${EXISTING_RULE})"
    elif [[ -z "$RULESET_ID" ]]; then
        RESP=$(cf_api PUT \
            "/zones/${CF_ZONE_ID}/rulesets/phases/http_request_transform/entrypoint" \
            -d "$(jq -n --argjson rule "$RULE_BODY" '{name:"Zone Rewrite Rules",rules:[$rule]}')")
        echo "$RESP" | jq -e '.success == true' &>/dev/null \
            || { err "[7.5] $(echo "$RESP" | jq -r '.errors[0].message')"; exit 1; }
        ok "[7.5] URL rewrite rule created: ${CUSTOM_DOMAIN}/ → /index.html"
    else
        RESP=$(cf_api POST "/zones/${CF_ZONE_ID}/rulesets/${RULESET_ID}/rules" -d "$RULE_BODY")
        echo "$RESP" | jq -e '.success == true' &>/dev/null \
            || { err "[7.5] $(echo "$RESP" | jq -r '.errors[0].message')"; exit 1; }
        ok "[7.5] URL rewrite rule added: ${CUSTOM_DOMAIN}/ → /index.html"
    fi
fi

# ════════════════════════════════════════════════════════════════════════════
# Step 8 — Upload public signing key to R2, shred local copy
# ════════════════════════════════════════════════════════════════════════════

KEY_LIVE=false
if ! $DRY_RUN && curl -fsSL "https://${CUSTOM_DOMAIN}/key.gpg" 2>/dev/null \
        | gpg --show-keys &>/dev/null 2>&1; then
    KEY_LIVE=true
fi

if $KEY_LIVE; then
    ok "[8] key.gpg already reachable at https://${CUSTOM_DOMAIN}/key.gpg"
    [[ -f "${PUB_KEY}" ]] && shred -u "${PUB_KEY}"
else
    info "[8] Uploading public key → r2://${R2_BUCKET}/key.gpg"
    if $DRY_RUN; then
        echo "  [dry-run] PUT /accounts/.../r2/buckets/${R2_BUCKET}/objects/key.gpg"
        echo "  [dry-run] shred -u ${PUB_KEY}"
    else
        [[ -f "${PUB_KEY}" ]] \
            || die "[8] Public key missing — re-export: gpg --armor --export ${KEY_EMAIL} > ${PUB_KEY}"
        curl -fsSL -X PUT \
            "https://api.cloudflare.com/client/v4/accounts/${CF_ACCOUNT_ID}/r2/buckets/${R2_BUCKET}/objects/key.gpg" \
            -H "Authorization: Bearer ${CF_API_TOKEN}" \
            -H "Content-Type: application/octet-stream" \
            --data-binary @"${PUB_KEY}" \
            >/dev/null
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
info "Step 10 — sync and push the first release tag to trigger CI:"
info "  task sync-and-release TAG=v0.0.1"
info "  # Watch: https://github.com/${GH_REPO}/actions"
echo ""
info "Opening repo landing page: https://${CUSTOM_DOMAIN}/"
xdg-open "https://${CUSTOM_DOMAIN}/" 2>/dev/null || open "https://${CUSTOM_DOMAIN}/" 2>/dev/null || true
