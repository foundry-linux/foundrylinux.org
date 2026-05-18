#!/usr/bin/env bash
# Phase 0 installer for the foundry-linux-retro-tools metapackage.
#
# Installs the arcade reverse-engineering / 6502+Z80+68k+6809 porting toolchain:
#
#   apt (from foundry-apt/packages/foundry-linux-retro-tools/debian/control Depends):
#     mame mame-tools dasm cc65 z80dasm z80asm radare2 binwalk sox
#     binutils-m68k-linux-gnu xa65 f9dasm libvgm vgmstream ghidra
#
# What you get for the 6502 specifically:
#   sim65   (from cc65)     — 6502 instruction-set simulator / unit-test runner
#   da65    (from cc65)     — 6502 disassembler
#   mame                    — full-system 6502 emulator (Atari, Apple, Q*bert, etc.)
#   radare2                 — multi-arch disasm (Capstone covers 6502)
#   xa65                    — Atari/C64-style 6502 cross-assembler (Ubuntu universe)
#
# See docs/investigations/2026-05-15-claude-arcade-tooling.md for the rationale.

set -euo pipefail

for arg in "$@"; do
    case "$arg" in
        -h|--help)
            cat <<EOF
Phase 0 installer for foundry-linux-retro-tools

Installs the arcade reverse-engineering / 6502 / Z80 / 68k / 6809 porting
toolchain via apt.foundrylinux.org:
  MAME, dasm, cc65 (sim65 + da65), z80dasm, z80asm, radare2, binwalk, sox,
  xa65, f9dasm, libvgm (vgm2wav + vgm-player), vgmstream-cli, ghidra.

Usage: $(basename "$0") [--dry-run|-n] [-h|--help]

Options:
  -n, --dry-run   Print commands without executing
  -h, --help      Show this help and exit

What this installs that's relevant to the 6502:
  sim65   — cc65's 6502 simulator (great for unit-testing 6502 routines)
  da65    — cc65's 6502 disassembler
  mame    — full-system 6502 emulator
  radare2 — Capstone-based multi-arch disasm (covers 6502)
  xa65    — Atari/C64-style 6502 cross-assembler (Ubuntu universe)
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
    warn() { echo "⚠ $*"; }
    die()  { echo "✗ $*" >&2; exit 1; }
    step() { echo; echo "━━━ $* ━━━"; }
    run()       { if $DRY_RUN; then echo "  [dry-run] $*"; else "$@"; fi; }
    run_sudo()  { if $DRY_RUN; then echo "  [dry-run] sudo $*"; else sudo "$@"; fi; }
    apt_update() { run_sudo apt-get update -q 2>&1 || echo "⚠ apt-get update had errors; continuing"; }
fi

step "Installing foundry-linux-retro-tools (apt)"
apt_update
run_sudo apt-get install -y foundry-linux-retro-tools

step "foundry-linux-retro-tools install complete"
ok "6502: sim65, da65, mame, radare2, xa65"
ok "Z80:  z80dasm, z80asm, mame, radare2"
ok "68k:  m68k-linux-gnu-objdump, mame, radare2"
ok "6809: ghidra (apt), f9dasm (apt), mame"
ok "Audio: sox, vgmstream-cli (apt), libvgm (apt), mame -wavwrite"
