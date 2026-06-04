# Plan: USB-stick-sized ISO editions (4 GB SLIM; 2 GB / 1 GB = installer media)

**Date:** 2026-06-04
**Status:** implemented (0.9.37) — anvil trimmed to ~3.23 GiB; fits 4 GB stick

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
| **anvil 0.9.37** (ghidra→atelier, wallpapers+snapd stripped) | **~3.23 GiB** | **4 GB** ✅ |
| anvil 0.9.36 (baseline) | ~4.49 GiB | 8 GB |

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

**DECISION (Will, 2026-06-04): keep the most — cut only ghidra.** Because ghidra
is the one near-incompressible giant (~0.8 GiB on the ISO), cutting *just it*
(plus its JDK and the two safe free strips) frees the needed ~0.9 GiB while
keeping MAME, every emulator, the sci-Python stack, Blender, and frameworks. The
Action column reflects that final call, not the largest-possible cut:

| # | Cluster | MiB | Final action |
|--:|---|--:|---|
| 1a | **ghidra** (within retro-tools; ~0.8 GiB ISO, jars) | 864 | ✅ **→ atelier** (`foundry-retro-tools` 1.0.7; hook 1010 autoremoves) |
| 1b | OpenJDK (ghidra's only consumer) | 286 | ✅ follows ghidra → atelier (autoremoved by hook 1010) |
| 1c | MAME + rest of `foundry-retro-tools` | ~553 | ✅ **KEEP** in anvil |
| 2 | `foundry-python-gamedev-extras` (VTK · numba · scipy · librosa) | 554 | ✅ **KEEP** |
| 3 | `worldfoundry` (Blender + WF tools) | 360 | ✅ **KEEP** — the point of the distro |
| 4 | `foundry-emulators-consoles` (ScummVM…) | 296 | ✅ **KEEP** |
| 5 | stock Plasma **extra** wallpapers (`plasma-workspace-wallpapers`) | 217 | ✅ **stripped** (Recommends-only; `strip.list.chroot.purge`) |
| 6 | `foundry-game-frameworks` (SDL/SFML/LÖVE/Tiled…) | 217 | ✅ **KEEP** |
| 7 | snapd | 140 | ✅ **stripped** — apt pin `no-snapd.pref` (priority −1) + hook 0020 purge |
| 8 | `foundry-emulators-computers` | 75 | ✅ **KEEP** |
| 9 | exotic firmware (`linux-firmware-mellanox-spectrum`) | 59 | **NOT stripped** — hard Depends of the `linux-firmware` umbrella; apt purge cascades and can autoremove ALL firmware |
| 10 | `foundry-image-cli` (CLI image utils) | 29 | ✅ **KEEP** |
| — | `breeze-wallpaper` | 38 | **KEEP** — hard Depends of `breeze` (default Plasma style) |

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

## Implemented: trim anvil itself, move ghidra → atelier

Not a new "slim" edition — **anvil itself becomes the ~3.23 GiB 4 GB-stick image**,
and ghidra (the only thing that has to go) moves up to the atelier "complete
edition". Everything else anvil shipped stays.

**Metapackages** (apt.foundrylinux.org): ✅
- ~~`foundry-retro-tools` 1.0.6 → **1.0.7**~~: drop `ghidra` from `Depends` (+ its
  `Suggests: java-common`). Since retro-tools is pulled by `foundry-core`, this
  removes ghidra from core/anvil/sprite. MAME and the rest stay.
- ~~`foundry-atelier` 0.9.1 → **0.9.2**~~: add `ghidra` to `Depends` (its OpenJDK
  rides along). The complete edition still ships it.
- ~~`foundry-core` 1.0.2 → **1.0.3**~~: Description-only (ghidra now atelier).

**ISO build** (`foundry-iso/`): ✅
- ~~`strip.list.chroot.purge`: add `plasma-workspace-wallpapers`~~ (217 MiB, safe —
  Recommends-only; we ship our own wallpaper). **Not** `breeze-wallpaper`
  (hard Depends of `breeze`) and **not** any `linux-firmware-*` (hard Depends of
  the umbrella → cascade risk).
- ~~New hook **`1010-trim-atelier-only-pkgs.hook.chroot`**~~: runs after the
  local-debs install; `apt-mark auto ghidra openjdk-21-*` then
  `apt-get autoremove --purge`. Dependency-driven, so it removes ghidra+JDK from
  anvil/sprite (now orphaned by retro-tools 1.0.7) but leaves them on atelier
  (where `foundry-atelier` Depends ghidra). Never cascades. Gated off atelier as
  a belt-and-suspenders guard. This handles the transient pull from hook 0030
  (`apt install foundry-anvil` against the still-published 1.0.6) until the
  metapackage bump is published.
- ~~snapd neutralised by `config/apt/preferences.d/no-snapd.pref` pin~~ (priority −1)
  + hook-0020 purge — no extra work.

**create-foundry-usb**: targets the anvil `-latest-` image directly — anvil is
now the 4 GB-capable image, so no separate edition for the tool to choose.

2 GB / 1 GB images stay **out of scope** as *live* images (the KDE floor is
~2.44 GiB) — captured above as the "installer media" finding for a possible future
network-installer product.

---

## Spin-off findings

1. **snapd** — was shipping in 0.9.36 despite the strip-list (re-pulled as a
   Recommends). Now fixed by an apt pin (`no-snapd.pref`, priority −1) applied
   before package install. ~140 MiB freed across every edition.
2. **VTK / numba stack (554 MiB) via `foundry-python-gamedev-extras`** —
   `libvtk9.5` alone is 276 MiB. We chose to **keep** it in anvil (keep-the-most),
   but it remains the next-fattest lever if anvil ever needs to shrink further;
   worth revisiting whether `-extras` (librosa/numba/scipy/VTK) should be opt-in.
   Tracked in TODO.

---

## Verification

1. ~~`task build` (foundry-apt) → `foundry-retro-tools` 1.0.7 `.deb` no longer
   `Depends: ghidra`; `foundry-atelier` 0.9.2 does.~~ ✅ (2026-06-04)
2. `EDITION=anvil task iso-build` → ISO **≤ 3.6 GiB** (target ~3.3). Confirm via
   `unsquashfs -cat … var/lib/dpkg/status`: **ghidra + openjdk-21-\* absent**;
   **MAME, ScummVM, Blender, SDL/SFML, VTK present**; **snapd absent**;
   `plasma-workspace-wallpapers` absent, our wallpaper present.
3. `EDITION=atelier task iso-build` (or metapackage resolve) → ghidra present.
4. Boot anvil in QEMU: KDE desktop + Wi-Fi applet work (QtWebEngine retained),
   foundry-welcome appears.
5. Regenerate `foundry-iso/docs/howto-kubuntu-remix-installed-packages.md` against
   the new anvil as the committed size source-of-truth.
6. Write the anvil ISO to a real 4 GB stick via isoimagewriter; boot end-to-end.
6. Write SLIM to a real 4 GB USB via `isoimagewriter` and boot it end-to-end.
