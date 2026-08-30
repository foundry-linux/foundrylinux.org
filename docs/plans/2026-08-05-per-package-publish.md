# Per-package publish for apt.foundrylinux.org

**Date:** 2026-08-05
**Scope:** Make publishing one package cost minutes instead of ~45 minutes, by replacing the GitHub Actions `.deb` cache with an authoritative R2-backed `dist/` mirror and adding a targeted-build input to the publish workflow. Covers `apt.foundrylinux.org` first, then the same change on `apt.worldfoundry.org`.

---

## TL;DR

Publishing xemu on 2026-08-05 took **45+ minutes** even though only one package changed. I told the user "republishing everything is unavoidable." That was half wrong, and worth correcting precisely:

| Layer | Avoidable? | Real cost |
|---|---|---|
| Re-generating `Packages`/`Release` for the suite, and re-signing | **No** — inherent to the apt format. One signed manifest covers the whole suite. | Seconds |
| Re-uploading unchanged pool files | **Already avoided** — `rclone sync --checksum` skips them | Seconds |
| **Re-building the other 26 packages from source** | **Yes — and this is the entire problem** | **~45 min** |

So "you can't publish one package" is false. What is true is that you can't *re-index* one package — and re-indexing is not what costs the time.

## Root cause: the cache is the wrong durability primitive

`build-all.sh` already skips any package whose `.deb` (and `.dsc`) is present in `dist/`, and it already accepts a single-package filter:

```bash
bash scripts/build-all.sh xemu     # supported today; the workflow never passes it
```

That skip-logic only works if `dist/` arrives populated. Today it is populated by `actions/cache`:

```yaml
- name: Restore built .deb cache (incremental builds)
  uses: actions/cache@v5
  with:
    key: foundry-apt-dist-${{ github.sha }}
    restore-keys: |
      foundry-apt-dist-
```

Two failure modes, both hit us:

1. **GitHub evicts caches not accessed for 7 days.** The previous publish was v1.5.38 on **2026‑07‑13**; the xemu publish was **2026‑08‑05**, 23 days later. The cache was almost certainly gone, so the "incremental" build rebuilt all 27 packages from source. This is the leading explanation for the 45-minute run and must be confirmed from that run's logs (see Verification step 1).
2. **10 GB per-repo cache limit.** `dist/` is already ~1.5 GB and now carries xemu's **185 MB** `orig.tar.gz`. Source tarballs for future vendored upstreams push toward that ceiling, at which point evictions become routine rather than occasional.

Meanwhile **the artifacts we keep throwing away already exist in R2**: the live pool holds 55 binary and 53 source packages, each at a stable URL:

```
pool/main/a/asar-snes-assembler/asar-snes-assembler_1.91-1foundry2_amd64.deb
```

We rebuild from source what we are already paying to store.

## Design

### 1. Replace the cache with an R2 `dist/` mirror

Keep a flat mirror of `dist/` at `R2:foundry-apt/.dist-cache/` — the exact shape `build-all.sh` and `init-repo.sh` expect, so no flattening logic is needed on restore. The workflow credentials are scoped to the existing `foundry-apt` bucket, so the mirror is an excluded internal prefix rather than a separate bucket.

- **Hydrate** at job start, before the build: `rclone copy R2:foundry-apt/.dist-cache/ ./dist/ --checksum`
- **Persist** at job end, after a successful publish: `rclone sync ./dist/ R2:foundry-apt/.dist-cache/ --checksum`
- **Protect** the mirror during public repo sync: `--exclude '.dist-cache/**'`.

Durable, no eviction, no size cap that matters, and it costs one extra R2 prefix. Cheaper than the compute it removes.

Keep `actions/cache` as a *second* layer if desired (fast local restore), but R2 becomes the source of truth — a cache miss then costs a download, not a rebuild.

> Why a separate `dist/` prefix rather than reconstructing from the published `pool/`? The pool is laid out by hash-prefix directories and does not carry the `.dsc`/`.tar.*` naming `build-all.sh` probes for. A flat mirror keeps the restore a one-line `rclone copy` with no path rewriting, and keeps the published pool exactly as aptly wants it.

### 2. Add a targeted-build input to the workflow

