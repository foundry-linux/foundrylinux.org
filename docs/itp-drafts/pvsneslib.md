# ITP draft: pvsneslib

## Upstream packaging audit

Audited the pinned upstream payload on 2026-08-05 with
`foundry-apt/scripts/audit-upstream-packaging.sh`. **No upstream packaging signals found:** no
upstream-maintained `debian/`, `.deb`/`.dsc` builder, `PKGBUILD`/`.spec`, or PPA/Launchpad/OBS/COPR
packaging reference.

## wnpp check

Searched wnpp for "pvsneslib" (SNES homebrew SDK, alekmaul). **No existing wnpp bug found** — search returned unrelated `snes9x` RFP/O bugs (relevant to the separate `snes9x-gtk` draft in this batch) and upstream GitHub/community links, nothing for pvsneslib itself.

## ITP draft

```
Package name    : pvsneslib
Version         : 4.5.0
Upstream contact: https://github.com/alekmaul/pvsneslib/issues
URL             : https://github.com/alekmaul/pvsneslib
Licence         : Zlib
Programming lang: C, with bundled 816-tcc (compiler) and WLA DX (assembler/linker)
Description     : SNES homebrew development kit — toolchain and library
```

PVSnesLib is a small, open and free development kit for the Nintendo
SNES: a complete C toolchain (the `816-tcc` compiler plus a pinned WLA DX
cross-assembler/linker) and a C library, with asset-conversion tools for
graphics, tilemaps, and sound (`gfx4snes`, `gfx2snes`, `smconv`,
`snesbrr`, `snestools`, `tmx2snes`, `bin2txt`, `constify`, `816-opt`).
Projects build via a Makefile including `$PVSNESLIB_HOME/devkitsnes/snes_rules`.

Given that `asar-snes-assembler` and `wla-dx` are both separately queued
in this batch, pvsneslib rounds out a complete SNES homebrew toolchain
story for Debian — this is a coherent, thematically-linked set of ITPs
rather than one-off tools.

## Packaging obstacle

**Our package ships upstream's pre-built Linux release, not a
from-source build.** `debian/control` says outright: "This package ships
upstream's pre-built Linux release under `/usr/lib/pvsneslib`" — the
`816-tcc` compiler binary and WLA DX binaries are pre-compiled, not
built at package-build time. Same category of problem as `ghidra`,
`ldtk`, `mesen2`: Debian main requires from-source builds. Unlike those,
though, this one may be the **easiest to actually fix** in this batch,
because PVSnesLib's dependencies (a C compiler toolchain, and WLA DX —
already separately queued as its own ITP here) are themselves buildable
from source with modest effort; a from-source PVSnesLib package could
plausibly `Build-Depends` on a packaged `wla-dx` rather than bundling
its own pinned copy, which would also resolve potential version-skew
between the two.

## Notes

- Single license (Zlib), DFSG-clean, permissive.
- Consider filing `wla-dx` first (or in tandem) given the dependency
  relationship noted above.
