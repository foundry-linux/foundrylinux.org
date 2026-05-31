---
title: Foundry Linux & WorldFoundry package inventory
date: 2026-05-22
status: snapshot — both apt repos now live; supersedes 2026-05-19-package-inventory.md
---

# Package inventory

There are **two distinct APT repositories** in the Foundry/WorldFoundry universe:

| Repository | Status | Scope |
|---|---|---|
| **`apt.worldfoundry.org`** | ✅ live (Cloudflare R2, signed) | WorldFoundry game-authoring tooling (CLIs + Blender add-ons + umbrella metapackages) |
| **`apt.foundrylinux.org`** | ✅ live (Cloudflare R2, signed) | Broader Foundry Linux distro packages (vendored upstreams + creative-software + dev metapackages) |

Plus **Phase 0** bash installer scripts in `foundry-setup/` that bridge "stock Ubuntu" to "WF-ready dev box" until Phase 1's apt repos cover everything.

This document inventories **all three** so we can see what's shipped, what's drafted, and where the gaps are. The 2026-05-19 doc captured the moment apt.foundrylinux.org was still 🚧 planned and apt.worldfoundry.org was at its pre-rename / pre-ubuntu:26.04-rebuild state; today both repos are live, end-to-end install on stock ubuntu:26.04 is verified, and the metapackage names + version pins reflect the matrix-validated 4.0.2-through-5.0.1 Blender support.

---

## Repository 1: `apt.worldfoundry.org` — live

| | |
|---|---|
| **URL** | https://apt.worldfoundry.org |
| **Suite / Codename** | `stable` / `stable` |
| **Components** | `main` |
| **Architectures** | `all`, `amd64`, `arm64` |
| **Signing key** | `0xA51CECB8676E2158` — fingerprint `F19FF3274278148D5EE57FB3A51CECB8676E2158` (rsa4096, 2026-05-18, expires 2028-05-17) |
| **Maintainer** | WorldFoundry Packages `<packages@worldfoundry.org>` |
| **Layout** | Classic Debian: `/dists/stable/main/binary-{amd64,arm64,all}/Packages` + `/pool/main/<l>/<name>/` |
| **Hosting** | Cloudflare R2 |
| **Release date (snapshot)** | 2026-05-21 18:22:54 UTC (apt-v0.1.37) |

Add it with deb822:

```
Types: deb
URIs: https://apt.worldfoundry.org
Suites: stable
Components: main
Signed-By: /etc/apt/keyrings/worldfoundry.gpg
Architectures: amd64 arm64
```

### Metapackages (4)

| Package | Version | Arch | Depends |
|---|---|---|---|
| `worldfoundry` | 1.1.4 | amd64 | `worldfoundry-cli (>=1.0.0)`, `worldfoundry-blender-addons (>=1.1.2)` — full game-dev install (umbrella) |
| `worldfoundry-cli` | 1.0.3 | amd64 | `cdpack`, `iffcomp`, `iffdump`, `levcomp`, `lvldump`, `oaddump`, `oas2oad`, `textile`, `blender-asset-finder-cli`, `prep` — headless CI / build-server set, no Blender |
| `worldfoundry-blender-addons` | 1.1.3 | amd64 | `worldfoundry-blender-editor-exporter (>=0.2.1)`, `blender-asset-finder (>=0.2.0)`, `blender (>=4.0.2), blender (<<5.1)` — artist workstation, no compile chain. **Renamed from `worldfoundry-blender` (2026-05-21)**; Blender pin matrix-validated across 4.0.2, 4.2.21, 4.5.10, 5.0.1. |
| `worldfoundry-development` | 1.0.4 | amd64 | `worldfoundry (>=1.1.4)`, `build-essential`, `cmake (>=3.22)`, `libx11-dev`, `libgl-dev`, `libglu1-mesa-dev`, `python3`; **Recommends:** `gdb`, `xxd`; **Suggests:** `openjdk-17-jdk`, `adb` — for engine contributors |

### Binary packages (10)

