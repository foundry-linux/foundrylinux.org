# WLA-DX Project Health Investigation

**Date**: 2026-06-11  
**Repo**: [github.com/vhelin/wla-dx](https://github.com/vhelin/wla-dx)  
**Context**: We ship wla-dx 10.6 in foundry-apt. Checking whether to track HEAD more
closely, when v10.7 will land, and who to contact.

---

## TL;DR

**v10.7 is imminent.** The CHANGELOG already contains a dated v10.7 entry
(25-May-2026); the tag just hasn't been pushed yet. Two small bug-fix commits landed
after the CHANGELOG entry (June 1). Ville is clearly in "final polish" mode —
expect a tag within days to a few weeks.

**Project is healthy and solo.** Ville Helin (Kivistö, Vantaa, Finland) has been the
sole committer for 12 years. Activity surged dramatically in early 2026 after a quiet
2024–2025. The open issue queue is large but mostly old feature requests; Ville clears
bugs quickly when they affect correctness.

**Action**: bump our package to v10.7 as soon as the tag appears. The three new
CPUs (eZ80, Cx4, SH-2) and 50+ bug fixes make it a meaningful upgrade.

---

## Maintainer

| Field | Value |
|---|---|
| Name | Ville Helin |
| GitHub | [@vhelin](https://github.com/vhelin) |
| Email | ville.helin@gmail.com |
| Twitter/X | [@HelinVille](https://twitter.com/HelinVille) |
| Website | [villehelin.com](http://www.villehelin.com) |
| Location | Kivistö, Vantaa, Finland |

Solo project — no org, no co-maintainers, no issue labels or milestones. All commits
are from Ville directly. He is accessible and responsive; most issues get a reply and
many get a fix.

---

## v10.7 — Status and What's In It

**CHANGELOG already dated 25-May-2026. No tag yet (as of 2026-06-11).**

Two commits landed after the CHANGELOG entry:

| SHA | Date | Message |
|---|---|---|
| `a5c10670` | 2026-06-01 | Fixed Amiga SAS/C related compiler errors and warnings |
| `8e55b088` | 2026-06-01 | [658] .L could fool instructions decoder for some instructions that don't support 24-bit values. Fixes #718 |

The Amiga fix and the Z80 decoder fix both look like the kind of last-minute polish
that precedes a tag. HEAD is 191 commits ahead of v10.6.

**New CPUs in v10.7** (our biggest packaging change — 16 → 19 assemblers):

| Binary | CPU | Notes |
|---|---|---|
| `wla-ez80` | Zilog eZ80 | ZX Spectrum Next; with `.ADL` directive |
| `wla-cx4` | Capcom Cx4 | SNES custom chip (Mega Man X2/X3) |
| `wla-sh2` | Hitachi SH-2 | Sega 32X; ships with 32X memory maps |

**Platform additions** (assembler directives / linker formats):

- Sega Mega Drive/Genesis + Sega CD (68k): `.MDVECTORS`, `.MCDHEADER`, memory maps
- Neogeo (68k): `.NGHEADER`, `.NGSOFTDIP`, `.NGVECTORS`, include files
- NES: `.INESHEADER` / `.ENDINES`
- Atari Lynx: `.LYNXHEADER` / `.ENDLYNX`
- Commodore 64 cartridge types (wlalink)

**Notable bug fixes**:

- 65816: BRL and PER fixed; accumulator width warning when `.ACCU`/`.INDEX` overrides rep/sep
- 68000: BCLR/BSET/BTST bit ranges corrected (0–7 / 0–31, not 1–8 / 1–32); removed
  silent MOVE→MOVEQ / ADD→ADDQ / SUB→SUBQ "optimizations" that changed semantics
- 6502: STX and STY parsing fixed
- 8008: LMI opcode corrected ($3E, was $2E)
- All CPUs: `.MACRO` call-stack printed on error; `.ASSERT` can be postponed to link
  time; ANSI C compliance pass; full-instruction-set tests added

---

## Release Cadence

| Release | Date | Gap |
|---|---|---|
| v10.1 | 2022-02-19 | — |
| v10.2 | 2022-06-24 | 4 mo |
| v10.3 | 2022-10-02 | 3 mo |
| v10.4 | 2022-11-24 | 2 mo |
| v10.5 | 2023-06-23 | 7 mo |
| v10.6 | 2023-11-19 | 5 mo |
| **v10.7** | **pending** | **~31 mo** ← unusually long gap |

The 31-month gap is real but the project wasn't dormant — Ville was just accumulating
a large batch. He resumed heavy activity in early 2026 (a burst of 191 commits from
~February through June). The CHANGELOG entry is written; the tag is late.

---

## Open Issues (selected)

40 issues open. Most are old feature requests or documentation asks. Correctness bugs
get addressed quickly.

**Active / recent:**

| # | Title | Date | Status |
|---|---|---|---|
| [#704](https://github.com/vhelin/wla-dx/issues/704) | 65816: Accumulator width tracking ignores control flow | 2026-03-21 | Open — no reply yet; related warning was added but the root fix wasn't |
| [#691](https://github.com/vhelin/wla-dx/issues/691) | Document differences between CA65 / ASM6 and WLA syntax | 2026-01-16 | Open — documentation ask |
| [#674](https://github.com/vhelin/wla-dx/issues/674) | 65816/SNES LOROM: data outside sections ignores slot start addr | 2025-09-24 | Open — one response, not resolved |
| [#673](https://github.com/vhelin/wla-dx/issues/673) | `INSTANCEOF` in `.RAMSECTION` with unnumbered instance | 2025-09-22 | Open |

**Ville's own open issues** (self-filed roadmap items, not bugs):

| # | Title |
|---|---|
| [#646](https://github.com/vhelin/wla-dx/issues/646) | Listfiles are wonky inside .MACRO |
| [#589](https://github.com/vhelin/wla-dx/issues/589) | Add Amiga executable binary support |
| [#392](https://github.com/vhelin/wla-dx/issues/392) | Add DSP-1 support |
| [#391](https://github.com/vhelin/wla-dx/issues/391) | Add CP1610 support |
| [#376](https://github.com/vhelin/wla-dx/issues/376) | Add more tests |
| [#485](https://github.com/vhelin/wla-dx/issues/485) | Add verbose-output tests |

These are essentially a personal roadmap — not blockers.

**Long-tail open issues** (2014–2020, largely unanswered):

Issues #23, #25, #53, #56, #100, #106, #107 date back to 2014–2016 and discuss
fundamental design questions (refactoring, docs reorganization, multi-bank allocation).
They pre-date the cmake era and are unlikely to be addressed as stated, but the
underlying problems have been worked around incrementally.

---

## Open PRs

| # | Title | Author | Date | Assessment |
|---|---|---|---|---|
| [#699](https://github.com/vhelin/wla-dx/pull/699) | Fixes to Makefile generation (#657) | vhelin | 2026-02-07 | Ville's own PR, unmerged — probably lands pre-v10.7 |
| [#567](https://github.com/vhelin/wla-dx/pull/567) | Actually remove Makefile from VCS | jeffythedragonslayer | 2023-02-24 | Still relevant — Makefile is still in VCS |
| [#216](https://github.com/vhelin/wla-dx/pull/216) | [WIP] Legacy Makefiles | Neui | 2019-02-17 | Stale |
| [#199](https://github.com/vhelin/wla-dx/pull/199) | [WIP] Tests via CTest/CMake | Neui | 2019-02-17 | Stale — tests were added via a different approach |

No external contributions are pending on correctness or CPU support. The two active
items (#699 and #567) are both about the legacy Makefile, not the assembler itself.

---

## Project Health Summary

| Dimension | Assessment |
|---|---|
| **Maintainer activity** | ✅ Active — 191 commits in 2026 alone |
| **Bug fix responsiveness** | ✅ Good — correctness bugs get same-day fixes |
| **Release cadence** | ⚠️ Irregular — v10.7 is 31 months overdue but clearly imminent |
| **Community** | ✅ Moderate — 597 stars, 105 forks, active issue reporters |
| **Bus factor** | ⚠️ 1 — Ville is the sole maintainer; no succession plan visible |
| **Test coverage** | ✅ Improving — all-instruction-set tests added per-CPU in v10.7 |
| **CI** | ✅ Azure Pipelines (Linux, Windows, Windows XP 32-bit, ASAN/UBSAN) |
| **Documentation** | ⚠️ Sparse — RST docs exist but aren't published; no hosted homepage |
| **License** | ⚠️ Nonstandard — GitHub reports `NOASSERTION`; upstream uses a custom permissive notice |

---

## Packaging Implications for Foundry Linux

1. **Wait for v10.7 tag** — CHANGELOG is written, the tag could drop any day. The
   upgrade adds 3 new CPUs and fixes several 65816/68k correctness bugs that matter
   for real projects (PVSnesLib, Mega Drive homebrew).

2. **v10.7 packaging changes** (see [packaging plan](../plans/2026-06-11-package-wla-dx.md#when-v107-is-tagged)):
   - Add `debian/man/wla-ez80.1`, `wla-cx4.1`, `wla-sh2.1`
   - Update `debian/control`: 16 → 19 CPU architectures
   - Update `debian/wla-dx.manpages`
   - New `debian/changelog` entry

3. **Contacting Ville** — He's reachable at ville.helin@gmail.com or
   [@HelinVille](https://twitter.com/HelinVille). If we want to nudge him to tag
   v10.7, a polite GitHub issue or email noting "the CHANGELOG is written, are you
   planning to tag soon?" would be appropriate. He has been responsive in the past.
   We could also mention we're packaging it for a Linux distro — that kind of
   downstream signal sometimes accelerates a tag.

4. **License note** — GitHub's SPDX detector says `NOASSERTION`. The actual upstream
   license is a short custom permissive text in the source files. This is already
   handled correctly in our `debian/copyright`.
