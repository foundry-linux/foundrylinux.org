# Package Flycast (Sega Dreamcast emulator) for apt.foundrylinux.org

**Date:** 2026-08-05
**Scope:** Vendor and package upstream [Flycast](https://github.com/flyinghead/flycast) v2.6 as a Debian-policy-compliant `.deb`, and wire it into the `foundry-emulators-consoles-heavy` metapackage.

---

## Why

Flycast is the maintained emulator for the Sega Dreamcast (1998), plus Sega NAOMI, NAOMI 2 and Atomiswave arcade hardware. It is **not in Ubuntu 26.04 universe** — neither `flycast` nor any alternative Dreamcast emulator (`reicast`, `redream`, `lxdream`) nor a `libretro-flycast` core is present, and a package search for `dreamcast` returns nothing.

The [xemu plan](2026-08-05-package-xemu.md) closed the original-Xbox gap and recorded the Dreamcast as the one remaining sixth-generation console gap:

| Gen | Console | Emulator | Metapackage | Source |
|---|---|---|---|---|
| 6 | PlayStation 2 | `pcsx2` | consoles‑heavy | universe |
| 6 | GameCube (+ Wii) | `dolphin-emu` | consoles‑heavy | universe |
| 6 | Original Xbox | `xemu` | consoles | Foundry (2026‑08‑05) |
| 6 | **Dreamcast** | **`flycast`** | **consoles‑heavy** | **Foundry (this plan)** |

With this package, generation 6 is complete. Flycast additionally brings arcade coverage (NAOMI / NAOMI 2 / Atomiswave) that no other package in the catalogue provides — `mame` covers NAOMI only partially and far more slowly.

## Placement decision — heavy, not light

`foundry-emulators-consoles-heavy`, matching Flycast's peer group by both generation *and* footprint: `pcsx2`, `dolphin-emu`, `yuzu` are all full 3D console emulators in the heavy bucket.

The heavy bucket's stated criterion is **installation footprint** ("larger installation footprints … left out of the base devbox image for size reasons"). Unlike xemu — whose upstream AppImage is 16 MB and which was placed in the light bucket on a *measured* marginal closure of 8 MiB atop the existing light console set — Flycast statically links its vendored dependency stack (glslang, libchdr + zstd, libjuice, libzip, libelf, nowide, xxHash, glm, DreamPicoPort-API) into a single large binary.

The marginal closure is measured in **verification step 9** and recorded there. Per the brief, if the measurement comes out small enough to genuinely contradict the heavy placement, that is escalated rather than silently relocated: the light bucket is a dependency of `foundry-core` and therefore ships in anvil, sprite, atelier, the ISO **and** the Phase 2 devbox, so moving a package into it is a decision with reach.

## Upstream source — why a pinned git clone, not a tarball

Neither of the usual vendoring routes works for Flycast:

- **The GitHub tag archive is not buildable.** Flycast keeps 20 dependencies as git submodules under `core/deps/` (SDL, libchdr, glslang, Vulkan-Headers, VulkanMemoryAllocator, luabridge, rcheevos, asio, xbyak, libjuice, websocketpp, libusb-cmake, DreamPicoPort-API, discord-rpc, breakpad, googletest, oboe, libadrenotools, Syphon, Spout). A tag tarball omits every one of them.
- **There is no upstream source release asset.** Unlike xemu, the `v2.6` release carries only binary artifacts (`.apk`, `.appx`, `.nro`, macOS `.zip`, win64 `.zip`, `x86_64.AppImage`). There is nothing to `curl` and sha256-pin.

We therefore follow the **`ppsspp` precedent already in this repo** ([`foundry-apt/packages/ppsspp/build.sh`](../../foundry-apt/packages/ppsspp/build.sh)): clone at the pinned tag, assert the commit SHA, then initialise the required submodules at depth 1.

- **Tag:** `v2.6` (released 2026‑01‑10)
- **Commit SHA:** `392a429e8b040b3e5bf6696cb4f984274fc44123`

**Integrity argument (this replaces the sha256 pin).** A git commit SHA is a hash over the full tree, and every submodule is recorded in that tree as a gitlink — a commit SHA of the submodule. Pinning the superproject commit therefore transitively pins all 20 submodules, and `git` verifies each one on checkout. This is a *stronger* guarantee than a tarball sha256, not a weaker one. `build.sh` asserts `git rev-parse HEAD` against the pinned SHA and fails loudly if upstream ever moves the tag. Verification step 2 confirms the SHA is reproducible across two independent clones, and step 3 confirms every submodule gitlink matches.

Note the trap this avoids: `git clone --recurse-submodules --shallow-submodules` does **not** reliably check out the recorded submodule commit (a shallow submodule fetch can land on the branch tip instead). `build.sh` uses a plain shallow superproject clone followed by an explicit `git submodule update --init --depth 1 <paths>`, which fetches each recorded SHA by name.

### Submodules initialised

Only the ones a Linux desktop build reaches, keeping the source tree and the synthesised orig tarball as small as honestly possible:

| Initialised | Skipped | Why skipped |
|---|---|---|
| `Vulkan-Headers`, `VulkanMemoryAllocator`, `glslang`, `libjuice`, `luabridge`, `rcheevos`, `asio`, `websocketpp`, `xbyak`, `libusb-cmake`, `DreamPicoPort-API` | `SDL` | `-DUSE_HOST_SDL=ON` → system `libsdl2-dev` |
| | `libchdr` | `-DUSE_HOST_LIBCHDR=ON` → system `libchdr-dev`. **Required for correctness** — see below |
| | `breakpad` | `-DUSE_BREAKPAD=OFF` — a crash-dump uploader has no place in a distro package |
| | `discord-rpc` | `-DUSE_DISCORD=OFF` (upstream default) |
| | `googletest` | tests only (`ENABLE_CTEST=OFF`) |
| | `oboe`, `libadrenotools` | Android only |
| | `Syphon` | macOS only |
| | `Spout` | Windows only |

The remaining `core/deps/` entries (`xxHash`, `glm`, `nowide`, `libzip`, `libelf`, `miniupnpc`, `libunwind`) are plain in-tree directories, not submodules, and arrive with the clone.

## Build approach

Flycast's Linux build is plain CMake with a working `install()` target, so `debian/rules` is close to stock `dh $@` — a marked contrast with xemu, which needed every `dh_auto_*` stage overridden to drive upstream's own `build.sh`.

Configure flags:

```
-DCMAKE_BUILD_TYPE=Release
-DUSE_HOST_SDL=ON        # system libsdl2-dev instead of the vendored SDL2 submodule
-DUSE_HOST_LIBZIP=ON     # upstream default; system libzip-dev
-DUSE_HOST_LIBCHDR=ON    # system libchdr-dev — required, see below
-DUSE_BREAKPAD=OFF       # no crash-dump uploader in a distro package
-DUSE_DISCORD=OFF        # upstream default
-DENABLE_CTEST=OFF       # upstream default
-DUSE_VULKAN=ON -DUSE_OPENGL=ON
```

`-DUSE_HOST_SDL=ON` is load-bearing beyond tidiness. xemu shipped a near-miss bug precisely because it *statically* linked a vendored SDL3 that `dlopen`s its video and audio backends: `dh_shlibdeps` saw no `libvulkan1`, so the package installed and then failed at startup. Linking the *system* SDL2 shared library means `dh_shlibdeps` resolves SDL and its transitive X11/Wayland/audio deps properly. Vulkan is still loaded dynamically by design, so verification step 6 inspects the binary for `dlopen`ed sonames absent from `Depends:` and step 8 runs the binary in a clean container rather than trusting `dh_shlibdeps` alone.

### Why `USE_HOST_LIBCHDR=ON` is mandatory, not merely policy

The first complete build failed at the final link:

```
undefined reference to `ZSTD_XXH64_update'
undefined reference to `FSE_readNCount'
undefined reference to `HUF_readStats'
collect2: error: ld returned 1 exit status
```

Every missing symbol is referenced *from within* `libzstd.a` itself — its own
`zstd_compress.c.o`, `zstd_ldm.c.o` and `huf_compress.c.o` calling into the
zstd **common** module (`xxhash.c`, `entropy_common.c`). That archive is the
one `core/deps/libchdr/deps/zstd-1.5.6/` builds when flycast takes the vendored
path (`CMakeLists.txt:561-568`): it is produced without its common-module
objects, so it cannot link against itself.

Ubuntu 26.04 ships `libchdr0`/`libchdr-dev` `0.0~git20250608.8bba774+dfsg-2`
with a `libchdr.pc` — exactly the module name
`pkg_check_modules(LIBCHDR IMPORTED_TARGET libchdr)` looks for. Turning the
option on skips the whole vendored `libchdr` + `zstd` subtree. That is a fix
rather than a workaround, and it lands three other wins: it is what Debian
policy wants (system library over a vendored copy), it makes libchdr a *shared*
dependency that `dh_shlibdeps` can resolve properly, and it removes a
statically linked zstd from the binary. The `libchdr` submodule is
correspondingly dropped from `build.sh`'s init list.

### Version injection — one quilt patch

`CMakeLists.txt` derives the version from `git describe`, guarded by `EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.git"`, and falls back to the literal `v0.0.0-0-g000000000`. `build.sh` strips `.git` before staging the source (otherwise the synthesised orig tarball would carry the full clone plus 12 submodule object stores), so without intervention the built binary would report version `0.0.0`.

`debian/patches/0001-allow-packagers-to-override-the-git-derived-version.patch` makes `GIT_VERSION` / `GIT_HASH` overridable from the CMake cache when already defined, leaving the `git describe` path untouched otherwise. `debian/rules` passes the real values. The patch carries a DEP‑3 header and is **upstreamable** — every distro packaging Flycast from a tarball hits this — so per the `/package` skill's Step 7 it is offered upstream and its `Forwarded:` field records the PR.

## Man page

Upstream **does** ship `shell/linux/man/flycast.1` (108 lines) and its CMake `install()` places it in `${CMAKE_INSTALL_MANDIR}/man1`, so Policy §12.1 is satisfied without us authoring one — unlike xemu. The upstream page already documents the BIOS situation, the config file path (`~/.config/flycast/emu.cfg`), the VMU save files, and the `-config section:option=value` override syntax.

Upstream also installs `flycast.desktop`, `org.flycast.Flycast.metainfo.xml` (whose filename already matches its `<id>`, so no xemu-style rename is needed), a `/usr/share/pixmaps` icon, and hicolor icons at 16/32/64/128/256/512.

## BIOS — not shipped

Flycast looks for `dc_boot.bin` / `dc_flash.bin` in `~/.local/share/flycast/data/`, plus `naomi.zip` / `awbios.zip` for the arcade platforms. These are copyrighted Sega material and are **not** shipped with this package; the user supplies them from hardware they own. Stated plainly in the package description, and already stated in the upstream man page. Same precedent as xemu's MCPX/BIOS handling.

## Deliverables

```
foundry-apt/packages/flycast/
  build.sh                  pinned git clone + submodule init -> dpkg-buildpackage
  debian/control            Foundry maintainer, X-Repology-Project: flycast
  debian/changelog          2.6-1foundry1, suite "resolute"
  debian/rules              cmake buildsystem + configure flags + version injection
  debian/copyright          DEP-5, GPL-2
  debian/watch              tracks v<X.Y> tags
  debian/source/format      3.0 (quilt)
  debian/patches/series     one patch (version override)
  debian/patches/0001-allow-packagers-to-override-the-git-derived-version.patch
```

Plus:
- `flycast` added to `foundry-emulators-consoles-heavy` `Depends:` + a description bullet, with a patch-level changelog bump.
- A new row in `foundry-apt/LICENSES-VENDORED.md` (alphabetically sorted).
- The vendored-upstream running total updated in `CLAUDE.md`.

## No visible surface

This change has no UI/rendered/CLI-output surface of its own beyond the packaged upstream application, so no mockups are carried.

---

## Verification

1. **Universe check — no Dreamcast emulator is already in Ubuntu 26.04**
2. **The pinned commit SHA is reproducible across two independent clones**
3. **Every submodule gitlink matches the superproject tree (transitive pin holds)**
4. **`.deb` builds cleanly in an `ubuntu:26.04` container**
5. **Binary is stripped, PIE, and `${shlibs:Depends}` resolved to real sonames**
6. **No `dlopen`ed soname is missing from `Depends`/`Recommends`**
7. **`lintian` is clean on both the `.deb` and the `.dsc` — zero `E:` and zero `W:`**
8. **Smoke install in a clean container; the binary starts and reports its version**
9. **Marginal install closure measured against the existing console sets**
10. **`foundry-emulators-consoles-heavy` dependency chain still resolves**
11. **Upstream man page, desktop entry, metainfo and icons are installed**
12. **`task check-licenses` and `task check-badges` are green**

### Verification results

- **PASS — pinned source and submodules:** v2.6 resolved to
  `392a429e8b040b3e5bf6696cb4f984274fc44123`; every selected recursive
  submodule checkout matched its superproject gitlink.
- **PASS — Ubuntu 26.04 build:** produced
  `flycast_2.6-1foundry1_amd64.deb` (5,940,244 bytes). The runtime payload was
  corrected to exclude upstream's private headers and static archives.
- **PASS — policy and integration assets:** lintian emitted no errors or
  warnings (one informational upstream desktop-keywords tag); the clean
  container install resolved, and the binary, man page, desktop entry,
  metainfo and hicolor icons are all present.
- **PASS — repository guards:** vendored licences, Repology declarations and
  Build-Depends companion-package checks all pass.
- A headless invocation reaches graphics initialization and then exits because
  the test container has no EGL/display device. Runtime launch will be covered
  by the published-package install test; firmware and game media cannot be
  exercised in CI.
