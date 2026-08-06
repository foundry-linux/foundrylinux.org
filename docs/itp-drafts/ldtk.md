# ITP draft: ldtk

## Upstream packaging audit

Audited the pinned upstream payload on 2026-08-05 with
`foundry-apt/scripts/audit-upstream-packaging.sh`. **No upstream packaging signals found:** no
upstream-maintained `debian/`, `.deb`/`.dsc` builder, `PKGBUILD`/`.spec`, or PPA/Launchpad/OBS/COPR
packaging reference.

## wnpp check

Searched wnpp for "ldtk" (level editor / Deepnight Games). **No existing wnpp bug found.**

## ITP draft

```
Package name    : ldtk
Version         : 1.5.3
Upstream contact: Sébastien Bénard - Deepnight Games
URL             : https://ldtk.io/
Licence         : Expat (MIT)
Programming lang: Haxe (core editor) + Electron/TypeScript shell
Description     : Level Designer Toolkit — modern 2D level editor
```

LDtk is a free, open-source (MIT) 2D level editor for game developers,
built around tile layers, auto-tiling rules, entities, and a clean JSON
(`.ldtk`) export consumed by LibGDX, Godot, Unity, raylib, Heaps, and
more. It's a modern, actively-maintained successor to older tools like
Tiled for teams that want the LDtk-specific JSON pipeline.

LDtk has a substantial and growing user base in the indie/game-jam
community and no packaging in Debian today. It would be a genuinely
useful addition to Debian's small stable of game-development tools.

## Packaging obstacle — this is not a from-source build

**Upstream ships no source tarball and no `.deb`.** Our own package
(`foundry-apt/packages/ldtk/build.sh`) is explicit about this: it
sha256-pins the official `ubuntu-distribution.zip`, extracts the
**AppImage's AppDir at build time**, and repackages the pre-built
Electron binary into `/opt/ldtk`. This is the same category of problem
as `ghidra`, `mesen2`, and `pvsneslib` in this batch: Debian main
requires building from source, and LDtk's core editor logic (written in
Haxe, per upstream's repo) is not built from source here — we ship
upstream's pre-compiled Electron/Haxe-to-JS output.

A real Debian ITP for LDtk would need to build from the Haxe source
(`github.com/deepnight/ldtk`) using Debian's Electron packaging
conventions (bundling a stripped Chromium via `electron` from Debian,
not a private Electron copy) — a meaningfully larger undertaking than
"repackage the AppImage," which is what our Foundry package does today.

## Verdict

Legitimate long-term candidate given its MIT license and real demand,
but **the current Foundry packaging approach (AppImage repack) is not
Debian-ITP-ready as-is.** Flag this the same way as `ghidra`/`mesen2`:
worth pursuing only as a dedicated from-source packaging effort, not a
quick ITP filing.
