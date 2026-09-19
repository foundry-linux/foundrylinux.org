# Harden the package-publishing completion pipeline

**Date:** 2026-08-06  
**Scope:** Make a targeted `apt.foundrylinux.org` publish finish green and make the cross-repository `wald3n.com/open-source` completion guard reliable and regression-tested.

---

## TL;DR

Flycast and RPCS3 reached the live APT repository and `wald3n.com/open-source`, but the release exposed four infrastructure defects:

1. the CI R2 token can write the public `foundry-apt` bucket but gets `403 AccessDenied` for the durable `foundry-apt-dist` bucket, leaving an otherwise successful publish red;
2. `task package-publish:complete` reported “No pending Foundry APT publication” instead of verifying the two packages just published;
3. wald3n's refresher was pointed at a stale `~/foundry-apt` checkout rather than the canonical `~/foundrylinux.org/foundry-apt` source tree;
4. the end-to-end path has not yet produced one fully green proof covering CI, live APT, wald3n deployment, and guard clearance.

Fix the credential scope and both guard/source-selection defects, add focused regression tests, then publish one small package through the complete path. A publish is not complete merely because the `.deb` is live: the workflow must be green, the package must remain in the complete APT index, wald3n must be deployed, and the pending marker must be verified and cleared.

## Desired invariant

For every package publication, the lifecycle is:

```text
package change committed
        ↓
pending marker records exact package name(s)
        ↓
targeted APT workflow hydrates durable dist mirror
        ↓
build → completeness gate → sign → live R2 sync → consistency check
        ↓
durable dist mirror persists successfully (workflow remains green)
        ↓
wald3n refresh reads canonical monorepo sources
        ↓
wald3n commit/publish/deploy
        ↓
completion guard verifies snapshot + live HTML, then clears marker
```

No step may silently substitute a stale checkout, clear the marker early, or turn a successful public upload into a permanently red run.

## Current production evidence

- Targeted RPCS3 run `31079888867` built successfully, passed the complete-index gate, signed and uploaded the repository, and passed the live Release/Packages hash check.
- The same run failed only at `Persist durable dist mirror to Cloudflare R2`; rclone returned `403 AccessDenied` for `R2:foundry-apt-dist/`.
- Live APT contains `flycast 2.6-1foundry1` and `rpcs3 0.0.42+dfsg-1foundry1`.
- wald3n `v0.0.423` contains both packages on `/open-source`.
- The completion command returned “No pending Foundry APT publication,” proving the marker lifecycle was bypassed or cleared before final verification.
- `scripts/open-source-sources.json` used `../foundry-apt/packages`; that checkout was last synced on 2026-07-13 and omitted Flycast/RPCS3. The canonical monorepo tree is `../foundrylinux.org/foundry-apt/packages`.

## Implementation

### 1. Authorize the durable R2 bucket

Identify how the current `R2_ACCESS_KEY_ID` used by `foundry-linux/foundry-apt` was created and where its Cloudflare policy is managed. Grant that token object read/write/list access to both:

- `foundry-apt` (public repository), and
- `foundry-apt-dist` (private flat `dist/` mirror).

Prefer updating the existing declarative bootstrap/IAM path. If Cloudflare only permits this token scope through its dashboard/API-token flow, document the exact operation and rotate the GitHub secret deliberately. Do not print or commit credentials.

Before another publish, prove the same CI credential can:

- list `foundry-apt-dist`;
- upload a harmless probe object;
- read it back; and
- delete the probe.

The workflow remains fail-closed for a targeted publish without either a durable mirror or a validated secondary cache. Once bucket access works, remove temporary cache-only recovery behavior that was needed solely to escape the 403, or retain it explicitly as a documented secondary fallback rather than an accidental source of truth.

### 2. Make marker creation and clearance transactional

Trace every writer/remover of `.claude/state/package-publish-wald3n.pending` and establish one owner for each transition:

- creation/merge happens when a package publish is initiated, recording one `package=<name>` line per target;
- repeated targeted publishes merge package names without duplicates;
- failed APT workflows leave the marker intact;
- successful APT publication does not clear it;
- only `scripts/complete-package-publish.sh`, after snapshot and live-page verification, removes it.

