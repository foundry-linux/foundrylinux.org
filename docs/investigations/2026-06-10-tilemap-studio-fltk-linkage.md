# Packaging Tilemap Studio: how to handle its FLTK 1.4.5 dependency

**Date**: 2026-06-10
**Context**: Vendoring [Tilemap Studio](https://github.com/Rangi42/tilemap-studio) (`v4.0.1`, **LGPL-3**)
into `apt.foundrylinux.org` as the retro-accurate tile/map editor for `foundry-retro-tools` (see the
[tUME successor investigation](2026-06-10-tume-map-editor-port-and-successors.md)). It depends on
**FLTK 1.4.5**, which Ubuntu 26.04 does not ship — forcing a linkage decision.
**Decision**: **Static-link FLTK 1.4.5 into the Tilemap Studio binary** (option A below).

---

## The problem

Tilemap Studio is a C++17/FLTK app. Its build wants **FLTK 1.4.5, built X11-only**
(`FLTK_BACKEND_WAYLAND=0`) and **static-linked** (the Makefile defaults to
`fltk-config --use-images --ldstaticflags`). Ubuntu 26.04 ships something close but not equal.

### Evidence gathered

| Fact | Finding |
|---|---|
| Not in universe | `apt-cache policy tilemap-studio` → empty on `ubuntu:26.04`. We must package it. |
| Upstream version | latest tag `v4.0.1`; license **LGPL-3** (`LICENSE.md`). |
| tarball sha256 | `aba346519dc3be7f46bacdd481fa7fce61c187032544593c78d1e40c98210c97` (reproducible across 2 fetches) |
| System FLTK | `libfltk1.4-dev` = **1.4.4-3**, built **Wayland-hybrid**. (`libfltk1.3-dev` = 1.3.11 also present.) |
| **Compile vs system FLTK 1.4.4** | ❌ **FAILS** — `fl_display`, `fl_xid`, `Pixmap`, `XA_CARDINAL`, `XInternAtom` undeclared in `main-window.cpp`. |
| Why it fails | Upstream's `maximize()` / `apply_transparency()` call X11 symbols directly under `#ifndef _WIN32`, including only `<X11/xpm.h>`. They resolve transitively **only on an X11-only FLTK build**. Debian's Wayland-hybrid FLTK hides them — and even force-included, those calls risk a **null-`fl_display` crash under a Wayland session**. This is precisely why upstream builds `FLTK_BACKEND_WAYLAND=0`. |
| Other FLTK consumers in foundry-apt | **None.** f9dasm, ghidra, libvgm, vgmstream, ppsspp, snes9x-gtk (GTK), task, ruff, python3-*, blender-asset-finder — none use FLTK. Tilemap Studio would be the *only* consumer. |

So **using system FLTK 1.4.4 is off the table** (option C): it doesn't compile, and the Wayland-hybrid
runtime is the wrong backend for this code. We must supply FLTK 1.4.5 ourselves — the question is *how*.

---

## Options compared

| | **A. Static-link FLTK 1.4.5 into tilemap-studio** | **B. Ship FLTK 1.4.5 as a separate foundry-apt package** | **C. System FLTK 1.4.4** |
|---|---|---|---|
| Builds at all? | ✅ upstream's tested config | ✅ (we build the lib) | ❌ **fails to compile** |
| Runtime deps | system X11 / libpng / zlib only — **no libfltk dep** | our `libfltk1.4.5` shared lib + X11 | — |
| Conflict with system `libfltk1.4` (1.4.4) | **none** — FLTK baked into the binary; nothing on disk collides | ⚠️ **real** — same SONAME `libfltk.so.1.4`; our 1.4.5 either shadows Ubuntu's 1.4.4 system-wide (affecting every FLTK app) or needs a private name + path | — |
| .deb size | larger binary (~3–5 MB est.) | small app .deb + ~1.5 MB lib .deb | smallest |
| Reuse benefit | none lost — only consumer is tilemap-studio | the *point* of B — but **zero other consumers today** | — |
| Security / CVE response | rebuild tilemap-studio on an FLTK CVE | rebuild one lib; all consumers pick it up | distro handles it |
| Debian-policy | needs a justified `embedded-library` lintian override | cleanest in principle | — |
| Divergence risk | **lowest** — matches upstream exactly | we own an X11-only FLTK build indefinitely | high |
| CI cost | compiles FLTK once per tilemap build (~few min) | compiles FLTK once per FLTK release | none |

---

## Decision: **A — static-link FLTK 1.4.5**

Deciding factors:

1. **B's shared lib collides with Ubuntu's `libfltk1.4` (1.4.4) at the SONAME level** (`libfltk.so.1.4`).
   Shipping a system-wide FLTK 1.4.5 means every other FLTK app on the machine silently gets our
   build — a distro-wide blast radius for the sake of one map editor.
2. **Reuse is the only reason to prefer B, and there are no other FLTK consumers** in foundry-apt.
   You'd maintain a shared library for a single client.
3. **A matches upstream's own default** (`--ldstaticflags`, X11-only) — least chance of surprise; the
   binary depends on nothing but stock X11/png/zlib.
4. Security exposure of a **local, non-networked tile editor** statically embedding FLTK is low;
   rebuild-on-CVE is acceptable.

**Honest cost of A**: a few-MB binary, a one-line justified `embedded-library` lintian override, and
CI compiling FLTK 1.4.5 on each tilemap-studio build (~few minutes).

**What would flip the decision to B**: if foundry plans to ship *several* FLTK-1.4.5 apps, or a
system-wide FLTK upgrade. Then a shared package amortizes — and it must get a **distinct package name
and private install path** to avoid the 1.4.4 SONAME clash. Given today's package set, that's
speculative; revisit if a second FLTK consumer appears.

---

## Implementation sketch (option A)

A vendored-upstream `foundry-apt/packages/tilemap-studio/` per the `/package` skill, where the build
fetches **two** sha256-pinned tarballs and builds FLTK before the app:

- **tilemap-studio v4.0.1** — sha256 `aba34651…`
- **FLTK 1.4.5** (`github.com/fltk/fltk` tag `release-1.4.5`) — sha256
  `7715e69ce081fa9ce6da48bb0dd3b07a4cf2cf937813814c04272f36fff593ea`

`build.sh` / `debian/rules` steps:

1. Build FLTK 1.4.5 into the source tree prefix with upstream's flags:
   `cmake -B build -D FLTK_BACKEND_WAYLAND=0 -D FLTK_GRAPHICS_CAIRO=1 -D FLTK_BUILD_GL=0
   -D FLTK_BUILD_TEST=0 -D FLTK_ABI_VERSION=10405 -D FLTK_USE_SYSTEM_LIBPNG=0
   -D FLTK_USE_SYSTEM_ZLIB=0 …` → `cmake --build` → `cmake --install` (gives `bin/fltk-config`).
2. `make fltk-config=bin/fltk-config` — builds `bin/tilemapstudio`, statically linked.
3. `override_dh_auto_install`: install `bin/tilemapstudio` → `/usr/bin/`, plus `res/app.xpm` and a
   `.desktop` file; author `debian/man/tilemapstudio.1` (Policy §12.1).
4. **Build-Depends**: `debhelper-compat (= 13), cmake, pkg-config, make, g++, zlib1g-dev, libpng-dev,
   libxpm-dev, libx11-dev, libxft-dev, libxinerama-dev, libfontconfig1-dev, libxext-dev,
   libxrender-dev, libxfixes-dev, libcairo2-dev, libpango1.0-dev`. **Depends** resolves to
   `${shlibs:Depends}` (X11/png/zlib/cairo/pango) only — no libfltk.
5. `debian/tilemap-studio.lintian-overrides`: justified `embedded-library` for the static FLTK.
6. Wire into `foundry-retro-tools` `Depends:` + `dch` a changelog entry.

> **Status**: decision recorded; not yet built. The FLTK-1.4.5 + app compile PoC was set up
> (tarballs pinned, X11-only cmake recipe staged) but not run to completion. Next step: finish the
> static build to capture the real binary size and `ldd` output, then generate the `debian/` tree.

---

## Sources

- [Tilemap Studio](https://github.com/Rangi42/tilemap-studio) · [INSTALL.md (Linux/FLTK build)](https://github.com/Rangi42/tilemap-studio/blob/master/INSTALL.md) · [LICENSE.md (LGPL-3)](https://github.com/Rangi42/tilemap-studio/blob/master/LICENSE.md)
- [FLTK 1.4.5 release](https://github.com/fltk/fltk/releases/tag/release-1.4.5)
- Compile/version evidence: captured in this session against `ubuntu:26.04` (FLTK 1.4.4-3; tilemap-studio compile failure on `fl_display`/`Pixmap`/`XA_CARDINAL`).
