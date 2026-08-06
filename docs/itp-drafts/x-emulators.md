# ITP draft: x-emulators

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

The bare upstream name is "Xemu" (project self-description:
"X-Emulators"), and the binaries and this ITP's proposed source/binary
package name are **`x-emulators`, not `xemu`**. That is deliberate, not
a Debian-imposed rename:

There are two unrelated upstreams both called "xemu":

- `xemu-project/xemu`, the original **Xbox** emulator (xemu.app, linked
  above). We already package this one under the name `xemu` in this
  repo.
- [`lgblgblgb/xemu`](https://github.com/lgblgblgb/xemu) — this package,
  Gabor Lenart's suite of Commodore and Hungarian 8-bit emulators.

Since the bare name is unavailable to us, we ship LGB's suite under its
own full project name, `x-emulators`, mirroring how we resolved the
identical collision between `asar` (SNES assembler) and the existing
`asar` name — see the `asar-snes-assembler` draft. We flag rather than
assume this is what Debian will want: Debian may already have a view on
`xemu` (the Xbox one isn't in Debian either, at last check), and a
Debian ITP reviewer/mentor may propose a different disambiguation
(e.g. `xemu-8bit`, `lgb-xemu`). The ITP bug should present the reasoning
above and let ftp-master/mentors weigh in, rather than presenting
`x-emulators` as already settled.

## ITP draft

```
Package name    : x-emulators
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
  compiled from source during the Debian build. `x-emulators` has no
  such issue — `dpkg-buildpackage` compiles all seven emulator binaries
  from the vendored C source via upstream's own build system, no
  prebuilt artifacts are copied in.
- **No tagged upstream release** — like `libvgm` in this same batch,
  x-emulators packages from a pinned git commit (see
  `foundry-apt/packages/x-emulators/build.sh`), because upstream
  publishes no tags or GitHub releases; master is the only ref. The
  "Version" field above is therefore a snapshot version
  (`0~git20260129.40dfef0d`), not a stable upstream release number.
  Debian's ITP process handles git-snapshot packages routinely, but the
  filer should be upfront about it in the bug body and expect a
  reviewer to ask whether upstream intends to ever tag releases.
- Licence is uniformly GPL-2.0-or-later per `debian/copyright` (`Files:
  *` — no separately-licensed bundled components, unlike `bsnes-jg` or
  `libvgm` in this batch, which each carry several different bundled
  licences). One vendored asset, `debian/x-emulators.xpm` (upstream's
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
