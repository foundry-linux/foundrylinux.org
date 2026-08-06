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
