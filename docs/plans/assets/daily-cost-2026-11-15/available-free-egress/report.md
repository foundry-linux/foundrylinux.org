# Daily Foundry build cost simulation

**Modeled total: $54.18 over 30 days; average $1.806/day.**

Historical activity replay with current measured timing proxies

| Component | USD |
|---|---:|
| Compute + IPv4 | 2.878 |
| Retained EBS | 5.837 |
| Publication/download transfer | 43.720 |
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
| 2026-12-01 | powerdevil, python3-glfw | 4.62 | 0.108 | 0.195 | 0.000 | 0.004 | **0.306** |
| 2026-12-02 | libvgm, xemu-xbox | 7.37 | 0.118 | 0.195 | 0.000 | 0.008 | **0.320** |
| 2026-12-03 | — | 0.00 | 0.091 | 0.195 | 0.000 | 0.011 | **0.297** |
| 2026-12-04 | — | 0.00 | 0.091 | 0.195 | 0.000 | 0.015 | **0.300** |
| 2026-12-05 | uv, xemu-xbox | 6.99 | 0.116 | 0.195 | 0.000 | 0.019 | **0.330** |
| 2026-12-06 | — | 0.00 | 0.091 | 0.195 | 0.000 | 0.022 | **0.308** |
| 2026-12-07 | ghidra, uv, xemu-xbox | 12.12 | 0.135 | 0.195 | 1.822 | 0.027 | **2.178** |
| 2026-12-08 | ruff | 0.20 | 0.092 | 0.195 | 1.822 | 0.031 | **2.139** |
| 2026-12-09 | — | 0.00 | 0.091 | 0.195 | 1.822 | 0.034 | **2.142** |
| 2026-12-10 | — | 0.00 | 0.091 | 0.195 | 1.822 | 0.038 | **2.145** |
| 2026-12-11 | vgmstream | 1.04 | 0.095 | 0.195 | 1.822 | 0.042 | **2.153** |
| 2026-12-12 | libvgm, ruff, uv | 1.17 | 0.095 | 0.195 | 1.822 | 0.046 | **2.157** |
| 2026-12-13 | — | 0.00 | 0.091 | 0.195 | 1.822 | 0.049 | **2.156** |
| 2026-12-14 | — | 0.00 | 0.091 | 0.195 | 1.822 | 0.053 | **2.160** |
| 2026-12-15 | — | 0.00 | 0.091 | 0.195 | 1.822 | 0.057 | **2.164** |
| 2026-12-16 | — | 0.00 | 0.091 | 0.195 | 1.822 | 0.060 | **2.167** |
| 2026-12-17 | — | 0.00 | 0.091 | 0.195 | 1.822 | 0.064 | **2.171** |
| 2026-12-18 | — | 0.00 | 0.091 | 0.195 | 1.822 | 0.068 | **2.175** |
| 2026-12-19 | uv | 0.30 | 0.092 | 0.195 | 1.822 | 0.071 | **2.180** |
| 2026-12-20 | — | 0.00 | 0.091 | 0.195 | 1.822 | 0.075 | **2.182** |
| 2026-12-21 | libvgm | 0.68 | 0.093 | 0.195 | 1.822 | 0.079 | **2.188** |
| 2026-12-22 | ruff | 0.20 | 0.092 | 0.195 | 1.822 | 0.082 | **2.190** |
| 2026-12-23 | — | 0.00 | 0.091 | 0.195 | 1.822 | 0.086 | **2.193** |
| 2026-12-24 | pvsneslib | 0.53 | 0.093 | 0.195 | 1.822 | 0.090 | **2.199** |
| 2026-12-25 | — | 0.00 | 0.091 | 0.195 | 1.822 | 0.093 | **2.201** |
| 2026-12-26 | — | 0.00 | 0.091 | 0.195 | 1.822 | 0.097 | **2.204** |
| 2026-12-27 | python3-picire | 0.10 | 0.091 | 0.195 | 1.822 | 0.101 | **2.208** |
| 2026-12-28 | python3-librosa | 0.08 | 0.091 | 0.195 | 1.822 | 0.104 | **2.212** |
| 2026-12-29 | ruff, uv | 0.49 | 0.093 | 0.195 | 1.822 | 0.108 | **2.217** |
| 2026-12-30 | foundry-desktop, powerdevil, python3-inators, python3-mss, python3-picire, python3-pydub | 4.89 | 0.109 | 0.195 | 1.822 | 0.112 | **2.237** |
