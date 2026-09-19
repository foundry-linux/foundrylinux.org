# Fix the Foundry APT durable-mirror R2 403

**Date:** 2026-08-06  
**Scope:** Make the post-publication `dist/` persistence step succeed without broadening the existing
GitHub Actions R2 credential or weakening the live-repository completeness gates.

## Problem

Targeted publish run `31082327159` successfully built the four xemu rename packages, reconstructed and
signed the complete APT repository, uploaded it to the public `foundry-apt` bucket, and passed the live
repository consistency check. It then finished red in `Persist durable dist mirror to Cloudflare R2`:

```text
rclone sync ./dist/ R2:foundry-apt-dist/ ...
AccessDenied: Access Denied (HTTP 403)
```

The same credential can write `R2:foundry-apt/`, so this is a bucket-policy mismatch rather than bad
credentials or a broken endpoint. The public repository is healthy, but the failed persistence step
prevents the next targeted publish from treating R2 as its authoritative complete `dist/` baseline.

## Decision

Store the private flat artifact mirror at `R2:foundry-apt/.dist-cache/` instead of provisioning access to
a second `foundry-apt-dist` bucket.

This keeps the credential least-privileged to the bucket it already owns and avoids a secret rotation or
manual Cloudflare IAM change. The prefix is storage-private by convention, not an APT path: no generated
index references it and clients cannot discover it through `Packages` or `Sources`.

Because the public upload uses mirror semantics, every public `rclone sync` must explicitly exclude
`.dist-cache/**`; otherwise publishing `./public/` would delete the durable mirror before the persistence
step recreates it.

## Implementation

1. Change durable hydration in `foundry-apt/.github/workflows/publish.yml`:

   ```text
   R2:foundry-apt-dist/ → R2:foundry-apt/.dist-cache/
   ```

2. Change durable persistence to the same prefix.

3. Add `--exclude '.dist-cache/**'` to the public repository's data sync. Keep the existing exclusions
   for `Release`, `Release.gpg`, and `InRelease`; metadata must still be uploaded last.

4. Preserve the current secondary GitHub Actions cache as recovery-only. R2 remains authoritative once
   the first successful persistence has populated `.dist-cache/`.

5. Update the parent per-package-publish plan and dispatch workflow path so both documentation and the
   supported release entry point name the same prefix.

6. Do not alter the public APT layout, signing process, package versions, or completeness threshold.

## Production proof

Run a targeted no-change publish for a small, already-current package set. It should restore the retained
cache for the bootstrap run, skip current builds, publish the complete repository, and populate
`.dist-cache/`.

Then run a second targeted no-change publish. This is the decisive round-trip proof: hydration must read
from `.dist-cache/` successfully before any build, and persistence must finish without `403`.

For each run, record:

- GitHub Actions run ID and green conclusion;
- hydrate and persist step results;
- number of `.deb` files before publication versus the live index;
- confirmation that an unrelated control package remains indexed;
- signed live `InRelease`/`Packages` consistency result; and
- confirmation that `.dist-cache/` is absent from all generated APT indexes.

### Result — PASS (2026-08-06)

Implementation commit: `a00480d` (`fix(foundry-apt): persist dist cache in authorized bucket`).

Two targeted no-change publishes of `xemu` completed green:

| proof | run | result |
|---|---:|---|
| bootstrap/persist | `31083910889` | **PASS** — 198 durable artifacts checked, current `xemu` skipped, 58 local `.deb`s matched 58 live packages, repository signed/synced/verified, and all 198 artifacts persisted without `403` |
| hydrate round-trip | `31084186722` | **PASS** — hydrated and checked all 198 artifacts from `R2:foundry-apt/.dist-cache/`, skipped current `xemu`, passed 58/58 completeness, and persisted all 198 artifacts again; workflow conclusion `success` |

The live index still contains `xemu`, `xemu-xbox`, and the unrelated control package
`asar-snes-assembler`; it contains neither an `x-emulators` stanza nor a `.dist-cache/` filename. A
clean Ubuntu 26.04 container accepted the signed repository and resolved candidates
`xemu 0~git20260129.40dfef0d-1foundry3` and `xemu-xbox 0.8.136-1foundry2`.

## Verification

- [x] ShellCheck passes for the modified workflow scripts.
- [x] Public sync excludes `.dist-cache/**`.
- [x] Bootstrap targeted publish persists the complete flat mirror successfully.
- [x] Second targeted publish hydrates from the durable mirror and skips current artifacts.
- [x] Both workflow runs conclude green.
- [x] Live `Packages` still contains `xemu`, `xemu-xbox`, and an unrelated control package.
- [x] Live `Packages` contains no `x-emulators` stanza and no `.dist-cache/` filename.
- [x] Clean Ubuntu 26.04 `apt-get update` succeeds against the signed repository.
- [x] No R2 access key, secret, endpoint, or probe payload is written to logs or commits.