```yaml
workflow_dispatch:
  inputs:
    packages:
      description: 'Space-separated package names to (re)build; empty = all'
      type: string
      default: ''
```

Threaded into the build step as `bash scripts/build-all.sh ${{ inputs.packages }}`. Empty preserves today's behaviour exactly.

`build-all.sh` takes one positional filter today; widen it to accept several (loop the filter list rather than a single `PKG_FILTER` string comparison).

### 3. The safety property that must not be got wrong

After a targeted build, **`dist/` must still contain every other package**, because `init-repo.sh` + `publish-local.sh` rebuild the aptly repo from whatever is in `dist/`. If hydration silently fails and we then build only xemu, aptly would publish a repo containing *one* package — and `rclone sync` would delete the rest of the pool. Every installed system would lose its upgrade path in one run.

So the workflow gains a hard gate between build and publish:

```bash
# scripts/check-dist-complete.sh
# Abort if dist/ has fewer binary packages than the live repo currently publishes.
LIVE=$(curl -fsSL https://apt.foundrylinux.org/dists/resolute/main/binary-amd64/Packages.gz \
        | gzip -dc | grep -c '^Package: ')
LOCAL=$(ls dist/*.deb 2>/dev/null | wc -l)
if (( LOCAL < LIVE )); then
    echo "ERROR: dist/ has $LOCAL .debs but the live repo publishes $LIVE — refusing to publish a truncated index." >&2
    exit 1
fi
```

Deliberately a floor, not equality: adding a package legitimately makes `LOCAL > LIVE`. Deliberately fail-closed: if the check itself cannot reach the live repo, treat that as failure rather than proceeding blind. This gate is the single most important part of the change — it is what makes targeted builds safe.

### 4. Keep tag-driven publish as the release path

No change to how releases happen: `task bump` still tags and publishes everything. `workflow_dispatch` with `packages:` becomes the targeted path for a one-package fix. The two share the same job.

## Expected effect

| Scenario | Today | After |
|---|---|---|
| Metapackage `Depends:` change | ~45 min (cold cache) | ~3 min |
| New vendored upstream (e.g. xemu) | ~45 min | ~build time of that one package |
| Warm cache, one package | ~10 min | ~3 min |
| Cache evicted | full 27-package rebuild | download from R2 |

## Implementation and production status (updated 2026-08-29)

