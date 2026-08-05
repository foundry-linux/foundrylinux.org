# Package RPCS3 (PlayStation 3 emulator) for apt.foundrylinux.org

**Date:** 2026-08-05
**Status:** **BLOCKED — escalated.** No packaging work performed. This document records the
upstream investigation and the three candidate build approaches; the choice between them is a
design decision that has not been made.
**Scope (intended):** Vendor and package upstream [RPCS3](https://github.com/RPCS3/rpcs3) as a
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

## No visible surface

Nothing here has a UI or CLI-output surface of its own beyond the packaged upstream application,
so no mockups are carried.

---

## Verification

Not run — no package was built. The verification steps below are the ones the eventual
implementation must satisfy, carried over from the xemu plan and extended for this package's
specific risks. They are recorded here so whichever approach is chosen inherits them.

1. **Universe check — rpcs3 is not already in Ubuntu 26.04** — *already run, PASS (see above).*
2. **sha256 of every fetched upstream artifact is reproducible across two fetches** — *run for
   the tag tarball only, PASS: `a0908c99…c166f7` twice.*
3. **`.deb` builds cleanly in an `ubuntu:26.04` container** — not run.
4. **Binary is stripped and `${shlibs:Depends}` resolved to real sonames** — not run.
5. **`Depends` explicitly names the `dlopen`ed Vulkan/SDL/display libraries** that
   `dh_shlibdeps` cannot see — not run. This is the xemu near-miss and RPCS3 has the same profile.
6. **`lintian` is clean on both the `.deb` and the `.dsc` — zero `E:` and zero `W:`** — not run.
7. **Man page is installed** — not run.
8. **Smoke install in a clean 26.04 container; the binary starts and reports a real version**
   (not `local_build`) — not run.
9. **The package description and man page state that PS3 firmware is user-supplied and not
   shipped** — not run.
10. **`foundry-emulators-consoles-heavy` dependency chain still resolves** — not run.

---

## Result

**ESCALATED.** Upstream publishes no source archive bundling its submodules, so there is no
xemu-shaped path. Choosing between approach A (18-tarball pin table), B (AppImage repack), and
C (defer) is a design decision with real maintenance consequences, and it was not made here.
No files under `foundry-apt/` were touched; `foundry-emulators-consoles-heavy`,
`LICENSES-VENDORED.md`, `README.md`, and the root `CLAUDE.md` are all unmodified.
