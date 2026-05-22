# Plan — Style apt.foundrylinux.org via shared foundrylinux.org CSS

## Context

The APT repo index at `apt.foundrylinux.org` currently has its own embedded style block with
GitHub-dark colours and generic mono fonts. The user wants the apt page to feel like a sibling
of `foundrylinux.org` (same fonts, colours, design language), but without maintaining a separate
copy of the CSS. The solution: link `https://foundrylinux.org/styles.css` directly from the apt
index, and use a small scoped `<style>` block for apt-specific layout that relies entirely on the
CSS variables defined in the shared sheet.

---

## What changes

### 1. `site/styles.css` — no change

`@import` inside CSS creates a serial waterfall (outer CSS → @import → fonts) vs parallel
loading. For the apt page, having the font `<link>` directly in `<head>` is faster, and for
`foundrylinux.org` the existing `<link>` preloads already handle it. Don't touch `styles.css`.

### 2. `foundry-apt/scripts/generate-index.sh` — replace embedded style block

The `<head>` gets two `<link>` tags for parallel loading (fonts + shared CSS), then a small
scoped `<style>` that only uses `var(--...)` from the shared sheet — nothing to maintain:

Replace the large embedded `<style>` with:

```html
<link rel="preconnect" href="https://fonts.googleapis.com" />
<link rel="preconnect" href="https://fonts.gstatic.com" crossorigin />
<link rel="stylesheet" href="https://fonts.googleapis.com/css2?family=Space+Grotesk:wght@400;500;600&family=JetBrains+Mono:wght@400;500&display=swap" />
<link rel="stylesheet" href="https://foundrylinux.org/styles.css" />
<style>
  /* apt-index layout — variables come from foundrylinux.org/styles.css */
  * { box-sizing: border-box; margin: 0; padding: 0; }
  a { color: var(--accent); text-decoration: none; }
  a:hover { text-decoration: underline; }
  .wrap { max-width: 860px; margin: 0 auto; padding: 2rem 1rem; }
  h1 {
    font-family: var(--font-mono);
    font-size: 11px;
    letter-spacing: 0.25em;
    text-transform: uppercase;
    color: var(--ink-faint);
    margin-bottom: .5rem;
  }
  .site-title {
    font-family: var(--font-wordmark);
    font-size: 2.2rem;
    font-weight: 900;
    text-transform: uppercase;
    letter-spacing: -0.01em;
    color: var(--ink);
    margin-bottom: .25rem;
  }
  .subtitle { color: var(--ink-soft); margin-bottom: 2rem; font-size: 14px; }
  h2 {
    font-family: var(--font-mono);
    font-size: 10.5px;
    color: var(--ink-faint);
    text-transform: uppercase;
    letter-spacing: 0.2em;
    margin: 2.5rem 0 .75rem;
  }
  pre {
    background: rgba(255,255,255,0.015);
    border: 1px solid var(--hairline-strong);
    padding: 1rem 1.25rem;
    overflow-x: auto;
    font-family: var(--font-mono);
    font-size: 13px;
    line-height: 1.7;
    color: var(--ink);
  }
  table {
    width: 100%;
    border-collapse: collapse;
    border: 1px solid var(--hairline);
  }
  th {
    font-family: var(--font-mono);
    font-size: 10px;
    letter-spacing: 0.2em;
    text-transform: uppercase;
    color: var(--ink-faint);
    padding: .5rem .75rem;
    text-align: left;
    border-bottom: 1px solid var(--hairline-strong);
  }
  td { padding: .5rem .75rem; border-top: 1px solid var(--hairline); font-size: 14px; }
  td:nth-child(1) { white-space: nowrap; font-family: var(--font-mono); font-size: 13px; }
  td:nth-child(2) { color: var(--ink-soft); white-space: nowrap; font-family: var(--font-mono); font-size: 12px; }
  footer {
    margin-top: 3rem;
    color: var(--ink-faint);
    font-family: var(--font-mono);
    font-size: 10.5px;
    letter-spacing: 0.14em;
    text-transform: uppercase;
    border-top: 1px solid var(--hairline);
    padding-top: 1rem;
  }
</style>
```

Also fix the stale `worldfoundry-dev` reference in the Quick Install snippet
→ `foundry-retro-tools`.

### 3. Bump tag to republish

After committing in the monorepo, sync to `/tmp/foundry-apt-sync` and push tag `v0.0.29`
(or `v0.0.30` if the universe-fix run already used `v0.0.28`/`v0.0.29`).

---

## Execution order

1. Edit `site/styles.css` — add `@import` for Google Fonts at top.
2. Edit `foundry-apt/scripts/generate-index.sh` — new `<link>` + scoped `<style>` + fix Quick Install.
3. Commit both + push to `foundrylinux.org` main (new site deploy tag for styles.css; apt tag for the index).
4. Confirm `styles.css` is already live (no change needed to the main site).
5. Sync to `/tmp/foundry-apt-sync`, commit, push, tag for foundry-apt.
6. Watch CI.

---

## Critical files

| Path | Action |
|------|--------|
| `site/styles.css` | No change |
| `foundry-apt/scripts/generate-index.sh` | Edit — replace embedded `<style>`, add shared CSS + font links, fix Quick Install package name |

---

## Verification

1. `curl -fsSL https://foundrylinux.org/styles.css | head -3` → shows `@import url(...googleapis...)`
2. `curl -fsSL https://apt.foundrylinux.org/ | grep 'foundrylinux.org/styles.css'` → link tag present
3. Open `https://apt.foundrylinux.org/` in browser — black background, ember-orange links, Space Grotesk body
4. `curl -fsSL https://apt.foundrylinux.org/ | grep 'worldfoundry'` → empty
