#!/usr/bin/env bash
# Phase 0 installer for the WorldFoundry Blender stack.
#
# Installs the `worldfoundry-blender` metapackage from apt.worldfoundry.org,
# which transitively pulls:
#   - blender (Ubuntu 26.04 universe, ≥4.2)
#   - wf-blender (the World Foundry level-editor add-on)
#   - blender-asset-finder (CC0 asset search/download add-on)
#
# The two add-ons can be enabled in Blender via the post-install wrappers
# wf-blender-install(1) and blender-asset-finder-install(1).

set -euo pipefail

for arg in "$@"; do
    case "$arg" in
        -h|--help)
            cat <<EOF
Phase 0 installer for foundry-blender

Installs worldfoundry-blender from apt.worldfoundry.org — Blender 4.2+ plus
the WorldFoundry Blender add-on (wf-blender) and the asset-finder add-on
(blender-asset-finder). Add-on registration into Blender is handled by the
per-add-on -install wrappers shipped inside each .deb.

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
    run_sudo()  { if $DRY_RUN; then echo "  [dry-run] sudo $*"; else sudo "$@"; fi; }
    apt_update() { run_sudo apt-get update -q 2>&1 || echo "⚠ apt-get update had errors; continuing"; }
fi

step "Configuring apt.worldfoundry.org"
dry=()
$DRY_RUN && dry=(--dry-run)
FOUNDRY_LOG_FILE="${FOUNDRY_LOG_FILE:-}" bash "$SCRIPT_DIR/setup-worldfoundry-apt-source.sh" "${dry[@]}"

step "Installing worldfoundry-blender"
apt_update
run_sudo apt-get install -y worldfoundry-blender
ok "worldfoundry-blender installed (Blender + wf-blender + blender-asset-finder)"
