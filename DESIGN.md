# DESIGN.md — foundrylinux.org

> Raised silver on black. Ember orange glow. Line-art everything else.

---

## Philosophy

The site reads as a machinist's wall plaque — industrial, precise, no decoration that doesn't carry weight. Black background, chrome-gradient type, a single hot ember accent. Every graphic is hand-drawn line art in `currentColor`. Nothing soft, nothing rounded beyond a 2px border-radius on the apt block.

---

## Color Tokens

All tokens are CSS custom properties on `:root`. The accent and background can be overridden by a Tweaks panel (not yet wired in production).

| Token | Value | Role |
|---|---|---|
| `--accent` | `#ff5b1a` | Ember orange — prompts, active states, edition highlights |
| `--accent-soft` | `rgba(255,91,26,0.18)` | Kit cell hover glow, edition tint |
| `--accent-glow` | `rgba(255,91,26,0.5)` | Text-shadow on accent elements |
| `--bg` | `#000` | Page background |
| `--silver-hi` | `#f7f7f7` | Gradient highlight end |
| `--silver-mid` | `#b8b8b8` | Mid-tone |
| `--silver-lo` | `#4a4a4a` | Shadow end |
| `--silver-shadow` | `#0d0d0d` | Deep drop |
| `--hairline` | `rgba(255,255,255,0.08)` | Default rule between cells/sections |
| `--hairline-strong` | `rgba(255,255,255,0.16)` | Codeblock borders, fingerprint box, apt block |
| `--ink` | `rgba(235,232,226,0.92)` | Body text |
| `--ink-soft` | `rgba(200,197,190,0.62)` | Secondary text, tagline, kit desc |
| `--ink-faint` | `rgba(170,167,160,0.75)` | Labels, metadata, requirements keys |

### Background Variants

Set via `data-bg` on `<body>`:

- **flat** (default): `#000`, no texture.
- **gradient**: radial warm dark at top (`#1a1410 → #000`).
- **textured**: SVG feTurbulence noise overlay on the same radial gradient. Subtle carbon-paper feel.

---

## Typography

Three typefaces, each with a fixed role. All self-hosted in `site/fonts/` — no external requests.

| Font | Weights | `font-display` | Role |
|---|---|---|---|
| **Big Shoulders Display** | 900 | `optional` (no CLS) | Wordmark, section titles, edition names, kit names |
| **Space Grotesk** | 400–600 | `swap` | Body copy, taglines, descriptions |
| **JetBrains Mono** | 400–500 | `swap` | Nav, labels, apt commands, metadata, fingerprints |

Fallback for Big Shoulders: Impact/Arial Black scaled with `size-adjust: 95%`, `ascent-override: 88%`, `descent-override: 12%` to match block height.

### Scale

- Wordmark: `clamp(72px, 16vw, 260px)` — expands to `clamp(88px, 22vw, 360px)` for single-word marks (`[data-mark="anvil"]`, `[data-mark="foundry"]`).
- Section titles: `clamp(48px, 6vw, 84px)`.
- Body: 16px base, 14–15px for secondary copy.
- Mono UI labels: 10.5–13px, `letter-spacing: 0.14–0.2em`, `text-transform: uppercase`.

---

## The Wordmark

`<h1 class="wordmark">` with two data attributes:

### `data-material`

Controls the gradient fill clipped to the text. Three variants:

- **`chrome`** (default) — sharp reflective banding: `#fff → #e8e8 → #b0b0 → #6a6a → #3a3a → #2a2a → #6a6a → #c8c8 → #f0f0 → #aaaa`.
- **`brushed`** — flatter gradient (`#c8 → #d8 → #a8 → #70 → #9a → #bc`) with a 1px horizontal line texture overlay (`repeating-linear-gradient` at 90°, alternating 4% / 3% opacity).
- **`hammered`** — SVG `feTurbulence + feDiffuseLighting` bump texture blended multiply over a simple gradient. Dimpled cast-iron look.

All three use `background-clip: text; -webkit-text-fill-color: transparent` plus a four-layer drop-shadow filter for the raised-on-black effect.

### `data-font`

Swaps the typeface for exploration builds:

