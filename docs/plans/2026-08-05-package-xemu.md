# Package xemu (original Xbox emulator) for apt.foundrylinux.org

> **Renamed 2026-08-05.** This package shipped as `xemu`; it is now **`xemu-xbox`**, because LGB's
> unrelated 8-bit emulator suite has shipped its own `.deb` named `xemu` since 2016 and takes the bare
> name. The installed binary is still `/usr/bin/xemu`. See
> [the rename plan](2026-08-05-xemu-rename-and-upstream-pr.md). Everything below is preserved as the
> original packaging record and still refers to the old name.

**Date:** 2026-08-05
**Scope:** Vendor and package upstream [xemu](https://github.com/xemu-project/xemu) v0.8.136 as a Debian-policy-compliant `.deb`, and wire it into the `foundry-emulators-consoles` metapackage.

---

## Why

xemu is the only maintained emulator for the original Xbox (2001), and it is **not in Ubuntu 26.04 universe** (the universe check returns only the unrelated `gxemul`). Packaging it closes the last major sixth-generation console gap in the Foundry emulator catalogue.

### Console emulator catalogue

Every console/handheld emulator shipped by the two console metapackages, by hardware generation. Source column: **universe** = Ubuntu 26.04 universe, **Foundry** = built in `foundry-apt/packages/`.

| Gen | Console(s) | Emulator | Metapackage | Source |
|---|---|---|---|---|
| 2 | Atari 2600 | `stella` | consoles | universe |
| 3 | NES / Famicom | `fceux` | consoles | universe |
| 3 | NES / Famicom | `nestopia` | consoles‑heavy | universe |
| 3 | Sega Master System, Game Gear | `mesen2` | consoles‑heavy | Foundry |
| 4 | Super Nintendo / Super Famicom | `snes9x-gtk` | consoles + heavy | Foundry |
| 4 | Super Nintendo (cycle‑accurate) | `bsnes-jg` | consoles + heavy | Foundry |
| 4 | **Atari Lynx**, PC Engine / TurboGrafx‑16, GB/GBA, … | `mednafen` | consoles | universe |
| 4 | Game Boy / Game Boy Color (incidental — via the multi‑system cores above) | `mednafen`, `mesen2` | consoles + heavy | universe / Foundry |
| 5 | PlayStation 1 | `pcsxr` | consoles‑heavy | universe |
| 5 | Nintendo 64 | `mupen64plus-ui-console` | consoles‑heavy | universe |
| 6 | PlayStation 2 | `pcsx2` | consoles‑heavy | universe |
| 6 | GameCube (+ Wii, gen 7) | `dolphin-emu` | consoles‑heavy | universe |
| 6 | **Original Xbox** | **`xemu`** | **consoles** | **Foundry (this plan)** |
| 7 | Nintendo DS | `desmume` | consoles | universe |
| 7 | PlayStation Portable | `ppsspp` | consoles + heavy | Foundry |
| 8 | Nintendo Switch | `yuzu` | consoles‑heavy | universe |
| — | libretro multi‑system frontend | `retroarch` | consoles‑heavy | universe |
| — | Sierra/LucasArts/Westwood adventure engines | `scummvm` | consoles | universe |
| — | Z‑machine (Infocom text adventures) | `frotz` | consoles | universe |

Gen 6 had three principal consoles — PS2, GameCube, Xbox. The first two were already covered by universe packages; the **Dreamcast** (gen 6, 1998) is the one remaining gen‑6 gap and is tracked separately (see below). Home‑computer emulators (`dosbox-x`, `hatari`, `fs-uae`, `openmsx`, `vice`, `atari800`, `fbzx`) live in `foundry-emulators-computers` / `-vintage` and are out of scope for this table.

### Coverage gaps and follow‑ups

Audited 2026‑08‑05 against a fresh `ubuntu:26.04` container (`apt-cache policy`) plus `ares --help`'s own system list. None of these block this plan; each is tracked as its own `TODO.md` item.

| System | Emulator | In 26.04 universe? | Follow‑up |
|---|---|---|---|
| ColecoVision | `ares` | ✅ `147+dfsg-3` | ~~Add to `foundry-emulators-consoles`~~ ✅ **done** — `1.0.4`; also brings MSX/MSX2, SG‑1000, Neo Geo Pocket, WonderSwan, Mega Drive/CD/32X |
| Game Boy / GBC / GBA | `sameboy`, `mgba-qt` | ✅ `1.0.2+ds-2`, `0.10.5+dfsg-3build1` | ~~Add both~~ ✅ **done** — `1.0.4`; dedicated emulators, versus the previous incidental coverage |
| PONG (1972, discrete logic) | `mame` driver `pong` | ✅ `0.285+dfsg1-1` | ~~Placement undecided~~ ✅ **done** — `mame` moved to `foundry-emulators-consoles` `1.0.5` |
| Magnavox Odyssey (1972) | `mame` driver `odyssey` | ✅ same | ~~Same placement question~~ ✅ **done** — same move |
| Philips Odyssey² / Videopac (1978) | `mame` drivers `odyssey2`, `videopac` | ✅ same · ❌ standalone `o2em` absent | ~~Same placement question~~ ✅ **done** — same move |
| Atari Lynx | `mednafen` | ✅ already shipped | **Already covered** — the table row now names Lynx explicitly |
| **Dreamcast** | Flycast | ❌ `flycast`, `reicast`, `redream`, `lxdream`, `libretro-flycast` all absent | **Vendor and package** — same shape as xemu (user‑supplied BIOS, not shipped) |
| **PlayStation 3** | RPCS3 | ❌ AppImage‑only upstream | **Vendor and package** — needs user‑supplied PS3 firmware, same licensing shape as xemu's MCPX/BIOS. Large → `-heavy` |
| PS4 / PS5 | shadPS4 / — | ❌ | Out of scope: PS4 emulation is pre‑alpha; nothing exists for PS5 |
| Xbox 360 / One / Series | Xenia / — | ❌ | Out of scope: Xenia is Windows‑only (runs under Wine/Proton); nothing exists for One/Series |

**MAME drivers — verified 2026‑08‑05.** The earlier probe returned zero lines because `mame` installs to `/usr/games/`, which was not on the container's `PATH`. Re-run against `mame 0.285+dfsg1-1` on `ubuntu:26.04` (49,617 systems listed), exact `-listfull` rows:

```
pong              "Pong (Rev E)"
odyssey           "Odyssey"
odyssey2          "Odyssey 2 (US)"
videopac          "Videopac G7000 (Europe)"
videopacp         "Videopac+ G7400 (Europe)"
coleco            "ColecoVision (NTSC)"
colecop           "ColecoVision (PAL)"
gameboy           "Game Boy"
gbcolor           "Game Boy Color"
gba               "Game Boy Advance"
```

One trap worth recording: MAME also lists `lynx128k` "Lynx 128k", which is the **Camputers Lynx** home computer, not an Atari handheld. Atari Lynx coverage in the catalogue comes from `mednafen`; no Lynx claim is made for MAME in any package description.

**RetroArch caveat:** `retroarch` is packaged, but the archive's cores are only `gambatte`, `sameboy`, `mgba`, `desmume`, `nestopia`, `snes9x`, `bsnes-mercury`, `genesisplusgx`, and `beetle-{pce-fast,psx,vb,wswan}`. There is no `o2em`, `bluemsx`, `handy`, or `flycast` core in universe, so RetroArch does **not** fill the ColecoVision / Odyssey² / Dreamcast gaps from packages alone — only via its runtime online core downloader.

## Placement decision — light, not heavy

Initial instinct was `foundry-emulators-consoles-heavy`, since xemu's *peer group by console generation* is `pcsx2` / `dolphin-emu` / `yuzu`. That was wrong: the heavy bucket's own stated criterion is **installation footprint** ("larger installation footprints … left out of the base devbox image for size reasons"), not generation.

Measured evidence:

| Metric | Value |
|---|---|
| Upstream Linux AppImage size | 16 MB |
| Runtime closure on a **bare** `ubuntu:26.04` | 239 MiB / 99 packages |
| **Marginal** closure atop the existing light console set | **8 MiB / 4 packages** (`libslirp0`, `libssl3t64`, `libusb-1.0-0`, `openssl-provider-legacy`) |

Nearly all of the bare-container 239 MiB is the SDL2/Vulkan/X11/PipeWire desktop stack that `foundry-emulators-consoles` already drags in via `mednafen`, `scummvm`, `stella`, `desmume`, and `fceux`. The true incremental cost is ~8 MiB of libraries plus a ~16 MB binary.

Consequence of the choice: `foundry-emulators-consoles` is a dependency of `foundry-core`, so xemu ships in **anvil, sprite, atelier, the ISO, and the Phase 2 devbox**. `-heavy` is atelier-only. The measured footprint justifies the wider reach.

## Upstream source — why the release asset, not the tag archive

The plain GitHub tag archive is **not buildable**: xemu keeps its meson subprojects (imgui, implot, glslang, SPIRV-Reflect, VulkanMemoryAllocator, SDL3, berkeley-softfloat, …) as git submodules, which a tag tarball omits.

We therefore vendor the release asset `xemu-0.8.136.tar.zst` (176 MB compressed, 550 MB extracted), which bundles all subprojects.

- **sha256:** `92a09305a87d7dabaecc335fa54ca888117aab10fe22af6ad73fc0d3cdd20c1f` — confirmed identical across two independent fetches.
- It ships an `XEMU_VERSION` file (`0.8.136`). Upstream's `scripts/xemu-version.sh` falls back to that file when no `.git` is present, so the built binary reports the correct version without a git checkout.

## Why we did not adopt upstream's in-tree `debian/`

xemu ships a `debian/` directory, but it is inherited from Debian's QEMU packaging and is stale:

- `debian/changelog` frozen at `1:0.5.2-18-g12b58544b1-0` (May 2021)
- `debian/compat` = 11; `Standards-Version: 3.9.8`
- `debian/watch` points at `download.qemu.org`, tracking **QEMU** releases, not xemu
- `debian/copyright` describes QEMU's authors and Debian's QEMU maintainers verbatim
- `debian/build_deb.sh` hardcodes `XEMU_VERSION_TAG=0.0.0` and appends to `debian/control` on every run

We reuse it only as a **Build-Depends reference** and for the confirmed install layout (`dist/xemu`, `ui/xemu.desktop`, `ui/icons/xemu_*.png`). Everything else is written fresh to current standards (compat 13, Standards-Version 4.7.2, DEP-5 copyright, `Rules-Requires-Root: no`, `X-Repology-Project: xemu`).

## Build approach

xemu's supported Linux build path is upstream's `./build.sh`, which drives QEMU's `configure` with `--target-list=i386-softmmu` and `-DXBOX=1`, then stages `dist/xemu` via its `package_linux` step. Driving `dh_auto_*`/meson directly would bypass both, so `debian/rules` overrides every `dh_auto_*` stage and calls `build.sh`.

`SOURCE_DATE_EPOCH` is exported from the changelog date, because upstream's version script stamps `XEMU_DATE` from `date -u`.

## Man page

Upstream ships none, so Policy §12.1 requires we write one. `debian/man/xemu.1` documents the xemu-specific options confirmed present in the source — `-dvd_path` (`system/vl.c:3079`) and `-config_path` (`ui/xemu.c:1315`) — plus inherited `-full-screen`, and notes that the required MCPX/BIOS/HDD images are **not** shipped.

## Deliverables

```
foundry-apt/packages/xemu/
  build.sh                  fetch .tar.zst (sha256-pinned) -> extract -> dpkg-buildpackage
  debian/control            Foundry maintainer, X-Repology-Project: xemu
  debian/changelog          0.8.136-1foundry1, suite "resolute"
  debian/rules              overrides dh_auto_* to drive upstream build.sh
  debian/copyright          DEP-5, GPL-2
  debian/watch              tracks v<X.Y.Z> tags
  debian/source/format      3.0 (quilt)
  debian/patches/series     empty
  debian/man/xemu.1         hand-written (upstream ships none)
  debian/xemu.manpages
```

Plus: add `xemu` to `foundry-emulators-consoles` `Depends:` and bump its changelog.

## No visible surface

This change has no UI/rendered/CLI-output surface of its own beyond the packaged upstream application, so no mockups are carried.

---

## Verification

1. **Universe check — xemu is not already in Ubuntu 26.04**
2. **sha256 of the upstream archive is reproducible across two fetches**
3. **`.deb` builds cleanly in an `ubuntu:26.04` container**
4. **Binary is stripped and `${shlibs:Depends}` resolved to real sonames**
5. **`lintian` is clean — zero `E:` and zero `W:`**
6. **Man page is installed**
7. **Smoke install in a clean container; `xemu -version` runs**
8. **`foundry-emulators-consoles` dependency chain still resolves**

---

### 1. Universe check — xemu is not already in Ubuntu 26.04

```
=== apt-cache policy xemu ===
=== apt-cache search xemu ===
gxemul - machine emulator for multiple architectures
gxemul-doc - gxemul documentation
```

`Candidate:` is empty — Ubuntu ships no `xemu`; the only hits are the unrelated `gxemul`. **PASS**

### 2. sha256 of the upstream archive is reproducible across two fetches

```
92a09305a87d7dabaecc335fa54ca888117aab10fe22af6ad73fc0d3cdd20c1f  xemu-0.8.136.tar.zst
92a09305a87d7dabaecc335fa54ca888117aab10fe22af6ad73fc0d3cdd20c1f  verify.tar.zst
```

**PASS**

### 3. `.deb` builds cleanly in an `ubuntu:26.04` container

```
OK   /repo/dist/xemu_0.8.136-1foundry1_amd64.deb  (5128814 bytes)
```

**PASS** — 5.1 MB `.deb` from a 550 MB source tree.

Two build failures were found and fixed on the way, both recorded in `build.sh`:

- `dh: error: debhelper compat level specified both in debian/compat and in debian/control` — upstream vendors its own `debian/`, and `cp -a` merges into an existing directory rather than replacing it, so upstream's `debian/compat` (11) survived. Fixed with `rm -rf "$SRC_DIR/debian"` before the copy.
- `../meson.build:1697:20: ERROR: Unable to find CMake` — meson needs CMake to configure the CMake-based subprojects (SPIRV-Reflect, VulkanMemoryAllocator, glslang). Added `cmake` to Build-Depends.

### 4. Binary is stripped and `${shlibs:Depends}` resolved to real sonames

```
/usr/bin/xemu: ELF 64-bit LSB pie executable, x86-64, version 1 (SYSV),
dynamically linked, interpreter /lib64/ld-linux-x86-64.so.2, stripped
```

```
Depends: libc6 (>= 2.43), libcurl3t64-gnutls (>= 7.16.3), libepoxy0 (>= 1.5.2),
 libgcc-s1 (>= 3.4), libglib2.0-0t64 (>= 2.78.0), libpcap0.8t64 (>= 0.9.8),
 libsamplerate0 (>= 0.1.7), libslirp0 (>= 4.9.1), libstdc++6 (>= 13.1),
 libusb-1.0-0 (>= 2:1.0.23~), zlib1g (>= 1:1.2.0), libvulkan1, libx11-6,
 libx11-xcb1, libxcursor1, libxext6, libxfixes3, libxi6, libxrandr2, libxss1,
 libegl1, libgl1, libgbm1, libdrm2
Recommends: mesa-vulkan-drivers, libasound2t64, libpulse0,
 libpipewire-0.3-0t64, libdecor-0-0, libdbus-1-3
```

PIE and stripped, version-constrained sonames. **PASS**

**A real pre-ship bug was caught at this step.** The auto-resolved `Depends` contained no `libvulkan1` and no display libraries. xemu statically links the SDL3 it vendors as a meson subproject, and that SDL3 **`dlopen`s** its video and audio backends rather than linking them — confirmed by the absence of any `libSDL`/`libvulkan` `NEEDED` entry alongside `libvulkan.so.1`, `libX11.so.6`, `libEGL.so.1`, and `libpipewire-0.3.so.0` appearing as plain strings in the binary. `dh_shlibdeps` cannot see a `dlopen`, so the package would have installed cleanly and then failed at startup for want of a Vulkan loader. The required set is now explicit in `Depends`, the optional backends in `Recommends`.

### 5. `lintian` is clean — zero `E:` and zero `W:`

```
=== lintian .deb ===
CLEAN
=== lintian .dsc ===
CLEAN
```

**PASS.** Seven warnings from the first build were each fixed at source rather than suppressed:

| Warning | Fix |
|---|---|
| `appstream-metadata-validation-failed` (`metainfo-filename-cid-mismatch`) | install as `app.xemu.xemu.metainfo.xml` to match the component `<id>` |
| `debian-rules-calls-nproc` | honour `DEB_BUILD_OPTIONS=parallel=N` |
| `unknown-paragraph-in-dep5-copyright` | folded the stray `Comment` paragraph into the DEP‑5 header |
| `global-files-wildcard-not-first-paragraph-in-dep5-copyright` | same fold |
| `inconsistent-appstream-metadata-license` | added `Files: xemu.metainfo.xml` / `License: CC0-1.0` |
| `old-fsf-address-in-copyright-file` | replaced the FSF postal address with the licenses URL |
| `source-contains-prebuilt-windows-binary` | the one override — upstream's VulkanMemoryAllocator subproject vendors a Visual Studio sample `.exe`, inert on Linux, headers-only usage, absent from the binary package |

### 6. Man page is installed

```
-rw-r--r-- root/root 1541 ./usr/share/man/man1/xemu.1.gz
-rw-r--r-- root/root  214 ./usr/share/applications/xemu.desktop
-rw-r--r-- root/root 2169 ./usr/share/metainfo/app.xemu.xemu.metainfo.xml
```

**PASS**

### 7. Smoke install in a clean container; `xemu -version` runs

```
=== installed version ===
Package: xemu
Installed-Size: 15708
Version: 0.8.136-1foundry1
=== xemu -version ===
xemu_version: 0.8.136
xemu_commit: fc24584ce88f0915ad7f04775bb7712c2e3f49ee
xemu_date: Wed Aug  5 14:50:41 UTC 2026
```

**PASS** — confirms the `XEMU_VERSION` fallback works without a `.git` directory.

Known minor issue: `xemu_date` is stamped from `date -u` inside upstream's `scripts/xemu-version.sh`, which does not honour the `SOURCE_DATE_EPOCH` we export, so that one field is not reproducible. Cosmetic; would need an upstream patch.

### 8. `foundry-emulators-consoles` dependency chain still resolves

```
=== resolve foundry-emulators-consoles (simulate) ===
Inst xemu (0.8.136-1foundry1 localhost [amd64])
=== exit: 0 ===
  Depends: xemu
```

**PASS**

---

## Result

All 8 steps PASS. Committed in `abcedd3`. Publishing to `apt.foundrylinux.org` remains.
