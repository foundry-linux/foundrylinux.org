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

…but Ubuntu 26.04 now ships **libavcodec62 / libavformat62 / libavutil60** (`ffmpeg 8.0.1-3ubuntu2`, from `resolute/universe`). The pre-ffmpeg-8 sonames were SRU'd out, leaving `foundry-retro-tools` unresolvable on any freshly-pulled 26.04 box.

`1foundry2` was built on 2026-05-18 — Ubuntu's ffmpeg bump to 8 has shipped since then, invalidating the pinned shlib deps.

## Approach

The control file uses `${shlibs:Depends}` — Debian's auto-resolver against the build host's installed libraries. The first attempt was a changelog-only bump to `2083-1foundry3` (v0.0.34, 2026-05-20), under the assumption that CI builds inside `ubuntu:26.04`.

**That assumption was wrong.** Inspecting `.github/workflows/publish.yml` after v0.0.34 republished 1foundry3 with the *same* broken sonames revealed that the `build-and-publish` job runs on `runs-on: ubuntu-latest` with **no container directive** — i.e. directly on the GitHub-hosted noble (24.04) runner. Apt installed `libavcodec60 7:6.1.1-3ubuntu5` from `noble/universe`, dpkg-shlibdeps pinned to `libavcodec60`, the .deb was uninstallable on 26.04 — same defect as 1foundry2. Only the smoke-install job uses `container: ubuntu:26.04`, and that one only runs `apt-cache show` (which doesn't catch unmet dependencies).

This is the silent failure mode `feedback_build_in_containers.md` warned about: host deps satisfy Build-Depends invisibly and CI ships a broken artifact.

**Real fix:** wrap the `Build all .debs` step in `docker run ubuntu:26.04`, so `dpkg-buildpackage` runs against 26.04's libs and `${shlibs:Depends}` resolves to libav*62.

1. Modify `.github/workflows/publish.yml`'s "Build all .debs" step to run inside `ubuntu:26.04` via `docker run -v "$PWD:/work" -w /work`. The container installs the cross-cutting build deps (build-essential, debhelper, dpkg-dev, devscripts, fakeroot, lintian, sudo, curl, ca-certificates, pkg-config); each package's `build.sh` continues to install its own specific deps (`cmake`, `yasm`, ffmpeg-dev, etc.) inside the same container.
2. Bump vgmstream changelog to `2083-1foundry4` so the new build is identifiable as the actually-fixed one.
3. Sync `foundry-apt/` to remote and tag `v0.0.35`.
4. Wait for CI; verify libavcodec62 sonames in the published Packages file.
5. Re-run the e2e test.

## Verification

1. After tag push: `gh run watch` until publish.yml is green.
2. `curl -fsSL https://apt.foundrylinux.org/dists/resolute/main/binary-amd64/Packages | awk '/^Package: vgmstream$/,/^$/' | grep '^Depends:'` shows `libavcodec62` (not 60).
3. `bash foundry-setup/test/test-retro-tools-e2e.sh` passes end-to-end with `Results: 15 passed, 0 failed`.
