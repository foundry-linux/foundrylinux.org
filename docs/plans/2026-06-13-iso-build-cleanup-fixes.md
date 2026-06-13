# ISO Build Cleanup Fixes

**Date:** 2026-06-13  
**Goal:** Fix two bugs that caused ISO builds to fail unrecoverably without manual `sudo` intervention after a disk-full abort.

## Context

A `task iso-build` run failed mid-build when xorriso ran out of disk space:

```
xorriso : FAILURE : Image size 2124040s exceeds free space on media 966656s
```

The disk was at 98% before the build started (12 GB free). The live-build chroot consumed the remaining space before the ISO could be written. Manual cleanup (`sudo rm -rf foundry-iso/chroot foundry-iso/binary foundry-iso/cache`) was needed, but `.build/` was not removed.

The next `task iso-build` then failed silently: the docker container's `chattr -R -i .build/` was ineffective (e2fsprogs not installed → `chattr` not found), so `rm -rf .build/` left the stale `.build/bootstrap` sentinel in place. `lb_testroot` saw the sentinel and skipped debootstrap, then tried to use the empty chroot → `chroot: failed to run command '/usr/bin/env': No such file or directory`.

`task iso-clean` (the intended recovery path) also failed with `E: Unable to locate package live-build` because it ran `apt-get install` without `apt-get update` first on a fresh docker container with empty package lists.

## Root Causes

1. **`iso-clean` missing `apt-get update`** — fresh `ubuntu:26.04` containers have no package lists; install fails immediately.

2. **`iso-clean` wrong working directory** — ran `lb clean --purge` from `-w /work/config` instead of `-w /work` (the live-build project root), causing `E: /work/config is not a good Debian Live working directory to clean.`

3. **`build-iso.sh` unconditionally creates `cache/bootstrap/` before `lb bootstrap`** — the script writes a DNS pre-seed (`nameserver 8.8.8.8`) to `cache/bootstrap/run/systemd/resolve/stub-resolv.conf` before every build. `lb_bootstrap_cache restore` checks only `if [ -d cache/bootstrap ]` — if the directory exists it copies the entire directory into `chroot/`, creates the `.build/bootstrap` sentinel, and exits. When the cache is populated from a real previous build, this is correct. When `cache/` was deleted after the disk-full abort, the pre-seed unconditionally recreates `cache/bootstrap/` with only a DNS file. On the next build, `lb_bootstrap_cache restore` finds the directory, copies just the DNS stub into `chroot/`, marks bootstrap done, and debootstrap never runs — leaving `chroot/` with no `/usr/bin/env`.

## Fixes

### 1. `iso-clean` — add `apt-get update` and fix working directory

`Taskfile.yml`, task `iso-clean`:

- Added `apt-get update -qq &&` before `apt-get install`
- Changed `-w /work/config` → `-w /work` so `lb clean --purge` runs from the project root
- Added `chattr -R -i .build/ chroot/` before `lb clean` (now that `e2fsprogs` is available after install)
- Changed `rm -f /work/.lock` → `rm -f .lock` (path is relative to `-w /work`)
- Added `--privileged` to the docker run (needed for `chattr` to work)

### 2. `build-iso.sh` — no apostrophes/contractions in comments inside the `bash -c '...'` block

The entire docker build runs inside a single-quoted `bash -c '...'` string. The script already documents this (lines 114-116): *"NO apostrophes or single quotes... ANYWHERE here, comments included — each one would end the bash -c string."* The comment added for fix #3 contained `doesn't`, whose apostrophe terminated the string prematurely. Commands after the break (`lb bootstrap`, chroot operations) ran on the host where `lb` doesn't exist, producing `scripts/build-iso.sh: line N: lb: command not found`.

Fixed by changing `doesn't` → `does not` in the comment.

### 3. `build-iso.sh` — guard DNS pre-seed on cache actually existing

`foundry-iso/scripts/build-iso.sh`, DNS pre-seed block:

- Wrapped `mkdir -p cache/bootstrap/...` + DNS write in `if [ -d cache/bootstrap/usr ]` — only pre-patches DNS when a real bootstrap cache is present (real cache has `usr/`, a fresh or deleted cache does not)
- Added `e2fsprogs` to the apt-get install list so `chattr` is available for the existing `.build/`/`chroot/` cleanup step

## Verification

1. `task iso-clean` runs cleanly and removes `.build/`, `chroot/`, `dist/`

```
[2026-06-13 07:53:02] lb_clean --purge
P: Cleaning chroot
...
```

```
ls foundry-iso/.build 2>/dev/null && echo "remains" || echo "gone"
```

```
state gone
```

PASS

2. `task iso-build` completes and produces a `.iso` in `foundry-iso/dist/`

```
-rw-rw-rw- 1 will will 4.1G Jun 13 15:41 foundry-iso/dist/foundry-anvil-0.9.116-amd64.iso
=== ISO ready: foundry-iso/dist/foundry-anvil-0.9.116-amd64.iso (4.1G) ===
```

PASS

3. `asar-snes --version` is callable after installing from the ISO

```
(pending boot test)
```

PENDING