All amd64. Built from `git0a19d26c` (or `git3fa94cbe` for `worldfoundry-blender-editor-exporter`) and rebuilt for ubuntu:26.04 — note the `1foundry2` revision (was `1foundry1` in the 05-19 snapshot).

| Package | Version | Arch | Section | What it does |
|---|---|---|---|---|
| `cdpack` | 0.1.0+git0a19d26c-1foundry2 | amd64 | utils | CD master-image packer — assembles a WorldFoundry `GAME` cd.iff from a SHEL Forth script + N compiled level IFFs; sector-aligned (2 KB) output |
| `iffcomp` | 0.1.0+git0a19d26c-1foundry2 | amd64 | devel | IFF text-DSL compiler — `.iff.txt` source → binary IFF chunks; drop-in for the legacy C++/Go iffcomp |
| `iffdump` | 0.1.0+git0a19d26c-1foundry2 | amd64 | utils | Inverse of iffcomp — binary `.iff` → text DSL; lossless round-trip with iffcomp |
| `levcomp` | 0.1.0+git0a19d26c-1foundry2 | amd64 | devel | `.lev` + `.oad` blobs → `.lvl` level file; replaces legacy DOS/Watcom iff2lvl |
| `lvldump` | 0.1.0+git0a19d26c-1foundry2 | amd64 | utils | Inverse of levcomp — `.lvl` → human-readable text (object list, OAD values, geometry refs) |
| `oaddump` | 0.1.0+git0a19d26c-1foundry2 | amd64 | utils | `.oad` blob inspector — prints attributes resolved against OAS schemas |
| `oas2oad` | 0.1.0+git0a19d26c-1foundry2 | amd64 | devel | OAS schema → OAD blob compiler; Linux successor to DOS Watcom pigtool. **Runtime deps:** `prep`, `gcc`, `binutils` |
| `prep` | 0.103+git0a19d26c-1foundry2 | amd64 | devel | Kevin T. Seghetti's token-stream macro preprocessor (used inside oas2oad's pipeline). Currently a binary re-pack; lex/yacc grammar restoration is a follow-up |
| `textile` | 0.1.0+git0a19d26c-1foundry2 | amd64 | graphics | Texture-atlas packer — N input images (TGA/BMP/PNG) → packed atlas + UV table |
| `worldfoundry-blender-editor-exporter` | 0.2.1+git3fa94cbe-2foundry2 | amd64 | graphics | World Foundry Blender add-on — dynamic OAD property panel (from Rust schema descriptors), validation, `.lev` text-IFF level export. Ships extension zip + `wf-blender-install` wrapper. Bundles the native `wf_core` Rust extension (pyo3 abi3). **Renamed from `wf-blender` (2026-05-21).** |

### arm64 coverage

`arm64` is announced in the Release file but **currently empty**: the binary-arm64 Packages file is 0 bytes. The 4 metapackages declare `Architecture: amd64` (the 05-19 snapshot still had them as `all` — they were corrected for honesty against the transitive amd64 graph). No package in this repo currently has an arm64 build. arm64 stays in the suite metadata as a placeholder for the eventual cross-build effort.

---

## Repository 2: `apt.foundrylinux.org` — live

| | |
|---|---|
| **URL** | https://apt.foundrylinux.org |
| **Suite / Codename** | `resolute` / `resolute` (matches Ubuntu 26.04 codename) |
| **Components** | `main` |
| **Architectures** | `all`, `amd64`, `arm64` |
| **Signing key** | `0x1CFB48FC928F7B21` — fingerprint `47A48E026F7509560033A8691CFB48FC928F7B21` (rsa4096, 2026-05-17, expires 2028-05-16) |
| **Maintainer** | Foundry Linux Packages `<packages@foundrylinux.org>` |
| **Layout** | Classic Debian: `/dists/resolute/main/binary-{amd64,arm64,all}/Packages` + `/pool/main/<l>/<name>/` |
| **Hosting** | Cloudflare R2 |
| **Release date (snapshot)** | 2026-05-21 16:58:26 UTC |

Add it with deb822:

```
Types: deb
URIs: https://apt.foundrylinux.org
Suites: resolute
Components: main
Signed-By: /etc/apt/keyrings/foundrylinux.gpg
Architectures: amd64 arm64
```

