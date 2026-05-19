---
title: Foundry Linux & WorldFoundry package inventory
date: 2026-05-19
status: snapshot — live state of apt.worldfoundry.org plus foundry-apt/ source tree
---

# Package inventory

There are **two distinct APT repositories** in the Foundry/WorldFoundry universe:

| Repository | Status | Scope |
|---|---|---|
| **`apt.worldfoundry.org`** | ✅ live (Cloudflare R2, signed) | WorldFoundry game-authoring tooling (CLIs + Blender add-ons + umbrella metapackages) |
| **`apt.foundrylinux.org`** | 🚧 planned — source tree in `foundry-apt/` | Broader Foundry Linux distro packages (vendored upstreams + dev metapackages) |

Plus **Phase 0** bash installer scripts in `foundry-linux-setup/` that bridge "stock Ubuntu" to "WF-ready dev box" until the Phase 1 APT repos cover everything.

This document inventories **all three** so we can see what's shipped, what's drafted, and where the gaps are.

---

## Repository 1: `apt.worldfoundry.org` — live

| | |
|---|---|
| **URL** | https://apt.worldfoundry.org |
| **Suite / Codename** | `stable` / `stable` |
| **Components** | `main` |
| **Architectures** | `all`, `amd64`, `arm64` |
| **Signing key** | `0xA51CECB8676E2158` — fingerprint `F19FF3274278148D5EE57FB3A51CECB8676E2158` |
| **Maintainer** | WorldFoundry Packages `<packages@worldfoundry.org>` |
| **Layout** | Classic Debian: `/dists/stable/main/binary-{amd64,arm64}/Packages` + `/pool/main/<l>/<name>/` |
| **Hosting** | Cloudflare R2 |
| **Index date (snapshot)** | 2026-05-19 11:45:03 UTC |

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
| `worldfoundry` | 1.1.0 | all | `worldfoundry-cli (≥1.0.0)`, `worldfoundry-blender (≥1.0.0)` — full game-dev install (umbrella) |
| `worldfoundry-cli` | 1.0.0 | all | `cdpack`, `iffcomp`, `iffdump`, `levcomp`, `lvldump`, `oaddump`, `oas2oad`, `textile`, `wf-asset`, `prep` — headless CI / build-server set, no Blender |
| `worldfoundry-blender` | 1.0.0 | all | `wf-blender (≥0.2.1)`, `blender-asset-finder (≥0.2.0)`, `blender (≥4.2.0)` — artist workstation, no compile chain |
| `worldfoundry-development` | 1.0.1 | all | `worldfoundry`, `build-essential`, `cmake (≥3.22)`, `libx11-dev`, `libgl-dev`, `libglu1-mesa-dev`, `python3`; **Recommends:** `gdb`, `xxd`; **Suggests:** `openjdk-17-jdk`, `adb` — for engine contributors |

### Binary packages (12)

All built from `git0a19d26c` (or `git3fa94cbe` for wf-blender) and packaged with a `1foundryN` revision suffix.

| Package | Version | Arch | Section | What it does |
|---|---|---|---|---|
| `cdpack` | 0.1.0+git0a19d26c-1foundry1 | amd64 | utils | CD master-image packer — assembles a WorldFoundry `GAME` cd.iff from a SHEL Forth script + N compiled level IFFs; sector-aligned (2 KB) output |
| `iffcomp` | 0.1.0+git0a19d26c-1foundry1 | amd64 | devel | IFF text-DSL compiler — `.iff.txt` source → binary IFF chunks; drop-in for the legacy C++/Go iffcomp |
| `iffdump` | 0.1.0+git0a19d26c-1foundry1 | amd64 | utils | Inverse of iffcomp — binary `.iff` → text DSL; lossless round-trip with iffcomp |
| `levcomp` | 0.1.0+git0a19d26c-1foundry1 | amd64 | devel | `.lev` + `.oad` blobs → `.lvl` level file; replaces legacy DOS/Watcom iff2lvl |
| `lvldump` | 0.1.0+git0a19d26c-1foundry1 | amd64 | utils | Inverse of levcomp — `.lvl` → human-readable text (object list, OAD values, geometry refs) |
| `oaddump` | 0.1.0+git0a19d26c-1foundry1 | amd64 | utils | `.oad` blob inspector — prints attributes resolved against OAS schemas |
| `oas2oad` | 0.1.0+git0a19d26c-1foundry1 | amd64 | devel | OAS schema → OAD blob compiler; Linux successor to DOS Watcom pigtool. **Runtime deps:** `prep`, `gcc`, `binutils` |
| `prep` | 0.103+git0a19d26c-1foundry1 | amd64 | devel | Kevin T. Seghetti's token-stream macro preprocessor (used inside oas2oad's pipeline). Currently a binary re-pack; lex/yacc grammar restoration is a follow-up |
| `textile` | 0.1.0+git0a19d26c-1foundry1 | amd64 | graphics | Texture-atlas packer — N input images (TGA/BMP/PNG) → packed atlas + UV table |
| `wf-asset` | 0.1.0+git0a19d26c-1foundry1 | all | utils | CLI front-end to the WF asset-provider library — search/download CC0 3D assets (Polyhaven, Kenney, AmbientCG, Quaternius, OpenGameArt, Sketchfab). Subcommands: `search`, `download`, `policy`, `providers`. Python 3.10+ |
| `wf-blender` | 0.2.1+git3fa94cbe-1foundry1 | amd64 | graphics | World Foundry Blender add-on — dynamic OAD property panel (from Rust schema descriptors), validation, `.iff.txt` level export. Ships extension zip + `wf-blender-install` wrapper. Bundles the native `wf_core` Rust extension |
| `blender-asset-finder` | 0.2.0+git0a19d26c-1foundry1 | all | graphics | Blender 4.2+ extension — same asset-provider library as `wf-asset` but in-Blender, with multi-provider online search + per-blend licence policy + `manifest.json` provenance. Pairs with `wf-asset` CLI |

