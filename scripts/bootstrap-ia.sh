#!/usr/bin/env bash
# Provision the Internet Archive S3-like API keys (access + secret) for ISO
# hosting — the dual-home credential pattern bootstrap.sh / backup-secret.sh use:
# stored in the private foundry-linux-secrets R2 bucket (disaster recovery) AND set as
# GitHub Actions secrets on the publish repo (CI use). The local copy is never
# written to disk.
#
# These feed the v1.0.0 ISO-hosting migration (archive.org backend behind the
# iso.foundrylinux.org Worker redirect). The publish workflow that consumes them
# is gated until v1.0.0 — see TODO.md and
# docs/investigations/2026-05-22-iso-hosting.md. Provisioning the keys now is the
# prerequisite; nothing reads them until the migration lands.
#
# Get/rotate keys at: https://archive.org/account/s3.php
#
# Secret names set:
#   IA_S3_ACCESS_KEY   the "S3 access key"
#   IA_S3_SECRET_KEY   the "S3 secret key"
#
# Values are never passed on the command line — read from hidden prompts (or the
# IA_S3_ACCESS_KEY / IA_S3_SECRET_KEY env vars for non-interactive runs), so they
# stay out of shell history, the process list, and any transcript.
#
# Usage:
#   bash scripts/bootstrap-ia.sh [--gh-repo <owner/repo>] [--no-verify] [--dry-run]
#   IA_S3_ACCESS_KEY=... IA_S3_SECRET_KEY=... bash scripts/bootstrap-ia.sh
#
# Needs: bash, curl, sha256sum, gh (authenticated). Cloudflare creds
# (CF_API_TOKEN, CF_ACCOUNT_ID) come from the env or .foundry/bootstrap.env,
# exactly as backup-secret.sh expects.
#
# Idempotent: re-running overwrites the same R2 objects + GitHub secrets.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BACKUP_SECRET="${SCRIPT_DIR}/backup-secret.sh"

DEFAULT_GH_REPO="foundry-linux/foundrylinux.org"
IA_S3_ENDPOINT="https://s3.us.archive.org/"

info() { echo "  [info]  $*"; }
ok()   { echo "  [ok]    $*"; }
warn() { echo "  [warn]  $*" >&2; }
err()  { echo "  [error] $*" >&2; }
die()  { err "$*"; exit 1; }

usage() {
    sed -n '2,38p' "$0" | sed 's/^# \{0,1\}//'
    exit 0
}

# ── args (-h short-circuits before any validation or credential access) ───────
GH_REPO="$DEFAULT_GH_REPO"
VERIFY=true
DRY_RUN=false

while [[ $# -gt 0 ]]; do
    case "$1" in
        -h|--help)   usage ;;
        --gh-repo)   shift; GH_REPO="${1:-}" ;;
        --gh-repo=*) GH_REPO="${1#*=}" ;;
        --no-verify) VERIFY=false ;;
        --dry-run)   DRY_RUN=true ;;
        -*)          die "Unknown option: $1 (try --help)" ;;
        *)           die "Unexpected argument: $1 (try --help)" ;;
    esac
    shift
done

[[ -n "$GH_REPO" ]] || die "--gh-repo cannot be empty"
[[ -x "$BACKUP_SECRET" || -f "$BACKUP_SECRET" ]] || die "backup-secret.sh not found at $BACKUP_SECRET"
command -v curl >/dev/null || die "curl not found"

# ── read the two keys (never from argv) ───────────────────────────────────────
# Prefer env vars (non-interactive / CI), else hidden prompt from the terminal.
ACCESS="${IA_S3_ACCESS_KEY:-}"
SECRET="${IA_S3_SECRET_KEY:-}"

if [[ -z "$ACCESS" ]]; then
    read -rs -p "  Internet Archive S3 access key (hidden): " ACCESS </dev/tty
    echo >&2
fi
if [[ -z "$SECRET" ]]; then
    read -rs -p "  Internet Archive S3 secret key (hidden): " SECRET </dev/tty
    echo >&2
fi

[[ -n "$ACCESS" ]] || die "Empty access key — nothing to store."
[[ -n "$SECRET" ]] || die "Empty secret key — nothing to store."

# ── best-effort liveness check: do these keys actually authenticate? ──────────
# IA S3 auth is a header: `authorization: LOW <access>:<secret>`. Pass it via a
# curl config on stdin so the secret never lands in argv / the process list.
# A valid pair returns 200 (ListAllMyBuckets-style XML); a bad pair returns 403.
verify_ia() {
    local code
    code="$(printf 'url = "%s"\nheader = "authorization: LOW %s:%s"\n' \
                "$IA_S3_ENDPOINT" "$ACCESS" "$SECRET" \
            | curl -sS -o /dev/null -w '%{http_code}' -K - 2>/dev/null)" || {
        warn "Could not reach ${IA_S3_ENDPOINT} to verify keys — storing anyway."
        return 0
    }
    case "$code" in
        2*)        ok "IA keys authenticated against ${IA_S3_ENDPOINT} (HTTP ${code})." ;;
        401|403)   die "IA rejected these keys (HTTP ${code}). Rotate at https://archive.org/account/s3.php and retry." ;;
        *)         warn "IA liveness check inconclusive (HTTP ${code}) — storing anyway." ;;
    esac
}

if $VERIFY && ! $DRY_RUN; then
    verify_ia
fi

# ── store both keys via backup-secret.sh (R2 backup + verified read-back + GH) ─
store() {
    local name="$1" value="$2" args=(--from-stdin --gh-repo "$GH_REPO")
    $DRY_RUN && args+=(--dry-run)
    printf '%s' "$value" | bash "$BACKUP_SECRET" "$name" "${args[@]}"
}

info "Provisioning IA keys → R2 (foundry-linux-secrets) + GitHub secrets on ${GH_REPO}"
store IA_S3_ACCESS_KEY "$ACCESS"
store IA_S3_SECRET_KEY "$SECRET"

unset ACCESS SECRET

if $DRY_RUN; then
    info "[dry-run] no changes made."
else
    ok "Done — IA_S3_ACCESS_KEY + IA_S3_SECRET_KEY are in foundry-linux-secrets R2 and set on ${GH_REPO}."
    info "Consumed by the ISO-hosting migration at v1.0.0 (held — see TODO.md)."
fi
