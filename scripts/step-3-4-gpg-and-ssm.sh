#!/usr/bin/env bash
# Generate the GPG signing key and store the private key in AWS SSM.
# Phase 1 bootstrap Steps 3 & 4 — run once from any trusted machine.
#
# Usage:
#   bash scripts/step-3-4-gpg-and-ssm.sh [--dry-run] [-h]
#
# Prerequisites:
#   gpg installed (gnupg2)
#   aws CLI installed and authenticated with permission to ssm:PutParameter
#
# Outputs (kept for later steps):
#   /tmp/foundry-packages.pub.gpg   — public key (needed for Step 8: upload to R2)
#
# The private key export is shredded immediately after SSM upload.

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
DRY_RUN=false

# ── helpers ──────────────────────────────────────────────────────────────────

info() { echo "  [info]  $*"; }
ok()   { echo "  [ok]    $*"; }
err()  { echo "  [error] $*" >&2; }
die()  { err "$*"; exit 1; }

usage() {
    sed -n '2,9p' "$0" | sed 's/^# //'
    exit 0
}

run() {
    if $DRY_RUN; then
        echo "  [dry-run] $*"
    else
        "$@"
    fi
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

command -v gpg  &>/dev/null || die "gpg not found — install gnupg2"
command -v aws  &>/dev/null || die "aws CLI not found"
command -v shred &>/dev/null || die "shred not found (util-linux)"

if ! $DRY_RUN; then
    aws sts get-caller-identity &>/dev/null || die "AWS credentials not configured — run: aws configure"
fi

# ── idempotency checks ────────────────────────────────────────────────────────

KEY_EXISTS=false
SSM_EXISTS=false

if gpg --list-keys "${KEY_EMAIL}" &>/dev/null; then
    KEY_EXISTS=true
    ok "GPG key for ${KEY_EMAIL} already exists in local keyring"
fi

if ! $DRY_RUN && aws ssm get-parameter --name "${SSM_PARAM}" &>/dev/null 2>&1; then
    SSM_EXISTS=true
    ok "SSM parameter ${SSM_PARAM} already exists"
fi

if $KEY_EXISTS && $SSM_EXISTS; then
    ok "Both key and SSM parameter already present — nothing to do."
    ok "Public key is at: ${PUB_KEY} (export it again if needed)"
    if ! $KEY_EXISTS || [[ ! -f "$PUB_KEY" ]]; then
        info "Re-exporting public key to ${PUB_KEY}"
        run gpg --armor --export "${KEY_EMAIL}" > "${PUB_KEY}"
    fi
    exit 0
fi

# ── step 3: generate GPG key ──────────────────────────────────────────────────

if ! $KEY_EXISTS; then
    info "Generating ${KEY_BITS}-bit RSA signing key for ${KEY_EMAIL} (expiry: ${KEY_EXPIRY})"

    BATCH_FILE="$(mktemp /tmp/gpg-batch-XXXXXX)"
    trap 'rm -f "$BATCH_FILE"' EXIT

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
        echo "  [dry-run] batch file contents:"
        sed 's/^/              /' "${BATCH_FILE}"
    else
        gpg --batch --gen-key "${BATCH_FILE}"
        ok "Key generated"
    fi
else
    info "Skipping key generation — already in keyring"
fi

# ── export keys ───────────────────────────────────────────────────────────────

info "Exporting public key → ${PUB_KEY}"
if $DRY_RUN; then
    echo "  [dry-run] gpg --armor --export ${KEY_EMAIL} > ${PUB_KEY}"
else
    gpg --armor --export "${KEY_EMAIL}" > "${PUB_KEY}"
    ok "Public key written to ${PUB_KEY}"
fi

info "Exporting private key → ${SEC_KEY}"
if $DRY_RUN; then
    echo "  [dry-run] gpg --armor --export-secret-keys ${KEY_EMAIL} > ${SEC_KEY}"
    echo "  [dry-run] chmod 600 ${SEC_KEY}"
else
    gpg --armor --export-secret-keys "${KEY_EMAIL}" > "${SEC_KEY}"
    chmod 600 "${SEC_KEY}"
    ok "Private key written to ${SEC_KEY}"
fi

# ── step 4: store private key in SSM, then shred ─────────────────────────────

if $SSM_EXISTS; then
    info "Skipping SSM write — parameter already exists"
else
    info "Uploading private key to SSM at ${SSM_PARAM}"
    if $DRY_RUN; then
        echo "  [dry-run] aws ssm put-parameter --name ${SSM_PARAM} --type SecureString --value <private-key> --description '${SSM_DESC}'"
    else
        aws ssm put-parameter \
            --name "${SSM_PARAM}" \
            --type SecureString \
            --value "$(cat "${SEC_KEY}")" \
            --description "${SSM_DESC}"
        ok "Private key stored in SSM"
    fi
fi

info "Shredding local private key"
if $DRY_RUN; then
    echo "  [dry-run] shred -u ${SEC_KEY}"
else
    shred -u "${SEC_KEY}"
    ok "Private key shredded"
fi

# ── done ─────────────────────────────────────────────────────────────────────

echo ""
ok "Steps 3 & 4 complete."
info "Public key kept at: ${PUB_KEY}"
info "  → needed in Step 8: aws s3 cp ${PUB_KEY} s3://foundry-apt/key.gpg"
info "SSM parameter: ${SSM_PARAM}"
info "  → referenced by foundry-apt-publish IAM role (Step 5)"
