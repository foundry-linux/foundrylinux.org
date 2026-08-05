# ITP draft: m8te

## wnpp check

Searched wnpp for "m8te" (SNES tile/map editor, nesdoug). **No existing wnpp bug found** — general search results returned the upstream GitHub repo and unrelated NES/SNES tile-editor forum threads, nothing on bugs.debian.org.

## ITP draft

```
Package name    : m8te
Version         : 1.5
Upstream contact: Doug Fraker <https://www.nesdoug.com/>
URL             : https://github.com/nesdoug/M8TE
Licence         : Expat (MIT)
Programming lang: C# (Mono/.NET WinForms)
Description     : SNES 8bpp tile, tilemap and palette editor
```

M8TE ("mate") is a graphical editor for Super Nintendo / Super Famicom
8-bits-per-pixel graphics: it generates, edits and arranges SNES tiles,
tilemaps and palettes for homebrew SNES development, covering Mode 3/4
(planar, 32x32 map, up to four tilesets, tile flipping) and Mode 7
(linear, 128x128 map via sixteen 32x32 sub-maps, single tileset). Import
images as tiles or palettes, paint maps with multi-select/map-edit
brushes, sort palettes by brightness, and export data ready for an SNES
project.

## Packaging obstacle

**Upstream ships a pre-built CIL assembly, not source built at package
time.** Our own `build.sh` fetches the tagged source tarball but notes:
"M8TE is a Mono/.NET WinForms app: upstream ships a pre-built CIL
assembly (sha256 f6470bfa...)". If that pre-built assembly is what ends
up in the `.deb` (rather than a `mcs`/`csc`-from-source build at package
build time), this has the same "not built from source" problem as
`ghidra`/`ldtk`/`mesen2`/`pvsneslib` in this batch, and would need to be
resolved (compile the C# source with Mono's compiler during the Debian
build) before a from-source Debian package is possible. This needs
verifying directly against `foundry-apt/packages/m8te/build.sh` by
whoever picks up the actual packaging — flagged here, not resolved.

**Runtime dependency on Mono** — `/usr/bin/m8te` invokes the app via
`mono`. Debian does carry Mono (`mono-runtime`/`mono-complete`), so this
is a Depends, not a blocker, but it's a heavier runtime footprint than
most of the other tools in this batch and worth being upfront about in
the ITP.

## Notes

- Single license (Expat/MIT), DFSG-clean.
- Niche but real utility for the SNES homebrew community, filling a gap
  similar to `tilemap-studio` but for SNES-specific 8bpp Mode 3/4/7
  workflows Tilemap Studio doesn't cover the same way.
