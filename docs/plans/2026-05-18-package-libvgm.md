# Package libvgm as a .deb (Phase 1)

**Date:** 2026-05-18
**Status:** Done
**Skill used:** `/package` (~/.claude/skills/package/SKILL.md)
**Predecessor:** [`2026-05-18-package-skill.md`](2026-05-18-package-skill.md), [`2026-05-18-package-f9dasm.md`](2026-05-18-package-f9dasm.md) (if present)

## Goal

Replace the `~/opt/libvgm/` source-build sidecar in `install-foundry-retro-tools.sh` with a real `.deb` shipped from `apt.foundrylinux.org`. Promote `libvgm` from `Recommends:` to `Depends:` on the `foundry-retro-tools` metapackage so that `apt install foundry-retro-tools` pulls in `vgm-player` + `vgm2wav` and the libvgm static libraries + headers + CMake config.

## Constraints from the skill

1. **Universe check is BLOCKING.** Verify libvgm is not in Ubuntu 26.04 universe before touching `dh_make`.
2. **Use debhelper, not hand-rolled `dpkg-deb --build`.** The skill documents why (auto-strip, accurate shlibs, hardening, lintian-clean).
3. **Lintian gate: zero E: AND zero W: lines** before commit. `lintian-overrides` only for genuinely-impossible-to-fix tags.
4. **Every `/usr/bin/<name>` ships a man page.** Debian Policy §12.1; hand-written for the two binaries in this package.

## Approach

Single binary package `libvgm` containing:

| Path | Content |
|---|---|
| `/usr/bin/vgm-player` | renamed from upstream's generic `player` (collision-prone) |
| `/usr/bin/vgm2wav` | offline WAV renderer |
| `/usr/include/vgm/**` | C/C++ headers (`audio/`, `emu/`, `emu/cores/`, `player/`, `utils/`) |
| `/usr/lib/<triplet>/libvgm-{audio,emu,player,utils}.a` | **STATIC** libs (see below) |
| `/usr/lib/<triplet>/cmake/libvgm/` | CMake config package |
| `/usr/lib/<triplet>/pkgconfig/vgm-*.pc` | pkg-config files |
| `/usr/share/man/man1/vgm-{player,2wav}.1.gz` | hand-written man pages |
| `/usr/share/doc/libvgm/{copyright,changelog.Debian.gz}` | standard docs |

### Key decisions

