# snes9x-gtk — licence research (NOT a routine ITP)

This item was flagged `[T3]` in `TODO.md`, not `[T1]`, specifically
because the Snes9x licence is suspected non-commercial and likely
non-DFSG. This draft is the research that flag called for — **the
conclusion is that a routine ITP should not be filed.**

## wnpp check

Prior wnpp history for Snes9x-family packages exists, all historical and
about *removal*, not addition:

- [Bug #643595](http://bugs.debian.org/643595) — "RFP: snes9x -- Cross-platform SNES emulator" (a request, not an active ITP).
- [Bug #541004](https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=541004) — "O: snes9x -- X binaries for snes9x - Super NES Emulator" (an orphaning notice — someone gave up maintaining it).
- Historical note found via search: a Debian **removal** ticket (#617588) requested removal of `snes9x-gtk`, and it was in fact **removed from Debian after Oneiric** (Ubuntu 11.10-era) — i.e. Debian/Ubuntu carried a GTK Snes9x frontend once and then dropped it.
- Earlier GTK-frontend ITPs for *other* Snes9x frontends exist and were closed as stale (e.g. `goosnes`, closed after 450 days of inactivity).

No currently-open ITP exists for `snes9x-gtk` or `snes9x` today. That's
not really the point here, though — the point is whether one *should*
be filed at all.

## Licence text (verbatim, from our own `debian/copyright`)

```
License: Snes9x
 Permission to use, copy, modify and/or distribute Snes9x in both binary
 and source form, for non-commercial purposes, is hereby granted without
 fee, providing that this license information and copyright notice appear
 with all copies and any derived work.
 .
 This software is provided 'as-is', without any express or implied
 warranty. In no event shall the authors be held liable for any damages
 arising from the use of this software or it's derivatives.
 .
 Snes9x is freeware for PERSONAL USE only. Commercial users should
 seek permission of the copyright holders first. Commercial use includes,
 but is not limited to, charging money for Snes9x or software derived from
 Snes9x, including Snes9x or derivatives in commercial game bundles, and/or
 using Snes9x as a promotion for your commercial product.
 .
 The copyright holders request that bug fixes and improvements to the code
 should be forwarded to them so everyone can benefit from the modifications
 in future versions.
```

This is confirmed as the *current* upstream licence (checked directly
against `github.com/snes9xgit/snes9x/blob/master/LICENSE` via web
search) — this is not a stale/outdated copy in our packaging, it is
still what upstream ships today.

## DFSG analysis

The licence fails **DFSG #6 (No Discrimination Against Fields of
Endeavor)** unambiguously and explicitly:

- "for non-commercial purposes... is hereby granted"
- "freeware for PERSONAL USE only"
- "Commercial users should seek permission of the copyright holders first"

This is not a borderline or debatable case requiring interpretation —
the licence text itself uses the words "non-commercial," "personal use
only," and singles out "commercial users" for a separate permission
requirement. DFSG #6 exists specifically to reject exactly this pattern.
Debian's own packaging history confirms this isn't a novel reading: the
package was carried at various points as non-free-adjacent/frontend
packaging and was ultimately **removed**, and no one has successfully
re-added it since.

The three small bundled components under `jma/*` (GPL-2+), `filter/xbrz*`
(GPL-3+), and `filter/snes_ntsc*` (LGPL-2.1+) are all individually
DFSG-free, but they don't rescue the whole — the bulk of the codebase
(`Files: *`) is under the non-commercial Snes9x licence, and Debian
licensing review looks at the whole package's redistributability, not
just isolated files.

## Verdict: do not file a routine ITP

**Snes9x's core licence is non-DFSG-free**, and by extension so is
`snes9x-gtk` (a GTK frontend over the same non-commercial-licensed
core). This cannot go into Debian **main**. The remaining theoretical
options, in descending order of plausibility:

1. **Do nothing.** Given Debian previously carried and then removed a
   Snes9x-family package for reasons that likely include exactly this
   licensing concern, and given no one has re-attempted it in over a
   decade, this is the realistic outcome. Recommended.
2. **File as RFP, not ITP, targeting `non-free`.** Debian's `non-free`
   archive area does carry some non-commercial-licensed software, but
   Debian has been actively *shrinking* its non-free footprint for
   years (e.g. the `non-free-firmware` split), and ftp-master review of
   a *new* non-free addition — for a niche SNES emulator frontend, when
   Debian already tried and removed one — would face real headwinds.
   Not recommended without a much stronger case than exists today.
3. **Do not distribute Snes9x-based tooling in Debian at all**; treat it
   as a Foundry-Linux-only, `apt.foundrylinux.org`-only package (which
   is exactly its current status) and stop there.

## Recommendation for the TODO item

Mark this item resolved as **"researched — will not file"** rather than
leaving it open or escalating further. The licence question has a clear
answer; there's no additional judgment call left that a higher tier
would resolve differently. If the user wants to revisit non-free
distribution specifically, that's a separate, deliberate decision this
draft flags but does not make.
