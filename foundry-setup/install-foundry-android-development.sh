#!/usr/bin/env bash
# Phase 0 installer for the Android development toolchain.
#
# Routes through the foundry-android-development metapackage on
# apt.foundrylinux.org, which pulls openjdk-17-jdk, adb, and
# google-android-ndk-r26c-installer (NDK r26c from Ubuntu multiverse).

set -euo pipefail

for arg in "$@"; do
    case "$arg" in
        -h|--help)
            cat <<EOF
Phase 0 installer for foundry-android-development

Installs the foundry-android-development metapackage from
apt.foundrylinux.org: openjdk-17-jdk, adb, and Android NDK r26c.
The NDK lives in Ubuntu multiverse — this script enables multiverse
first.

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

step "Configuring apt.foundrylinux.org"
dry=()
$DRY_RUN && dry=(--dry-run)
FOUNDRY_LOG_FILE="${FOUNDRY_LOG_FILE:-}" bash "$SCRIPT_DIR/setup-foundry-apt-source.sh" "${dry[@]}"

step "Enabling Ubuntu multiverse (required for google-android-ndk-r26c-installer)"
run_sudo add-apt-repository -y multiverse
apt_update

step "Installing foundry-android-development"
run_sudo apt-get install -y foundry-android-development
ok "foundry-android-development installed (JDK 17, adb, NDK r26c)"
