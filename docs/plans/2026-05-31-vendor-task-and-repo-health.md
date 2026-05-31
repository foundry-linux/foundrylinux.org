# Plan: Vendor `task` into foundry-apt + apt-source health check

## Context

go-task rotated its Cloudsmith apt repo layout from the distro-agnostic
`deb/any-distro any-version` to per-distro `deb/ubuntu/<codename>`. Anyone still
on the old source line gets a hard **404** on `InRelease`/`Release` (CloudFront
even caches the error, so it reads as "intermittent"), which fails `apt update`.

This surfaced while fixing a machine whose `task.list` still used the old path.
**Foundry itself is not currently broken** — its active config uses the new
per-distro path everywhere (`foundry-iso/config/archives/cloudsmith-task.list.chroot`
→ `ubuntu resolute`, verified 200, serving the current task **3.51.1**;
`install-task.sh` / devbox use `setup.deb.sh`, which OS-detects → `ubuntu resolute`).

But relying on a third-party repo whose layout can change under us is the exact
"manual setup = liability" the repo's reproducibility rules warn against. Two
defenses: **own the package** (vendor `task` into apt.foundrylinux.org) and
**detect rot early** (health-check every shipped apt source).

## Done (this change — verified)

1. **Vendored `task` into foundry-apt** — `foundry-apt/packages/task/`
   (`build.sh` + `debian/`), mirroring the `ruff` binary-vendor pattern:
   downloads `task_linux_<arch>.tar.gz` (3.51.1), sha256-verified, packages
   `/usr/bin/task` + bash/zsh/fish completions. `build-all.sh` globs
   `packages/*/`, so it's auto-built. **Verified:** `bash packages/task/build.sh`
   → `task_3.51.1-1foundry1_amd64.deb`; packaged binary runs `task --version` → 3.51.1.
2. **`scripts/check-apt-repos.sh`** + `task check-apt-repos` — resolves each
   shipped `deb` line's `InRelease` (falls back to `Release`) and fails on
   non-200. `--host` also checks the running machine. **Verified:** passes the 4
   ISO sources; catches the dead `any-distro` line (exit 1); `-h` clean.

## Follow-up (Phase 2 — gated on the foundry-apt publish; NOT done here)

The consumer-flip must come **after** foundry-apt actually serves `task`, or a
built image's `apt install foundry-anvil` (Depends: task) would have no source.
Sequence:

1. Publish foundry-apt (its apt-`v*` pipeline → R2) so `apt.foundrylinux.org`
   serves `task 3.51.1-1foundry1`. Confirm live (`task check-apt-repos` already
   covers apt.foundrylinux.org; also check the `task` Version in its Packages).
2. Drop the Cloudsmith `task` source from the consumers:
   - `foundry-iso/config/archives/cloudsmith-task.list.chroot` (delete) + the
     `cloudsmith-task` key fetch/copy in `foundry-iso/scripts/build-iso.sh`.
   - `foundry-devbox/Dockerfile`, `foundry-setup/install-task.sh`,
     `site/setup.sh` — remove the `setup.deb.sh | bash` lines; `task` now comes
     from foundry-apt via `Depends:`.
3. Wire `task check-apt-repos` into the ISO/release preflight (host-side, before
   the container build) so a future upstream rotation fails fast, loudly.

## Out of scope / notes
- Keeping the Cloudsmith source alongside foundry-apt is harmless in the interim
  (apt dedupes by name+version); Phase 2 is cleanup + decoupling, not a fix.
- Bumping task later: edit `packages/task/build.sh` (`UPSTREAM_VERSION` + the two
  `SHA256_*` from the release `task_checksums.txt`) and add a `debian/changelog`
  entry.

## Verification (this change)
1. `bash foundry-apt/packages/task/build.sh` → builds `task_3.51.1-1foundry1_amd64.deb`; `dpkg-deb -x` binary → `task --version` = 3.51.1.  **PASS**
2. `task check-apt-repos` → 4/4 reachable, exit 0.  **PASS**
3. `bash scripts/check-apt-repos.sh <dead-any-distro-line>` → exit 1, offender listed.  **PASS**
4. `task check-apt-repos -- -h` → usage, exit 0, no network.  **PASS**