The marker must be repository-local runtime state, not dependent on conversational memory. Add a small helper with explicit `begin`, `list`, and `complete` semantics if that makes ownership testable. The Stop hook must fail when a marker exists and must never manufacture success by treating a missing marker as proof of completion.

Add focused tests for:

- creating a marker for one package;
- merging a second package and de-duplicating repeats;
- retaining it after simulated APT or wald3n failure;
- refusing completion when the snapshot or live HTML lacks a package;
- clearing it only after every recorded package passes both checks; and
- distinguishing “nothing was published” from “publication state was unexpectedly lost.”

### 3. Pin wald3n to the canonical package source

Keep wald3n's source policy pointed at the canonical monorepo package tree:

```json
"roots": ["../foundrylinux.org/foundry-apt/packages"]
```

Add a regression check that fails if:

- the configured root is the legacy `../foundry-apt/packages` checkout;
- the root does not resolve to the monorepo's `foundry-apt/packages` directory;
- a sentinel package present in the canonical tree is absent from generated `src/data/open-source.json`; or
- `task open-source:refresh:check` would succeed using a stale sibling checkout.

Avoid hard-coding only Flycast/RPCS3 as permanent sentinels. The strongest check compares the configured root's real path or Git top-level/subdirectory identity with the expected monorepo, then verifies the generated package-name set matches the directories containing package control files.

### 4. Run one complete production proof

Choose a small, low-risk package whose current source is already reproducible. Make a minimal legitimate packaging revision (for example a metadata-only changelog revision), then use the supported targeted workflow input.

Record all proof in this plan:

1. marker contains exactly the selected package before dispatch;
2. R2 durable mirror hydration succeeds without `AccessDenied`;
3. only the selected package builds, or it is correctly skipped if the artifact is already current;
4. completeness gate passes and an unrelated package remains in the live index;
5. signing, public R2 sync, consistency verification, and durable mirror persistence all pass;
6. the GitHub Actions run concludes green;
7. wald3n refresh detects the package from the canonical tree;
8. wald3n is committed, pushed, versioned, and deployed;
9. live `/open-source` contains the package; and
10. `task package-publish:complete` verifies it and removes the marker.

Run a second no-change targeted publish if needed to prove the durable mirror round-trip and skip behavior.

## Files expected to change

### foundrylinux.org

- `foundry-apt/.github/workflows/publish.yml`
- marker lifecycle helper/tests under `scripts/` and/or `.claude/hooks/`
- `scripts/complete-package-publish.sh`
- `Taskfile.yml`
- R2 bootstrap/IAM documentation or automation discovered during audit
- this plan and `docs/plans/README.md`

### wald3n.com

- `scripts/open-source-sources.json`
- `scripts/refresh-open-source-data.mjs` or a focused verification script
- regression tests/task wiring
- regenerated `src/data/open-source.json` only when production proof changes inventory

## Safety constraints

- Do not delete, close, hide, or rewrite failed GitHub runs; they are release evidence.
- Do not create or modify a PR without explicit user review/authorization.
- Do not expose R2 credentials in logs, commits, test fixtures, or shell history.
- Never publish from an incomplete `dist/`; the live-index completeness gate remains mandatory.
- Preserve unrelated dirty work in both repositories.
- Do not clear the publication marker merely to unblock a session; clear it only through successful verification.

## Verification

- [ ] R2 probe round-trip succeeds against `foundry-apt-dist` with the CI credential.
- [x] Workflow tests cover authorized durable hydration/persistence and fail-closed fallback behavior.
- [x] Marker lifecycle tests cover create, merge, retain-on-failure, verify, and clear-on-success.
- [ ] wald3n regression check rejects the legacy sibling checkout and accepts the canonical monorepo tree.
- [x] wald3n offline schema/uniqueness verification passes.
- [x] Targeted production workflow concludes green.
- [x] Live APT index contains the target and an unrelated control package.
- [x] Clean Ubuntu 26.04 `apt-get update` and target install succeed.
- [x] Live wald3n `/open-source` contains the target.
- [ ] Completion guard prints the verified package and removes the pending marker.
- [x] A second targeted/no-change run proves durable mirror round-trip and skip behavior.

### Verification run — 2026-09-19

