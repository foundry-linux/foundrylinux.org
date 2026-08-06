# Package x-emulators (LGB's Xemu suite) for apt.foundrylinux.org

**Date:** 2026-08-05
**Scope:** Vendor and package [LGB's Xemu](https://github.com/lgblgblgb/xemu) ("X-Emulators") as a
Debian-policy-compliant `.deb` under the name **`x-emulators`**, and wire it into the
`foundry-emulators-computers` metapackage.

**No visible surface** — a package and a metapackage dependency. No UI, page, or rendered output, so no
mockup bundle.

---

## Why the name is not `xemu`

There are two entirely unrelated projects called "xemu", and we now ship both:

| | `xemu` (already packaged) | `x-emulators` (this plan) |
|---|---|---|
| upstream | [xemu-project/xemu](https://github.com/xemu-project/xemu) | [lgblgblgb/xemu](https://github.com/lgblgblgb/xemu) |
| homepage | xemu.app | lgb.hu |
| what it is | original **Xbox** emulator | Commodore/Hungarian **8-bit** emulator suite |
| binaries | `xemu` | `xc65`, `xmega65`, `xclcd`, `xvic20`, `xep128`, `xprimo`, `xtvc` |

The bare name is taken, so this follows the existing house precedent for name collisions
(`asar` → `asar-snes-assembler`). `x-emulators` is upstream's own full project name ("X-Emulators ~ Xemu").
Repology has no entry for LGB's project — three name searches returned zero — so
`X-Repology-Project: none`.

Both `debian/control`'s long description and `debian/changelog` state the distinction explicitly, so a
future packager doesn't "fix" the name.

## Why it earns a place

`xc65` is the only *working* Commodore 65 emulator available to us. MAME has a `c65` driver, but it is
flagged `status: preliminary, emulation: preliminary` and its own source TODO says
*"rom8 / roma / rome all causes bootstrap issues if hooked up"* — `rome` being the `$E000` ROM window,
which is therefore left unmapped, so nothing executes. None of `dosbox-x`, `hatari`, `fs-uae`, `openmsx`
or `mame` covers the C65 usefully.

It also has a second, less obvious use: `c65_load_rom()` accepts **any** file of exactly `0x20000` bytes
as the system ROM with no checksum or version check, so `xc65` doubles as a bare-metal **65CE02 execution
environment** for compiler and toolchain work. That is what motivated the packaging — it was used to
validate [llvm-mos PR #585](https://github.com/llvm-mos/llvm-mos/pull/585)'s native `ASR` selection by
execution rather than inspection.

## Packaging decisions

- **Version.** Upstream publishes no tags and no releases; `master` is the only ref. Pinned commit
  `40dfef0d` (2026-01-29) with a git-snapshot version `0~git20260129.40dfef0d-1foundry1`, which sorts
  below any future real release.
- **Targets shipped.** The seven that upstream's default `all` target builds. `libreadline-dev` is a
  non-obvious build dependency — without it the `ep128` target fails with an `#error`, taking the whole
  build down.
- **Hardening.** Upstream assigns `CFLAGS` with a plain `=`, so passing flags on the make command line
  would clobber the flags the build needs. Instead `debian/rules` runs `build/configure/configure`
  itself and *appends* `dpkg-buildflags` output to the generated `config-native.make`, which is included
  before `CONFIG_CFLAGS` is used. Result: PIE + relro + bindnow without displacing upstream's flags.
- **Build identity.** From a tarball there is no `.git`, so the binary would report
  `unknown_commit_id`. `build/show-git-info` honours `TRAVIS_COMMIT`/`TRAVIS_BRANCH`, so `debian/rules`
  exports the pinned commit — `xc65 -h` now reports the exact revision.
- **Reproducibility.** The embedded build date came from `date` when git is absent, so two builds of
  identical source differed. Patched to prefer `SOURCE_DATE_EPOCH` (see below).
- **`make clean` is incomplete.** It leaves the configure probe's scratch files and `cdate.data` behind,
  which makes `dpkg-source -b` fail with *"unrepresentable changes to source"*. `override_dh_auto_clean`
  removes everything in `build/objs` and `build/bin` that upstream does not track.
- **No ROMs bundled**, so the package fits `foundry-emulators-computers`' redistribution criterion.

## Upstreamable patch

`debian/patches/0001-honour-SOURCE_DATE_EPOCH.patch` — prefer `SOURCE_DATE_EPOCH` for the embedded build
timestamp, keeping the git and `date` fallbacks for developer builds. This is a genuine portability /
reproducibility fix, not Debian glue, so per the `/package` skill's Step 7 it belongs upstream.
`Forwarded: https://github.com/lgblgblgb/xemu/pull/448` — opened upstream on 2026-08-05.

## Verification

All steps run 2026-08-05 in a clean `ubuntu:26.04` container.

1. **Universe check.**

    ```
    apt-cache policy x-emulators   -> (empty)
    apt-cache policy xemu          -> (empty)
    apt-cache search xemu          -> gxemul (unrelated MIPS/ARM emulator)
    ```

    **PASS** — neither name is in Ubuntu 26.04 universe.

2. **Build** via `packages/x-emulators/build.sh`.

    ```
    === Built /repo/dist/x-emulators_0~git20260129.40dfef0d-1foundry1_amd64.deb ===
    OK   dist/x-emulators_0~git20260129.40dfef0d-1foundry1.dsc
    OK   dist/x-emulators_0~git20260129.40dfef0d-1foundry1.debian.tar.xz
    OK   dist/x-emulators_0~git20260129.40dfef0d.orig.tar.gz
    ```

    **PASS** — binary and source packages both produced.

3. **lintian, binary and source.**

    ```
    === LINTIAN .deb ===
    === LINTIAN .dsc ===
    === (both empty = CLEAN) ===
    ```

    **PASS** — zero E: and zero W: on both. (Two warnings were fixed rather than
    overridden along the way: `old-fsf-address-in-copyright-file` and
    `build-depends-on-obsolete-package pkg-config => pkgconf`.)

4. **Contents and hardening.**

    ```
    /usr/bin/{xc65,xclcd,xep128,xmega65,xprimo,xtvc,xvic20}
    /usr/share/man/man1/{xc65,xclcd,xep128,xmega65,xprimo,xtvc,xvic20}.1.gz
    Depends: libc6 (>= 2.34), libglib2.0-0t64 (>= 2.12.0), libgtk-3-0t64 (>= 3.0.0),
             libreadline8t64 (>= 6.0), libsdl2-2.0-0 (>= 2.0.12), ...
    xc65: ELF 64-bit LSB pie executable, x86-64, dynamically linked
    ```

    **PASS** — 7 binaries, 7 man pages, versioned deps, PIE, stripped.

5. **Smoke install + run.**

    ```
    VERSION: unknown_remote master 40dfef0d1d5f56be2469492715c12bdb32c75b67 20260805020000 custom-build
    EMULATE: Commodore 65 (c65): xc65 ... for c65 on linux (native) using cc
    ```

    **PASS** — the real upstream commit is reported (not `unknown_commit_id`) and the
    timestamp comes from `SOURCE_DATE_EPOCH`. Upstream refuses to run as root
    (`ERROR: Xemu must not be run as user root`), so the test creates a normal user.

6. **End-to-end 65CE02 execution** — a bare-metal ASR kernel run on the *packaged* `xc65`:

    ```
    flag=0x5a result=0xe0e8
    ```

    **PASS** — `0xE0E8` matches the host oracle, so the shipped binary reproduces the
    llvm-mos #585 validation result.

7. **Metapackage.**

    ```
    === lintian 1.0.2 ===
    === (empty = clean) ===
    Inst x-emulators (0~git20260129.40dfef0d-1foundry1 localhost [amd64])
    ```

    **PASS** — `foundry-emulators-computers` 1.0.2 lints clean and pulls `x-emulators`.

8. **Live publication and install verification.**

    Release [`v1.5.42`](https://github.com/foundry-linux/foundry-apt/tree/v1.5.42)
    published `x-emulators 0~git20260129.40dfef0d-1foundry2` to
    `apt.foundrylinux.org`. The
    [publish workflow](https://github.com/foundry-linux/foundry-apt/actions/runs/31054323727)
    built and signed the repository, synced it to Cloudflare R2, passed repository-consistency
    verification, and passed its Ubuntu 26.04 indexing smoke test.

    A separate `task live-test -- --package x-emulators` run then installed the exact live version
    in a fresh Ubuntu 26.04 container. All seven installed binaries had clean corresponding man
    pages:

    ```text
    ok /usr/share/man/man1/xc65.1.gz
    ok /usr/share/man/man1/xclcd.1.gz
    ok /usr/share/man/man1/xep128.1.gz
    ok /usr/share/man/man1/xmega65.1.gz
    ok /usr/share/man/man1/xprimo.1.gz
    ok /usr/share/man/man1/xtvc.1.gz
    ok /usr/share/man/man1/xvic20.1.gz
    [PASS] x-emulators
    Results: 1 passed, 0 failed
    ```

    **PASS** — the signed package is published, indexed, installable, and validated from the live
    repository.

9. **Functional check of the live `-1foundry2` package.** Step 8 confirms the published package
    installs and carries its man pages; this confirms it still *works*, across the desktop/icon
    changes that landed in `-1foundry2` after the original verification.

    Installed from `apt.foundrylinux.org` in a fresh Ubuntu 26.04 container via the signed repo,
    then ran the bare-metal 65CE02 ASR kernel on the live `xc65`:

    ```text
    Package: x-emulators
    Status: install ok installed
    Version: 0~git20260129.40dfef0d-1foundry2
    desktop entries: 7   icon paths: 4
    flag=0x5a result=0xe0e8
    ```

    **PASS** — `0xE0E8` matches the host oracle, so the published package still reproduces the
    llvm-mos #585 validation result. This is the check that would catch a packaging change breaking
    the emulator itself, which a man-page/file-presence test cannot.

10. **Public open-source inventory and publication guard.** Refreshed the checked-in inventory in
    `wbniv/wald3n.com` from a clean clone, adding both `x-emulators` and the previously missing Xbox
    `xemu` row and removing the unvendored `task` row. The full wald3n suite passed 67/67; commits
    `b4c25c0` (inventory) and `cc14935` (release `v0.0.421`) were pushed and deployed. The live
    [wald3n open-source page](https://wald3n.com/open-source) was then verified to contain both
    `data-sort-name="x-emulators"` and `data-sort-name="xemu"`.

    Commit `09c9b11` makes this mandatory for future package releases: a Claude
    `PostToolUse[Bash]` hook marks `task bump`, `task release`, and `task sync-and-release` as pending;
    the project Stop hook blocks completion until `task package-publish:complete` verifies the
    refreshed wald3n snapshot and its live rows. The positive hook test detected the two missing rows,
    blocked completion, and cleared its marker only after the production deployment passed.

    **PASS** — both public surfaces are current, and future package publication cannot be reported
    complete while the wald3n refresh remains outstanding.

## Retroactive Step 2.5 findings (upstream's own packaging)

The `/package` skill gained a **Step 2.5 — check for upstream's own Debian packaging** *because of*
this package: we picked the name `x-emulators` without knowing upstream ships his own `.deb` builder
that calls it `xemu`. Running that check retroactively against the source turned up more:

| Finding | Status |
|---|---|
| No upstream `debian/` dir; one script, `build/deb-build-simple.sh` | — |
| Its declared deps (`libsdl2-2.0-0`, `libc6`, `libreadline*`, `libgtk-3-0`) | ✅ match what `${shlibs:Depends}` resolved for us — independent confirmation |
| Upstream names its binaries **`xemu-*`** (`b="$BINDIR/xemu-$(basename $a .native)"`), we ship bare `xc65`, `xmega65`, … | ⚠️ deliberate divergence, see below |
| Upstream installs a data dir `/usr/share/xemu` (with a `.placeholder`); we ship none | ⚠️ probably harmless — ROM paths are user-configured via `-rom`/config — but unverified |
| Upstream **deliberately disabled** its ROM-download helper: *"it seems it's a legality problem to ship the package with a helper inside which downloads ROM images copyrighted by some angry companies"* | ✅ our package ships no ROMs and no fetcher — independently the same conclusion |

That last row is the most useful: it is upstream's own considered legal position, and it means "add a
convenience ROM downloader" is a **bad** future idea, not an obvious improvement. Recorded here so
nobody re-litigates it.

On binary naming: bare `xc65`/`xmega65` are the names upstream's *build* produces and its own docs and
`-h` output use, and they are what the `.desktop` files and our man pages reference. Prefixing to
`xemu-xc65` would also read oddly (double `x`). Keeping bare names, but this is now a recorded decision
rather than an unexamined default — and it is worth mentioning in the upstream heads-up.

## Follow-ups

- [x] Ship `.desktop` files and icons for the seven GUI emulators. Done 2026-08-05: static
      `debian/<binary>.desktop` per binary (validated with `desktop-file-validate`),
      installed to `/usr/share/applications/`. Upstream ships one generic 48x48 XPM icon
      (`build/xemu-48x48.xpm`, referenced by its own `build/xemu.desktop` template and
      `build/deb-build-simple.sh`) — no per-machine artwork exists, so all seven entries share it,
      vendored verbatim to `debian/x-emulators.xpm` →
      `/usr/share/icons/hicolor/48x48/apps/x-emulators.xpm`. Flat filenames under `debian/`
      (not a subdirectory) match the existing `mesen2`/`tilemap-studio` convention and avoid the
      repo's `packages/**/debian/*/` gitignore rule, which only excepts `man/`, `source/`,
      `patches/`. Changelog bumped to
      `-1foundry2`. See verification step 8 below.
- [x] Open the upstream PR for the `SOURCE_DATE_EPOCH` patch and record the URL in the patch's
      `Forwarded:` header. Opened as [lgblgblgb/xemu#448](https://github.com/lgblgblgb/xemu/pull/448)
      on 2026-08-05. It is intentionally left untouched pending user review.
- [ ] File a Debian ITP for `x-emulators` — GPL-2.0-or-later,
      [github.com/lgblgblgb/xemu](https://github.com/lgblgblgb/xemu). Check
      [wnpp](https://bugs.debian.org/cgi-bin/pkgreport.cgi?pkg=wnpp) for an existing RFP/ITP first. Note
      the `xemu` name is effectively spoken for in Debian by the Xbox emulator, so the ITP should use
      `x-emulators` or whatever upstream-facing name Debian prefers. Draft staged (wnpp search
      result: no existing RFP/ITP under any related name; full bug text) at
      [itp-draft.md](2026-08-05-package-x-emulators/itp-draft.md) — **not filed**, per instructions;
      the user files it.
