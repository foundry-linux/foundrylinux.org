#!/usr/bin/env bash
# Back up a secret to the private foundry-linux-secrets R2 bucket (disaster recovery),
# and optionally mirror it to a GitHub Actions secret — the same dual-home
# pattern bootstrap.sh uses for the GPG key and R2 tokens.
#
# The value is never passed on the command line: it's read from a hidden prompt
# (or stdin with --from-stdin), so it stays out of shell history, the process
# list, and any transcript.
#
# Usage:
#   bash scripts/backup-secret.sh <NAME> [--gh-repo <owner/repo>] [--from-stdin] [--dry-run]
#   bash scripts/backup-secret.sh FOUNDRYLINUX_DISPATCH_PAT
#   printf '%s' "$TOKEN" | bash scripts/backup-secret.sh NAME --from-stdin
#
# Credentials (CF_API_TOKEN, CF_ACCOUNT_ID) come from the environment or from
# .foundry/bootstrap.env (written by bootstrap.sh). Needs: curl, sha256sum.
# gh (authenticated) is only needed for --gh-repo.
#
# Idempotent: re-running overwrites the same object with the same value.

set -euo pipefail

REPO_ROOT="$(cd "$(dirname "$0")/.." && pwd)"

SECRETS_BUCKET="foundry-linux-secrets"
BOOTSTRAP_CACHE="${REPO_ROOT}/.foundry/bootstrap.env"

info() { echo "  [info]  $*"; }
ok()   { echo "  [ok]    $*"; }
err()  { echo "  [error] $*" >&2; }
die()  { err "$*"; exit 1; }

usage() {
    sed -n '2,20p' "$0" | sed 's/^# \{0,1\}//'
    exit 0
}

# ── args (-h short-circuits before any validation or credential access) ───────
NAME=""
GH_REPO=""
FROM_STDIN=false
DRY_RUN=false

while [[ $# -gt 0 ]]; do
    case "$1" in
        -h|--help)    usage ;;
        --gh-repo)    shift; GH_REPO="${1:-}" ;;
        --gh-repo=*)  GH_REPO="${1#*=}" ;;
        --from-stdin) FROM_STDIN=true ;;
        --dry-run)    DRY_RUN=true ;;
        -*)           die "Unknown option: $1 (try --help)" ;;
        *)            if [[ -z "$NAME" ]]; then NAME="$1"; else die "Unexpected argument: $1"; fi ;;
    esac
    shift
done

[[ -n "$NAME" ]] || die "Missing <NAME> (the secret key). Try --help."
[[ "$NAME" =~ ^[A-Za-z0-9._-]+$ ]] || die "NAME must match [A-Za-z0-9._-]+ (got: $NAME)"

# ── credentials ───────────────────────────────────────────────────────────────
if [[ -f "$BOOTSTRAP_CACHE" ]]; then
    # shellcheck source=/dev/null
    source "$BOOTSTRAP_CACHE"
fi
: "${CF_API_TOKEN:?not set — export it, or run scripts/bootstrap.sh to populate .foundry/bootstrap.env}"
: "${CF_ACCOUNT_ID:?not set — export it, or run scripts/bootstrap.sh to populate .foundry/bootstrap.env}"

R2_OBJECT="https://api.cloudflare.com/client/v4/accounts/${CF_ACCOUNT_ID}/r2/buckets/${SECRETS_BUCKET}/objects/${NAME}"

# ── read the value (never from argv) ──────────────────────────────────────────
VALUE=""
if $FROM_STDIN; then
    VALUE="$(cat)"
else
    read -rs -p "  Value for ${NAME} (hidden): " VALUE </dev/tty
    echo >&2
fi
[[ -n "$VALUE" ]] || die "Empty value — nothing to store."

local_sha="$(printf '%s' "$VALUE" | sha256sum | cut -d' ' -f1)"
local_len="$(printf '%s' "$VALUE" | wc -c | tr -d ' ')"

if $DRY_RUN; then
    info "[dry-run] PUT ${R2_OBJECT}  (${local_len} bytes, sha256 ${local_sha:0:12}…)"
    [[ -n "$GH_REPO" ]] && info "[dry-run] gh secret set ${NAME} --repo ${GH_REPO}"
    unset VALUE
    exit 0
fi

# ── store in R2 ───────────────────────────────────────────────────────────────
info "Storing ${NAME} → r2://${SECRETS_BUCKET}/${NAME} (${local_len} bytes)"
printf '%s' "$VALUE" | curl -fsSL -X PUT "$R2_OBJECT" \
    -H "Authorization: Bearer ${CF_API_TOKEN}" \
    -H "Content-Type: text/plain; charset=utf-8" \
    --data-binary @- >/dev/null || die "R2 PUT failed"

# ── verify the upload: read it back and compare (never print the value) ───────
remote_sha="$(curl -fsSL -X GET "$R2_OBJECT" -H "Authorization: Bearer ${CF_API_TOKEN}" \
    | sha256sum | cut -d' ' -f1)" || die "R2 read-back failed"
[[ "$remote_sha" == "$local_sha" ]] \
    || die "Verify FAILED: R2 copy sha256 ${remote_sha:0:12}… != local ${local_sha:0:12}…"
ok "R2 backup verified (sha256 ${local_sha:0:12}… matches, ${local_len} bytes)"

# ── optionally mirror to a GitHub Actions secret (value via stdin, not argv) ──
if [[ -n "$GH_REPO" ]]; then
    command -v gh >/dev/null || die "gh not found — needed for --gh-repo"
    if printf '%s' "$VALUE" | gh secret set "$NAME" --repo "$GH_REPO"; then
        ok "GitHub secret ${NAME} set on ${GH_REPO}"
    else
        die "gh secret set failed"
    fi
fi

unset VALUE
ok "Done — ${NAME} is in foundry-linux-secrets R2${GH_REPO:+ and set on ${GH_REPO}}."
