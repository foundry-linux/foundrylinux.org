#!/usr/bin/env bash
# Smoke-test that every tool foundry-anvil promises is on PATH inside the
# locally-built ghcr.io/foundry-linux/devbox:local image.
#
# Tools tested correspond to foundry-anvil's transitive Depends; sprite
# and atelier extensions are NOT in the image so their tools (krita, ardour,
# milkytracker, etc.) are NOT tested here.
#
# Usage:
#   bash test/smoke-test.sh                     # uses :local
#   IMAGE=ghcr.io/foundry-linux/devbox:26.04 bash test/smoke-test.sh

set -euo pipefail

IMAGE="${IMAGE:-ghcr.io/foundry-linux/devbox:local}"

TOOLS=(
    # foundry-retro-tools
    mame chdman ghidra ghidra-headless
    vgmstream-cli f9dasm vgm-player vgm2wav
    z80dasm z80asm dasm cc65 radare2 binwalk
    # foundry-emulators-computers
    dosbox-x hatari fs-uae openmsx
    # foundry-emulators-consoles
    fceux mednafen stella scummvm frotz desmume
    # foundry-game-frameworks
    tiled love sdl2-config sdl3-config glslangValidator spirv-cross spirv-val
    # foundry-image-cli
    magick gm
    # Cloudsmith task
    task
    # worldfoundry (cross-repo)
    blender cdpack iffcomp iffdump levcomp lvldump oaddump oas2oad textile prep
)

# /usr/games is in PATH inside the image — mame lives at /usr/games/mame per
# Debian games convention.
PATH_OVERRIDE="/usr/local/bin:/usr/local/sbin:/usr/bin:/usr/sbin:/usr/games:/bin:/sbin"

missing=()
for tool in "${TOOLS[@]}"; do
    if docker run --rm -e "PATH=$PATH_OVERRIDE" "$IMAGE" \
        bash -c "command -v $tool >/dev/null 2>&1"; then
        echo "  ✓ $tool"
    else
        echo "  ✗ $tool" >&2
        missing+=("$tool")
    fi
done

if (( ${#missing[@]} > 0 )); then
    echo >&2
    echo "MISSING: ${missing[*]}" >&2
    echo "${#missing[@]} of ${#TOOLS[@]} tools missing" >&2
    exit 1
fi

echo
echo "${#TOOLS[@]}/${#TOOLS[@]} tools verified"
