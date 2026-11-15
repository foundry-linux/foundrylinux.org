# Daily Foundry build costs from 30 days of activity

## AWS cleanup completed — 2026-11-15

Both spike instances were terminated and all four EBS volumes (364 GiB total) deleted. Temporary spike IAM roles/profiles, security groups and any remaining receipted stop schedules were removed. The final API check found no spike-tagged EBS volumes; audit also found no spike-tagged snapshots or Elastic IPs. [Cleanup receipt](assets/aws-spike-2026-11-15/cleanup.json).

The prior approximately $33.20/month EBS retention charge no longer accrues for these deleted disks; usage already incurred remains payable. Local screenshots, inventories, comparison results and committed normalized measurements remain. Cloud-only raw logs, caches and experimental build artifacts were disposable and removed with the disks. No production repository, public release, shared VPC/subnet or home-lab infrastructure was changed.


## Closed — rejected for public download cost, 2026-11-15

Will closed the AWS/S3 deployment option because direct S3 ISO downloads are too expensive. Latest-only retention is corrected: keep one validated ISO, replacing the previous ISO after verification. This reduces storage but does not reduce bandwidth per download: approximately $482 for 1,000 complete measured-ISO downloads, or $471 if the shared monthly 100-GB allowance is available, before requests/taxes.

- [x] Correct the storage model and record the decision to reject AWS/S3 production deployment.
- [x] Cancel the proposed 2026-11-16 AWS 4-GiB follow-up trial; no replacement AWS run is scheduled.
- [x] Hand production planning to the [home-lab/R2 deployment plan](../../../home-lab/docs/plans/2026-11-15-foundry-daily-builds-on-homelab.md).

Existing measurements, scripts and unmet validation gates below are preserved as historical evidence, not active AWS tasks or claims of completed proof. The subsequently authorized AWS cleanup is complete, as recorded above. Resource-retention statements in the historical evidence below describe the earlier experiment, not current infrastructure.


**Evidence window:** 16 October–14 November 2026, Asia/Bangkok. **Corrected publication architecture:** EC2 builds and S3 Standard artifact storage are both in Mumbai (`ap-south-1`). Publication no longer copies AWS-built artifacts to R2.

**Build and publication estimate: $8.87 per 30 days ($0.30/day), keeping one validated ISO and the current package/source catalog.** These totals exclude public download traffic and request charges. The earlier $25.58 estimate incorrectly assumed uploading every daily build out of AWS to R2; it is superseded.

![Daily cost components](assets/daily-cost-2026-11-15/daily-costs.png)

[Daily CSV](assets/daily-cost-2026-11-15/baseline/daily.csv) · [Simulation JSON](assets/daily-cost-2026-11-15/baseline/simulation.json) · [Reusable script instructions](../../scripts/aws-spike/DAILY-COST-HISTORY.md).

## Publication and downloads

```mermaid
flowchart LR
    E[EC2 build and validation in Mumbai] -->|Same-region transfer: $0| S[S3 Standard in Mumbai]
    S --> D[Public package and ISO downloads]
    W[wald3n.com/open-source release records] -->|Versioned artifact links| D
```

Use immutable versioned S3 object keys for packages, sources, repository snapshots, ISOs, checksums and validation receipts. Publish tested candidates by updating the release manifest and signed APT metadata. Keep the website release history at wald3n.com/open-source and link to these artifacts. Existing production R2 repositories stay in place until a separately validated migration; no bucket, DNS or existing release was changed by this plan update.

