# Plan — https://foundrylinux.org/ landing page

## Context

`foundrylinux.org` has an active Cloudflare zone (set up by `scripts/bootstrap.sh` for the APT repo), but no root website — the apex domain returns nothing. This plan adds a single Astro landing page under `site/` in this monorepo, deployed to Cloudflare Pages on tag push, and served at `https://foundrylinux.org/`.

`https://foundrylinux.org/install.sh` will serve the Phase 0 installer directly from the static build so `curl … | bash` works without hitting GitHub raw.

---

## Deliverables

| Artifact | Location |
|----------|----------|
| Astro site | `site/` |
| Bootstrap script | `scripts/bootstrap-site.sh` |
| GitHub Actions workflow | `.github/workflows/site-deploy.yml` |
| Taskfile additions | top-level `Taskfile.yml` |

---

## Phase 1 — Bootstrap script (`scripts/bootstrap-site.sh`)

Runs once. Follows the same pattern as `scripts/bootstrap.sh` (loads `BOOTSTRAP_CACHE`, `cf_api` helper, `--dry-run` flag, `set -euo pipefail`).

Steps:
1. Load cached `CF_ACCOUNT_ID` and `CF_API_TOKEN` from `BOOTSTRAP_CACHE` (already written by Phase 1 bootstrap).
2. **Create Pages project** via Cloudflare API:
   ```
   POST /accounts/{CF_ACCOUNT_ID}/pages/projects
   { "name": "foundrylinux-org", "production_branch": "main" }
   ```
3. **Attach custom domain** `foundrylinux.org` (Cloudflare will create CNAME-flattened A record automatically):
   ```
   POST /accounts/{CF_ACCOUNT_ID}/pages/projects/foundrylinux-org/domains
   { "name": "foundrylinux.org" }
   ```
4. **Create scoped API token** for Pages deploy (using existing `cf_api` helper pattern from bootstrap.sh):
   - Policy: `com.cloudflare.api.account.page:edit` scoped to the account
   - Name: `foundrylinux-site-ci`
5. **Wire GitHub secrets** on `foundry-linux/foundrylinux.org`:
   - `CF_PAGES_ACCOUNT_ID` — from cache
   - `CF_PAGES_API_TOKEN` — the scoped token from step 4

---

## Phase 2 — Static placeholder (`site/`)

No framework — a single hand-written HTML file. Absolute minimum:

```
site/
  index.html    — "FOUNDRY LINUX" and nothing else
  Taskfile.yml  — stub (no build step needed)
```

`index.html`:
```html
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>foundrylinux.org</title>
</head>
<body>
  <p>foundrylinux.org</p>
</body>
</html>
```

No build step. The deploy workflow uploads `site/` directly as a Pages deploy:
```
wrangler pages deploy site/ --project-name foundrylinux-org
```

---

## Phase 3 — GitHub Actions workflow (`.github/workflows/site-deploy.yml`)

Tag-driven, identical trigger pattern to `foundry-apt`'s `publish.yml`:

```yaml
on:
  push:
    tags: ['v*']
  workflow_dispatch:

jobs:
  deploy:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v6
      - uses: cloudflare/wrangler-action@v4
        with:
          apiToken: ${{ secrets.CF_PAGES_API_TOKEN }}
          accountId: ${{ secrets.CF_PAGES_ACCOUNT_ID }}
          command: pages deploy site/ --project-name foundrylinux-org
```

No pnpm, no build step — `site/` is pure static HTML.

---

## Phase 4 — Taskfile additions

Add to top-level `Taskfile.yml`:

```yaml
  site-deploy:
    desc: "Deploy site to Cloudflare Pages (requires CF_PAGES_API_TOKEN + CF_PAGES_ACCOUNT_ID)"
    cmds:
      - wrangler pages deploy site/ --project-name foundrylinux-org
```

No build task needed — no build step.

---

## Phase 5 — Skill updates (alongside implementation)

Every file we create has a corresponding template addition to `~/.claude/skills/cloudflare-static-site/`. Update the skill in the same commit as the project file it mirrors.

### New templates

