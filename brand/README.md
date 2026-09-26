# Foundry Linux — brand marks

Off-site brand assets (deliberately **not** under `site/`, so they aren't
deployed with the website).

## Site favicons

[`../site/favicon.svg`](../site/favicon.svg) is the canonical anvil mark.
The main, APT, and ISO sites serve it alongside a transparent ICO fallback
containing 16, 32, 48, and 64 px images. After editing the SVG, run
`bash scripts/generate-favicons.sh` from the repository root (requires
librsvg's `rsvg-convert` and ImageMagick) to regenerate the ICO and sync both
assets to the APT and ISO publish sources. The generated assets are tracked,
so publishing needs no image conversion tools.

## GitHub org avatar

- `org-avatar.svg` — canonical (ember-orange `#ff5b1a`, the brand accent).
- `org-avatar-dark.svg` — alternate (dark, ember-glow; matches the site hero
  but reads muddier at small sizes).
- `org-avatar.png` — upload-ready 1000×1000 render of the canonical SVG.

Derived from [`../site/favicon.svg`](../site/favicon.svg) — the lit/shadow
anvil mark — centered on a solid brand-accent square.

Regenerate the PNG:

```sh
rsvg-convert -w 1000 -h 1000 org-avatar.svg -o org-avatar.png
```

**Upload (manual — org avatars have no REST API):**
`github.com/organizations/foundry-linux/settings/profile` → *Profile picture*.
