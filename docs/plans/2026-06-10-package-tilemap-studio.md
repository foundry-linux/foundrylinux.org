# Plan: package Tilemap Studio for apt.foundrylinux.org

**Date**: 2026-06-10
**Goal**: Ship Tilemap Studio (v4.0.1, LGPL-3) — the retro-accurate tile/map editor and modern
successor to tUME — in `apt.foundrylinux.org`, wired into `foundry-retro-tools`.
**Linkage decision** (evidence in
[docs/investigations/2026-06-10-tilemap-studio-fltk-linkage.md](../investigations/2026-06-10-tilemap-studio-fltk-linkage.md)):
build **FLTK 1.4.5 X11-only and static-link** it; png/zlib/jpeg come from the system shared libs.

## Why not simpler

- Ubuntu 26.04 ships FLTK **1.4.4** as a **Wayland-hybrid** build; upstream's X11 window code
  (`fl_display`, `XA_CARDINAL`, …) won't compile or run against it. So we vendor FLTK 1.4.5.
- A separate FLTK-1.4.5 apt package would clash with the system `libfltk1.4` SONAME and has no other
  consumer → static-link instead.

## Package shape — `foundry-apt/packages/tilemap-studio/`

- `build.sh` — fetches **two** sha256-pinned tarballs: Tilemap Studio v4.0.1
  (`aba34651…`) and FLTK 1.4.5 (`7715e69c…`), places FLTK in `lib/fltk`, overlays `debian/`, runs
  `dpkg-buildpackage`.
- `debian/rules` — `override_dh_auto_configure/build` builds FLTK X11-only with
  `FLTK_USE_SYSTEM_{LIBPNG,ZLIB,LIBJPEG}=1` + `CMAKE_POSITION_INDEPENDENT_CODE=ON`, installs it into
  the source prefix, then puts `bin/` on `PATH` so the upstream Makefile's bare `fltk-config`
  resolves; `override_dh_auto_install` installs the binary, icons and `.desktop`.
- `debian/patches/` (quilt, both **upstreamable**):
  - `0001-x11-platform-includes.patch` — add `<FL/platform.H>`/`<X11/Xlib.h>`/`<X11/Xatom.h>` to
    `utils.h` (FLTK 1.4.5 headers no longer leak them).
  - `0002-cstring-in-preferences.patch` — `#include <cstring>` for GCC 15.
- `debian/tilemap-studio.lintian-overrides` — one justified `embedded-library [… fltk]` (only FLTK
  is static; png/zlib/jpeg are system libs).
- `debian/man/tilemapstudio.1`, `debian/tilemap-studio.desktop`, DEP-5 `debian/copyright`.

## Steps

1. ~~Universe check~~ — not in 26.04 universe; proceed. ✅
2. ~~Pin both tarballs~~ (sha256 reproducible). ✅
3. ~~Prove the static build recipe in Docker~~ — PIE, 2.1 MB, `ldd` shows no libfltk (static), png/z
   dynamic. ✅
4. ~~Author the `debian/` tree + patches + build.sh~~. ✅
5. **Build via `build.sh` in `ubuntu:26.04` + lintian clean** (zero E/W beyond the FLTK override).
6. **Wire into `foundry-retro-tools`**: add `tilemap-studio` to `Depends:`, `dch` a changelog bump.
7. **Local dependency-chain smoke test** (`dpkg-scanpackages` + install in a fresh container).
8. **Commit**, then `task sync` / tag to publish; verify live `apt install tilemap-studio`.
9. **Upstream the two patches** — open PRs against `Rangi42/tilemap-studio`, record the URLs in each
   patch's `Forwarded:` header and the changelog. (New mandatory last step in the `/package` skill,
   added this session.)

## Verification

1. `bash build.sh` in `ubuntu:26.04` produces `dist/tilemap-studio_4.0.1-1foundry1_amd64.deb`.
2. `lintian` returns clean apart from the documented FLTK `embedded-library` override.
3. `dpkg-deb -I` shows `${shlibs:Depends}` resolved (libpng16, libz, cairo, pango, X11), no libfltk.
4. `file usr/bin/tilemapstudio` reports `pie executable`, stripped after `dh_strip`.
5. Fresh-container `apt install ./tilemap-studio_*.deb` succeeds; binary launches `--help`/version.
6. `foundry-retro-tools` resolves `tilemap-studio` via `apt-cache depends`.

(Paste raw command output under each step as it's run, per the SRC plan-verification convention.)

## Upstreaming (step 9 detail)

Both patches are toolchain/portability fixes that help every Linux packager, not Debian glue, so they
go upstream:

- Fork `Rangi42/tilemap-studio`, branch per fix, apply the hunk, PR with the DEP-3 `Description` as
  the body (note build env: Ubuntu 26.04, GCC 15, FLTK 1.4.5).
- Update `Forwarded: <PR-url>` in `debian/patches/000*.patch` and note in `debian/changelog`.
- Drop each patch once an upstream release ships the fix.
