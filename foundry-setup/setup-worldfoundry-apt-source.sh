#!/usr/bin/env bash
# Add apt.worldfoundry.org to /etc/apt/sources.list.d. Idempotent.
#
# Once this runs, `apt install worldfoundry-*` and the WF authoring
# tools (cdpack, iffcomp, levcomp, wf-blender, …) resolve from the
# WorldFoundry apt repo. This is a sibling repo to apt.foundrylinux.org;
# they are deliberately separate and a workstation typically configures
# both.

set -euo pipefail

for arg in "$@"; do
    case "$arg" in
        -h|--help)
            cat <<EOF
Configure apt.worldfoundry.org as an apt source.

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

KEYRING=/etc/apt/keyrings/worldfoundry.gpg
LIST=/etc/apt/sources.list.d/worldfoundry.list
URL=https://apt.worldfoundry.org
SUITE=stable

if [[ -f "$KEYRING" && -f "$LIST" ]]; then
    info "WorldFoundry apt source already configured at $LIST — skipping"
    exit 0
fi

step "Configuring $URL as an apt source"
run_sudo install -d /etc/apt/keyrings
run_sudo bash -c "curl -fsSL '$URL/key.gpg' | gpg --dearmor -o '$KEYRING'"
echo "deb [signed-by=$KEYRING] $URL $SUITE main" \
    | run_sudo tee "$LIST" > /dev/null
apt_update
ok "WorldFoundry apt source live (key at $KEYRING, list at $LIST)"
