#!/usr/bin/env bash
# Phase 0 installer for the iOS development toolchain.
#
# Installs the Linux-side iOS device-communication tools via the
# foundry-linux-ios-development metapackage on apt.foundrylinux.org
# (libimobiledevice-utils, ideviceinstaller, usbmuxd, ifuse) PLUS
# codemagic-cli-tools via pipx — Codemagic's Python CLI for App Store
# Connect API access and build-artifact management.
#
# The actual iOS build (Metal backend, UIKit) runs on macOS via
# Codemagic cloud CI; Linux contributes editing, git, and device-side
# tooling only.

set -euo pipefail

for arg in "$@"; do
    case "$arg" in
        -h|--help)
            cat <<EOF
Phase 0 installer for foundry-linux-ios-development

Installs Linux-side iOS development tooling:
  apt (apt.foundrylinux.org → foundry-linux-ios-development):
    libimobiledevice-utils, ideviceinstaller, usbmuxd, ifuse
  pipx:
    codemagic-cli-tools (App Store Connect API + signing)

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
    run()      { if $DRY_RUN; then echo "  [dry-run] $*"; else "$@"; fi; }
    apt_update() { run_sudo apt-get update -q 2>&1 || echo "⚠ apt-get update had errors; continuing"; }
fi

step "Configuring apt.foundrylinux.org"
dry=()
$DRY_RUN && dry=(--dry-run)
FOUNDRY_LOG_FILE="${FOUNDRY_LOG_FILE:-}" bash "$SCRIPT_DIR/setup-foundry-apt-source.sh" "${dry[@]}"

step "Installing foundry-linux-ios-development"
apt_update
run_sudo apt-get install -y foundry-linux-ios-development pipx
ok "foundry-linux-ios-development installed"

step "Installing codemagic-cli-tools via pipx"
# pipx must run as the calling user so the venv lands in their $HOME/.local.
# codemagic-cli-tools is not in any apt source — it's a pure-Python package
# only published to PyPI. pipx (Ubuntu universe) isolates it in its own venv.
run pipx install codemagic-cli-tools
ok "codemagic-cli-tools installed (app-store-connect command available)"
