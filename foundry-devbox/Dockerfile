# ghcr.io/foundry-linux/devbox:26.04
#
# Distrobox-compatible OCI image for World Foundry game-dev and retro-porting.
# Base MUST be ubuntu:26.04 — matches the apt suite ("resolute") and the
# distribution we ship. Host's KDE renders GUI; no Plasma needed inside.

FROM ubuntu:26.04

ENV DEBIAN_FRONTEND=noninteractive \
    LANG=C.UTF-8 \
    LC_ALL=C.UTF-8 \
    NVIDIA_VISIBLE_DEVICES=all \
    NVIDIA_DRIVER_CAPABILITIES=all

# Layer 1: apt bootstrap + multiverse repo + both Foundry-family apt sources +
# Cloudsmith (for task).
#
# multiverse is enabled at the image level so users inside the running
# container can `apt install vice atari800 fbzx mame-extra` themselves
# without ceremony. We don't preinstall any of those — see "deliberately
# excluded from image preinstall" in the design plan.
RUN apt-get update \
 && apt-get install -y --no-install-recommends \
        ca-certificates curl gnupg software-properties-common \
 && add-apt-repository -y multiverse \
 && install -d /etc/apt/keyrings \
 && curl -fsSL https://apt.foundrylinux.org/key.gpg \
      | gpg --dearmor -o /etc/apt/keyrings/foundry.gpg \
 && echo "deb [signed-by=/etc/apt/keyrings/foundry.gpg] https://apt.foundrylinux.org stable main" \
      > /etc/apt/sources.list.d/foundry.list \
 && curl -fsSL https://apt.worldfoundry.org/key.gpg \
      | gpg --dearmor -o /etc/apt/keyrings/worldfoundry.gpg \
 && echo "deb [signed-by=/etc/apt/keyrings/worldfoundry.gpg] https://apt.worldfoundry.org stable main" \
      > /etc/apt/sources.list.d/worldfoundry.list \
 && curl -1sLf 'https://dl.cloudsmith.io/public/task/task/setup.deb.sh' | bash \
 && apt-get update

# Layer 2: one metapackage. foundry-anvil (the "base layer" edition)
# pulls the entire devbox stack — sub-metapackages we own + worldfoundry
# cross-repo + task from Cloudsmith + distrobox conveniences. No bare
# Ubuntu packages here. Sprite (graphics/audio) and atelier (everything)
# editions exist on apt.foundrylinux.org but are NOT preinstalled in the
# image — users layer them on inside the running container if wanted.
#
# Transitive expansion of foundry-anvil (apt resolves these; comments are for readers):
#   foundry-anvil               Depends:
#     ├── foundry-retro-tools         = mame mame-tools dasm cc65 z80dasm z80asm radare2 binwalk
#     │                                 sox binutils-m68k-linux-gnu xa65 f9dasm libvgm vgmstream ghidra
#     ├── foundry-emulators-computers = dosbox-x hatari fs-uae openmsx openmsx-data
#     ├── foundry-emulators-consoles  = fceux mednafen stella desmume scummvm frotz
#     ├── foundry-game-frameworks     = tiled love libsdl2-dev libsdl3-dev libsfml-dev
#     │                                 liballegro5-dev libtcod-dev glslang-tools spirv-cross spirv-tools
#     ├── foundry-image-cli           = imagemagick graphicsmagick
#     ├── task                          (resolved directly from the Cloudsmith repo configured in Layer 1)
#     ├── worldfoundry [cross-repo]   = worldfoundry-cli + worldfoundry-blender
#     │     ├── worldfoundry-cli      = cdpack iffcomp iffdump levcomp lvldump oaddump
#     │     │                           oas2oad textile blender-asset-finder-cli prep
#     │     └── worldfoundry-blender  = wf-blender + blender-asset-finder + blender (5.0.1)
#     └── sudo libvte-2.91-0 bash-completion man-db   (distrobox conveniences)
RUN apt-get install -y --no-install-recommends \
        foundry-anvil \
 && apt-get clean \
 && rm -rf /var/lib/apt/lists/*

LABEL org.opencontainers.image.title="Foundry Linux devbox" \
      org.opencontainers.image.description="Distrobox-compatible WF authoring + retro-porting environment for Ubuntu 26.04" \
      org.opencontainers.image.source="https://github.com/foundry-linux/foundry-devbox" \
      org.opencontainers.image.licenses="GPL-2.0-or-later"

CMD ["/bin/bash"]
