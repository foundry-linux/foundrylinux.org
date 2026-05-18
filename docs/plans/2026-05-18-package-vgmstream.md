# Plan: Package vgmstream as a `.deb`

**Date:** 2026-05-18
**Status:** Draft

## Context

vgmstream is the streamed-video-game-audio decoder in the Phase 0 retro-tools source-build sidecar set ([`foundry-linux-setup/install-foundry-linux-retro-tools.sh:162-169`](../../foundry-linux-setup/install-foundry-linux-retro-tools.sh)). Of the three source-built tools still needing to ship as `.deb`s (libvgm, vgmstream, ghidra) it's the meatiest of the two non-Ghidra ones: cmake-based, many optional codec dependencies (mpg123 / vorbis / ffmpeg / opus / speex), produces a CLI plus optional plugins.

**Universe check (skill Step 1, mandatory):** confirmed absent. `apt-cache policy vgmstream` and `apt-cache search vgmstream` on `ubuntu:26.04` both return empty. Not a duplicate.

## Vendoring decision

| Field | Value |
|---|---|
| Upstream | <https://github.com/vgmstream/vgmstream> |
| Tag | `r2083` (2026-01-25; latest GitHub release) |
| Tarball URL | `https://github.com/vgmstream/vgmstream/archive/refs/tags/r2083.tar.gz` |
| License | ISC-style ("Permission to use, copy, modify, and distribute … as is"). GitHub API reports `NOASSERTION` because the text is not byte-identical to the canonical ISC. Multiple "Portions Copyright" lines name additional contributors (Marko Kreen, jagarl/Kazunori Ueno, Justin Frankel/Nullsoft, Paul Hsieh, Leshade Entis, Sun Microsystems public domain). The combined-work grant is a single ISC-style block. |
| Build root | repo root (`cmake -S . -B build && cmake --build build`) |
| Primary binary | `vgmstream-cli` (decoder; reads any of the 100+ supported video-game audio formats and emits PCM) |
| Out of scope | `vgmstream123` (needs `libao-dev`), Winamp/foobar2000/XMPlay/Audacious plugins (Windows/macOS-only or out-of-scope native plugin), nightly auto-builds |
| Architectures | `amd64` only for first pass |
| Debian version | `r2083-1foundry1` |
| Copyright holders | 2008-2025 Adam Gashlin, Fastelbja, Ronny Elfert, bnnm, Christopher Snowhill, NicknineTheEagle, bxaimc, Thealexbarney, CyberBotX, EdnessP, et al — plus the named Portions holders |

### License-shortname note for `dh_make`

`dh_make`'s `--copyright` flag accepts `gpl2 / gpl3 / lgpl2 / lgpl3 / apache / bsd / mit / artistic / custom`. ISC is not one of them. Closest match is `bsd` (which generates BSD-2-clause text); we'll use `--copyright bsd` and then hand-rewrite `debian/copyright` to be DEP-5 with the actual ISC-style permission grant plus the Portions list. This pattern is worth adding to the `/package` skill — see "Skill update" below.

### Tag pattern note

vgmstream tags as `r<number>` (e.g. `r2083`) rather than `vN.N.N`. Affects `debian/watch` — the existing skill template's GitHub pattern uses `v?@ANY_VERSION@\.tar\.gz` which won't match `r2083`. Override with `r@ANY_VERSION@\.tar\.gz`. Worth noting in the skill.

## Files to change

| File | Action |
|---|---|
| `foundry-apt/packages/vgmstream/build.sh` | **new** — skill's `templates/build.sh` shim |
| `foundry-apt/packages/vgmstream/debian/control` | **new** — Source: vgmstream, Architecture: any, Section: sound; Build-Depends include `cmake`, `libmpg123-dev`, `libvorbis-dev`, `libopus-dev`, `libspeex-dev`, `libavformat-dev`, `libavcodec-dev`, `libavutil-dev`, `libswresample-dev`, `yasm` |
| `foundry-apt/packages/vgmstream/debian/changelog` | **new** — `r2083-1foundry1` targeting `resolute` |
| `foundry-apt/packages/vgmstream/debian/copyright` | **new** — DEP-5 with ISC text + Portions list |
| `foundry-apt/packages/vgmstream/debian/rules` | **new** — `dh $@` with cmake handled by debhelper auto-detection; `override_dh_auto_install` to copy `obj-*/cli/vgmstream-cli` into `/usr/bin/`; pass cmake flags to disable plugins (`-DBUILD_AUDACIOUS=OFF`, etc. — names to be confirmed during build) |
| `foundry-apt/packages/vgmstream/debian/source/format` | **new** — `3.0 (quilt)` |
| `foundry-apt/packages/vgmstream/debian/watch` | **new** — GitHub pattern with `r@ANY_VERSION@` instead of `v?@ANY_VERSION@` |
| `foundry-apt/packages/vgmstream/debian/patches/series` | **new** — empty |
| `foundry-apt/packages/vgmstream/debian/man/vgmstream-cli.1` | **new** — hand-written man page from `vgmstream-cli --help` |
| `foundry-apt/packages/vgmstream/debian/vgmstream.manpages` | **new** — lists `vgmstream-cli.1` |
| `foundry-apt/packages/foundry-linux-retro-tools/debian/control` | **edit** — promote `vgmstream` from `Recommends:` → `Depends:`; bump `Version:` to `1.0.3` |
| `foundry-apt/packages/foundry-linux-retro-tools/debian/changelog` | **edit** — new `1.0.3` entry |
| `foundry-linux-setup/install-foundry-linux-retro-tools.sh` | **defer** — strip the source-build block once Phase 0 configures foundry-apt as a source (blocked) |
| `TODO.md` | **edit** — flip vgmstream item to `[x]` and move to Done |
| `docs/plans/2026-05-18-package-vgmstream.md` | **new** — this file |

