# Plan: trim anvil to fit a 4 GB stick — move ghidra to atelier

> **SUPERSEDED 2026-06-20 — the 4 GB-stick goal was dropped.** Adding the LLVM-backed
> test-case reducers (`cvise`+`delta`, ~330 MiB) to `foundry-retro-tools` pushed the default
> **anvil** ISO past a 4 GB stick for good, so the giveaway stick now targets **8 GB** (see
> [`2026-05-30-create-foundry-usb.md`](2026-05-30-create-foundry-usb.md)). Replaced by
> [`2026-06-20-add-reducers-to-retro-tools.md`](2026-06-20-add-reducers-to-retro-tools.md).
> **ghidra stays atelier-only regardless** (~860 MiB of jars) — that placement is unchanged.
> Kept for history; do not implement.

## Context

Goal: the default **anvil** ISO should fit a **4 GB USB stick** (so the
[create-foundry-usb](../../SRC/foundrylinux.org/docs/plans/2026-05-30-create-foundry-usb.md)
tool can write a giveaway stick), keeping **the most** that fits — not a stripped
"slim" image.

Full anvil today = **4.53 GiB**; a 4 GB stick holds **~3.6 GiB** → trim ~0.9 GiB.
Analysis (`docs/investigations/2026-06-04-usb-iso-sizing/`) showed the blocker is
`foundry-retro-tools`, which bundles **ghidra (~0.8 GiB ISO, near-incompressible
jars) + MAME (~0.45) + the JDK (~0.14)** — ~1.4 GiB sitting on the 2.44 GiB
bare-KDE floor. ghidra is the single biggest, least-compressible item, so cutting
it frees the most while keeping everything else.

**Decisions (Will, 2026-06-04):**
1. **Move ghidra from anvil → atelier** (the "complete edition"). Its OpenJDK
   runtime is pulled *only* by ghidra, so the JDK follows it automatically. MAME
   and the rest of the retro toolkit (radare2, dasm, cc65, z80, binwalk, f9dasm,
   libvgm, vgmstream, sox, xa65) **stay in anvil**.
2. **Trim anvil itself** — anvil *becomes* the ~3.3 GiB image; no separate edition.

**Projected result:** anvil 4.53 − ghidra+JDK (~0.92) − free strips (~0.31) ≈
**~3.3 GiB** → fits a 4 GB stick, keeping MAME, all emulators, the
VTK/numba/scipy stack, Blender + WorldFoundry, game frameworks, image-cli.

## Changes

### A. Move ghidra+JDK from core → atelier (apt metapackages)

- `foundry-apt/packages/foundry-retro-tools/debian/control` — **remove `ghidra`**
  from `Depends:` (retro-tools is pulled by `foundry-core`, so this drops ghidra
  from core/anvil/sprite). Bump `debian/changelog` (1.0.x).
- `foundry-apt/packages/foundry-atelier/debian/control` — **add `ghidra`** to
  `Depends:` so the complete edition still ships it (JDK rides along). Bump
  changelog. *(Confirm `foundry-atelier` package exists; per CLAUDE.md edition
  table it does.)*
- Rebuild + verify: `task build`; `foundry-retro-tools` no longer lists ghidra,
  `foundry-atelier` resolves ghidra.

### B. Free strips — apply at the ISO level (all editions, no functional loss)

In `foundry-iso/config/package-lists/strip.list.chroot.purge` + hook
`0020-strip-kubuntu-bloat.hook.chroot`:
- **Fix the snapd leak** — `snapd snap-store` is already listed yet snapd
  (140 MiB) still ships in 0.9.36 (re-pulled as a Recommends after the purge
  pass). Repeat the purge after the autoremove, or add `--purge` of `snapd` late
  enough that nothing re-pulls it; verify it's gone.
- **Drop stock Plasma wallpapers** — `plasma-workspace-wallpapers`,
  `breeze-wallpaper` (~255 MiB; we ship our own via `foundry-kde-theme`). Confirm
  they're Recommends-not-Depends of plasma so the purge is clean.
- **Drop exotic firmware** — `linux-firmware-mellanox-spectrum` (datacenter
  switches) and similarly-irrelevant `linux-firmware-*` for hardware a desktop
  live image won't see. Be conservative: keep nvidia/intel/amd/realtek wifi.

### C. Docs + TODO — update EVERY place that says ghidra ∈ core/anvil/retro-tools

Driver step: **`grep -rin "ghidra" CLAUDE.md README.md docs/ foundry-apt/ foundry-setup/ foundry-iso/`**
and fix each hit that asserts ghidra's edition placement (skip pure
vendoring/packaging refs — ghidra stays a vendored upstream in foundry-apt, it
just changes which edition depends on it). Known hits to update:

- **Package Descriptions** (the prose inside `debian/control`, easy to miss):
  - `foundry-apt/packages/foundry-core/debian/control` — its Description blurb
    lists *"the retro-porting toolkit (MAME, Ghidra, dasm, …)"*; **drop Ghidra**.
  - `foundry-apt/packages/foundry-retro-tools/debian/control` — Description /
    long-desc enumerating ghidra; **drop it**.
  - `foundry-apt/packages/foundry-atelier/debian/control` — **add ghidra** to the
    Description (and the Depends, per §A).
