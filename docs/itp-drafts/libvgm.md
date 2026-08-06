# ITP draft: libvgm

## Upstream packaging audit

Audited the pinned upstream payload on 2026-08-05 with
`foundry-apt/scripts/audit-upstream-packaging.sh`. **No upstream packaging signals found:** no
upstream-maintained `debian/`, `.deb`/`.dsc` builder, `PKGBUILD`/`.spec`, or PPA/Launchpad/OBS/COPR
packaging reference.

## wnpp check

Searched wnpp for "libvgm" (chiptune VGM library, ValleyBell). **No existing wnpp bug found** — search turned up unrelated ITPs (furnace, uxn) but nothing for libvgm itself.

## ITP draft

```
Package name    : libvgm
Version         : 0.1+git20260406.d115188 (no tagged upstream release;
                  packaged from a pinned git commit)
Upstream contact: ValleyBell <https://github.com/ValleyBell/libvgm/issues>
URL             : https://github.com/ValleyBell/libvgm
Licence         : GPL-2.0+ (core), plus per-file: LGPL-2.1+, BSD-3-Clause,
                  Expat, GPL-2.0 for individual bundled components — see
                  debian/copyright for the full per-file breakdown
Programming lang: C
Description     : chip-register VGM playback library with player utilities
```

libvgm is a modular rewrite of the audio components from VGMPlay. It
exposes sound emulation cores (vgm-emu), audio output drivers
(vgm-audio), a generic chiptune player (vgm-player), and helper
utilities (vgm-utils) as shared libraries usable by other applications.
It supports VGM (Video Game Music) and S98/DRO formats by emulating
original sound chips (YM2151, YM2203, YM2608, YM2612, YM3526, YM3812,
AY-3-8910, SN76489, NES APU, SegaPCM, RF5C68, and many others), with
ALSA/PulseAudio/libao output. Two reference applications are included:
`vgm-player` (interactive playback) and `vgm2wav` (offline WAV
rendering). Headers and a CMake config package are installed for
downstream consumers such as `vgmstream`.

Debian has no VGM chiptune playback library today. Given that our own
`vgmstream` package (also queued in this ITP batch) is a downstream
consumer of libvgm-style chip emulation, packaging libvgm first would
put the dependency order the right way round for eventual upstream
Debian coordination.

## Notes

- **No tagged upstream release** — libvgm packages from a pinned git
  commit (see `foundry-apt/packages/libvgm/build.sh`). Debian ITP
  process handles this fine (many packages track git snapshots), but
  it does mean the "Version" field above is a snapshot version, not a
  stable upstream release number — worth being upfront about that in
  the actual bug.
- Multiple bundled-component licenses (GPL-2.0+, LGPL-2.1+, BSD-3-Clause,
  Expat, GPL-2.0) — all individually DFSG-free, but the ITP filer should
  double check `debian/copyright`'s per-file `Files:` stanzas map
  cleanly to upstream's actual per-directory licensing before filing,
  since git-snapshot packaging is more prone to drift than a tagged
  release.
