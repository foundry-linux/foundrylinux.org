# Shared helpers for foundry-setup install scripts.
#
# Sourced by install.sh and each install-foundry-*.sh per-metapackage
# script. Defines colors, logging, and dry-run-aware command wrappers.
#
# Usage:
#   source "$(dirname "$0")/lib.sh"
#
# State the caller must set before sourcing (or rely on defaults):
#   DRY_RUN              — true/false; default false
#   LOG_FILE             — path; default ${HOME}/.local/state/foundry-install.log
#                          (env override: FOUNDRY_LOG_FILE)
#   LOG_FILE_INITIALISED — set by init_logging; until then log_to_file is a no-op
#                          (this lets standalone -h short-circuit without touching disk)

# shellcheck shell=bash

set -euo pipefail

: "${DRY_RUN:=false}"
: "${LOG_FILE:=${FOUNDRY_LOG_FILE:-${HOME}/.local/state/foundry-install.log}}"

if [[ -t 1 ]]; then
    RED=$'\e[31m'; GREEN=$'\e[32m'; YELLOW=$'\e[33m'; BLUE=$'\e[34m'
    BOLD=$'\e[1m'; RESET=$'\e[0m'
else
    RED=; GREEN=; YELLOW=; BLUE=; BOLD=; RESET=
fi

log_to_file() {
    [[ -n "${LOG_FILE_INITIALISED:-}" ]] || return 0
    echo "[$(date -u +%Y-%m-%dT%H:%M:%SZ)] $*" >> "$LOG_FILE"
}

info()  { echo "${BLUE}ℹ ${RESET}$*"; log_to_file "INFO  $*"; }
ok()    { echo "${GREEN}✓ ${RESET}$*"; log_to_file "OK    $*"; }
warn()  { echo "${YELLOW}⚠ ${RESET}$*"; log_to_file "WARN  $*"; }
err()   { echo "${RED}✗ ${RESET}$*" >&2; log_to_file "ERROR $*"; }
die()   { err "$*"; exit 1; }

step() {
    echo
    echo "${BOLD}${BLUE}━━━ $* ━━━${RESET}"
    log_to_file "STEP  $*"
}

run() {
    log_to_file "RUN   $*"
    if $DRY_RUN; then
        echo "  ${YELLOW}[dry-run]${RESET} $*"
    else
        "$@"
    fi
}

run_sudo() {
    log_to_file "SUDO  $*"
    if $DRY_RUN; then
        echo "  ${YELLOW}[dry-run]${RESET} sudo $*"
    else
        sudo "$@"
    fi
}

apt_update() {
    # apt-get update exits non-zero when any configured source fails (e.g. a
    # stale cloudsmith/PPA entry). That shouldn't abort the whole install — the
    # cached package lists are still usable for the packages we actually need.
    log_to_file "SUDO  apt-get update -q"
    if $DRY_RUN; then
        echo "  ${YELLOW}[dry-run]${RESET} sudo apt-get update -q"
    else
        sudo apt-get update -q 2>&1 || warn "apt-get update had errors (stale repo entry?); using cached package lists — install may still succeed"
    fi
}

init_logging() {
    mkdir -p "$(dirname "$LOG_FILE")"
    : > "$LOG_FILE"
    LOG_FILE_INITIALISED=1
    log_to_file "Foundry Linux install — started ($(basename "${0:-shell}"))"
}

# Resolve foundry-setup/ (directory containing lib.sh) so helpers can locate
# sibling setup scripts regardless of which install-foundry-*.sh sourced lib.sh.
FOUNDRY_SETUP_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Enable Ubuntu multiverse component. Idempotent — no-op if already enabled.
# Required for: foundry-emulators-vintage (vice/atari800/fbzx), foundry-game-
# reimplementations (vcmi/openrct2/...), foundry-android-development (NDK),
# foundry-atelier (transitively).
enable_multiverse() {
    log_to_file "CHECK multiverse component"
    if grep -hsE '^(deb |Components:)' /etc/apt/sources.list /etc/apt/sources.list.d/* 2>/dev/null \
        | grep -qw multiverse; then
        info "Ubuntu multiverse already enabled"
        return 0
    fi
    log_to_file "SUDO  add-apt-repository -y multiverse"
    if $DRY_RUN; then
        echo "  ${YELLOW}[dry-run]${RESET} sudo add-apt-repository -y multiverse"
    else
        sudo add-apt-repository -y multiverse
    fi
}

# Wire up apt.worldfoundry.org as an apt source. Idempotent.
# Required for: foundry-anvil, foundry-sprite, foundry-atelier (cross-Depend
# on worldfoundry / worldfoundry-blender / worldfoundry-development).
setup_worldfoundry_source() {
    if [[ -f /etc/apt/sources.list.d/worldfoundry.list ]]; then
        info "WorldFoundry apt source already configured"
        return 0
    fi
    local dry=()
    $DRY_RUN && dry=(--dry-run)
    FOUNDRY_LOG_FILE="$LOG_FILE" bash "$FOUNDRY_SETUP_DIR/setup-worldfoundry-apt-source.sh" "${dry[@]}"
}
