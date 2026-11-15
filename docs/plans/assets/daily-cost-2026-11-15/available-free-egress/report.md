# Daily Foundry build cost simulation

**Modeled total: $14.65 over 30 days; average $0.488/day.**

Historical activity replay with current measured timing proxies

| Component | USD |
|---|---:|
| Compute + IPv4 | 2.878 |
| Retained EBS | 5.837 |
| AWS outbound transfer | 4.811 |
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
| 2026-12-01 | powerdevil, python3-glfw | 4.62 | 0.108 | 0.195 | 0.000 | 0.002 | **0.305** |
| 2026-12-02 | libvgm, xemu-xbox | 7.37 | 0.118 | 0.195 | 0.000 | 0.005 | **0.317** |
| 2026-12-03 | — | 0.00 | 0.091 | 0.195 | 0.000 | 0.007 | **0.293** |
| 2026-12-04 | — | 0.00 | 0.091 | 0.195 | 0.000 | 0.010 | **0.295** |
| 2026-12-05 | uv, xemu-xbox | 6.99 | 0.116 | 0.195 | 0.000 | 0.012 | **0.323** |
| 2026-12-06 | — | 0.00 | 0.091 | 0.195 | 0.000 | 0.014 | **0.300** |
| 2026-12-07 | ghidra, uv, xemu-xbox | 12.12 | 0.135 | 0.195 | 0.000 | 0.017 | **0.347** |
| 2026-12-08 | ruff | 0.20 | 0.092 | 0.195 | 0.000 | 0.020 | **0.306** |
| 2026-12-09 | — | 0.00 | 0.091 | 0.195 | 0.000 | 0.022 | **0.308** |
| 2026-12-10 | — | 0.00 | 0.091 | 0.195 | 0.000 | 0.025 | **0.310** |
| 2026-12-11 | vgmstream | 1.04 | 0.095 | 0.195 | 0.000 | 0.027 | **0.316** |
| 2026-12-12 | libvgm, ruff, uv | 1.17 | 0.095 | 0.195 | 0.000 | 0.029 | **0.319** |
| 2026-12-13 | — | 0.00 | 0.091 | 0.195 | 0.000 | 0.032 | **0.317** |
| 2026-12-14 | — | 0.00 | 0.091 | 0.195 | 0.000 | 0.034 | **0.320** |
| 2026-12-15 | — | 0.00 | 0.091 | 0.195 | 0.000 | 0.036 | **0.322** |
| 2026-12-16 | — | 0.00 | 0.091 | 0.195 | 0.000 | 0.039 | **0.324** |
| 2026-12-17 | — | 0.00 | 0.091 | 0.195 | 0.000 | 0.041 | **0.327** |
| 2026-12-18 | — | 0.00 | 0.091 | 0.195 | 0.000 | 0.044 | **0.329** |
| 2026-12-19 | uv | 0.30 | 0.092 | 0.195 | 0.000 | 0.046 | **0.333** |
| 2026-12-20 | — | 0.00 | 0.091 | 0.195 | 0.000 | 0.048 | **0.334** |
| 2026-12-21 | libvgm | 0.68 | 0.093 | 0.195 | 0.142 | 0.051 | **0.481** |
| 2026-12-22 | ruff | 0.20 | 0.092 | 0.195 | 0.520 | 0.053 | **0.859** |
| 2026-12-23 | — | 0.00 | 0.091 | 0.195 | 0.518 | 0.055 | **0.859** |
| 2026-12-24 | pvsneslib | 0.53 | 0.093 | 0.195 | 0.519 | 0.058 | **0.865** |
| 2026-12-25 | — | 0.00 | 0.091 | 0.195 | 0.518 | 0.060 | **0.863** |
| 2026-12-26 | — | 0.00 | 0.091 | 0.195 | 0.518 | 0.063 | **0.866** |
| 2026-12-27 | python3-picire | 0.10 | 0.091 | 0.195 | 0.518 | 0.065 | **0.868** |
| 2026-12-28 | python3-librosa | 0.08 | 0.091 | 0.195 | 0.518 | 0.067 | **0.871** |
| 2026-12-29 | ruff, uv | 0.49 | 0.093 | 0.195 | 0.523 | 0.070 | **0.880** |
| 2026-12-30 | foundry-desktop, powerdevil, python3-inators, python3-mss, python3-picire, python3-pydub | 4.89 | 0.109 | 0.195 | 0.518 | 0.072 | **0.893** |
