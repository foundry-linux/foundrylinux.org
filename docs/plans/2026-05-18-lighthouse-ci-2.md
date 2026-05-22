# Lighthouse CI pipeline for foundrylinux.org + import-claude-design skill

## Context

foundrylinux.org has no performance/quality gate. Both sibling projects (worldfoundry.org,
indri.studio) use a consistent Lighthouse pattern: custom Taskfile task → JSON reports in
`/tmp/lh/latest/` → `scripts/lighthouse-threshold.sh` gate → GitHub Actions step summary +
artifact upload. Adopt the same pattern for consistency; adapt for the simpler single-page
site (no Astro build, no public/lh archive needed).

---

## Approach

- **`lighthouse@13.3.0` directly** (not `@lhci/cli`) — same as siblings
- **`scripts/lighthouse-threshold.sh`** — copy verbatim from worldfoundry.org (threshold: 95)
- **Post-deploy**, `continue-on-error: true` throughout — Lighthouse never blocks a ship
- **curl polling** (indri.studio pattern, cap 60s) instead of fixed sleep
- **`RUNS=1` in CI**, `RUNS=${RUNS:-3}` locally — same as siblings
- **Artifact upload 90 days**, step summary — same as siblings
- **Archive to `site/lh/<tag>/`** — commit JSONs back to main; served at `https://foundrylinux.org/lh/<tag>/home.run-1.report.json` on next deploy (wrangler deploys `site/` directly, so files land immediately on the next `task site-deploy`)

---

## Files

### 1. `scripts/lighthouse-threshold.sh` (new)

Copy verbatim from `worldfoundry.org/scripts/lighthouse-threshold.sh`. Threshold: 95.
Reads `/tmp/lh/latest/*.run-1.report.json`, writes Markdown table to `$GITHUB_STEP_SUMMARY`
and stdout, exits 1 on violations.

### 2. `Taskfile.yml` — add `lighthouse` task

Adapted from worldfoundry.org (single URL):

```yaml
  lighthouse:
    desc: "Run Lighthouse against https://foundrylinux.org, RUNS=${RUNS:-3} runs, devtools throttling. Reports under /tmp/lh/latest/."
    dir: "{{.ROOT_DIR}}"
    cmds:
      - |
        set -euo pipefail
        mkdir -p /tmp/lh/latest
        RUNS=${RUNS:-3}

        URLS=(
          "https://foundrylinux.org/|home"
        )
        SLUGS=(); for pair in "${URLS[@]}"; do SLUGS+=( "${pair#*|}" ); done

        for pair in "${URLS[@]}"; do
          URL="${pair%|*}"; SLUG="${pair#*|}"
          for RUN in $(seq 1 "$RUNS"); do
            TS=$(date -u +%Y-%m-%dT%H:%M:%SZ)
            echo "[$TS] === ${SLUG} run ${RUN} ==="
            npx --yes lighthouse@13.3.0 "$URL" \
              --form-factor=mobile \
              --throttling-method=devtools \
              --chrome-flags="--headless=new --no-sandbox --disable-gpu" \
              --quiet --output=json \
              --output-path="/tmp/lh/latest/${SLUG}.run-${RUN}.report.json"
          done
        done
        echo
        echo "=== devtools / n=${RUNS} / scores per run ==="
        printf "%-30s %-3s %-4s %-4s %-4s %-5s %-6s %-5s %s\n" "page" "run" "perf" "a11y" "bp" "fcp" "lcp" "tbt" "cls"
        for SLUG in "${SLUGS[@]}"; do
          for RUN in $(seq 1 "$RUNS"); do
            jq -r --arg slug "$SLUG" --arg run "$RUN" '
              [$slug, $run,
               (.categories.performance.score*100|round|tostring),
               (.categories.accessibility.score*100|round|tostring),
               (.categories["best-practices"].score*100|round|tostring),
               .audits["first-contentful-paint"].displayValue,
               .audits["largest-contentful-paint"].displayValue,
               .audits["total-blocking-time"].displayValue,
               .audits["cumulative-layout-shift"].displayValue
              ] | @tsv' "/tmp/lh/latest/${SLUG}.run-${RUN}.report.json"
          done
        done | column -t -s $'\t'
        echo
        if [ "$RUNS" -gt 1 ]; then
          echo "=== Perf medians (n=${RUNS}) ==="
        else
          echo "=== Perf scores (n=1) ==="
        fi
        for SLUG in "${SLUGS[@]}"; do
          PERF=()
          for RUN in $(seq 1 "$RUNS"); do
            PERF+=( "$(jq -r '(.categories.performance.score*100)|round' \
              /tmp/lh/latest/${SLUG}.run-${RUN}.report.json)" )
          done
          MED=$(printf '%s\n' "${PERF[@]}" | sort -n | sed -n $(( (RUNS + 1) / 2 ))p)
          printf "  %-30s  %d\n" "$SLUG" "$MED"
        done
```

### 3. `.github/workflows/site-deploy.yml` — add `lighthouse` job

Add after the existing `deploy` job. Adapted from indri.studio:

