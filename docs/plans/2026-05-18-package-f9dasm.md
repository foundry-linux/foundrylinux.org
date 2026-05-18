# Plan: Package f9dasm as a `.deb` (first test of the `/package` skill)

**Date:** 2026-05-18
**Status:** Done

## Context

f9dasm is the 6800/6801/6802/6803/6808/6809/6301/6303/6309 disassembler in the Phase 0 retro-tools source-build sidecar set ([`foundry-linux-setup/install-foundry-linux-retro-tools.sh:145-151`](../../foundry-linux-setup/install-foundry-linux-retro-tools.sh)). Of the four source-built tools still needing to ship as `.deb`s (f9dasm, libvgm, vgmstream, ghidra), f9dasm is the simplest: four single-file C programs, ~270 KB total source, tiny Makefile, no native deps beyond libc.

This is **the first run** of the new [`/package` skill](2026-05-18-package-skill.md). Goals: prove the skill end-to-end, and ship a debhelper-built `.deb` that beats the hand-rolled xa65 packaging on every dimension (size, hardening, `${shlibs:Depends}`, source-package availability) — see the skill's "Why debhelper" comparison.

**Universe check (skill Step 1, mandatory):** confirmed absent. `apt-cache policy f9dasm` on `ubuntu:26.04` returns empty; Launchpad API `getPublishedBinaries`/`getPublishedSources` for `f9dasm` against the Ubuntu primary archive returns `{"total_size": 0}` across all releases. f9dasm is also not in Debian. Not a duplicate.

## Vendoring decision

