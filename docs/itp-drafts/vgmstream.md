# ITP draft: vgmstream

## Upstream packaging audit

Audited the pinned upstream payload on 2026-08-05 with
`foundry-apt/scripts/audit-upstream-packaging.sh`. **No upstream packaging signals found:** no
upstream-maintained `debian/`, `.deb`/`.dsc` builder, `PKGBUILD`/`.spec`, or PPA/Launchpad/OBS/COPR
packaging reference.

## wnpp check

Searched wnpp for "vgmstream" (video-game audio decoder). **No existing wnpp bug found** — search returned unrelated audio-tool ITPs (ffmpeg, decibels, libaribcaption, audiotools, audio-recorder), nothing for vgmstream itself.

## ITP draft

```
Package name    : vgmstream
Version         : 2083 (upstream uses a bare revision-number scheme, e.g. "r2083")
Upstream contact: https://github.com/vgmstream/vgmstream/issues
URL             : https://github.com/vgmstream/vgmstream
Licence         : ISC-vgmstream
Programming lang: C
Description     : Video-game audio decoder (CLI)
```

vgmstream is a library and command-line decoder for streamed
(prerecorded) video-game audio in over 100 formats — from typical
game-engine container files (`.adx`, `.aix`, `.ahx`, `.at3`, `.brstm`,
`.bnsf`, `.gcm`, `.genh`, `.hca`, `.vag`, …) to obscure single-game
codecs. Output is PCM WAV (or stdout) suitable for mastering and
analysis pipelines. Our package builds `vgmstream-cli` with the ffmpeg,
mpg123, vorbis, opus, speex, and G.722.1 (Siren 14) backends; ATRAC9,
FSB CELT, and G.719 are excluded since their decoders are Windows-only
DLLs that don't compile on Linux; plugins (Audacious, Winamp, foobar2000,
XMPlay) and the `vgmstream123` libao player are intentionally not built.

Widely used in game audio preservation, modding, and ripping communities
— a well-known, actively maintained tool with no comparable equivalent
in Debian today. Pairs naturally with `libvgm` (also queued in this
batch) for a complete VGM/streamed-audio toolchain.

## Notes

- Custom licence name `ISC-vgmstream` in `debian/copyright` suggests a
  slight variant of the standard ISC text, not a verbatim match — worth
  double-checking against Debian's `common-licenses` ISC text or
  confirming it needs to stay as a custom short-name entry in the actual
  Debian `debian/copyright` (this is a routine DEP-5 detail, not a
  licensing risk, since ISC-family licences are uniformly DFSG-free).
- Excluding the Windows-only-DLL codecs (ATRAC9, FSB CELT, G.719) is the
  right call for Debian too — those can't be built from source on Linux
  regardless, so there's no "did we skip something fixable" concern
  there.
- Build pulls in `ffmpeg`/`libavcodec` as a backend — same general
  category of dependency that complicated the `ppsspp` ITP historically
  (see that draft) — worth confirming vgmstream's ffmpeg usage is a
  clean `Build-Depends`/`Depends` against Debian's system ffmpeg, not a
  bundled copy, before filing (our own `build.sh` excerpt doesn't show
  bundling, which is a good sign, but wasn't exhaustively checked here).
