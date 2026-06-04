# Plan: Link-check gate for publishing

## Context

The site-deploy pipeline has Lighthouse performance/CLS gates post-deploy, but nothing currently
validates that every link in the built HTML actually resolves before the deploy goes out. The site
has ~20 hardcoded external URLs (GitHub, apt repo, iso downloads, magnet links) plus internal
anchor/path links across two pages (`index.html`, `packages.html`). A broken link slips through
silently today. This adds a pre-deploy gate that fails the GitHub Actions deploy job if any link
is broken.

---

## Implementation

### New file: `scripts/check-links.js`

Zero new dependencies — uses Node 22's built-in `fetch`. Runs as the gate.

Logic:
1. Read all `site/*.html` files.
2. Extract every `href="..."` value with a regex (React SSR output is well-formed).
3. Categorise each href:
   - **`#anchor`** → look for `id="<anchor>"` in the same HTML file.
   - **`/path`** → check that `site/<path>.html` or `site/<path>/index.html` exists on disk.
   - **`https?://`** → `fetch(url, { method: 'HEAD', signal: AbortSignal.timeout(10_000) })` with 2 retries on 5xx/network error.
   - **`magnet:`** → skip (not HTTP-checkable).
4. Deduplicate external URLs across pages so each is fetched once.
5. Print a Markdown table of results; if `$GITHUB_STEP_SUMMARY` is set, append to it.
6. Exit 1 if any check failed.

CLI flag: `--skip-external` skips HTTP fetches for quick local runs (internal links only).

### Edit: `Taskfile.yml`

Add `link-check` task after the existing `site-build` task:

```yaml
link-check:
  desc: "Check all links in built HTML (--skip-external to skip HTTP checks)"
  deps: [site-build]
  cmds:
    - node scripts/check-links.js {{.CLI_ARGS}}
```

### Edit: `.github/workflows/site-deploy.yml`

Insert a new step in the `deploy` job, **after** the SSR render and **before** `wrangler-action`:

```yaml
- name: Check links
  run: node scripts/check-links.js
```

This blocks the `wrangler pages deploy` step if any link returns a broken status.

---

## Verification

1. Run `task link-check` locally after `task site-build` — should print a clean table and exit 0.
2. Temporarily break a link in `site/sections.jsx` (e.g. change a GitHub URL to a 404 path),
   rebuild, re-run `task link-check` — should exit 1 with the broken URL listed.
3. Push a tag; confirm the "Check links" CI step appears between SSR render and wrangler deploy
   in the GitHub Actions run log and passes.