EC2-to-S3 transfer within the same region is free. Use the regional S3 endpoint directly; a gateway endpoint can avoid NAT processing if private subnets are later used. Do not introduce a NAT gateway for publication. S3 upload requests and multipart requests still have request charges. Public S3 downloads incur internet delivery and GET charges; a custom HTTPS/CDN front end is a separate delivery decision, with its own traffic pricing. [AWS transfer FAQ](https://aws.amazon.com/s3/faqs/), [S3 pricing](https://aws.amazon.com/s3/pricing/).

## Cost breakdown

| Component | Average/day | First 30 days |
|---|---:|---:|
| Compute + public IPv4 | $0.096 | $2.88 |
| Retained 64-GiB EBS | $0.195 | $5.84 |
| EC2 → S3 publication transfer | $0 | **$0** |
| S3 Standard, one ISO plus current package/source catalog | $0.005 | $0.16 |
| **Build/publication subtotal** | **$0.296** | **$8.87** |

Mumbai S3 Standard costs $0.025 per binary GB-month for the first 50 TB; the [Pricing API receipt](assets/daily-cost-2026-11-15/aws-s3-rate.json) is saved. Storage uses 2^30 bytes per billed GB. One measured ISO is 4,736,024,576 bytes; the current binary/source catalog adds 1,933,081,528 bytes. Total retained payload is about **6.67 decimal GB / 6.21 binary GB**, costing **$0.155/month** at the saved S3 rate, before billing rounding and temporary promotion overlap. This replaces the earlier 30-daily-ISO retention assumption. A full current catalog is included even for unchanged recipes. Existing unrelated repository content, package upgrade/source-license retention and noncurrent object versions add storage if required.

Keep the last good ISO until its replacement passes validation and public verification, then retire its bytes. Preserve dated release metadata; do not promise permanent downloads of retired ISOs. Publish immutable candidate names and update the latest manifest after verification. Configure cleanup for old ISO versions and failed multipart uploads; do not use versioning to retain hidden copies indefinitely. No existing bucket objects were deleted by this estimate change.

### Public download sensitivity

Downloads depend on users, not build frequency. At the saved Mumbai internet-transfer planning rate of $0.1093/GB, the following are additions to the subtotal, before GET/CDN request fees. The 100-GB allowance is account-wide and conditional on remaining availability; it is applied once per billing month, not once per bucket.

| Public downloads/month, billing GB | Gross download transfer | If 100 GB allowance is available |
|---|---:|---:|
| 0 | $0 | $0 |
| 100 | $10.93 | $0 |
| 500 | $54.65 | $43.72 |
| 1,000 | $109.30 | $98.37 |

These are direct-S3 delivery scenarios, not assumed audience traffic or a CDN quote. AWS/S3 requests, taxes, failures, signing/monitoring services, website operation, initial cold bootstrap and existing repository storage remain outside the subtotal. Separate retained diagnostic 300-GiB EBS volumes still add $27.36/month while kept; they are excluded from the daily production model.

## If the 4-GiB build trial succeeds

| Build runtime on c7i.large | First 30-day subtotal | Monthly decrease |
|---|---:|---:|
| Same time | $7.67 | **$1.20 (13.5%)** |
| 50% longer | $8.13 | **$0.74 (8.3%)** |
| Twice as long | $8.59 | **$0.28 (3.1%)** |

The absolute savings are unchanged: c7i.large build compute/IP costs $0.09425/hour versus m7i.xlarge $0.2171/hour. Control and validation retain the larger-host rate. Break-even is about 2.30× build runtime before added transitions/transfers. With equal runtime, the subtotal falls from $8.87 to $7.67. The proposed AWS trial is canceled; these remain unproven sensitivity scenarios.

## What was measured and what is modeled

The collector covers the **58 current Foundry overlay recipes**, excluding packages marked for the separate Shared Electron repository. It does not claim to query release histories for all 2,812 packages in the ISO. Ubuntu archive updates are handled by the daily ISO refresh and are not counted as source builds by Foundry. The current recipe catalog is projected backward; historical catalog membership and deleted upstream releases are not reconstructed.

There are **36 raw dated events**: 29 upstream events and seven recipe-change events from three local commits. After the baseline's PowerDevil 6.6 maintenance-branch filter and per-package/per-day coalescing, upstream activity calls for **22 recipe rebuilds on 14 days**; repository activity calls for **six recipe rebuilds on one day**. Their union is **28 rebuilds on 15 days**. Multiple same-day uv releases, for example, become one build. These are eligible rebuild opportunities under the stated policy, not proof that Foundry historically published each release.

Stable GitHub release publication dates and PyPI upload dates are primary evidence. Five PowerDevil tags lack GitHub release records and have tag/commit-date proxies; the four 6.7/6.8 branch tags are retained in the JSON but excluded from the baseline. The 6.6.7 tag-date proxy remains. Four libvgm commits coalesce into three snapshot-build days. Snapshot streams for xemu and drmon have no events in this window. All API queries completed; “no events” describes the current feed/policy, not proof that no upstream work happened elsewhere.

The 58 build intervals come from timestamped cold-run Docker logs and total 5,089 seconds, versus the 5,095-second full package stage. Each interval includes source preparation, dependency work, compile/test and packaging until the next recipe begins. Changed-version builds use these intervals as **timing proxies**; we did not rebuild every historical version. Binary sizes are measured AWS artifacts. Source sizes are separately verified local DSC inventories matching current recipe versions; all 58 passed size/SHA256 checks. They proxy historical/future payload sizes. Upstream changes upload new source archives; repo-only revisions reuse existing orig archives.

The model assumes correct per-recipe selective reuse. The experiment proved whole-set unchanged-output reuse; per-recipe fingerprints now have container fixture and a real package cache-hit check, while full changed-source and promotion validation remain pending. No hypothetical compiler-cache or other optimization savings are included.


## Per-day table

Each row estimates one fresh ISO publication plus the selected recipe changes. Costs include the allowances above. An em dash means no selected event on that date. PowerDevil's 16 October upstream entry uses an explicitly approximate tag date; libvgm entries are snapshot commit days.

| Bangkok date | Upstream recipe updates | Repo recipe changes | Build proxy min | Compute/IP | EBS | Egress | S3 | Total USD |
|---|---|---|---:|---:|---:|---:|---:|---:|
| 2026-10-16 | powerdevil, python3-glfw | — | 4.62 | 0.108 | 0.195 | 0.000 | 0.005 | **0.307** |
| 2026-10-17 | libvgm, xemu-xbox | — | 7.37 | 0.118 | 0.195 | 0.000 | 0.005 | **0.317** |
| 2026-10-18 | — | — | 0.00 | 0.091 | 0.195 | 0.000 | 0.005 | **0.291** |
| 2026-10-19 | — | — | 0.00 | 0.091 | 0.195 | 0.000 | 0.005 | **0.291** |
| 2026-10-20 | uv, xemu-xbox | — | 6.99 | 0.116 | 0.195 | 0.000 | 0.005 | **0.316** |
| 2026-10-21 | — | — | 0.00 | 0.091 | 0.195 | 0.000 | 0.005 | **0.291** |
| 2026-10-22 | ghidra, uv, xemu-xbox | — | 12.12 | 0.135 | 0.195 | 0.000 | 0.005 | **0.335** |
| 2026-10-23 | ruff | — | 0.20 | 0.092 | 0.195 | 0.000 | 0.005 | **0.291** |
| 2026-10-24 | — | — | 0.00 | 0.091 | 0.195 | 0.000 | 0.005 | **0.291** |
| 2026-10-25 | — | — | 0.00 | 0.091 | 0.195 | 0.000 | 0.005 | **0.291** |
| 2026-10-26 | vgmstream | — | 1.04 | 0.095 | 0.195 | 0.000 | 0.005 | **0.295** |
| 2026-10-27 | libvgm, ruff, uv | — | 1.17 | 0.095 | 0.195 | 0.000 | 0.005 | **0.295** |
| 2026-10-28 | — | — | 0.00 | 0.091 | 0.195 | 0.000 | 0.005 | **0.291** |
| 2026-10-29 | — | — | 0.00 | 0.091 | 0.195 | 0.000 | 0.005 | **0.291** |
| 2026-10-30 | — | — | 0.00 | 0.091 | 0.195 | 0.000 | 0.005 | **0.291** |
| 2026-10-31 | — | — | 0.00 | 0.091 | 0.195 | 0.000 | 0.005 | **0.291** |
| 2026-11-01 | — | — | 0.00 | 0.091 | 0.195 | 0.000 | 0.005 | **0.291** |
| 2026-11-02 | — | — | 0.00 | 0.091 | 0.195 | 0.000 | 0.005 | **0.291** |
| 2026-11-03 | uv | — | 0.30 | 0.092 | 0.195 | 0.000 | 0.005 | **0.292** |
| 2026-11-04 | — | — | 0.00 | 0.091 | 0.195 | 0.000 | 0.005 | **0.291** |
| 2026-11-05 | libvgm | — | 0.68 | 0.093 | 0.195 | 0.000 | 0.005 | **0.293** |
| 2026-11-06 | ruff | — | 0.20 | 0.092 | 0.195 | 0.000 | 0.005 | **0.291** |
| 2026-11-07 | — | — | 0.00 | 0.091 | 0.195 | 0.000 | 0.005 | **0.291** |
| 2026-11-08 | pvsneslib | — | 0.53 | 0.093 | 0.195 | 0.000 | 0.005 | **0.293** |
| 2026-11-09 | — | — | 0.00 | 0.091 | 0.195 | 0.000 | 0.005 | **0.291** |
| 2026-11-10 | — | — | 0.00 | 0.091 | 0.195 | 0.000 | 0.005 | **0.291** |
| 2026-11-11 | python3-picire | — | 0.10 | 0.091 | 0.195 | 0.000 | 0.005 | **0.291** |
| 2026-11-12 | python3-librosa | — | 0.08 | 0.091 | 0.195 | 0.000 | 0.005 | **0.291** |
| 2026-11-13 | ruff, uv | — | 0.49 | 0.093 | 0.195 | 0.000 | 0.005 | **0.293** |
| 2026-11-14 | — | foundry-desktop, powerdevil, python3-inators, python3-mss, python3-picire, python3-pydub | 4.89 | 0.109 | 0.195 | 0.000 | 0.005 | **0.308** |


## Reproducibility and assumptions

The simulator saves activity, measurements and policy/rates separately. It uses the measured 1,089-second warm workflow each day and cold per-recipe intervals for changed packages. Startup/discovery/validation/publication total seven minutes/day of explicit unmeasured allowances. Compile-cache and ISO optimization savings await measurement. No new AWS build was run for this correction.

All scenarios and graphs have been regenerated for latest-only S3 retention. `egress_per_gb=0` models same-region publication. `public_download_gb_per_day` and `public_download_per_gb` model direct public delivery independently; `available_egress_free_gb_per_calendar_month` applies to those downloads. Storage keys are provider-neutral: `storage_per_gb_month`, `storage_bytes_per_gb`, `existing_storage_gb`.
