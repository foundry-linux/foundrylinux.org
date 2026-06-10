# Whatever happened to tUME? Can we port it — and is there a better successor?

**Date**: 2026-06-10
**Question**: tUME was an impressive tile-map editor in its day. What became of it, can Foundry
Linux port it (and give it a bigger/higher-resolution **editing window**, not bigger tiles), and is
there a more capable modern successor worth shipping in `foundry-retro-tools`?
**Short answer**: tUME still exists as freeware, but it's a DOS/Amiga app whose **editing canvas is
hard-wired to a vintage fixed display mode** (VGA 320×200/640×480 / Amiga modes). Giving it a large,
resizable, high-DPI editing window means rewriting its entire rendering/UI layer — at which point
you've rebuilt an editor that **already exists and resizes/zooms/pans for free**. **Don't port it;
ship the successors.** The de-facto modern tUME is **Tiled** (resizable & zoomable canvas, infinite
maps, scriptable, almost certainly already in 26.04 universe); for the specifically-retro crowd,
**Tilemap Studio** (GB/GBC/GBA/NES/SNES). Keep tUME itself as a documented historical footnote /
DOSBox curio.

---

## 1. What happened to tUME

tUME — **the Universal Map Editor** — was written by **Echidna**, the development house Gregg
Tavares ("greggman") and friends founded in **1990**. It started life on the **Amiga** for the
publisher Future Classics, was then **acquired by Virgin Games**, where **Dan Chang** ported it to
**MS-DOS**. Between **1991 and 1996** it was licensed to many studios and used in **11+ shipped
games** across SNES, Sega Genesis, and Amiga.

What made it notable for its era:

- **It copied the Deluxe Paint UI** — the most familiar art tool of the time, so artists were
  productive immediately.
- **It was genuinely universal.** It could author maps for "just about any 8-bit or 16-bit system":
  SNES, Genesis, Game Gear, Game Boy, TurboGrafx, Famicom/NES, Master System, IBM PC, and Amiga.
- **It decoupled the editor from the target format.** Where contemporaries saved proprietary,
  ship-it-straight-into-the-ROM blobs, tUME saved a **generic IFF** file and used pluggable
  converters called **tUMEPACKs** to emit each console's format. That "edit once, target many"
  separation is exactly the architecture every modern editor now takes for granted.

**Why it faded**: by the end of **1994** development had slowed — the authors believed the market for
a 2D map editor was shrinking. They were wrong (2D tilemaps outlived the 16-bit era by 30 years and
counting), but by then the team had moved on. tUME is **now released as freeware**, with downloads
on greggman's site, and the **supporting Echidna libraries (`elibs`) are open-sourced under the New
BSD license** on GitHub.

### The full source exists — recovered 2026-06-10 (MPL 1.1)

You were right that you'd seen it. On **June 18, 2000**, Echidna released the **complete tUME source
as "Open tUME" under the Mozilla Public License 1.1**. It lived at `members.aol.com/opentume` — long
dead (AOL Hometown shut down in 2008) — but **fully preserved in the Wayback Machine**, and now
**recovered and vendored into this repo** at
[`vendor/tume/`](../../vendor/tume/) (`tumesrc.zip`, sha256
`22c9eb2b…`, full provenance + exact recovery URLs in [`vendor/tume/README.md`](../../vendor/tume/README.md)).

What's in it:

- **179 files — 59 `.c`, 70 `.h`, 8 real-mode `.asm`**, a `MAKEFILE` and batch build scripts.
- **Every file carries the MPL-1.1 header**: *"The Original Code is tUME: the Universal Map Editor,
  released June 18, 2000. The Initial Developer … is Echidna. Copyright (c) 1989-2000 Echidna."*
- Programmer credits in the source: **Greg Marquez, John Alvarado, Gregg Tavares**.
- Sibling archives recovered too: `tumeexe.zip` (DOS binary), `tp16/tpbin/dumptume.zip` (tUMEPack +
  utility *source*), `tumedemo/tumedocs/samples.zip`.

MPL-1.1 is a real FOSS, file-level weak-copyleft license — it **permits redistribution and
modification** (keep the MPL header on modified files; combine with other-licensed files freely). So
a port is legally clear. The earlier "freeware binary only" caveat is **resolved: source confirmed,
in hand, openly licensed.** The separately-maintained
[github.com/greggman/elibs](https://github.com/greggman/elibs) (New BSD) is the modern descendant of
the EGGS libraries tUME builds against.

---

## 2. Can we port it? (and "increase resolution")

With the MPL source now recovered (§1), a port is **legally clear and technically feasible** — but
it's still a real project, not a weekend, and it's hard to justify over shipping the successors. Two
reasons.

**a) It's 16-bit DOS C welded to Borland tooling and Echidna's DOS libs.** Per `src/BUILD.TXT`, tUME
builds only under **Borland C 3.1 + Turbo Assembler 3.1** against a **bundled, frozen version of
Echidna's EGGS libraries** (the modern [`elibs`](https://github.com/greggman/elibs) descendant won't
link it unchanged). A Linux port means: rewrite/remove **8 real-mode `.asm` files**, untangle 16-bit
near/far-pointer and 640K/EMS memory-model assumptions across 59 `.c` files, and replace the entire
EGGS DOS graphics+input layer with a modern toolkit (SDL/Qt). That's a multi-week port of a tool
whose feature set is a strict subset of editors that already run natively on Linux. The cheap "port"
that needs **zero** code is just **DOSBox** — which runs the recovered `tumeexe.zip` today.

