# Context

The Install section channel cards are being rearranged into 3 rows (01 full-width / 02–03–04 as thirds / 05 full-width). Card 05 "Fresh install" is the most committed path — replacing the OS — and should visually signal that weight. The CSS grid change (3-col, nth-child span) is already done. What remains is making card 05 feel distinct and premium.

## What "fancier" means for 05

Card 05 is the terminal commitment card. It should feel like a hero/CTA card rather than an info card:

1. **Accent ember glow background** — `radial-gradient` from `var(--accent-soft)` at the bottom-left, matching the existing hover treatment on regular cards but always-on and more intense. Gives it warmth without being garish.
2. **Stronger bottom border** — `3px solid` with `var(--accent)` at partial opacity (the current `replaces-os` rule only sets it on `.channel-inv`; promote to the card border itself).
3. **Larger title** — `channel-label` inside 05 bumped from 20px to 26–28px to fill the wider canvas.
4. **Horizontal internal layout** — because 05 is full-width, a single-column card layout wastes the space. Split the card interior into two zones:
   - Left (⅔): number / label / hook text — the existing content
   - Right (⅓): ISO download rows (Anvil + Atelier, each with ↓ icon and size), replacing the `channel-cmd` line — so the user gets the download directly from the card without scrolling to the detail block below.
5. **No `channel-inv` label** — drop "replaces-os" from card 05; the visual treatment already communicates the weight.

## Implementation

### `site/sections.jsx`

- Card 05 render: instead of the standard inner layout, render a special two-zone layout:
  ```jsx
  <a className="channel-card channel-card--hero" href="#path-iso" data-invasiveness="replaces-os">
    <div className="hero-left">
      <span className="channel-num">05</span>
      <span className="channel-label">Fresh install</span>
      <p className="channel-hook">Replace everything. Full branded desktop from first boot.</p>
    </div>
    <div className="hero-right">
      <span className="hero-dl-label">download iso</span>
      <a className="hero-dl-row" href="https://iso.foundrylinux.org/foundry-anvil-latest-amd64.iso">
        <span>Anvil</span><span className="hero-dl-size">~5 GB</span><DownloadIcon />
      </a>
      <a className="hero-dl-row" href="https://iso.foundrylinux.org/foundry-atelier-latest-amd64.iso">
        <span>Atelier</span><span className="hero-dl-size">~15 GB</span><DownloadIcon />
      </a>
    </div>
  </a>
  ```
- Pull card 05 out of the CHANNELS map — render it separately after the map so it gets the special treatment without a flag/conditional inside the loop.

### `site/styles.css`

```css
.channel-card--hero {
  flex-direction: row;
  align-items: center;
  gap: 40px;
  background: radial-gradient(ellipse at 0% 100%, var(--accent-soft) 0%, transparent 55%);
  border-bottom: 3px solid rgba(var(--accent-rgb, 230,100,30), 0.4);
  padding: 32px 36px;
}
.channel-card--hero .channel-label { font-size: 28px; }
.channel-card--hero:hover {
  background: radial-gradient(ellipse at 0% 100%, var(--accent-soft) 0%, transparent 45%),
              radial-gradient(circle at 70% 30%, var(--accent-soft) 0%, transparent 50%);
}
.hero-left { flex: 1; }
.hero-right {
  flex-shrink: 0;
  width: 260px;
  border-left: 1px solid var(--hairline-strong);
  padding-left: 32px;
  display: flex;
  flex-direction: column;
  gap: 8px;
}
.hero-dl-label { /* mono, 9.5px, ink-faint, uppercase, letter-spacing */ }
.hero-dl-row {
  display: flex; align-items: center; gap: 8px;
  color: var(--ink-soft); font-family: var(--font-mono); font-size: 12px;
  text-decoration: none;
}
.hero-dl-row:hover { color: var(--accent); }
.hero-dl-size { margin-left: auto; color: var(--ink-faint); font-size: 11px; }
.hero-dl-row svg { width: 13px; height: 13px; flex-shrink: 0; }
/* Mobile: collapse hero back to column */
@media (max-width: 700px) {
  .channel-card--hero { flex-direction: column; }
  .hero-right { width: 100%; border-left: none; border-top: 1px solid var(--hairline-strong);
                padding-left: 0; padding-top: 16px; }
}
```

## Eliminate the PATH 04–05 detail block

Card 05 now contains the ISO downloads inline, so the `#path-iso` detail block (the third block in `install-left`) is redundant. Remove it from `sections.jsx`. The dual-boot callout strip moves into card 05's `hero-right` panel as a third item (small text below the two download rows, linking to the Ubuntu dual-boot guide) — so the info isn't lost.

Cards 04 and 05 currently `href="#path-iso"`. After the block is gone:
- Card 04 (Dual boot) → `href="#path-apt"` scroll target is wrong; update to `href="#path-iso"` pointing at card 05 itself (the browser scrolls to the card, which is the destination).
- Card 05 → no href needed since it's self-contained; use `<div>` instead of `<a>`.

This leaves two clean detail blocks in `install-left`:
1. `#path-apt` — apt codeblock (path 01)
2. `#path-vm` — VM downloads (path 03)

The `install-left` section is now visibly shorter and matches the sidebar height better.

Also update `docs/plans/2026-05-28-install-section-redesign.md` status to `done` and add a note that the PATH 04–05 block was deferred and is now resolved here.

## Critical files

- `site/sections.jsx` — extract card 05 from CHANNELS map → `.channel-card--hero`; remove PATH 04–05 block; move dual-boot callout into hero-right; card 04 href update
- `site/styles.css` — `.channel-card--hero`, `.hero-left`, `.hero-right`, `.hero-dl-*`
- `docs/plans/2026-05-28-install-section-redesign.md` — status → done, note added

## Verification

1. `node scripts/ssr-render.js` exits 0
2. Screenshot at 1280px: 01 full-width top / 02–03–04 row / 05 full-width hero with ember glow and two ISO download rows + dual-boot note in right panel
3. Screenshot at 375px: hero card collapses to single column, download rows stack below text
4. `grep "path-iso" site/index.html` → only the card id and card 04 href; no separate block
5. Clicking Anvil/Atelier rows in card 05 navigates to iso.foundrylinux.org correctly