**Phase 1 is complete and proven in production.** Release `v1.5.47`
([run 33285936705](https://github.com/foundry-linux/foundry-apt/actions/runs/33285936705),
2026‑08‑30 01:33–01:40 UTC) went green end to end in **7 min 9 s**, hydrating
198 artifacts from the R2 mirror, emitting **55 `SKIP` lines** (no vendored
upstream recompiled), passing the completeness gate at 59 local vs 58 live, and
persisting the mirror back. The live repo now publishes **59** binary packages
with `Origin`/`Label: Foundry Linux`. Raw evidence is under **Verification**
below.

The previously recorded `AccessDenied` is **resolved by design, not by a grant**:
the mirror was moved to the `.dist-cache/` prefix inside the already‑authorized
`foundry-apt` bucket, so no separate `foundry-apt-dist` bucket is used and none
needs authorizing. The run's logs contain no `AccessDenied` and no `403`.

**Phase 2 is implemented** in `~/worldfoundry.org` (see the Phase 2 section) and
verified locally against real builds. It has **not** been released — publishing
it is a tag push and is left for the user.

### Superseded status notes (2026-08-06)

- The cache-eviction hypothesis is confirmed: v1.5.39 run `31020896519`
  restored no usable `dist/` cache, emitted zero `SKIP` lines, and rebuilt all
  27 packages.
- The multi-package selector and completeness gate pass focused local tests.
  A deliberately truncated `dist/` is rejected before aptly publication.
- Full bootstrap run `31071356548` failed safely while installing RPCS3 build
  dependencies. The same control file subsequently solved in a clean Ubuntu
  26.04 container, so no RPCS3 packaging change was made.
- Bootstrap outputs now use explicit `actions/cache/restore` and
  `actions/cache/save` steps. Successful packages therefore survive a later
  package failure; the durable R2 mirror is still persisted only after a
  successful publish.
- Run `31074610445` exposed and stopped at a pre-existing ShellCheck regression
  in `audit-upstream-packaging.sh`; that lint error is fixed and synced.
- Full bootstrap run [`31076135187`](https://github.com/foundry-linux/foundry-apt/actions/runs/31076135187)
  is currently in progress.

### R2 mirror location

The first production attempt proved that the repository credentials cannot list
a separate `foundry-apt-dist` bucket (`AccessDenied`). The signed public repo was
already published successfully, but the final mirror-persist step failed. The
mirror therefore lives under the authorized `foundry-apt` bucket at the excluded
`.dist-cache/` prefix. Both publishing workflows exclude that prefix so a public
repo sync cannot delete it.

**One production caveat worth recording rather than hand-waving.** The v1.5.47
persist step logged seven `NotImplemented: Not Implemented` errors on
`rclone copy` before succeeding:

```
2026-08-30 01:39:02 ERROR : foundry-sprite_1.0.2_all.deb: Failed to copy: NotImplemented: Not Implemented
2026-08-30 01:39:02 ERROR : losslesscut_3.69.0-1foundry1.dsc: Failed to copy: NotImplemented: Not Implemented
2026-08-30 01:39:13 ERROR : Attempt 1/3 failed with 7 errors and: NotImplemented: Not Implemented
2026-08-30 01:39:13 ERROR : S3 bucket foundry-apt path .dist-cache: not deleting files as there were IO errors
2026-08-30 01:39:18 ERROR : Attempt 2/3 succeeded
```

The concrete cause is R2 rejecting a subset of the S3 upload calls rclone issues,
not a permission problem or a network blip: attempt 1 failed for exactly the
newly built artifacts, rclone's own 3-attempt retry re-issued them, and attempt 2
completed `400 / 400` checks with `Deleted: 3`. The important secondary
behaviour is that rclone **refused to delete anything** while errors were
outstanding — so a partial failure cannot truncate the mirror. No change is
needed; if the retry ever exhausts all three attempts the step fails and the
mirror is simply left at its previous good state.

### Phase 2 landing (2026-08-29)

The Phase 2 changes described on 2026‑08‑06 as "prepared but uncommitted" were
**not present in the working tree** when this item was resumed — they had been
lost. They were re-implemented from the now-proven foundry-apt design and
verified locally against real container builds (steps 7–10 below).

Two deliberate deviations from the Phase 2 list as originally written:

1. **The mirror is `R2:worldfoundry-apt/.dist-cache/`, not `R2:worldfoundry-apt-dist/`.**
   Item 3 of the Phase 2 list predates the R2-mirror-location finding above. A
   separate bucket would need a new grant on the workflow token for no benefit;
   an excluded in-bucket prefix is the pattern foundry-apt actually proved.
   **No `worldfoundry-apt-dist` bucket is created or authorized.**
2. **`prune-dist.sh` is part of the port, and was not on the original list.**
   Dropping `rm -f dist/*.deb` from `build-all.sh` is what makes the mirror
   work, but it also removes the only thing that was stopping orphaned and
   superseded `.debs` from being republished forever. foundry-apt learned this
   and added `scripts/prune-dist.sh`; porting the mirror without it would ship a
   known bug.

## Phase 2 — the same change on `apt.worldfoundry.org`

In scope, sequenced after foundry-apt is proven. The repos ship independently by design — separate workflows, separate R2 buckets, separate tag namespaces (`v*` vs `apt-v*`) — so this is a **pattern copied, not a dependency introduced**. Nothing here makes either repo's publish wait on the other.

Its pipeline (`worldfoundry.org/.github/workflows/apt-publish.yml`, source tree `worldfoundry.org/apt/`) is **further behind** than foundry-apt, so it needs strictly more work:

| Capability | foundry-apt | worldfoundry-apt |
|---|---|---|
| `.deb` cache between runs | `actions/cache` (fragile, but present) | **none at all** |
| "already current" skip logic in `build-all.sh` | yes | **no** — only skips dirs lacking `debian/control` |
| Single-package filter | yes (`build-all.sh <name>`) | **no** |

So today it unconditionally rebuilds all **15** packages on every publish. The work there is therefore:

1. Port the `dist/`-aware skip logic from foundry-apt's `build-all.sh` (the `.deb` + `.dsc` existence check).
2. Port the package-name filter, with the same multi-name widening.
3. Add the R2 `dist/` mirror hydrate/persist steps against `R2:worldfoundry-apt-dist/`.
4. Add the same completeness gate, pointed at that repo's own published `Packages.gz` under [apt.worldfoundry.org](https://apt.worldfoundry.org/).
5. Add the `workflow_dispatch` `packages:` input.

**Different cost/benefit, and worth being honest about it.** worldfoundry's 15 packages are 9 small C CLIs, a Blender add-on, and metapackages — none is a QEMU fork, so the wall-clock saving is minutes, not 45 of them. The justification there is consistency and not maintaining two divergent publish designs, rather than raw time saved. That argues for doing it *after* foundry-apt has proven the design in production, exactly as sequenced — and for lifting the shared parts (`build-all.sh` skip/filter logic, the completeness gate) into a form both repos can carry rather than hand-porting twice and letting them drift.

## Out of scope

- Changing the apt repo format, suite, or signing model.
- Per-package *signing*. The `Release` file is one signed manifest for the suite; that is apt's design and is not something to work around.

## No visible surface

This is CI and publishing infrastructure; there is no UI, rendered page, or CLI output surface, so no mockups are carried.

---

## Verification

1. **PASS (2026-08-05): Confirm the cache-eviction hypothesis** — v1.5.39 run 31020896519 had **0 `SKIP` lines** and rebuilt all **27 packages**. (The downloaded GitHub job log contains each build marker twice, for 54 raw matches.) The cache was empty/evicted; xemu was not the sole cost.
2. **PASS (2026-08-06): Hydrate works from empty** — production run 31082484311 populated the authorized `R2:foundry-apt/.dist-cache/` prefix, and follow-up run 31082913139 restored 198 mirror items from that prefix before building.
3. **PASS (2026-08-06): Targeted build publishes one package** — `packages: rpcs3` built and published RPCS3 while retaining the complete live index; the mirror-backed follow-up run 31082913139 reported `SKIP rpcs3 (dist/rpcs3_0.0.42+dfsg-1foundry1_amd64.deb already current)`.
4. **PASS (focused test, 2026-08-05): The completeness gate fires** — a deliberately truncated synthetic `dist/` was rejected before publication; repeat in production after step 2.

    **Repeated in production, 2026‑08‑29** (run 33285936705, step "Refuse to publish a truncated repository"):

    ```
    dist/ completeness check passed: 59 local .debs, 58 live packages
    ```

    The gate ran on the real tree and passed as a floor (`59 ≥ 58`; the +1 is
    the newly added package this release shipped). **PASS**

5. **No regression in the published repo** — after a targeted publish, the live `Packages.gz` count is unchanged (or +1), `apt-get update` succeeds in a clean `ubuntu:26.04`, and an unrelated package still installs.

    Live index and Release metadata after run 33285936705:

    ```
    $ curl -fsSL https://apt.foundrylinux.org/dists/resolute/main/binary-amd64/Packages.gz | gzip -dc | grep -c '^Package: '
    59
    $ curl -fsSL https://apt.foundrylinux.org/dists/resolute/Release | head -8
    Origin: Foundry Linux
    Label: Foundry Linux
    Suite: resolute
    Codename: resolute
    Date: Sun, 30 Aug 2026 01:38:31 UTC
    Architectures: all arm64 amd64
    Components: main
    Description: Generated by aptly
    ```

    `apt-get update` in a clean `ubuntu:26.04` with both sources wired, and an
    unrelated package installing from the live repo:

    ```
    Get:5 https://apt.foundrylinux.org resolute InRelease [12.3 kB]
    Get:8 https://apt.foundrylinux.org resolute/main all Packages [21.6 kB]
    Get:9 https://apt.foundrylinux.org resolute/main amd64 Packages [36.8 kB]
    ...
    ii  blender-asset-finder-cli 0.1.0+git0a19d26c-1foundry1 all   CLI companion to the blender-asset-finder Blender add-on
    ```

    Count went 58 → 59 (+1, the new package), signed index verified by apt, and an
    unrelated package installed. **PASS**

6. **PASS (2026-08-06): Round-trip** — production run 31082484311 completed successfully through durable-cache persistence; follow-up run 31082913139 hydrated the mirror and skipped RPCS3 as already current instead of rebuilding it.

    **Confirmed again at full release scale, 2026‑08‑29** (run 33285936705).
    Hydrate restored the whole mirror:

    ```
    Transferred:   	    1.756 GiB / 1.756 GiB, 100%, 55.110 MiB/s, ETA 0s
    Transferred:          198 / 198, 100%
    Elapsed time:        31.4s
    ```

    The build then recompiled **nothing** that the mirror already carried —
    55 `SKIP` lines, e.g.:

    ```
    SKIP asar-snes-assembler (dist/asar-snes-assembler_1.91-1foundry2_amd64.deb already current)
    SKIP rpcs3 (dist/rpcs3_0.0.42+dfsg-1foundry1_amd64.deb already current)
    SKIP xemu (…already current)
    SKIP ghidra (…already current)
    ```

    and persist closed the loop:

    ```
    Checks:               400 / 400, 100%
    Deleted:                3 (files), 0 (dirs)
    Elapsed time:        18.6s
    ```

    Whole run: **7 m 9 s**, against the ~45 min the plan set out to remove.
    **PASS**

### Phase 2 (`apt.worldfoundry.org`)

Implemented 2026‑08‑29 in `~/worldfoundry.org`. Steps 7–10 were run locally
against real container builds; a production run awaits an `apt-v*` tag push.

7. **Skip logic ported** — re-run a publish with no package changes; all 15 packages report `SKIP` and nothing compiles.

    ```
    $ bash scripts/in-docker.sh bash scripts/build-all.sh
    SKIP cdpack (dist/cdpack_0.1.0+git0a19d26c-1foundry2_amd64.deb already current)
    SKIP iffcomp (dist/iffcomp_0.1.0+git0a19d26c-1foundry2_amd64.deb already current)
    SKIP iffdump (dist/iffdump_0.1.0+git0a19d26c-1foundry2_amd64.deb already current)
    SKIP levcomp (dist/levcomp_0.1.0+git0a19d26c-1foundry2_amd64.deb already current)
    SKIP lvldump (dist/lvldump_0.1.0+git0a19d26c-1foundry2_amd64.deb already current)
    SKIP oaddump (dist/oaddump_0.1.0+git0a19d26c-1foundry2_amd64.deb already current)
    SKIP oas2oad (dist/oas2oad_0.1.0+git0a19d26c-1foundry2_amd64.deb already current)
    SKIP prep (dist/prep_0.103+git0a19d26c-1foundry2_amd64.deb already current)
    SKIP textile (dist/textile_0.1.0+git0a19d26c-1foundry2_amd64.deb already current)
    SKIP worldfoundry-blender-addons (dist/worldfoundry-blender-addons_1.1.3_amd64.deb already current)
    SKIP worldfoundry-blender-editor-exporter (dist/worldfoundry-blender-editor-exporter_0.2.1+git3fa94cbe-2foundry2_amd64.deb already current)
    SKIP worldfoundry-cli (dist/worldfoundry-cli_1.0.3_amd64.deb already current)
    SKIP worldfoundry-development (dist/worldfoundry-development_1.0.4_amd64.deb already current)
    SKIP worldfoundry-editor-dev (dist/worldfoundry-editor-dev_1.0.0_all.deb already current)
    SKIP worldfoundry (dist/worldfoundry_1.1.4_amd64.deb already current)
    ```

    15 `SKIP`, zero `OK`, no compiler invoked. **PASS**

8. **Targeted build works** — `workflow_dispatch` with `packages: textile`; exactly one package builds.

    First run, from an empty `dist/`:

    ```
    $ bash scripts/in-docker.sh bash scripts/build-all.sh textile
    ...
    dpkg-deb: building package 'textile' in '../textile_0.1.0+git0a19d26c-1foundry2_amd64.deb'.
    OK   /work/apt/dist/textile_0.1.0+git0a19d26c-1foundry2_amd64.deb  (317722 bytes)

    === dist/ ===
    total 312K
    -rw-r--r-- 1 ubuntu ubuntu 311K Aug 30 02:50 textile_0.1.0+git0a19d26c-1foundry2_amd64.deb
    ```

    Second run, unchanged:

    ```
    $ bash scripts/in-docker.sh bash scripts/build-all.sh textile
    SKIP textile (dist/textile_0.1.0+git0a19d26c-1foundry2_amd64.deb already current)
    ```

    Exactly one package built; the other 14 were never entered. **PASS**

9. **Completeness gate fires** — truncated `dist/` aborts the publish before the R2 sync, same as step 4.

    ```
    $ DIST_DIR=<synthetic dir with 2 .debs> bash scripts/check-dist-complete.sh
    ERROR: dist/ has 2 .debs but the live repo publishes 14 — refusing to publish a truncated index.
    exit=1

    $ DIST_DIR=<synthetic dir with 100 .debs> bash scripts/check-dist-complete.sh
    dist/ completeness check passed: 100 local .debs, 14 live packages
    exit=0

    $ PACKAGES_URL=…/NOPE.gz bash scripts/check-dist-complete.sh     # fail-closed
    curl: (22) The requested URL returned error: 404
    exit=22

    $ bash scripts/in-docker.sh bash scripts/check-dist-complete.sh  # the real dist/
    dist/ completeness check passed: 15 local .debs, 14 live packages
    ```

    Truncation rejected, floor honoured (`15 ≥ 14` — the +1 is the unpublished
    `worldfoundry-editor-dev`), and an unreachable live index fails closed
    rather than proceeding blind. **PASS**

    `prune-dist.sh`, which replaces the `rm -f dist/*.deb` this change removed,
    was tested the same way — a synthetic superseded version and a synthetic
    orphan were both dropped and the 15 real packages kept:

    ```
    $ bash scripts/in-docker.sh bash scripts/prune-dist.sh
    drop  ghost-removed-pkg_1.0_amd64.deb  (renamed/removed — no current source for 'ghost-removed-pkg')
    prune textile_0.0.1-old_amd64.deb  (superseded by textile 0.1.0+git0a19d26c-1foundry2)
    prune-dist: kept 15 package(s); removed 1 orphan(s) + 1 superseded .deb(s), 0 orphan + 0 superseded source artifact(s)
    ```

10. **Both repos still resolve together** — in a clean `ubuntu:26.04` with both apt sources wired, `foundry-core` still resolves (it `Depends: worldfoundry`), confirming the cross-repo coupling survived changes to both pipelines.

    **The step as written is not runnable, and that is a pre-existing fact about
    `foundry-core`, not a regression.** `foundry-core` `Depends: task`, and
    `task` is Cloudsmith-sourced — it is in neither apt repo:

    ```
    $ apt-cache policy task
    task:
      Installed: (none)
      Candidate: (none)
    $ apt-get install --dry-run foundry-core
    E: Unable to satisfy dependencies. Reached two conflicting assignments:
       1. foundry-core:amd64=1.0.6 is selected for install
       2. foundry-core:amd64 Depends task
          but none of the choices are installable
    ```

    The property the step was written to check — that a package in one repo
    resolves against a package in the other — is checked instead with
    `worldfoundry-cli`, which lives in `apt.worldfoundry.org` and
    `Depends:` on `blender-asset-finder*` in `apt.foundrylinux.org`:

    ```
    $ apt-get install --no-install-recommends --dry-run worldfoundry-cli
    Inst libexpat1 (2.7.4-1 Ubuntu:26.04/resolute [amd64])
    Inst python3 (3.14.3-0ubuntu2 Ubuntu:26.04/resolute [amd64])
    ...
    (no E: lines — full plan produced across both repos)

    $ apt-get install --no-install-recommends blender-asset-finder-cli
    ii  blender-asset-finder-cli 0.1.0+git0a19d26c-1foundry1 all
    ```

    Cross-repo coupling intact. **PASS (via `worldfoundry-cli`; step text should
    be corrected to name a package that does not need the Cloudsmith source).**
