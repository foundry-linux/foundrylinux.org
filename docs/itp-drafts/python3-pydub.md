# ITP draft: python3-pydub

## wnpp check — PACKAGE ALREADY IN DEBIAN, do not file

**This is not an ITP-worthy situation at all — `python3-pydub` (as
`python-pydub` upstream, packaged under the name `python3-pydub`)
already ships in Debian.**

- [Debian Package Tracker: python-pydub](https://tracker.debian.org/pkg/python-pydub) confirms the source package exists and is maintained.
- Search shows the package was **accepted into Debian unstable back on 2018-08-03** at version 0.22.1-1 ([tracker.debian.org accepted-into-unstable notice](https://tracker.debian.org/news/977501/accepted-python-pydub-0221-1-source-all-into-unstable/)), maintained by Josue Ortega.
- Current status (per `packages.debian.org` search): **0.25.1-2 in both testing and unstable (sid)** — the same upstream version (0.25.1) our own Foundry package tracks, just one Debian revision ahead of ours (`-2` vs. our `-1foundry1`).
- There is an open bug about a build issue — [Bug #1082254: python-pydub FTBFS with Python 3.13](https://www.mail-archive.com/debian-bugs-dist@lists.debian.org/msg1992252.html) — but that's a maintenance bug against the *existing* package, not a wnpp/ITP matter.

**Verdict: no ITP action of any kind.** The package name we'd file under
(`python3-pydub`) is *already the actual Debian package name* for this
*exact* upstream project. Filing an ITP would be filing against a
package that already exists — nonsensical, and if attempted would be
closed immediately by wnpp maintainers pointing at the existing
package.

## What this means for Foundry's own package

Our `foundry-apt/packages/python3-pydub/` exists because `apt.foundrylinux.org`
needs pydub on Ubuntu 26.04 before/independent of whatever's in Debian's
archive — that's a legitimate reason to keep vendoring it ourselves for
our own repo. But it should **never be conflated with a Debian ITP
candidate**, and this TODO item should be marked done-with-no-filing
rather than "escalate" or "blocked" — there's nothing to escalate, the
research conclusively answers the question.

## Reference (not an ITP — for completeness only)

```
Package name    : python3-pydub  [ALREADY EXISTS IN DEBIAN — see above]
Version         : 0.25.1 (Debian: 0.25.1-2; ours: 0.25.1-1foundry1)
Upstream contact: James Robert <jiaaro@gmail.com>
URL             : http://pydub.com
Licence         : MIT
Programming lang: Python 3
Description     : high-level audio manipulation library for Python
```
