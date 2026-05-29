#!/usr/bin/env bash
# Foundry Linux setup script (Phase 0 of the distro plan)
#
# Installs Foundry Linux system packages by composing per-metapackage
# installers (mirroring foundry-apt/packages/).
#
# WF engine workspace setup (repo clones, Rust, wftools build, Blender addon)
# is handled by setup-wf-workspace.sh — see docs/plans/2026-05-17-wf-workspace-setup.md
#
# Usage:
#   curl -fsSL https://install.foundry-linux.org/install.sh | bash
#   bash install.sh                             # local
#   bash install.sh --role game-dev             # specify role
#   bash install.sh --dry-run                   # print plan, don't execute
#
# Base: Ubuntu 26.04 LTS ("Resolute Raccoon") only — no other release is
# supported. --force exists as a panic-button override, not a path.
#
# Roles (control which metapackages are installed):
#
#   Phase 2 nested editions (preferred — see docs/plans/2026-05-21-phase-2-devbox-image.md):
#     anvil      — base layer; WF stack + retro-tools + light emulators + game frameworks
#     sprite     — anvil + heavy graphics + audio (krita, gimp, ardour, trackers, …)
#     atelier    — sprite + everything else (ROM-bundled emulators, free games, mobile dev)
#
#   Legacy roles (kept for back-compat with pre-edition installs):
#     play       — just play games (no dev tools; needs a runtime metapackage, coming later)
#     game-dev   — author WF games (engine-build-deps + blender + retro-tools + task)
#     engine-dev — hack on the engine (engine-build-deps + retro-tools + task; no blender)
#     both       — game-dev + engine-dev (default; installs foundry-dev umbrella)
#     maintainer — both + android-dev + foundry distro repos
#
# Idempotent: safe to re-run.
# Logs to: ~/.local/state/foundry-install.log

set -euo pipefail

# ============================================================================
# Configuration
# ============================================================================
SUPPORTED_RELEASE="26.04"
LOG_FILE="${HOME}/.local/state/foundry-install.log"
FOUNDRY_GITHUB_ORG="foundry-linux"
FOUNDRY_REPOS=(foundry-setup foundry-apt foundry-devbox foundry-iso foundry-docs foundry-branding)

# Defaults (overridable via flags)
ROLE="both"
SKIP_BLENDER=false
SKIP_RETRO=false
APT_ONLY=false
SKIP_CLONE=false
FORCE=false
DRY_RUN=false

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck source=lib.sh
source "$SCRIPT_DIR/lib.sh"

# ============================================================================
# Arg parsing
# ============================================================================
parse_args() {
    while [[ $# -gt 0 ]]; do
        case "$1" in
            --role)         shift; ROLE="$1" ;;
            --role=*)       ROLE="${1#*=}" ;;
            --skip-blender) SKIP_BLENDER=true ;;
            --skip-retro)   SKIP_RETRO=true ;;
            --apt-only)     APT_ONLY=true ;;
            --skip-clone)   SKIP_CLONE=true ;;
            --force)        FORCE=true ;;
            --dry-run|-n)   DRY_RUN=true ;;
            -h|--help)      show_help; exit 0 ;;
            *)              die "Unknown option: $1 (try --help)" ;;
        esac
        shift
    done

    case "$ROLE" in
        anvil|sprite|atelier) ;;
        play|game-dev|engine-dev|both|maintainer) ;;
        *) die "Invalid role: '$ROLE' (must be anvil, sprite, atelier, play, game-dev, engine-dev, both, or maintainer)" ;;
    esac
}

show_help() {
    cat <<EOF
${BOLD}Foundry Linux setup script${RESET} (Phase 0)

Installs Foundry Linux system packages. For WF engine workspace setup
(repo clones, Rust, wftools build, Blender addon), run setup-wf-workspace.sh
after this script completes.

Usage: $(basename "$0") [OPTIONS]

Base: Ubuntu 26.04 LTS ("Resolute Raccoon") only.

Options:
  --role ROLE       Install role. Phase 2 nested editions (preferred):
                      anvil, sprite, atelier
                    Legacy roles (back-compat):
                      play, game-dev, engine-dev, both, maintainer
                    (default: both)
  --skip-blender    Skip foundry-blender install (legacy roles only)
  --skip-retro      Skip foundry-retro-tools install (legacy roles only)
  --apt-only        Forwarded to retro-tools: skip source-build sidecars
  --skip-clone      Skip cloning Foundry distro repos (maintainer role only)
  --force           Bypass distro/version checks (use at own risk)
  -n, --dry-run     Print the plan without executing anything
  -h, --help        Show this help

Examples:
  curl -fsSL https://install.foundry-linux.org/install.sh | bash
  bash install.sh --role anvil                 # Phase 2 base edition
  bash install.sh --role atelier               # Phase 2 full kit (~10 GB)
  bash install.sh --role engine-dev            # legacy
  bash install.sh --dry-run --role anvil

The script logs to: $LOG_FILE
Per-metapackage installers live next to this script as install-<name>.sh.
EOF
}

