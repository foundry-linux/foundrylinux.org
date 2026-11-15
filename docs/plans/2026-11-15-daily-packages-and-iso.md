# Daily Foundry package and ISO updates

**Date:** 2026-11-15
**Status:** Proposal for Will’s review; scheduling and publication are not enabled by this document.

## Outcome

Check for updates every day, publish tested package updates, and produce a fresh Anvil ISO from the resulting repository snapshot. Publish a dated record of **every package and ISO update under https://wald3n.com/open-source**, with permanent links, downloads, checksums, change summaries and validation results. A run is complete only after the artifacts and their public update records are verified live.

Use local/lab container builds and independently runnable publishing commands. GitHub remains a Git remote; GitHub Actions is excluded. Will agreed on 2026-11-15 to a dedicated build VM on an always-on home-lab machine, with builds running in containers and a systemd timer inside the VM; the same Task commands can become Kubernetes jobs when the lab scheduler exists. An always-on host is required for a dependable daily service; a sleeping or disconnected laptop can only catch up later.

## Evidence and existing work

- [`scripts/refresh-distro.sh`](../../scripts/refresh-distro.sh) and `task distro-refresh` rebuild pinned Foundry packages and the ISO against current APT indexes, but deliberately stop before signing and publishing. Rebuilding a pinned version does not discover a new upstream release.
- [`Taskfile.yml`](../../Taskfile.yml) has ISO build, signing and upload tasks. Refactor their version allocation and cache checks for unattended runs: one version per run, external APT changes included in freshness decisions, no writes to Will’s active checkout.
- [`foundry-apt/scripts/publish-local.sh`](../../foundry-apt/scripts/publish-local.sh) supports signing but defaults to unsigned output without a key. Production automation must refuse missing signing credentials and avoid exposing a partial repository during replacement.
- [`foundry-iso/test/boot-smoke.sh`](../../foundry-iso/test/boot-smoke.sh) currently accepts a QEMU process surviving its timeout. That is insufficient evidence of a working desktop.
- Anvil 0.9.140 was built in a container, its PowerDevil version checked, and its KDE desktop observed in a UEFI VM. Its USB copy passed checksum verification. This is a manual baseline, not an automated daily service.
- The sibling `../wald3n.com/` already owns `src/data/open-source.json`, `scripts/open-source-sources.json`, `task open-source:refresh`, `task open-source:verify`, and `task open-source:publish`. Its Foundry source root is the canonical `../foundrylinux.org/foundry-apt/packages`.
- The site publisher uses an isolated checkout, commits, pushes, deploys and checks the live site. Its current live status checks focus on compiler manifest/graph hashes; extend them to package and ISO release records. Preserve the existing compiler dashboard.
- [`scripts/complete-package-publish.sh`](../../scripts/complete-package-publish.sh) currently checks package presence on the public page. Require exact versions, artifact hashes and release IDs; name presence alone cannot prove an update is live. Carry forward the [publication completion plan](2026-08-06-package-publish-pipeline-hardening.md).

## Daily schedule and update policy

Agreed schedule (Will, 2026-11-15): **03:00 Asia/Bangkok each day**, with a persistent timer to catch up after downtime. Lock the entire run, bound retries and runtime, and report a skipped overlapping run. Never mark a failed or missed run green.

| Input | Daily action | Publication gate |
|---|---|---|
| Ubuntu 26.04 packages | Refresh authenticated indexes and resolve current supported updates | Dependency resolution, installation and ISO validation |
| Foundry packages | Detect upstream releases; update source pins, checksums and Debian versions in reviewable commits | Package tests, source/license checks, install/upgrade tests; hold changes outside the approved automatic-update policy for review |
| Foundry patches | Reapply and test patches; detect upstream inclusion | Never silently discard a patch or assume a newer upstream fixes the issue |
| WorldFoundry packages | Consume the published repository and check dependency availability | Keep its independent publishing owner; report stale or unavailable dependencies |
| Anvil ISO | Build daily from the selected tested package snapshot, even if Foundry sources are unchanged | Embedded package manifest, UEFI desktop boot and installer launch |
| Atelier ISO | Explicit additional schedule after capacity is measured | Same gates; initially outside the daily Anvil scope |