**End-to-end cross-repo install verified today** (2026-05-22) — fresh `ubuntu:26.04` container with both apt sources configured installs `worldfoundry-blender-addons` cleanly, with `blender-asset-finder` resolving from this repo and `blender 5.0.1+dfsg-1ubuntu1` resolving from Ubuntu universe.

### Vendored upstreams (4)

| Package | Pkg version | Upstream | Why vendored |
|---|---|---|---|
| `f9dasm` | 1.83-1foundry3 | github.com/Arakula/f9dasm | Not in Ubuntu universe; needed for 6800/6809 ROM disassembly |
| `ghidra` | 12.1-1foundry1 | ghidra-sre.org | NSA ships ZIPs only; pre-built x86_64 decompiler. Depends: `openjdk-21-jdk` |
| `libvgm` | 0.1+git20260406.d115188-1foundry1 | github.com/ValleyBell/libvgm | Git-tip only, no upstream releases. Chip-register VGM playback library + player utilities |
| `vgmstream` | 2083-1foundry4 | github.com/vgmstream/vgmstream | Not in Ubuntu universe. 100+ game-audio formats → WAV. ATRAC9/FSB CELT/G.719 deliberately excluded (Windows-only DLLs) |

### Blender add-ons + CLI companion (2)

Moved here from `apt.worldfoundry.org` per the 2026-05-20 reorg — these are not WorldFoundry-specific, they work with any Blender project / asset workflow.

| Package | Version | Arch | What it does |
|---|---|---|---|
| `blender-asset-finder` | 0.2.0+git0a19d26c-2foundry1 | all | Blender extension — multi-provider online 3D-asset search (Polyhaven, Kenney, AmbientCG, Quaternius, OpenGameArt, Sketchfab) with per-blend licence policy + `manifest.json` provenance |
| `blender-asset-finder-cli` | 0.1.0+git0a19d26c-1foundry1 | all | CLI companion (renamed from `wf-asset`) — same asset-provider library, shell front-end. Subcommands: `search`, `download`, `policy`, `providers` |

### Retro toolchain metapackage (1)

| Package | Version | Arch | Depends |
|---|---|---|---|
| `foundry-retro-tools` | 1.0.6 | all | `mame`, `mame-tools`, `dasm`, `cc65`, `z80dasm`, `z80asm`, `radare2`, `binwalk`, `sox`, `binutils-m68k-linux-gnu`, `f9dasm`, `libvgm`, `vgmstream`, `ghidra`. Arcade reverse-engineering + 6502/Z80/68k/6809 porting toolchain. xa65 was dropped (now resolved from 26.04 universe directly per the 2026-05-18 retire-xa65 plan). |

### Creative-software metapackages (15)

A family of `foundry-*` umbrella metapackages new since the 05-19 snapshot. Each pulls a curated set of Ubuntu universe packages for a creative-workflow vertical; designed to compose for the Foundry Linux distro experience.

| Package | Version | Scope |
|---|---|---|
| `foundry-anvil` | 1.0.0 | Foundry Linux base layer — full kit, devbox image preinstall |
| `foundry-atelier` | 1.0.0 | Foundry Linux complete edition — everything |
| `foundry-sprite` | 1.0.0 | Foundry Linux retro / graphics / audio edition (anvil + heavy art + audio) |
| `foundry-art` | 1.0.0 | Raster, vector, and pixel-art suites |
| `foundry-pixel-art` | 1.0.0 | Pixel-art paint programs |
| `foundry-image-cli` | 1.0.0 | Command-line image manipulation utilities |
| `foundry-daw` | 1.0.0 | Digital Audio Workstation stack and trackers |
| `foundry-trackers` | 1.0.0 | Chiptune and module trackers |
| `foundry-game-frameworks` | 1.0.0 | 2D/3D game-dev frameworks, headers, and shader tools |
| `foundry-game-reimplementations` | 1.0.0 | FOSS engines for commercial games (BYO data) |
| `foundry-free-games` | 1.0.0 | Bundled FOSS games with multi-GB data packs |
| `foundry-emulators` | 1.0.0 | Umbrella over the four emulator families below |
| `foundry-emulators-computers` | 1.0.0 | Vintage home-computer emulators (no bundled ROMs) |
| `foundry-emulators-consoles` | 1.0.0 | Light console + adventure-engine emulators |
| `foundry-emulators-consoles-heavy` | 1.0.0 | Heavy console + handheld emulators |
| `foundry-emulators-vintage` | 1.0.0 | Vintage home-computer emulators with bundled ROMs (multiverse) |

