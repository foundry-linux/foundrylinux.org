# Plan: Add JRE 17 to foundry-core, JRE 21 to foundry-atelier

## Context

anvil is a game-dev distro — Java runtimes are a baseline expectation (LibGDX,
Gradle, Tiled, various tools). Currently neither JRE is intentionally in any
edition: openjdk-17 only arrives via foundry-android-development (atelier-only),
and openjdk-21 only arrives transiently via ghidra then gets autoremoved by hook 1010.

Decision:
- **foundry-core** → `openjdk-17-jre-headless`: Java 17 LTS baseline, available in
  every tier (devbox, anvil, sprite, atelier) and in Phase 0 `apt install foundry-core`.
- **foundry-atelier** → `openjdk-21-jre-headless`: current LTS, for heavier atelier
  tooling (ghidra already implies it; making it explicit means it's intentional rather
  than a side-effect that gets autoremoved). On non-atelier builds hook 1010 continues
  to autoremove openjdk-21 since foundry-core only depends on 17.

Size impact:
- anvil: +~185 MiB installed (JRE 17 only) → ~90 MiB on ISO → ~3.93 GiB
- atelier: +JRE 21 (~194 MiB) in addition to JRE 17 (already there via core)

---

## Changes

### 1. `foundry-apt/packages/foundry-core/debian/control`

Add to Depends (after `task,`):
```
 openjdk-17-jre-headless,
```

Update the Description long-text to mention Java 17 runtime.

### 2. `foundry-apt/packages/foundry-core/debian/changelog`

New entry (1.0.3 → 1.0.4):
```
foundry-core (1.0.4) resolute; urgency=medium

  * Add openjdk-17-jre-headless to Depends.
    Java 17 LTS baseline for LibGDX, Gradle, and game-dev toolchains.
```

### 3. `foundry-apt/packages/foundry-atelier/debian/control`

Add to Depends:
```
 openjdk-21-jre-headless,
```

Update the Description to note JRE 21 is explicitly included.

### 4. `foundry-apt/packages/foundry-atelier/debian/changelog`

New entry bumping version (check current; likely 0.9.3 → 0.9.4).

### 5. `foundry-iso/config/package-lists/foundry.list.chroot`

Add `openjdk-17-jre-headless` as an explicit entry. This ensures it installs during
the main live-build phase (apt runs from the host with working gpgv) rather than
relying on hook 1000's in-chroot apt-get to pull in a brand-new package.

### 6. `foundry-iso/config/hooks/1010-trim-atelier-only-pkgs.hook.chroot`

**No changes needed.** The existing `apt-mark auto 'openjdk-21-*' && autoremove`
continues to correctly remove openjdk-21-jdk (full JDK, only ghidra needed it)
from non-atelier builds. openjdk-21-jre-headless is NOT in foundry-core's Depends,
so it remains eligible for autoremoval on anvil/sprite — which is what we want.
On atelier builds the `[[ "${EDITION}" != "atelier" ]]` guard skips the block
entirely, so openjdk-21-jre-headless survives there as an explicit dep of foundry-atelier.

---

## Build & verify

```bash
task apt-build
task iso-stage-deb PACKAGE=foundry-core
task iso-build EDITION=anvil
```

Verification:
```bash
sudo unsquashfs -cat foundry-iso/dist/foundry-anvil-*.iso \
  filesystem.squashfs var/lib/dpkg/status \
  | grep -E '^Package: (openjdk-17-jre-headless|openjdk-21-jre-headless|openjdk-21-jdk|ghidra)'
```
Expected on anvil:
- `openjdk-17-jre-headless` **present** ✓
- `openjdk-21-jre-headless` **absent** (autoremoved by hook 1010 — not in foundry-core) ✓
- `openjdk-21-jdk` **absent** ✓
- `ghidra` **absent** ✓

After confirming the size, update `docs/plans/2026-06-04-usb-sized-iso-editions.md`:
- Add a new row to the scenario table for the new build (e.g. `anvil 0.9.54 + JRE 17`)
- Replace the now-stale intermediate rows (0.9.40 actual, 0.9.44–0.9.49) — keep only
  the 0.9.53 actual and the new build's actual as the two concrete data points

---

## Files to modify

| File | Change |
|---|---|
| `foundry-apt/packages/foundry-core/debian/control` | add `openjdk-17-jre-headless` dep + update description |
| `foundry-apt/packages/foundry-core/debian/changelog` | bump to 1.0.4 |
| `foundry-apt/packages/foundry-atelier/debian/control` | add `openjdk-21-jre-headless` dep |
| `foundry-apt/packages/foundry-atelier/debian/changelog` | bump version |
| `foundry-iso/config/package-lists/foundry.list.chroot` | add `openjdk-17-jre-headless` |
| `docs/plans/2026-06-04-usb-sized-iso-editions.md` | add new scenario row with actual ISO size; replace the stale 0.9.40 actual row |
