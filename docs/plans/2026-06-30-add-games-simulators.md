# Add KDE Games + Simulators to Anvil

**Date:** 2026-06-30
**Status:** Draft

## Goal

Add `kdegames`, `step`, `kig`, `stellarium`, and `simutrans` to the `foundry-anvil` edition (all game devs get these). No changes to `foundry-atelier`. Note that `0ad` + `0ad-data` and `openttd` are already present in `foundry-atelier` via `foundry-free-games`.

## Package inventory

Sizes are `Installed-Size` from `apt-cache show` on Ubuntu 26.04. Incremental cost on the ISO is lower since KDE Plasma and all shared Qt/KDE libs from `kubuntu-desktop` are already present.

| Package | Installed‑Size | Notes | Edition |
|---|---|---|---|
| `step` | 14 MiB | KDE physics sim — springs, particles, rigid bodies, gravity | anvil |
| `kig` | 11 MiB | KDE interactive geometry — constructions, curves, loci | anvil |
| `kdegames` | 12 KiB + ~150 MiB deps | Metapackage for 38 KDE games; shared KDE libs already on ISO | anvil |
| `stellarium` | 30 MiB | Real-time 3D planetarium; reference for space/sky renderers | anvil |
| `simutrans` | 5.7 MiB | Transport network sim; pathfinding and economy reference | anvil |
| `openttd` | 12.6 MiB | Already in `foundry-free-games` → already in atelier | *(present)* |
| `0ad` + `0ad-data` | 28 MiB + 3.4 GiB | Already in `foundry-free-games` → already in atelier | *(present)* |

Fresh bare-Ubuntu install sizes (KDE not pre-installed, cumulative):
- `kdegames` cold: 926 MiB
- `step` + `kig` on top: +118 MiB
- `stellarium` + `simutrans` on top: +1,021 MiB

On the ISO these cold numbers are not relevant — shared libs are already there.

## Rationale

**Anvil tier** (all game devs get these):

- `kdegames` — Kubuntu ships the full set by default. 38 games spanning every genre (arcade, puzzle, board, card, strategy). Good playable references. The ISO previously stripped `kde-games-*`; this plan reverses that.
- `step` — Direct utility for game devs learning physics simulation: springs, friction, gravity, particle systems. KDE-native, small.
- `kig` — Interactive geometry; useful for understanding curves, beziers, and geometric constructions that show up in tooling. Previously in the ISO strip list; removed here.
- `stellarium` — Photo-realistic sky rendering with astronomical accuracy. Reference for space/sky scene renderers.
- `simutrans` — Open-source transport network simulation with sophisticated pathfinding and economy. Strong procedural generation and AI reference.

## Changes

### 1. `foundry-apt/packages/foundry-anvil/debian/control`

Add `kdegames`, `step`, `kig`, `stellarium`, `simutrans` to `Depends:`.

### 2. `foundry-apt/packages/foundry-anvil/debian/changelog`

Bump `1.0.4` → `1.0.5`.

### 3. `foundry-iso/config/package-lists/strip.list.chroot.purge`

Remove `kde-games-*` and `kig` from the purge list. Keep `kalgebra kgeography kbruch` stripped (math edu tools, not in scope).

## Verification

1. Build the metapackages: `task build`

   ```
   === Building foundry-anvil (canonical debian/ source format) ===
   OK   dist/foundry-anvil_1.0.5_all.deb  (2432 bytes)
   OK   dist/foundry-anvil_1.0.5.dsc
   OK   dist/foundry-anvil_1.0.5.tar.xz
   === Building foundry-atelier (canonical debian/ source format) ===
   SKIP foundry-atelier (dist/foundry-atelier_0.9.5_all.deb + .dsc already current)
   ```

   PASS

2. Confirm `kdegames` and its deps survive the strip list (not purged during ISO build):

   ```
   grep -E 'kde-games|kig' foundry-iso/config/package-lists/strip.list.chroot.purge
   ```

   Expected: no matches.

   ```
   (no output — exit 1)
   ```

   PASS

3. Confirm new packages are listed in built `.deb` Depends:

   ```
   dpkg-deb --info dist/foundry-anvil_1.0.5_all.deb | grep 'Depends:'
   dpkg-deb --info dist/foundry-atelier_0.9.5_all.deb | grep 'Depends:'
   ```

   Expected: anvil Depends includes `kdegames, step, kig, stellarium, simutrans`; atelier unchanged.

   ```
   Depends: foundry-core, foundry-desktop, kdegames, step, kig, stellarium, simutrans
   Depends: foundry-sprite, foundry-emulators-vintage, foundry-emulators-consoles-heavy, foundry-game-reimplementations, foundry-free-games, foundry-android-development, foundry-ios-development, worldfoundry-development, ghidra, foundry-cv
   ```

   PASS

4. Publish local and resolve:

   ```
   task publish-local && task apt-test
   ```

   ```
   # TODO: run after ISO build completes
   ```

   PENDING
