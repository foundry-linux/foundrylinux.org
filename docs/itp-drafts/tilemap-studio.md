# ITP draft: tilemap-studio

## Upstream packaging audit

Audited the pinned upstream payload on 2026-08-05 with
`foundry-apt/scripts/audit-upstream-packaging.sh`. **No upstream packaging signals found:** no
upstream-maintained `debian/`, `.deb`/`.dsc` builder, `PKGBUILD`/`.spec`, or PPA/Launchpad/OBS/COPR
packaging reference.

## wnpp check

Searched wnpp for "tilemap-studio" (Rangi42). **No existing wnpp bug found** — search returned the upstream GitHub repo and an unrelated ITP (tilemill, a different map-design tool), nothing on bugs.debian.org for tilemap-studio.

## ITP draft

```
Package name    : tilemap-studio
Version         : 4.0.1
Upstream contact: Rangi <https://github.com/Rangi42>
URL             : https://github.com/Rangi42/tilemap-studio
Licence         : LGPL-3 (core), FLTK licence (statically-linked FLTK 1.4.5)
Programming lang: C++ (FLTK)
Description     : tilemap editor for Game Boy, GBC, GBA, NES and SNES
```

Tilemap Studio is a tilemap editor for retro consoles. It opens and
saves tilemaps in the native VRAM formats of the Game Boy, Game Boy
Color, Game Boy Advance, NES/Famicom, and Super NES/Super Famicom, plus
formats used by Pokémon Game Boy/GBA games and several homebrew and
decompilation projects. It pairs a tileset image with a tilemap,
supports per-tile palette/flip/priority attributes, and edits maps of
arbitrary size in a resizable, zoomable canvas — a modern, maintained
successor to 1990s tools like tUME.

Debian has no dedicated multi-console tilemap editor today (there's
`tiled`, but it's a different, more generic 2D-map tool, not
console-VRAM-format-aware). This fills a real, narrow gap for retro
console homebrew developers.

## Licence note — FLTK static linking

Tilemap Studio **statically links FLTK 1.4.5** (per our own
`debian/control`: "Tilemap Studio statically links FLTK 1.4.5
(hybrid Wayland/X11 build with GL)"). The FLTK licence is itself
DFSG-free (LGPL with a static-linking exception, specifically written to
permit exactly this), so this isn't a blocker — but Debian strongly
prefers dynamic linking against the system `libfltk1.3-dev`/`libfltk1.4-dev`
where possible, both for security-update reasons and to avoid bundled-copy
scrutiny. Given our own build notes mention needing "a Wayland-hybrid
build that upstream's X11 window code won't compile [without]" plus a
CRLF patch workaround for GNU patch 2.8, there may be a real technical
reason for the static link (a patched/hybrid FLTK build not available as
a system package) rather than mere convenience — whoever files the real
ITP should investigate whether Debian's packaged FLTK can be used as-is
or whether the hybrid build genuinely needs to stay bundled, and explain
that choice up front in the ITP to pre-empt review pushback.

## Notes

- Two licenses in `debian/copyright` (LGPL-3 core, FLTK for the bundled
  library) — both DFSG-free.
- `4.0.1-3foundry1` — our own changelog history (3 Foundry revisions on
  one upstream version) suggests some packaging churn; worth reviewing
  `debian/changelog` for what changed before treating our `debian/`
  tree as a clean starting point for a Debian submission.
