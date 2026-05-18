# Plan — Lighthouse CI pipeline for foundrylinux.org

**Status: DONE** — committed alongside this plan.

## Context

foundrylinux.org had no performance/quality gate. Both sibling projects (worldfoundry.org,
indri.studio) use a consistent Lighthouse pattern adopted here verbatim:
`lighthouse@13.3.0` → JSON reports in `/tmp/lh/latest/` →
`scripts/lighthouse-threshold.sh` gate (≥ 95) → GitHub Actions step summary +
90-day artifact upload + durable archive at `site/lh/<tag>/`.

---

## Approach

- **`lighthouse@13.3.0` via npx** — same as siblings; no `package.json` needed
- **`scripts/lighthouse-threshold.sh`** — copied verbatim from worldfoundry.org; threshold: 95
- **Post-deploy**, `continue-on-error: true` throughout — Lighthouse never blocks a ship
- **curl polling** (indri.studio pattern, cap 60 s) to wait for Cloudflare propagation
- **`RUNS=1` in CI**, `RUNS=${RUNS:-3}` locally
- **Archive to `site/lh/<tag>/`** — JSONs committed back to main; served at
  `https://foundrylinux.org/lh/<tag>/home.run-1.report.json` on the next deploy

---

## Files changed

| File | Change |
|---|---|
| `scripts/lighthouse-threshold.sh` | New — copied from worldfoundry.org |
| `Taskfile.yml` | New `lighthouse` task |
| `.github/workflows/site-deploy.yml` | New `lighthouse` job (needs: deploy) |
| `~/.claude/skills/import-claude-design/SKILL.md` | Steps 6–8; Lighthouse notes |

---

## Verification

1. `task lighthouse` runs locally and prints scores table for `https://foundrylinux.org`
2. `bash scripts/lighthouse-threshold.sh` prints Markdown table; exits 0 if scores ≥ 95
3. Push a tag — `lighthouse` job appears in Actions after `deploy` completes
4. Actions step summary shows Lighthouse score table + CLS budget check
5. JSON artifact `lighthouse-<tag>` downloadable from Actions run
6. After next `task site-deploy`, `https://foundrylinux.org/lh/<tag>/home.run-1.report.json` returns the JSON report