Unchanged package binaries need not be rebuilt or republished. Record a successful “checked; no updates” result separately from a new package release. New upstream versions needing review remain visible as pending; do not claim everything is current while candidates are held. Define permitted unattended updates per package before enabling automatic promotion.

## Execution and publication flow

```mermaid
flowchart TD
    A[Daily timer or manual Task] --> B[Lock, clean checkout, credentials and disk preflight]
    B --> C[Refresh indexes and discover upstream updates]
    C --> D[Pin candidate sources and build changed packages in containers]
    D --> E{Package tests and update policy pass?}
    E -- No --> H[Record failure or review needed; preserve published artifacts]
    E -- Yes --> F[Sign and publish complete APT snapshot; verify live]
    F --> G[Build Anvil from that snapshot in a container]
    G --> I{Manifest, desktop boot and installer launch pass?}
    I -- No --> H
    I -- Yes --> J[Sign and upload immutable ISO; verify live]
    F --> K[Publish each package update record on wald3n]
    J --> L[Publish ISO record and package snapshot links on wald3n]
    K --> M[Verify exact public release IDs, versions and hashes]
    L --> M
    M --> N[Mark stages complete; update freshness status]
    H --> O[Retain logs and report incomplete daily run]
```

Package publication may succeed while an ISO fails. Publish those successful package records promptly and report the ISO failure separately. A website failure leaves publication incomplete; retry the site stage from durable receipts without rebuilding or republishing the same binaries.

## Public pages and review screens

Proposed URLs belong to the existing Astro site in `../wald3n.com/`. These are new routes to implement, not pages that already exist. Artifact bytes remain on the appropriate APT/ISO storage; the site provides the public release record and download links.

```text
/open-source                         Existing dashboard
  Foundry updates                    [View update history]
  Last check: 2026-11-16 03:00 +07:00
  Packages: published   ISO: testing   Website: current

/open-source/foundry                  Update history
  [All] [Packages] [ISOs]             [Search package or version]
  2026-11-16  PowerDevil <version>     Published  [Changes] [Downloads]
  2026-11-16  Anvil <version>          Published  [Packages] [Download ISO]
  Last successful ISO: <timestamp>    Daily check: healthy / stale

/open-source/foundry/releases/<release-id>
  Anvil <version> — published <timestamp with timezone>
  Changes: <human-readable summary>
  Validation: UEFI desktop PASS; installer launch PASS
  [Download ISO] [SHA256] [Signature] [Package manifest]
  Source commit: <link>   Package snapshot: <link>
  Related package updates: <version-specific links>
```

The history retains a record for each updated source package, including its binary packages and exact versions, and each ISO. A batch release may group entries but must not hide individual updates. Supply stable IDs and permalinks; use an additive schema-validated release ledger rather than overwriting history with today’s inventory. Include source/build revisions, artifact URLs and SHA256, UTC publication time, displayed timezone, upstream/Foundry change summaries, validation evidence and superseded-release links. An RSS/JSON feed can expose the same ledger without a separate source of truth.

```text
Operator review — proposed `task daily:status`
  Run <id>  Source <commit>  Mode: candidate / automatic
  Packages: 3 passed, 1 needs review
  ISO: waiting for package snapshot
  Public records: 3 verified, 0 pending
  [Inspect evidence] -> task daily:inspect RUN=<id>
  [Retry failed stage] -> task daily:retry RUN=<id> STAGE=site
  [Promote reviewed candidate] -> task daily:promote RUN=<id>
```

These Task names are proposed interfaces. Promotion operates on the exact tested manifest and refuses changed inputs. Routine runs follow the configured promotion policy; the review screen is for held candidates and failures.

## Implementation checklist

