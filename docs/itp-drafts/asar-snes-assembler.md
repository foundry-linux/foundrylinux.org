# ITP draft: asar-snes-assembler

## wnpp check

Searched [bugs.debian.org wnpp](https://bugs.debian.org/cgi-bin/pkgreport.cgi?pkg=wnpp) (via web search, `site:bugs.debian.org` queries for "asar", "asar-snes-assembler", "SNES assembler"). **No existing wnpp bug found** — no ITP, RFP, O, or RFA for this package or upstream project (RPGHacker/asar).

Note: Debian's real `asar` source package name is taken by `@electron/asar` (the Electron archive tool) in some contexts — worth double-checking `apt-cache policy asar` / `packages.debian.org/search?keywords=asar` before filing, to confirm `asar-snes-assembler` (or a similarly disambiguated name) is the right Debian source-package name. Our local package is already named `asar-snes-assembler` for exactly this reason (see `debian/changelog`: renamed from `asar-snes` to match Repology and avoid collision).

## ITP draft

```
Package name    : asar-snes-assembler
Version         : 1.91
Upstream contact: RPGHacker <https://github.com/RPGHacker/asar/issues>
URL             : https://github.com/RPGHacker/asar
Licence         : LGPL-3+ (core), WTFPL-2+ (src/asar-dll-bindings/c/asardll.{h,c})
Programming lang: C++
Description     : SNES 65816/SPC700/SuperFX cross-assembler
```

asar is a multi-architecture cross-assembler targeting the SNES and its
co-processors: 65816 (main CPU), SPC700 (audio DSP), and SuperFX (graphics
co-processor). It is used extensively for SNES ROM hacking, homebrew
development, and Super Mario World ROM hacking toolchains. Features include
a macro system, math expressions, RAM map tables, multiple-pass assembly,
symbol file output (wla/nocash format), and optional ROM checksum fixing.
It reads an `.asm` source file and patches or creates a `.sfc`/`.smc` ROM.

Debian ships several retro cross-assemblers (`cc65`, `xa65`, `dasm`) but
nothing targeting the SNES's 65816/SPC700/SuperFX combination specifically.
asar is the de facto standard assembler in the SNES ROM-hacking and
homebrew community (used by toolchains such as pvsneslib's build chain and
countless Super Mario World hacking projects), and packaging it would fill
a real gap in Debian's retro-development coverage alongside packages like
`wla-dx`.

## Notes

- Clean case: single upstream license (LGPL-3+) plus one small WTFPL-2+
  file for the C DLL bindings header/source, both DFSG-free.
- CMake build (`Build-Depends: debhelper-compat (= 13), cmake`), no
  unusual build dependencies.
