# Move stellarium from the anvil edition to atelier

**Date:** 2026-07-11
**Status:** Done — shipped in foundry-anvil 1.0.6 / foundry-atelier 0.9.6, verified
absent from the anvil ISO (foundry-anvil 0.9.124)

## Context

`stellarium` (3D planetarium) was a direct `Depends` of `foundry-anvil`, the
**default** ISO edition. Its data package `stellarium-data` is ~352 MB — noticed
while watching an ISO build download it (`Get:54 … stellarium-data … [352 MB]`).
That is disproportionate weight for the default edition, whose identity is a game
development workstation, not an astronomy suite.

It was added on 2026-06-30 alongside `kdegames`, `step`, `kig`, and `simutrans`
(see `2026-06-30-add-games-simulators.md`). Only stellarium moves; the others stay
in anvil (per user decision — the size driver is stellarium's data set alone).

## Edition model (why "add to atelier" = "move")

```
foundry-core ⊆ foundry-anvil ⊆ foundry-sprite ⊆ foundry-atelier
```

atelier `Depends: foundry-sprite`, which transitively pulls anvil. So a package
listed directly in anvil is inherited by sprite and atelier. Removing it from
anvil and adding it to atelier's own `Depends` means **only the complete edition**
ships it — sprite no longer carries it either. That is the intended "move to the
heaviest edition only."

## Changes

- **`foundry-apt/packages/foundry-anvil/debian/control`** — drop the `stellarium,`
  line from `Depends` (keep `kdegames`, `step`, `kig`, `simutrans`).
- **`foundry-apt/packages/foundry-atelier/debian/control`** — add `stellarium` to
  `Depends` (as a raw package dep, like `ghidra`).
- Changelog bumps (dep change → patch bump per repo convention):
  `foundry-anvil` `1.0.5 → 1.0.6`, `foundry-atelier` `0.9.5 → 0.9.6`.

## Verification

1. `cd foundry-apt && task build PKG=foundry-anvil && task build PKG=foundry-atelier`.
2. Deb-level dep check:
   - `dpkg-deb -f dist/foundry-anvil_1.0.6_all.deb Depends` → **no** stellarium.
   - `dpkg-deb -f dist/foundry-atelier_0.9.6_all.deb Depends` → **has** stellarium.
3. ISO-level (definitive): rebuild the anvil ISO (`task iso-build`) and grep the
   live-build package manifest for absence — no boot needed to prove a negative:
   ```bash
   xorriso -osirrox on -indev dist/foundry-anvil-<ver>-amd64.iso \
     -extract /live/filesystem.packages /tmp/fs.packages
   grep -c '^stellarium' /tmp/fs.packages   # → 0
   ```
   Result on `0.9.124`: **stellarium 0 matches**; `kdegames`/`kig`/`step`/`simutrans`
   still present; ISO shrank **4.9 GB → 4.4 GB**.

## Out of scope / follow-up

- The **atelier** ISO (~10 GB) was not rebuilt; the metapackage dep is
  authoritative, but a full atelier build would confirm stellarium lands there.
- No change to `step`/`kig`/`kdegames`/`simutrans` — all remain in anvil.
