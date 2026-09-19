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
- [ ] Workflow tests cover authorized durable hydration/persistence and fail-closed fallback behavior.
- [ ] Marker lifecycle tests cover create, merge, retain-on-failure, verify, and clear-on-success.
- [ ] wald3n regression check rejects the legacy sibling checkout and accepts the canonical monorepo tree.
- [ ] wald3n offline schema/uniqueness verification passes.
- [ ] Targeted production workflow concludes green.
- [ ] Live APT index contains the target and an unrelated control package.
- [ ] Clean Ubuntu 26.04 `apt-get update` and target install succeed.
- [ ] Live wald3n `/open-source` contains the target.
- [ ] Completion guard prints the verified package and removes the pending marker.
- [ ] A second targeted/no-change run proves durable mirror round-trip and skip behavior.

## Out of scope

- Porting the durable mirror design to `apt.worldfoundry.org`; do that after this proof is green.
- Changing APT suite names, signing keys, repository layout, or package contents beyond the small proof revision.
- Treating GitHub Actions cache as the long-term authoritative artifact store.