**b) A bigger/higher-res *editing window* — and here the recovered source corrects an earlier
over-estimate.** Reading `vendor/tume/src/`, tUME's renderer is **better-architected than a "talks
straight to the framebuffer" 1990s app**, and your instinct ("surely they render tiles with loops")
is exactly right:

- The map is drawn by `ShowRoomRectLayer()` (`TUMEDRAW.C:2079`) — **row/column loops that stamp tiles
  into an abstract `ByteMap *destBM`**, parameterized by an arbitrary destination rect
  (`dxDst,dyDst,cxDst,cyDst`). It is *not* hard-wired to a fixed framebuffer; it fills whatever
  surface and rectangle you hand it.
- **Zoom already exists**: `SetGlobalZoom()` (`TUMEDRAW.C:439`) drives `wSrcSkip`/`wDstDup`
  (source-skip / destination-duplicate), and `wSrcSizeToDstSize()` does the scaling math.
- **Scroll over a virtual canvas already exists**: there's a `BigByteMap *BigBackBM` *larger than the
  screen*, with `CopySmallRectToBig`/`CopyBigRectToSmall` (`MSDOS.C:750‑753`) blitting a viewport of
  it onto the display. Panning a window over a big buffer is the original design, not something to add.
- They **already abstracted the backend across two platforms** — `#if __AMIGAOS__` vs DOS branches
  throughout — so the seam where the renderer meets the OS surface is *already* a defined boundary.

So what actually pins the window to its cramped size is narrow and localized: the physical display is
**MCGA 320×200×8bpp** — `DisplayBM` is one global `ByteMap` whose dimensions come from a couple of
constants (`ScreenWidth/Height` ← `NormalScreenWidth/Height`) and whose `.data` points at the VGA
framebuffer via `GetDisplayAddress()` (`OpenDBufGraphics(MODE_MCGA, …, 64000U, …)`; `64000 = 320×200`).

That reframes "make the editing window bigger and higher-resolution" into a **bounded backend port**,
not a rewrite:

1. **Enlarge the display surface** — set those `Screen*` constants to e.g. 1280×720 and point
   `DisplayBM->data` at an SDL/Qt surface instead of `GetDisplayAddress()`. The existing tile loop +
   zoom + `BigBackBM` scroll fill the larger rect with no change to the render core.
2. **Replace the DOS MCGA/EGGS display+input backend with SDL** (and the `IBMMOUSE.C` mouse driver,
   which hardcodes a 320-wide range). This is the same boundary they already cross for Amiga.
3. **Reflow the fixed-position UI chrome** (`EUI.C`, `MENUITM2.C`, `COLORREQ.C` dialogs/palettes lay
   out against 320×200). This is the genuine grunt-work — but it's UI furniture, not the renderer.

So my earlier "most of the work of writing a new editor" was wrong: the renderer is
resolution-independent already and zoom/scroll are built in. The real cost is **(2) a DOS→SDL backend
swap + (3) reflowing fixed UI coordinates** — a real multi-week port, but a *port*, not a rewrite.

Still, the cheaper paths remain worth weighing: **DOSBox** runs it today with zero code (you just get
a magnified 320×200, not more visible map), and **a resizable high-DPI canvas is free in Tiled/LDtk**.
The port is now genuinely feasible and a fun one — just be clear it's a "modernise the display
backend of a 26-year-old codebase" project, not a weekend afternoon.

---

## 3. The more capable successors

The lineage tUME pioneered (edit-once / convert-per-target, metatile composition) is fully realised
in today's open-source editors:

