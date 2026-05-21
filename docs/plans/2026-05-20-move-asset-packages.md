# Plan: move `blender-asset-finder` + rename/move `wf-asset` → `blender-asset-finder-cli`

## Context

`blender-asset-finder` (the Blender add-on) and `wf-asset` (the CLI front-end) are not
WorldFoundry-specific — they work with any Blender project or asset workflow. Both belong
in `apt.foundrylinux.org` alongside `foundry-retro-tools`.

`wf-asset` gets renamed to `blender-asset-finder-cli` to make the relationship with the
add-on obvious: same provider library, two UIs (Blender add-on vs. shell).

No existing users, so no transitional stubs.

## What moves where

| Package | From | To | Rename? |
|---|---|---|---|
| `blender-asset-finder` | `worldfoundry.org/apt/packages/` | `foundrylinux.org/foundry-apt/packages/` | No |
| `wf-asset` | `worldfoundry.org/apt/packages/` | `foundrylinux.org/foundry-apt/packages/` | → `blender-asset-finder-cli` |

## Cross-repo dependency note

After the move, `worldfoundry-cli` and `worldfoundry-blender` (in `apt.worldfoundry.org`)
will depend on packages from `apt.foundrylinux.org`. apt resolves across all configured
sources — this works fine as long as both repos are set up. The Foundry Linux install scripts
already wire both sources (`setup-worldfoundry-apt-source.sh` + `setup-foundry-apt-source.sh`).

## Changes in `foundrylinux.org/`

### New: `foundry-apt/packages/blender-asset-finder/`

Copy from `worldfoundry.org/apt/packages/blender-asset-finder/`, then:
- Update `Maintainer:` → `World Foundry <packages@worldfoundry.org>` (already correct)
- Update `Homepage:` if needed
- Bump changelog with a "moved to apt.foundrylinux.org" entry

### New: `foundry-apt/packages/blender-asset-finder-cli/`

Copy from `worldfoundry.org/apt/packages/wf-asset/`, then:
- Rename `Source:` and `Package:` → `blender-asset-finder-cli`
- Update `Description:` — drop "WorldFoundry asset CLI" framing, describe as the CLI
  companion to the `blender-asset-finder` Blender add-on
- Update `build.sh` `NAME=` variable
- Bump changelog with rename + move entry

## Changes in `worldfoundry.org/`

| File | Change |
|---|---|
| `apt/packages/blender-asset-finder/` | **Delete** (moved to foundrylinux.org) |
| `apt/packages/wf-asset/` | **Delete** (moved + renamed to foundrylinux.org) |
| `apt/packages/worldfoundry-cli/debian/control` | `Depends: wf-asset (>= 0.1.0)` → `blender-asset-finder-cli (>= 0.1.0)` + bump changelog |
| `apt/packages/worldfoundry-cli/debian/control` | Update description: `wf-asset` → `blender-asset-finder-cli` |
| `apt/packages/worldfoundry/debian/control` | Update description mentions of `wf-asset` → `blender-asset-finder-cli` |
| `apt/packages/worldfoundry-blender/debian/control` | `blender-asset-finder` dep name unchanged; update description cross-reference to mention `blender-asset-finder-cli` |

## Implementation steps

1. Copy `blender-asset-finder/` into `foundrylinux.org/foundry-apt/packages/`, bump changelog.
2. Copy `wf-asset/` into `foundrylinux.org/foundry-apt/packages/blender-asset-finder-cli/`,
   rename Source/Package/NAME, update description, bump changelog.
3. Build foundry-apt: `task foundry-apt:build` — expect both new debs.
4. Lintian both new debs — expect empty output.
5. Delete `blender-asset-finder/` and `wf-asset/` from `worldfoundry.org/apt/packages/`.
6. Update `worldfoundry-cli` Depends + description + changelog.
7. Update `worldfoundry` description.
8. Build worldfoundry.org apt: `task apt:build` — confirm no `blender-asset-finder_*.deb`
   or `wf-asset_*.deb`, and worldfoundry-cli builds clean.
9. Commit both repos, push, bump both tags → CI publishes both.

## Verification

