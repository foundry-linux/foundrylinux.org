# Daily Foundry build cost simulation

**Modeled total: $25.22 over 30 days; average $0.841/day.**

Historical activity replay with current measured timing proxies

| Component | USD |
|---|---:|
| Compute + IPv4 | 2.748 |
| Retained EBS | 5.837 |
| AWS outbound transfer | 15.530 |
| R2 artifact storage | 1.101 |

## Assumptions and coverage

- Current package catalog is projected backward; deleted releases and historical promotion eligibility cannot be reconstructed from current feeds.
- Per-recipe cold intervals are proxies for future changed-source builds, not measured historical runtimes.
- Current artifact sizes proxy historical uploads. Verified local source sizes are included only when include_source_uploads is enabled; metadata indexes are not sized.
- Ubuntu archive updates are outside overlay activity coverage; daily ISO still refreshes dependencies.
- Validation/startup/publication/discovery inputs are explicit scenario allowances, not measured automated stage durations.
- No automatic CPU-count timing scaling; hardware changes require explicit measured timing overrides.
- Shared account free allowances are zero by default; retention starts empty unless existing_r2_gb is supplied. Requests, taxes, failures, signing/monitoring services and website operations are excluded.

## Per-day calculation

| Date | Packages to rebuild (coalesced) | Build proxy min | Compute | EBS | Egress | R2 | Total USD |
|---|---|---:|---:|---:|---:|---:|---:|
| 2026-10-16 | — | 0.00 | 0.091 | 0.195 | 0.518 | 0.002 | **0.806** |
| 2026-10-17 | — | 0.00 | 0.091 | 0.195 | 0.518 | 0.005 | **0.808** |
| 2026-10-18 | — | 0.00 | 0.091 | 0.195 | 0.518 | 0.007 | **0.810** |
| 2026-10-19 | — | 0.00 | 0.091 | 0.195 | 0.518 | 0.009 | **0.813** |
| 2026-10-20 | — | 0.00 | 0.091 | 0.195 | 0.518 | 0.012 | **0.815** |
| 2026-10-21 | — | 0.00 | 0.091 | 0.195 | 0.518 | 0.014 | **0.817** |
| 2026-10-22 | — | 0.00 | 0.091 | 0.195 | 0.518 | 0.017 | **0.820** |
| 2026-10-23 | — | 0.00 | 0.091 | 0.195 | 0.518 | 0.019 | **0.822** |
| 2026-10-24 | — | 0.00 | 0.091 | 0.195 | 0.518 | 0.021 | **0.825** |
| 2026-10-25 | — | 0.00 | 0.091 | 0.195 | 0.518 | 0.024 | **0.827** |
| 2026-10-26 | — | 0.00 | 0.091 | 0.195 | 0.518 | 0.026 | **0.829** |
| 2026-10-27 | — | 0.00 | 0.091 | 0.195 | 0.518 | 0.028 | **0.832** |
| 2026-10-28 | — | 0.00 | 0.091 | 0.195 | 0.518 | 0.031 | **0.834** |
| 2026-10-29 | — | 0.00 | 0.091 | 0.195 | 0.518 | 0.033 | **0.836** |
| 2026-10-30 | — | 0.00 | 0.091 | 0.195 | 0.518 | 0.036 | **0.839** |
| 2026-10-31 | — | 0.00 | 0.091 | 0.195 | 0.518 | 0.038 | **0.841** |
| 2026-11-01 | — | 0.00 | 0.091 | 0.195 | 0.518 | 0.040 | **0.843** |
| 2026-11-02 | — | 0.00 | 0.091 | 0.195 | 0.518 | 0.043 | **0.846** |
| 2026-11-03 | — | 0.00 | 0.091 | 0.195 | 0.518 | 0.045 | **0.848** |
| 2026-11-04 | — | 0.00 | 0.091 | 0.195 | 0.518 | 0.047 | **0.851** |
| 2026-11-05 | — | 0.00 | 0.091 | 0.195 | 0.518 | 0.050 | **0.853** |
| 2026-11-06 | — | 0.00 | 0.091 | 0.195 | 0.518 | 0.052 | **0.855** |
| 2026-11-07 | — | 0.00 | 0.091 | 0.195 | 0.518 | 0.054 | **0.858** |
| 2026-11-08 | — | 0.00 | 0.091 | 0.195 | 0.518 | 0.057 | **0.860** |
| 2026-11-09 | — | 0.00 | 0.091 | 0.195 | 0.518 | 0.059 | **0.862** |
| 2026-11-10 | — | 0.00 | 0.091 | 0.195 | 0.518 | 0.062 | **0.865** |
| 2026-11-11 | — | 0.00 | 0.091 | 0.195 | 0.518 | 0.064 | **0.867** |
| 2026-11-12 | — | 0.00 | 0.091 | 0.195 | 0.518 | 0.066 | **0.870** |
| 2026-11-13 | — | 0.00 | 0.091 | 0.195 | 0.518 | 0.069 | **0.872** |
| 2026-11-14 | foundry-desktop, powerdevil, python3-inators, python3-mss, python3-picire, python3-pydub | 4.89 | 0.109 | 0.195 | 0.518 | 0.071 | **0.892** |
