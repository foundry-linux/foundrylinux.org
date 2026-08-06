# ITP draft: f9dasm

## Upstream packaging audit

Audited the pinned upstream payload on 2026-08-05 with
`foundry-apt/scripts/audit-upstream-packaging.sh`. **No upstream packaging signals found:** no
upstream-maintained `debian/`, `.deb`/`.dsc` builder, `PKGBUILD`/`.spec`, or PPA/Launchpad/OBS/COPR
packaging reference.

## wnpp check

Searched wnpp for "f9dasm" (Motorola 6800/6809/6309 disassembler). **No existing wnpp bug found** — results returned other, unrelated disassembler ITPs (apngdis, edb-debugger, zydis, smali, libudis86) but nothing for f9dasm itself or its author (Hermann Seib / Arto Salmi).

## ITP draft

```
Package name    : f9dasm
Version         : 1.83
Upstream contact: Hermann Seib <https://github.com/Arakula/f9dasm/issues>
URL             : https://github.com/Arakula/f9dasm
Licence         : GPL-2.0+
Programming lang: C
Description     : Motorola 6800/6809/6309 family disassembler
```

f9dasm is a portable disassembler for the Motorola 6800, 6801, 6802,
6803, 6808, 6809 and Hitachi 6301, 6303, 6309 microprocessors. It reads
Intel Hex, Motorola S-record, FLEX9 binary, and raw binary input, and
emits TSC assembler-compatible (or, via the RB_VARIANT build flag,
Rainer Buchty's variant) output. Information files with directives can
guide disassembly of non-trivial binaries. Three companion utilities are
included: `hex2bin` (Intel Hex → binary), `mot2bin` (Motorola S-record →
binary), and `cmd2mot` (FLEX CMD → Motorola S-record). Based on Arto
Salmi's original 6809/6309 disassembler engine, with substantial
additions by Hermann Seib, Colin Bourassa, and Rainer Buchty.

Debian has disassemblers for several other classic architectures (via
`binutils`'s targets, `z80asm`/`z80dasm`-adjacent tooling) but nothing
specifically for the Motorola 6800/6809/6309 family, which remains
relevant to retro-computing (TRS-80 Color Computer, Dragon 32/64,
Vectrex peripherals) and homebrew development. A small, focused,
long-maintained (2000-2022 development span) tool with a clean
GPL-2.0+ license is a straightforward Debian fit.

## Notes

- Single license throughout, DFSG-clean.
- Build-Depends: `debhelper-compat (= 13)` only — no external libraries,
  about as simple a build as this list gets.
- Foundry's own package has already needed two man-page lint fixes
  (`f9dasm (1.83-1foundry3)`, mandoc `.PP after .SH` warnings) — worth
  confirming those don't reappear if the actual Debian package rebuilds
  the man pages from a different source.
