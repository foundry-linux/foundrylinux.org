# ITP draft: drmon

## Upstream packaging audit

Audited the pinned upstream payload on 2026-08-05 with
`foundry-apt/scripts/audit-upstream-packaging.sh`. **No upstream packaging signals found:** no
upstream-maintained `debian/`, `.deb`/`.dsc` builder, `PKGBUILD`/`.spec`, or PPA/Launchpad/OBS/COPR
packaging reference.

## wnpp check

Searched wnpp for "drmon" (debugger) and "drdevtools". **No existing wnpp bug found.** This is an obscure upstream (`developer-resources-co/drdevtools`, a 1991-1994 DOS product ported to Linux) with essentially no public packaging footprint — no surprise there's no existing bug.

## ITP draft

```
Package name    : drmon
Version         : 1.0.1
Upstream contact: packages@foundrylinux.org (Foundry Linux port maintainer;
                  original DOS product by Developer Resources, 1991-1994)
URL             : https://github.com/developer-resources-co/drdevtools
Licence         : GPL-2
Programming lang: C++ (ncurses)
Description     : SNES/Genesis source-level debugger for game development
```

drmon is the Developer Resources Monitor, a source-level debugger and
memory monitor for cartridge-console game development, targeting the SNES
(Nintendo 65816) and Sega Genesis (Motorola 68000). Originally a DOS
product from Developer Resources (1991-1994), it has been ported to
Linux as a wide-ncurses terminal application. When connected to MAME via
a custom TCP bridge (SNES) or MAME's GDB RSP (Genesis), it provides live
register read/write, memory inspection, breakpoints, single-step,
continue, halt, and soft reset; it also runs disconnected for
expression/symbol/macro work with target reads returning zero. Ships two
binaries, `snesmon` and `genmon`, sharing one UI and command language.

This fills a real gap: Debian's retro-development tooling (`mame`,
`cc65`, `dasm`, `wla-dx`, etc.) has assemblers and emulators but no
source-level debugger purpose-built for 65816/68000 cartridge-console
development. Its niche is narrow but the tool is functionally unique —
worth weighing against the risk that a single-maintainer, low-traffic
upstream (no Debian-relevant activity found anywhere) may be hard to
get sponsored/reviewed.

## Notes

- **Provenance flag for the ITP submitter**: `debian/copyright`
  attributes the DOS-era code to "Developer Resources" (1991-1994) and
  the Linux port to "Will Norris <wbnorris@gmail.com>" (2026) — i.e. this
  package's own maintainer did the port. Debian ITP process expects the
  filer to be either upstream or a genuine third-party packager; here
  they're effectively the same, which is fine but should be stated
  explicitly in the actual bug (`Upstream Author` field) rather than
  left implicit.
- Single license (GPL-2, not "or later" — note the exact version pin),
  DFSG-clean.
- Build-Depends: `cmake`, `ninja-build`, `libncurses-dev`, `libcppdap-dev`,
  `libjsoncpp-dev` — `libcppdap-dev` is less common in Debian main; worth
  confirming it's packaged there before committing to this ITP.
