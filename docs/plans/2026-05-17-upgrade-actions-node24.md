# Upgrade GitHub Actions to eliminate Node 20 deprecation warning

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
