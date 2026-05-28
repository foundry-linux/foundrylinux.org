---
title: Install section redesign — 5 ordered paths, least → most invasive
date: 2026-05-28
status: in-progress
---

# Install section redesign

## Problem

The current Install section has four channel cards in an arbitrary order (Existing Ubuntu → Container → ISO → VM), then a code block and two separate download tables below that have no clear connection to the cards above. There is no dual-boot path. The progression from "least invasive" to "full replacement" is invisible to the reader.

Primary audience: **Windows game developers** who've never run Linux. The Install section is their decision tree.

## Changes

### 1. `site/sections.jsx`

**CHANNELS array** — reorder to 5 paths, strictly least → most invasive:

| # | Label | Hook | `href` | Invasiveness tag |
|---|---|---|---|---|
| 01 | Existing Ubuntu | Already on 26.04? Add the repo — no reinstall. | `#path-apt` | `non-destructive` |
| 02 | Container | Any host — Linux, macOS, or Windows via WSL. | foundry-devbox repo | `isolated` |
| 03 | Virtual machine | Sandboxed — VirtualBox, VMware, or QEMU. Your OS unchanged. | `#path-vm` | `isolated` |
| 04 | Dual boot | Keep Windows. Boot from USB; Calamares offers "Install alongside" automatically. | `#path-iso` | `additive` |
| 05 | Fresh install | Replace everything. Full branded desktop from first boot. | `#path-iso` | `replaces-os` |

Each card gets a `data-invasiveness` attribute (value = the tag above) so CSS can colour the indicator.

**Section header copy** — change from "Add the forge to your sources." to "Pick your path." with blurb:

> Five ways in — ordered from least to most disruptive. A Windows developer new to Linux should start at Container or VM.

**Detail blocks** — the area below the cards becomes three anchored sections rather than an undifferentiated dump:

```
#path-apt   ← Shell codeblock (existing apt steps, unchanged)
#path-vm    ← VM download table (OVA / VMDK / qcow2)
#path-iso   ← ISO download table (Anvil + Atelier, torrent + magnet)
             + Dual boot callout strip
```

The Container card links out to the devbox repo (existing behaviour) — no detail block needed.

**Dual boot callout** — a strip inside `#path-iso`, between the ISO table and the VM table, styled as `.dualboot-callout`:

> **Dual boot with Windows:** Boot the ISO, choose *Install alongside Windows Boot Manager* in Calamares, and it shrinks your Windows partition automatically. No manual partitioning. Recommended: give Foundry Linux at least 60 GB. [→ Ubuntu dual-boot guide](https://ubuntu.com/tutorials/install-ubuntu-desktop#1-overview)

This is a static HTML strip — no scripting, no separate docs page needed for v1.

**Invasiveness indicator** — each channel card gets a `<span className="channel-inv">` tag in the bottom-right showing the invasiveness label. Styled dimly by default; the active card accent colour in hover state.

### 2. `site/styles.css`

- `.channels-grid`: `repeat(4, 1fr)` → `repeat(5, 1fr)`. Breakpoints: ≤1100 px → `repeat(3,1fr)`, ≤700 px → `repeat(2,1fr)`, ≤420 px → `1fr`.
- `.channel-card[data-invasiveness="replaces-os"]`: subtle warm tint on the bottom border (accent colour) to signal "highest commitment".
- `.channel-inv`: `font-mono`, 9 px, `ink-faint`, `letter-spacing 0.15em`, uppercase, bottom of card.
- `.dualboot-callout`: bordered strip, `hairline-strong`, small padding, `ink-soft` text, accent-coloured "→" link.
- `.path-anchor`: `<div id="path-*">` heading label above each detail block — `font-mono 10px`, `ink-faint`, `letter-spacing 0.2em`, uppercase, with a thin left-border in `hairline-strong`.

### 3. No data file changes

No `packages-data.json` or `data/` changes. All copy is hand-written in `sections.jsx`.

## Critical files

- `site/sections.jsx` — CHANNELS, Install(), detail blocks
- `site/styles.css` — grid, channel-card, new classes

## Verification

1. `task site-build` exits 0; both `site/index.html` and `site/packages.html` non-empty.
2. `grep -c 'data-invasiveness' site/index.html` → 5.
3. `grep 'dualboot-callout' site/index.html` → present.
4. `grep 'path-apt\|path-vm\|path-iso' site/index.html | wc -l` → ≥ 6 (card hrefs + anchor divs).
5. Open `localhost:8080`, verify: cards in order 01–05, invasiveness labels visible, dual-boot callout appears between ISO and VM tables, section header says "Pick your path."