### Mobile-development metapackages (2)

| Package | Version | Depends |
|---|---|---|
| `foundry-android-development` | 1.0.1 | `openjdk-17-jdk`, `adb`, `google-android-ndk-r26c-installer` — enables Ubuntu multiverse |
| `foundry-ios-development` | 1.0.1 | `libimobiledevice`, `ideviceinstaller`, `usbmuxd`, `ifuse` (+ Phase 0 script pipx-installs `codemagic-cli-tools`) |

---

## Phase 0: `foundry-setup/` installer scripts

Bash installers that run on stock Ubuntu 26.04 — each is the apt-expansion of one would-be metapackage. All have `set -euo pipefail`, `-h/--help` short-circuit, lib.sh sourcing + inline shim fallback, `--dry-run` support.

| Script | What it does | Phase 1 target |
|---|---|---|
| `setup-foundry-apt-source.sh` | Adds apt.foundrylinux.org key + sources.list.d entry | — (bootstrap) |
| `setup-worldfoundry-apt-source.sh` | Adds apt.worldfoundry.org key + sources.list.d entry | — (bootstrap) |
| `install-foundry-engine-build-deps.sh` | `build-essential`, `cmake`, `libx11-dev`, `libgl1-mesa-dev`, `libglu1-mesa-dev`, `gdb`, `xxd`, `python3`, `pkg-config`, `git`, `curl`, `wget`, `unzip` (all Ubuntu universe) | overlaps with `worldfoundry-development`'s engine deps |
| `install-foundry-blender.sh` | `apt-get install -y worldfoundry-blender-addons` | `worldfoundry-blender-addons` (apt.worldfoundry.org) |
| `install-foundry-retro-tools.sh` | `apt install foundry-retro-tools` | `foundry-retro-tools` (apt.foundrylinux.org) |
| `install-foundry-android-development.sh` | `apt install foundry-android-development` | `foundry-android-development` (apt.foundrylinux.org) |
| `install-foundry-ios-development.sh` | `apt install foundry-ios-development pipx` + `pipx install codemagic-cli-tools` | `foundry-ios-development` (apt.foundrylinux.org) |
| `install-task.sh` | `task` (apt-get install — go-task package) | N/A — Cloudsmith / Ubuntu |
| `install-foundry-dev.sh` | umbrella — chains worldfoundry-development + task + foundry-retro-tools | overlaps with `worldfoundry-development` |

**No drift:** `install-foundry-blender.sh` targets `worldfoundry-blender-addons` (corrected after the 2026-05-21 rename; the pre-rename `worldfoundry-blender` is no longer in apt.worldfoundry.org).

---

## Cross-repo dependency picture

```
       ┌──────────────────────── apt.worldfoundry.org (live) ────────────────────────┐
       │                                                                              │
       │  worldfoundry-development ─► worldfoundry ──┬─► worldfoundry-cli ─► 9 CLIs   │
       │           │                                 │                                │
       │           │                                 └─► worldfoundry-blender-addons  │
       │           │                                       │       │                  │
       │           ▼                                       │       ▼                  │
       │   ubuntu universe                                 │   worldfoundry-blender-  │
       │   (build-essential,                               │   editor-exporter        │
       │    cmake, libx11-dev,                             │                          │
       │    libgl-dev, …)                                  │                          │
       │                                                   │                          │
       └───────────────────────────────────────────────────┼──────────────────────────┘
                                                           │
                                  cross-repo Depends ──────┘
                                                           │
       ┌──────────────────────── apt.foundrylinux.org (live) ────────────────────────┐
       │                                                   │                          │
       │                                                   ▼                          │
       │                                       blender-asset-finder                   │
       │                                                                              │
       │  foundry-retro-tools ─► mame, cc65, dasm, z80*, radare2, binwalk, …          │
       │      (+ vendored f9dasm, libvgm, vgmstream, ghidra)                          │
       │                                                                              │
       │  foundry-{atelier,anvil,sprite,art,pixel-art,daw,trackers,…} (15 creative)   │
       │  foundry-{android,ios}-development                                           │
       │                                                                              │
       └──────────────────────────────────────────────────────────────────────────────┘
                                                           │
                                                           ▼
                                                  blender 5.0.1+dfsg-1ubuntu1
                                                    (Ubuntu universe / resolute)
```

