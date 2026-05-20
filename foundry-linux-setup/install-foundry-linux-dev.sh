#!/usr/bin/env bash
# Phase 0 installer for the dev umbrella.
#
# Default path: apt install worldfoundry-development (apt.worldfoundry.org
# umbrella — pulls every WF authoring CLI + Blender + the WF Blender add-on
# + Blender asset-finder + engine build deps), then chains install-task.sh
# (Cloudsmith) and install-foundry-linux-retro-tools.sh (apt.foundrylinux.org).
#
# --skip-blender fallback path: install-foundry-linux-engine-build-deps.sh
# + apt install worldfoundry-cli (no Blender) + task + retro-tools.

set -euo pipefail

for arg in "$@"; do
    case "$arg" in
        -h|--help)
            cat <<EOF
Phase 0 installer for the dev umbrella

Default path:  apt install worldfoundry-development + task + foundry-linux-retro-tools
--skip-blender: engine-build-deps + worldfoundry-cli + task + retro-tools

Usage: $(basename "$0") [--dry-run|-n] [--skip-blender] [--skip-retro] [-h|--help]

Options:
  -n, --dry-run     Print all commands without executing
  --skip-blender    Don't install Blender / WF Blender add-ons (uses worldfoundry-cli instead of worldfoundry-development)
  --skip-retro      Skip foundry-linux-retro-tools
  -h, --help        Show this help and exit
EOF
            exit 0
            ;;
    esac
done

DRY_RUN=false
SKIP_BLENDER=false
SKIP_RETRO=false
APT_ONLY=false
FORCE=false
for arg in "$@"; do
    case "$arg" in
        -n|--dry-run)   DRY_RUN=true ;;
        --skip-blender) SKIP_BLENDER=true ;;
        --skip-retro)   SKIP_RETRO=true ;;
        --apt-only)     APT_ONLY=true ;;
        --force)        FORCE=true ;;
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

DRY_FLAG=()
$DRY_RUN && DRY_FLAG=(--dry-run)

run_subscript() {
    local name="$1"; shift
    local path="$SCRIPT_DIR/$name"
    if [[ ! -x "$path" ]]; then
        die "Sub-installer missing or not executable: $path"
    fi
    info "→ $name $*"
    bash "$path" "$@"
}

if $SKIP_BLENDER; then
    step "dev umbrella (no Blender): engine-build-deps + worldfoundry-cli"
    run_subscript install-foundry-linux-engine-build-deps.sh "${DRY_FLAG[@]}"
    run_subscript setup-worldfoundry-apt-source.sh "${DRY_FLAG[@]}"
    apt_update
    run_sudo apt-get install -y worldfoundry-cli
    ok "worldfoundry-cli installed (10 CLI tools, no Blender)"
else
    step "dev umbrella: worldfoundry-development (cli + Blender + dev deps)"
    run_subscript setup-worldfoundry-apt-source.sh "${DRY_FLAG[@]}"
    apt_update
    run_sudo apt-get install -y worldfoundry-development
    ok "worldfoundry-development installed"
fi

step "dev umbrella: task runner"
run_subscript install-task.sh "${DRY_FLAG[@]}"

if $SKIP_RETRO; then
    info "Skipping foundry-linux-retro-tools (--skip-retro)"
else
    step "dev umbrella: retro tools"
    retro_args=("${DRY_FLAG[@]}")
    $APT_ONLY && retro_args+=(--apt-only)
    $FORCE    && retro_args+=(--force)
    run_subscript install-foundry-linux-retro-tools.sh "${retro_args[@]}"
fi

step "Dev umbrella install complete"
ok "All metapackages installed"
