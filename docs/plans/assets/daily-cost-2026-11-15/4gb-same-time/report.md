# Daily Foundry build cost simulation

**Modeled total: $7.67 over 30 days; average $0.256/day.**

Historical activity replay with current measured timing proxies

| Component | USD |
|---|---:|
| Compute + IPv4 | 1.679 |
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
| 2026-10-16 | powerdevil, python3-glfw | 4.62 | 0.061 | 0.195 | 0.000 | 0.005 | **0.261** |
| 2026-10-17 | libvgm, xemu-xbox | 7.37 | 0.065 | 0.195 | 0.000 | 0.005 | **0.265** |
| 2026-10-18 | — | 0.00 | 0.054 | 0.195 | 0.000 | 0.005 | **0.254** |
| 2026-10-19 | — | 0.00 | 0.054 | 0.195 | 0.000 | 0.005 | **0.254** |
| 2026-10-20 | uv, xemu-xbox | 6.99 | 0.065 | 0.195 | 0.000 | 0.005 | **0.265** |
| 2026-10-21 | — | 0.00 | 0.054 | 0.195 | 0.000 | 0.005 | **0.254** |
| 2026-10-22 | ghidra, uv, xemu-xbox | 12.12 | 0.073 | 0.195 | 0.000 | 0.005 | **0.273** |
| 2026-10-23 | ruff | 0.20 | 0.054 | 0.195 | 0.000 | 0.005 | **0.254** |
| 2026-10-24 | — | 0.00 | 0.054 | 0.195 | 0.000 | 0.005 | **0.254** |
| 2026-10-25 | — | 0.00 | 0.054 | 0.195 | 0.000 | 0.005 | **0.254** |
| 2026-10-26 | vgmstream | 1.04 | 0.055 | 0.195 | 0.000 | 0.005 | **0.255** |
| 2026-10-27 | libvgm, ruff, uv | 1.17 | 0.056 | 0.195 | 0.000 | 0.005 | **0.255** |
| 2026-10-28 | — | 0.00 | 0.054 | 0.195 | 0.000 | 0.005 | **0.254** |
| 2026-10-29 | — | 0.00 | 0.054 | 0.195 | 0.000 | 0.005 | **0.254** |
| 2026-10-30 | — | 0.00 | 0.054 | 0.195 | 0.000 | 0.005 | **0.254** |
| 2026-10-31 | — | 0.00 | 0.054 | 0.195 | 0.000 | 0.005 | **0.254** |
| 2026-11-01 | — | 0.00 | 0.054 | 0.195 | 0.000 | 0.005 | **0.254** |
| 2026-11-02 | — | 0.00 | 0.054 | 0.195 | 0.000 | 0.005 | **0.254** |
| 2026-11-03 | uv | 0.30 | 0.054 | 0.195 | 0.000 | 0.005 | **0.254** |
| 2026-11-04 | — | 0.00 | 0.054 | 0.195 | 0.000 | 0.005 | **0.254** |
| 2026-11-05 | libvgm | 0.68 | 0.055 | 0.195 | 0.000 | 0.005 | **0.255** |
| 2026-11-06 | ruff | 0.20 | 0.054 | 0.195 | 0.000 | 0.005 | **0.254** |
| 2026-11-07 | — | 0.00 | 0.054 | 0.195 | 0.000 | 0.005 | **0.254** |
| 2026-11-08 | pvsneslib | 0.53 | 0.055 | 0.195 | 0.000 | 0.005 | **0.254** |
| 2026-11-09 | — | 0.00 | 0.054 | 0.195 | 0.000 | 0.005 | **0.254** |
| 2026-11-10 | — | 0.00 | 0.054 | 0.195 | 0.000 | 0.005 | **0.254** |
| 2026-11-11 | python3-picire | 0.10 | 0.054 | 0.195 | 0.000 | 0.005 | **0.254** |
| 2026-11-12 | python3-librosa | 0.08 | 0.054 | 0.195 | 0.000 | 0.005 | **0.254** |
| 2026-11-13 | ruff, uv | 0.49 | 0.055 | 0.195 | 0.000 | 0.005 | **0.254** |
| 2026-11-14 | foundry-desktop, powerdevil, python3-inators, python3-mss, python3-picire, python3-pydub | 4.89 | 0.062 | 0.195 | 0.000 | 0.005 | **0.261** |
