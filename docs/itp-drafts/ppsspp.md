# ITP draft: ppsspp

## Upstream packaging audit

Audited the pinned upstream payload on 2026-08-05 with
`foundry-apt/scripts/audit-upstream-packaging.sh`. **No upstream packaging signals found:** no
upstream-maintained `debian/`, `.deb`/`.dsc` builder, `PKGBUILD`/`.spec`, or PPA/Launchpad/OBS/COPR
packaging reference.

## wnpp check — EXISTING ITP FOUND, do not file a new one

**[Bug #697821](https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=697821) — "ITP: ppsspp -- portable PSP emulator"**, filed by John Paul Adrian Glaubitz on 2013-01-10. Search results show it is tracked as blocked by a later **RFS** ([#801262](https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=801262) — "RFS: ppsspp/1.2.2+dfsg1-1 [ITP]"), meaning packaging work was actually done (a `+dfsg1` repack, implying non-free content was stripped from a real Debian packaging attempt) but apparently never made it through sponsorship/upload. The current open/closed state as of 2026-08 wasn't confirmed by search alone (the bug tracker itself was unreachable from this environment) — **whoever picks this up must check [bugs.debian.org/697821](https://bugs.debian.org/697821) directly before doing anything else.**

**Do not file a new ITP.** If the existing bug is still open, the correct move is to comment on it (offering to pick up packaging, referencing the `+dfsg1` precedent for what needed stripping) rather than duplicate it. If it's closed/abandoned, retitling and reopening (or filing a fresh ITP that explicitly references #697821 and #801262 as prior art) is more appropriate than a from-scratch bug with no context.

## ITP draft (for reference / for the eventual bug comment)

```
Package name    : ppsspp
Version         : 1.20.4
Upstream contact: Henrik Rydgård <hrydgard@gmail.com>
URL             : https://www.ppsspp.org
Licence         : GPL-2+ (core), BSD-3-clause (bundled components)
Programming lang: C++
Description     : fast and portable PSP emulator
```

PPSSPP is a PSP emulator written in C++ that translates PSP CPU
instructions into optimized x86, x64, ARM, or ARM64 machine code via JIT
recompilation. It runs many PSP games at full HD resolution (or higher)
with enhancements including anisotropic filtering, texture scaling,
increased draw distance, and post-processing shaders. Vulkan, OpenGL,
and software rendering backends are supported, along with save states,
rewind, and netplay. PSP ROMs/ISOs are not included — users must supply
their own legally obtained game files.

## Notes on why the 2013-2015 attempt likely stalled

The `+dfsg1` version suffix on the RFS (#801262) strongly suggests the
original packaging effort had to strip non-free content before it could
be uploaded — almost certainly PPSSPP's bundled fonts/assets or its
embedded `ffmpeg`/`libav` copy (a well-known PPSSPP packaging pain point:
it historically bundled a specific ffmpeg version for format
compatibility, which conflicts with Debian's strong preference for
system `ffmpeg`). Anyone picking this ITP back up should expect the same
issues: managing embedded libraries (ffmpeg/libav especially), handling
bundled atlas fonts for multi-language UI text, and reconciling PPSSPP's
separate SDL and Qt frontend code paths into one Debian-appropriate
build. This is real, non-trivial packaging work, not a T1-shippable ITP
filing on its own — but the wnpp legwork (finding and reading the prior
attempt) is exactly what this draft was for.

## Licensing note

`allLicenses` in our own `debian/copyright` lists GPL-2+ and
BSD-3-clause only, which reads as clean, but given the `+dfsg1` history
above, whoever re-attempts this should re-audit PPSSPP's current bundled
assets from scratch rather than trusting that our `debian/copyright` (a
Foundry packaging artifact, not a Debian-grade audit) is exhaustive.
