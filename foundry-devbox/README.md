# foundry-devbox

Distrobox-compatible OCI image: **`ghcr.io/foundry-linux/devbox:26.04`**.

A single `apt install foundry-core` brings the full Foundry Linux base layer:
WorldFoundry authoring stack (Blender + 10 WF CLIs + WF Blender add-ons),
retro-porting toolkit (MAME, Ghidra, dasm, z80, cc65, …), light universe
emulators, game-dev frameworks (SDL2/3, SFML, Allegro, libtcod, Tiled, LÖVE,
shader tools), CLI image utilities, and go-task — all preinstalled.

See [`docs/plans/2026-05-21-phase-2-devbox-image.md`](../docs/plans/2026-05-21-phase-2-devbox-image.md)
for the full design.

## Use it

```bash
distrobox create -i ghcr.io/foundry-linux/devbox:26.04 -n foundry
distrobox enter foundry
```

Inside the container, everything in foundry-core is on `PATH`. To layer in
heavy graphics + audio (`foundry-sprite`) or the full kit (`foundry-atelier`):

```bash
sudo apt install foundry-sprite     # + krita/gimp/inkscape/ardour/trackers
sudo apt install foundry-atelier    # + everything else (~10 GB)
```

## Develop on it

```bash
task build      # build the image locally  → ghcr.io/foundry-linux/devbox:local
task run        # shell into the local image
task smoke      # run the foundry-core tool-presence smoke test
task clean      # remove the local image
```

## Publish

```bash
# from the foundrylinux.org monorepo (this dir is mirrored to
# github.com/foundry-linux/foundry-devbox via task devbox-sync):
task devbox-sync                # sync foundry-devbox/ to the remote
task devbox-release TAG=v0.0.1  # tag the remote; CI builds + pushes GHCR
```

The publish workflow lives at `.github/workflows/publish.yml`. It triggers on
`v*` tag pushes, builds + pushes the image to GHCR under three tags
(`26.04`, the tag itself, and `latest`), and smoke-tests the pushed image by
running every `foundry-core` tool through `command -v`.

## What's NOT in this image

`foundry-sprite` (heavy graphics + audio production) and `foundry-atelier`
(everything else — ROM-bundled emulators, free games, mobile dev, etc.) are
*not* preinstalled. Users opt in inside the running container. The base image
stays at ~3.3 GB; atelier would push it past 10 GB.
