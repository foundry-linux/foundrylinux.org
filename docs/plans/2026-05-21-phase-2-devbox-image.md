# Phase 2 — Foundry Linux devbox container image

## Context

**Why now:** Phases 0 (curl-bash installer) and 1 (signed apt repos on R2) are complete. Phase 2 ships `ghcr.io/foundry-linux/devbox:26.04` — a Distrobox-compatible OCI image with the full WF authoring + retro-porting stack preinstalled. Users get a one-`distrobox create` working environment without touching their host OS; per-game work goes into ephemeral Distroboxen layered on top.

**What's already decided** (this conversation, 2026-05-21):

- **Scope:** base image only. The `wf-game-create` per-game tooling has been **deferred to its own follow-up plan** (to be written separately, e.g. `docs/plans/2026-05-21-phase-2-per-game-tooling.md`). Users can still create per-game Distroboxen by hand (`distrobox create -i ghcr.io/foundry-linux/devbox:26.04 -n wf-<game>`) until that helper ships.
- **Image contents — metapackages only (excluding `-dev` / `-development` suffixed Foundry/WF names) + a curated Ubuntu universe game-dev set:**

  **From `apt.foundrylinux.org`:**
  `foundry-retro-tools` (transitively brings the standalone-binary packages `f9dasm`, `ghidra`, `libvgm`, `vgmstream` plus Ubuntu's `mame`, `mame-tools`, `dasm`, `cc65`, `z80dasm`, `z80asm`, `radare2`, `binwalk`, `sox`, `binutils-m68k-linux-gnu`, `xa65`). Excluded: `foundry-android-development`, `foundry-ios-development`.

  **From `apt.worldfoundry.org`:**
  `worldfoundry`, `worldfoundry-cli`, `worldfoundry-blender-addons` (transitively brings the 10 WF CLIs, `worldfoundry-blender-editor-exporter`, `blender-asset-finder`, `blender-asset-finder-cli`, and the regular Ubuntu `blender` package). Excluded: `worldfoundry-development`.

  **From `apt.foundrylinux.org` — metapackages to author this round (NEW):**

  The devbox image installs only **metapackages we own** — no bare Ubuntu universe packages in the Dockerfile. The hierarchy mirrors the site's three edition names (Anvil / Sprite / Atelier), each a strict superset of the previous; underneath, composable sub-metapackages do the actual work.

  **Tier 0 — composable sub-metapackages (6):**
  - `foundry-emulators-computers` — vintage home-computer emulators that do NOT bundle non-redistributable ROMs in the .deb (universe-only): `dosbox-x`, `hatari`, `fs-uae`, `openmsx`, `openmsx-data`. (`vice`, `atari800`, `fbzx` ship Commodore/Atari/Sinclair ROMs inside the .deb — those live in the opt-in `foundry-emulators-vintage` metapackage so we don't redistribute non-redistributable ROMs in a publicly-pulled image.)
  - `foundry-emulators-consoles` — light console/handheld + adventure-engine emulators (universe): `fceux`, `mednafen`, `stella`, `desmume`, `scummvm`, `frotz`
  - `foundry-game-frameworks` — 2D/3D game-dev frameworks + headers: `tiled`, `libsdl2-dev`, `libsdl3-dev`, `libsfml-dev`, `liballegro5-dev`, `libtcod-dev`, `glslang-tools`, `spirv-cross`, `spirv-tools` (LÖVE 2D engine `love` is excluded — Ubuntu 26.04's `love` 11.5-3 has a broken postinst; will be re-added once upstream fixes it)
  - `foundry-trackers` — chiptune/module trackers: `milkytracker`, `schism`, `furnace`, `openmpt123`
  - `foundry-pixel-art` — pixel-art paint: `mtpaint`, `grafx2`
  - `foundry-image-cli` — image CLI: `imagemagick`, `graphicsmagick`

  (`task` is installed directly from the Cloudsmith repo — no wrapper metapackage. The Dockerfile's Layer 1 configures Cloudsmith, and the bare-metal `setup.sh` curl-bash does the same; once configured, `apt install task` resolves naturally.)

  **Tier 1 — opt-in extensions (7)** (full Depends lists in §"Companion metapackages on apt.foundrylinux.org"):
  - `foundry-art`, `foundry-daw`, `foundry-emulators-vintage`, `foundry-emulators-consoles-heavy`, `foundry-emulators` (umbrella), `foundry-game-reimplementations`, `foundry-free-games`

  **Tier 2 — nested editions (3)** (names match the site's Editions section; also intended to be the ISO image names later). Each edition is a strict superset of the previous: `foundry-anvil ⊆ foundry-sprite ⊆ foundry-atelier`. No separate "everything" umbrella — atelier IS everything.
  - `foundry-anvil` — **base layer**, the "Foundry Linux full kit" most users want. `Depends:` `worldfoundry` (cross-repo) + `foundry-retro-tools` + `foundry-emulators-computers` + `foundry-emulators-consoles` + `foundry-game-frameworks` + `foundry-image-cli` + `task` (direct from Cloudsmith) + Distrobox conveniences (`sudo libvte-2.91-0 bash-completion man-db`). **The devbox image installs this and nothing else** — a single `apt install foundry-anvil` resolves the entire stack. Bare-metal Foundry Linux users running the same command (after `setup.sh` wires up the apt sources) get the same set.
  - `foundry-sprite` — anvil + **heavy graphics + audio production**. `Depends:` `foundry-anvil` + `foundry-pixel-art` + `foundry-art` + `foundry-trackers` + `foundry-daw`. Adds krita/gimp/inkscape, the pixel-art tools, the trackers, and the full DAW stack.
  - `foundry-atelier` — sprite + **everything else**. `Depends:` `foundry-sprite` + `foundry-emulators-vintage` + `foundry-emulators-consoles-heavy` + `foundry-game-reimplementations` + `foundry-free-games` + `foundry-android-development` + `foundry-ios-development` + `worldfoundry-development` (cross-repo). The "give me the lot" install (~10 GB).

  **Source-wiring prereqs** (whose responsibility: Phase 0 install layer). Several metapackages Depend on packages that live in a source the user may not have configured by default. Three distinct missing-source failure modes, all resolved the same way — the Phase 0 install script wires up the needed sources before `apt install` runs:

  | Metapackage(s) | Source(s) required beyond apt.foundrylinux.org | Failure mode if missing | Phase 0 wiring step |
  |---|---|---|---|
  | Every edition (`foundry-anvil`, `-sprite`, `-atelier`) | apt.worldfoundry.org (cross-repo Depends on `worldfoundry`; atelier transitively on `worldfoundry-development`) | `E: Unable to locate package worldfoundry` | `setup-worldfoundry-apt-source.sh` (already exists) |
  | `foundry-emulators-vintage`, `foundry-game-reimplementations`, `foundry-android-development`, `foundry-atelier` (transitively) | Ubuntu multiverse component | `E: Unable to locate package vice` (or `Depends: vice but it is not installable`) | `add-apt-repository -y multiverse` (already used by the existing Android script at `install-foundry-android-development.sh:58-59`) |
  | Every edition (`task` is in anvil's Depends) | Cloudsmith `task` repo | `E: Unable to locate package task` | Cloudsmith setup (existing `install-task.sh` pattern) |

  The Dockerfile handles all three in Layer 1 before any `apt install`. On bare metal, the corresponding `foundry-setup/install-foundry-<edition>.sh` script does the same idempotent source-wiring up front. Phase 0 owns "make apt able to resolve our Depends"; Phase 1 metapackages get to declare Depends naively, without postinst hacks or maintainer-script source manipulation. **Add idempotent helpers to `foundry-setup/lib.sh` — `enable_multiverse()` and `setup_worldfoundry_source()` — so every new install script is a one-liner.**

  **Explicit:** `blender` itself (pulled transitively via `worldfoundry-blender-addons`, but listed for visibility / manual-installed marker). Ubuntu 26.04 universe ships **Blender 5.0.1+dfsg-1ubuntu1**, not 4.2 — `worldfoundry-blender-addons`'s `Depends: blender (>= 4.2.0)` is satisfied. The `worldfoundry-blender-editor-exporter` and `blender-asset-finder` add-ons were originally written for 4.2's Python API; 5.0 compatibility is tracked in WorldFoundry's TODO ("Verify worldfoundry-blender-editor-exporter + blender-asset-finder on Blender 5.0.1") and is a runtime concern, not a packaging blocker.

  **From Cloudsmith:** `task`.

  **Deliberately excluded from image preinstall** (per user direction):

  We **do not preinstall any package that ships non-redistributable ROMs, BIOS, or sample data inside the .deb itself.** The image is published to a public GHCR registry; redistributing non-redistributable bundled data through it is wrong. These stay out:
  - `vice` — Commodore KERNAL/CHAR/BASIC ROMs (Cloanto Corp)
  - `atari800` — Atari 8-bit OS ROMs
  - `fbzx` — ZX Spectrum ROM (Amstrad/Sky)
  - `mame-extra` — sample files + debug ROMs (mixed licensing; the core `mame` in universe is FOSS and IS preinstalled)

  **Ubuntu multiverse repo is enabled** in the image, so a user inside the container can `apt install vice atari800 fbzx mame-extra` themselves in one line — their choice, their legal posture, no ceremony required. We just don't pre-bundle the ROMs ourselves.

  Also excluded for **size** rather than licensing:
  - Free games + their massive data packs (`0ad` + 3.5 GB data, `supertuxkart` + 770 MB data, `flightgear` + ~3 GB data, etc.) — pulled via opt-in `foundry-free-games`.
  - Heavy console emulators (`dolphin-emu` 68 MB, `pcsx2` 50 MB, `yuzu` 39 MB, `ppsspp`, `retroarch-assets` 116 MB) — pulled via opt-in `foundry-emulators-consoles-heavy`.
  - Heavy DAWs (`ardour` 62 MB, etc.) — pulled via opt-in `foundry-daw`.
  - General raster/vector art (`krita` 98 MB, `gimp` 33 MB, `inkscape` 107 MB) — pulled via opt-in `foundry-art`.
  - FOSS engines that require commercial game data (`vcmi`, `openrct2`, `fheroes2`, `exult`, `sauerbraten` ~1 GB) — pulled via opt-in `foundry-game-reimplementations` (these don't bundle non-redistributable content, just need it user-provided at runtime; size is the reason for opt-in).
- **Source location:** monorepo subdir `foundry-devbox/` mirrored to `github.com/foundry-linux/foundry-devbox` via `task devbox-sync`, matching the existing `foundry-apt/` pattern. GHCR publish triggered by tag push on the remote.

**Image size — ~3.3 GB uncompressed:**

```
ghcr.io/foundry-linux/devbox:26.04
────────────────────────────────────────────────────────────────
ubuntu:26.04 base                                       250 MB
worldfoundry  +  worldfoundry-cli  +  worldfoundry-blender-addons
  ├─ blender 4.2+ (regular Ubuntu universe pkg)         555 MB
  ├─ worldfoundry-blender-editor-exporter add-on + blender-asset-finder add-on      1 MB
  └─ 10 WF CLIs (cdpack, iffcomp, levcomp, …)             4 MB
foundry-retro-tools
  ├─ ghidra + openjdk-21-jdk                          1.40 GB
  ├─ mame + mame-tools                                  500 MB
  ├─ blender-asset-finder-cli                            <1 MB
  └─ dasm/cc65/z80*/radare2/binwalk/sox/m68k/etc.        60 MB
Ubuntu universe game-dev additions (pulled by foundry-anvil) ~470 MB
  ├─ light emulators (dosbox-x, hatari, fs-uae, openmsx,
  │  openmsx-data, fceux, mednafen, stella, scummvm,
  │  desmume, frotz)                                   215 MB
  ├─ game-dev frameworks + SDL/SFML/Allegro/tcod
  │  headers + Vulkan shader tools                     245 MB
  └─ image CLI (imagemagick, graphicsmagick)             6 MB
task                                                    15 MB
────────────────────────────────────────────────────────────────
TOTAL                                                  ~3.3 GB
```

The Ubuntu universe additions cost ~470 MB on top of the bare ~2.8 GB Foundry-stack base. Everything in the universe-additions set is small individually — `scummvm` is the largest at 119 MB, followed by `spirv-tools` at 23 MB. Excluded from the image (live in `foundry-sprite`): trackers (`milkytracker`, `schism`, `furnace`) and pixel-art tools (`mtpaint`, `grafx2`) — adds ~35 MB when sprite is layered on. Also excluded (live in `foundry-atelier`): heavy art suites (krita, gimp, inkscape via `foundry-art`), DAWs (ardour, etc. via `foundry-daw`), ROM-bundled multiverse emulators (`vice`, `atari800`, `fbzx` via `foundry-emulators-vintage`), heavy console emulators (`dolphin-emu`, `pcsx2`, `yuzu` via `foundry-emulators-consoles-heavy`), free-game data packs (multi-GB via `foundry-free-games`), and the mobile-dev NDK + iOS tooling.

**Outcome:** `distrobox create -i ghcr.io/foundry-linux/devbox:26.04 -n foundry && distrobox enter foundry` drops the user into a working WF env in under a minute.

## Layout

```
foundry-devbox/                              # NEW monorepo subdir
  Dockerfile                                 # ubuntu:26.04 → apt install foundry-anvil (base edition)
  Taskfile.yml                               # build, run, push (local dev tasks)
  README.md
  .github/workflows/publish.yml              # tag-driven GHCR publish (built on the mirrored remote)
  test/
    smoke-test.sh                            # `docker run` the local image; assert tools on PATH
```

Top-level `Taskfile.yml` gains two tasks (mirroring the existing `sync` / `release` for foundry-apt):

- `task devbox-sync` — archive `foundry-devbox/` from HEAD, overlay on a fresh clone of `foundry-linux/foundry-devbox`, commit + push if anything changed.
- `task devbox-release TAG=v0.x.y` — tag the remote and trigger CI.

Reuse the **already-fixed sync detection** from `Taskfile.yml:30-37` (`git status --porcelain`, not `git diff --quiet`) so new files actually propagate — that fix landed in commit `3e85f90` for foundry-apt's sync.

## Critical files

- **Reuse:** `foundry-apt/.github/workflows/publish.yml` (tag-trigger + `dry_run` input pattern, secrets pattern), `Taskfile.yml:19-50` (sync/release tasks template), `foundry-setup/install-foundry-dev.sh:85-103` (exact `apt install` sequence to mirror in the Dockerfile), `foundry-setup/setup-foundry-apt-source.sh` / `setup-worldfoundry-apt-source.sh` (key+source format).
- **Reference (read-only):** `docs/investigations/2026-05-16-foundry-distro-proposal.md:562-591` (original Channel 2 spec — Dockerfile sketch + GHCR rationale). The per-game container pattern (§763-836) is the subject of the companion plan in `docs/plans/2026-05-21-phase-2-per-game-tooling.md`.

## Dockerfile (single-stage, ~3.3 GB)

```dockerfile
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
# Cloudsmith (for task)
#
# multiverse is enabled at the image level so users inside the running
# container can `apt install vice atari800 fbzx mame-extra` themselves
# without ceremony. We don't preinstall any of those — see "deliberately
# excluded from image preinstall" in the plan.
RUN apt-get update \
 && apt-get install -y --no-install-recommends \
        ca-certificates curl gnupg software-properties-common \
 && add-apt-repository -y multiverse \
 && install -d /etc/apt/keyrings \
 && curl -fsSL https://apt.foundrylinux.org/key.gpg \
      | gpg --dearmor -o /etc/apt/keyrings/foundry.gpg \
 && echo "deb [signed-by=/etc/apt/keyrings/foundry.gpg] https://apt.foundrylinux.org resolute main" \
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
#     ├── foundry-game-frameworks     = tiled libsdl2-dev libsdl3-dev libsfml-dev
#     │                                 liballegro5-dev libtcod-dev glslang-tools spirv-cross spirv-tools
#     ├── foundry-image-cli           = imagemagick graphicsmagick
#     ├── task                          (resolved directly from the Cloudsmith repo configured in Layer 1)
#     ├── worldfoundry [cross-repo]   = worldfoundry-cli + worldfoundry-blender-addons
#     │     ├── worldfoundry-cli      = cdpack iffcomp iffdump levcomp lvldump oaddump
#     │     │                           oas2oad textile blender-asset-finder-cli prep
#     │     └── worldfoundry-blender-addons  = worldfoundry-blender-editor-exporter + blender-asset-finder + blender (5.0.1)
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
```

**Layer ordering rationale:** Layer 1 is tiny + stable (apt sources rarely change); Layer 2 is the big ~3 GB metapackage install. When a new tag bumps any of the foundry-* metapackage versions, only Layer 2 rebuilds — Layer 1 stays cached.

`task` (Cloudsmith), `sudo`, `libvte-2.91-0`, `bash-completion`, `man-db` — all reachable through `foundry-anvil`'s `Depends:`. No bare apt-install lines in the Dockerfile; every package the image ships is reachable through a metapackage we own.

## GHCR publish workflow (lives in `foundry-devbox/.github/workflows/publish.yml`)

```yaml
name: Build and publish devbox image

on:
  push:
    tags: ['v*']
  workflow_dispatch:
    inputs:
      dry_run:
        description: 'Build only; do not push to GHCR'
        type: boolean
        default: false

permissions:
  contents: read
  packages: write

jobs:
  publish:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v6

      - uses: docker/setup-buildx-action@v3

      - name: Log in to GHCR
        if: ${{ !inputs.dry_run }}
        uses: docker/login-action@v3
        with:
          registry: ghcr.io
          username: ${{ github.actor }}
          password: ${{ secrets.GITHUB_TOKEN }}

      - uses: docker/build-push-action@v6
        with:
          context: .
          push: ${{ !inputs.dry_run }}
          tags: |
            ghcr.io/foundry-linux/devbox:26.04
            ghcr.io/foundry-linux/devbox:${{ github.ref_name }}
            ghcr.io/foundry-linux/devbox:latest
          cache-from: type=gha
          cache-to:   type=gha,mode=max

      - name: Smoke-check image
        if: ${{ !inputs.dry_run }}
        run: |
          docker pull ghcr.io/foundry-linux/devbox:26.04
          # Tools installed by foundry-anvil (the base edition the image ships).
          # Tools from foundry-sprite (trackers, pixel-art) and foundry-atelier
          # (heavy art/daw/emulators/mobile-dev) are NOT smoke-tested here —
          # they're not preinstalled in the image. Users opt in.
          for tool in \
              mame chdman ghidra ghidra-headless blender \
              vgmstream-cli f9dasm vgm-player vgm2wav task \
              cdpack iffcomp levcomp \
              dosbox-x scummvm fceux mednafen stella hatari fs-uae \
              openmsx frotz desmume-cli \
              tiled sdl2-config \
              glslangValidator spirv-cross spirv-val \
              magick gm; do
            docker run --rm -e PATH=/usr/local/bin:/usr/local/sbin:/usr/bin:/usr/sbin:/usr/games:/bin:/sbin \
              ghcr.io/foundry-linux/devbox:26.04 \
              bash -c "command -v $tool" || { echo "MISSING: $tool" >&2; exit 1; }
          done
```

Note: `/usr/games` in PATH inside the image (or via the run command above) — this is the lesson from `test-retro-tools-e2e-inner.sh`, where `mame` lives at `/usr/games/mame` per Debian games convention.

## Local Taskfile (`foundry-devbox/Taskfile.yml`)

```yaml
version: '3'
tasks:
  build:
    desc: "Build the image locally (no push)"
    cmds: [docker build -t ghcr.io/foundry-linux/devbox:local .]
  run:
    desc: "Shell into the locally-built image"
    cmds: [docker run --rm -it ghcr.io/foundry-linux/devbox:local]
  smoke:
    desc: "Smoke-check tools on PATH in the locally-built image"
    cmds: [bash test/smoke-test.sh]
```

## Verification

Run each step; paste raw output in a code block below it, then PASS/FAIL.

1. **Local build succeeds and image is the expected size.**
   ```
   task -d foundry-devbox build
   docker images ghcr.io/foundry-linux/devbox:local --format '{{.Size}}'
   ```
   Expected: build exits 0; size between 3.0 GB and 3.6 GB (target ~3.3 GB).

2. **Smoke test — all tools on PATH inside the locally-built image.**
   ```
   bash foundry-devbox/test/smoke-test.sh
   ```
   Expected: prints `✓ <tool>` for each foundry-anvil tool — Foundry stack: `mame`, `chdman`, `ghidra`, `ghidra-headless`, `blender`, `vgmstream-cli`, `f9dasm`, `vgm-player`, `vgm2wav`, `task`, `cdpack`, `iffcomp`, `levcomp`; universe emulators: `dosbox-x`, `scummvm`, `fceux`, `mednafen`, `stella`, `hatari`, `fs-uae`, `openmsx`, `frotz`, `desmume-cli`; game-dev frameworks: `tiled`, `sdl2-config`; shader/image: `glslangValidator`, `spirv-cross`, `spirv-val`, `magick`, `gm`. Final line "N/N tools verified" with N≈30. (Sprite/atelier tools — trackers, pixel-art, krita, ardour, dolphin, etc. — are NOT in the image and not smoke-tested.)

3. **Sync to the mirror remote.**
   ```
   task devbox-sync
   ```
   Expected: clones `foundry-linux/foundry-devbox`, overlays `foundry-devbox/`, commits + pushes (if not already up to date).

4. **Tag a release and confirm CI publishes.**
   ```
   task devbox-release TAG=v0.0.1
   gh run watch --repo foundry-linux/foundry-devbox
   ```
   Expected: workflow green; both build job and smoke step pass.

5. **Pull the published image and run a Distrobox.**
   ```
   docker pull ghcr.io/foundry-linux/devbox:26.04
   distrobox create -i ghcr.io/foundry-linux/devbox:26.04 -n foundry-test
   distrobox enter foundry-test -- bash -c 'mame -version && blender --version && task --version'
   distrobox rm -f foundry-test
   ```
   Expected: each command prints a real version line and exits 0.

6. **Universe game-dev additions usable.**
   ```
   distrobox enter foundry-test -- bash -c 'dosbox-x -version; tiled --version; sdl2-config --version; glslangValidator --version | head -1; magick -version | head -1'
   ```
   Expected: each emits a recognisable version banner; non-zero exits are OK as long as the binary launches.

7. **Full metapackage audit — every metapackage in both Foundry-family repos, with its Depends/Recommends/Suggests + dependency graph.**

   Audit script (run after CI publishes the new metapackages):

   ```
   for src in \
       https://apt.foundrylinux.org/dists/resolute/main/binary-amd64/Packages \
       https://apt.foundrylinux.org/dists/resolute/main/binary-all/Packages \
       https://apt.worldfoundry.org/dists/stable/main/binary-amd64/Packages \
       https://apt.worldfoundry.org/dists/stable/main/binary-all/Packages; do
     curl -fsSL "$src"
   done | awk -v RS="" '
     /^Section: metapackages/ {
       pkg=""; ver=""; arch=""; deps=""; rec=""; sug=""; prov=""; conf=""; maint=""
       n = split($0, lines, "\n")
       for (i=1; i<=n; i++) {
         if (lines[i] ~ /^Package:/)      { sub(/^Package: */,     "", lines[i]); pkg   = lines[i] }
         else if (lines[i] ~ /^Version:/)      { sub(/^Version: */,     "", lines[i]); ver   = lines[i] }
         else if (lines[i] ~ /^Architecture:/) { sub(/^Architecture: */,"", lines[i]); arch  = lines[i] }
         else if (lines[i] ~ /^Depends:/)      { sub(/^Depends: */,     "", lines[i]); deps  = lines[i] }
         else if (lines[i] ~ /^Recommends:/)   { sub(/^Recommends: */,  "", lines[i]); rec   = lines[i] }
         else if (lines[i] ~ /^Suggests:/)     { sub(/^Suggests: */,    "", lines[i]); sug   = lines[i] }
         else if (lines[i] ~ /^Provides:/)     { sub(/^Provides: */,    "", lines[i]); prov  = lines[i] }
         else if (lines[i] ~ /^Conflicts:/)    { sub(/^Conflicts: */,   "", lines[i]); conf  = lines[i] }
         else if (lines[i] ~ /^Maintainer:/)   { sub(/^Maintainer: */,  "", lines[i]); maint = lines[i] }
       }
       if (pkg != "") {
         print "## " pkg " " ver " (" arch ")"
         print "  Maintainer:  " maint
         print "  Depends:     " deps
         if (rec  != "") print "  Recommends:  " rec
         if (sug  != "") print "  Suggests:    " sug
         if (prov != "") print "  Provides:    " prov
         if (conf != "") print "  Conflicts:   " conf
         print ""
       }
     }
   '
   ```

   Expected output: structured audit of every metapackage (after Phase 2 publishes, ~23 entries — three existing on foundry-apt, four existing on apt.worldfoundry.org, plus the 16 new ones being added by this plan). Fields shown: Package, Version, Architecture, Maintainer, Depends, Recommends, Suggests, Provides, Conflicts (the latter three only when non-empty).

   **(a) Editions — nested-set view** (each outer box strictly contains the inner ones, mirroring `foundry-anvil ⊆ foundry-sprite ⊆ foundry-atelier`; `↗` marks a package whose source is not apt.foundrylinux.org — `worldfoundry`/`worldfoundry-development` are cross-repo to apt.worldfoundry.org, `task` is from Cloudsmith):

   ```
   ┌─ foundry-atelier (22 pkgs total) ─────────────────────────────────┐
   │  foundry-emulators-vintage · foundry-emulators-consoles-heavy ·   │
   │  foundry-game-reimplementations · foundry-free-games ·            │
   │  foundry-android-development · foundry-ios-development ·          │
   │  worldfoundry-development ↗                                       │
   │ ┌─ foundry-sprite (15 pkgs total) ──────────────────────────────┐ │
   │ │  foundry-pixel-art · foundry-art · foundry-trackers ·         │ │
   │ │  foundry-daw                                                  │ │
   │ │ ┌─ foundry-anvil (11 pkgs total) ────────────────────────────┐│ │
   │ │ │  worldfoundry ↗ · foundry-retro-tools ·                    ││ │
   │ │ │  foundry-emulators-computers · foundry-emulators-consoles ·││ │
   │ │ │  foundry-game-frameworks · foundry-image-cli ·             ││ │
   │ │ │  task ↗ · sudo · libvte-2.91-0 · bash-completion · man-db  ││ │
   │ │ └────────────────────────────────────────────────────────────┘│ │
   │ └───────────────────────────────────────────────────────────────┘ │
   └───────────────────────────────────────────────────────────────────┘
   ```

   **(b) Opt-in extensions — internal structure:**

   ```mermaid
   graph LR
     ART[foundry-art<br/>krita, gimp,<br/>inkscape, mypaint]
     DAW[foundry-daw<br/>ardour, audacity,<br/>lmms, rosegarden, …]
     EMU[foundry-emulators<br/>umbrella]
     REIMPL[foundry-game-reimplementations<br/>vcmi, openrct2,<br/>fheroes2, exult, …]
     GAMES[foundry-free-games<br/>0ad, supertuxkart,<br/>flightgear, +data]

     EMU_C[foundry-emulators-computers<br/>dosbox-x, hatari, …]
     EMU_K[foundry-emulators-consoles<br/>fceux, scummvm, …]
     TRACKERS[foundry-trackers<br/>milkytracker, …]
     PIXEL[foundry-pixel-art<br/>mtpaint, grafx2]
     IMGCLI[foundry-image-cli<br/>imagemagick, gm]
     EMU_V[foundry-emulators-vintage<br/>vice, atari800,<br/>fbzx multiverse]
     EMU_KH[foundry-emulators-consoles-heavy<br/>dolphin, pcsx2,<br/>yuzu, retroarch]

     ART --> PIXEL
     ART --> IMGCLI
     DAW --> TRACKERS
     EMU --> EMU_C
     EMU --> EMU_K
     EMU --> EMU_V
     EMU --> EMU_KH

     classDef tier0 fill:#1f4e1f,stroke:#3fb950,color:#e6edf3
     classDef optin fill:#1f3d4e,stroke:#58a6ff,color:#e6edf3
     class EMU_C,EMU_K,TRACKERS,PIXEL,IMGCLI tier0
     class ART,DAW,EMU,REIMPL,GAMES,EMU_V,EMU_KH optin
   ```

   (No separate `foundry-everything` umbrella — `foundry-atelier` *is* everything; see graph (a).)

   **Dependency graph — worldfoundry-* metapackage tree:**

   ```mermaid
   graph TD
     WF[worldfoundry<br/>**in devbox image**]
     WF --> WFCLI[worldfoundry-cli<br/>**in devbox image**]
     WF --> WFB[worldfoundry-blender-addons<br/>**in devbox image**]
     WFCLI --> CLIS[10 CLI binaries:<br/>cdpack iffcomp iffdump levcomp lvldump<br/>oaddump oas2oad textile blender-asset-finder-cli prep]
     WFB --> WFBLENDER[worldfoundry-blender-editor-exporter extension]
     WFB --> BAF[blender-asset-finder extension]
     WFB --> BLENDER[blender 5.0.1<br/>Ubuntu universe]
     WFDEV[worldfoundry-development<br/>*NOT in devbox image* — engine-dev tier] --> WF
     WFDEV --> EBD[build-essential, cmake 3.22+, libx11-dev,<br/>libgl-dev, libglu1-mesa-dev, python3]

     classDef base fill:#1f4e1f,stroke:#3fb950,color:#e6edf3
     classDef separate fill:#4e1f1f,stroke:#f85149,color:#e6edf3
     class WF,WFCLI,WFB base
     class WFDEV separate
   ```

   **Cross-repo dependency graph — what the devbox image actually pulls:**

   ```mermaid
   graph TD
     IMG[ghcr.io/foundry-linux/devbox:26.04]
     IMG --> ANVIL[foundry-anvil<br/>apt.foundrylinux.org]
     ANVIL --> R[8 Tier 0 sub-metapackages +<br/>retro-tools + distrobox conveniences]
     ANVIL --> WF[worldfoundry<br/>apt.worldfoundry.org<br/><i>cross-repo</i>]
     WF --> WFCLI[worldfoundry-cli]
     WF --> WFB[worldfoundry-blender-addons]

     classDef img fill:#3d2f1f,stroke:#d29922,color:#e6edf3
     class IMG img
   ```

   Expected: all three graphs render in any markdown viewer with mermaid support (the `task md` preview does); the audit text output matches the expected metapackage list inline above.

## Companion metapackages on `apt.foundrylinux.org`

The base devbox image installs a tight curated set. Everything *not* in the base — heavy art suites, free games + multi-GB data packs, full DAW stacks, the long tail of emulators — lives behind opt-in **metapackages on `apt.foundrylinux.org`**. Users `apt install foundry-<group>` from inside the running devbox (or off the Phase 1 install script on bare metal) to add a category in one shot.

Why this shape: case-by-case include/exclude decisions for every Ubuntu package don't scale — each one's "is it worth the disk?" is its own conversation. Grouping by user intent (art, games, DAW, emulators) means the decision is "does the user want art tools?" not "does the user want krita and gimp and inkscape and mypaint and tuxpaint…". The metapackages are published from `foundry-apt/packages/<name>/debian/` like every other foundry-* metapackage; they ship arch:all and are tiny `.deb`s (~8 KB) that pull everything through `Depends:`.

Sizes below assume installation on top of the ~3.3 GB base; numbers are uncompressed.

### `foundry-art` — raster/vector/pixel-art (+~265 MB beyond base)

Heavy art suites + the pixel-art tools already in the base. (Base-included tools are listed here too so users can install just this metapackage on a non-devbox machine and get the same set.)

```
Depends: krita,        # 98 MB — pixel-based, sprite art, tablet support
         gimp,         # 33 MB — raster
         inkscape,     # 107 MB — vector
         mypaint,      # 3.5 MB — tablet-friendly painting
         mtpaint,      # 2.1 MB — pixel-art focused
         grafx2,       # 2 MB — 256-color Deluxe-Paint style
         tuxpaint,     # 647 KB — kid-friendly entry point
         imagemagick,  # CLI (also in base)
         graphicsmagick
```

### `foundry-daw` — Digital Audio Workstation + trackers (+~140 MB beyond base)

Full DAW stack on top of the trackers already in the base. Trackers (MilkyTracker, Schism, Furnace, openmpt123) are also listed here so the metapackage is self-sufficient on a bare-metal install.

```
Depends: ardour,        # 62 MB — pro multitrack DAW (the FOSS Logic/Reaper)
         audacity,      # 26 MB — basic editor
         lmms,          # 14 MB — Linux Multimedia Studio (FL Studio-like)
         rosegarden,    # 14 MB — MIDI + audio sequencer
         hydrogen,      # 12 MB — drum machine / step sequencer
         qtractor,      # 8 MB  — MIDI/audio sequencer
         mixxx,         # 16 MB — DJ interface
         milkytracker,  # tracker (also in base)
         schism,        # tracker (also in base)
         furnace,       # multi-chip chiptune (also in base)
         openmpt123     # module player CLI (also in base)
```

### Emulator metapackages

Two of these — `foundry-emulators-computers` and `foundry-emulators-consoles` — are **already in the base** image (defined under "From `apt.foundrylinux.org` — base-level metapackages"). They contain the universe-only emulators that don't bundle non-redistributable ROMs. The opt-in metapackages defined below extend that base with the heavier and/or ROM-bundling content.

#### `foundry-emulators-vintage` — opt-in (~50 MB; ROM-bundled multiverse)

Vintage home-computer emulators whose .deb ships non-redistributable ROMs. Users opt in explicitly — this metapackage is **not** preinstalled in any image we publish.

```
Depends: vice,            # multiverse — Commodore 64/128/VIC-20/PET (bundles KERNAL/CHAR/BASIC)
         atari800,        # multiverse — Atari 8-bit XL/XE (bundles OS ROMs)
         fbzx,            # multiverse — ZX Spectrum (bundles ROM)
         mame-extra       # multiverse — MAME sample files + debug ROMs (mixed licensing)
```

#### `foundry-emulators-consoles-heavy` — opt-in (~270 MB; heavy console emulators)

Heavy console/handheld emulators left out of the base for size reasons (all universe; no licensing issue).

```
Depends: # SNES
         snes9x-gtk,                  # SNES
         nestopia,                    # alternative NES
         # Sony
         pcsxr,                       # PSX
         pcsx2,                       # PS2 (~50 MB)
         ppsspp,                      # PSP
         # Nintendo handhelds + later
         mupen64plus-ui-console,      # N64
         dolphin-emu,                 # GameCube + Wii (~68 MB)
         yuzu,                        # Switch (~39 MB)
         # Frontend
         retroarch,                   # libretro frontend (~24 MB)
         retroarch-assets             # libretro UI assets (~116 MB)
```

#### `foundry-emulators` (umbrella) — opt-in; pull every emulator we package

Top-level emulator umbrella. Pulls the two base metapackages (so a user installing on bare metal gets the lot) plus the two opt-in extensions.

```
Depends: foundry-emulators-computers,         # base — light universe computers
         foundry-emulators-consoles,          # base — light universe consoles + adventure engines
         foundry-emulators-vintage,           # opt-in — ROM-bundled multiverse
         foundry-emulators-consoles-heavy     # opt-in — heavy consoles
```

Size delta when installed on top of the devbox base: `-emulators-vintage` ≈ +50 MB, `-emulators-consoles-heavy` ≈ +270 MB; umbrella delta ≈ +320 MB (the two base metapackages are already installed).

### `foundry-game-reimplementations` — FOSS engines that require original commercial game data (+~1.1 GB beyond base)

FOSS engine code; the user must legally own the original commercial game's data files (ROMs, IFF, .pak archives, etc.). These packages are all in Ubuntu **multiverse** specifically *because* of the data requirement — the engine code itself is fine. (`multiverse` is enabled in the devbox by default, so installation Just Works once this metapackage is requested.)

```
Depends: vcmi,                       # Heroes of Might & Magic 3 engine (~39 MB)
         openrct2,                   # RollerCoaster Tycoon 2 engine (~28 MB)
         openrct2-objects,           # required object library (~16 MB)
         openrct2-title-sequences,   # title-screen sequences (~8.6 MB)
         fheroes2,                   # Heroes of Might & Magic 2 engine (~11 MB)
         exult,                      # Ultima VII engine (~21 MB)
         sauerbraten                 # Cube 2 game/engine (~1 GB with bundled assets)
```

After installing, users drop their owned game data into the per-engine data directory (`~/.vcmi/Data/`, `~/.config/OpenRCT2/`, `~/.fheroes2/data/`, etc.) — each engine documents its own path. The metapackage doesn't bundle any commercial data and never will; that's the user's bring-your-own input.

### `foundry-free-games` — bundled FOSS games + data packs (+~6 GB beyond base)

Massive — installs ~6 GB once data packs land. Deliberately separate from the base so the devbox image stays sub-4 GB.

```
Depends: 0ad, 0ad-data,                                   # 29 MB + 3.5 GB data
         supertuxkart, supertuxkart-data,                 # 11 MB + 770 MB data
         supertux, supertux-data,                         # 4.4 MB + 238 MB data
         widelands, widelands-data,                       # 13 MB + 598 MB data
         warzone2100, warzone2100-data,                   # 16 MB + 292 MB data
         openttd,                                         # 13 MB
         freeciv,                                         # 114 KB (+ data subpackages)
         freedoom, freedm,                                # 59 + 24 MB
         hedgewars, hedgewars-data,                       # 28 + 160 MB
         endless-sky, endless-sky-data, endless-sky-high-dpi,
         wesnoth, wesnoth-1.18-data, wesnoth-1.18-music,  # 6 KB + 191 + 152 MB
         nethack-console, crawl-tiles,                    # 3.2 + 14.7 MB
         flightgear, flightgear-data-base, flightgear-data-ai, flightgear-data-models
                                                          # 46 MB + ~1.7 GB data
```

(Some of these may need a recursive split — e.g. `foundry-free-games-flight-sim` if flightgear's 1.7 GB is overkill — but the initial cut is one big bundle.)

### Implementation cost

Each metapackage is a tiny native source-package tree at `foundry-apt/packages/foundry-<name>/debian/{control,changelog,rules,source/format,copyright}` — same template as `foundry-retro-tools`. New metapackages to author this round:

**Tier 0 — composable sub-metapackages (6):**

1. `foundry-emulators-computers` — Depends: `dosbox-x hatari fs-uae openmsx openmsx-data`
2. `foundry-emulators-consoles` — Depends: `fceux mednafen stella desmume scummvm frotz`
3. `foundry-game-frameworks` — Depends: `tiled libsdl2-dev libsdl3-dev libsfml-dev liballegro5-dev libtcod-dev glslang-tools spirv-cross spirv-tools` (love excluded — broken upstream on 26.04)
4. `foundry-trackers` — Depends: `milkytracker schism furnace openmpt123`
5. `foundry-pixel-art` — Depends: `mtpaint grafx2`
6. `foundry-image-cli` — Depends: `imagemagick graphicsmagick`

(`task` ships directly from Cloudsmith — no `foundry-task` wrapper. `foundry-anvil`'s `Depends:` lists `task`; users configure Cloudsmith once via `setup.sh` or the Dockerfile.)

**Tier 1 — opt-in extensions (7):**

7. `foundry-art` — heavy raster/vector art (krita + gimp + inkscape + mypaint + tuxpaint, plus everything in -pixel-art and -image-cli for bare-metal self-sufficiency)
8. `foundry-daw` — DAW stack (ardour + audacity + lmms + rosegarden + hydrogen + qtractor + mixxx, plus -trackers)
9. `foundry-emulators-vintage` — ROM-bundled multiverse (vice atari800 fbzx mame-extra)
10. `foundry-emulators-consoles-heavy` — heavy console emulators (dolphin pcsx2 yuzu etc.)
11. `foundry-emulators` — umbrella; Depends: -computers + -consoles + -vintage + -consoles-heavy
12. `foundry-game-reimplementations` — BYO-data FOSS engines (vcmi openrct2 + objects + title-sequences fheroes2 exult sauerbraten)
13. `foundry-free-games` — bundled FOSS games + data packs

**Tier 2 — nested editions (3)** (names match the site's Editions section + future ISO image names; each a strict superset of the previous):

14. `foundry-anvil` — **base layer**. Depends: `worldfoundry` (cross-repo) + `foundry-retro-tools` + `foundry-emulators-computers` + `foundry-emulators-consoles` + `foundry-game-frameworks` + `foundry-image-cli` + `task` (Cloudsmith) + `sudo libvte-2.91-0 bash-completion man-db`. **The devbox image installs this and nothing else.**
15. `foundry-sprite` — **anvil + heavy graphics + audio**. Depends: `foundry-anvil` + `foundry-pixel-art` + `foundry-art` + `foundry-trackers` + `foundry-daw`.
16. `foundry-atelier` — **sprite + everything else**. Depends: `foundry-sprite` + `foundry-emulators-vintage` + `foundry-emulators-consoles-heavy` + `foundry-game-reimplementations` + `foundry-free-games` + `foundry-android-development` + `foundry-ios-development` + `worldfoundry-development` (cross-repo). Equivalent to the old "give me the lot" install (~10 GB).

Total: **16 new metapackages**. The existing `scripts/build-all.sh` builds them automatically once their dirs exist. Roughly **1 hour of authoring + 1 tag-push + CI rebuild** to publish all 16. CI smoke-installs them in a fresh ubuntu:26.04 container after publish (just like the existing retro-tools / android-development / ios-development metapackages).

## Ubuntu multiverse — what it is, why it matters here

`multiverse` is the Ubuntu archive component for software with **non-free licensing or redistribution constraints**. Packages in multiverse are typically one of:

1. **Emulators that ship non-redistributable ROMs or BIOS** — `vice` carries Commodore KERNAL/CHAR/BASIC ROMs (still owned by Cloanto Corp); `atari800` carries Atari 8-bit OS ROMs; `fbzx` ships the ZX Spectrum ROM (Amstrad/Sky). Emulator code is FOSS but the bundled ROM data isn't, hence multiverse.
2. **FOSS engines that *require* original commercial game data** — `vcmi` (Heroes 3), `openrct2` (RollerCoaster Tycoon 2), `fheroes2` (Heroes 2), `exult` (Ultima 7), `sauerbraten` (some assets). The code is fine; you legally need to own the original game.
3. **`mame-extra`** — sample files and debugging ROMs with mixed licensing; the core mame package (in universe) is FOSS, but the extras live in multiverse.

Practical consequences for the devbox:

- **The multiverse component IS enabled in the image** (via `add-apt-repository multiverse` in Layer 1 of the Dockerfile). That means a user inside the running container can `apt install vice atari800 fbzx mame-extra` themselves with no extra setup. Same for `vcmi`, `openrct2`, `fheroes2`, `exult`, `sauerbraten`.
- **What we do NOT preinstall in the image**: packages that bundle non-redistributable ROM/BIOS data inside the .deb — `vice` (Commodore KERNAL/CHAR/BASIC), `atari800` (Atari 8-bit OS), `fbzx` (ZX Spectrum), `mame-extra` (mixed-license samples). The image is on a public GHCR registry; redistributing bundled non-redistributable data through it is not OK. Users grabbing those packages from multiverse themselves is exactly the legal posture Ubuntu intends.
- **What we DO publish as opt-in metapackages**: `foundry-emulators-vintage` (vice/atari800/fbzx/mame-extra) and `foundry-game-reimplementations` (vcmi/openrct2/fheroes2/exult/sauerbraten — these don't bundle non-redistributable data, they require user-provided commercial game files at runtime). Installing them is the user's choice; the metapackages just bundle related packages for convenience.

Bottom line: **multiverse repo enabled, ROM-bundled packages excluded from preinstall**, opt-in metapackages publish the same convenient groupings users would otherwise type by hand.

## Known concerns / external dependencies

- **Blender 5.0.1 vs WF add-on compatibility.** Ubuntu 26.04 ships Blender 5.0.1; WF's `worldfoundry-blender-editor-exporter` + `blender-asset-finder` add-ons were authored against 4.2. Tracked in `../WorldFoundry-wbniv/TODO.md` under §TOOLS. If 5.0 breaks the add-ons, the fix lives in the WorldFoundry repo, not here.

## Out of scope (follow-up plans)

- **Per-game tooling** (`wf-game-create`, per-game Distrobox scaffolding, asset isolation, per-game Claude permissions, ROM library conventions) from proposal §763-836 — its own plan file to be created next.
- Maintainer-tier image (`:26.04-maintainer` with android-development + ios-development; +2.5 GB) — deferred until someone actually needs the NDK preinstalled.
- ROM-bundled Ubuntu **multiverse** emulators (`vice`, `atari800`, `fbzx`, `mame-extra`) — opt-in via `foundry-emulators-vintage`. The multiverse repo IS enabled in the image so users can `apt install <pkg>` directly without needing this metapackage.
- `apt.worldfoundry.org`'s builder Dockerfile still being `debian:bookworm-slim` (sibling-repo concern, already flagged earlier). Not blocking Phase 2, but for a more specific reason than first stated: the umbrella metapackages we install (`worldfoundry`, `worldfoundry-cli`, `worldfoundry-blender-addons`, `worldfoundry-development`) are themselves `Architecture: all`, **but** they `Depends:` transitively on 10 `Architecture: amd64` binaries (`cdpack`, `iffcomp`, `iffdump`, `levcomp`, `lvldump`, `oaddump`, `oas2oad`, `prep`, `textile`, `worldfoundry-blender-addons-editor-exporter`) — so the arch tag of the wrappers alone isn't the defense. The actual reason this hasn't bitten is that every one of those arch-specific binaries currently links only to `libc` / `python3` / minimal libgcc, all forward-compatible from bookworm to resolute. **Resolved on the worldfoundry side**: see `../../worldfoundry.org/docs/plans/2026-05-21-apt-builder-ubuntu-26-04.md`, executed 2026-05-21 (apt-v0.1.17).
- A `release-sniper` companion image for Steam builds (proposal §587; explicitly its own initiative).