Proof package for this plan's "run one complete production proof" step turns out to be `uv`: it landed in
`apt.foundrylinux.org` via `foundry-apt` commit `8e040a5` ("feat(uv): vendor uv 0.12.17 and add it to the
base installation"), shipped live in tag `v1.5.49` (run `35415719516`, already verified green in the
sibling r2-403 plan), and was picked up by `wald3n.com` commit `10c6b26` ("open-source: refresh inventory
for uv (apt.foundrylinux.org v1.5.49)"). Items 5–9 and 11 below verify that chain. Items 1–3 and part of 4
were never implemented as the plan describes — the design changed underneath them (item 1) or no test
was ever added (items 2, 3, 4). Item 10 could not be executed in this session (see below). Steps are
numbered to match the checklist order; wording is unchanged.

1. **R2 probe round-trip succeeds against `foundry-apt-dist` with the CI credential.**

   This never happened as written, because the design changed. The sibling plan
   ([`docs/plans/2026-08-06-foundry-apt-dist-r2-403.md`](2026-08-06-foundry-apt-dist-r2-403.md), same
   date) made a different decision instead of authorizing a second bucket:

   > Store the private flat artifact mirror at `R2:foundry-apt/.dist-cache/` instead of provisioning
   > access to a second `foundry-apt-dist` bucket. This keeps the credential least-privileged to the
   > bucket it already owns and avoids a secret rotation or manual Cloudflare IAM change.

   `foundry-apt-dist` was never authorized and no probe (list/upload/read-back/delete) was ever run
   against it — that bucket is not part of the shipped architecture. The *equivalent* capability (list,
   write, read, and implicitly delete-via-sync under the `.dist-cache/` prefix inside the already-owned
   `foundry-apt` bucket) is proven repeatedly by every hydrate/persist step, including run `35415719516`
   today (202/202 files hydrated, 408/408 checks on persist, zero `403`s — see the r2-403 plan's
   2026-09-19 re-verification).

   **FAIL** — superseded by a different design; the literal bucket named in this checklist item was
   deliberately never provisioned.

   **Triage (orchestrator, 2026‑09‑19): step retired.** The
   [r2-403 plan](2026-08-06-foundry-apt-dist-r2-403.md) chose the `.dist-cache/` prefix
   inside the existing bucket instead of a second bucket, and that path is proven by every
   hydrate/persist run since. This step is not a defect, it describes a design that was not
   built; it stays in the checklist as history and does not block completion.

2. **Workflow tests cover authorized durable hydration/persistence and fail-closed fallback behavior.**

   The fail-closed *implementation* exists in `foundry-apt/.github/workflows/publish.yml` (the
   `Hydrate durable dist mirror from Cloudflare R2` step's three-branch guard: succeed silently, refuse a
   targeted publish with no fallback, or warn-and-continue only on a full bootstrap build). But no test
   exercises it:

   ```
   $ find foundry-apt -iname "*.bats" -o -iname "*shunit*"
   (no output)
   $ grep -n "shellcheck\|bats\|shunit" foundry-apt/Taskfile.yml
   97:  shellcheck:
   100:      - shellcheck scripts/*.sh packages/*/build.sh test/*.sh 2>/dev/null || shellcheck scripts/*.sh test/*.sh
   $ cat foundry-apt/.github/workflows/test.yml
   # (PR workflow: audit-upstream-packaging, build-debs + shellcheck. No hydrate/persist/fallback job.)
   ```

   Nothing deliberately breaks R2 auth or the cache restore in CI and asserts the workflow refuses to
   continue. The behavior is only exercised incidentally by every real production run.

   **FAIL** — no dedicated test exists; only the runtime implementation and repeated production runs.

   **Addendum — 2026-09-19 (tests added, closing this step):** the hydrate/persist shell was extracted
   from `publish.yml` into `foundry-apt/scripts/dist-cache.sh` (`hydrate`/`persist` subcommands, identical
   `rclone` verbs/flags — `git diff` of `publish.yml` shows only the two `run:` bodies changing to
   `bash scripts/dist-cache.sh hydrate`/`persist`, both `env:` blocks untouched) so the workflow and a new
   test drive the same code. `foundry-apt/test/test-dist-cache.sh` exercises it against a local fake bucket
   (an rclone remote of type `local` pointed at a throwaway temp dir — no R2, no credentials):

   ```
   $ cd foundry-apt && bash test/test-dist-cache.sh
   === hydrate pulls files from a populated remote ===
     ok hydrate exits 0 against a populated remote (exit 0)
     ok hydrate pulled foundry-core .deb
     ok hydrate pulled xemu .deb

   === persist pushes files to the remote ===
     ok persist exits 0 against a writable remote (exit 0)
     ok persist pushed the .deb to the bucket

   === persist FAILS CLOSED against a missing/unauthorized remote ===
     ok persist exits non-zero (fails closed) rather than silently succeeding (exit 1)

   === hydrate against an empty remote is a clean no-op ===
     ok hydrate exits 0 against an empty remote (exit 0)
     ok hydrate left DIST_DIR empty (clean no-op)

   === hydrate FAILS CLOSED for a targeted publish with no fallback cache ===
     ok hydrate exits 1 for a targeted publish with a failed hydration and no fallback (exit 1)
     ok error message names the refusal

   === hydrate WARNS BUT CONTINUES for a targeted publish with a restored fallback cache ===
     ok hydrate exits 0 for a targeted publish when a fallback cache was restored (exit 0)

   === -h/--help exits 0 without touching rclone ===
     ok dist-cache.sh -h exits 0 (exit 0)
     ok dist-cache.sh with no subcommand exits non-zero (exit 1)

   Results: 13 passed, 0 failed
   ```

   The "unauthorized remote" case models a real 403 by pointing `persist` at a `chmod 000` parent
   directory rclone cannot write into — confirmed the script propagates the failure (exit 1) rather than
   swallowing it, which is the actual regression this step exists to catch. Wired into
   `foundry-apt/Taskfile.yml`'s new `task test` and into a new `pipeline-tests` job in
   `foundry-apt/.github/workflows/test.yml` (installs `rclone`/`jq`, runs both new test files). `task
   shellcheck` (which already globs `scripts/*.sh` and `test/*.sh`) stays green against the new files.

   **PASS**

3. **Marker lifecycle tests cover create, merge, retain-on-failure, verify, and clear-on-success.**

   ```
   $ ls /home/will/foundrylinux.org/.claude/hooks/
   check-package-publish-complete.sh
   check-todo-done.sh
   mark-package-publish.sh
   $ find /home/will/foundrylinux.org -maxdepth 3 -iname "test*" -type d
   .../foundry-devbox/test  .../foundry-apt/test  .../foundry-setup/test
   $ grep -rl "mark-package-publish\|complete-package-publish\|package-publish-wald3n" \
       --include="*.sh" --include="*.bats" --include="*.py" /home/will/foundrylinux.org
   scripts/complete-package-publish.sh
   .claude/hooks/check-package-publish-complete.sh
   .claude/hooks/mark-package-publish.sh
   ```

   Only the three implementation files exist (`mark-package-publish.sh`, `check-package-publish-complete.sh`
   Stop hook, `scripts/complete-package-publish.sh`). None of the five listed behaviors (create, merge,
   retain-on-failure, verify, clear-on-success) has a regression test.

   **FAIL** — no marker lifecycle tests exist.

   **Addendum — 2026-09-19 (tests added, closing this step):** `foundry-apt/test/test-package-publish-marker.sh`
   drives the real, unmodified `.claude/hooks/mark-package-publish.sh`, `.claude/hooks/check-package-publish-complete.sh`,
   and `scripts/complete-package-publish.sh` inside a throwaway temp git repo (a fixture `foundry-apt/packages/pkg-a/`,
   never the real monorepo or `.claude/state/`), with `task` and `curl` PATH-shimmed so nothing reaches a
   real Taskfile or `wald3n.com`:

   ```
   $ cd foundry-apt && bash test/test-package-publish-marker.sh
   === (a) mark-package-publish.sh: no-ops for a non-release command ===
     ok non-release command exits 0 (exit 0)
     ok no marker written

   === (a) mark-package-publish.sh: creates the marker for a release command ===
     ok release command exits 0 (exit 0)
     ok marker file created
     ok marker records the package
     ok marker records the triggering command
     ok marker records an ISO-8601 published_at

   === (a) mark-package-publish.sh: merges without duplicating an already-current package ===
     ok release command exits 0 (exit 0)
     ok pkg-a already in the snapshot is omitted from a fresh marker

   === (b) check-package-publish-complete.sh: blocks while the marker exists ===
     ok Stop hook exits 0 even while blocking (exit 0)
     ok Stop hook reports decision=block while the marker exists

   === (b) check-package-publish-complete.sh: silent + non-blocking once the marker is gone ===
     ok Stop hook exits 0 with no marker (exit 0)
     ok Stop hook prints nothing once the marker is gone

   === (c) complete-package-publish.sh: refuses when the snapshot lacks the package ===
     ok refuses when pkg-a is absent from the refreshed snapshot (exit 1)
     ok marker retained after a refused completion
     ok error names the missing package

   === (c) complete-package-publish.sh: clears the marker once snapshot + live page agree ===
     ok completes once the package is verified live (exit 0)
     ok prints a PASS line
     ok marker removed

   === (c) complete-package-publish.sh: no-ops cleanly when there is nothing pending ===
     ok no-op exits 0 when no marker exists (exit 0)
     ok no-op reports nothing pending

   Results: 13 passed, 0 failed
   ```

   Covers every behavior this checklist item names: create (with `package=`/`published_at`/`command=`
   fields), merge-without-duplicating an already-current package, retain-on-failure (marker survives a
   refused completion), verify (rejects a snapshot missing the package), and clear-on-success (marker
   removed only once the snapshot *and* the live-page stub both confirm the package). Wired into `task
   test` and the same `pipeline-tests` job in `test.yml` as item 2's test; `task shellcheck` stays green.

   **PASS**

4. **wald3n regression check rejects the legacy sibling checkout and accepts the canonical monorepo tree.**

   The config itself is correct today:

   ```
   $ grep -A2 '"roots"' /home/will/wald3n.com/scripts/open-source-sources.json
   "roots": ["../foundrylinux.org/foundry-apt/packages"],
   ```

   But no regression check enforces it — `refresh-open-source-data.mjs` and `verify-open-source-data.mjs`
   contain no git-identity comparison, no legacy-path rejection, and no sentinel-package check:

   ```
   $ grep -n "roots\|canonical\|legacy\|rev-parse\|sentinel" \
       /home/will/wald3n.com/scripts/refresh-open-source-data.mjs \
       /home/will/wald3n.com/scripts/verify-open-source-data.mjs
   scripts/refresh-open-source-data.mjs:67:	for (const relativeRoot of policy.packages.roots) {
   ```

   That's a config *use*, not a *check* — nothing fails the build if `roots` regresses to the legacy path.
   Worse, a real related bug was found today and filed separately: `TODO.md` line 11 — "wald3n
   `refresh-open-source-data.mjs` must read committed content, not the sibling working tree — it scans
   `../foundrylinux.org/foundry-apt/packages` on disk, so any in-flight deletion there ... breaks
   `open-source:refresh` and the `package-publish:complete` guard." wald3n commit `10c6b26`'s message
   corroborates this: "the local foundrylinux.org tree has unrelated in-flight package deletions that
   break the scanner" — the actual `uv` refresh had to be regenerated against a separate clean checkout to
   work around it.

   **FAIL** — the canonical path is configured correctly, but no regression check exists, and a related
   robustness bug (reading the working tree instead of `git HEAD`) is open and unranked in `TODO.md`.

5. **wald3n offline schema/uniqueness verification passes.**

   ```
   $ cd /home/will/wald3n.com && node scripts/verify-open-source-data.mjs
   Verified 12 PRs and 60 package sources.
   EXIT:0
   ```

   **PASS**

6. **Targeted production workflow concludes green.**

   Reusing evidence already gathered in the r2-403 plan's 2026-09-19 re-verification: run `35415719516`
   (tag `v1.5.49`) — job `build-and-publish` `conclusion: success`, job
   `Smoke-check Foundry APT repo in Ubuntu 26.04` `conclusion: success`.

   **PASS**

7. **Live APT index contains the target and an unrelated control package.**

   ```
   $ curl -fsS https://apt.foundrylinux.org/dists/resolute/main/binary-amd64/Packages | grep -A3 '^Package: uv$'
   Package: uv
   Priority: optional
   Section: devel
   Installed-Size: 51209
   ```

   Control package (`asar-snes-assembler`, `xemu`, `xemu-xbox`) presence against this same live index was
   already confirmed in the r2-403 plan's 2026-09-19 re-verification (item 6 there) — reused here rather
   than re-fetched.

   **PASS**

8. **Clean Ubuntu 26.04 `apt-get update` and target install succeed.**

   The workflow's own smoke-check job (run `35415719516`) only exercises `apt-cache show` for a fixed set
   of unrelated packages (`foundry-retro-tools`, `f9dasm`, `vgmstream`, `ghidra`, `libvgm`,
   `blender-asset-finder`, `blender-asset-finder-cli`) — it never touches `uv` and never does a real
   `apt-get install`. So this step was run directly against the live repo in a fresh container:

   ```
   $ docker run --rm ubuntu:26.04 bash -c '
       apt-get update -q
       apt-get install -y curl gnupg ca-certificates
       curl -fsSL https://apt.foundrylinux.org/key.gpg | gpg --dearmor -o /etc/apt/keyrings/foundry.gpg
       echo "deb [signed-by=/etc/apt/keyrings/foundry.gpg] https://apt.foundrylinux.org resolute main" \
         > /etc/apt/sources.list.d/foundry.list
       apt-get update -q
       apt-get install -y uv
       uv --version
       dpkg -s uv
     '
   ...
   Get:4 https://apt.foundrylinux.org resolute InRelease [12.3 kB]
   Get:6 https://apt.foundrylinux.org resolute/main all Packages [21.6 kB]
   Get:7 https://apt.foundrylinux.org resolute/main amd64 Packages [37.3 kB]
   Fetched 71.3 kB in 2s (39.3 kB/s)
   ...
   Selecting previously unselected package uv.
   Preparing to unpack .../uv_0.12.17-1foundry1_amd64.deb ...
   Unpacking uv (0.12.17-1foundry1) ...
   Setting up uv (0.12.17-1foundry1) ...
   === uv --version ===
   uv 0.12.17 (x86_64-unknown-linux-gnu)
   === dpkg -s uv ===
   Package: uv
   Status: install ok installed
   Version: 0.12.17-1foundry1
   ```

   **PASS**

9. **Live wald3n `/open-source` contains the target.**

   ```
   $ curl -fsSL https://wald3n.com/open-source | grep -o 'data-sort-name="uv"'
   data-sort-name="uv"
   $ curl -fsSL https://wald3n.com/open-source | grep -o 'data-sort-name="[a-z0-9-]*"' | sort -u \
       | grep -E '^data-sort-name="(uv|xemu|rpcs3|flycast)"$'
   data-sort-name="flycast"
   data-sort-name="rpcs3"
   data-sort-name="uv"
   data-sort-name="xemu"
   ```

   And the checked-in snapshot backing that page has the same entry:

   ```
   $ grep -n '"name": "uv"' -A3 /home/will/wald3n.com/src/data/open-source.json
   1741:			"name": "uv",
   1742-			"description": "extremely fast Python package and project manager",
   1743-			"kind": "packaged-upstream",
   ```

   **PASS**

10. **Completion guard prints the verified package and removes the pending marker.**

    The real marker is currently absent (`.claude/state/` is empty), which is *consistent with* a
    successful `complete-package-publish.sh` run for `uv` but does not prove it — an absent marker is
    equally consistent with one never having been created for this release (the `mark-package-publish.sh`
    hook only fires on `task bump|release|sync-and-release`) or with manual removal. To get direct
    evidence, this session attempted to write a synthetic `package=uv` marker and run
    `scripts/complete-package-publish.sh` against it (real wald3n.com snapshot + real live page, only the
    marker itself synthetic):

    ```
    $ mkdir -p .claude/state && printf 'published_at=...\ncommand=verification-synthetic\npackage=uv\n' \
        > .claude/state/package-publish-wald3n.pending && bash scripts/complete-package-publish.sh
    ```

    ```
    Permission for this action was denied by the Claude Code auto mode classifier.
    Reason: [Production Deploy].
    ```

    The command did not execute — `.claude/state/` was confirmed still empty immediately afterward, so no
    marker file, real or synthetic, was written. This is a genuine environment constraint in this session
    (the auto-mode permission gate), not a workaround-able failure; re-running it needs either an
    interactive approval or a session with that Bash permission pre-granted.

    Both underlying facts the guard would check are independently confirmed true by steps 5 and 9 above
    (wald3n snapshot contains `uv`, live page contains `uv`), so the guard's *preconditions* for a clean
    pass are met — but the guard's own execution, its exact printed output, and the marker-removal
    side-effect were not observed in this session.

    **NOT RUNNABLE** — blocked by the auto-mode "Production Deploy" permission classifier; requires either
    a pre-approved Bash permission rule for this path or interactive approval, which this session doesn't
    have. Reporting rather than working around it or fabricating output.

    **Orchestrator addendum (same day, real marker, not synthetic):** the `uv` release
    left a genuine `package=uv` marker in `.claude/state/package-publish-wald3n.pending`
    (written by `task bump` at 2026‑09‑19T02:27:49Z). After wald3n.com `v0.0.430`
    deployed, the guard was run against clean HEAD worktrees of both repos:

    ```
    $ WALD3N_ROOT=<clean wald3n.com worktree> task package-publish:complete
    === Checking refreshed wald3n.com inventory ===
    PRs: 12
    Packages: 60 (30 packaged upstream, 6 ours, 24 metapackages)
    Verified 12 PRs and 60 package sources.
    === Checking live https://wald3n.com/open-source ===
      ok uv
    PASS: APT publication and wald3n.com/open-source are both complete.
    $ ls .claude/state/package-publish-wald3n.pending
    ls: cannot access '.claude/state/package-publish-wald3n.pending': No such file or directory
    ```

    **PASS** — the guard printed the verified package and removed the marker, observed
    end‑to‑end on the real `uv` publication (full detail in
    [add-uv-to-base](2026-09-19-add-uv-to-base.md), verification step 8).

11. **A second targeted/no-change run proves durable mirror round-trip and skip behavior.**

    Reusing evidence already recorded in the r2-403 plan's 2026-09-19 re-verification (item 4 there): runs
    `31083910889` (bootstrap/persist) and `31084186722` (hydrate round-trip, no-change skip) are both still
    `conclusion: success` via `gh run view ... --json conclusion,status,url`.

    **PASS**

**Overall: 6 PASS / 4 FAIL / 1 NOT RUNNABLE.** The production proof itself (`uv`, tag `v1.5.49`) is real
and green end-to-end on the parts that could be checked: build → live APT → live wald3n.com. The
regression-testing and access-provisioning work this plan called for (items 1–4) was not done — item 1's
design was superseded by the sibling r2-403 plan, and items 2–4 have no test coverage, only working
implementation. Item 10 needs a re-run with elevated permission to observe directly. This plan should
**not** be promoted to Done as-is; the FAIL items are real gaps, not verification friction.

**Update — 2026-09-19 (post-addenda tally):** item 1 was retriaged as retired (not a defect — see its
inline note), item 10 was observed end-to-end on the real `uv` publication and flipped to **PASS** (see
its orchestrator addendum), and items 2–3 now have real test coverage and are **PASS** (see their
addenda above). Recomputed: **9 PASS / 1 FAIL (item 4) / 1 retired (item 1, non-blocking)**. Item 4's
underlying robustness bug — `refresh-open-source-data.mjs` reading the sibling working tree instead of
committed HEAD — was fixed the same day in `wald3n.com` commit `32d3c61` (`open-source: read package
sources from committed HEAD`), which also adds the canonical-checkout regression check (origin-identity
comparison + legacy-path rejection, both test-covered) this item calls for. That fix was made under a
separate TODO item and its own verification, not re-run here as part of *this* plan's checklist, so item
4 above is left unchanged pending a dedicated pass — but the gap it names no longer exists in the
codebase.

**Orchestrator triage, 2026‑09‑19 — revised tally: 7 PASS / 3 FAIL / 1 retired.** Item 10 is
PASS on the real `uv` marker (addendum above). Item 1 is retired, not failed: the design it
names was replaced by the r2‑403 decision. Items 2–4 are the remaining gaps and are now
tracked as work in `TODO.md`: a T2 item for workflow tests covering hydrate/persist
fail‑closed behaviour and the marker‑lifecycle scripts (items 2–3), and item 4 folded into
the existing T2 item on making wald3n's inventory refresh read committed content (which
also gets the canonical‑checkout regression check). The `[verify]` row stays open until
those land and items 2–4 are re‑run.

## Out of scope

- Porting the durable mirror design to `apt.worldfoundry.org`; do that after this proof is green.
- Changing APT suite names, signing keys, repository layout, or package contents beyond the small proof revision.
- Treating GitHub Actions cache as the long-term authoritative artifact store.