# ============================================================================
# Pre-flight checks
# ============================================================================
check_distro() {
    step "Checking distribution"
    if [[ ! -f /etc/os-release ]]; then
        die "/etc/os-release not found — not an Ubuntu-family system"
    fi
    # shellcheck disable=SC1091
    source /etc/os-release

    info "Detected: ${PRETTY_NAME:-unknown}"

    local id_like="${ID_LIKE:-}"
    if [[ "${ID:-}" != "ubuntu" && "$id_like" != *"ubuntu"* && "$id_like" != *"debian"* ]]; then
        if $FORCE; then
            warn "Non-Ubuntu-family distro (${ID:-unknown}); --force is set, proceeding"
        else
            die "This script targets Ubuntu-family distros (got ID='${ID:-unknown}'). Use --force to override (untested)."
        fi
    fi

    case "${VERSION_ID:-}" in
        "$SUPPORTED_RELEASE")
            ok "Ubuntu-family $SUPPORTED_RELEASE detected — supported"
            ;;
        *)
            if $FORCE; then
                warn "Ubuntu-family ${VERSION_ID:-unknown} — untested but --force is set"
            else
                die "Unsupported release: ${VERSION_ID:-unknown}. Foundry Linux targets Ubuntu $SUPPORTED_RELEASE LTS only. Use --force to override (untested)."
            fi
            ;;
    esac
}

check_sudo() {
    step "Checking sudo access"
    if [[ $EUID -eq 0 ]]; then
        warn "Running as root — proceed with caution"
        return
    fi
    if ! $DRY_RUN && ! sudo -v; then
        die "sudo access required for apt operations"
    fi
    if ! $DRY_RUN; then
        ( while true; do sleep 50; sudo -n true 2>/dev/null; kill -0 $$ 2>/dev/null || exit; done ) &
        SUDO_KEEPALIVE_PID=$!
        trap '[[ -n "${SUDO_KEEPALIVE_PID:-}" ]] && kill "$SUDO_KEEPALIVE_PID" 2>/dev/null || true' EXIT
    fi
    ok "sudo access confirmed"
}

# ============================================================================
# Foundry apt source
# ============================================================================
setup_foundry_apt_source() {
    local dry=()
    $DRY_RUN && dry=(--dry-run)
    FOUNDRY_LOG_FILE="$LOG_FILE" bash "$SCRIPT_DIR/setup-foundry-apt-source.sh" "${dry[@]}"
}

# ============================================================================
# Per-metapackage dispatch
# ============================================================================
run_subscript() {
    local name="$1"; shift
    local path="$SCRIPT_DIR/$name"
    if [[ ! -x "$path" ]]; then
        die "Sub-installer missing or not executable: $path"
    fi
    info "→ $name $*"
    FOUNDRY_LOG_FILE="$LOG_FILE" bash "$path" "$@"
}

install_edition() {
    # Phase 2 nested editions — author the same on bare-metal as the
    # devbox image. anvil/sprite/atelier all cross-Depend on apt.worldfoundry.org;
    # atelier additionally needs multiverse for ROM-bundled emulators + game
    # reimplementations + the Android NDK.
    local edition="$1"
    setup_worldfoundry_source
    if [[ "$edition" == "atelier" ]]; then
        enable_multiverse
    fi
    apt_update
    step "Installing foundry-$edition"
    if $DRY_RUN; then
        echo "  ${YELLOW}[dry-run]${RESET} sudo apt-get install -y foundry-$edition"
    else
        sudo apt-get install -y "foundry-$edition"
    fi
    ok "foundry-$edition installed"
}

