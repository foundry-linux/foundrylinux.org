#!/usr/bin/env bash
# Add apt.foundrylinux.org to /etc/apt/sources.list.d. Idempotent.
#
# Once this runs, `apt install foundry-*` and the vendored
# packages (f9dasm, libvgm, future vgmstream/ghidra) resolve from
# Foundry's apt repo.

set -euo pipefail

for arg in "$@"; do
    case "$arg" in
        -h|--help)
            cat <<EOF
Configure apt.foundrylinux.org as an apt source.

Usage: $(basename "$0") [-h|--help] [--dry-run|-n]

Idempotent — safe to re-run. Skips if the keyring + sources.list.d
entry already exist.
EOF
            exit 0
            ;;
    esac
done

DRY_RUN=false
for arg in "$@"; do
    case "$arg" in
        -n|--dry-run) DRY_RUN=true ;;
        -h|--help) ;;
        *) echo "Unknown option: $arg (try --help)" >&2; exit 1 ;;
    esac
done

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
if [[ -f "$SCRIPT_DIR/lib.sh" ]]; then
    # shellcheck source=lib.sh
    source "$SCRIPT_DIR/lib.sh"
else
    info()    { echo "ℹ $*"; }
    ok()      { echo "✓ $*"; }
    warn()    { echo "⚠ $*"; }
    die()     { echo "✗ $*" >&2; exit 1; }
    step()    { echo; echo "━━━ $* ━━━"; }
    run_sudo(){ if $DRY_RUN; then echo "  [dry-run] sudo $*"; else sudo "$@"; fi; }
    apt_update() {
        if $DRY_RUN; then echo "  [dry-run] sudo apt-get update -q"
        else sudo apt-get update -q 2>&1 || echo "⚠ apt-get update had errors; continuing"; fi
    }
fi

KEYRING=/etc/apt/keyrings/foundry.gpg
LIST=/etc/apt/sources.list.d/foundry.list
URL=https://apt.foundrylinux.org
SUITE=resolute

if [[ -f "$KEYRING" && -f "$LIST" ]]; then
    info "Foundry apt source already configured at $LIST — skipping"
    exit 0
fi

step "Configuring $URL as an apt source"
# gpg --dearmor needs gnupg. A real Ubuntu desktop ships it, but minimal/server/CI
# bases (e.g. the ubuntu:26.04 Docker image the tests use) don't — install if
# missing. apt lists may be empty on such bases, so refresh first. Idempotent.
if ! command -v gpg >/dev/null 2>&1; then
    info "gpg not found — installing gnupg"
    apt_update
    run_sudo apt-get install -y --no-install-recommends gnupg
fi
run_sudo install -d /etc/apt/keyrings
run_sudo bash -c "curl -fsSL '$URL/key.gpg' | gpg --dearmor -o '$KEYRING'"
echo "deb [signed-by=$KEYRING] $URL $SUITE main" \
    | run_sudo tee "$LIST" > /dev/null
apt_update
ok "Foundry apt source live (key at $KEYRING, list at $LIST)"
