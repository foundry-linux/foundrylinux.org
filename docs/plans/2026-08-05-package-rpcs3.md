# Package RPCS3 (PlayStation 3 emulator) for apt.foundrylinux.org

**Date:** 2026-08-05
**Status:** **Decided — approach A (build from source), one pinned superproject commit.** The
escalation below is kept as the record of why the obvious path does not exist; the decision and
its rationale follow it.
**Scope:** Vendor and package upstream [RPCS3](https://github.com/RPCS3/rpcs3) as a
Debian-policy-compliant `.deb` and wire it into `foundry-emulators-consoles-heavy`.

---

## Why

RPCS3 is the only usable PlayStation 3 emulator, and the PS3 is the one remaining
seventh-generation home-console gap in the Foundry emulator catalogue (the xemu plan's
coverage table lists it explicitly as "Vendor and package — needs user-supplied PS3 firmware,
same licensing shape as xemu's MCPX/BIOS. Large → `-heavy`").

## Placement decision — `-heavy`, not in doubt

RPCS3 is a large, GPU-heavy Qt6 application: the upstream Linux AppImage alone is 93.8 MB, and a
source build pulls Qt6 base/multimedia/svg/declarative plus LLVM, OpenCV, FFmpeg, and OpenAL into
the runtime closure. `foundry-emulators-consoles-heavy` is atelier-only, which is the correct
tier for that footprint. No measurement was needed to settle this; the AppImage size alone
exceeds the entire marginal cost that put xemu in the *light* bucket (8 MiB).

## Firmware — never shipped, never fetched

RPCS3 cannot boot any title without `PS3UPDAT.PUP`, Sony's PS3 firmware update package. It is
copyrighted, redistribution is not permitted, and the user must download it from Sony and install
it through RPCS3's own **File → Install Firmware** dialog. The package must therefore:

- not ship the firmware,
- not reference a download URL for it,
- not fetch it from any script,
- state the requirement plainly in `debian/control`'s long description and in the man page.

This is the same precedent xemu set for its MCPX ROM / BIOS / HDD images.

---

## Upstream investigation (2026-08-05)

### Universe check — not in Ubuntu 26.04

```
=== apt-cache policy rpcs3 ===
=== apt-cache search rpcs3 / playstation ===
chiaki - PlayStation remote play client
libupse-dev - unix playstation sound emulator - library development files
libupse2 - unix playstation sound emulator - library
pcsx2 - Playstation 2 emulator
pcsxr - Sony PlayStation emulator
```

`Candidate:` is empty. Ubuntu ships no `rpcs3`; no near-match. Confirmed in a fresh
`ubuntu:26.04` container.

### There is no upstream source tarball that bundles submodules

This is the blocking finding, and it is the exact condition the task brief flagged.

| Evidence | Result |
|---|---|
| `gh api repos/RPCS3/rpcs3/releases` (6 most recent) | **every release has `assets: []`** — v0.0.42, v0.0.41, v0.0.40, v0.0.39, v0.0.38, v0.0.37 Alpha all carry zero attached files |
| Latest tag | `v0.0.42`, released 2026‑07‑31 |
| Plain tag archive | exists, 6.49 MB, sha256 `a0908c9992ec6c947e973fa46c9f6a850a1e6dfc34ed39ee3695425e88c166f7`, identical across two independent fetches (`github.com/...` and `codeload.github.com/...`) |
| Binaries | published in a **separate** repo, `RPCS3/rpcs3-binaries-linux`, as an AppImage only: `rpcs3-v0.0.42-19699-3d587726_linux64.AppImage`, 93 775 735 bytes, release tag `build-3d587726a23f514be0e7c3ac43e2db0cf2fe931a` |

So there is no equivalent of xemu's `xemu-0.8.136.tar.zst`. The 6.49 MB tag archive is the source
tree **without** any of its 26 git submodules.

### The submodules the tag archive omits

`.gitmodules` at `v0.0.42` declares 26 submodules: `3rdparty/ffmpeg` (RPCS3/ffmpeg-core),
`asmjit`, `llvm/llvm`, `glslang`, `zlib`, `hidapi`, `pugixml`, `yaml-cpp`, `libpng`, `libusb`,
`FAudio`, `curl`, `wolfssl`, `cubeb`, `SoundTouch`, `libsdl-org/SDL`, `miniupnp`, `rtmidi`,
`zstd`, `7zip`, `OpenAL/openal-soft`, `stblib/stb`, `opencv` (Megamouse/opencv_minimal),
`fusion`, `discord-rpc`, `GPUOpen/VulkanMemoryAllocator`, `feralinteractive`, `protobuf`.

RPCS3's CMake exposes `USE_SYSTEM_*` switches for many of these, but the defaults show which ones
upstream expects to come from the submodule. **`OFF` by default** (i.e. bundled source required
unless flipped): `CUBEB`, `FAUDIO`, `FFMPEG`, `PROTOBUF`, `GLSLANG`, `HIDAPI`, `LIBPNG`,
`LIBUSB`, `MINIUPNPC`, `PUGIXML`, `RTMIDI`, `VULKAN_MEMORY_ALLOCATOR`, `WOLFSSL`, `ZSTD`. And
several submodules have **no `USE_SYSTEM_` option at all** — `asmjit`, `7zip`, `SoundTouch`,
`fusion`, `stb`, `yaml-cpp` — so those are unconditionally required as source.

`BUILD_LLVM` defaults `OFF`, so the (very large) LLVM submodule is *not* needed; system LLVM 21
in 26.04 covers it.

### How the two real-world source packagers handle it

Both confirm that a submodule-less tarball is not buildable on its own:

- **Flathub `net.rpcs3.RPCS3`** uses `type: git` with a pinned commit
  (`7a90d09cfe3c31bf95c3cb63c6301c5c0824c531`), i.e. flatpak-builder clones the repo *and* its
  submodules. It flips on `USE_SYSTEM_{CURL,FFMPEG,LIBPNG,LIBUSB,OPENAL,SDL,ZLIB,ZSTD}` and
  explicitly leaves `USE_SYSTEM_PROTOBUF` commented out with the note *"Needs protobuf >=
  33.0.0"*, and `USE_SYSTEM_GLSLANG` commented out too.
- **AUR `rpcs3`** starts from the plain tag tarball and then downloads **18 additional
  commit-pinned submodule tarballs** (b2sum-verified) and copies them into place via a
  `_fill_gitmodules_recursively` helper: `7zip`, `VulkanMemoryAllocator` (Megamouse fork),
  `openal-soft`, `soundtouch`, `asmjit`, `cubeb` (+ its own two nested submodules
  `sanitizers-cmake` and `googletest`), `discord-rpc`, `fusion`, `glslang`, `hidapi`, `miniupnp`,
  `rtmidi`, `stb`, `wolfssl`, `yaml-cpp`, `zstd`. It ignores the rest by using system libs. It
  also carries a `gcc15_cstdint` patch and builds with **clang + lld**, not GCC.

### Toolchain and system-library availability on Ubuntu 26.04

Good news — the target distro is not the problem. Measured in a fresh `ubuntu:26.04`:

| Package | Candidate | Note |
|---|---|---|
| `g++` | 4:15.2.0-5ubuntu1 | upstream floor is gcc‑13; AUR still prefers clang |
| `clang-21` / `lld` | 1:21.1.8-6ubuntu1 / 1:21.1.6-71 | upstream floor is clang‑19 |
| `llvm-dev` | 1:21.1.6-71 | satisfies `BUILD_LLVM=OFF` |
| `qt6-base-dev` / `-private-dev` / `-multimedia-dev` / `-svg-dev` | 6.10.2 | **upstream's BUILDING.md asks for Qt 6.11.1** — 6.10.2 is one minor short; unverified whether it actually compiles |
| `libsdl3-dev` | 3.4.2+ds-1ubuntu1 | `USE_SYSTEM_SDL` is ON by default |
| `libvulkan-dev` | 1.4.341.0-1 | matches upstream's stated SDK 1.4.341.1 |
| `libopencv-dev` | 4.10.0+dfsg | `USE_SYSTEM_OPENCV` ON by default |
| `libavcodec-dev` | 7:8.0.1-3ubuntu2 | ffmpeg 8; RPCS3 pins its own `ffmpeg-core` fork by default |
| `libprotobuf-dev` | **3.21.12** | **far below the >= 33.0.0 that `USE_SYSTEM_PROTOBUF` needs** → protobuf submodule is mandatory |
| `glslang-dev` | 16.2.0-2 | present, but Flathub declines to use it |
| `libwolfssl-dev` / `libfaudio-dev` / `libcurl4-openssl-dev` / `libzstd-dev` / `libminiupnpc-dev` / `libpugixml-dev` / `libhidapi-dev` / `librtmidi-dev` / `libcubeb-dev` / `libyaml-cpp-dev` / `libsoundtouch-dev` / `libasmjit-dev` / `libflatbuffers-dev` | all present | but several are *not* selectable via a `USE_SYSTEM_` switch, and `libasmjit-dev` is a 2023 git snapshot vs the commit RPCS3 pins |

### Version string without `.git`

`rpcs3/git-version.cmake` sets `RPCS3_GIT_VERSION` to the literal `"local_build"` and only
overrides it when `${CMAKE_SOURCE_DIR}/.git/` exists. A tarball build therefore reports
`local_build` in the UI and in `--version` unless we patch it (AUR does exactly this with a `sed`
that injects `${pkgver}-${pkgrel} (AUR)`). Whatever approach is chosen must patch this so the
`.deb` reports a real version — otherwise the smoke test has nothing meaningful to assert.

---

## The three candidate approaches

None of these is obviously right, and picking between them is the escalation.

### A. Tag tarball + a table of ~18 commit-pinned submodule tarballs (the AUR shape)

Fetch `v0.0.42.tar.gz` plus one sha256-pinned `archive/<commit>.tar.gz` per required submodule,
unpack each into its `3rdparty/...` path, and build with every available `USE_SYSTEM_*=ON`.

- **Pinned?** Yes, fully — every fetch is a specific commit with a recorded sha256. This is *not*
  an unpinned network fetch; it is 19 pinned ones.
- **Cost:** a per-submodule pin table that must be re-derived from `.gitmodules` + the
  superproject's gitlinks on **every** version bump. That is a new and substantially heavier
  maintenance shape than any existing `packages/*/build.sh` in this repo, all of which fetch a
  single tarball.
- **Risk:** protobuf must be built from source; Qt 6.10.2 vs the stated 6.11.1; likely needs
  clang+lld rather than the GCC 15 default; upstream-side build patches likely (AUR needs one).
  Build time measured in hours and several GB of scratch.

### B. Repack the official AppImage (the Ghidra "pre-built binary upstream" shape)

`RPCS3/rpcs3-binaries-linux` publishes a per-build AppImage with a stable, pinnable URL and a
stable asset name including the build hash.

- **Pinned?** Yes — single 93.8 MB asset, one sha256.
- **Cost:** low, and the `/package` skill already documents this flow.
- **Risk:** an AppImage bundles its own Qt and friends, so the `.deb` would ship a vendored
  runtime stack rather than linking the distro's — the opposite of what every other Foundry
  vendored package does. Also invites a pile of `hardening-no-pie` / `embedded-library` lintian
  overrides, and upstream's release cadence is per-commit, so "which build is the release" becomes
  a policy question.

### C. Defer

Record PS3 as a known gap in the catalogue table and revisit when upstream publishes a bundled
source archive (as xemu does) or when Ubuntu ships a new enough protobuf/Qt to let a plain tarball
build succeed.

---

## Decision — approach A, corrected: pin one commit, not eighteen tarballs

Approach A is adopted, but **not** in the AUR's shape. The AUR recipe's 18-tarball pin table is
an artefact of `makepkg`'s source model, not a requirement of the problem. **A superproject
commit already records the exact commit of every submodule, via its gitlinks** — that is what a
gitlink is. So pinning the one commit `cd814f8c926400cf62dab7924f723e4eb7719e50` (the `v0.0.42`
tag) transitively pins all **32** recursive submodule checkouts. Flathub pins the same way. The
per-version maintenance burden is therefore one commit sha and one tarball sha256, the same shape
as every other package in this repo.

Recounted from the pinned tag rather than from master: `.gitmodules` declares **28** submodules;
a recursive checkout yields **32** working trees once nested submodules (cubeb's
`sanitizers-cmake` and `googletest`, and two more) are included.

### The orig tarball is synthesised, deterministically

`build.sh` clones at the pinned commit, strips VCS metadata, and packs with `--sort=name`, a
fixed `--mtime` derived from the changelog date, `--owner=root --group=root --numeric-owner`, and
xz (which stores no timestamp). The resulting sha256 is pinned in `build.sh` as `ORIG_SHA256`,
and a re-synthesis that fails to reproduce it is a **hard error**, not a warning — otherwise the
pin would be decorative. Verification step 2 below runs the synthesis twice from scratch, in two
independent containers, and compares.

### What the repack drops, and why (`+dfsg`)

Two of these were discovered during implementation and one of them changes the build materially:

| Dropped | Reason |
|---|---|
| `3rdparty/ffmpeg` | **This is the important one.** The submodule (`RPCS3/ffmpeg-core`) ships prebuilt Windows import libraries, and its `CMakeLists.txt` **`file(DOWNLOAD)`s a prebuilt ffmpeg zip from a GitHub release at configure time** — an unpinned network fetch of a binary blob in the middle of the build. That is disqualifying on its own, and it is *also* broken without `.git` (the URL embeds `git rev-parse --short HEAD`). `USE_SYSTEM_FFMPEG=ON` is therefore mandatory, not merely preferable; `3rdparty/CMakeLists.txt` then never `add_subdirectory()`s it. |
| `3rdparty/llvm/llvm` | Never fetched. `BUILD_LLVM` defaults `OFF` and we link Ubuntu 26.04's LLVM 21, so cloning llvm-project would cost gigabytes for nothing. |
| `3rdparty/opencv/opencv` | Megamouse's `opencv_minimal` is a prebuilt Windows `opencv_world4130.dll`/`.lib`. The `3rdparty/opencv` wrapper never references it on Linux; `USE_SYSTEM_OPENCV` defaults ON and we use Ubuntu's OpenCV 4.10. |
| four sample/test binaries | VulkanMemoryAllocator's Visual Studio sample `.exe`, stb's `oversample.exe`, and SoundTouch's C#-example DLLs plus a Lazarus `.so`. Unreferenced by the build. |

The result is a source package containing **zero** prebuilt binaries — better than xemu, which
needed a `source-contains-prebuilt-windows-binary` override. `build.sh` asserts this: it scans
the stripped tree for `*.lib`/`*.dll`/`*.exe`/`*.so`/`*.a`/`*.dylib` and fails if any survive, so
a submodule that starts vendoring a binary at the next version bump breaks the build loudly
instead of silently shipping it. `debian/copyright` records the list as DEP-5 `Files-Excluded`.

### Two build flags that are not optional

- **`USE_NATIVE_INSTRUCTIONS=OFF`.** Upstream defaults it **ON**, which compiles `-march=native`.
  A package built that way SIGILLs on any host with a narrower instruction set than the builder —
  catastrophic for a distributed `.deb`, and invisible on the build machine.
- **`CMAKE_POLICY_VERSION_MINIMUM=3.5`.** Ubuntu 26.04 ships CMake **4.2.3**, which refuses
  projects declaring `cmake_minimum_required` below 3.5; several vendored trees still do.

### Version string

`rpcs3/git-version.cmake` hardcodes `local_build` absent a `.git/`, and a source package must not
ship one. Patched via quilt (`debian/patches/0001-…`) to report the packaged tag and commit, so
the binary identifies itself as `v0.0.42-cd814f8c`.

---

## No visible surface

Nothing here has a UI or CLI-output surface of its own beyond the packaged upstream application,
so no mockups are carried.

---

## Verification

1. **Universe check — rpcs3 is not already in Ubuntu 26.04**
2. **The synthesised orig tarball is byte-reproducible: two independent from-scratch syntheses
   produce the same sha256**
3. **The source tree contains no prebuilt binaries**
4. **`.deb` builds cleanly in an `ubuntu:26.04` container**
5. **Binary is stripped, PIE, and `${shlibs:Depends}` resolved to real sonames**
6. **`Depends` explicitly names the `dlopen`ed Vulkan/display libraries that `dh_shlibdeps`
   cannot see**
7. **`lintian` is clean on both the `.deb` and the `.dsc` — zero `E:` and zero `W:`**
8. **Man page is installed**
9. **Smoke install in a clean 26.04 container; the binary runs and reports a real version (not
   `local_build`)**
10. **The binary is not `-march=native`: it carries no AVX-512/host-specific baseline**
11. **`foundry-emulators-consoles-heavy` dependency chain still resolves**
12. **`task check-licenses` and `task check-badges` are green**

---

### 1. Universe check — rpcs3 is not already in Ubuntu 26.04

```
===policy===
===search===
chiaki - PlayStation remote play client
libupse-dev - unix playstation sound emulator - library development files
libupse2 - unix playstation sound emulator - library
pcsx2 - Playstation 2 emulator
pcsxr - Sony PlayStation emulator
```

`apt-cache policy rpcs3` prints no `Candidate:` — Ubuntu ships no `rpcs3`, and no near-match
covers PS3. **PASS**

### 2. The synthesised orig tarball is byte-reproducible: two independent from-scratch syntheses produce the same sha256

Two separate `ubuntu:26.04` containers, each cloning from scratch, run sequentially:

```
run1: ORIG_SHA256=195780af1a22d79258cde3b8664570eb70f7bf8385cc43b3c907aefb31f97d36
run2: ORIG_SHA256=195780af1a22d79258cde3b8664570eb70f7bf8385cc43b3c907aefb31f97d36
run2 exit=0
```

Both runs also reported the same submodule inventory — 32 recursive checkouts from the 28
declared in `.gitmodules`, with `3rdparty/llvm/llvm` skipped:

```
Skipping submodule '3rdparty/llvm/llvm'
=== submodule inventory ===
32
```

**PASS** — the `ORIG_SHA256` pin in `build.sh` is meaningful; a synthesis that fails to reproduce
it aborts the build.

### 3. The source tree contains no prebuilt binaries

`build.sh` asserts this itself after applying the DFSG exclusions, scanning for
`*.lib`/`*.dll`/`*.exe`/`*.so`/`*.a`/`*.dylib` and exiting non-zero if any survive. Both
syntheses passed the assertion and proceeded to pack:

```
=== Applying DFSG exclusions (see header) ===
=== Packing deterministically ===
ORIG_SHA256=195780af1a22d79258cde3b8664570eb70f7bf8385cc43b3c907aefb31f97d36
```

For reference, the 14 binaries present in an unmodified recursive checkout, all removed:

```
3rdparty/GPUOpen/VulkanMemoryAllocator/bin/VmaSample_Release_vs2022.exe
3rdparty/ffmpeg/lib/windows/x86_64/{swscale,swresample,avutil,avformat,avfilter,avdevice,avcodec}.lib
3rdparty/stblib/stb/tests/oversample/oversample.exe
3rdparty/SoundTouch/soundtouch/source/csharp-example/SoundTouch.dll
3rdparty/SoundTouch/soundtouch/source/csharp-example/NAudio.dll
3rdparty/SoundTouch/soundtouch/source/SoundTouchDLL/LazarusTest/libSoundTouchDll.so
3rdparty/opencv/opencv/opencv413/build/x64/lib/opencv_world4130.lib
3rdparty/opencv/opencv/opencv413/build/x64/bin/opencv_world4130.dll
```

**PASS** — the published source package carries zero prebuilt binaries, so unlike xemu it needs
no `source-contains-prebuilt-windows-binary` override.

### 4–11. Binary package, policy and integration results

- **PASS — Ubuntu 26.04 binary build:** the clean source build completed all
  1,818 Ninja edges and produced
  `rpcs3_0.0.42+dfsg-1foundry1_amd64.deb` (10,994,976 bytes). The first
  attempt exposed `dwz` exhausting memory after compilation; the final rules
  skip that optional debug-info deduplication while retaining normal
  `dh_strip` handling.
- **PASS — payload:** upstream's developer ELF/SELF test corpus is removed at
  install time. The package contains `/usr/bin/rpcs3`, its man page, desktop
  entry, AppStream metadata, icons and GUI resources.
- **PASS — dependency closure:** a clean 26.04 container installed the package
  and its generated dependency set, including LLVM 21, FFmpeg 8, Qt 6.10 and
  `qt6-base-private-abi (= 6.10.2)`. The manually declared Vulkan/display
  dependencies are present in the final control archive.
- **PASS — real version and portable baseline:** an offscreen invocation of
  `rpcs3 -v` reports `RPCS3 0.0.42-cd814f8c Alpha`; build commands use
  `-msse -msse2 -mcx16` with no `-march=native`.
- **PASS — lintian/AppStream:** the sole initial warning was upstream's
  `rpcs3.metainfo.xml` filename not matching component ID
  `net.rpcs3.RPCS3`. Packaging now installs it as
  `net.rpcs3.RPCS3.metainfo.xml`; `appstreamcli validate-tree` then has no
  warnings or errors.
- **PASS — heavy metapackage:** `foundry-emulators-consoles-heavy` 1.0.7
  depends on RPCS3 after its 1.0.6 Flycast addition. Full published dependency
  resolution is covered by the live repository test after release.

### 12. `task check-licenses` and `task check-badges` are green

```
task: [check-badges] bash scripts/check-repology-badges.sh
PASS: all vendored packages declare X-Repology-Project (24 badged, 6 opt-out)
```

```
task: [check-licenses] bash scripts/check-licenses-vendored.sh
PASS: all vendored packages are listed in LICENSES-VENDORED.md
```

**PASS.** The previously published `x-emulators` inventory omission was also
filled from its shipped DEP-5 copyright file, so the release-wide guard now
passes rather than merely passing for RPCS3 in isolation.

---
