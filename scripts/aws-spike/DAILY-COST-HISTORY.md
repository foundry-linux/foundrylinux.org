# Reusable daily build history and cost projections

`daily-cost-history.py` separates observed activity, observed build measurements, and scenario assumptions. It never starts AWS instances, rebuilds packages, signs or publishes artifacts. Collection needs Python 3.11+, `git`, authenticated `gh`, and network access. Simulation needs only Python and the three saved JSON files; it works offline in a container.

## Collect a reproducible activity window

From the repository root:

```sh
python3 scripts/aws-spike/daily-cost-history.py collect \
  --days 30 --end-date 2026-11-14 --timezone Asia/Bangkok \
  --cache docs/plans/assets/daily-cost-2026-11-15/api-cache \
  --output docs/plans/assets/daily-cost-2026-11-15/activity.json
```

The end date is inclusive. Events are grouped into local calendar days; this is not a replay of the proposed 03:00 job. The collector discovers the current `foundry-apt/packages` catalog and excludes `.shared-electron-only` recipes. That current catalog is projected backward; it does not reconstruct historic catalog membership. Ubuntu archive dependencies are not individually queried. Recipes without a separate upstream use repository commits.

GitHub release `publishedAt` and PyPI file upload dates are preferred. Matching tags with no release record become `tag_proxy` events: annotated-tag or target-commit dates are **not proven publication dates**. The watch-specific WorldFoundry prefixes are preserved. libvgm, xemu and drmon are explicitly treated as snapshot streams; their default-branch commit days are prospective rebuild opportunities, not a claim that Foundry published every commit. Draft/prerelease feeds are excluded. Unknown/private/unavailable feeds are recorded as unknown, never as confirmed zero activity.

Each raw API response includes its request and retrieval timestamp in a SHA256-keyed cache file. Re-running an identical request reuses evidence younger than 24 hours; older responses refresh automatically. `--cache-max-age-hours` changes that limit, and `--refresh` explicitly replaces cached responses. Simulation never refreshes or accesses APIs. A changed date window requests new snapshot history but can reuse release pages. Increase `--max-pages` if a feed exceeds the default 20-page bound. Collection errors persist as explicit coverage gaps. Git committer dates determine local recipe-change activity.

## Simulate from saved data without network

```sh
docker run --rm --network none \
  -v "$PWD:/work" -w /work --entrypoint python3 \
  foundry-apt-builder:ubuntu-26.04 scripts/aws-spike/daily-cost-history.py simulate \
  --history docs/plans/assets/daily-cost-2026-11-15/activity.json \
  --measurements docs/plans/assets/daily-cost-2026-11-15/build-measurements.json \
  --scenario scripts/aws-spike/daily-cost-baseline.json \
  --output docs/plans/assets/daily-cost-2026-11-15/baseline
```

Outputs are `simulation.json`, `daily.csv` and `report.md`. Each package is rebuilt at most once per modeled day, even if multiple releases or both upstream and local events occur. Baseline always rebuilds Anvil daily, including no-change days. `activity` selects `both`, `upstream` or `repo`; proxy tags and snapshot commit streams can be excluded independently. `upstream_version_prefixes` can constrain a maintenance branch: the baseline keeps PowerDevil on `v6.6.` while preserving later branch events in the history. Local recipe fixes are not suppressed by this upstream version filter. Daily costs use per-recipe cold measurements as **estimates**, not claims about historical runtime. Missing measurements produce warnings and leave the unknown component unpriced; such totals are incomplete.

## Future scenarios and projections

Copy the scenario JSON and change its inputs, keeping the original measured baseline. `recipe_seconds_override` supplies explicit per-recipe new measurements; `warm_workflow_seconds` controls ISO/orchestration duration. Changing hardware price does **not** automatically scale runtime. `iso_policy: "changed"` is a separate hypothetical policy, not the default daily-ISO result.

Use `--projection-days 90` and an optional scenario `projection_start` date to repeat the recorded sequence across 90 days. This is a clearly labeled cyclic workload scenario, not a prediction of future releases. With 30-day artifact retention, the first month accumulates storage; later months show steady retention. Existing published storage can be included through `existing_storage_gb`.

Same-region EC2-to-S3 publication uses `egress_per_gb: 0`. Public downloads use `public_download_gb_per_day` and `public_download_per_gb`, defaulting to zero traffic. The default has no available internet egress free allowance. If known account-wide allowance is available, set `available_egress_free_gb_per_calendar_month`; it resets by calendar month, including a historical window spanning two months. The calculator does not assume that AWS account credits are reserved for Foundry. S3 storage uses `storage_bytes_per_gb: 1073741824`, with Mumbai Standard at $0.025/GB-month. Storage uses the configured planning month length (default 30 days).

Startup, discovery, automated visual validation and publication durations are editable **allowances** because the spike's manually supervised wall-clock intervals do not establish automated timings. Current binary `.deb`, verified local source archives and ISO sizes proxy future uploads. `include_source_uploads` enables the source sizes; repo-only revisions reuse existing orig archives, while upstream changes count new orig archives. Metadata indexes, request charges, failures, taxes and publishing/monitoring services remain excluded unless modeled separately. Legacy diagnostic disks are separate from the 64-GiB daily-runner scenario.

## Measurement provenance

`build-measurements.json` was extracted from `docker logs --timestamps foundry-spike-packages-cold` on the retained cold-run instance, without rebuilding. A recipe interval starts at its `=== Running ...` / `=== Building ...` marker and ends at the next recipe marker (the final recipe ends at the last Docker log timestamp). This includes preparation, dependency fetch/install, compile, test and packaging; it is not pure compiler time. The 58 intervals total about 5,089 seconds versus the recorded 5,095-second full package stage. Their durations apply to this sequential order and builder environment; isolated builds can have different dependency overhead.


To refresh source-size evidence without AWS, use `import-sizes --measurements <raw-measurements.json> --dist foundry-apt/dist --output <enriched-measurements.json>` in the container. This matches each current recipe version to a local DSC, verifies listed SHA256 and byte sizes, and saves source-file metadata and coverage. It does not rebuild or upload. Preserve the raw timing receipt separately; local source files may differ from the timed AWS build and are labeled as size proxies. Missing or mismatched files become explicit unpriced coverage warnings, not zero cost.


Stage-specific hardware scenarios use `build_compute_per_hour`, `build_ipv4_per_hour`, and `build_runtime_multiplier`. They apply only to the ISO workflow and changed-package build intervals. Discovery, startup, validation and publication retain `compute_per_hour`/`ipv4_per_hour`. Leaving build-specific keys absent reproduces the baseline. Runtime multipliers are explicit what-if assumptions until measured; a cheaper host is never assumed to have equal speed automatically. Separate build/test hosts can add transfer/startup work beyond these allowances; the sensitivity table is not proof that a 4-GiB host can run the validation guest.
