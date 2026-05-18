#!/usr/bin/env bash
# Phase 0 installer for Blender. Installs blender + python3 from Ubuntu 26.04 universe.

set -euo pipefail

for arg in "$@"; do
    case "$arg" in
        -h|--help)
            cat <<EOF
Phase 0 installer for foundry-linux-blender

Installs Blender 4.2+ via apt. Blender addon registration is handled
separately by setup-wf-workspace.sh (requires the engine repo to be cloned).

Usage: $(basename "$0") [--dry-run|-n] [-h|--help]

Options:
  -n, --dry-run   Print apt commands without executing
  -h, --help      Show this help and exit
EOF
            exit 0
            ;;
    esac
done

DRY_RUN=false
for arg in "$@"; do
    case "$arg" in
        -n|--dry-run) DRY_RUN=true ;;
        *) echo "Unknown option: $arg (try --help)" >&2; exit 1 ;;
    esac
done

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
if [[ -f "$SCRIPT_DIR/lib.sh" ]]; then
    # shellcheck source=lib.sh
    source "$SCRIPT_DIR/lib.sh"
else
    info() { echo "ℹ $*"; }
    ok()   { echo "✓ $*"; }
    die()  { echo "✗ $*" >&2; exit 1; }
    step() { echo; echo "━━━ $* ━━━"; }
    run_sudo()  { if $DRY_RUN; then echo "  [dry-run] sudo $*"; else sudo "$@"; fi; }
    apt_update() { run_sudo apt-get update -q 2>&1 || echo "⚠ apt-get update had errors; continuing"; }
fi

step "Installing Blender"
info "Enabling universe (Blender 5.0+ is in Ubuntu 26.04 universe — no PPA needed)"
run_sudo add-apt-repository -y universe
apt_update
run_sudo apt-get install -y blender python3
ok "Blender installed"
