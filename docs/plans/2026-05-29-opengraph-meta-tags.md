# Plan: OpenGraph meta tags on foundrylinux.org

## Context
The main site (`foundrylinux.org` and `/packages`) has `<meta name="description">` but no
OpenGraph or Twitter Card tags, so Discord/Mastodon link previews show no structured data.
The apt index already has OG tags (shipped in v1.5.0). This adds them to the two main pages.

## File
**`scripts/ssr-render.js`** — single file, two changes.

---

## Change 1 — add `url` to each entry in the `PAGES` table (lines 16–32)

```js
{ entry: 'site/app.jsx', ...,
  url: 'https://foundrylinux.org/', },

{ entry: 'site/packages.jsx', ...,
  url: 'https://foundrylinux.org/packages', },
```

## Change 2 — add OG tags in `wrap()` after `<meta name="description">` (line 79)

```html
  <meta property="og:type"        content="website" />
  <meta property="og:url"         content="${url}" />
  <meta property="og:title"       content="${title}" />
  <meta property="og:description" content="${description}" />
  <meta name="twitter:card"       content="summary" />
```

Update the `wrap()` signature to destructure `url`:
```js
function wrap(body, { title, description, url, extraBodyScripts }) {
```

No `og:image` — no social-card PNG exists (same decision as apt index).

---

## Execution

1. Edit `scripts/ssr-render.js` (2 hunks above)
2. `node scripts/ssr-render.js` — regenerates `site/index.html` and `site/packages.html`
3. `grep og:title site/index.html site/packages.html` → 1 hit each
4. Commit `scripts/ssr-render.js` + `site/index.html` + `site/packages.html`; push; tag
