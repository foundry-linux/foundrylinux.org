# Debian ITP drafts — summary

Prepare-only drafts for the 26 packages queued under `### Debian ITP` in
`TODO.md`. **Nothing has been filed with the Debian BTS.** Each
package's own `<name>.md` has the full draft, wnpp research, and
sourcing notes; this file is the roll-up table plus the headline
findings that change what happens next for several packages.

## Headline findings — read these before filing anything

1. **`python3-pydub` is already shipped in Debian** (0.25.1-2 in
   testing/unstable, same upstream version we package). No ITP needed
   or possible — see [python3-pydub.md](python3-pydub.md).
2. **`ppsspp` already has an open-or-stalled ITP** — [Bug #697821](https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=697821) from 2013, blocked on an [RFS](https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=801262) that never landed. Comment on the existing bug; do not duplicate. See [ppsspp.md](ppsspp.md).
3. **`python3-librosa` and `ruff` both have active 2026-era ITPs** under
   the Debian Python Team ([#1130020](https://bugs.debian.org/1130020)/[#1133705](https://bugs.debian.org/1133705) for librosa, [#1088706](https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=1088706) for ruff). Do not file — see [python3-librosa.md](python3-librosa.md) and [ruff.md](ruff.md).
4. **`ghidra` has two prior ITP→RFP demotions** ([#923851](https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=923851), [#973309](https://bugs.debian.org/973309)) — both abandoned, almost certainly because Ghidra ships pre-built binaries (~860 MiB of jars, a pre-compiled native decompiler) that Debian main can't accept as-is. See [ghidra.md](ghidra.md).
5. **`python3-glfw` risks functional duplication** with Debian's
   existing `python3-pyglfw` (a *different* upstream, same purpose,
   already in bookworm/trixie/sid). See [python3-glfw.md](python3-glfw.md).
6. **`snes9x-gtk` verdict: do not file.** Snes9x's licence is explicitly
   "for non-commercial purposes... PERSONAL USE only" — a clean DFSG #6
   violation, confirmed against the current upstream `LICENSE` file, not
   just our packaging. See [snes9x-gtk.md](snes9x-gtk.md) for the full
   analysis (this is why it was ranked `[T3]`, not `[T1]`, in `TODO.md`).
7. **Four packages ship pre-built binaries, not from-source builds** —
   `ghidra`, `ldtk` (AppImage repack), `mesen2` (.NET AOT single-file
   binary), and `pvsneslib` (pre-built toolchain). Each needs a real
   from-source packaging effort before a Debian ITP is fileable, not
   just a bug report. `m8te` may have the same issue (bundled pre-built
   CIL assembly) — flagged for verification, not confirmed.
8. **`blender-asset-finder` and `blender-asset-finder-cli` should
   probably be one ITP, not two** — they currently duplicate the same
   asset-provider library across two separate Debian source packages.
   See both drafts for the recommendation to combine before filing.
9. **Two unrelated upstreams are both called "xemu"** — the original-Xbox
   emulator (`xemu-project/xemu`, packaged here as `xemu`) and LGB's
   Commodore/Hungarian 8-bit suite (`lgblgblgb/xemu`). Neither name is
   taken in Debian yet (no `xemu` source or binary package as of
   2026-08-05, and no ITP for the Xbox one), so this is not a forced
   rename: we deliberately allocate `xemu` to the Xbox emulator (its own
   name, the better-known project) and `x-emulators` to the suite
   (upstream's own full project name), so each package is named after
   itself rather than after the conflict. Same spirit as
   `asar` → `asar-snes-assembler`, but proposed before either name is
   claimed. Final call is ftp-master's. Builds cleanly from source (no
   prebuilt-binary issue like `ghidra`/`ldtk`/`mesen2`/`pvsneslib`).
   See [x-emulators.md](x-emulators.md).

## Full table

| Package | Licence (from `debian/copyright`) | wnpp status | Filing verdict |
|---|---|---|---|
| [asar-snes-assembler](asar-snes-assembler.md) | LGPL-3+ / WTFPL-2+ | none found | ready to file |
| [blender-asset-finder](blender-asset-finder.md) | GPL-2.0-or-later | none found | combine with -cli first (see note) |
| [blender-asset-finder-cli](blender-asset-finder-cli.md) | GPL-2.0-or-later | none found | combine with base pkg first (see note) |
| [bsnes-jg](bsnes-jg.md) | GPL-3.0-or-later (+ bundled Zlib/Expat/ISC/BSD-2-Clause/LGPL-2.1+) | none found | ready to file |
| [drmon](drmon.md) | GPL-2 | none found | ready to file (niche upstream) |
| [f9dasm](f9dasm.md) | GPL-2.0+ | none found | ready to file |
| [ghidra](ghidra.md) | Apache-2.0 | **2 prior ITP→RFP, both abandoned** | needs from-source build effort first |
| [halfempty](halfempty.md) | Apache-2.0 | none found | ready to file |
| [ldtk](ldtk.md) | Expat (MIT) | none found | needs from-source build effort first (AppImage repack today) |
| [libvgm](libvgm.md) | GPL-2.0+ (+ bundled LGPL-2.1+/BSD-3-Clause/Expat/GPL-2.0) | none found | ready to file (git-snapshot version) |
| [m8te](m8te.md) | Expat (MIT) | none found | verify from-source status before filing |
| [mesen2](mesen2.md) | GPL-3 | none found | needs from-source build effort first (.NET AOT binary today) |
| [ppsspp](ppsspp.md) | GPL-2+ (+ BSD-3-clause) | **existing ITP #697821, stalled since ~2015** | comment on existing bug, do not duplicate |
| [pvsneslib](pvsneslib.md) | Zlib | none found | needs from-source build effort first (pre-built toolchain today) |
| [python3-glfw](python3-glfw.md) | MIT | none for this upstream; **different project `python3-pyglfw` already in Debian** | resolve duplication question first |
| [python3-inators](python3-inators.md) | BSD-3-Clause | none found | ready to file (pairs with picire) |
| [python3-librosa](python3-librosa.md) | ISC-librosa | **2 active 2026 ITPs, Debian Python Team** | do not file |
| [python3-mss](python3-mss.md) | MIT | none found | ready to file |
| [python3-picire](python3-picire.md) | BSD-3-Clause | none found | ready to file (depends on inators) |
| [python3-pydub](python3-pydub.md) | MIT | **package already in Debian** (0.25.1-2) | no action — nothing to file |
| [ruff](ruff.md) | MIT | **active ITP #1088706, Debian Python Team** | do not file |
| [snes9x-gtk](snes9x-gtk.md) | Snes9x (non-commercial) | historical O/RFP only, none open | **do not file — non-DFSG** |
| [tilemap-studio](tilemap-studio.md) | LGPL-3 (+ FLTK, statically linked) | none found | ready to file (review static-link justification) |
| [vgmstream](vgmstream.md) | ISC-vgmstream | none found | ready to file |
| [wla-dx](wla-dx.md) | GPL-2+ | none found | ready to file (file early — pvsneslib depends on it) |
| [x-emulators](x-emulators.md) | GPL-2.0-or-later | none found | ready to file (name deliberately split from the Xbox `xemu`; neither taken in Debian — see note) |

## Suggested filing order, if/when filing is authorized

1. **Independent, ready-to-file, no blockers**: `wla-dx`,
   `asar-snes-assembler`, `bsnes-jg`, `f9dasm`, `halfempty`, `libvgm`,
   `vgmstream`, `python3-mss`, `python3-inators` → `python3-picire`,
   `tilemap-studio`, `x-emulators`.
2. **Resolve first, then file**: `python3-glfw` (duplication check),
   `blender-asset-finder`/`-cli` (combine-or-not decision), `drmon`,
   `m8te` (from-source verification).
3. **Do not file — already handled elsewhere in Debian**: `ppsspp`
   (comment on existing bug instead), `python3-librosa`, `ruff`,
   `python3-pydub` (nothing to do).
4. **Do not file — needs a real from-source packaging project first,
   not a bug report**: `ghidra`, `ldtk`, `mesen2`, `pvsneslib`.
5. **Do not file — licensing blocker**: `snes9x-gtk`.

## Method

For each package: read `debian/control` (Section, Homepage,
Description, Depends), `debian/copyright` (DEP-5 licence — authoritative
over any other source), `debian/changelog` (packaged version), and
`build.sh` where present (pinned upstream version/URL/build method).
wnpp status was checked via web search restricted to
`bugs.debian.org` for each package name plus relevant upstream/author
names, since direct HTTP access to `bugs.debian.org` was not reachable
from this environment (network sandboxing) and DuckDuckGo's HTML search
began rate-limiting/CAPTCHA-blocking after the first few requests — the
native web-search tool was used for the remainder. No `reportbug` was
run and no mail was sent; every draft above is a file on disk only.
