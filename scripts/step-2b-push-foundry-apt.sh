#!/usr/bin/env bash
# Push foundry-apt/ to a standalone GitHub repo under the Foundry Linux org.
# Phase 1 bootstrap Step 2b — run once from the linuxfoundry.org repo root.
#
# Usage:
#   bash scripts/step-2b-push-foundry-apt.sh [--dry-run] [-h]
#
# Prerequisites:
#   gh CLI installed and authenticated (gh auth login)

set -euo pipefail

REPO_ROOT="$(cd "$(dirname "$0")/.." && pwd)"

# ── config (one place to change) ─────────────────────────────────────────────

GH_ORG="foundry-linux"
PKG_NAME="foundry-apt"
GH_REPO="${GH_ORG}/${PKG_NAME}"
SRC_DIR="${REPO_ROOT}/${PKG_NAME}"
WORK_DIR="/tmp/${PKG_NAME}-push"
REPO_DESC="Foundry Linux signed APT repo and metapackages"
DRY_RUN=false

# ── helpers ──────────────────────────────────────────────────────────────────

info()  { echo "  [info]  $*"; }
ok()    { echo "  [ok]    $*"; }
err()   { echo "  [error] $*" >&2; }
die()   { err "$*"; exit 1; }

usage() {
    sed -n '2,8p' "$0" | sed 's/^# //'
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
        -h|--help)    usage ;;
        --dry-run)    DRY_RUN=true ;;
        *)            die "Unknown argument: $arg" ;;
    esac
done

# ── preflight ────────────────────────────────────────────────────────────────

[[ -d "$SRC_DIR" ]] || die "${PKG_NAME}/ not found under ${REPO_ROOT}"

command -v gh &>/dev/null || die "gh CLI not found — install it first: https://cli.github.com"
gh auth status &>/dev/null || die "gh not authenticated — run: gh auth login"

# Idempotency check
if gh repo view "$GH_REPO" &>/dev/null; then
    ok "${GH_REPO} already exists on GitHub — nothing to do."
    exit 0
fi

# ── create standalone repo ───────────────────────────────────────────────────

info "Copying ${SRC_DIR} → ${WORK_DIR}"
run rm -rf "$WORK_DIR"
run cp -r "$SRC_DIR" "$WORK_DIR"

if ! $DRY_RUN; then
    trap 'rm -rf "$WORK_DIR"' EXIT

    cd "$WORK_DIR"
    git init -q
    git add .
    git commit -q -m "feat: initial ${PKG_NAME} import"

    info "Creating https://github.com/${GH_REPO} and pushing"
    gh repo create "$GH_REPO" \
        --public \
        --description "$REPO_DESC" \
        --source=. --remote=origin --push

    info "Enabling Discussions"
    gh repo edit "$GH_REPO" --enable-discussions
else
    echo "  [dry-run] cd ${WORK_DIR}"
    echo "  [dry-run] git init && git add . && git commit -m 'feat: initial ${PKG_NAME} import'"
    echo "  [dry-run] gh repo create ${GH_REPO} --public --description '${REPO_DESC}' --source=. --push"
    echo "  [dry-run] gh repo edit ${GH_REPO} --enable-discussions"
    echo "  [dry-run] rm -rf ${WORK_DIR}"
fi

ok "Done — https://github.com/${GH_REPO}"
