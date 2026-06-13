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
3. [ ] Docker build + lintian clean
4. [ ] Smoke install + `asar-snes --version` runs
5. [ ] Wire into `foundry-retro-tools` `Depends:`
6. [ ] Commit + push + release tag

## Verification

1. `lintian dist/asar-snes_*.deb` returns clean (zero E:/W: lines)
2. `docker run --rm ubuntu:26.04 ... asar-snes --version` prints `Asar v1.91`
3. `foundry-retro-tools` apt-get install resolves with asar-snes in the dep tree
