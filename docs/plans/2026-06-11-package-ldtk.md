# Package LDtk and restore monorepo publish secrets

**Date:** 2026-06-11  
**Status:** Complete

## Context

LDtk (Level Designer Toolkit) is a popular open-source 2D level editor by deepnight, MIT-licensed.
A `wip-shelf/ldtk/` package had been partially authored on 2026-06-10 (build.sh + full debian/ tree,
v1.5.3, AppImage-repack approach) but never promoted to `packages/` or wired into a metapackage.

Separately, the monorepo's publish CI had been failing at "Sign Release" since its first run
(2026-06-05) because the four required GitHub Actions secrets (`GPG_PRIVATE_KEY`, `R2_ACCESS_KEY_ID`,
`R2_SECRET_ACCESS_KEY`, `R2_ENDPOINT`) were never migrated from the old standalone
`foundry-linux/foundry-apt` repo when the monorepo was created.

## What was done

### 1. Promote and build LDtk

- Moved `foundry-apt/wip-shelf/ldtk/` → `foundry-apt/packages/ldtk/` (plain `mv`; wip-shelf
  was untracked).
- Removed now-empty `foundry-apt/wip-shelf/` directory.
- Fixed two bugs in `build.sh`:
  - `REPO_ROOT` navigated three levels up (`$SCRIPT_DIR/../../..`) instead of two, placing the
    output `.deb` in the container's root `/dist/` rather than `/work/dist/`. Changed to
    `$SCRIPT_DIR/../..`.
  - `dh_shlibdeps` aborted: the bundled `usr/lib/` Electron shims (`libindicator.so.7`,
    `libappindicator.so.1`, `libgconf-2.so.4`) have transitive system deps (`libgdk_pixbuf`,
    `libglib`, `libgtk2`, etc.) that are absent from the minimal `ubuntu:26.04` build container.
    `--ignore-missing-info` only covers missing `.shlibs` files, not absent libraries. Fix:
    override `dh_shlibdeps` to a no-op; declare explicit runtime `Depends:` in `debian/control`.
- Added explicit `Depends:` for Electron runtime: `libnss3`, `libgbm1`, `libasound2t64`,
  `libnotify4`, `libgtk-3-0t64`, `libglib2.0-0t64`, `libx11-6`, `libxext6`, `libdbus-1-3`,
  `libsecret-1-0`, `xdg-utils`.
- Added lintian overrides for expected pre-built binary warnings.

### 2. Wire into foundry-game-frameworks

- Added `ldtk` to `Depends:` and description bullet list in
  `packages/foundry-game-frameworks/debian/control`.
- Bumped `foundry-game-frameworks` to `1.0.2` in `debian/changelog`.

### 3. Restore monorepo publish secrets

Root cause: bootstrap.sh set secrets on `foundry-linux/foundry-apt`; the monorepo
`foundry-linux/foundrylinux.org` had only `CF_PAGES_ACCOUNT_ID` and `CF_PAGES_API_TOKEN`.

- Discovered secrets are backed up to R2 bucket `foundry-linux-secrets` (not `foundry-secrets`
  as hardcoded in bootstrap.sh — a second bug).
- Fixed `SECRETS_BUCKET` in `scripts/bootstrap.sh`: `foundry-secrets` → `foundry-linux-secrets`.
- Wrote `scripts/restore-gh-secrets.sh`: fetches `GPG_PRIVATE_KEY`, `R2_ACCESS_KEY_ID`,
  `R2_SECRET_ACCESS_KEY` from R2 and derives `R2_ENDPOINT` from account ID; sets all four on
  the monorepo via `gh secret set`.
- Ran `restore-gh-secrets.sh`; all four secrets confirmed set.
- Re-triggered `foundry-apt-publish.yml` via `gh workflow run`.

## Commits

- `6ad6d44` — `feat(foundry-apt): package LDtk 1.5.3 and add to foundry-game-frameworks`
- `4d2c8cc` — `fix(bootstrap): correct secrets bucket name foundry-secrets → foundry-linux-secrets`

## Verification

1. Build smoke-test: `docker run --rm -v $(pwd)/foundry-apt:/work -w /work ubuntu:26.04 bash -c 'apt-get update -qq && apt-get install -y -q dpkg-dev debhelper && bash scripts/build-all.sh ldtk'`

   ```
   === built: /work/dist/ldtk_1.5.3-1foundry1_amd64.deb ===
   -rw-r--r-- 1 root root 84M Jun 11 01:20 ldtk_1.5.3-1foundry1_amd64.deb
   ```
   PASS

2. Package metadata: `dpkg-deb --info foundry-apt/dist/ldtk_1.5.3-1foundry1_amd64.deb`

   ```
   Package: ldtk
   Version: 1.5.3-1foundry1
   Architecture: amd64
   Depends: libnss3, libgbm1, libasound2t64, libnotify4, libgtk-3-0t64,
            libglib2.0-0t64, libx11-6, libxext6, libdbus-1-3, libsecret-1-0, xdg-utils
   ```
   PASS

3. Payload layout: `dpkg-deb --contents ... | grep usr/bin`

   ```
   -rwxr-xr-x root/root  50 ./usr/bin/ldtk
   ```
   PASS — launcher, desktop entry, and icon all present.

4. Secrets set: `gh secret list --repo foundry-linux/foundrylinux.org`

   ```
   CF_PAGES_ACCOUNT_ID     2026-05-18
   CF_PAGES_API_TOKEN      2026-05-18
   GPG_PRIVATE_KEY         2026-06-11
   R2_ACCESS_KEY_ID        2026-06-11
   R2_ENDPOINT             2026-06-11
   R2_SECRET_ACCESS_KEY    2026-06-11
   ```
   PASS

5. Publish CI green — run [27328735408](https://github.com/foundry-linux/foundrylinux.org/actions/runs/27328735408) (in progress at time of writing; check for green).
