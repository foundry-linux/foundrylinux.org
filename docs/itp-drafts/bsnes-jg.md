# ITP draft: bsnes-jg

## Upstream packaging audit

Audited the pinned upstream payload on 2026-08-05 with
`foundry-apt/scripts/audit-upstream-packaging.sh`. **No upstream packaging signals found:** no
upstream-maintained `debian/`, `.deb`/`.dsc` builder, `PKGBUILD`/`.spec`, or PPA/Launchpad/OBS/COPR
packaging reference.

## wnpp check

Searched wnpp for "bsnes-jg" / "bsnes jg" / "Jolly Good bsnes". **No existing wnpp bug found.** (General web results turned up the Jolly Good / libretro bsnes-jg project itself and Gentoo's `games-emulation/bsnes-jg` package, but nothing on bugs.debian.org.)

## ITP draft

```
Package name    : bsnes-jg
Version         : 2.1.0
Upstream contact: Rupert Carmichael <https://gitlab.com/jgemu/bsnes>
URL             : https://gitlab.com/jgemu/bsnes
Licence         : GPL-3.0-or-later (core), plus bundled deps: Zlib (byuuML),
                  Expat (gb), ISC (libco), BSD-2-Clause (libsamplerate),
                  LGPL-2.1-or-later (snes_spc)
Programming lang: C++11
Description     : cycle-accurate Super Nintendo / Super Famicom emulator
```

bsnes-jg is a cycle-accurate emulator for the Super Famicom / Super
Nintendo Entertainment System — a fork of byuu's bsnes v115, maintained
under the Jolly Good Emulation project with the stated goal of accurate
preservation. It prioritises correctness over speed and stays close to
ISO C++11 for long-term buildability. Emulation covers the SNES plus the
Super Game Boy, BS-X Satellaview, and Sufami Turbo, and enhancement
coprocessors (SA-1, SuperFX/GSU, DSP-n, Hitachi/ARM, Cx4, S-DD1, SPC7110,
OBC1, MSU1, …) with selectable HLE or low-level emulation.

Debian currently has no cycle-accurate SNES emulator (the historical
`snes9x`/`zsnes` packages were removed; see the `snes9x-gtk` draft in
this same set for why re-adding upstream Snes9x specifically is blocked
on licensing). bsnes-jg's DFSG-clean GPL-3+ core and its focus on
accuracy as a preservation/reference tool make it a strong ITP candidate
where Snes9x is not.

## Notes

- Bundled third-party code is all under permissive/copyleft DFSG-free
  licenses (Zlib, Expat, ISC, BSD-2-Clause, LGPL-2.1+) — no non-free
  content spotted in `debian/copyright`.
- Build-Depends: `pkg-config`, `libsdl2-dev`, `libsamplerate0-dev` — all
  in Debian main already.
- Consider whether Debian's `libsamplerate0` and any system-level
  `byuuML`/`libco` equivalents should be used instead of the bundled
  copies (`deps/byuuML`, `deps/libco`, `deps/gb`, `deps/snes_spc`) —
  Debian generally prefers unbundling where a system library exists.
  `libsamplerate0-dev` is already a Build-Depends, suggesting partial
  unbundling is already possible upstream; worth checking during
  actual packaging, not resolved here.