| Skill template | Placeholders | Source |
|---|---|---|
| `templates/scripts/bootstrap-site.sh` | `<DOMAIN>`, `<SLUG>`, `<GH_ORG>/<GH_REPO>`, `<CF_ZONE_NAME>`, `<PROJECT_NAME>` | write alongside `scripts/bootstrap-site.sh` |
| `templates/site/index.html` | `<DOMAIN>` (in `<title>`) | write alongside `site/index.html` |
| `templates/.github/workflows/deploy-static.yml` | `<PROJECT_NAME>` | write alongside `.github/workflows/site-deploy.yml` |

### Existing template updates

- `templates/Taskfile.yml` — add `site-deploy` task (Pages, no build step variant).

### SKILL.md updates

Add a new **"Pages path (plain static HTML)"** section documenting the no-Astro/no-Terraform/no-SSM variant:
- When to choose it (placeholder or early-stage site before the design is decided)
- Inputs: `<DOMAIN>`, `<SLUG>`, `<GH_ORG>/<GH_REPO>`
- Steps: run `bootstrap-site.sh` → create `site/index.html` → add `deploy-static.yml` → tag to ship
- Update `description:` frontmatter to mention "plain static HTML + Cloudflare Pages" as an alternative path.
- Update placeholder table with the new placeholders (`<PROJECT_NAME>`).

---

## Phase 6 — TODO + plan tracking

- Add `TODO.md` entry: `[ ] flesh out foundrylinux.org site once placeholder is live`.

---

## Execution order

1. Run `bash scripts/bootstrap-site.sh` — creates Pages project + domain attachment + GitHub secrets.
2. Create `site/index.html`; write `templates/site/index.html` to skill.
3. Add `.github/workflows/site-deploy.yml`; write `templates/.github/workflows/deploy-static.yml` to skill.
4. Add `site-deploy` Taskfile task; update `templates/Taskfile.yml` in skill.
5. Write `templates/scripts/bootstrap-site.sh` to skill (after bootstrap-site.sh is proven working).
6. Update `SKILL.md` — add Pages path, update description + placeholder table.
7. Tag and release to trigger the deploy workflow.

---

## Verification — COMPLETE ✓

1. `curl -I https://foundrylinux.org/` → HTTP/2 200, content-type: text/html ✓
2. `curl -fsSL https://foundrylinux.org/` → contains `FOUNDRY LINUX` ✓
3. `curl -I https://apt.foundrylinux.org/` → HTTP/2 200 (no regression) ✓

## Post-launch updates

- Claude Design bundle imported (v0.2.0) — replaced plain text placeholder with React landing page
- Brand domain corrected to `foundrylinux.org` (topbar) + `apt.foundrylinux.org` (install snippets) — v0.2.3
- `apt.foundrylinux.org` index styled via shared `foundrylinux.org/styles.css` — v0.0.29

---

## Critical files

| Path | Action |
|------|--------|
| `scripts/bootstrap.sh` | **Read** — reuse `cf_api`, `cache_set`, `r2_put_secret`, `--dry-run` pattern |
| `scripts/bootstrap-site.sh` | **Create** — new, follows bootstrap.sh conventions |
| `site/index.html` | **Create** — minimal placeholder |
| `.github/workflows/site-deploy.yml` | **Create** — new workflow |
| `Taskfile.yml` | **Edit** — add `site-deploy` task |
| `TODO.md` | **Edit** — add site-launch tracking item |
| `~/.claude/skills/cloudflare-static-site/templates/scripts/bootstrap-site.sh` | **Create** — parameterised version |
| `~/.claude/skills/cloudflare-static-site/templates/site/index.html` | **Create** — parameterised version |
| `~/.claude/skills/cloudflare-static-site/templates/.github/workflows/deploy-static.yml` | **Create** — parameterised version |
| `~/.claude/skills/cloudflare-static-site/templates/Taskfile.yml` | **Edit** — add `site-deploy` task |
| `~/.claude/skills/cloudflare-static-site/SKILL.md` | **Edit** — add Pages path section, update description + placeholder table |
| `docs/plans/` | **Create** `2026-05-18-site-launch.md` — plan copy for the repo |
