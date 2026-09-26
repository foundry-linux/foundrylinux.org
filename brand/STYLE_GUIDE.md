# Foundry Linux style guide

Foundry Linux should look like a precise tool made in a forge: dark, solid,
technical, and warm only where the heat is useful. The visual language supports
game development and reverse-engineering work without turning into costume
industrial design.

This guide governs new public surfaces: the website, APT and ISO pages, package
art, social graphics, and desktop branding. [`../DESIGN.md`](../DESIGN.md) is
the implementation reference for the website's components and responsive CSS.

## Principles

- **Make the tool legible first.** Commands, packages, download choices, and
  system state must be easy to scan.
- **Use weight and restraint.** Large type, clear rules, and flat forms carry
  the identity. Decoration should explain a function or reinforce hierarchy.
- **Treat orange as heat.** Ember orange identifies the active path, a primary
  action, or a point of attention. It is not a general fill colour.
- **Keep the forge modern.** Use clean geometry, not rivets, rust, smoke,
  medieval ornament, or faux distressed effects.

## Palette

| Name | Value | Use |
|---|---|---|
| Ember | `#ff5b1a` | Active controls, commands, selected edition, small highlights |
| Near black | `#111` | Primary page background |
| Warm white | `#ebe8e2` | Main reading text |
| Silver highlight | `#f7f7f7` | Metal wordmark and bright UI surfaces |
| Silver midtone | `#b8b8b8` | Metal gradients and supporting detail |
| Silver shadow | `#4a4a4a` | Metal depth and subdued edges |
| Soft ink | `rgba(200,197,190,0.62)` | Secondary copy |
| Faint ink | `rgba(170,167,160,0.75)` | Labels and metadata |

Use the CSS custom properties in [`../site/styles.css`](../site/styles.css)
in product code. Do not introduce a second orange, a cool grey palette, or
large orange text blocks.

## Typography

| Typeface | Role |
|---|---|
| Big Shoulders Display, 900 | Wordmark, section titles, edition and product names |
| Space Grotesk, 400–600 | Body copy and explanatory text |
| JetBrains Mono, 400–500 | Commands, labels, versions, paths, package names, and status |

Set wordmarks and labels in uppercase. Use ordinary sentence case for prose.
Mono labels use modest letter spacing and should stay short; do not set body
paragraphs in mono.

## The anvil

The anvil represents the workbench: a place to build, inspect, repair, and
ship. It is the primary symbol for Foundry Linux.

- The favicon is a white, filled anvil with three forge sparks on an ember
  square. Its source is [`../site/favicon.svg`](../site/favicon.svg).
- Use the supplied asset unchanged at browser-tab sizes. Its large flat forms
  are deliberate; thin strokes and fine holes vanish at 16 px.
- The hero can use the larger split light-and-shadow anvil from
  [`../site/icons.jsx`](../site/icons.jsx). That treatment is for large dark
  surfaces only; it is not a favicon or app icon.
- Supporting interface icons are simple 64 px line drawings with rounded caps
  and joins. They inherit `currentColor` and use the shared stroke widths in
  `site/icons.jsx`.
- Keep the anvil upright. Do not add gradients, bevels, photorealistic metal,
  text inside the mark, or a separate icon colour scheme.

### Small icon delivery

Pages link to `favicon-anvil.svg` and `favicon-anvil.ico`. The distinct names
prevent browser and CDN caches from retaining a previous mark. Keep the SVG
and ICO identical in appearance; the ICO contains 16, 32, 48, and 64 px
frames.

After changing the source SVG, run:

```sh
bash scripts/generate-favicons.sh
```

This regenerates the ICO and copies both formats into the APT and ISO publish
sources.

## Layout and surfaces

Use a near-black ground, warm-white text, fine silver rules, and generous empty
space. Build hierarchy with size, weight, alignment, and thin dividers before
adding panels or effects.

- Use a silver gradient only for the Foundry wordmark and primary metallic
  buttons.
- Use square or nearly square corners. The website uses a 2 px radius where a
  soft edge helps a control read as interactive.
- Prefer simple grids and bordered cells for lists, package groups, and
  comparisons.
- Treat command blocks as work surfaces: mono text, an ember prompt or command
  token, and a quiet dark surround.

## Interaction and accessibility

- Every status, selection, or error needs text or a clear structural signal in
  addition to colour.
- Preserve strong contrast for text, commands, and controls on the dark
  background.
- Hover glow is a small confirmation, not an animation showpiece.
- Respect `prefers-reduced-motion`; transitions should be short and should not
  move essential content.

## Avoid

- Broad orange backgrounds outside the favicon and small active states.
- Stock gears, flames, anvils, or pseudo-medieval blacksmith imagery.
- Glossy glass, neon cyberpunk, chrome gradients on ordinary body text, or
  excessive shadows.
- Dense dashboards and decorative borders that compete with package names or
  commands.
- New icon styles beside the existing line-art system.

## Source files

| Need | Source |
|---|---|
| Visual rules and live CSS tokens | [`../DESIGN.md`](../DESIGN.md), [`../site/styles.css`](../site/styles.css) |
| UI icons and large hero anvil | [`../site/icons.jsx`](../site/icons.jsx) |
| Favicon source and fallbacks | [`../site/favicon.svg`](../site/favicon.svg), [`../site/favicon.ico`](../site/favicon.ico) |
| Organisation avatar files | [`org-avatar.svg`](org-avatar.svg), [`org-avatar-dark.svg`](org-avatar-dark.svg) |
