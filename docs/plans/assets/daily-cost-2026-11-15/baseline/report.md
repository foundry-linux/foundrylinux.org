# Daily Foundry build cost simulation

**Modeled total: $25.58 over 30 days; average $0.853/day.**

Historical activity replay with current measured timing proxies

| Component | USD |
|---|---:|
| Compute + IPv4 | 2.878 |
| Retained EBS | 5.837 |
| AWS outbound transfer | 15.741 |
| R2 artifact storage | 1.124 |

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
| 2026-10-16 | powerdevil, python3-glfw | 4.62 | 0.108 | 0.195 | 0.518 | 0.002 | **0.823** |
| 2026-10-17 | libvgm, xemu-xbox | 7.37 | 0.118 | 0.195 | 0.539 | 0.005 | **0.856** |
| 2026-10-18 | — | 0.00 | 0.091 | 0.195 | 0.518 | 0.007 | **0.810** |
| 2026-10-19 | — | 0.00 | 0.091 | 0.195 | 0.518 | 0.010 | **0.813** |
| 2026-10-20 | uv, xemu-xbox | 6.99 | 0.116 | 0.195 | 0.542 | 0.012 | **0.865** |
| 2026-10-21 | — | 0.00 | 0.091 | 0.195 | 0.518 | 0.014 | **0.818** |
| 2026-10-22 | ghidra, uv, xemu-xbox | 12.12 | 0.135 | 0.195 | 0.661 | 0.017 | **1.008** |
| 2026-10-23 | ruff | 0.20 | 0.092 | 0.195 | 0.520 | 0.020 | **0.826** |
| 2026-10-24 | — | 0.00 | 0.091 | 0.195 | 0.518 | 0.022 | **0.825** |
| 2026-10-25 | — | 0.00 | 0.091 | 0.195 | 0.518 | 0.025 | **0.828** |
| 2026-10-26 | vgmstream | 1.04 | 0.095 | 0.195 | 0.519 | 0.027 | **0.835** |
| 2026-10-27 | libvgm, ruff, uv | 1.17 | 0.095 | 0.195 | 0.524 | 0.029 | **0.843** |
| 2026-10-28 | — | 0.00 | 0.091 | 0.195 | 0.518 | 0.032 | **0.835** |
| 2026-10-29 | — | 0.00 | 0.091 | 0.195 | 0.518 | 0.034 | **0.837** |
| 2026-10-30 | — | 0.00 | 0.091 | 0.195 | 0.518 | 0.036 | **0.840** |
| 2026-10-31 | — | 0.00 | 0.091 | 0.195 | 0.518 | 0.039 | **0.842** |
| 2026-11-01 | — | 0.00 | 0.091 | 0.195 | 0.518 | 0.041 | **0.844** |
| 2026-11-02 | — | 0.00 | 0.091 | 0.195 | 0.518 | 0.044 | **0.847** |
| 2026-11-03 | uv | 0.30 | 0.092 | 0.195 | 0.521 | 0.046 | **0.854** |
| 2026-11-04 | — | 0.00 | 0.091 | 0.195 | 0.518 | 0.048 | **0.851** |
| 2026-11-05 | libvgm | 0.68 | 0.093 | 0.195 | 0.518 | 0.051 | **0.857** |
| 2026-11-06 | ruff | 0.20 | 0.092 | 0.195 | 0.520 | 0.053 | **0.859** |
| 2026-11-07 | — | 0.00 | 0.091 | 0.195 | 0.518 | 0.055 | **0.859** |
| 2026-11-08 | pvsneslib | 0.53 | 0.093 | 0.195 | 0.519 | 0.058 | **0.865** |
| 2026-11-09 | — | 0.00 | 0.091 | 0.195 | 0.518 | 0.060 | **0.863** |
| 2026-11-10 | — | 0.00 | 0.091 | 0.195 | 0.518 | 0.063 | **0.866** |
| 2026-11-11 | python3-picire | 0.10 | 0.091 | 0.195 | 0.518 | 0.065 | **0.868** |
| 2026-11-12 | python3-librosa | 0.08 | 0.091 | 0.195 | 0.518 | 0.067 | **0.871** |
| 2026-11-13 | ruff, uv | 0.49 | 0.093 | 0.195 | 0.523 | 0.070 | **0.880** |
| 2026-11-14 | foundry-desktop, powerdevil, python3-inators, python3-mss, python3-picire, python3-pydub | 4.89 | 0.109 | 0.195 | 0.518 | 0.072 | **0.893** |
