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

## ~~Follow-up (Phase 2 — gated on the foundry-apt publish)~~ — REVERSED 2026-08-05

The consumer-flip was gated on foundry-apt actually serving `task` (confirmed
live 2026-08-05 — `apt.foundrylinux.org`'s `Packages` index carried both
`task` and `xemu`), and was implemented + verified per-consumer: `foundry-iso`
(deleted `cloudsmith-task.list.chroot`/`.key`, dropped the two `fetch_key`/`cp`
lines in `build-iso.sh`, wired `check-apt-repos` as an `iso-build` Task dep),
`foundry-devbox` (dropped the `setup.deb.sh | bash` line, real `docker build`
of the Dockerfile with `dl.cloudsmith.io` blackholed via `--add-host` still
succeeded), `foundry-setup/install-task.sh` (now calls the same
`setup-foundry-apt-source.sh` helper `install-foundry-android-development.sh`
already used), `site/setup.sh` (block deleted).

**Then reversed, pre-commit, on explicit direction: don't vendor `task`.**
Everything above was `git restore`d back to Cloudsmith-wired before any commit
landed — none of the four consumers changed. **Decision:** un-vendor `task`
from foundry-apt instead (second retirement — see `LICENSES-VENDORED.md`'s
Retired entries; it was first retired in favour of Cloudsmith, then
re-vendored in commit `23809cc`, now retired again). Done in this session:

1. Deleted `foundry-apt/packages/task/` (`build.sh` + `debian/`).
2. Updated `foundry-apt/README.md` (dropped the `task` row from the vendored
   table), `foundry-apt/LICENSES-VENDORED.md` (moved `task` from the vendored
   table to Retired entries with a reason), and root `CLAUDE.md` (27→26
   vendored upstreams). `task check-licenses` green.
3. `foundry-core`'s `debian/control` keeps `Depends: task` unchanged — it
   still resolves, just from Cloudsmith (which every consumer still wires),
   not from this repo.

**Still open — not released:** the source deletion is committed but
`apt.foundrylinux.org` is still *serving* the orphaned `task 3.51.1-1foundry1`
build from before this change, because nothing has re-triggered
`foundry-apt`'s publish pipeline. `scripts/prune-dist.sh` already handles this
case by design (drops any `dist/*.deb` whose package name has no matching
`packages/*/debian/control`, exactly the scenario the `asar-snes` rename
comment describes) and the R2 sync step is `rclone sync` (mirror semantics,
not additive), so the **next** `foundry-apt` release tag will fully drop
`task` from the live repo — pool object, `Packages`/`Release` entries, all of
it. Triggering that release (`task sync-and-release TAG=…` from the monorepo
root) pushes to the public `foundry-linux/foundry-apt` GitHub repo and
signs+publishes to the live R2-backed `apt.foundrylinux.org` — real production
impact on a repo other machines already depend on, so it was left for an
explicit go-ahead rather than bundled into this pass.

## Out of scope / notes
- Bumping task later, if ever re-vendored a third time: edit
  `packages/task/build.sh` (`UPSTREAM_VERSION` + the two `SHA256_*` from the
  release `task_checksums.txt`) and add a `debian/changelog` entry.

## Verification (this change)
1. `bash foundry-apt/packages/task/build.sh` → builds `task_3.51.1-1foundry1_amd64.deb`; `dpkg-deb -x` binary → `task --version` = 3.51.1.  **PASS**
2. `task check-apt-repos` → 4/4 reachable, exit 0.  **PASS**
3. `bash scripts/check-apt-repos.sh <dead-any-distro-line>` → exit 1, offender listed.  **PASS**
4. `task check-apt-repos -- -h` → usage, exit 0, no network.  **PASS**
