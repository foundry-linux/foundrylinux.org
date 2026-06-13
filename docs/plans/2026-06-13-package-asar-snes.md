# Package asar-snes

**Date:** 2026-06-13  
**Goal:** Package the SNES 65816/SPC700/SuperFX cross-assembler "asar" (RPGHacker/asar) as `asar-snes` for `apt.foundrylinux.org` and wire it into `foundry-retro-tools`.

## Context

- The Electron archive tool `@electron/asar` is already packaged as `asar` (committed in `foundry-apt/packages/asar/`).
- Both tools produce a binary named `asar`, so the SNES assembler must be named `asar-snes` to coexist.
- Latest upstream: v1.91 — `https://api.github.com/repos/RPGHacker/asar/tarball/v1.91`
- SHA256: `b063ffff2e21bb9ef1e0de37557809b57ef8ee79908adf5a64ddae40be6e7c1d`
- License: LGPL-3+
- Build system: cmake (source in `src/` subdirectory); exe-only (`-DASAR_GEN_DLL=OFF -DASAR_GEN_LIB=OFF`)

## Steps

1. [x] Universe check — not in Ubuntu 26.04
2. [x] Create `foundry-apt/packages/asar-snes/` with `build.sh` + `debian/` tree
3. [x] Docker build + lintian clean
4. [x] Smoke install + `asar-snes --version` runs
5. [x] Wire into `foundry-retro-tools` `Depends:` (→ 1.0.13)
6. [x] Commit + push + release tag v1.5.23

## Verification

1. `lintian dist/asar-snes_*.deb` returns clean (zero E:/W: lines)

```
=== lintian asar-snes ===
(no output — clean)
```
PASS

2. `docker run --rm ubuntu:26.04 ... asar-snes --version` prints `Asar 1.91`

```
/usr/bin/asar-snes: ELF 64-bit LSB pie executable, x86-64, stripped
Asar 1.91, originally developed by Alcaro, maintained by Asar devs.
Source code: https://github.com/RPGHacker/asar
```
PASS

3. `foundry-retro-tools` apt-get install resolves with asar-snes in the dep tree

```
apt-cache depends foundry-retro-tools | grep asar
  Depends: asar-snes
```
PASS
