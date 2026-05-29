# Navbar motion graphics — `site/embers.js`

Ideas for the topbar canvas effect. Current effect: **shadow pass** (★9, shipped). Previously tried and retired: breathe glow, grey pinstripes, shockwave rings, ember scatter, sword glint, cooling bloom, edge light.

The canvas sits as the last child of the topbar, painted over white nav elements via `mix-blend-mode: multiply`.

---

## Shipped

- **Shadow pass** ★9 — soft Gaussian shadow drifts across via `mix-blend-mode:multiply`. The absence of light rather than its presence.

## Tried and retired

- **Cooling bloom** ★7 — warm orange-white Gaussian luminance blob appears at random x, spreads outward, cools and fades.
- **Edge light** ★4 (bad clipping) — bottom edge brightens to near-white orange, spreads upward as soft gradient, dims.

---

## Candidate ideas

### Thermal / forge

- **Molten seam** — 1–2 px bright horizontal line (white-orange core, soft glow) traverses the bar once left→right. The bar is a slab of metal; the seam is a crack showing what's beneath.
- **Thermal gradient creep** — one edge subtly warm-tinted (faint orange haze); that warmth conducts slowly across to the other side over 3–4 s, then cools. Unlike cooling bloom (point source expanding outward), this is directional edge-to-edge conduction.

### Mechanical / percussive

- **Anvil resonance** — single circular brightness ring expanding from a random point *within* the bar (not from below). One ring, very slow expansion, fades. The navbar surface is the anvil face.
- **Steel flex** — the bar subtly bows: a slow vertical-only displacement wave passes through once, as if the bar is a thin plate flexing. Near-imperceptible at low amplitude.
- **Weld flash** — at a random x, a 200–300 ms burst of blue-white overbrightness (welder's arc). No ramp-in: nothing → blinding dot → nothing. Brevity is the effect.
- **Mercury bead** — a single small bright specular dot (radial gradient, silvery-white, 4–6 px) rolls across the bottom of the bar end to end, leaving a brief luminous trail that fades immediately. Like a ball bearing rolling across a polished surface.
- **Ferrofluid spikes** — along the bottom edge, 4–8 thin vertical spikes of varying heights (10–20 px) shoot upward simultaneously, hold ~200 ms, retract. Like ferrofluid snapping to a magnet.

### Retro / digital

- **Scanline roll** — a band of faint horizontal scanlines (2 px on / 2 px off, low opacity) drifts downward through the bar once. CRT monitor refresh. Connects to the MAME/arcade side of the brand.
- **Bit corruption** — a traveling zone of mild pixel-level brightness jitter (`canvas ImageData`) sweeps left→right like a slow data error crossing a screen.
- **Current flow** — extremely sparse, nearly invisible points drifting in one direction (organized, not random). DC current / electron drift. Distinguished from ember scatter by directionality and faintness.
- **Letterpress ghost** — a watermark of "ANVIL" or a single glyph materializes at very low opacity (~0.05), holds, fades. No motion; purely the ghostly presence of something barely legible.
- **Phosphor afterimage** — the navbar leaves a faint ghost copy at its previous painted position, drifts upward a few pixels, and dissolves over ~0.6 s. CRT phosphor persistence.

### Abstract / mathematical

- **Interference rings** — two simultaneous circular wave systems from two random x positions; brightness adds at intersections (constructive interference). Wave-tank demo aesthetic.
- **Grain pulse** — film grain (Gaussian-distributed per-pixel brightness noise via `ImageData`) ramps up ~0.5 s, holds briefly, fades. Organic, photographic.
- **Chromatic slip** — R channel shifts 1 px left, B channel shifts 1 px right for 1–2 s, then snaps back. Near-invisible RGB split; felt more than seen. SVG `feOffset` + `feBlend`.
- **Standing wave** — the bar behaves as a plucked string: a standing wave (fixed nodes, oscillating antinodes) appears across the full width at 3–5 wavelengths, then decays in amplitude. Acoustic physics made visible.
- **Magnetic field lines** — faint Bézier arcs (like iron filings around a bar magnet) materialize across the bar, curving from one end, looping over, curving back — fade in slowly, hold, dissolve. Cool-toned and mathematical.

### Material / optical

- **Oil slick** — a very subtle iridescent sheen (desaturated spectral colors — pale blue, violet, green — at ~0.03 opacity) drifts slowly across the bar. Thin-film interference, like the surface of a freshly quenched blade or motor oil on water.
- **Spangle** — a brief overlay of delicate crystalline geometry (Voronoi cell edges, thin and bright) flashes onto the bar and fades. Like zinc spangle on galvanized steel: industrial, geometric, beautiful.
