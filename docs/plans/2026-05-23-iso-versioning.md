# Plan: ISO versioning — 0.9.x pre-release scheme

## Context

The repo is not yet public. Until it is, ISOs should be versioned `0.9.x` rather than `1.0`. On public release the version will jump to `1.0.0`. Additionally, the version should increment with each upload cycle so individual builds are distinguishable. Currently "1.0" is hardcoded in ~10 places; this plan centralises it in a single `foundry-iso/VERSION` file.

## Changes

### 1. New `foundry-iso/VERSION`

Single line: `0.9.0`

All scripts derive `ISO_VERSION` from this file. If it's missing, they fall back to the file's contents or error out.

### 2. All pipeline scripts — replace hardcoded `1.0` with `$ISO_VERSION`

Each script adds near the top (after `SCRIPT_DIR` is set):
```bash
ISO_VERSION="$(cat "$SCRIPT_DIR/../VERSION")"
```
Then replaces every `1.0` in filename/variable construction with `$ISO_VERSION`.

Files to update (all in `foundry-iso/scripts/`):
- `build-iso.sh` — `ISO_DST` rename line; also pass `-e ISO_VERSION="$ISO_VERSION"` to the Docker `run` call so the inner `bash -c` block can use it
- `sign-iso.sh` — `ISO=` path, manifest `"version"` field, manifest `"filename"` field
- `create-torrents.sh` — `ISO=` path, `DISPLAY_NAME`
- `upload-iso.sh` — `ISO=` path, `VERSIONED=` variable
- `inject-efi.sh` — takes ISO path as argument, no version hardcoding; no change needed
- `build-vm-images.sh` — `ISO=` path, `IMAGE_BASE`
- `upload-vm-images.sh` — `IMAGE_BASE`

### 3. `foundry-iso/config/auto/config` — dynamic volume label

Change:
```bash
--iso-volume "FOUNDRY-${EDITION^^}-1.0" \
```
to:
```bash
--iso-volume "FOUNDRY-${EDITION^^}-${ISO_VERSION:-0.9.0}" \
```

`ISO_VERSION` arrives as an env var passed by `build-iso.sh` via Docker's `-e` flag.

### 4. `foundry-iso/config/binary` — update comment only

The `LB_ISO_VOLUME` value here is overridden by `auto/config`. Change the hardcoded default in the comment from `1.0` to `0.9.0` for clarity; the functional line can be removed or left as a no-op comment since `auto/config` always wins.

### 5. New `foundry-iso/scripts/bump-version.sh`

Increments the patch component of `VERSION` (e.g. `0.9.0` → `0.9.1`) and prints the old and new value. Does not commit — the caller decides when to commit.

```
Usage: bash scripts/bump-version.sh
```

### 6. New `task iso-bump` in `Taskfile.yml`

```yaml
iso-bump:
  desc: "Bump the ISO patch version in foundry-iso/VERSION (e.g. 0.9.0 → 0.9.1) and commit"
  dir: foundry-iso
  cmds:
    - bash scripts/bump-version.sh
    - git add VERSION
    - git commit -m "chore(foundry-iso): bump ISO version to $(cat VERSION)"
```

### 7. `Taskfile.yml` — fix `iso-smoke` path

The existing task hardcodes `1.0`:
```
dist/foundry-{{.EDITION | default "anvil"}}-1.0-amd64.iso
```
Change to read from `VERSION`:
```
dist/foundry-{{.EDITION | default "anvil"}}-$(cat foundry-iso/VERSION)-amd64.iso
```

### 8. `site/sections.jsx` — version in magnet `dn=` parameter

The magnet links contain `dn=foundry-anvil-1.0-amd64`. These are already updated manually whenever create-torrents.sh regenerates them (because the infohash changes and we update sections.jsx). With `DISPLAY_NAME` now derived from `ISO_VERSION` in create-torrents.sh, the new magnet URIs written to `.magnet` files will automatically use the correct version. The manual sections.jsx update step remains the same.

## Workflow after this change

```
# First time (or before a new build cycle):
task iso-bump          # 0.9.0 → 0.9.1, commits VERSION

# Build:
EDITION=anvil bash foundry-iso/scripts/build-iso.sh     # produces foundry-anvil-0.9.1-amd64.iso

# Sign + torrent:
EDITION=anvil task iso-sign                              # sha256, gpg, torrent, manifest

# Upload:
EDITION=anvil task iso-upload                            # uploads foundry-anvil-0.9.1-amd64.iso
                                                         # + foundry-anvil-latest-amd64.iso alias
```

## Immediate work (post-plan approval)

1. Create `foundry-iso/VERSION` with `0.9.0`
2. Update all scripts (steps 2–4 above)
3. Add `bump-version.sh` and `task iso-bump`
4. Fix `iso-smoke` task path
5. Rename the already-built ISOs in `dist/` from `*-1.0-*` to `*-0.9.0-*` (mv, no rebuild needed)
6. Re-sign both ISOs under the new name, recreate torrents, update manifests
7. Upload to R2 under both versioned (`0.9.0`) and `latest` names; the old `1.0` objects on R2 can stay until pruned
8. Update `site/sections.jsx` magnet links from new `.magnet` files, rebuild site, push

## Verification

1. `cat foundry-iso/VERSION` → `0.9.0`
2. `EDITION=anvil bash scripts/build-iso.sh --help` exits 0
3. `task iso-bump` → VERSION becomes `0.9.1`, git log shows the bump commit
4. `curl -s https://iso.foundrylinux.org/foundry-anvil-latest-amd64.iso.sha256` matches `dist/foundry-anvil-0.9.0-amd64.iso.sha256`
5. `curl -sI https://iso.foundrylinux.org/foundry-anvil-0.9.0-amd64.iso` → 200 (versioned object exists on R2)
