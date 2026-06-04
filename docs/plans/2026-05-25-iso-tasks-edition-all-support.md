# Plan: Add `EDITION=all` support to iso-build, iso-smoke, iso-sign, iso-upload

## Context

`iso-publish` already handles `EDITION=all` by computing an `EDITIONS` var and
looping with `for: { var: EDITIONS }`. The four individual tasks — `iso-build`,
`iso-smoke`, `iso-sign`, `iso-upload` — only accept `anvil` or `atelier`.
This plan adds the same pattern to all four so each can be invoked standalone
with `EDITION=all`.

## Approach

All expansion happens **at the Taskfile level** (same pattern as `iso-publish`).
The scripts (`build-iso.sh`, `sign-iso.sh`, `upload-iso.sh`, `boot-smoke.sh`)
each handle one edition and need no changes.

## File to modify

**`Taskfile.yml`** — four task stanzas:

### 1. `iso-build`
- Add `vars: EDITIONS` (same `sh:` block as `iso-publish`)
- Update `generates:` from `dist/foundry-{{.EDITION | default "anvil"}}-*.iso`
  → `dist/foundry-*-*.iso` (glob covers both editions)
- Change `cmds:` to: `task: iso-bump` once, then `for: { var: EDITIONS }` +
  `cmd: EDITION="{{.ITEM}}" bash scripts/build-iso.sh`
- Update desc to `anvil|atelier|all`

### 2. `iso-smoke`
- Add `vars: EDITIONS`
- Change single `cmd:` to `for: { var: EDITIONS }` +
  `cmd: bash test/boot-smoke.sh dist/foundry-{{.ITEM}}-$(cat VERSION)-amd64.iso`
- Update desc to `anvil|atelier|all`

### 3. `iso-sign`
- Add `vars: EDITIONS`
- Change single `cmd:` to `for: { var: EDITIONS }` +
  `cmd: EDITION="{{.ITEM}}" bash scripts/sign-iso.sh`
- Update desc to `anvil|atelier|all`

### 4. `iso-upload`
- Add `vars: EDITIONS`
- Change single `cmd:` to `for: { var: EDITIONS }` +
  `cmd: EDITION="{{.ITEM}}" bash scripts/upload-iso.sh`
- Update desc to `anvil|atelier|all`

`iso-publish` is unchanged — it already loops and passes specific edition names
to sub-tasks, so no double-expansion occurs.

## Also update

**`foundry-iso/docs/howto-kubuntu-remix.md`** — the pipeline overview and Stage
2–5 task examples should show `all` as a valid option alongside
`anvil|atelier`.

## Verification

```bash
# Dry-run: check task --list shows updated descs
task --list | grep iso-

# Smoke: EDITION=all expansion for sign (no ISO needed — will error on missing
# file but the loop itself must iterate twice)
EDITION=all task iso-sign 2>&1 | grep -E "anvil|atelier"
```
