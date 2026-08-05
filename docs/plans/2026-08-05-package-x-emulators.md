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

1. Universe check — `xemu` and `x-emulators` are both absent from Ubuntu 26.04 universe (only the
   unrelated `gxemul`).
2. Build in a clean `ubuntu:26.04` container via `packages/x-emulators/build.sh`.
3. `lintian` on both the `.deb` and the `.dsc` — **both clean**.
4. Package contents: 7 binaries in `/usr/bin`, 7 man pages in `/usr/share/man/man1`, `${shlibs:Depends}`
   resolved with version constraints, binaries stripped and PIE.
5. Smoke install in a clean container and run. **Note:** upstream refuses to run as root
   (`ERROR: Xemu must not be run as user root`), so the smoke test creates a normal user.
6. End-to-end: run a bare-metal 65CE02 kernel on the *packaged* `xc65` and check the result against a
   host oracle — `0xE0E8`, matching.
7. Metapackage: `foundry-emulators-computers` 1.0.2 builds, lints clean, and pulls `x-emulators`.

## Follow-ups

- [ ] Open the upstream PR for the `SOURCE_DATE_EPOCH` patch and record the URL in the patch's
      `Forwarded:` header.
- [ ] File a Debian ITP for `x-emulators` — GPL-2.0-or-later,
      [github.com/lgblgblgb/xemu](https://github.com/lgblgblgb/xemu). Check
      [wnpp](https://bugs.debian.org/cgi-bin/pkgreport.cgi?pkg=wnpp) for an existing RFP/ITP first. Note
      the `xemu` name is effectively spoken for in Debian by the Xbox emulator, so the ITP should use
      `x-emulators` or whatever upstream-facing name Debian prefers.
- [ ] Consider shipping `.desktop` files and icons for the GUI emulators.
