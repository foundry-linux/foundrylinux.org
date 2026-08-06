# ITP draft: xemu

## Upstream packaging audit

Audited the pinned upstream payload on 2026-08-05 with
`foundry-apt/scripts/audit-upstream-packaging.sh`. **Positive:** upstream has no `debian/`,
`PKGBUILD`/`.spec`, or PPA/Launchpad/OBS/COPR packaging reference, but does ship its own binary-package
builder at `build/deb-build-simple.sh`. It names the package `xemu`; that naming fact and our resulting
`xemu`/`xemu-xbox` allocation are recorded below.

## wnpp check

Searched wnpp for "xemu", "x-emulators", and "lgblgblgb" (upstream
author Gabor Lenart's handle). **No existing wnpp bug found.** General
web results for "xemu" return unrelated emulator ITPs/RFPs (`cdemu`,
`kcemu`, `xmame`, `emulationstation`, `open-roms`) and, separately, the
original-Xbox `xemu-project/xemu` project
([xemu.app](https://xemu.app)) — but
nothing on bugs.debian.org for LGB's Commodore/Hungarian 8-bit suite
under either name. This is a fresh filing, not an adoption.

## The naming question

Two unrelated upstreams are both called "xemu":

- [`lgblgblgb/xemu`](https://github.com/lgblgblgb/xemu) — **this package**,
  Gabor Lenart's suite of Commodore and Hungarian 8-bit emulators, whose
  own `build/deb-build-simple.sh` has emitted `Package: xemu` since 2016.
- `xemu-project/xemu`, the original **Xbox** emulator ([xemu.app](https://xemu.app)).
  We package it separately as **`xemu-xbox`**.

Neither name is currently taken in Debian — as of 2026-08-05 there is no
`xemu` source or binary package in the archive (`sources.debian.org`
returns no exact match; only the unrelated `gxemul`), and the Xbox
emulator has no ITP either. So nothing is being displaced. We propose
that Debian adopt the same split we ship:

- **`xemu` → this suite.** Upstream has shipped its own Debian package
  under exactly this name for nearly a decade. Naming it anything else
  would mean Debian disagreeing with the software about what it is
  called.
- **`xemu-xbox` → the Xbox emulator.** Qualified, not renamed away: its
  binary is still `xemu`, and the qualifier disambiguates only the
  package.

We originally shipped this suite as `xemu` (upstream's full
project name, "X-Emulators") and gave `xemu` to the Xbox emulator, on the
reasoning that the Xbox project is the better known of the two. That was
reversed on 2026-08-05, for a concrete reason rather than a preference:
upstream's own `.deb` already claims `xemu`, so our old arrangement left
a live collision — his self-built package and our `xemu` shared a dpkg
identity while being unrelated software, and his `cdate` version
(`20260129235930`) outranks `0.8.136`, so installing his would have
silently replaced ours. Giving him the name removes the collision at its
root. See [the rename plan](../plans/2026-08-05-xemu-rename-and-upstream-pr.md).

That said, the name is ftp-master's call, not ours, and we would follow a
maintainer's preference. If a different disambiguation is wanted for the
*Xbox* package, `xemu-xbox` is already the qualified one; if Debian would
rather qualify **this** package instead, our order of preference is:

1. **`xemu-8bit`** — descriptive, accurate (every machine in the suite is
   an 8-bit design: 6502/65CE02/45GS02 and Z80), and it keeps the `xemu`
   root so someone searching "xemu" still finds it.
2. **`x-emulators`** — upstream's full project name; what we shipped
   until 2026-08-05.
3. **`lgb-xemu`** — unambiguous, but names the package after the author's
   handle rather than the software, which Debian generally avoids.

The ITP should put the reasoning above rather than treat the name as
arbitrary.

## Before filing: give upstream a heads-up

Courtesy and coordination, not permission. The Developer's Reference
recommends making upstream aware of Debian packaging, and it is simply
better that he hears "your software is being packaged for Debian" from us
than discovers an ITP bug with his project's name on it. Send it; do not
block on a reply.

The one thing genuinely worth checking first is **whether he is already
doing this**. He plainly has an interest in Debian packaging — the source
tree carries `build/deb-build-simple.sh`, his own binary `.deb` builder
("a very lame binary-level DEB package builder", (C) LGB), which sets
`PROJECT="xemu"` and installs to `/usr/bin` and `/usr/share/xemu`. Someone
who has already written a `.deb` script may well have Debian plans, a
preferred packaging arrangement, or a maintainer lined up. That is the
actual "stepping on toes" risk here, and one message removes it.

Worth raising while we have his attention:

- **Tagged releases.** Not a demand, but the single change that would most
  improve this package's standing: Debian is unenthusiastic about snapshot
  versioning in main, and `0~git20260129.40dfef0d` exists only because
  upstream publishes no tags. He may simply never have had a reason to tag.
- **Naming, as information rather than a question.** Debian will want to
  disambiguate his "xemu" from the unrelated Xbox emulator of the same
  name; our thinking is in the section above. He may have a view and it is
  worth hearing, but the final call belongs to ftp-master, not to us and
  not to him — so tell him what is likely to happen rather than asking him
  to decide it.
- **Staying in the loop** — whether he wants to be listed as upstream
  contact and have Debian bugs forwarded.

**Timing:** PR [lgblgblgb/xemu#448](https://github.com/lgblgblgb/xemu/pull/448)
(our `SOURCE_DATE_EPOCH` reproducibility fix) is open with him now, so this
is a natural follow-up on an existing thread rather than a cold approach.

Note anything he raises in this file. Only a "please don't" or a
"packaging is already under way" should actually stop the filing.

## ITP draft

```
Package name    : xemu
Version         : 0~git20260129.40dfef0d (no tagged upstream release;
                  packaged from a pinned git commit)
Upstream contact: Gabor Lenart (LGB) <lgblgblgb@gmail.com>
URL             : https://github.com/lgblgblgb/xemu
Licence         : GPL-2.0-or-later
Programming lang: C
Description     : emulators for Commodore and Hungarian 8-bit computers
```

Xemu ("X-Emulators") is LGB's suite of emulators for a set of mostly
Commodore and Hungarian 8-bit machines, built on a shared SDL2 core. It
is the reference emulator for several machines that are otherwise
poorly served, including the Commodore 65 prototype and the MEGA65.
This package ships seven emulators:

  * `xc65`     Commodore 65 / C64DX prototype (CSG 4510 CPU)
  * `xmega65`  MEGA65 (45GS02 CPU)
  * `xclcd`    Commodore LCD prototype
  * `xvic20`   Commodore VIC-20
  * `xep128`   Enterprise 64/128
  * `xprimo`   Primo
  * `xtvc`     Videoton TV Computer

No ROM images are bundled — users supply their own, so the package
carries no redistribution concerns. The C65 and MEGA65 emulators are of
particular interest beyond retrocomputing: `xc65` loads any 128 KiB
file as its system ROM without validating it, which makes it usable as
a bare-metal execution environment for 65CE02/45GS02 toolchain
development, not just a consumer emulator.

Debian currently has no package covering any of these seven machines.

## Notes

- **Builds from source**, unlike several other packages in this same
  ITP batch. This is worth stating explicitly: the sweep that produced
  this batch flagged `ldtk`, `mesen2`, `pvsneslib`, and (pending
  verification) `m8te` as Debian-main-disqualifying because they ship
  prebuilt upstream binaries repacked into a `.deb` rather than being
  compiled from source during the Debian build. `xemu` has no
  such issue — `dpkg-buildpackage` compiles all seven emulator binaries
  from the vendored C source via upstream's own build system, no
  prebuilt artifacts are copied in.
- **No tagged upstream release** — like `libvgm` in this same batch,
  xemu packages from a pinned git commit (see
  `foundry-apt/packages/xemu/build.sh`), because upstream
  publishes no tags or GitHub releases; master is the only ref. The
  "Version" field above is therefore a snapshot version
  (`0~git20260129.40dfef0d`), not a stable upstream release number.
  Debian's ITP process handles git-snapshot packages routinely, but the
  filer should be upfront about it in the bug body and expect a
  reviewer to ask whether upstream intends to ever tag releases.
- Licence is uniformly GPL-2.0-or-later per `debian/copyright` (`Files:
  *` — no separately-licensed bundled components, unlike `bsnes-jg` or
  `libvgm` in this batch, which each carry several different bundled
  licences). One vendored asset, `debian/xemu.xpm` (upstream's
  single generic 48x48 icon, reused across all seven `.desktop`
  entries since upstream ships no per-machine artwork), is also
  GPL-2.0-or-later, matching the rest of upstream's source.
- Build-Depends: `debhelper-compat (= 13)`, `pkgconf`, `libsdl2-dev`,
  `libgtk-3-dev`, `libreadline-dev`, `zlib1g-dev` — all in Debian main
  already.
- lintian is clean on both the built `.deb` and the source `.dsc`.
  Seven hand-written man pages are shipped (one per emulator binary).
  Hardening (PIE/relro/bindnow) is enabled; hardening flags are
  appended to the generated build config rather than passed on the
  `make` command line, so they don't clobber upstream's own `CFLAGS`.
  `.desktop` entries and the icon described above are shipped so all
  seven emulators appear in application menus.
- We carry one patch, `debian/patches/0001-honour-SOURCE_DATE_EPOCH.patch`
  (a reproducible-builds fix for embedded build timestamps), already
  forwarded upstream as
  [lgblgblgb/xemu#448](https://github.com/lgblgblgb/xemu/pull/448) —
  worth mentioning in the ITP bug as evidence of active upstream
  engagement, not just a downstream carry-patch.
