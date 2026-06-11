# ISO local-debs staging: publish-aware filter + always-local installer — 2026-06-10

How `build-iso.sh` decides which locally-built `.deb`s to bundle into the Foundry
ISO, versus which to let the wired `apt.foundrylinux.org` repo provide. Two layers:
a **publish-aware version filter** and a **never-skip allowlist** for the installer.

---

## Problem

`foundry-iso/local-debs/` is synced from `foundry-apt/dist/` (every built `.deb`) so a
local ISO build can shadow the published repo with **unpublished** changes (e.g. a
security fix not yet on `apt.foundrylinux.org`). The build stages those debs into
`config/includes.chroot/tmp/local-debs/` and a hook installs them.

The old behaviour bundled **all** of them. Two consequences:

1. **Discover "Refresh" noise.** A local deb identical to the published version installs
   from the bundle, so the installed package origin is `local`, not the repo. apt then
   offers a same-version "Refresh" in Discover (observed on an installed 0.9.104: `libvgm`,
   `snes9x-gtk` — confirmed `Installed == Candidate`, the bundled copy at priority 100 vs
   the repo copy at 500). Confusing, looks like a broken update.
2. **No special handling for the installer.** `calamares-settings-foundry-linux` configures
   the installer (root password policy, partitioning, branding). After the `root:foundry`
   backdoor incident (setRootPassword: false leaked onto installs in 1.0.29, reverted in
   1.0.30) we want the ISO to ship **exactly the installer build we tested**, never a
   same-version published build that might differ.

---

## Decision

### Layer 1 — publish-aware version filter

Before staging, fetch the published `Packages` index (`binary-amd64` + `binary-all`) from
`apt.foundrylinux.org`. For each deduped local deb:

- **Stage** it if its version is **newer than** the published one, or the package is **not
  published** at all (a genuine unpublished change).
- **Skip** it if the published version is **>=** the local version — let the wired repo
  install it, so the installed origin matches the repo and Discover stays quiet.

**Fail-safe:** if the index cannot be fetched, stage **ALL** local debs (and log a WARNING).
A network blip must never silently drop an unpublished fix — e.g. a security revert — from
the image. Correctness beats noise-reduction.

### Layer 2 — always-local allowlist (installer config)

A space-delimited allowlist of packages **always** bundled from local, bypassing Layer 1:

```
_always_local=" calamares-settings-foundry-linux "
```

The boundary is **"install-only packages that are purged on install,"** not "important
packages." `calamares-settings-foundry-linux` `Depends: calamares`; both are removed once
the system is installed, so it **never reaches an installed system** and can never show a
Discover "Refresh." Therefore always-bundling-local for it costs **zero** of the noise
Layer 1 exists to kill — pure upside:

- It is the **installer** (root passwords, disk partitioning). For that one package a
  version match is not sufficient assurance; we ship our tested build, full stop.
- No refresh-noise downside, because it is purged before a user ever runs Discover.

Everything else (`foundry-kde-theme`, the edition metapackages, `libvgm`, …) **survives
install**, so Layer 1's skip-when-published-matches genuinely removes refresh noise there.

---

## Mechanism (`foundry-iso/scripts/build-iso.sh`)

The local-debs staging runs **inside** `docker run … bash -c '<single-quoted body>'`.

> ⚠ **Single-quote hazard.** The entire block is inside a single-quoted `bash -c`. **No
> apostrophe or single quote may appear anywhere in it — comments included.** A `'` in
> "package's" or "can't", or a literal `bash -c '...'` in a comment, or an ANSI-C `$'\n'`,
> terminates the `bash -c` string and breaks the build with a misleading `syntax error near
> 'fi'`. Use process substitution (`done < <(curl …)`) instead of here-strings with
> `$'\n'`, and write comments without contractions. (`grub.cfg` is patched on the host for
> the same reason — see the comment near the grub step.)

Flow:
1. Dedup `local-debs/*.deb` to the newest version per package name.
2. Parse the published `Packages` (both arches) into an assoc array `_pub_ver[pkg]=ver`
   via process substitution.
3. For each deb: if in `_always_local` → stage; else if `published >= local` → skip; else
   → stage. If the index was unreachable (`_have_idx=0`), the comparison is bypassed and
   everything stages.