1. **foundry-apt builds both new debs.**
   ```
   ls foundry-apt/dist/blender-asset-finder_*.deb
   ls foundry-apt/dist/blender-asset-finder-cli_*.deb
   ```
   ```
   blender-asset-finder_0.2.0+git0a19d26c-2foundry1_all.deb  (341866 bytes)
   blender-asset-finder-cli_0.1.0+git0a19d26c-1foundry1_all.deb  (12470 bytes)
   ```
   PASS

2. **Both lintian-clean.**
   Empty output from lintian on each.
   ```
   (no output)
   ```
   PASS

3. **worldfoundry.org dist has neither old package.**
   ```
   ls apt/dist/ | grep -E "^(wf-asset|blender-asset-finder)_"
   # expect: no output
   ```
   ```
   (no output)
   ```
   PASS

4. **Complete package inventory — `apt.foundrylinux.org`** (after publish):
   ```
   curl -s https://apt.foundrylinux.org/dists/resolute/main/binary-amd64/Packages \
     | grep '^Package:' | awk '{print $2}' | sort
   ```
   Expected (9 packages — includes mobile-development plans shipped first):
   ```
   blender-asset-finder
   blender-asset-finder-cli
   f9dasm
   foundry-android-development
   foundry-ios-development
   foundry-retro-tools
   ghidra
   libvgm
   vgmstream
   ```

5. **Complete package inventory — `apt.worldfoundry.org`** (after publish):
   ```
   curl -s https://apt.worldfoundry.org/dists/stable/main/binary-amd64/Packages \
     | grep '^Package:' | awk '{print $2}' | sort
   ```
   Expected (14 packages — down from 16):
   ```
   cdpack
   iffcomp
   iffdump
   levcomp
   lvldump
   oaddump
   oas2oad
   prep
   textile
   worldfoundry
   worldfoundry-blender
   worldfoundry-blender-editor-exporter
   worldfoundry-cli
   worldfoundry-development
   ```

6. **Cross-repo resolution** — in fresh ubuntu:26.04 with both sources configured,
   `apt install worldfoundry` resolves all packages including `blender-asset-finder`
   and `blender-asset-finder-cli` from apt.foundrylinux.org.

---

## Status: done (verified 2026-05-22)

Steps 1–3 PASS inline above (recorded at implementation time). Steps 4–5 verified post-publish today:

### 4. `apt.foundrylinux.org` inventory — PASS (subset match)

```
$ curl -s https://apt.foundrylinux.org/dists/resolute/main/binary-amd64/Packages.gz \
    | gunzip | grep '^Package:' | awk '{print $2}' | sort | grep -E '^(blender-asset-finder|blender-asset-finder-cli)$'
blender-asset-finder
blender-asset-finder-cli
```

The 9-package list in the plan was a snapshot from 2026-05-20; the live repo now serves 25 packages (the extra 16 are the nested-editions metapackages — `foundry-anvil`, `foundry-sprite`, `foundry-atelier`, `foundry-emulators*`, `foundry-art`, `foundry-daw`, `foundry-free-games`, `foundry-game-frameworks`, `foundry-game-reimplementations`, `foundry-image-cli`, `foundry-pixel-art`, `foundry-trackers` — landed in a later commit). Both moved packages are present, which is what this plan was verifying.

### 5. `apt.worldfoundry.org` inventory — PASS (no `wf-asset` or `blender-asset-finder`)

```
$ curl -s https://apt.worldfoundry.org/dists/stable/main/binary-amd64/Packages.gz \
    | gunzip | grep '^Package:' | awk '{print $2}' | sort
cdpack
iffcomp
iffdump
levcomp
lvldump
oaddump
oas2oad
prep
textile
worldfoundry
worldfoundry-blender-addons
worldfoundry-blender-editor-exporter
worldfoundry-cli
worldfoundry-development
```

Matches the plan's expected 14 minus `worldfoundry-blender` (replaced by `worldfoundry-blender-addons` in a subsequent split — unrelated to this plan). No `wf-asset` and no `blender-asset-finder` in the worldfoundry index.

### 6. Cross-repo resolution — not independently re-verified

`worldfoundry-cli/debian/control` Depends list now reads `blender-asset-finder-cli (>= 0.1.0)` (apt.foundrylinux.org-side package), and apt resolves transitively across configured sources. Foundry Linux install scripts already wire both keyrings + sources.list.d entries.