- **`big-shoulders`** (default): Big Shoulders Display 900, `letter-spacing: -0.005em`.
- **`cinzel`**: Cinzel 900, serif, `letter-spacing: 0.02em`.
- **`major-mono`**: Major Mono Display 400, `letter-spacing: -0.02em`, size reduced to `clamp(54px, 12vw, 200px)`.
- **`black-ops`**: Black Ops One 400, `letter-spacing: 0.01em`.

---

## Layout

### Shell

```css
.shell { max-width: 1320px; margin: 0 auto; padding: 0 56px; }
/* ≤800px: padding collapses to 28px */
```

All sections use `.shell` as their width constraint. The shell is position-relative to anchor absolute decorative elements.

### Sections

```
border-top: 1px solid var(--hairline)
padding: 96px 0  →  64px 0 at ≤800px
```

Section header layout: title block (flex: 1) + blurb (flex: 0 0 320px) aligned to flex-end, collapses to column at ≤800px.

---

## Components

### Topbar

Sticky, `z-index: 50`, `backdrop-filter: blur(14px)`, `background: rgba(0,0,0,0.82)`.

Three columns: mark | nav | version badge.

- Mark: `FoundryMark` SVG (22×22) + `foundrylinux.org` in JetBrains Mono 11.5px.
- Nav: four anchors, mono 11.5px, `letter-spacing: 0.1em`.
- Version: `●` pulse dot (ember with glow) + `FOUNDRY 26.04 · "ANVIL" · LTS` in mono 11px. Hidden at ≤600px.

The topbar padding responds to a CSS typed property `--header-shrink` (0→1) — `padding: calc(22px - 14px * var(--header-shrink, 0)) 0` — for a scroll-shrink effect. Transitions at 220ms ease-in-out on `html`. No-op in current JS (property never set).

### Hero

Centered column. Top-to-bottom:

1. Eyebrow: `a linux distribution · est. 2026` — mono 11.5px, 0.3em tracking, flanked by 60px hairline rules using `::before`/`::after`.
2. `BigAnvilLogo` (420px wide) — half-lit silhouette (lit left, shadow right, seam highlight).
3. `.wordmark` — `FOUNDRY / LINUX` stacked.
4. Tagline — Space Grotesk 19px; `<em>` gets chrome gradient fill (no italic).
5. Apt command block — see below.
6. Hero actions — primary (silver CTA) + secondary (ghost outline).

Decorative `GearStackIcon` floats left at `opacity: 0.18`, hidden at ≤1100px.

### Apt Command Block

```
background: rgba(255,255,255,0.03)
border: 1px solid var(--hairline-strong)
border-radius: 2px
font: JetBrains Mono 14px
```

Three inline zones: `$` prompt (ember orange, glow), command text (flex: 1), COPY button (border-left, transitions on hover). A `::before` gradient overlay adds a top-edge sheen. Clipboard copy with 1400ms COPIED flash.

### CTAs

- **Primary** (`.cta-primary`): silver gradient button (`#fafa → #c0c0 → #fafa`), dark text, multi-layer inset + drop shadow. Lifts 1px + accent outer-glow on hover.
- **Secondary** (`.cta-secondary`): ghost, `inset 0 0 0 1px var(--hairline-strong)`. Lightens border on hover.

Both: JetBrains Mono 12px, 0.14em tracking, uppercase, 52px height.

### Kit Grid

3-column CSS grid (`→ 2-col at ≤900px`). Grid lines are `border-top + border-left` on the grid, `border-right + border-bottom` on each cell — produces seamless hairline lattice.

Each cell:
- 64×64 line-art icon, bottom-margin 32px.
- Role label: mono 10.5px, ember orange, 0.2em tracking.
- Name: Big Shoulders 700, 26px, uppercase.
- Desc: 14px, `--ink-soft`.
- Meta bar (pkg name + version): mono 10.5px, `--ink-faint`, justified, `border-top: 1px solid var(--hairline)`, `margin-top: auto`.
- Hover: radial accent glow at 40% 30%; icon color flips to ember with `drop-shadow`.

Last cell is a freeform "+1,400 more" callout using `SparksIcon`.

### Codeblock

Dark glass panel: `rgba(255,255,255,0.015)` fill + `var(--hairline-strong)` border + top-edge gradient.