1. **Upstream version pinning.** libvgm has no GitHub tags or releases — only rolling `master`. Pinned to commit `d1151884997ba4769b070b7c7aaedeffc60a25fb` (2026-04-06). Debian version string: `0.1+git20260406.d115188-1foundry1` (upstream's declared CMake `VERSION 0.1` + `+git<date>.<short-sha>` qualifier so future bumps sort correctly).

2. **STATIC libraries, not SHARED.** Upstream's `CMakeLists.txt` doesn't set `SOVERSION`/`VERSION` on the `vgm-*` library targets, so a SHARED build produces unversioned `.so` files. Shipping those would (a) trip lintian `shared-library-lacks-version` (W), (b) be hostile to downstream linkers (no SONAME means a libvgm upgrade silently breaks consumers). Static is the right choice for a pre-stable 0.1 library; revisit a proper soname-split (libvgm-audio0, libvgm-emu0, …, libvgm-dev) when upstream cuts a 1.0 with stable ABI.

3. **USE_SANITIZERS=OFF.** Upstream defaults `USE_SANITIZERS=ON` for debug-time convenience. A release build must not pull in `libasan`/`libubsan` as runtime deps.

4. **`/usr/bin/player` → `/usr/bin/vgm-player`.** Upstream's binary name is too generic and would collide with any future `player` package. Renamed in `override_dh_auto_install` to namespace it under the `vgm-` convention (matches the `vgm-player` library name).

5. **License: GPL-2.0+ umbrella.** Upstream ships no top-level `LICENSE`/`COPYING`. Source is a modular rewrite of VGMPlay (GPL-2.0+) with emulator cores from MAME and Nuked-OPM/OPL/OPLL carrying their own per-file headers (GPL-2.0+, LGPL-2.1+, BSD-3-Clause). DEP-5 copyright enumerates the per-file exceptions and declares GPL-2.0+ as the combined-work license.

## Verification

The plan is verified by the /package skill's [Step 7 checklist](~/.claude/skills/package/SKILL.md#verification-checklist). Evidence:

### Step 1 — Universe check (BLOCKING)

```bash
docker run --rm ubuntu:26.04 bash -c "
    apt-get update -qq
    apt-cache policy libvgm libvgm-dev libvgm-utils
    apt-cache search '^libvgm'
"
```

Output: all empty (`Candidate:` blank, no search hits). **PASS** — libvgm is not in Ubuntu 26.04 universe.

### Step 2 — Source pinning reproducibility

```bash
URL="https://github.com/ValleyBell/libvgm/archive/d1151884997ba4769b070b7c7aaedeffc60a25fb.tar.gz"
curl -fsSL -o libvgm-1.tar.gz "$URL"; sha256sum libvgm-1.tar.gz
curl -fsSL -o libvgm-2.tar.gz "$URL"; sha256sum libvgm-2.tar.gz
```

Both fetches: `7af2e58bc1bef990c0ee759cdef7bdf5e1e1ebc9d16cde489932b0370c0e7fbc`. **PASS** — reproducible.

### Step 3 — `debian/` tree

Authored (canonical Debian source format, no generated artifacts):

```
foundry-apt/packages/libvgm/
├── build.sh
└── debian/
    ├── changelog
    ├── control
    ├── copyright
    ├── libvgm.manpages
    ├── man/
    │   ├── vgm-player.1
    │   └── vgm2wav.1
    ├── patches/series
    ├── rules
    ├── source/format
    └── watch
```

### Step 4 — `dpkg-buildpackage` + `lintian`

```bash
bash foundry-apt/packages/libvgm/build.sh
lintian dist/libvgm_*.deb
```

```
running with root privileges is not recommended!
=== lintian exit: 0 ===
```

**PASS** — zero E: lines, zero W: lines (only root advisory).

Per-tag history (warnings caught and fixed during the run, all root-caused rather than overridden):

| Lintian tag | Root cause | Fix applied |
|---|---|---|
| `lacks-ldconfig-trigger` (E) | Initial attempt shipped SHARED libs without a triggers file | Switched to STATIC libs — no shared libs to trigger ldconfig for |
| `shared-library-lacks-version` (W) ×4 | Upstream `CMakeLists.txt` doesn't set `SOVERSION`/`VERSION` on `vgm-*` targets | STATIC libs sidestep the question; documented in changelog as "revisit at upstream 1.0" |
| `package-name-doesnt-match-sonames` (W) | Same root cause — multi-soname single package | Same fix |
| `debian-changelog-line-too-long` (W) | URL line in changelog >80 chars | Wrapped URL across two lines |

Binary checks:

```bash
$ file /usr/bin/vgm-player /usr/bin/vgm2wav
ELF 64-bit LSB pie executable, x86-64, …, dynamically linked, …, stripped
ELF 64-bit LSB pie executable, x86-64, …, dynamically linked, …, stripped

$ dpkg-deb -I libvgm_*.deb | grep Depends
Depends: libao4 (>= 1.1.0), libasound2t64 (>= 1.0.16), libc6 (>= 2.38),
         libgcc-s1 (>= 3.3.1), libpulse0 (>= 0.99.1), libstdc++6 (>= 5.2),
         zlib1g (>= 1:1.1.4)
```

All resolved with version constraints by `dh_shlibdeps`. **PASS** — PIE + stripped + auto-resolved deps.

Man pages mandoc-Tlint:

```bash
mandoc -Tlint debian/man/vgm-player.1; echo $?   # 0
mandoc -Tlint debian/man/vgm2wav.1;    echo $?   # 0
```

**PASS** — both clean (two warnings caught and fixed: bad `.TH` date format `"May 2026"` → `"2026-05-18"`; empty `.UR/.UE` blocks → with link text).

### Step 5 — Smoke install in fresh `ubuntu:26.04`

```bash
docker run --rm -v $WORKDIR:/debs ubuntu:26.04 bash -c '
    apt-get update -qq
    apt-get install -y -qq /debs/libvgm_*.deb
    vgm-player; echo $?
    vgm2wav;    echo $?
'
```

Output:

```
Usage: vgm-player inputfile
  exit: 0
Usage: vgm2wav [options] /path/to/vgm-file /path/to/out.wav
Available options:
    --samplerate n - sample rate (default: 44100)
    --bps n        - bits per sample (default: 16)
    --fade x       - fade out length in seconds (default: 8.0)
    --loops n      - numbers of loops before fade out (default: 2)
Specify "-" as output file to write to stdout.
  exit: 1
```

**PASS** — both binaries install and execute. Note: the container's `man` test failed because Ubuntu's minimal images strip `/usr/share/man/`; the man pages are confirmed present inside the `.deb` via `dpkg-deb -c`.

### Step 6 — Wired into the metapackage

- `foundry-apt/packages/foundry-retro-tools/debian/control` — `libvgm` moved from `Recommends:` to `Depends:`.
- `foundry-apt/packages/foundry-retro-tools/debian/changelog` — bumped to `1.0.3`.
- `foundry-apt/scripts/build-all.sh` picks the new package up automatically (no changes needed — it iterates `packages/*/`).

## Open follow-ups (not in scope here)

- **Phase 0 sidecar collapse.** `install-foundry-retro-tools.sh` still source-builds libvgm under `~/opt/libvgm/`. Strip that sidecar once `apt.foundrylinux.org` is configured as an apt source by `install.sh` (see TODO §"Housekeeping" — "Phase 0 configures foundry-apt as an apt source").
- **Re-evaluate SHARED packaging when upstream cuts a 1.0.** If `CMakeLists.txt` adds `SOVERSION`/`VERSION` on the `vgm-*` targets, split into `libvgm0` / `libvgm-dev` / `libvgm-utils` packages following Debian shared-library policy.
- **Bump cadence.** Until upstream cuts a release, periodic master-tip bumps (every few months) keep the package current; the bump procedure is in `build.sh`'s header comment.
