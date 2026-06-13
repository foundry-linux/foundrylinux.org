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

## Status: foundry-apt DONE — ISO build remaining

Revised decision 2026-06-13: both JREs belong in `foundry-core` so all tiers
(anvil, sprite, atelier, devbox) get both without special-casing. anvil ⊆ atelier,
so the explicit `openjdk-21-jre-headless` dep in foundry-atelier is now redundant
and removed.

- `foundry-core` 1.0.6 — `openjdk-17-jre-headless` + `openjdk-21-jre-headless` in Depends ✓
- `foundry-atelier` 0.9.5 — removed redundant `openjdk-21-jre-headless` dep ✓
- Published: foundry-apt v1.5.26 (pending)

## Remaining: build and verify the ISO

```bash
task iso-build EDITION=anvil
```

Verification (after build):
```bash
sudo unsquashfs -cat foundry-iso/dist/foundry-anvil-*.iso \
  filesystem.squashfs var/lib/dpkg/status \
  | grep -E '^Package: (openjdk-17-jre-headless|openjdk-21-jre-headless|openjdk-21-jdk|ghidra)'
```
Expected on anvil:
- `openjdk-17-jre-headless` **present** ✓
- `openjdk-21-jre-headless` **absent** (autoremoved by hook 1010) ✓
- `openjdk-21-jdk` **absent** ✓
- `ghidra` **absent** ✓