Header bar: label left, three traffic-light dots right (`rgba(255,255,255,0.12)`, inset shadow). Labels: mono 10.5px, 0.18em tracking, uppercase.

Syntax tokens in `<pre>`:

| Class | Color | Meaning |
|---|---|---|
| `.step` | `--ink-faint` | Step headers (①②③), mono 10.5px, uppercase |
| `.ember` | `--accent` | Commands (`curl`, `sudo apt install`) |
| `.str` | `#c0a878` | String arguments / URLs |
| `.kw` | `#d8d8d8` | Flags (`-fsSL`) |
| `.cmt` | `--ink-faint` | `# comments` |

### Install Side Panel

Freeform prose + two bordered boxes:

- **Fingerprint**: `border: 1px solid var(--hairline-strong)`, mono 11.5px. Label in 10px uppercase faint; fingerprint value in ember with text-shadow glow.
- **Requirements list**: same border style; each row `display: flex; justify-content: space-between`. Keys: mono 10.5px faint. Values: 13px ink.

### Editions Grid

3-column grid, same hairline-lattice pattern as Kit. Each edition: icon → tag → name (Big Shoulders 38px) → desc → pkg/size footer bar.

Anvil (default) edition has:
- Subtle `background: linear-gradient(180deg, transparent, var(--accent-soft) 200%)`.
- `::after` "DEFAULT" badge — ember orange, glow, absolute top-right.
- Icon rendered in ember + glow shadow.

### Footer

Three-column grid: mark | nav | meta. Collapses to single column at ≤700px. All mono, uppercase, faint ink. 60px top padding, 80px bottom.

---

## Icon System

All icons are inline SVG with `viewBox="0 0 64 64"` (kit/section icons) or `"0 0 32 32"` (mark). Three stroke presets defined in `I`:

| Preset | strokeWidth | Used for |
|---|---|---|
| `I.stroke` | 1.4 | Most icons |
| `I.strokeThin` | 1.0 | Sparks, fine detail |
| `I.strokeThick` | 1.8 | AnvilEditionIcon (bolder at small sizes) |

All strokes: `fill: none`, `strokeLinecap: round`, `strokeLinejoin: round`, `stroke: currentColor`. No hardcoded colors except `BigAnvilLogo` (intentional two-tone silhouette: `#ededed` lit side, `#7a7a7a` shadow outline).

The `BigAnvilLogo` is the exception: flat silhouette with left/right clipPaths simulating a half-lit forge anvil. Two elliptical drop shadows on the ground plane. A 1px white seam line at the center axis.

Icons in the export list:

```
AnvilIcon, HammerIcon, SparksIcon
WorldFoundryIcon, BlenderIcon, MameIcon, F9dasmIcon, AssemblerIcon, LibvgmIcon, VgmstreamIcon
GamepadIcon, AnvilEditionIcon, SpriteIcon
BigAnvilLogo, FoundryMark, DownloadIcon, CopyIcon, ArrowRightIcon
ForgeSceneIcon, GearStackIcon
```

---

## Motion

- **`--header-shrink`**: registered as a CSS typed `<number>` so the value itself (not just derived properties) interpolates. Transitions on `html` at 220ms ease-in-out. Currently never mutated by JS; wired in advance for scroll-shrink.
- **Kit cell hover**: `background 0.2s`, icon `color 0.2s`, icon `filter 0.2s`.
- **CTA hover**: `transform 0.15s`, `box-shadow 0.15s`, `background 0.15s`.
- **Copy button**: `color 0.15s`, `background 0.15s`.
- All transitions respect `@media (prefers-reduced-motion: reduce)` — only `html` transition is explicitly suppressed there; others should be too if added.

---

## Responsive Breakpoints

| px | Change |
|---|---|
| ≤1100 | Hero deco elements (`GearStackIcon`) hidden |
| ≤900 | Kit grid → 2 col; Install → 1 col; Editions → 1 col |
| ≤800 | `.shell` padding 56px → 28px; hero padding reduced; section padding 96px → 64px; section-head → column |
| ≤700 | Footer → single column |
| ≤600 | Topbar version badge hidden; mark text hidden; nav font/spacing reduced |
| ≤560 | Kit cells: smaller padding, smaller icon, smaller type throughout |
| ≤500 | BigAnvilLogo width → 260px |