The two repos remain deliberately separate in scope — foundrylinux ships the distro toolchain, worldfoundry ships the authoring stack — but they're no longer entirely disjoint: `worldfoundry-blender-addons → blender-asset-finder` is a real cross-repo dep edge that resolves cleanly when both apt sources are configured (verified end-to-end today on stock ubuntu:26.04).

---

## Conventions recap (for anyone authoring a new package)

- **Always check Ubuntu 26.04 universe first** (`apt-cache policy <pkg>` in a fresh `ubuntu:26.04` container) before vendoring. The xa65 episode (re-packaging a tool that was already in universe) is the cautionary tale.
- **Canonical Debian layout only**: `debian/{control,changelog,rules,source/format,copyright}`. No hand-rolled `dpkg-deb --build`; no uppercase `DEBIAN/` in source.
- **Changelog is authoritative for versions** — no `Version:` in `debian/control`. Bump via `dch -v <new> -D resolute "what changed"` (foundrylinux.org) or `... -D stable ...` (worldfoundry.org). `1.0.x` for dep changes; `1.x.0` for new packages.
- **Vendored upstreams** pin `UPSTREAM_VERSION` + `SHA256` at the top of `build.sh`; re-pin with `curl -fsSL <url> | sha256sum`.
- **Always build in a Docker container** — host deps silently satisfy Build-Depends and mask CI failures. CI mandates the ubuntu:26.04 base for both repos.
- **Use the `/package` skill** for new vendored packages — it generates the `dh_make` skeleton and patches in Foundry-customised fields.
- **Verify new version pins on stock ubuntu:26.04** — assumptions about "what Ubuntu ships" rot fast; `docker run --rm ubuntu:26.04 apt-cache madison <pkg>` is the 30-second check that beats extrapolation.

---

## See also

- [`2026-05-19-package-inventory.md`](2026-05-19-package-inventory.md) — predecessor snapshot, captures the moment apt.foundrylinux.org was still 🚧 planned.
- [`2026-05-16-foundry-distro-proposal.md`](2026-05-16-foundry-distro-proposal.md) — strategic rationale, four delivery channels, Phases 2 (Distrobox) and 3 (ISO).
- [`docs/plans/2026-05-20-move-asset-packages.md`](../plans/2026-05-20-move-asset-packages.md) — the move of `blender-asset-finder` + rename of `wf-asset` → `blender-asset-finder-cli`.
- [`docs/plans/2026-05-22-audit-apt-repos-script.md`](../plans/2026-05-22-audit-apt-repos-script.md) — the plan this doc was built under (manual refresh → automate via script → compare).
- [`~/SRC/worldfoundry.org/docs/plans/2026-05-21-rename-worldfoundry-blender-to-worldfoundry-blender-addons.md`](../../../worldfoundry.org/docs/plans/2026-05-21-rename-worldfoundry-blender-to-worldfoundry-blender-addons.md) — the metapackage rename.
- [`~/SRC/WorldFoundry-wbniv/docs/plans/2026-05-21-blender-version-test-matrix.md`](../../../WorldFoundry-wbniv/docs/plans/2026-05-21-blender-version-test-matrix.md) — the CI matrix that validated the wider Blender pin (`>=4.0.2, <<5.1`).
- [`CLAUDE.md`](../../CLAUDE.md) §Conventions — authoring rules in one screen.