### arm64 coverage

`arm64` is announced in the Release file but currently only carries the `Architecture: all` packages — `blender-asset-finder`, `wf-asset`, and the 4 metapackages. Native `amd64`-only packages (cdpack, iffcomp, iffdump, levcomp, lvldump, oaddump, oas2oad, prep, textile, wf-blender) are not yet built for arm64.

---

## Repository 2: `apt.foundrylinux.org` — planned (source tree only)

Lives in this repo as `foundry-apt/packages/<name>/debian/` (canonical Debian source-package layout). Not yet published. Mix of metapackages (3.0 native) and vendored upstreams not in Ubuntu (3.0 quilt).

### Metapackage

**`foundry-linux-retro-tools`** — 1.0.5 — `all` — section `metapackages`
Maintainer: World Foundry `<packages@worldfoundry.org>`
Depends: `mame`, `mame-tools`, `dasm`, `cc65`, `z80dasm`, `z80asm`, `radare2`, `binwalk`, `sox`, `binutils-m68k-linux-gnu`, `xa65`, `f9dasm`, `libvgm`, `vgmstream`, `ghidra`
Suggests: `java-common`
Arcade reverse-engineering + 6502/Z80/68k/6809 porting toolchain.
**Open issue:** xa65 still in Depends despite the in-progress retirement plan (`docs/plans/2026-05-18-retire-xa65.md`) — should be dropped so apt resolves it from 26.04 universe directly.

### Vendored upstreams (4)

| Package | Pkg version | Upstream | Pinned ref | SHA256 | Arch | Why vendored |
|---|---|---|---|---|---|---|
| `f9dasm` | 1.83-1foundry3 | github.com/Arakula/f9dasm | tag `V1.83` | `3a2d1e3f5de09be8060e4d754ff401622b5043b9a66687823fde188d035b0323` | any | Not in Ubuntu universe; needed for 6800/6809 ROM disassembly |
| `ghidra` | 12.1-1foundry1 | ghidra-sre.org | `Ghidra_12.1_build` dated 2026-05-13 | `aa5cbcbbf48f41ca185fce900e19592f1ade4cd5994eb6e0ede468dac8a6f302` | amd64 | NSA ships ZIPs only; pre-built x86_64 decompiler. Depends: `openjdk-21-jdk` |
| `libvgm` | 0.1+git20260406.d115188-1foundry1 | github.com/ValleyBell/libvgm | commit `d1151884997ba4769b070b7c7aaedeffc60a25fb` (2026-04-06) | `7af2e58bc1bef990c0ee759cdef7bdf5e1e1ebc9d16cde489932b0370c0e7fbc` | any | Git-tip only, no upstream releases. Build-Depends: cmake, pkg-config, zlib1g-dev, libasound2-dev, libpulse-dev, libao-dev |
| `vgmstream` | 2083-1foundry2 | github.com/vgmstream/vgmstream | tag `r2083` | `22cec642df2ea4148849d64d5af384f156a1dbebd95567e00236833dd9dbe136` | any | Not in Ubuntu universe. 100+ game-audio formats → WAV. ATRAC9/FSB CELT/G.719 deliberately excluded (Windows-only DLLs) |

---

## Phase 0: `foundry-linux-setup/` installer scripts

Bash installers that run on stock Ubuntu — each is the apt-expansion of one would-be metapackage. All have `set -euo pipefail`, `-h/--help` short-circuit, lib.sh sourcing + inline shim fallback, `--dry-run` support.

