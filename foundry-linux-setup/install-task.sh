#!/usr/bin/env bash
# Installs task (go-task) from the official Cloudsmith apt repo.
#
# Adds the Cloudsmith repo if not already present, then installs task
# system-wide via apt. Idempotent — safe to re-run.

set -euo pipefail

for arg in "$@"; do
    case "$arg" in
        -h|--help)
            cat <<EOF
Installs task (go-task) from the official Cloudsmith apt repo.

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
    apt_update() { run_sudo apt-get update -qq; }
fi

step "Installing task (Taskfile runner)"

if command -v task &>/dev/null; then
    info "task already installed: $(task --version 2>/dev/null || echo '?')"
    exit 0
fi

info "Adding official Cloudsmith apt repo for task"
run_sudo bash -c "curl -1sLf 'https://dl.cloudsmith.io/public/task/task/setup.deb.sh' | bash"
apt_update
run_sudo apt-get install -y task
ok "task installed"
