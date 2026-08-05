# ITP draft: python3-librosa

## wnpp check — EXISTING ACTIVE ITPs FOUND, do not file

Multiple wnpp bugs exist for librosa, including **two currently active
ITPs from 2026** — this is actively being worked on right now:

- [Bug #968467](https://bugs.debian.org/968467) — "ITP: librosa -- module for audio and music processing" (older; status not directly confirmed, likely superseded/stale given the two below).
- [Bug #1130020](https://bugs.debian.org/1130020) — "ITP: python-librosa -- a python package for music and audio analysis", version 0.11.0 (**matches our packaged version exactly**), intended for maintenance under the **Debian Python Team** at `salsa.debian.org/python-team/packages/python-librosa`.
- [Bug #1133705](https://bugs.debian.org/1133705) — "ITP: librosa -- Python package for music and audio analysis", owner Pieter Lenaerts, also noting librosa is an optional dependency for the `beets` music tagger — i.e. there's a concrete downstream Debian consumer motivating this.

There was also a prior, apparently-completed packaging attempt: [Bug #1014078](https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=1014078) — "RFS: librosa/0.9.2-1 [ITP]" — marked done, suggesting librosa may have been packaged and later removed/orphaned, which would explain why fresh ITPs keep appearing.

**Do not file a new ITP.** This package already has active, current-year
(2026) interest from at least two different people, one of them working
under the Debian Python Team with a Salsa repo already named. The right
action here is to **watch #1130020 and #1133705** and, once one lands in
Debian, simply depend on the real `python3-librosa` instead of
maintaining our own — or, if genuinely interested in helping, reach out
to the existing filers rather than duplicating their work.

## ITP draft (kept for reference only — not to be filed)

```
Package name    : python3-librosa
Version         : 0.11.0
Upstream contact: librosa development team
URL             : https://librosa.org
Licence         : ISC-librosa
Programming lang: Python 3
Description     : audio and music analysis library for Python
```

Librosa is a Python library for audio and music signal processing,
providing building blocks for music information retrieval (MIR) and
audio analysis: beat tracking and tempo estimation, onset detection, mel
spectrograms/STFT/constant-Q transforms, MFCC, pitch tracking and
chroma features, and time-frequency representations. Useful in game
development for rhythm-game beat maps, procedural music systems, BPM
detection from soundtracks, and audio visualizers.

## Notes

- Version match (0.11.0 in both our package and Bug #1130020) is a
  strong signal the Debian Python Team's effort is current and worth
  tracking directly rather than duplicating.
- If/when `python3-librosa` lands in Debian proper, Foundry's own
  vendored copy in `foundry-apt/packages/python3-librosa/` becomes a
  candidate for retirement in favor of the real Debian package — noting
  this for a future TODO item, not resolving it here.
