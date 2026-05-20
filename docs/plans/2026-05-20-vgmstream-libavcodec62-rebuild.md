---
title: Rebuild vgmstream against libavcodec62 (ffmpeg 8)
date: 2026-05-20
status: in-progress
---

# vgmstream rebuild for libavcodec62

## Context

The retro-tools E2E test (`docs/plans/2026-05-20-retro-tools-e2e-test.md`) caught that `vgmstream_2083-1foundry2` on `apt.foundrylinux.org` declares:

```
Depends: libavcodec60 (>= 7:6.0), libavformat60 (>= 7:6.0), libavutil58 (>= 7:6.0), …
```

…but Ubuntu 26.04 now ships **libavcodec62 / libavformat62 / libavutil60** (`ffmpeg 8.0.1-3ubuntu2`, from `resolute/universe`). The pre-ffmpeg-8 sonames were SRU'd out, leaving `foundry-linux-retro-tools` unresolvable on any freshly-pulled 26.04 box.

`1foundry2` was built on 2026-05-18 — Ubuntu's ffmpeg bump to 8 has shipped since then, invalidating the pinned shlib deps.

## Approach

The control file uses `${shlibs:Depends}` — Debian's auto-resolver against the build host's installed libraries. So the fix is purely a rebuild in a current `ubuntu:26.04` container; no source changes beyond a changelog entry are needed.

1. Append a `2083-1foundry3` changelog entry noting the ffmpeg 8 / libavcodec62 rebuild.
2. Sync `foundry-apt/` to the `foundry-linux/foundry-apt` remote (`task sync`).
3. Tag the next release (`task release TAG=v0.0.31` — confirm next tag with `gh release list`).
4. CI builds in `ubuntu:26.04`, which now resolves shlibs to libav*62.
5. Wait for the published `.deb` to land on R2, then re-run `bash foundry-linux-setup/test/test-retro-tools-e2e.sh`.

## Verification

1. After tag push: `gh run watch` until publish.yml is green.
2. `curl -fsSL https://apt.foundrylinux.org/dists/resolute/main/binary-amd64/Packages | awk '/^Package: vgmstream$/,/^$/' | grep '^Depends:'` shows `libavcodec62` (not 60).
3. `bash foundry-linux-setup/test/test-retro-tools-e2e.sh` passes end-to-end with `Results: 15 passed, 0 failed`.