install_metapackages() {
    local dry=()
    $DRY_RUN && dry=(--dry-run)

    case "$ROLE" in
        anvil|sprite|atelier)
            install_edition "$ROLE"
            ;;
        play)
            warn "Role 'play': no runtime metapackage yet — nothing to install via apt"
            ;;
        game-dev)
            run_subscript install-foundry-engine-build-deps.sh "${dry[@]}"
            run_subscript install-task.sh "${dry[@]}"
            $SKIP_BLENDER || run_subscript install-foundry-blender.sh "${dry[@]}"
            if ! $SKIP_RETRO; then
                local args=("${dry[@]}")
                $APT_ONLY && args+=(--apt-only)
                $FORCE    && args+=(--force)
                run_subscript install-foundry-retro-tools.sh "${args[@]}"
            fi
            ;;
        engine-dev)
            run_subscript install-foundry-engine-build-deps.sh "${dry[@]}"
            run_subscript install-task.sh "${dry[@]}"
            if ! $SKIP_RETRO; then
                local args=("${dry[@]}")
                $APT_ONLY && args+=(--apt-only)
                $FORCE    && args+=(--force)
                run_subscript install-foundry-retro-tools.sh "${args[@]}"
            fi
            ;;
        both)
            local dev_args=("${dry[@]}")
            $SKIP_BLENDER && dev_args+=(--skip-blender)
            $SKIP_RETRO   && dev_args+=(--skip-retro)
            $APT_ONLY     && dev_args+=(--apt-only)
            $FORCE        && dev_args+=(--force)
            run_subscript install-foundry-dev.sh "${dev_args[@]}"
            ;;
        maintainer)
            local dev_args=("${dry[@]}")
            $SKIP_BLENDER && dev_args+=(--skip-blender)
            $SKIP_RETRO   && dev_args+=(--skip-retro)
            $APT_ONLY     && dev_args+=(--apt-only)
            $FORCE        && dev_args+=(--force)
            run_subscript install-foundry-dev.sh "${dev_args[@]}"
            run_subscript install-foundry-android-development.sh "${dry[@]}"
            run_subscript install-foundry-ios-development.sh "${dry[@]}"
            if $SKIP_CLONE; then
                info "Skipping Foundry repo clone (--skip-clone)"
            elif ! $DRY_RUN; then
                clone_foundry_repos
            else
                for repo in "${FOUNDRY_REPOS[@]}"; do
                    echo "  ${YELLOW}[dry-run]${RESET} git clone https://github.com/$FOUNDRY_GITHUB_ORG/$repo.git"
                done
            fi
            ;;
    esac
}

clone_foundry_repos() {
    local projects_dir="${PROJECTS_DIR:-${HOME}/Projects}"
    mkdir -p "$projects_dir"
    for repo in "${FOUNDRY_REPOS[@]}"; do
        local target="$projects_dir/$repo"
        if [[ -d "$target/.git" ]]; then
            info "$repo already cloned — pulling"
            git -C "$target" pull --rebase || warn "$repo: pull failed, continuing"
        else
            info "Cloning $FOUNDRY_GITHUB_ORG/$repo..."
            git clone --depth 1 --filter=blob:none \
                "https://github.com/$FOUNDRY_GITHUB_ORG/$repo.git" "$target" || \
                warn "$repo: clone failed (non-fatal in Phase 0)"
        fi
    done
}

# ============================================================================
# Summary
# ============================================================================
summary() {
    step "Foundry Linux Phase 0 install complete"
    cat <<EOF

${GREEN}${BOLD}System packages installed!${RESET}

  Role:     $ROLE
  Log:      $LOG_FILE

${BOLD}Next steps:${RESET}
  • Add ~/.local/bin to PATH if needed:   export PATH="\$HOME/.local/bin:\$PATH"
  • Set up the WF engine workspace:       bash setup-wf-workspace.sh
  • Visit https://docs.foundry-linux.org for the full quickstart

${BLUE}Phase 0 (curl-bash installer). Phase 1+ will ship a signed APT repo
so future updates are one 'apt upgrade' away.${RESET}
EOF
}

# ============================================================================
# Main
# ============================================================================
main() {
    parse_args "$@"
    init_logging
    log_to_file "Args: ROLE=$ROLE SKIP_BLENDER=$SKIP_BLENDER SKIP_RETRO=$SKIP_RETRO APT_ONLY=$APT_ONLY SKIP_CLONE=$SKIP_CLONE FORCE=$FORCE DRY_RUN=$DRY_RUN"

    echo
    echo "${BOLD}${BLUE}Foundry Linux setup script${RESET} (Phase 0)"
    echo "Log: $LOG_FILE"
    $DRY_RUN && echo "${YELLOW}${BOLD}DRY-RUN MODE — no changes will be made${RESET}"
    echo

    check_distro
    check_sudo
    setup_foundry_apt_source
    install_metapackages
    summary
}

main "$@"
