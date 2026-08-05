# ITP draft: wla-dx

## wnpp check

Searched wnpp for "wla-dx" / "wla dx" (multi-CPU cross assembler, vhelin). **No existing wnpp bug found** — search returned an unrelated 6502-family cross-assembler ITP (`acme`) and general WNPP process pages, nothing for wla-dx itself.

## ITP draft

```
Package name    : wla-dx
Version         : 10.6
Upstream contact: Ville Helin <ville.helin@iki.fi>
URL             : https://github.com/vhelin/wla-dx
Licence         : GPL-2+
Programming lang: C
Description     : multi-platform cross-assembler package (16 CPU architectures)
```

WLA DX is a set of tools for assembling source code to object files
(`wla-ARCH`) and linking them together (`wlalink`). Each `wla-ARCH`
binary targets one specific CPU: 6502/65C02/65CE02/65816 (MOS/WDC 6502
family — NES, SNES), SPC700 (SNES audio CPU), SuperFX (Nintendo SuperFX
coprocessor), 6800/6801/6809 (Motorola 6800 family), 68000 (Motorola
68000), Z80/Z80N/GB (Zilog Z80 family / Game Boy GB-Z80), 8008/8080
(Intel), and HuC6280 (PC Engine). WLA DX isn't tied to a specific
system: it lets you define your own memory map (shared across object and
library files), making it suitable for cross-platform retro development
across multiple console families. Also included: `wlalink` (linker) and
`wlab` (binary-to-WLA-DB converter).

This is the single broadest cross-assembler in this whole ITP batch —
16 CPU targets in one package — and Debian currently has narrower,
single-architecture cross-assemblers (`cc65` for 6502, `xa65` for 6502,
`dasm` for a smaller target set) but nothing covering this breadth. It's
also a real dependency of `pvsneslib` (also in this batch — see that
draft's note on a possible `Build-Depends`/pinned-copy relationship),
so packaging wla-dx first would let pvsneslib depend on the real Debian
package rather than bundling its own pinned copy.

## Notes

- Single license (GPL-2+), DFSG-clean.
- Simple build (`debhelper-compat (= 13)` only, per `Build-Depends`), no
  unusual dependencies.
- Recommend filing this one early relative to `pvsneslib` and
  `asar-snes-assembler` given the "complete SNES homebrew toolchain"
  framing noted in the pvsneslib draft.