| Script | Apt-installs | Eventual Phase 1 target |
|---|---|---|
| `install-foundry-linux-engine-build-deps.sh` | `build-essential`, `cmake`, `libx11-dev`, `libgl1-mesa-dev`, `libglu1-mesa-dev`, `gdb`, `xxd`, `python3`, `pkg-config`, `git`, `curl`, `wget`, `unzip` | overlaps with live `worldfoundry-development`'s engine deps; no separate `.deb` planned in either repo |
| `install-foundry-linux-blender.sh` | `blender`, `python3` | satisfied by Ubuntu universe + live `worldfoundry-blender` |
| `install-foundry-linux-retro-tools.sh` | `mame`, `mame-tools`, `dasm`, `cc65`, `z80dasm`, `z80asm`, `radare2`, `binwalk`, `sox`, `binutils-m68k-linux-gnu`, `xa65`, `f9dasm`, `libvgm`, `vgmstream`, `ghidra` | `apt.foundrylinux.org` `foundry-linux-retro-tools` 1.0.5 |
| `install-foundry-linux-android-dev.sh` | `openjdk-17-jdk`, `adb`, `google-android-ndk-r26c-installer` | no `.deb` planned; covered by `worldfoundry-development` Suggests |
| `install-task.sh` | `task` (Cloudsmith repo) | N/A — upstream-owned |
| `install-foundry-linux-dev.sh` | umbrella: engine-build-deps → task → blender → retro-tools | overlaps with live `worldfoundry-development` |

---

## Cross-repo dependency picture

```
                ┌──────────────────────────── apt.worldfoundry.org (live) ───────────────────────────┐
                │                                                                                     │
                │  worldfoundry-development ──► worldfoundry ──┬─► worldfoundry-cli ──► 10 CLIs       │
                │           │                                  │                                      │
                │           │                                  └─► worldfoundry-blender ──► 2 add-ons │
                │           │                                                                         │
                │           └─► build-essential, cmake, libx11-dev, libgl-dev, …  (ubuntu universe)   │
                │                                                                                     │
                └─────────────────────────────────────────────────────────────────────────────────────┘
                                              ▲
                                              │  (independent — different scope)
                                              ▼
                ┌──────────────────────────── apt.foundrylinux.org (planned) ────────────────────────┐
                │                                                                                     │
                │  foundry-linux-retro-tools ──► xa65, f9dasm, libvgm, vgmstream, ghidra, mame, …    │
                │                                                                                     │
                │  (vendored: f9dasm, ghidra, libvgm, vgmstream)                                     │
                │                                                                                     │
                └─────────────────────────────────────────────────────────────────────────────────────┘
```

The two repos are deliberately separate: `apt.worldfoundry.org` ships the WF authoring stack (engine + tools that compile levels and CDs); `apt.foundrylinux.org` ships the broader Foundry Linux toolchain (vendored upstreams + retro-porting metapackage). A workstation could install from both with no overlap — they target disjoint problem domains.

---

## Conventions recap (for anyone authoring a new package)

- **Always check Ubuntu 26.04 universe first** (`apt-cache policy <pkg>` in a fresh `ubuntu:26.04` container) before vendoring. The xa65 episode is the cautionary tale.
- **Canonical Debian layout only**: `debian/{control,changelog,rules,source/format,copyright}`. No hand-rolled `dpkg-deb --build`; no uppercase `DEBIAN/` in source.
- **Changelog is authoritative for versions** — no `Version:` in `debian/control`. Bump via `dch -v <new> -D resolute "what changed"`. `1.0.x` for dep changes; `1.x.0` for new packages.
- **Vendored upstreams** pin `UPSTREAM_VERSION` + `SHA256` at the top of `build.sh`; re-pin with `curl -fsSL <url> | sha256sum`.
- **Always build in a Docker container** — host deps silently satisfy Build-Depends and mask CI failures.
- **Use the `/package` skill** for new vendored packages — it generates the `dh_make` skeleton and patches in Foundry-customised fields.

---

## See also

- [`2026-05-16-foundry-linux-distro-proposal.md`](2026-05-16-foundry-linux-distro-proposal.md) — strategic rationale, four delivery channels, Phases 2 (Distrobox) and 3 (ISO).
- [`docs/plans/2026-05-18-retire-xa65.md`](../plans/2026-05-18-retire-xa65.md) — drop xa65 from the retro-tools Depends (in progress).
- [`docs/plans/2026-05-18-worldfoundry-apt-repo.md`](../plans/2026-05-18-worldfoundry-apt-repo.md) — publishing pipeline for `apt.worldfoundry.org`.
- [`CLAUDE.md`](../../CLAUDE.md) §Conventions — authoring rules in one screen.