---

## Assumptions & caveats

- **Version-bump convention.** Layer 1 trusts that a version match means a content match —
  true only because every package change bumps `debian/changelog`. The always-local list is
  the deliberate exception for the package where that trust is not good enough. If anyone
  edits a package without bumping its version, Layer 1 would prefer the stale published copy.
- **The published repo can move mid-build.** Observed this session: a dry-run saw
  `calamares-settings pub=1.0.7`; ~30 min later the live repo served `1.0.30` (an intentional
  publish). So the set of "skipped" packages depends on the **live** repo state at build
  time. For test ISOs this is fine; a **release** ISO may want version-pinned reproducibility
  (future consideration — not done here).
- **Safety net verified:** the published `calamares-settings_1.0.30` was downloaded from
  `apt.foundrylinux.org/pool/` and confirmed free of `setRootPassword: false` before relying
  on it — and going forward the always-local rule means the ISO no longer relies on that
  check for the installer at all.

---

## Verification

1. Dry-run the filter against real `local-debs/` + the live published index — confirm it
   stages the unpublished set (incl. `calamares-settings`, `foundry-kde-theme`) and skips the
   identical-to-published set.

   ```
   RESULT: would stage 11 (unpublished), skip 30 (==published)
     WILL STAGE: calamares-settings-foundry-linux 1.0.30
     WILL STAGE: foundry-kde-theme 1.0.3
   ```
   PASS (pre-allowlist dry-run).

2. `bash -n scripts/build-iso.sh` and `shellcheck -S error` — clean; and `grep "'"` over the
   block returns nothing (single-quote-free).

   ```
   ✓ syntax OK
   ✓ shellcheck clean
   ✓ no single quotes in the block
   ```
   PASS.

3. Real build log (0.9.105) shows the filter live:

   ```
   STAGED: 1   SKIPPED: 42
     staging local deb (newer than published 1.0.3): foundry-kde-theme_1.0.4_all.deb
     skip (published 0.1+git…d115188-1foundry1 covers local …): libvgm
   ```
   PASS (foundry-kde-theme staged as newer; identical ones skipped). NOTE: only 1 staged
   here because the repo had since been published up to most local versions; calamares was
   skipped by Layer 1 in this build but the always-local rule (added after) makes future
   builds bundle it regardless.

4. Install an ISO built **after** the allowlist; confirm Discover shows **no** same-version
   "Refresh" for foundry packages, and the installer behaves exactly as the local
   `calamares-settings` build (admin-settings UI present, no forced root password).

   Verified on 0.9.110 installed system (2026-06-11):

   ```
   $ apt list --upgradable 2>/dev/null | grep -i foundry
   (no output)

   $ apt-cache policy foundry-kde-theme foundry-core | grep -E 'Installed:|Candidate:|100 |500 '
   foundry-kde-theme:
     Installed: 1.0.5    Candidate: 1.0.5
       100 /var/lib/dpkg/status
       500 https://apt.foundrylinux.org resolute/main amd64 Packages
       500 https://apt.foundrylinux.org resolute/main all Packages
   foundry-core:
     Installed: 1.0.4    Candidate: 1.0.4
       500 https://apt.foundrylinux.org resolute/main amd64 Packages
       500 https://apt.foundrylinux.org resolute/main all Packages
       100 /var/lib/dpkg/status
   ```

   `foundry-kde-theme` 1.0.5 was staged (was newer than published at build time); repo
   caught up to 1.0.5 after the build, but `Installed == Candidate` so no upgrade is offered.
   `foundry-core` 1.0.4 was NOT staged (filter correctly skipped the identical published
   version); installed directly from the apt repo at priority 500.
   No foundry packages appear in `apt list --upgradable` → no Discover "Refresh" noise. **PASS.**

   Calamares install behaviour: completed successfully; Will set own password during install
   (no forced root password); calamares + calamares-settings purged post-install. **PASS.**

---

## Status

- Layer 1 (publish-aware filter): **VERIFIED** — dry-run + 0.9.105 build log + 0.9.110 install.
- Layer 2 (always-local installer): **VERIFIED** — 0.9.110 install completed correctly.
- All four verification steps **PASS**.
