# Plan: Repology-badge audit tooling

## Context

The apt index renders a Repology version badge for any package whose
`debian/control` Source stanza declares `X-Repology-Project`. After backfilling
all vendored packages (2026-06-20), Will asked for **reusable tooling** — not
console one-offs — to (a) **audit** which hosted packages have / lack / opt out
of the badge, and (b) surface that in **the existing audit document** as a new
column. ("Create" is already covered — see below.)

Existing pieces (2026-06-20):
- `foundry-apt/scripts/check-repology-badges.sh` — GUARD: every `packages/*/build.sh`
  must declare `X-Repology-Project`; `none` is the opt-out. Wired into the
  git pre-commit hook, the `.claude/settings.json` PostToolUse hook, and
  `task check-badges`.
- `generate-index.sh` skips the badge when the value is empty or `none`.
- All 27 vendored packages backfilled (badge or `none`).

## Changes

1. **Audit mode on the guard** — `check-repology-badges.sh --report`: print every
   vendored package + status (`✅ badge <proj>` / `➖ opt-out` / `⚠️ MISSING`) and a
   summary line; exit 0. Wire as **`task audit-badges`** (foundry-apt Taskfile).
   *(done)*

2. **Badge column in the inventory doc** — `scripts/audit-apt-repos.sh` (the
   generator behind `docs/investigations/<date>-package-inventory.md`, run by
   `task audit-apt-repos` + the daily cron). Add a **Badge** column to the
   *Binary packages* table, keyed on the package's own source tree:
   - has `build.sh` + real `X-Repology-Project` → `` `<proj>` ``
   - has `build.sh` + `none` → `➖ none` (deliberate opt-out)
   - has `build.sh` + no field → `⚠️ MISSING` (a real gap to fix)
   - no `build.sh` (metapackage / canonical / sub-package / our-own) → `—` (n/a)

   Reuses the existing `read_local_control()` helper; add a `badge_status()`
   helper in the embedded Python. Metapackage table is unchanged (metapackages
   never carry a Repology badge).

3. **"Create" needs no new tool** — a missing badge on a *new* vendored package
   is already blocked at commit time by the guard, and the `/package` skill now
   prompts for `X-Repology-Project`. The audit (1, 2) makes existing gaps
   visible; fixing is a one-line `debian/control` edit (Repology project names
   need a human, so no auto-creator).

## Verification

Run from `foundry-apt/` (and repo root for the doc):

1. `task audit-badges` → table of all vendored packages; summary shows
   `MISSING: 0` (all backfilled). `shellcheck scripts/check-repology-badges.sh`.
2. `task audit-apt-repos` → regenerates `docs/investigations/2026-06-21-package-inventory.md`;
   the Binary packages table has a **Badge** column; spot-check halfempty=`halfempty`,
   python3-picire=`python:picire`, m8te=`➖ none`, calamares-settings=`—`, and no
   `⚠️ MISSING` rows.
3. `git diff` of the regenerated doc shows only the new column (plus the daily delta).
