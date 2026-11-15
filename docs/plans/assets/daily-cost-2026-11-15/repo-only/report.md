# Daily Foundry build cost simulation

**Modeled total: $8.74 over 30 days; average $0.291/day.**

Historical activity replay with current measured timing proxies

| Component | USD |
|---|---:|
| Compute + IPv4 | 2.748 |
| Retained EBS | 5.837 |
| Publication/download transfer | 0.000 |
| S3 artifact storage | 0.155 |

## Assumptions and coverage

- Current package catalog is projected backward; deleted releases and historical promotion eligibility cannot be reconstructed from current feeds.
- Per-recipe cold intervals are proxies for future changed-source builds, not measured historical runtimes.
- Current artifact sizes proxy historical uploads. Verified local source sizes are included only when include_source_uploads is enabled; metadata indexes are not sized.
- Ubuntu archive updates are outside overlay activity coverage; daily ISO still refreshes dependencies.
- Validation/startup/publication/discovery inputs are explicit scenario allowances, not measured automated stage durations.
- No automatic CPU-count timing scaling; hardware changes require explicit measured timing overrides.
- Shared account free allowances are zero by default; rolling retention starts empty; latest retention includes one ISO and the current measured package/source catalog. existing_storage_gb adds other retained content. Requests, taxes, failures, signing/monitoring services and website operations are excluded.

## Per-day calculation

| Date | Packages to rebuild (coalesced) | Build proxy min | Compute | EBS | Egress | S3 | Total USD |
|---|---|---:|---:|---:|---:|---:|---:|
| 2026-10-16 | — | 0.00 | 0.091 | 0.195 | 0.000 | 0.005 | **0.291** |
| 2026-10-17 | — | 0.00 | 0.091 | 0.195 | 0.000 | 0.005 | **0.291** |
| 2026-10-18 | — | 0.00 | 0.091 | 0.195 | 0.000 | 0.005 | **0.291** |
| 2026-10-19 | — | 0.00 | 0.091 | 0.195 | 0.000 | 0.005 | **0.291** |
| 2026-10-20 | — | 0.00 | 0.091 | 0.195 | 0.000 | 0.005 | **0.291** |
| 2026-10-21 | — | 0.00 | 0.091 | 0.195 | 0.000 | 0.005 | **0.291** |
| 2026-10-22 | — | 0.00 | 0.091 | 0.195 | 0.000 | 0.005 | **0.291** |
| 2026-10-23 | — | 0.00 | 0.091 | 0.195 | 0.000 | 0.005 | **0.291** |
| 2026-10-24 | — | 0.00 | 0.091 | 0.195 | 0.000 | 0.005 | **0.291** |
| 2026-10-25 | — | 0.00 | 0.091 | 0.195 | 0.000 | 0.005 | **0.291** |
| 2026-10-26 | — | 0.00 | 0.091 | 0.195 | 0.000 | 0.005 | **0.291** |
| 2026-10-27 | — | 0.00 | 0.091 | 0.195 | 0.000 | 0.005 | **0.291** |
| 2026-10-28 | — | 0.00 | 0.091 | 0.195 | 0.000 | 0.005 | **0.291** |
| 2026-10-29 | — | 0.00 | 0.091 | 0.195 | 0.000 | 0.005 | **0.291** |
| 2026-10-30 | — | 0.00 | 0.091 | 0.195 | 0.000 | 0.005 | **0.291** |
| 2026-10-31 | — | 0.00 | 0.091 | 0.195 | 0.000 | 0.005 | **0.291** |
| 2026-11-01 | — | 0.00 | 0.091 | 0.195 | 0.000 | 0.005 | **0.291** |
| 2026-11-02 | — | 0.00 | 0.091 | 0.195 | 0.000 | 0.005 | **0.291** |
| 2026-11-03 | — | 0.00 | 0.091 | 0.195 | 0.000 | 0.005 | **0.291** |
| 2026-11-04 | — | 0.00 | 0.091 | 0.195 | 0.000 | 0.005 | **0.291** |
| 2026-11-05 | — | 0.00 | 0.091 | 0.195 | 0.000 | 0.005 | **0.291** |
| 2026-11-06 | — | 0.00 | 0.091 | 0.195 | 0.000 | 0.005 | **0.291** |
| 2026-11-07 | — | 0.00 | 0.091 | 0.195 | 0.000 | 0.005 | **0.291** |
| 2026-11-08 | — | 0.00 | 0.091 | 0.195 | 0.000 | 0.005 | **0.291** |
| 2026-11-09 | — | 0.00 | 0.091 | 0.195 | 0.000 | 0.005 | **0.291** |
| 2026-11-10 | — | 0.00 | 0.091 | 0.195 | 0.000 | 0.005 | **0.291** |
| 2026-11-11 | — | 0.00 | 0.091 | 0.195 | 0.000 | 0.005 | **0.291** |
| 2026-11-12 | — | 0.00 | 0.091 | 0.195 | 0.000 | 0.005 | **0.291** |
| 2026-11-13 | — | 0.00 | 0.091 | 0.195 | 0.000 | 0.005 | **0.291** |
| 2026-11-14 | foundry-desktop, powerdevil, python3-inators, python3-mss, python3-picire, python3-pydub | 4.89 | 0.109 | 0.195 | 0.000 | 0.005 | **0.308** |
