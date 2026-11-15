# Daily Foundry build cost simulation

**Modeled total: $10.46 over 30 days; average $0.349/day.**

Historical activity replay with current measured timing proxies

| Component | USD |
|---|---:|
| Compute + IPv4 | 2.878 |
| Retained EBS | 5.837 |
| Publication/download transfer | 0.000 |
| S3 artifact storage | 1.744 |

## Assumptions and coverage

- Current package catalog is projected backward; deleted releases and historical promotion eligibility cannot be reconstructed from current feeds.
- Per-recipe cold intervals are proxies for future changed-source builds, not measured historical runtimes.
- Current artifact sizes proxy historical uploads. Verified local source sizes are included only when include_source_uploads is enabled; metadata indexes are not sized.
- Ubuntu archive updates are outside overlay activity coverage; daily ISO still refreshes dependencies.
- Validation/startup/publication/discovery inputs are explicit scenario allowances, not measured automated stage durations.
- No automatic CPU-count timing scaling; hardware changes require explicit measured timing overrides.
- Shared account free allowances are zero by default; retention starts empty unless existing_storage_gb is supplied. Requests, taxes, failures, signing/monitoring services and website operations are excluded.

## Per-day calculation

| Date | Packages to rebuild (coalesced) | Build proxy min | Compute | EBS | Egress | S3 | Total USD |
|---|---|---:|---:|---:|---:|---:|---:|
| 2026-10-16 | powerdevil, python3-glfw | 4.62 | 0.108 | 0.195 | 0.000 | 0.004 | **0.306** |
| 2026-10-17 | libvgm, xemu-xbox | 7.37 | 0.118 | 0.195 | 0.000 | 0.008 | **0.320** |
| 2026-10-18 | — | 0.00 | 0.091 | 0.195 | 0.000 | 0.011 | **0.297** |
| 2026-10-19 | — | 0.00 | 0.091 | 0.195 | 0.000 | 0.015 | **0.300** |
| 2026-10-20 | uv, xemu-xbox | 6.99 | 0.116 | 0.195 | 0.000 | 0.019 | **0.330** |
| 2026-10-21 | — | 0.00 | 0.091 | 0.195 | 0.000 | 0.022 | **0.308** |
| 2026-10-22 | ghidra, uv, xemu-xbox | 12.12 | 0.135 | 0.195 | 0.000 | 0.027 | **0.356** |
| 2026-10-23 | ruff | 0.20 | 0.092 | 0.195 | 0.000 | 0.031 | **0.317** |
| 2026-10-24 | — | 0.00 | 0.091 | 0.195 | 0.000 | 0.034 | **0.320** |
| 2026-10-25 | — | 0.00 | 0.091 | 0.195 | 0.000 | 0.038 | **0.324** |
| 2026-10-26 | vgmstream | 1.04 | 0.095 | 0.195 | 0.000 | 0.042 | **0.331** |
| 2026-10-27 | libvgm, ruff, uv | 1.17 | 0.095 | 0.195 | 0.000 | 0.046 | **0.335** |
| 2026-10-28 | — | 0.00 | 0.091 | 0.195 | 0.000 | 0.049 | **0.335** |
| 2026-10-29 | — | 0.00 | 0.091 | 0.195 | 0.000 | 0.053 | **0.338** |
| 2026-10-30 | — | 0.00 | 0.091 | 0.195 | 0.000 | 0.057 | **0.342** |
| 2026-10-31 | — | 0.00 | 0.091 | 0.195 | 0.000 | 0.060 | **0.346** |
| 2026-11-01 | — | 0.00 | 0.091 | 0.195 | 0.000 | 0.064 | **0.349** |
| 2026-11-02 | — | 0.00 | 0.091 | 0.195 | 0.000 | 0.068 | **0.353** |
| 2026-11-03 | uv | 0.30 | 0.092 | 0.195 | 0.000 | 0.071 | **0.358** |
| 2026-11-04 | — | 0.00 | 0.091 | 0.195 | 0.000 | 0.075 | **0.361** |
| 2026-11-05 | libvgm | 0.68 | 0.093 | 0.195 | 0.000 | 0.079 | **0.367** |
| 2026-11-06 | ruff | 0.20 | 0.092 | 0.195 | 0.000 | 0.082 | **0.369** |
| 2026-11-07 | — | 0.00 | 0.091 | 0.195 | 0.000 | 0.086 | **0.372** |
| 2026-11-08 | pvsneslib | 0.53 | 0.093 | 0.195 | 0.000 | 0.090 | **0.377** |
| 2026-11-09 | — | 0.00 | 0.091 | 0.195 | 0.000 | 0.093 | **0.379** |
| 2026-11-10 | — | 0.00 | 0.091 | 0.195 | 0.000 | 0.097 | **0.383** |
| 2026-11-11 | python3-picire | 0.10 | 0.091 | 0.195 | 0.000 | 0.101 | **0.387** |
| 2026-11-12 | python3-librosa | 0.08 | 0.091 | 0.195 | 0.000 | 0.104 | **0.390** |
| 2026-11-13 | ruff, uv | 0.49 | 0.093 | 0.195 | 0.000 | 0.108 | **0.395** |
| 2026-11-14 | foundry-desktop, powerdevil, python3-inators, python3-mss, python3-picire, python3-pydub | 4.89 | 0.109 | 0.195 | 0.000 | 0.112 | **0.415** |
