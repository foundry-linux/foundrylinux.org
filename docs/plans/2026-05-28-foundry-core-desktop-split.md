# foundry-core / foundry-desktop metapackage split

**Date:** 2026-05-28
**Status:** Draft

## Problem

`foundry-anvil` currently bundles developer tools and KDE desktop packages in a single metapackage. This breaks the "existing Ubuntu" install path:

- `foundry-welcome` depends on `qml6-module-org-kde-kirigami` (KDE)
- `foundry-kde-theme` depends on `plasma-workspace`
- `calamares-settings-foundry-linux` is ISO-installer-only; meaningless on a running GNOME desktop

An existing Ubuntu 26.04 (GNOME) user who runs `sudo apt install foundry-anvil` today drags in KDE/Plasma dependencies. That is wrong — their desktop should be untouched.

On the full ISO the situation is fine: the base is Kubuntu 26.04 (KDE), so KDE packages are already present and welcome. But the same metapackage cannot serve both audiences.

## Proposed layer model

```
foundry-core
  Pure developer tools. Zero KDE/Plasma/Kirigami deps.
  Safe to install on any Ubuntu flavour (GNOME, XFCE, Budgie, …).
  = current foundry-anvil minus desktop packages.

foundry-desktop
  KDE/Plasma-specific Foundry packages.
  Only meaningful on the ISO (Kubuntu base) or an existing KDE machine.
  Packages:
    - foundry-welcome      (Kirigami welcome app)
    - foundry-kde-theme    (Plasma colour scheme + shell theme)

foundry-anvil  (unchanged name, new definition)
  Thin umbrella: foundry-core + foundry-desktop.
  = what the ISO pre-installs and the devbox image installs.
  Existing meaning preserved for ISO users; they notice nothing.
```

### What each audience installs

| Audience | Command | Gets |
|---|---|---|
| Existing Ubuntu (GNOME/other) | `sudo apt install foundry-core` | All dev tools, no KDE |
| Distrobox / container | `sudo apt install foundry-anvil` | Same as today |
| ISO fresh install | pre-installed `foundry-anvil` | Tools + KDE desktop packages |

## Changes required

### 1. `foundry-apt/packages/` — new and modified metapackages

**New: `foundry-core/debian/control`**
- Depends: everything currently in `foundry-anvil` **except** `foundry-welcome`
- Architecture: all
- Description: Foundry Linux developer toolkit — no desktop dependencies

**New: `foundry-desktop/debian/control`**
- Depends: `foundry-welcome`, `foundry-kde-theme`
- Architecture: all
- Description: Foundry Linux KDE desktop integration (ISO / KDE hosts only)

**Modified: `foundry-anvil/debian/control`**
- Depends: `foundry-core`, `foundry-desktop` (thin umbrella only)
- Drop all direct tool deps (they now come via `foundry-core`)

Each gets a `debian/changelog` entry at `1.0.0` (new) or a version bump.

### 2. Site — install card 01

`sections.jsx` card 01 "Existing Ubuntu":

- Command changes from `sudo apt install foundry-anvil` → `sudo apt install foundry-core`
- Hook text: "Already on 26.04? Add the repo — no reinstall." — unchanged, still accurate
- The COPY buttons copy `foundry-core`

### 3. Site — `packages-data.json`

**New category `foundry-desktop`:**
```json
{
  "slug": "foundry-desktop",
  "title": "Foundry Desktop",
  "role": "KDE · ISO only",
  "blurb": "KDE desktop integration shipped on the Foundry Linux ISO — welcome app and Plasma theme. Not needed on existing Ubuntu installs.",
  "icon": "SparksIcon",
  "metapackages": ["foundry-desktop"],
  "packages": [
    { "name": "foundry-welcome", "origin": "foundry", … },
    { "name": "foundry-kde-theme", "origin": "foundry", … }
  ]
}
```

`foundry-welcome` and `foundry-kde-theme` move from being unowned/implicit to this category. The unowned-package audit error goes away.

**`foundry-anvil` edition entry:** update description to note it is `foundry-core + foundry-desktop`.

**Editions `install_command`:** no change — ISO users still install `foundry-anvil` as the full edition.

### 4. Devbox image

`foundry-devbox/Dockerfile` (or equivalent): currently installs `foundry-anvil`. This should switch to `foundry-core` — the container has no display server so KDE packages are dead weight and `foundry-welcome` will never run.

Verify the image still builds and the tool set is identical post-split.

## What does NOT change

- `foundry-sprite` and `foundry-atelier` — they depend on `foundry-anvil`; that remains valid since they are ISO/container layered editions, not existing-Ubuntu targets.
- `apt.worldfoundry.org` — no changes.
- The ISO build — pre-installs `foundry-anvil` which now pulls in `foundry-core + foundry-desktop`. Identical result for ISO users.

## Verification

1. **Package build:** `task build` produces `.deb` for `foundry-core`, `foundry-desktop`, and the slimmed `foundry-anvil` with no errors.

2. **Dependency check (existing Ubuntu simulation):**
   ```
   docker run --rm ubuntu:26.04 bash -c '
     apt-get update -q
     apt-get install -y --simulate foundry-core 2>&1 | grep -E "kde|plasma|kirigami"
   '
   ```
   Output must be empty — no KDE packages pulled.

3. **Dependency check (full anvil):**
   ```
   docker run --rm ubuntu:26.04 bash -c '
     apt-get install -y --simulate foundry-anvil 2>&1 | grep -E "foundry-welcome|foundry-kde-theme"
   '
   ```
   Both packages must appear.

4. **Anvil equivalence:** package set of `foundry-anvil` (new) equals package set of `foundry-anvil` (old). Diff must be empty.

5. **Site audit:** run the catalogue self-check script. `foundry-welcome` and `foundry-kde-theme` must each appear in exactly one category (`foundry-desktop`). Zero unowned packages.

6. **Devbox build:** `task devbox-build` succeeds. `docker run foundry-devbox which ghidra` returns a path. No KDE packages in the image layer diff.
