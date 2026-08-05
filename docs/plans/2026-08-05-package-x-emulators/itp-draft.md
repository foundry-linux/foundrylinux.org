# Debian ITP draft — `x-emulators`

**Status: DRAFT ONLY. Not filed.** No `reportbug`/mail has been sent to the Debian BTS. This is
staged for the user to review and send by hand.

## wnpp search result (2026-08-05)

Checked [bugs.debian.org/.../pkgreport.cgi?pkg=wnpp](https://bugs.debian.org/cgi-bin/pkgreport.cgi?pkg=wnpp)
(full `src=wnpp;dist=unstable` listing) for any existing RFP/ITP:

```
$ curl -fsS 'https://bugs.debian.org/cgi-bin/pkgreport.cgi?src=wnpp;dist=unstable' \
    | grep -io "xemu[a-z0-9-]*" | sort -u
(no output)
```

No hits for `xemu`, `x-emulators`, or the upstream author's handle `lgblgblgb` anywhere in the
current wnpp bug list. The only Commodore-adjacent entry present is unrelated:

- `#1107930` — **RFP: opencbm** — a driver/library for controlling serial CBM devices like the
  Commodore 1541 disk drive. Not an emulator, not the same upstream, no overlap.

**Conclusion: the field is clear. No existing RFP or ITP covers this package under any name.**

## Naming note (address explicitly, per instructions)

Upstream's own project name is "Xemu", but Debian **already has an unrelated package field
claimed by the name** in our own repo: we ship `xemu` for
[xemu-project/xemu](https://github.com/xemu-project/xemu), the original Xbox emulator (xemu.app) —
and Debian's own archive would hit the identical collision if LGB's Xemu were ever proposed under
its bare name, since `xemu-project/xemu` is itself a live, maintained, popular emulator likely to
reach Debian first or already present via similar naming pressure. To pre-empt that collision at
the point of ITP rather than after upload, this draft proposes the name **`x-emulators`** —
upstream's own full project name, spelled out as "X-Emulators ~ Xemu" in its README and
[doxygen docs](http://doxygen.lgb.hu/xemu/d3/dcc/md__r_e_a_d_m_e.html) — rather than the bare
`xemu`.
This mirrors the precedent Foundry Linux already set for `asar` → `asar-snes-assembler` when the
same kind of collision came up. The ITP text below states this rationale explicitly so a Debian
mentor reviewing the bug doesn't propose reverting to `xemu` without seeing the reason.

---

## Draft bug text

To: submit@bugs.debian.org
Subject: ITP: x-emulators -- emulators for Commodore and Hungarian 8-bit computers

```
Package: wnpp
Severity: wishlist
Owner: Foundry Linux <packages@foundrylinux.org>
X-Debbugs-CC: debian-devel@lists.debian.org

* Package name    : x-emulators
  Version         : 0~git20260129.40dfef0d
  Upstream Contact : Gábor Lénárt (LGB) <lgblgblgb@gmail.com>
* URL             : https://github.com/lgblgblgb/xemu
* License         : GPL-2+
  Programming Lang: C
  Description     : emulators for Commodore and Hungarian 8-bit computers

Xemu ("X-Emulators") is LGB's suite of software emulators for a set of mostly
Commodore and Hungarian 8-bit machines, sharing a common SDL2-based core. It
is the reference emulator for several machines that are otherwise poorly
served by anything currently in Debian, including the Commodore 65 prototype
and the MEGA65.

The suite ships seven emulator binaries:

  * xc65      Commodore 65 / C64DX prototype (CSG 4510 CPU)
  * xmega65   MEGA65 (45GS02 CPU)
  * xclcd     Commodore LCD prototype
  * xvic20    Commodore VIC-20
  * xep128    Enterprise 64/128
  * xprimo    Primo
  * xtvc      Videoton TV Computer

No ROM images are bundled with the package; users supply their own, so there
are no redistribution concerns.

Why it belongs in Debian: xc65 is, to my knowledge, the only *working*
open-source emulator for the Commodore 65 prototype. QEMU has no C65 support;
MAME's "c65" driver is explicitly flagged status: preliminary,
emulation: preliminary, and its own source comments note the ROM window at
$E000 is left unmapped because hooking it up causes bootstrap failures — so
nothing actually executes on it. None of dosbox-x, hatari, fs-uae, openmsx,
or mame cover the C65 usefully either. The MEGA65 (a modern FPGA
reimplementation of an unreleased Commodore machine) has essentially one
emulator, period, and this is it. It also functions as a general 65CE02
execution environment for 6502-family toolchain and compiler work, since
xc65 accepts any 128 KiB file as a ROM without validation.

Packaging note on the name: I am proposing the package name "x-emulators"
rather than the bare upstream name "xemu". Debian packaging efforts for the
*unrelated* xemu-project/xemu (the original Xbox emulator, xemu.app) create
an unavoidable name collision with this package if both were called "xemu" —
we (Foundry Linux, a downstream distribution building on Debian/Ubuntu
packaging conventions) already carry both in our own archive and hit this
exact collision, which is what prompted checking wnpp before proposing this
ITP. "X-Emulators" is upstream's own full project name (used in its README
and doxygen docs, not an invented alias), so this is a rename to upstream's
already-existing longer name, not a fabricated one. Happy to defer to
whatever name a Debian mentor/sponsor prefers if there's an existing
convention for this kind of collision (e.g. an epoch-style suffix instead).

I maintain a Debian-policy-compliant packaging of this suite already, built
via the canonical debian/{control,changelog,rules,source/format,copyright}
layout with debhelper-compat 13, for a downstream apt repository
(apt.foundrylinux.org). I'm willing to prepare the Debian upload directly if
sponsorship is available, or hand off to an existing games/emulation team
maintainer.
```

## Fields worth double-checking before sending

- **Owner** — currently drafted as the Foundry Linux packaging identity used across this repo's
  `debian/control` `Maintainer:` field. Replace with whichever individual DD/DM identity + email
  will actually own the ITP if a real person (not the project alias) should be the contact.
- **Version** in the pseudo-header is our packaged git-snapshot version
  (`0~git20260129.40dfef0d`), which is accurate for *our* package but Debian may prefer just
  citing the upstream commit/date without the Debian-suffix-shaped version string — trim to taste.
- **License** — confirmed `GPL-2+` from `foundry-apt/packages/x-emulators/debian/copyright`
  (`Files: * / Copyright: 2016-2026 Gabor Lenart (LGB) / License: GPL-2+`). Not upgraded to
  "GPL-2.0-or-later" SPDX spelling here since the draft mirrors classic ITP prose style; swap if
  the sender prefers SPDX identifiers.