| Field | Value |
|---|---|
| Upstream | <https://github.com/Arakula/f9dasm> (Hermann Seib's repo; Arakula is his handle) |
| Tag | `V1.83` (2023-07-04, latest release) |
| Tarball URL | `https://github.com/Arakula/f9dasm/archive/refs/tags/V1.83.tar.gz` |
| License | GPL-2.0-or-later (`f9dasm.c` header says "either version 2 … or (at your option) any later version"; the three small utility C files have no header but inherit the repo `LICENSE` (GPL-2). dh_make's DEP-5 short name is `GPL-2.0+`.) |
| Build root | repo root (`make`) |
| Binaries produced | `f9dasm`, `hex2bin`, `mot2bin`, `cmd2mot` (4 binaries, all C, ~30-100 KB each) |
| Man pages | **none upstream** — lintian will warn (`binary-without-manpage`); accept the warning for now, file an upstream PR later. |
| Other docs | `README.md` (markdown manual, 19 KB), `f9dasm.htm` (HTML manual, 18 KB), `LICENSE` |
| Examples | `sample.zip` (53 KB, .inf directive files) — skip in package; users can grab from GitHub |
| Architectures | `amd64` only for first pass (matches xa65 scope; arm64 follow-up tracked in TODO) |
| Debian version | `1.83-1foundry1` (strip the `V` prefix per Debian convention) |
| Copyright holders | Arto Salmi (2000), Hermann Seib (2001-2022), Colin Bourassa (2013), Rainer Buchty (2014-2015) |

## Why debhelper this time (vs hand-roll like xa65)

xa65 was hand-rolled because the `/package` skill didn't exist yet. Now it does. The xa65 outcome documented in [`docs/plans/2026-05-18-package-xa65.md`](2026-05-18-package-xa65.md) shipped a 223 KB unstripped `.deb` with bare `Depends: libc6`. The skill's debhelper path will instead produce:

- Auto-stripped binaries (`dh_strip`) → likely 3-5× smaller
- `Depends: libc6 (>= 2.38), ${misc:Depends}` resolved from actual linked sonames (`dh_shlibdeps`)
- PIE + stack-protector + FORTIFY_SOURCE (dpkg-buildflags via `dh`)
- DEP-5 `debian/copyright` (machine-readable)
- A real source package (`.dsc` + `.orig.tar.gz` + `.debian.tar.xz`) we can later push to a PPA
- `debian/watch` for `uscan` future version detection

If this run goes cleanly, the xa65 cleanup TODO (TODO.md line 14) gets simpler: when we delete `packages/xa65/`, we just add `xa65` to the apt-install list and stop maintaining our own redundant version of an Ubuntu universe package — no need to rewrite xa65 with debhelper at all.

## Files to change

| File | Action |
|---|---|
| `foundry-apt/packages/f9dasm/build.sh` | **new** — skill's `templates/build.sh` shim: fetch tarball, sha256-verify, drop in `debian/`, run `dpkg-buildpackage -us -uc -b`, move `.deb` to `dist/` |
| `foundry-apt/packages/f9dasm/debian/control` | **new** — Source: f9dasm, Build-Depends: debhelper-compat (= 13), Architecture: any |
| `foundry-apt/packages/f9dasm/debian/changelog` | **new** — single `1.83-1foundry1` entry, target `resolute` (Ubuntu 26.04) |
| `foundry-apt/packages/f9dasm/debian/copyright` | **new** — DEP-5; lists all four upstream copyright holders + GPL-2 license body |
| `foundry-apt/packages/f9dasm/debian/rules` | **new** — `dh $@` plus `override_dh_auto_install` (upstream Makefile has no `install` target) |
| `foundry-apt/packages/f9dasm/debian/source/format` | **new** — `3.0 (quilt)` |
| `foundry-apt/packages/f9dasm/debian/watch` | **new** — version=4, GitHub tags pattern, matches `V?@ANY_VERSION@` |
| `foundry-apt/packages/f9dasm/debian/patches/series` | **new** — empty placeholder |
| `foundry-apt/packages/foundry-linux-retro-tools/DEBIAN/control` | **edit** — promote `f9dasm` from `Recommends:` → `Depends:`; bump `Version: 1.0.1` → `1.0.2` |
| `foundry-linux-setup/install-foundry-linux-retro-tools.sh` | **defer** — the f9dasm source-build block (lines 145-151) stays for now. Stripping it is blocked on the "Phase 0 configures foundry-apt as a source" TODO, same as the xa65 cleanup. |
| `TODO.md` | **edit** — flip line 16 to `[x]`; promote to done section |

`foundry-apt/scripts/build-all.sh` already discovers `packages/*/build.sh` — no change needed.

## `override_dh_auto_install` shape

Upstream's Makefile builds `f9dasm hex2bin mot2bin cmd2mot` in the repo root but has no `install` target (`Makefile:9` is just a `.PHONY clean:`). The override:

```make
override_dh_auto_install:
	install -d $(CURDIR)/debian/f9dasm/usr/bin
	install -m 0755 f9dasm hex2bin mot2bin cmd2mot \
	    $(CURDIR)/debian/f9dasm/usr/bin/
```

`README.md` and `f9dasm.htm` get installed automatically by `dh_installdocs` into `/usr/share/doc/f9dasm/` (via the implicit `debian/f9dasm.docs` mechanism or by explicit listing).

## Verification

1. **Universe check re-confirmed.** `docker run --rm ubuntu:26.04 bash -c "apt-get update -qq && apt-cache policy f9dasm"` returns no Candidate.

    ```
    $ docker run --rm ubuntu:26.04 bash -c "apt-get update -qq 2>/dev/null && apt-cache policy f9dasm"
    (empty — no Candidate line, package not known)
    ```

    PASS.

2. **sha256 pinned.** `curl -fsSL https://github.com/Arakula/f9dasm/archive/refs/tags/V1.83.tar.gz | sha256sum` matches the value pinned in `build.sh`. Re-running the curl produces the same digest.

    ```
    fetch1:  3a2d1e3f5de09be8060e4d754ff401622b5043b9a66687823fde188d035b0323
    fetch2:  3a2d1e3f5de09be8060e4d754ff401622b5043b9a66687823fde188d035b0323
    pinned:  3a2d1e3f5de09be8060e4d754ff401622b5043b9a66687823fde188d035b0323
    reproducible: yes
    ```

    PASS.

3. **Local build.** From `foundry-apt/`:

    ```
    task build
    ```

    Expect: `dist/f9dasm_1.83-1foundry1_amd64.deb` exists; build exits 0. `lintian dist/f9dasm_*.deb 2>&1 | grep '^E:'` returns empty (no errors; `binary-without-manpage` warnings expected and accepted).

    ```
    $ bash scripts/build-all.sh
    ...
    OK   dist/f9dasm_1.83-1foundry1_amd64.deb  (50426 bytes)
    OK   dist/foundry-linux-retro-tools_1.0.2_all.deb  (~2.2K)
    OK   dist/worldfoundry-*  (×4)
    OK   dist/xa65_2.4.1-1foundry1_amd64.deb  (228058 bytes)

    $ lintian dist/f9dasm_*.deb
    W: f9dasm: no-manual-page [usr/bin/cmd2mot]
    W: f9dasm: no-manual-page [usr/bin/f9dasm]
    W: f9dasm: no-manual-page [usr/bin/hex2bin]
    W: f9dasm: no-manual-page [usr/bin/mot2bin]
    (no E: lines)
    ```

    PASS. 50 KB — 4.5× smaller than xa65's 223 KB unstripped hand-rolled build, as predicted.

    **Follow-up (1foundry2, same day):** the 4× `no-manual-page` lintian warnings above are a Debian Policy §12.1 violation, not a soft style nit. Authored four hand-written section-1 man pages (`debian/man/{f9dasm,hex2bin,mot2bin,cmd2mot}.1`), listed in `debian/f9dasm.manpages`, rebuilt as `1.83-1foundry2_amd64.deb` (53 KB; man pages add ~3 KB). Re-ran lintian:

    ```
    $ lintian dist/f9dasm_1.83-1foundry2_amd64.deb
    (clean — no E:, no W:, only the root-privileges advisory)
    ```

    Skill updated in `~/.claude/skills/package/SKILL.md`: man-page authoring is now an explicit Step 3 sub-step, and lintian is a BLOCKING gate before commit/upload (both E: and W: must be clean, not just E:).

4. **`dpkg-deb` introspection.**

    ```
    dpkg-deb -I dist/f9dasm_1.83-1foundry1_amd64.deb
    dpkg-deb -c dist/f9dasm_1.83-1foundry1_amd64.deb
    file --extract-to=/tmp/f9dasm-check dist/f9dasm_*.deb || dpkg-deb -x dist/f9dasm_*.deb /tmp/f9dasm-check
    file /tmp/f9dasm-check/usr/bin/*
    ```

    Expect: control shows `Package: f9dasm`, `Version: 1.83-1foundry1`, `Architecture: amd64`, `Section: devel`, `Depends: libc6 (>= …), ${misc:Depends}` substituted. Contents include `./usr/bin/{f9dasm,hex2bin,mot2bin,cmd2mot}` and DEP-5 copyright. `file` output reports "stripped" for all four binaries.

    ```
    $ dpkg-deb -I dist/f9dasm_1.83-1foundry1_amd64.deb
     Package: f9dasm
     Version: 1.83-1foundry1
     Architecture: amd64
     Maintainer: Foundry Linux <packages@foundrylinux.org>
     Installed-Size: 170
     Depends: libc6 (>= 2.38)
     Section: devel
     Priority: optional
     Homepage: https://github.com/Arakula/f9dasm
     Description: Motorola 6800/6809/6309 family disassembler
     ...

    $ dpkg-deb -c dist/f9dasm_1.83-1foundry1_amd64.deb
    -rwxr-xr-x ./usr/bin/cmd2mot   14568
    -rwxr-xr-x ./usr/bin/f9dasm    91512
    -rwxr-xr-x ./usr/bin/hex2bin   14568
    -rwxr-xr-x ./usr/bin/mot2bin   14568
    -rw-r--r-- ./usr/share/doc/f9dasm/README.md.gz      6234
    -rw-r--r-- ./usr/share/doc/f9dasm/changelog.Debian.gz 261
    -rw-r--r-- ./usr/share/doc/f9dasm/copyright         1244
    -rw-r--r-- ./usr/share/doc/f9dasm/f9dasm.htm       18419

    $ file /tmp/x/usr/bin/*
    cmd2mot: ELF 64-bit LSB pie executable, x86-64, ..., stripped
    f9dasm:  ELF 64-bit LSB pie executable, x86-64, ..., stripped
    hex2bin: ELF 64-bit LSB pie executable, x86-64, ..., stripped
    mot2bin: ELF 64-bit LSB pie executable, x86-64, ..., stripped
    ```

    PASS. All four binaries PIE + stripped; `${shlibs:Depends}` resolved to `libc6 (>= 2.38)`; DEP-5 copyright shipped.

5. **Smoke install + binary runs.**

    ```
    docker run --rm -v "$PWD/foundry-apt/dist:/debs" ubuntu:26.04 bash -c '
        apt-get update -qq &&
        apt-get install -y --no-install-recommends /debs/f9dasm_*_amd64.deb &&
        which f9dasm hex2bin mot2bin cmd2mot &&
        f9dasm 2>&1 | head -5 &&
        hex2bin 2>&1 | head -3 &&
        mot2bin 2>&1 | head -3 &&
        cmd2mot 2>&1 | head -3
    '
    ```

    Expect: all four binaries on PATH; each prints its usage/banner without segfault (f9dasm with no args shows "F9DASM …" usage line).

    ```
    /usr/bin/f9dasm
    /usr/bin/hex2bin
    /usr/bin/mot2bin
    /usr/bin/cmd2mot

    f9dasm: M6800/1/2/3/8/9 / H6309 Binary/OS9/FLEX9 Disassembler V1.83
    hex2bin: convert Intel .HEX to Binary
    mot2bin: convert Motorola .HEX to Binary
    cmd2mot: convert FLEX Binary to Motorola .HEX
    ```

    PASS. All four on PATH; each prints its banner cleanly with no segfault.

6. **Metapackage rebuild.**

    ```
    task build && task verify
    ```

    Expect: `dist/foundry-linux-retro-tools_1.0.2_all.deb` exists; `dpkg-deb -I` shows `f9dasm` in `Depends:` (not `Recommends:`); `f9dasm` removed from `Recommends:`.

    ```
    $ dpkg-deb -I dist/foundry-linux-retro-tools_1.0.2_all.deb
     Version: 1.0.2
     Depends: mame, mame-tools, dasm, cc65, z80dasm, z80asm, radare2,
              binwalk, sox, binutils-m68k-linux-gnu, xa65, f9dasm
     Recommends: ghidra, vgmstream, libvgm
    ```

    PASS. f9dasm in Depends; Recommends down to {ghidra, vgmstream, libvgm}.

7. **Metapackage resolves f9dasm against the local repo.**

    ```
    task apt-test 2>&1 | grep -A3 f9dasm
    ```

    Expect: `apt-cache depends foundry-linux-retro-tools` lists `f9dasm` under Depends (resolved from `./public/`), and `apt-cache policy f9dasm` shows the local repo as the candidate.

    (Equivalent run via docker since `task apt-test` requires host sudo; aptly publish + apt-cache inside a fresh `ubuntu:26.04`.)

    ```
    $ apt-cache policy f9dasm
    f9dasm:
      Installed: (none)
      Candidate: 1.83-1foundry1
      Version table:
         1.83-1foundry1 500
            500 file:/work/public resolute/main amd64 Packages

    $ apt-cache depends foundry-linux-retro-tools | grep -E '(Depends|Recommends):'
      Depends: mame
      Depends: mame-tools
      Depends: dasm
      Depends: cc65
      Depends: z80dasm
      Depends: z80asm
      Depends: radare2
      Depends: binwalk
      Depends: sox
      Depends: binutils-m68k-linux-gnu
      Depends: xa65
      Depends: f9dasm
      Recommends: <ghidra>
      Recommends: <vgmstream>
      Recommends: <libvgm>
    ```

    PASS. f9dasm resolves via local repo at `1.83-1foundry1`; metapackage Depends includes f9dasm; ghidra/vgmstream/libvgm shown as `<…>` (virtual/unsatisfied) — expected, those four are the next `/package` runs.

## Out of scope

- arm64 build (defer until any of the four sidecars needs it; cross-build via `gcc-aarch64-linux-gnu` follows the same pattern that xa65's plan deferred).
- Stripping the Phase 0 source-build block from `install-foundry-linux-retro-tools.sh` (blocked on the "Phase 0 configures foundry-apt as a source" TODO).
- Adding an upstream man page (file a PR to Arakula/f9dasm later; accept `binary-without-manpage` warnings for now).
- Shipping the `sample.zip` example .inf directives (low-value bloat; reachable from upstream).
- libvgm, vgmstream, ghidra (`/package` runs of their own; tracked in TODO.md).