```yaml
  lighthouse:
    needs: deploy
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v6

      - uses: arduino/setup-task@v2
        with:
          version: '3.x'
          repo-token: ${{ secrets.GITHUB_TOKEN }}

      - name: Wait for Cloudflare propagation
        timeout-minutes: 1
        run: |
          set -euo pipefail
          until curl -sf https://foundrylinux.org/ -o /dev/null; do sleep 3; done

      - name: Lighthouse audit
        id: lh
        continue-on-error: true
        env:
          RUNS: '1'
        run: task lighthouse

      - name: Stage Lighthouse artifacts
        if: steps.lh.outcome == 'success'
        run: |
          mkdir -p lighthouse-bundle
          cp /tmp/lh/latest/*.json lighthouse-bundle/

      - name: Upload Lighthouse bundle
        if: steps.lh.outcome == 'success'
        uses: actions/upload-artifact@v4
        with:
          name: lighthouse-${{ github.ref_name }}
          path: lighthouse-bundle/*.json
          retention-days: 90

      - name: Lighthouse summary
        if: steps.lh.outcome == 'success'
        run: |
          {
            echo "## Lighthouse — ${{ github.ref_name }}"
            echo
            echo "| Page | Perf | FCP | LCP | TBT | CLS |"
            echo "|---|---:|---:|---:|---:|---:|"
            for f in lighthouse-bundle/*.run-1.report.json; do
              slug=$(basename "$f" .run-1.report.json)
              jq -r --arg slug "$slug" '
                "| \($slug) | \((.categories.performance.score*100)|round) | \(.audits["first-contentful-paint"].displayValue) | \(.audits["largest-contentful-paint"].displayValue) | \(.audits["total-blocking-time"].displayValue) | \(.audits["cumulative-layout-shift"].displayValue) |"
              ' "$f"
            done
          } >> $GITHUB_STEP_SUMMARY

      - name: CLS budget check
        if: steps.lh.outcome == 'success'
        continue-on-error: true
        env:
          CLS_BUDGET: '0.05'
        run: |
          set -euo pipefail
          violations=0
          {
            echo
            echo "### CLS budget: ≤ ${CLS_BUDGET}"
            echo
            echo "| Page | CLS | Status |"
            echo "|---|---:|:---:|"
            for f in /tmp/lh/latest/*.run-1.report.json; do
              SLUG=$(basename "$f" .run-1.report.json)
              CLS=$(jq -r '.audits["cumulative-layout-shift"].numericValue' "$f")
              STATUS=$(awk -v c="$CLS" -v b="$CLS_BUDGET" \
                'BEGIN { print (c+0 <= b+0) ? "✓ OK" : "⚠️ OVER" }')
              printf "| %s | %.4f | %s |\n" "$SLUG" "$CLS" "$STATUS"
              if [ "${STATUS#⚠️}" != "$STATUS" ]; then
                violations=$((violations + 1))
              fi
            done
          } >> "$GITHUB_STEP_SUMMARY"
          if [ "$violations" -gt 0 ]; then
            echo "::warning::$violations page(s) over CLS budget ${CLS_BUDGET}; see job summary."
            exit 1
          fi

      - name: Threshold check
        if: steps.lh.outcome == 'success'
        continue-on-error: true
        run: ./scripts/lighthouse-threshold.sh

      - name: Archive Lighthouse bundle to main
        if: |
          always() &&
          steps.lh.outcome == 'success' &&
          startsWith(github.ref, 'refs/tags/v')
        run: |
          set -euo pipefail
          TAG=${{ github.ref_name }}

          git config user.name 'github-actions[bot]'
          git config user.email '41898282+github-actions[bot]@users.noreply.github.com'

          git fetch origin main
          git checkout -B main origin/main

          mkdir -p "site/lh/$TAG"
          cp /tmp/lh/latest/*.run-1.report.json "site/lh/$TAG/"

          git add "site/lh/$TAG/"
          git commit -m "CI: archive Lighthouse bundle for $TAG [skip ci]"
          git push origin main
```

### 4. `~/.claude/skills/import-claude-design/SKILL.md` — full enhanced Lighthouse section

Insert as new Step 6 between deploy and tag. Renumber current Step 6 (Verification) to Step 7.

```markdown
## Step 6 — Lighthouse

task lighthouse

Audits <DOMAIN> after deploy (RUNS=1). Prints a score table (Perf / A11y / BP / FCP / LCP /
TBT / CLS). Then run the threshold gate:

bash scripts/lighthouse-threshold.sh

Exits 0 if all four categories (Perf / A11y / BP / SEO) are ≥ 95. If any score is below
threshold, fix before tagging. JSON reports are in /tmp/lh/latest/ for inspection.
```

Also add a **Notes** entry:

```markdown
- **Lighthouse setup** — requires `scripts/lighthouse-threshold.sh` (copied from the
  worldfoundry.org pattern) and a `lighthouse` Taskfile task using `lighthouse@13.3.0`
  with devtools throttling. See the worldfoundry.org or foundrylinux.org implementations
  as reference. Threshold: 95 on Perf / A11y / BP / SEO. JSON reports land in
  `/tmp/lh/latest/`; CI uploads them as a 90-day artifact.
```

---

## Critical files to read before writing

- `worldfoundry.org/scripts/lighthouse-threshold.sh` — copy verbatim ✓ (already read)
- `worldfoundry.org/Taskfile.yml` lines 332–390 — lighthouse task ✓ (already read)
- `indri.studio/.github/workflows/deploy.yml` lines 47–190 — CI pattern ✓ (already read)
- `foundrylinux.org/.github/workflows/site-deploy.yml` — existing deploy job ✓ (already read)

---

## Verification

1. `task lighthouse` runs locally and prints scores table for `foundrylinux.org`
2. `bash scripts/lighthouse-threshold.sh` prints Markdown table; exits 0 if scores ≥ 95
3. Push a tag — `lighthouse` job appears in Actions after `deploy` completes
4. Actions step summary shows Lighthouse score table + CLS budget check
5. JSON artifact `lighthouse-<tag>` downloadable from Actions run
6. After next `task site-deploy`, `https://foundrylinux.org/lh/<tag>/home.run-1.report.json` returns the JSON report
