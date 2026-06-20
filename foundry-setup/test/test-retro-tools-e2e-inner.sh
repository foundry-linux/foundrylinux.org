#!/usr/bin/env bash
# Runs INSIDE an ubuntu:NN.NN container. Don't invoke directly on a host —
# use test-retro-tools-e2e.sh instead.
#
# 1. Installs apt prereqs (sudo, curl, gpg, ca-certificates).
# 2. Runs setup-foundry-apt-source.sh to wire apt.foundrylinux.org.
# 3. Runs install-foundry-retro-tools.sh — the Phase 0 entry point.
# 4. Verifies every binary from the metapackage Depends: by invoking it.

set -euo pipefail

REPO=/work
CONTROL="$REPO/foundry-apt/packages/foundry-retro-tools/debian/control"

# --- 1. base apt prereqs ----------------------------------------
export DEBIAN_FRONTEND=noninteractive
# ubuntu:26.04 minbase strips man pages / docs via dpkg path-excludes;
# lift the exclusion so installed packages land cleanly.
rm -f /etc/dpkg/dpkg.cfg.d/excludes
apt-get update -q
apt-get install -y --no-install-recommends sudo curl gpg ca-certificates

# --- 2. wire apt.foundrylinux.org ------------------------------
bash "$REPO/foundry-setup/setup-foundry-apt-source.sh"

# --- 3. Phase 0 entry point users actually hit ------------------
bash "$REPO/foundry-setup/install-foundry-retro-tools.sh"

# --- 4. verify each tool ----------------------------------------
# debian/control Depends: → list of packages → invocation per package.
# Failure modes: signal-terminated (rc >= 128), "command not found" (127),
# "not executable" (126). Other non-zero exits are accepted — lots of CLI
# tools exit non-zero on --help or no-args usage and that's fine.
#
# Add /usr/games to PATH: Debian convention puts game binaries (mame, etc.)
# there. Real desktop sessions get it via /etc/profile.d/, but a minimal
# container's default PATH doesn't include it.
export PATH="$PATH:/usr/games"
declare -A TOOL_CHECKS=(
    [mame]="mame -version"
    [mame-tools]="chdman --help"
    [dasm]="dasm"
    [cc65]="cc65 -V && sim65 -V && da65 -V"
    [z80dasm]="z80dasm -v"
    [z80asm]="z80asm -v"
    [radare2]="r2 -v"
    [binwalk]="binwalk --help"
    [sox]="sox --version"
    [binutils-m68k-linux-gnu]="m68k-linux-gnu-as --version && m68k-linux-gnu-objdump --version"
    [xa65]="xa -h"
    [f9dasm]="f9dasm -V"
    [libvgm]="vgm-player --help && vgm2wav --help"
    [vgmstream]="vgmstream-cli"
    [ghidra]="command -v ghidra && ghidra-headless"
    [cvise]="cvise --help"
    # delta ships singledelta/multidelta/topformflat (there is no 'delta'
    # command — that's git-delta). Presence-check the three binaries rather
    # than invoking them, since they read stdin and would hang the harness.
    [delta]="command -v singledelta && command -v multidelta && command -v topformflat"
)

# Extract Depends: from the metapackage control file. Strips ${shlibs:Depends}
# (build-time substitution) and version constraints like (>= 1.2).
mapfile -t DEPS < <(
    awk '
        /^Depends:/  { in_d = 1; sub(/^Depends:[[:space:]]*/, ""); }
        in_d && /^[A-Za-z]/ && !/^Depends:/ { in_d = 0 }
        in_d { print }
    ' "$CONTROL" \
    | tr ',' '\n' \
    | sed -E 's/^[[:space:]]+//; s/[[:space:]]+$//; s/[[:space:]]*\([^)]*\)//' \
    | grep -v '^\${' \
    | grep -v '^$' \
    | sort -u
)

echo
echo "======== Verifying ${#DEPS[@]} tools ========"

pass=0
fail=0
for dep in "${DEPS[@]}"; do
    check=${TOOL_CHECKS[$dep]:-}
    if [[ -z $check ]]; then
        echo "  ✗ $dep — no TOOL_CHECKS entry (update $0)"
        fail=$((fail + 1))
        continue
    fi
    if out=$(bash -c "$check" 2>&1); then
        echo "  ✓ $dep"
        pass=$((pass + 1))
    else
        rc=$?
        case $rc in
            126) reason="not executable" ;;
            127) reason="command not found" ;;
            *)   if (( rc >= 128 )); then reason="signal $((rc - 128))"; else reason=""; fi ;;
        esac
        if [[ -n $reason ]]; then
            echo "  ✗ $dep ($reason)"
            echo "$out" | sed 's/^/      /'
            fail=$((fail + 1))
        else
            echo "  ✓ $dep (exit $rc)"
            pass=$((pass + 1))
        fi
    fi
done

echo
echo "Results: ${pass} passed, ${fail} failed (out of ${#DEPS[@]})"
[[ $fail -eq 0 ]]
