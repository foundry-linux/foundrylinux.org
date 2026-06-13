# Plan: audit-apt-repos — scheduled daily snapshots

## Context

`scripts/audit-apt-repos.sh` and the `audit-apt-repos` Taskfile task already exist and
produce `docs/investigations/YYYY-MM-DD-package-inventory.md` on demand. The TODO calls
out a missing daily/weekly schedule to capture those snapshots automatically.

A GitHub Actions workflow is the right mechanism — project mandate requires everything
reproducible from the repo, and local cron would be fragile (machine-off = missed run).
The script is designed for CI: it has no GUI deps (bash + curl + python3 stdlib), and
gracefully degrades when the sibling `../worldfoundry.org` repo is absent (prints WARN,
continues with live HTTP data).

## Change

**New file:** `.github/workflows/audit-apt-repos-snapshot.yml`

```yaml
name: Daily apt repo snapshot

on:
  schedule:
    - cron: '0 4 * * *'   # 04:00 UTC daily
  workflow_dispatch:

jobs:
  snapshot:
    runs-on: ubuntu-latest
    permissions:
      contents: write
    steps:
      - uses: actions/checkout@v6
        with:
          ref: main
          token: ${{ secrets.GITHUB_TOKEN }}

      - name: Generate and commit snapshot
        run: |
          set -euo pipefail
          OUT="docs/investigations/$(date -u +%F)-package-inventory.md"
          bash scripts/audit-apt-repos.sh -o "$OUT"
          git config user.name "foundry-bot"
          git config user.email "packages@foundrylinux.org"
          git add "$OUT"
          if git diff --cached --quiet; then
            echo "No changes — snapshot unchanged since last run"
            exit 0
          fi
          git commit -m "chore(audit): daily apt repo snapshot $(date -u +%Y-%m-%d)"
          git push
```

**Also:** mark the `audit-apt-repos — scheduled snapshots` TODO item done.

## Scope notes

- Only the new inventory file for today's date is staged (`git add "$OUT"`) — no other
  `docs/investigations/` files are touched.
- The sibling `worldfoundry.org` repo is not checked out; the script logs a WARN and
  falls back to live HTTP data (this is documented in the script at lines 77–84).
- No secrets beyond the default `GITHUB_TOKEN` are required.
- `actions/checkout@v6` targets Node 24 per project pinning convention.
- Existing workflows (`foundry-apt-publish.yml`, `site-deploy.yml`, etc.) are unchanged.
- If both apt repos are unreachable, the script exits non-zero and the job fails —
  intentional (flaky repo availability is worth a CI notification).

## Verification

1. After merging, trigger a manual run via GitHub Actions → `Daily apt repo snapshot`
   → **Run workflow**.
2. Confirm the job completes green and commits
   `docs/investigations/<today>-package-inventory.md` to `main`.
3. Check that the committed file contains the Mermaid dependency graph section and
   at least one `## Packages` table (both repos reachable).
4. Run a second manual trigger the same day — confirm "No changes — snapshot unchanged
   since last run" in the logs and no new commit is made.
5. Verify the next day's 04:00 UTC scheduled run produces a new dated file.
