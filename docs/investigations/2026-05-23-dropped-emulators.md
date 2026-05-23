# Dropped emulators: snes9x-gtk + ppsspp

**Date:** 2026-05-23  
**Question:** Why were snes9x-gtk and ppsspp dropped from Ubuntu 26.04? Should we package them in foundry-apt?

---

## snes9x-gtk

### Status

Not a 26.04 removal — **absent from Ubuntu for 15 years.** Last shipped in Ubuntu 11.10 (Oneiric, 2011).

### Root cause

Debian removed snes9x in March 2011 via bug #617588 (RoQA): *"orphaned, dead upstream, low popcon."* The Debian maintainer had dropped it; upstream was stalled at the time. Ubuntu stopped syncing when Debian removed it. The decision was correct for 2011 — but upstream has since fully revived.

### Upstream health

**Actively maintained.** Latest release: 1.63 (July 2024). Daily commits through May 2026 (Vulkan AMD anti-lag, save-slot fixes, input handling). Maintained by OV2, bearoso, itsoli. 3.2k GitHub stars, continuous CI. No ROM bundling, clean GPL-compatible license.

### Current Debian status

Not in any Debian suite. No active maintainer. No open RFP. `libretro-snes9x` is a separate libretro fork and not a substitute.

### Packaging effort

**Low–moderate (~3–4 h).** Standard GTK3/SDL C application with autotools. All deps (GTK3, libpng, zlib, SDL2) in Ubuntu universe. No non-free assets, no bundled libs to unbundle.

### Recommendation

**Package it.** The only reason it's gone is maintainer abandonment — upstream is healthy. Use `/package` skill against the 1.63 tarball from `https://github.com/snes9xgit/snes9x/releases`.

---

## ppsspp

### Status

**Never officially packaged** in Debian or Ubuntu. Not a removal — it was never there. Debian RFP #697821 filed January 2013, still open, never closed.

### Root cause

The 2013–2015 packaging attempt by John Paul Adrian Glaubitz stalled on:
1. **libpng 1.6 circular dependency** during Debian's libpng16 transition — build chain broke and was never fixed.
2. **Bundled ffmpeg** needed unbundling patches; work was started but not completed.
3. Maintainer dropped the effort; `glaubitz/ppsspp-debian` repo has 2 commits and no activity.

**Neither blocker exists today.** libpng16 has been in Debian stable for years; modern CMake builds handle system ffmpeg cleanly.

### Upstream health

**Actively maintained.** Latest release: v1.20.4 (May 16, 2026). 45k+ commits, weekly releases, active Discord. GPL 2.0+ core; xbrz filter is GPL 3 (compatible). No ROM bundling, no non-free assets.

### Current availability

Unofficial PPAs exist (Xu Zhen's PPSSPP PPA ships v1.20.4 for current Ubuntu) but are unsupported/untrusted. Nothing in any official archive.

### Packaging effort

**Moderate (~3–4 h).** CMake build — clean dh_cmake integration. Deps: SDL2, OpenGL, libavcodec/libavformat (system ffmpeg). The two 2015 blockers are non-issues now. `debian/copyright` needs to note the xbrz GPL-3 component.

### Recommendation

**Package it — higher priority than snes9x.** Upstream is more active (weekly releases vs annual for snes9x), the 13-year-old RFP makes clear Debian won't pick this up, and PSP emulation is a genuine gap in foundry-emulators. Use `/package` skill against the v1.20.4 tarball from `https://github.com/hrydgard/ppsspp/releases`.

---

## Summary table

| | snes9x-gtk | ppsspp |
|---|---|---|
| Last in Ubuntu | 11.10 (2011) | Never |
| Removal reason | Debian orphaned (maintainer dropout) | Never packaged (stalled 2013 ITP) |
| Blockers now | None | None (libpng16 ✓, system ffmpeg ✓) |
| Upstream | Active (1.63, July 2024) | Very active (v1.20.4, May 2026) |
| License | GPL-compatible | GPL 2.0+ (xbrz: GPL-3) |
| Non-free assets | None | None |
| Effort | ~3–4 h | ~3–4 h |
| Recommendation | **Package it** | **Package it (higher priority)** |

Both are good `/package` skill targets. Do ppsspp first — more users, more active upstream, bigger gap in the emulator catalogue.
