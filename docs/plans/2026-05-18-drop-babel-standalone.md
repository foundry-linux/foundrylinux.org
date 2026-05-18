# Plan — Drop Babel standalone from foundrylinux.org; update import-claude-design skill

**Status: DONE** — committed `f0109d3`.

## Context

`site/index.html` loaded `@babel/standalone@7.29.0` (2.9 MB) from unpkg and transpiled four
`.jsx` files in the browser at runtime. This caused a multi-second blank page before anything
rendered. The fix is to pre-compile JSX to plain JS with esbuild (a one-line build step, no
config), commit the compiled files, and remove Babel standalone from the page entirely.

The import-claude-design skill previously said "no build step" — that note was updated so
every design import also pre-compiles before committing.

---

## Approach

**esbuild in transform mode** (no bundling): each `.jsx` file compiles to a `.js` file that
is plain JS but still uses React/ReactDOM as globals from CDN. No imports added, no module
system introduced — the existing window.* architecture is preserved.

Compiled files are **committed** to the repo so Cloudflare Pages deploys them as static
files without needing a build step in CI.

Note: `--loader=jsx` flag is not needed — esbuild infers the jsx loader from the `.jsx`
extension automatically. Passing it explicitly causes an error ("loader without extension
only applies when reading from stdin").

---

## Changes

### `Taskfile.yml` — `site-build` task

```yaml
site-build:
  desc: Compile site/*.jsx → site/*.js via esbuild (drops Babel standalone dep)
  dir: "{{.ROOT_DIR}}"
  cmds:
    - npx --yes esbuild site/tweaks-panel.jsx --platform=browser --outfile=site/tweaks-panel.js
    - npx --yes esbuild site/icons.jsx        --platform=browser --outfile=site/icons.js
    - npx --yes esbuild site/sections.jsx     --platform=browser --outfile=site/sections.js
    - npx --yes esbuild site/app.jsx          --platform=browser --outfile=site/app.js
```

### `site/index.html`

Replaced development React + Babel standalone with production React + pre-compiled `.js`:

```html
<script src="https://unpkg.com/react@18.3.1/umd/react.production.min.js"
  integrity="sha384-DGyLxAyjq0f9SPpVevD6IgztCFlnMF6oW/XQGmfe+IsZ8TqEiDrcHkMLKI6fiB/Z"
  crossorigin="anonymous"></script>
<script src="https://unpkg.com/react-dom@18.3.1/umd/react-dom.production.min.js"
  integrity="sha384-gTGxhz21lVGYNMcdJOyq01Edg0jhn/c22nsx0kyqP0TxaV5WVdsSH1fSDUf5YJj1"
  crossorigin="anonymous"></script>
<script src="tweaks-panel.js"></script>
<script src="icons.js"></script>
<script src="sections.js"></script>
<script src="app.js"></script>
```

### `~/.claude/skills/import-claude-design/SKILL.md`

Added Step 3 — Compile JSX (`task site-build`) between extraction and commit. Replaced
"No build step" note with "Build step required" describing the esbuild compile.

---

## Verification

1. `task site-build` runs without error and produces `site/{tweaks-panel,icons,sections,app}.js`
2. Open `site/index.html` in a browser — page renders without Babel, no console errors
3. Network tab shows no `babel.min.js` request; React loads as `production.min.js`
4. Run `/import-claude-design` with a test zip — compiled `.js` files appear in the commit
