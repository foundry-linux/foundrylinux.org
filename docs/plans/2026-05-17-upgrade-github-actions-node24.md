# Upgrade GitHub Actions to eliminate Node 20 deprecation warning

**Status:** Done

## Context

Both `publish.yml` and `test.yml` use `actions/checkout@v4` and `actions/upload-artifact@v4`,
which target Node.js 20. GitHub is deprecating Node 20 on Actions runners, so every run
produces an annotation warning. We already have `FORCE_JAVASCRIPT_ACTIONS_TO_NODE24: true`
as a workaround, but the right fix is to upgrade to the action versions that natively target
Node 24.

## What to change

**Files:**
- `foundry-apt/.github/workflows/publish.yml`
- `foundry-apt/.github/workflows/test.yml`

**In each file:**
1. `actions/checkout@v4` → latest major version with Node 24 support (verify at execution time via `gh api repos/actions/checkout/releases/latest`)
2. `actions/upload-artifact@v4` → latest major version with Node 24 support (same verification)
3. Remove `FORCE_JAVASCRIPT_ACTIONS_TO_NODE24: true` env var — no longer needed once actions
   natively target Node 24

## Verification

```bash
bash scripts/bootstrap.sh --dry-run     # still exits 0
task bump                                # push next tag, confirm no annotation warning in run
```

Watch the Actions run — annotations section should be empty (or at most show the smoke-install
container warning, not the Node 20 deprecation).

## Result

Upgraded to `actions/checkout@v6` and `actions/upload-artifact@v7` (both Node 24 native).
Removed `FORCE_JAVASCRIPT_ACTIONS_TO_NODE24` from both workflows. Shipped as v0.0.8.

## Verification — executed 2026-05-18

### Step 1 — `bash scripts/bootstrap.sh --dry-run` still exits 0

```
$ bash scripts/bootstrap.sh --dry-run; echo "rc=$?"
... 46 lines of dry-run output ...
  [info]  Step 10 — sync and push the first release tag to trigger CI:
  [info]    task sync-and-release TAG=v0.0.1
rc=0
```

**PASS** — exit 0.

### Step 2 — current action versions pinned to Node 24 majors

```
$ grep -n 'uses: actions\|FORCE_JAVASCRIPT' foundry-apt/.github/workflows/*.yml
foundry-apt/.github/workflows/publish.yml:21:      - uses: actions/checkout@v6
foundry-apt/.github/workflows/publish.yml:103:        uses: actions/upload-artifact@v7
foundry-apt/.github/workflows/test.yml:13:      - uses: actions/checkout@v6
foundry-apt/.github/workflows/test.yml:34:        uses: actions/upload-artifact@v7

$ gh api repos/actions/checkout/releases/latest --jq '.tag_name'
v6.0.2
$ gh api repos/actions/upload-artifact/releases/latest --jq '.tag_name'
v7.0.1
```

**PASS** — workflows pin the current major lines (v6 / v7); `FORCE_JAVASCRIPT_ACTIONS_TO_NODE24` is gone.

### Step 3 — no Node 20 deprecation warnings in recent runs

```
$ gh run view -R foundry-linux/foundry-apt 26010000687 --log | grep -iE 'node.?20|deprecat'
(empty)
```

Latest 5 workflow runs on `foundry-linux/foundry-apt` all `conclusion: success`.

**PASS** — annotation warnings cleared.
