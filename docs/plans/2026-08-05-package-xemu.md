# Package xemu (original Xbox emulator) for apt.foundrylinux.org

**Date:** 2026-08-05
**Scope:** Vendor and package upstream [xemu](https://github.com/xemu-project/xemu) v0.8.136 as a Debian-policy-compliant `.deb`, and wire it into the `foundry-emulators-consoles` metapackage.

---

## Why

xemu is the only maintained emulator for the original Xbox (2001), and it is **not in Ubuntu 26.04 universe** (the universe check returns only the unrelated `gxemul`). Packaging it closes the last major sixth-generation console gap in the Foundry emulator catalogue.

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

_(Output to be pasted below each step.)_