## CMake-aware `debian/rules`

debhelper auto-detects `CMakeLists.txt` and runs an out-of-source cmake build into `obj-<DEB_HOST_GNU_TYPE>/`. Likely overrides we'll need:

- `override_dh_auto_configure` — pass `-DCMAKE_BUILD_TYPE=Release` and `-DBUILD_AUDACIOUS=OFF`, `-DBUILD_V123=OFF`, `-DBUILD_FB2K=OFF`, `-DBUILD_WINAMP=OFF`, `-DBUILD_XMPLAY=OFF` (actual cmake option names confirmed during first build attempt).
- `override_dh_auto_install` — `install -D obj-*/cli/vgmstream-cli debian/vgmstream/usr/bin/vgmstream-cli` (because vgmstream's CMakeLists has `install()` only on WIN32, so cmake's install target is a no-op on Linux).

If vgmstream-cli links to libvgmstream as a static archive (the cmake default for `add_library` without explicit `SHARED`), we ship a single self-contained binary. If shared, we'd need to revisit and split into `vgmstream` (lib) + `vgmstream-bin` (binary). First-pass assumption: static.

## Verification

1. **Universe check re-confirmed.** `apt-cache policy vgmstream` returns no Candidate on a fresh `ubuntu:26.04`.

2. **sha256 pinned.** `curl -fsSL https://github.com/vgmstream/vgmstream/archive/refs/tags/r2083.tar.gz | sha256sum` matches the value in `build.sh`; re-fetch produces the same digest.

3. **Local build.** From `foundry-apt/`:

    ```
    bash packages/vgmstream/build.sh
    ```

    Expect: `dist/vgmstream_r2083-1foundry1_amd64.deb` exists. Lintian zero E:/W: (only the root-privileges advisory).

4. **`dpkg-deb` introspection.**

    ```
    dpkg-deb -I dist/vgmstream_*.deb
    dpkg-deb -c dist/vgmstream_*.deb
    file /tmp/x/usr/bin/vgmstream-cli
    ```

    Expect: `Depends:` lists `libc6`, `libmpg123-0` (or current soname), `libvorbis0a`, `libavcodec*`, `libavformat*`, `libavutil*`, `libswresample*`, `libopus0`, `libspeex1`, `${misc:Depends}` — all auto-resolved with version constraints by `dh_shlibdeps`. `vgmstream-cli` is PIE + stripped.

5. **Smoke install + binary runs.**

    ```
    docker run --rm -v "$PWD/foundry-apt/dist:/debs" ubuntu:26.04 bash -c '
        apt-get update -qq &&
        apt-get install -y -qq /debs/vgmstream_*.deb &&
        which vgmstream-cli &&
        vgmstream-cli 2>&1 | head -10
    '
    ```

    Expect: `vgmstream-cli` on PATH; running with no args prints usage banner (vgmstream-cli version, usage line).

6. **`mandoc -Tlint` on man page.** Hand-written `vgmstream-cli.1` lints clean at `-W warning`.

7. **Metapackage rebuild.** `foundry-linux-retro-tools 1.0.3` builds; `Depends:` includes `vgmstream` (out of `Recommends:`).

8. **Live test gate.** After publish: `bash foundry-apt/test/run-test.sh --package vgmstream` reports `[PASS] vgmstream` including the new mandoc man-page assertion.

## Skill update opportunities (per [feedback memory](../../../../.claude/projects/-home-will-SRC-foundrylinux-org/memory/feedback_package_skill_iterate.md))

While packaging vgmstream, three skill refinements surfaced. Land them in `~/.claude/skills/package/SKILL.md` in the same session as this commit:

1. **ISC license** — not in dh_make's `--copyright` list. Add a note in Step 3 §2 (`debian/copyright`) that for ISC/0BSD/MIT-no-attribution variants, use `--copyright bsd` and then hand-rewrite DEP-5 with the actual permission grant.

2. **Non-`vN.N.N` upstream tag formats** — vgmstream uses `r<number>` (`r2083`). The skill's `debian/watch` template references `v?@ANY_VERSION@\.tar\.gz` which won't match `r`-prefix tags. Make Step 3 §6 mention common alternatives: `v?` for typical `vN.N.N`, `r` for `r<number>` rolling releases, `release-` for some projects, etc.

3. **CMake-without-install() projects** — vgmstream has cmake but no Unix `install()` target. dh_auto_install becomes a no-op; we hand-install in `override_dh_auto_install`. Add to Step 3 §4's example overrides.

## Out of scope

- arm64 build (defer; same cross-compile follow-up as xa65 and f9dasm).
- vgmstream123 CLI player (needs libao-dev runtime dep; defer to follow-up package).
- Audacious / Winamp / foobar2000 / XMPlay plugins (Windows-only or out of scope).
- libvgmstream as a separate shared-library package (first pass: static-link into `vgmstream-cli`).
- Stripping the Phase 0 source-build block (blocked on the "Phase 0 configures foundry-apt as a source" TODO).
