# Plan: list-isos.sh — add VMs to local table

## Context

`task iso-list` shows a columnar R2 table (iso | ova | vmdk | qcow2 per edition) but the local table is flat file-per-row, which means VM files would just silently appear as extra rows rather than being grouped with their edition. The user wants the local section to mirror the R2 format. Naming is fine as-is — `iso-list` fits the `iso-*` task namespace.

## Change

**File:** `foundry-iso/scripts/list-isos.sh`

Replace the local table (lines 64–80) with the same per-edition columnar layout used by the R2 table:

```
┌─────────────────┬──────────────────┬───────┬────────┬────────┬────────┐
│ local edition   │ timestamp        │ iso   │ ova    │ vmdk   │ qcow2  │
├─────────────────┼──────────────────┼───────┼────────┼────────┼────────┤
│ anvil-0.9.0     │ 2026-05-23 14:02 │ 4.6G  │ -      │ -      │ -      │
│ atelier-0.9.0   │ 2026-05-23 11:20 │ 15G   │ -      │ -      │ -      │
└─────────────────┴──────────────────┴───────┴────────┴────────┴────────┘
```

Logic (mirrors existing R2 loop):
- Glob `dist/foundry-{edition}-{version}-amd64.iso` to enumerate editions present locally.
- For each edition, extract version from filename, set timestamp from the ISO's mtime.
- For each of ova/vmdk/qcow2, check if the file exists; show size or "-".
- Keep the `_gb()` helper for R2 sizes; use `du -h` for local (consistent with current code).
- "no local builds" row if nothing found.

No change to the R2 section or Taskfile.

## Naming

Keep `iso-list` — all related tasks share the `iso-` prefix; renaming just one would be inconsistent.

## Verification

```bash
task iso-list
```

- R2 table unchanged.
- Local table has one row per edition with iso size populated and ova/vmdk/qcow2 as "-" (since no VMs built locally yet).
- After building a VM: confirm its size appears in the correct column.