- **Edition/category tables & roadmap docs:**
  - `CLAUDE.md` — edition table (§Editions, the `core` line listing retro-tools)
    and the Phase-0 `install-foundry-retro-tools.sh` comment (line ~51) that lists
    `ghidra`. Note ghidra is now **atelier-tier**.
  - `foundry-apt/README.md` — the edition/category metapackage table.
  - `README.md` (repo root) if it enumerates editions/tools.
  - `docs/plans/2026-05-22-phase-3-foundry-iso.md` — anvil/atelier size targets
    (anvil now ~3.3 GiB / 4 GB-stick; ghidra weight moves to atelier).
  - `docs/investigations/2026-05-29-state-of-the-distro.md` — if it cites edition
    contents or ISO sizes.
- **Scripts:**
  - `foundry-setup/install-foundry-retro-tools.sh` — comment/echo listing ghidra
    among installed tools; remove (it now only comes via atelier paths).
- **This analysis:**
  - `docs/plans/2026-06-04-usb-sized-iso-editions.md` — replace the SLIM
    recommendation with this "trim anvil, move ghidra → atelier" outcome;
    regenerate the scenario chart with the **anvil-4GB (~3.3 GiB)** bar (script:
    `docs/investigations/2026-06-04-usb-iso-sizing/charts.py`).
  - `foundry-iso/docs/howto-kubuntu-remix-installed-packages.md` — regenerate the
    manifest appendix against the new anvil (drops the ghidra row; new size
    header) so it's the committed size source-of-truth.
- **TODO.md** — fold the three SLIM/snapd/extras items into one "trim anvil to
  4 GB (ghidra → atelier) + free strips" item; note create-usb now targets anvil.

Consequence to call out in the docs: the **devbox** installs `foundry-core`, so it
loses ghidra too — acceptable (ghidra is heavy for a container; atelier is the
"everything" tier). Update the devbox note if it claims ghidra is included.

## Critical files

| File | Change |
|---|---|
| `foundry-apt/packages/foundry-retro-tools/debian/{control,changelog}` | remove `ghidra` dep; bump |
| `foundry-apt/packages/foundry-atelier/debian/{control,changelog}` | add `ghidra` dep; bump |
| `foundry-iso/config/package-lists/strip.list.chroot.purge` | snapd fix + wallpapers + exotic firmware |
| `foundry-iso/config/hooks/0020-strip-kubuntu-bloat.hook.chroot` | matching `_purge` lines / ordering fix |
| `foundry-apt/packages/foundry-core/debian/control` | drop "Ghidra" from Description blurb |
| `docs/plans/2026-06-04-usb-sized-iso-editions.md` + chart | pivot to this outcome; regen chart |
| `foundry-iso/docs/howto-kubuntu-remix-installed-packages.md` | regenerate manifest (no ghidra; new size) |
| `CLAUDE.md` | edition table + retro-tools comment: ghidra → atelier; devbox note |
| `foundry-apt/README.md`, `README.md` | edition/category tables |
| `docs/plans/2026-05-22-phase-3-foundry-iso.md`, `docs/investigations/2026-05-29-state-of-the-distro.md` | anvil/atelier size + edition contents |
| `foundry-setup/install-foundry-retro-tools.sh` | comment/echo listing ghidra |
| `TODO.md` | reconcile the 4 GB / ghidra / snapd items |
| *(driver)* `grep -rin ghidra …` | catch any straggler edition-placement refs |

## Verification

1. `task build` (foundry-apt) → `foundry-retro-tools` `.deb` no longer
   `Depends: ghidra`; `foundry-atelier` does. `task verify`/lintian clean.
2. `EDITION=anvil task iso-build` → ISO **≤ 3.6 GiB** (target ~3.3). Confirm via
   `unsquashfs -cat … var/lib/dpkg/status`: **ghidra + openjdk-21-\* absent**;
   **MAME, ScummVM, Blender, SDL/SFML, VTK present**; **snapd absent**; stock
   wallpapers + mellanox firmware absent, our wallpaper present.
3. `EDITION=atelier task iso-build` (or `apt-cache`/metapackage resolve) → ghidra
   present in atelier.
4. Boot anvil in QEMU: KDE desktop + Wi-Fi applet work, foundry-welcome appears.
5. Regenerate `foundry-iso/docs/howto-kubuntu-remix-installed-packages.md` against
   the new anvil as the committed size source-of-truth.
6. Write the anvil ISO to a real 4 GB stick via isoimagewriter; boot end-to-end.
7. **Doc sweep clean:** re-run `grep -rin "ghidra" CLAUDE.md README.md docs/
   foundry-apt/ foundry-setup/ foundry-iso/` and confirm every remaining hit is a
   *vendoring/packaging* reference or an *atelier* placement — no doc still claims
   ghidra ships in core/anvil/retro-tools.