### Re-verification — 2026-09-19

The checklist above was checked off on 2026-08-06 alongside a prose Result summary but without raw
command output recorded per step. This re-run pastes live evidence under each original bullet, gathered
from today's production run `35415719516` (tag `v1.5.49`, workflow `publish.yml`, job `build-and-publish`
ID `105823877231` + job `Smoke-check Foundry APT repo in Ubuntu 26.04` ID `105824576132`, both
`conclusion: success`) and from re-confirming the original two-run proof is still green. Steps are
numbered to match the checklist order; wording of each step is unchanged.

1. **ShellCheck passes for the modified workflow scripts.**

   ```
   $ cd foundry-apt && shellcheck scripts/*.sh
   EXIT:0
   ```

   The CI run's own gate step also passed: `build-and-publish` step 5 "shellcheck scripts (same lint gate
   as the PR workflow)" → `conclusion: success`.

   **PASS**

2. **Public sync excludes `.dist-cache/**`.**

   ```
   $ grep -n "exclude" foundry-apt/.github/workflows/publish.yml
   185:            --exclude '.dist-cache/**' \
   186:            --exclude 'Release' --exclude 'Release.gpg' --exclude 'InRelease' \
   ```

   **PASS**

3. **Bootstrap targeted publish persists the complete flat mirror successfully.**

   The original bootstrap proof (run `31083910889`) is still green today:

   ```
   $ gh run view 31083910889 --repo foundry-linux/foundry-apt --json conclusion,status,url
   {"conclusion":"success","status":"completed","url":"https://github.com/foundry-linux/foundry-apt/actions/runs/31083910889"}
   ```

   Fresh corroboration from today's persist step (job `105823877231`, step 21, "Persist durable dist
   mirror to Cloudflare R2", `rclone sync ./dist/ R2:foundry-apt/.dist-cache/ --checksum --progress
   --transfers=8`), pulled via `gh api repos/foundry-linux/foundry-apt/actions/jobs/105823877231/logs`:

   ```
   2026-09-19T02:31:22.8767287Z Checks:               408 / 408, 100%
   2026-09-19T02:31:22.8767868Z Deleted:                3 (files), 0 (dirs)
   2026-09-19T02:31:22.8768390Z Elapsed time:         10.8s
   ```

   No `403` / `AccessDenied` anywhere in the job log (`grep -inE "403|AccessDenied" job1.log` matches only
   unrelated lines — Ubuntu package fetch progress and an rclone `Checks: 403 / 405` counter, not an HTTP
   status).

   **PASS**

4. **Second targeted publish hydrates from the durable mirror and skips current artifacts.**

   The original round-trip proof (run `31084186722`) is still green today:

   ```
   $ gh run view 31084186722 --repo foundry-linux/foundry-apt --json conclusion,status,url
   {"conclusion":"success","status":"completed","url":"https://github.com/foundry-linux/foundry-apt/actions/runs/31084186722"}
   ```

   Fresh corroboration from today's hydrate step (job `105823877231`, step 8, "Hydrate durable dist mirror
   from Cloudflare R2", `rclone copy R2:foundry-apt/.dist-cache/ ./dist/ --checksum --progress
   --transfers=8`):

   ```
   2026-09-19T02:28:56.3036298Z  *                       ghidra_12.1.orig.tar.gz:100% /538.888Mi, 18.081Mi/s, 0sTransferred:   	    2.113 GiB / 2.113 GiB, 100%, 53.170 MiB/s, ETA 0s
   2026-09-19T02:28:56.3037308Z Transferred:          202 / 202, 100%
   2026-09-19T02:28:56.3037823Z Elapsed time:        35.5s
   ```

   All 202/202 durable artifacts hydrated from `.dist-cache/` before any build ran (the `if rclone copy
   ...; then exit 0; fi` guard succeeded on the first branch, so none of the fallback/failure echo lines
   printed), and the completeness gate then passed:

   ```
   $ awk '/check-dist-complete/{f=1} f{print} f&&/##\[endgroup\]/{exit}' job1.log  # (step header)
   ...
   2026-09-19T02:30:33.7282351Z dist/ completeness check passed: 60 local .debs, 59 live packages
   ```

   Caveat: today's run built real new work for `v1.5.49` (60 local vs. 59 live, one package changed)
   rather than repeating the Aug 6 proof's exact "skip everything, no-change" shape — that specific
   no-change scenario is what run `31084186722` already covers and remains the literal evidence for
   "skips current artifacts." Today's run corroborates that hydrate-before-build still works unchanged six
   weeks later; it does not re-run the no-change case.

   **PASS** (via re-confirmed `31084186722` + today's corroborating hydrate)

5. **Both workflow runs conclude green.**

   ```
   $ gh run list --repo foundry-linux/foundry-apt --limit 20 --json databaseId,displayTitle,headBranch,conclusion,event,createdAt
   ...
   {"conclusion":"success", ..., "databaseId":35415719516, "headBranch":"v1.5.49", ...}
   {"conclusion":"success", ..., "databaseId":31084186722, "headBranch":"main", ...}
   {"conclusion":"success", ..., "databaseId":31083910889, "headBranch":"main", ...}
   ```

   Note: run `35415569022` (tag `v1.5.48`, same day) shows `conclusion: failure`, but that is unrelated to
   this fix — it failed at `build-and-publish` step 3 "Check upstream-packaging baseline inventory", a
   pre-build guard that runs before the hydrate step (step 8) or persist step (step 21):

   ```
   $ gh run view 35415569022 --repo foundry-linux/foundry-apt --json jobs | python3 -c '...'
   build-and-publish -> 3 Check upstream-packaging baseline inventory failure
   ```

   **PASS** (for the runs this fix is responsible for; `v1.5.48`'s failure is out of scope — a separate,
   already-diagnosed pre-build guard issue)

6. **Live `Packages` still contains `xemu`, `xemu-xbox`, and an unrelated control package.**

   ```
   $ curl -fsS https://apt.foundrylinux.org/dists/resolute/main/binary-amd64/Packages \
     | grep -E "^Package: (xemu|xemu-xbox|asar-snes-assembler|x-emulators)$"
   Package: asar-snes-assembler
   Package: xemu
   Package: xemu-xbox
   ```

   **PASS**

7. **Live `Packages` contains no `x-emulators` stanza and no `.dist-cache/` filename.**

   ```
   $ curl -fsS https://apt.foundrylinux.org/dists/resolute/main/binary-amd64/Packages | grep -ci "dist-cache"
   0
   ```

   (No `x-emulators` line appeared in the step 6 grep above either.)

   **PASS**

8. **Clean Ubuntu 26.04 `apt-get update` succeeds against the signed repository.**

   The workflow's own `Smoke-check Foundry APT repo in Ubuntu 26.04` job (ID `105824576132`,
   `conclusion: success`) does exactly this in a fresh container against the live signed repo published by
   this same run:

   ```
   2026-09-19T02:32:53.2916602Z Fetched 26.1 MB in 1s (23.2 MB/s)
   2026-09-19T02:32:58.5201082Z Fetched 10.1 MB in 2s (4848 kB/s)
   2026-09-19T02:33:04.4468045Z Fetched 71.3 kB in 1s (50.8 kB/s)
   ```

   No `E:` apt errors, no `GPG error`, no `NO_PUBKEY` anywhere in the job log; the subsequent
   `apt-cache show foundry-retro-tools|f9dasm|vgmstream|ghidra|libvgm|blender-asset-finder|
   blender-asset-finder-cli` calls all returned valid stanzas.

   **PASS**

9. **No R2 access key, secret, endpoint, or probe payload is written to logs or commits.**

   ```
   $ gh api repos/foundry-linux/foundry-apt/actions/jobs/105823877231/logs > job1.log
   $ grep -n "RCLONE_CONFIG_R2" job1.log
   RCLONE_CONFIG_R2_TYPE: s3
   RCLONE_CONFIG_R2_PROVIDER: Cloudflare
   RCLONE_CONFIG_R2_ACCESS_KEY_ID: ***
   RCLONE_CONFIG_R2_SECRET_ACCESS_KEY: ***
   RCLONE_CONFIG_R2_ENDPOINT: ***
   RCLONE_CONFIG_R2_REGION: auto
   $ grep -inE "AKIA[0-9A-Z]{16}|https://[0-9a-f]{32}\.r2\.cloudflarestorage\.com" job1.log job2.log
   (no matches)
   $ git show a00480d --stat
   .github/workflows/foundry-apt-publish.yml    |  1 +
   docs/plans/2026-08-05-per-package-publish.md | 25 +++++++++++++------------
   foundry-apt/.github/workflows/publish.yml    |  5 +++--
   (no key material in the diff)
   ```

   **PASS**

**Overall: PASS.** All nine checklist items re-verified against live evidence dated 2026-09-19. Items 3
and 4 rely on the original Aug 6 two-run proof (still green and reachable via `gh run view`) for their
literal "bootstrap"/"no-change skip" shape, corroborated by today's production run exercising the same
hydrate → build → persist path end-to-end with zero `403`s.

## Rollback

Revert the workflow prefix changes. This returns targeted releases to the GitHub Actions cache fallback
and reproduces the known final-step `403`, but does not roll back or damage the independently published
APT repository. Do not remove `.dist-cache/` during rollback; it is harmless and may contain the only
complete durable artifact baseline.

## Out of scope

- Broadening the CI credential to additional R2 buckets.
- Porting the mirror to `apt.worldfoundry.org` before this path has two green production proofs.
- Fixing unrelated package build failures such as RPCS3 dependency resolution.
- Changing package contents or republishing solely to manufacture a new version.
