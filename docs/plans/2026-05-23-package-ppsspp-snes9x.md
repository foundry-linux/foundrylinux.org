# Package ppsspp and snes9x-gtk for foundry-emulators

**Date:** 2026-05-23  
**Status:** In progress  
**Linked investigation:** [2026-05-23-dropped-emulators.md](../investigations/2026-05-23-dropped-emulators.md)

---

## Context

`foundry-emulators-consoles-heavy` currently lists ppsspp and snes9x-gtk as deps that don't exist in Ubuntu 26.04. The investigation confirmed both are viable `/package` targets with no remaining blockers. Both ship in `foundry-emulators-consoles-heavy`. ppsspp is higher priority (weekly upstream releases, 13-year-old stalled Debian ITP, genuine gap in PSP coverage).

---

## ppsspp

### Upstream

| Field | Value |
|---|---|
| Version | 1.20.4 |
| Tag | v1.20.4 |
| Commit | fa50bb1976065c4f8b1b47af227d367fe9771555 |
| Source | https://github.com/hrydgard/ppsspp |
| License | GPL-2+ (core); PSPSDK BSD-3; xbrz GPL-2+ |
| Build system | CMake |
| Installed binary | `ppsspp` (renamed from upstream's `PPSSPPSDL`) |

### Submodule problem

GitHub auto-tarballs do **not** include git submodule content. The following submodules are empty in the v1.20.4 tarball:

| Submodule | Required for | Workaround |
|---|---|---|
| `ext/glslang` | Vulkan shader compilation | git clone (no system alternative) |
| `ext/SPIRV-Cross` | Vulkan SPIR-V cross-compilation | git clone (no system alternative) |
| `ext/armips` | JIT assembler (has own sub-submodule) | git clone --recursive |
| `ext/cpu_features` | CPU feature detection | git clone (no system alternative) |
| `ext/lua` | Lua scripting (unconditional) | git clone (no system alternative) |
| `ext/rcheevos` | RetroAchievements (unconditional) | git clone (no system alternative) |
| `ext/aemu_postoffice` | network/netplay (unconditional on Linux) | git clone (no system alternative) |
| `ext/nanosvg` | SVG headers (included by UIAtlas.cpp) | git clone (header-only) |
| `ext/libchdr` | CHD file support | `USE_SYSTEM_LIBCHDR=ON` (libchdr-dev in 26.04) |
| `ext/zstd` | compression | `USE_SYSTEM_ZSTD=ON` |
| `ext/miniupnp` | UPnP port forwarding | `USE_SYSTEM_MINIUPNPC=ON` |
| `ext/freetype` | font rendering | `USE_SYSTEM_FREETYPE=ON` |
| `ext/rapidjson` | JSON | `USE_SYSTEM_RAPIDJSON=ON` + `rapidjson-dev` Build-Dep required |
| `ffmpeg` | media decoding | `USE_SYSTEM_FFMPEG=ON` |
| `ext/discord-rpc` | Discord integration | `USE_DISCORD=OFF` |

**Solution in `build.sh`:** git clone the repo at `v1.20.4` (verified by commit hash), then init the required submodules with `--depth 1`:

```bash
git clone --depth 1 --branch v1.20.4 https://github.com/hrydgard/ppsspp.git ppsspp-1.20.4
cd ppsspp-1.20.4
git submodule update --init --depth 1 \
    ext/glslang ext/SPIRV-Cross ext/cpu_features \
    ext/lua ext/rcheevos ext/aemu_postoffice ext/nanosvg
# armips has its own sub-submodule (ext/filesystem) — use --recursive
git submodule update --init --recursive --depth 1 ext/armips
# Verify commit hash
```

### cmake flags

```cmake
-DCMAKE_BUILD_TYPE=Release
-DUSE_SYSTEM_FFMPEG=ON
-DUSE_SYSTEM_LIBSDL2=ON
-DUSE_SYSTEM_LIBPNG=ON
-DUSE_SYSTEM_SNAPPY=ON
-DUSE_SYSTEM_LIBZIP=ON
-DUSE_SYSTEM_ZSTD=ON
-DUSE_SYSTEM_MINIUPNPC=ON
-DUSE_SYSTEM_FREETYPE=ON
-DUSE_SYSTEM_RAPIDJSON=ON
-DUSE_DISCORD=OFF
```

### Binary rename

cmake installs the binary as `PPSSPPSDL`. `debian/rules` renames it to `ppsspp` in `override_dh_auto_install` (and patches the installed `.desktop` file accordingly).

### Build-Depends

```
debhelper-compat (= 13), cmake, pkg-config, git,
libsdl2-dev, libgl-dev, libglu-dev, libsdl2-ttf-dev, libfontconfig1-dev,
libavcodec-dev, libavformat-dev, libavutil-dev, libavdevice-dev, libswscale-dev, libswresample-dev,
libvulkan-dev, libpng-dev, libsnappy-dev, libzip-dev, libzstd-dev,
libminiupnpc-dev, libfreetype-dev, libchdr-dev, libopenxr-dev, rapidjson-dev
```

Notes:
- `libchdr-dev` — system replacement for `ext/libchdr` (available in ubuntu 26.04)
- `libopenxr-dev` — `Common/VR/OpenXRLoader.h` unconditionally includes `<openxr/openxr.h>` even without VR/OpenXR enabled; cmake adds `-Iext/OpenXR-SDK/include` unconditionally (CMakeLists.txt:648)
- `rapidjson-dev` — `Core/RetroAchievements.cpp` includes `<rapidjson/document.h>`; `-DUSE_SYSTEM_RAPIDJSON=ON` redirects to system headers but the package still needs to be installed

### Files

```
foundry-apt/packages/ppsspp/
├── build.sh
└── debian/
    ├── control
    ├── changelog
    ├── copyright
    ├── rules
    ├── source/format
    ├── watch
    ├── ppsspp.manpages
    └── man/ppsspp.1
```

---

## snes9x-gtk

### Upstream

| Field | Value |
|---|---|
| Version | 1.63 |
| Tag | v1.63 (or 1.63?) |
| Source | https://github.com/snes9xgit/snes9x |
| License | Snes9x custom (free/redistributable but not GPL) |
| Build system | CMake (GTK3 frontend in `gtk/` subdirectory) |
| Installed binary | `snes9x-gtk` |

### Submodule status

TBD — check at packaging time. Likely similar submodule issues to ppsspp; check `ext/` and `.gitmodules` before choosing tarball vs clone approach.

### License note

Snes9x uses a custom license (not GPL/MIT/BSD). `dh_make --copyright` will not have a shortname for it. Hand-write `debian/copyright` from the upstream `LICENSE` file.

---

## Metapackage wiring

After each package builds cleanly:
1. Add to `foundry-emulators-consoles-heavy/debian/control` `Depends:` field
2. Add new `debian/changelog` entry: `dch -v <new-version> -D resolute "Add ppsspp / snes9x-gtk"`
3. Build and verify the metapackage still resolves cleanly

---

## Verification

For each packaged binary (run in `ubuntu:26.04` container):

1. `dpkg-deb -I dist/<name>_*.deb` — confirm Depends has resolved shlibs
2. `dpkg-deb -c dist/<name>_*.deb` — confirm `/usr/bin/<name>` and `/usr/share/man/man1/<name>.1.gz` present
3. `lintian dist/<name>_*.deb` — zero E:/W: lines
4. `apt-get install -y dist/<name>_*.deb && <name> --version` — runs without error
5. `foundry-emulators-consoles-heavy` metapackage still installs cleanly after Depends update
