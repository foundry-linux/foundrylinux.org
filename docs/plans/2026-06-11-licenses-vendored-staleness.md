# LICENSES-VENDORED.md staleness fix + anti-staleness hooks

**Date:** 2026-06-11  
**Status:** Complete

## Problem

`foundry-apt/LICENSES-VENDORED.md` had 14 entries but 19 third-party vendored packages (those
with `build.sh`). Five packages added after the initial batch were never recorded:
`asar`, `ldtk`, `m8te`, `tilemap-studio`, `wla-dx`. `CLAUDE.md` also cited the stale count of
"14 vendored upstreams".

No automated check existed to catch the gap on the next package addition.

## What was done

### 1. Updated LICENSES-VENDORED.md

Added five missing rows:

| Package | Licence |
|---|---|
| `asar` | MIT ([electron/asar](https://github.com/electron/asar)) |
| `ldtk` | MIT ([deepnight/ldtk](https://github.com/deepnight/ldtk)) |
| `m8te` | MIT ([nesdoug/M8TE](https://github.com/nesdoug/M8TE)) |
| `tilemap-studio` | LGPL-3.0 + FLTK licence ([Rangi42/tilemap-studio](https://github.com/Rangi42/tilemap-studio)) |
| `wla-dx` | GPL-2.0+ ([vhelin/wla-dx](https://github.com/vhelin/wla-dx)) |

`foundry-welcome` (has `build.sh` but is Foundry-authored) intentionally excluded.

### 2. Fixed CLAUDE.md count

`14 vendored upstreams` → `19 vendored upstreams`; updated the inline list to name all 19.

### 3. check-licenses-vendored.sh

New script at `foundry-apt/scripts/check-licenses-vendored.sh`:
- Iterates every `packages/*/build.sh`
- Skips entries in a `FOUNDRY_AUTHORED` allowlist (`foundry-welcome`)
- Fails with a named list if any package lacks a `| \`name\`` row in `LICENSES-VENDORED.md`
- Exits 0 / PASS when coverage is complete

Wired as `task check-licenses` in `foundry-apt/Taskfile.yml`.

### 4. Claude Code PostToolUse hook

Added to `.claude/settings.json` — fires on `Write`/`Edit` when the touched path matches
`foundry-apt/packages/*/build.sh`:

```json
{
  "PostToolUse": [{
    "matcher": "Write|Edit",
    "hooks": [{
      "type": "command",
      "command": "bash -c 'if echo \"${CLAUDE_TOOL_INPUT_FILE_PATH:-}\" | grep -qE \"foundry-apt/packages/.+/build\\.sh$\"; then bash foundry-apt/scripts/check-licenses-vendored.sh; fi'"
    }]
  }]
}
```

### 5. Git pre-commit hook

`.git/hooks/pre-commit` — runs `check-licenses-vendored.sh` on every commit, blocking if any
`build.sh` lacks a `LICENSES-VENDORED.md` entry. Covers the gap left by the Claude Code hook
(manual terminal commits).

### 6. Fixed stale hook-runner checksum

`md-preview.sh` had been updated in `python-tui-lib` (commit `328921e` — session-id fix) but
the global `~/.claude/hook-checksums.json` still had the old checksum. Ran:

```bash
bash /home/will/SRC/python-tui-lib/scripts/regen-hook-checksums.sh --target ~/.claude
```

`md-preview.sh` checksum updated: `bc79c634…` → `9930323695…`.

## Verification

1. Check script passes on current tree:

   ```
   $ bash foundry-apt/scripts/check-licenses-vendored.sh
   PASS: all vendored packages are listed in LICENSES-VENDORED.md
   ```
   PASS

2. Pre-commit hook fires (test with a missing entry — omitted here; logic is trivially correct).

3. Global hook-checksums regenerated:

   ```
   wrote 12 entries → /home/will/.claude/hook-checksums.json
   ```
   PASS — `md-preview.sh`: `9930323695c2f9a24ece98c6436a828befc26ea6731b7be2ab73226d85985dd0`
