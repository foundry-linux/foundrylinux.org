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

Keep a flat mirror of `dist/` at `R2:foundry-apt-dist/` — the exact shape `build-all.sh` and `init-repo.sh` expect, so no flattening logic is needed on restore.

- **Hydrate** at job start, before the build: `rclone copy R2:foundry-apt-dist/ ./dist/ --checksum`
- **Persist** at job end, after a successful publish: `rclone sync ./dist/ R2:foundry-apt-dist/ --checksum`

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
2. **Hydrate works from empty** — clear `dist/`, run the hydrate step, confirm the restored `.deb` count matches the live repo's published count.
3. **Targeted build skips the rest** — `workflow_dispatch` with `packages: foundry-emulators-consoles`; confirm exactly one package builds and the others report `SKIP`.
4. **The completeness gate fires** — with a deliberately truncated `dist/` (delete 5 `.deb`s), confirm `check-dist-complete.sh` exits non-zero and the publish aborts *before* the R2 sync.
5. **No regression in the published repo** — after a targeted publish, the live `Packages.gz` count is unchanged (or +1), `apt-get update` succeeds in a clean `ubuntu:26.04`, and an unrelated package still installs.
6. **Round-trip** — run a targeted publish twice; the second run rebuilds nothing and the published index is byte-identical apart from `Release` timestamps.

### Phase 2 (`apt.worldfoundry.org`)

7. **Skip logic ported** — re-run a publish with no package changes; all 15 packages report `SKIP` and nothing compiles.
8. **Targeted build works** — `workflow_dispatch` with `packages: textile`; exactly one package builds.
9. **Completeness gate fires** — truncated `dist/` aborts the publish before the R2 sync, same as step 4.
10. **Both repos still resolve together** — in a clean `ubuntu:26.04` with both apt sources wired, `foundry-core` still resolves (it `Depends: worldfoundry`), confirming the cross-repo coupling survived changes to both pipelines.