- [ ] Select the physical lab host and provision the dedicated build VM with storage, network capacity and nested KVM for ISO boot tests; measure a complete Anvil run. Use the established container stacks and Ubuntu 26.04 where required. Resolve interactive `pkexec` steps through a narrowly scoped build service before scheduling.
- [ ] Add a Task include and a single resumable orchestration command with stage receipts, a lock, deadlines and bounded retries. Use isolated checkouts and pinned builder images; record all input revisions and resolved package versions.
- [ ] Add upstream discovery and reviewable pin/version updates. Define the unattended-update policy and dependency rebuild rules. Reuse verified cached outputs keyed by source, recipe and builder inputs.
- [ ] Make signed APT publication independently runnable outside Actions. Preserve complete package/source inventories and durable artifacts; upload content before metadata, use by-hash indexes or an equivalent consistent snapshot scheme, and verify from a fresh client. Never replace different bytes under the same package version.
- [ ] Make ISO versions unique per run and build from the verified package snapshot. Use published package inputs for release ISOs; prevent arbitrary stale `local-debs/` from overriding them.
- [ ] Replace the process-survival smoke criterion with an explicit guest desktop-ready check and installer-launch evidence. Keep the VM diskless/network-isolated where possible; collect screenshots and logs. Assert PowerDevil regression coverage and installer branding presence.
- [ ] Provision signing/upload credentials to the publishing service outside Git and logs. Validate credential availability in preflight; unsigned output cannot reach public channels.
- [ ] Publish ISO files and signatures under immutable names, verify remotely, then update the latest pointer. Preserve the previous working image if any stage fails.
- [ ] Extend the wald3n data model, history/detail pages and publisher with package/ISO records and exact-version live checks. Reuse its container development workflow and isolated publishing checkout; keep compiler updates independent enough that an unrelated compiler refresh cannot block Foundry history publication.
- [ ] Add durable pending-publication receipts and idempotent retries across repositories. Verify the exact ledger hash/release IDs through the deployed site before clearing completion markers.
- [ ] Add a freshness monitor independent of the build process: alert on failures, missed checks and no successful daily ISO for more than 36 hours. Select an existing authorized notification destination before enabling outbound messages. Show separate package-check, ISO and website timestamps.
- [ ] Install the 03:00 timer with catch-up behavior, validate a manual full run, then observe two scheduled runs. Later migrate the same commands to lab Kubernetes jobs when that platform is ready.

## Validation and operational limits

Test discovery with a known new release; unchanged inputs; patch conflicts; failed package tests; missing signing keys; incomplete package inventory; unavailable upstreams; low disk; overlapping jobs; interrupted uploads; a VM stuck at the splash screen; and a website deployment that serves an old version. Each failure must preserve the previous published artifacts and retain a truthful pending/failed status. Confirm site retries do not duplicate release entries.

Success requires two consecutive scheduled runs, signed artifacts installable/downloadable from their public locations, matching ISO and repository package versions, real desktop/installer validation, and the exact package/ISO update records visible under `/open-source`. Exercise missed-run catch-up and the independent stale monitor. A schedule cannot guarantee a successful update during an upstream outage; it must make that outage visible.

Do not automatically install daily packages on Will’s laptop or write removable media as part of this job. Ventoy remains an explicit, checksum-verified sync using `../home-lab/Taskfile.distros.yml` and `home-lab/boot/foundry/`. Do not delete Docker images, caches or volumes. Choose and approve artifact retention separately; retain published release records even if an older downloadable artifact is eventually retired.

## Review decisions

- [x] Will agreed on 2026-11-15: dedicated build VM on an always-on home-lab machine, container builds, and a systemd timer at 03:00 Asia/Bangkok.
- [ ] Select the physical lab machine, size/provision the VM and verify nested KVM; no machine has been assigned or scheduler activated yet.
- [ ] Confirm per-package automatic promotion policy and initial Anvil-only daily scope.
- [ ] Confirm the proposed `/open-source/foundry` history and release-detail routes.
- [ ] Confirm signing service, notification destination and artifact retention policy before activation.

Reference: [systemd timer documentation](https://www.freedesktop.org/software/systemd/man/latest/systemd.timer.html) describes calendar scheduling and `Persistent=true` catch-up. The historical [daily APT audit plan](2026-06-13-audit-apt-repos-scheduled-daily-snapshots.md) uses Actions; this plan supersedes that scheduling approach for this work.

AI assistance: Codex CLI 0.169.0; model gpt-6-astra; reasoning effort medium, verified from session metadata.
