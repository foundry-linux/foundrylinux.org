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
`Forwarded: not-yet` — **PR not yet opened; see "Follow-ups".**

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

**Not yet done:** publish to the live repo and verify `apt install x-emulators` against
apt.foundrylinux.org (skill Step 6) — that is a release action, tracked below.

## Follow-ups

- [x] Ship `.desktop` files and icons for the seven GUI emulators. Done 2026-08-05: static
      `debian/desktop/<binary>.desktop` per binary (validated with `desktop-file-validate`),
      installed to `/usr/share/applications/`. Upstream ships one generic 48x48 XPM icon
      (`build/xemu-48x48.xpm`, referenced by its own `build/xemu.desktop` template and
      `build/deb-build-simple.sh`) — no per-machine artwork exists, so all seven entries share it,
      vendored verbatim to `debian/icons/x-emulators.xpm` →
      `/usr/share/icons/hicolor/48x48/apps/x-emulators.xpm`. Changelog bumped to
      `-1foundry2`. See verification step 8 below.
- [ ] Open the upstream PR for the `SOURCE_DATE_EPOCH` patch and record the URL in the patch's
      `Forwarded:` header. Draft staged (title, body, clean-apply check against current upstream
      master) at
      [upstream-pr-draft.md](2026-08-05-package-x-emulators/upstream-pr-draft.md) — **not opened**,
      per instructions; the user sends it.
- [ ] File a Debian ITP for `x-emulators` — GPL-2.0-or-later,
      [github.com/lgblgblgb/xemu](https://github.com/lgblgblgb/xemu). Check
      [wnpp](https://bugs.debian.org/cgi-bin/pkgreport.cgi?pkg=wnpp) for an existing RFP/ITP first. Note
      the `xemu` name is effectively spoken for in Debian by the Xbox emulator, so the ITP should use
      `x-emulators` or whatever upstream-facing name Debian prefers. Draft staged (wnpp search
      result: no existing RFP/ITP under any related name; full bug text) at
      [itp-draft.md](2026-08-05-package-x-emulators/itp-draft.md) — **not filed**, per instructions;
      the user files it.
