#!/usr/bin/env bash
# Phase 0 installer for uv (Astral's Python package and project manager).
#
# Routes through the vendored uv package on apt.foundrylinux.org, which
# ships both the uv and uvx binaries plus bash/zsh/fish completions.
# Editions (anvil/sprite/atelier) get uv transitively via foundry-core;
# this script exists for the legacy roles that never install an edition.
#
# Idempotent — safe to re-run.

set -euo pipefail

for arg in "$@"; do
    case "$arg" in
        -h|--help)
            cat <<EOF
Phase 0 installer for uv

Installs the uv package from apt.foundrylinux.org: the uv and uvx
binaries (Python package/project manager, tool runner, venv and
lockfile management) plus shell completions.

Usage: $(basename "$0") [--dry-run|-n] [-h|--help]

Options:
  -n, --dry-run   Print commands without executing
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
    run_sudo() { if $DRY_RUN; then echo "  [dry-run] sudo $*"; else sudo "$@"; fi; }
    apt_update() { run_sudo apt-get update -q 2>&1 || echo "⚠ apt-get update had errors; continuing"; }
fi

step "Installing uv (Python package and project manager)"

if ! $DRY_RUN && command -v uv &>/dev/null; then
    info "uv already installed: $(uv --version 2>/dev/null || echo '?')"
    exit 0
fi

step "Configuring apt.foundrylinux.org"
dry=()
$DRY_RUN && dry=(--dry-run)
FOUNDRY_LOG_FILE="${FOUNDRY_LOG_FILE:-}" bash "$SCRIPT_DIR/setup-foundry-apt-source.sh" "${dry[@]}"

apt_update
run_sudo apt-get install -y uv
ok "uv installed (uv + uvx)"