| Tool | License | Best for | Why it's the heir | Editing canvas (the thing you want) |
|------|---------|----------|-------------------|-------------------------------------|
| **[Tiled](https://www.mapeditor.org/)** ([github](https://github.com/mapeditor/tiled)) | BSD / GPL | **General — the de-facto standard** | Direct spiritual successor: engine-agnostic, TMX/JSON export, **Lua scripting** for custom exporters (the modern tUMEPACK), object layers, huge ecosystem | **Resizable, high-DPI, free zoom/pan, minimap, multi-monitor**; infinite maps |
| **[LDtk](https://ldtk.io/)** | MIT | Indie level design | Best-in-class **auto-tiling** (rule-based), clean JSON, multiple tilesets/level | Resizable, smooth zoom/pan, world-view across levels |
| **[Tilemap Studio](https://github.com/Rangi42/tilemap-studio)** | LGPL | **Retro consoles specifically** | The closest *retro* heir — purpose-built for **Game Boy / GBC / GBA / NES / SNES** tilemaps, understands real VRAM formats | Resizable window + zoom, still emits console-accurate 8×8 data |
| **[M8TE](https://github.com/nesdoug/M8TE)** | open | SNES homebrew | SNES map + tile editor for the homebrew toolchain | Modern resizable window; SNES-native output |

**Recommendation by use-case:**
- *"I want what tUME did, but with a big modern editing window"* → **Tiled**. Resizable high-DPI
  canvas, free zoom/pan, infinite maps, scriptable exporters. This is the answer to "more capable
  successor."
- *"I'm targeting actual 16-bit consoles"* → **Tilemap Studio** (and/or M8TE for SNES). These keep
  the retro-accurate output tUME was prized for, on a maintained modern codebase.

---

## 4. What this means for Foundry Linux

Foundry's audience is game devs, and `foundry-retro-tools` already curates the 8/16-bit toolchain —
this slots right in. Concrete moves:

1. **Tiled — already shipped.** ✅ It's a `Depends:` of `foundry-game-frameworks`
   (`foundry-apt/packages/foundry-game-frameworks/debian/control:13`, Tiled `1.11.90-1`), which
   `foundry-core` pulls in — so Tiled is in **every edition** (core ⊆ anvil ⊆ sprite ⊆ atelier) *and*
   the Phase 2 devbox, and it's installed in the ISO (`build.log`: "Setting up tiled (1.11.90-1)").
   Nothing to do. (The successor analysis above is *why* this was the right call.)
2. **Vendor Tilemap Studio** via the `/package` skill — it's the retro-accurate heir and a natural
   companion to the existing `foundry-retro-tools` set (mame, cc65, dasm, …). C++/FLTK, LGPL-3,
   sha256-pinned release tarball (`v4.0.1`) from
   [github.com/Rangi42/tilemap-studio](https://github.com/Rangi42/tilemap-studio). This is the
   higher-value packaging job than any tUME port. **Linkage caveat**: it needs FLTK 1.4.5 (26.04
   ships 1.4.4, Wayland-hybrid, which won't compile it) — the decision to **static-link FLTK 1.4.5**
   is worked out in
   [2026-06-10-tilemap-studio-fltk-linkage.md](2026-06-10-tilemap-studio-fltk-linkage.md).
3. **Optionally** add **LDtk** (MIT, but ships as a precompiled Electron app — check redistribution
   terms and whether universe carries it before vendoring).
4. **tUME itself**: the MPL-1.1 source is now **recovered and vendored** at
   [`vendor/tume/`](../../vendor/tume/) — preserved against the dead AOL host regardless of what we do
   next. For end users, a "run tUME in DOSBox" recipe satisfies the nostalgia with zero porting. A
   real SDL/Qt port is now *possible* (source in hand, license permits it) but remains a multi-week
   project; reserve it for a dedicated branch and only if there's a specific reason Tiled/Tilemap
   Studio can't cover. It is **not** in `foundry-apt/packages/` — that tree is for things that build
   to a `.deb` on 26.04, which 16-bit DOS tUME does not.

**Bottom line**: tUME earned its reputation by inventing the edit-once-convert-per-target workflow.
The right tribute isn't to port the 1992 binary — it's to ship the tools that perfected its idea.
**Tiled already ships** in every edition; the only open packaging move is **Tilemap Studio** for the
retro-console crowd. Keep tUME as a vendored footnote (and a tempting rainy-day port).

---

## Sources

- [tUME — the Universal Map Editor (Greggman / Echidna)](https://games.greggman.com/game/tume___the_universal_map_editor/)
- [The Echidna Libraries (`elibs`, New BSD)](https://greggman.github.io/elibs/) · [github.com/greggman/elibs](https://github.com/greggman/elibs)
- [tUME — Giant Bomb concept page](https://www.giantbomb.com/tume/3015-6808/)
- [tUME (the Universal Map Editor) — SpritesMind / Sega dev forum](https://gendev.spritesmind.net/forum/viewtopic.php?t=2685)
- [Tiled — mapeditor.org](https://www.mapeditor.org/) · [github.com/mapeditor/tiled](https://github.com/mapeditor/tiled)
- [LDtk — Level Designer Toolkit](https://ldtk.io/)
- [Tilemap Studio (GB/GBC/GBA/NES/SNES)](https://github.com/Rangi42/tilemap-studio)
- [M8TE — SNES Map and Tile Editor](https://github.com/nesdoug/M8TE)
