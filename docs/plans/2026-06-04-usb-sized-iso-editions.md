# Plan: USB-stick-sized ISO editions (4 GB SLIM; 2 GB / 1 GB = installer media)

**Date:** 2026-06-04
**Status:** analysis complete; SLIM edition proposed, awaiting go-ahead

---

## Context

The [create-foundry-usb tool](2026-05-30-create-foundry-usb.md) lets a user write
a Foundry Linux USB stick in one click — but the current **anvil** ISO is
**4.8 GB (~4.47 GiB)**, so it only fits an **8 GB** stick. The question this plan
answers: *what can we **not** put on the image if the target stick is 4 GB or
less?* — and what's the cheapest coherent way to ship a smaller image.

All numbers below come from extracting the package DB straight out of the built
`foundry-anvil-0.9.36-amd64.iso` squashfs and computing each metapackage's
**exclusive dependency closure** (so shared packages aren't double-counted).
Reproducible backing scripts + the captured `dpkg-status` live in
[`docs/investigations/2026-06-04-usb-iso-sizing/`](../investigations/2026-06-04-usb-iso-sizing/).
Model validation: the FULL projection (4.49 GiB) matches the real ISO (4.53 GiB).

**Stick usable capacity** (nominal GB → GiB after FS overhead; `dd`/isoimagewriter
writes the whole image, so the ISO must fit): 1 GB ≈ 0.9 GiB · 2 GB ≈ 1.8 GiB ·
4 GB ≈ 3.6 GiB · 8 GB ≈ 7.4 GiB.

---

## Key finding: the KDE-desktop floor is ~2.44 GiB

A bare Kubuntu/Plasma live desktop is **~2.44 GiB ISO before a single Foundry
tool**. That single number drives everything:

<img src="screenshots/2026-06-04-usb-scenarios.png" width="760">

| Scenario | Projected ISO | Smallest stick |
|---|--:|---|
| NODESK — no GUI, net-installer | ~0.60 GiB | **1 GB** |
| BASE — KDE desktop floor (zero Foundry tools) | ~2.44 GiB | 4 GB |
| MINI — KDE + Blender/WF only | ~2.75 GiB | 4 GB |
| **SLIM — drop heavy toolkit** (recommended) | **~3.11 GiB** | **4 GB** |
| FULL anvil (current) | ~4.49 GiB | 8 GB |

**Consequence:** below ~2.4 GiB you stop trimming packages and start changing
what the image *is*.

- **4 GB stick** → feasible as a trimmed live desktop (SLIM, below).
- **2 GB stick (~1.8 GiB)** → **impossible with Plasma** (already 0.6 GiB under
  the floor). Only paths: a lightweight session (LXQt/Openbox — a different
  desktop) or a **live installer** image (boots Calamares, installs from network).
- **1 GB stick (~0.9 GiB)** → only a **no-GUI text/ncurses net-installer**
  (~0.60 GiB) fits; it installs the base then pulls `foundry-core` from apt.
  A separate product.

So 4 GB is the smallest stick that holds a real Foundry KDE desktop; 2 GB and
1 GB are *installer media*, not a live distro.

---

## The trim list (largest first)

<img src="screenshots/2026-06-04-usb-cutlist.png" width="760">

Uncompressed MiB (ghidra jars / wallpapers compress ~1:1, so their **on-ISO**
savings are even larger than shown):

| # | Cluster | MiB | Action |
|--:|---|--:|---|
| 1 | `foundry-retro-tools` (ghidra 864 · MAME 553 · JDK 286 · radare2/binwalk…) | **1890** | **drop** → heavier edition |
| 2 | `foundry-python-gamedev-extras` (VTK 276 · numba/llvmlite · scipy · librosa) | **554** | **drop** |
| 3 | `worldfoundry` (Blender + WF tools) | 360 | **KEEP** — the point of the distro |
| 4 | `foundry-emulators-consoles` (ScummVM…) | 296 | **drop** |
| 5 | stock Plasma wallpapers (we ship our own) | 255 | **strip** (free) |
| 6 | `foundry-game-frameworks` (SDL/SFML/LÖVE/Tiled…) | 217 | **KEEP** |
| 7 | snapd | 140 | **strip** — already in strip-list but **still present = bug** |
| 8 | `foundry-emulators-computers` | 75 | **drop** |
| 9 | exotic firmware (`linux-firmware-mellanox-spectrum` = datacenter switches…) | 59+ | **strip** (free) |
| 10 | `foundry-image-cli` (CLI image utils) | 29 | **KEEP** (per Will, 2026-06-04) |

Plus `foundry-emulators-vintage` (shares MAME with retro-tools — must drop both
to actually free MAME).

---

## Qt WebEngine — investigated, **KEEP** (not a cut candidate)

Earlier guess was that QtWebEngine (~270 MiB) might be orphaned after the
plasma-welcome/PIM purge. **It is not.** Installed reverse-deps:

| Depends on QtWebEngine | What it is |
|---|---|
| **`plasma-nm`** | network / Wi-Fi system-tray applet — **essential** on live |
| **`khelpcenter`** | KDE Help Center |
| **`plasma-widgets-addons`** | stock Plasma applets |
| `kaccounts-providers`, `signon-ui-qt`, `qml6-module-qtwebview` | KDE online-accounts / web-view |

It is also a base Qt shared library other KDE components link against. Removing it
cascade-breaks the Wi-Fi applet. **Verdict: keep; do not strip.**

---

## Recommended deliverable: a SLIM edition below anvil

The weight is structural — it all rides in via `foundry-core` (shared by anvil,
the devbox, and sprite/atelier), so we can't gut core. The clean vehicle is a new
edition in the `core ⊆ anvil ⊆ sprite ⊆ atelier` hierarchy, sitting **below
anvil**:

**SLIM = anvil − {retro-tools, all emulators, python-gamedev-extras}**, i.e. a
new metapackage depending on:
`foundry-desktop` (KDE welcome + theme), `worldfoundry` (Blender + WF tools),
`foundry-game-frameworks`, `foundry-python-gamedev`, `foundry-image-cli`,
`firefox`, `btop`, `task`. → **~3.11 GiB ISO, fits a 4 GB stick**, keeps the
actual "make a game with WorldFoundry" core. This is the natural
"hand-a-friend-a-USB" image.

Naming TBD (e.g. `foundry-spark` / `foundry-mini`). Implementation mirrors the
existing edition metapackages + ISO targets:

- New `foundry-apt/packages/<slim>/debian/` metapackage (Depends as above).
- New ISO edition wired into `foundry-iso/scripts/build-iso.sh` + Taskfile
  `iso-build` `EDITION=` dispatch (same pattern as anvil/atelier).
- Apply the **free strips** to *all* editions while here: fix the snapd leak,
  drop stock wallpapers (we ship our own), drop exotic/datacenter firmware.
- The [create-foundry-usb](2026-05-30-create-foundry-usb.md) tool should target
  this SLIM `-latest-` image (or offer a small-stick choice), since it's the one
  that fits common giveaway sticks.

2 GB / 1 GB images are explicitly **out of scope** for this plan as *live*
images — captured here as the "installer media" finding for a future plan if a
network-installer product is ever wanted.

---

## Spin-off issues found (worth fixing regardless of edition work)

1. **snapd ships despite the strip-list.** `snapd snap-store` is in
   `strip.list.chroot.purge` + hook 0020, yet snapd (140 MiB) is present in
   0.9.36. The purge isn't taking (likely re-pulled as a Recommends after the
   purge pass, or the autoremove doesn't catch it). Investigate ordering;
   ~140 MiB free across every edition once fixed.
2. **VTK / numba stack (554 MiB) rides in via `foundry-python-gamedev-extras`.**
   `libvtk9.5` alone is 276 MiB. Heavy for what it adds to anvil; reconsider
   whether `-extras` (librosa/numba/scipy/VTK) belongs in the default toolkit or
   should be opt-in even on the full anvil edition.

---

## Verification

1. Regenerate the package-manifest appendix against 0.9.36
   (`foundry-iso/docs/howto-kubuntu-remix-installed-packages.md` currently
   reflects 0.9.33) so the numbers here have a committed source of truth.
2. Build the SLIM edition: `EDITION=<slim> task iso-build` → resulting ISO
   **≤ 3.6 GiB** (fits a 4 GB stick). Confirm ghidra/MAME/ScummVM/JDK/VTK absent
   and Blender + game frameworks + foundry-image-cli present
   (`unsquashfs -cat … var/lib/dpkg/status`).
3. Boot SLIM in QEMU: KDE Plasma desktop comes up, Wi-Fi applet works (proves
   QtWebEngine retained), foundry-welcome appears.
4. snapd fix: rebuild any edition → `snapd` absent from the squashfs dpkg DB.
5. Free-strip check: stock `plasma-workspace-wallpapers` and
   `linux-firmware-mellanox-spectrum` absent; our wallpaper still present.
6. Write SLIM to a real 4 GB USB via `isoimagewriter` and boot it end-to-end.
