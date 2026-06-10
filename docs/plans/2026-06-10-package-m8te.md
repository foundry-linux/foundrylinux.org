# Package M8TE (SNES tile/map editor) as a .deb

**Date:** 2026-06-10
**Skill:** `/package` on [github.com/nesdoug/M8TE](https://github.com/nesdoug/M8TE)
**Status:** built, lintian-clean, verified; pending publish

## What M8TE is

[M8TE](https://github.com/nesdoug/M8TE) ("mate") is a SNES 8bpp tile, tilemap
and palette editor by Doug Fraker (nesdoug.com) — Mode 3/4 (planar) and Mode 7
(linear). MIT (Expat) licensed. Latest release **1.5** (Dec 2022).

It is a **C# .NET Framework 4.5.2 WinForms** application — the first Mono/.NET
package in foundry-apt. The upstream README states it "works with MONO on
non-Windows systems", and upstream ships a pre-built CIL assembly
(`M8TE.exe`) as the official 1.5 release artifact.

## Approach: prebuilt CIL repack (not a Mono source build)

- A CIL assembly is **architecture-independent IL** → `Architecture: all`.
- Upstream officially distributes `M8TE.exe` as its 1.5 release. The committed
  `bin/Release/M8TE.exe` is **byte-identical** to that release download
  (sha256 `f6470bfa…`). Building the VS2014-era `.csproj` under Mono's msbuild
  would be fragile and produce equivalent IL — zero benefit.
- So we vendor the **tag-1.5 source tarball** (one sha256-pinned download:
  `0c72a643…`), install the committed prebuilt `M8TE.exe` under
  `/usr/lib/m8te/`, and ship a `/usr/bin/m8te` wrapper that execs `mono`.
- Mono in Ubuntu 26.04 was **repackaged**: the old per-assembly
  `libmono-system-windows-forms4.0-cil` packages are gone, consolidated into a
  single `mono-libraries` (Arch: all) that ships `System.Windows.Forms.dll` +
  `System.Drawing.dll`. Runtime deps: `mono-runtime`, `mono-libraries`,
  `libgdiplus`. `libgtk2.0-0t64` is a Recommends (native WinForms theming).

## Wiring (decided with Will, 2026-06-10)

M8TE is **both** a retro tool and an art tool, so it joins both category
metapackages (precedent: mtpaint/grafx2 live in both pixel-art and art):

- `foundry-retro-tools` (1.0.7 → **1.0.8**) — this is what lands M8TE in
  **anvil** (retro-tools ∈ core ∈ anvil). Description broadened from
  RE/porting to also cover retro-console asset authoring.
- `foundry-pixel-art` (1.0.0 → **1.0.1**) — the art categorization.

Consequence: retro-tools ∈ core, and the Phase 2 devbox installs core, so this
also pulls M8TE + the Mono runtime (~30–40 MB) into the devbox container. Core
already ships GUI apps (snes9x-gtk), so not unprecedented; Will accepted this.

> Open (separate decision): collapse `foundry-sprite` into `foundry-anvil`.
> sprite's unique-over-anvil payload = pixel-art, art, trackers, daw, digikam,
> showfoto. Mechanically trivial; the trade is the default ISO (anvil) gaining
> the full heavy art+DAW+photo stack. Not done in this change.

## Files

```
foundry-apt/packages/m8te/
  build.sh                     fetch tag-1.5 tarball (sha256-pinned), install the
                               prebuilt M8TE.exe, build (Arch: all)
  debian/control               Depends: mono-runtime, mono-libraries, libgdiplus
  debian/rules                 installs assembly + mono wrapper + desktop + icon
  debian/m8te.wrapper          #!/bin/sh exec /usr/bin/mono /usr/lib/m8te/M8TE.exe
  debian/m8te.desktop          Categories=Graphics;2DGraphics;
  debian/man/m8te.1            hand-written (GUI app, no CLI --help)
  debian/{changelog,copyright,watch,source/format,m8te.manpages,patches/series}
```

## Verification

### 1. Universe check (BLOCKING) — not a duplicate

```
$ docker run --rm ubuntu:26.04 apt-cache policy m8te
(empty — no candidate)
```
**PASS** — m8te is not in Ubuntu 26.04.

### 2. sha256 pin reproducible + prebuilt matches official release

```
tarball sha256: 0c72a643ef857308614a709882e39d2d00685e931f1f0d40aeedc14f50ebd6be
committed bin/Release/M8TE.exe: f6470bfa7315b44bbc5ac75d7959155ffbe03fc92ef5b05f47e2a66f8dbcaa94
release 1.5 asset M8TE.exe:     f6470bfa7315b44bbc5ac75d7959155ffbe03fc92ef5b05f47e2a66f8dbcaa94
```
**PASS** — committed prebuilt == official release download.

### 3. Build + lintian (in ubuntu:26.04)

```
OK   /repo/dist/m8te_1.5-1foundry1_all.deb  (40572 bytes)
Depends: mono-runtime, mono-libraries, libgdiplus
Recommends: libgtk2.0-0t64
contents: /usr/bin/m8te, /usr/lib/m8te/M8TE.exe (121344), m8te.desktop,
          hicolor/32x32/apps/m8te.png, man1/m8te.1.gz
LINTIAN: running with root privileges is not recommended!   (only advisory)
```
**PASS** — lintian clean, zero E/W.

### 4. Smoke install + headless Mono launch

```
apt resolves mono-runtime + mono-libraries + libgdiplus on install (no errors)
desktop-file-validate: OK
xvfb-run -a timeout 8 m8te → EXIT 124   (timeout killed a running GUI process)
```
**PASS** — the installed `/usr/bin/m8te` wrapper launches M8TE under Mono.

### 5. Metapackage chain resolves

```
foundry-retro-tools 1.0.8 + foundry-pixel-art 1.0.1: lintian clean
apt-get install -s: Inst m8te + mono-runtime + mono-libraries + libgdiplus
```
**PASS** — both metapackages pull m8te and its Mono chain cleanly.

## Publish

`task bump` (sync foundry-apt → publish repo, tag next patch) → CI builds in
`ubuntu:26.04`, signs, syncs to R2. Then verify live `apt install m8te`.
