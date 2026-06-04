# Plan: Wire ppsspp + snes9x-gtk into ISO/scripts (post-packaging)

## Context

ppsspp 1.20.4 and snes9x-gtk 1.63 were packaged and committed in the previous session. Both are wired into `foundry-emulators-consoles-heavy` (Depends field + description updated, v1.0.3 changelog). `foundry-emulators-consoles-heavy` is in `foundry-atelier`'s Depends, which is what the atelier ISO installs. The packages are **not yet published** to apt.foundrylinux.org — that happens on the next tag push. The user wants to update ISO config and scripts to reflect the new emulators.

**Key finding:** The ISO build and `build-all.sh` already pick up the new packages automatically — no structural changes needed. The only missing piece is a cosmetic description update in `foundry-atelier/debian/control` (currently says "Dolphin, PCSX2, Yuzu, RetroArch" without ppsspp or snes9x-gtk) and its changelog bump.

---

## What needs changing

### 1. `foundry-apt/packages/foundry-atelier/debian/control`

Update the `foundry-emulators-consoles-heavy` bullet in the Description from:

```
  * foundry-emulators-consoles-heavy     Dolphin, PCSX2, Yuzu, RetroArch
```

to:

```
  * foundry-emulators-consoles-heavy     Dolphin, PCSX2, Yuzu, RetroArch, ppsspp, snes9x-gtk
```

### 2. `foundry-apt/packages/foundry-atelier/debian/changelog`

Add a new entry bumping to `0.9.1`:

```
foundry-atelier (0.9.1) resolute; urgency=low

  * Note ppsspp (PSP) and snes9x-gtk (SNES) in description; both now
    ship via foundry-emulators-consoles-heavy 1.0.3.

 -- Foundry Linux <packages@foundrylinux.org>  Sat, 23 May 2026 00:00:00 +0000
```

---

## Why nothing else needs changing

- **`foundry-iso/config/package-lists/foundry.list.chroot`** — lists `foundry-atelier`; ppsspp and snes9x-gtk arrive transitively. No change.
- **`foundry-iso/scripts/build-iso.sh`** — calls `lb chroot` which installs the metapackage chain. No change.
- **`0020-strip-kubuntu-bloat.hook.chroot`** — only purges KDE PIM/LibreOffice/snap; does not touch emulators. No change.
- **`foundry-apt/scripts/build-all.sh`** — auto-discovers any `packages/*/build.sh`. ppsspp and snes9x-gtk both have one. CI will build them without any modification.
- **`foundry-setup/install.sh`** — role `atelier` installs `foundry-atelier` which transitively pulls everything. No change.

---

## Dependency tracking for automatic ISO rebuild (Taskfile)

Currently `iso-build` and the apt publish step have no dependency link — you have to remember to rebuild the ISO after publishing new packages. The fix is two-pronged:

### A. File fingerprinting on `iso-build`

Add `sources:` + `generates:` to the `iso-build` task in `Taskfile.yml`. Go Task skips the task entirely if sources haven't changed since the ISO was last built:

```yaml
  iso-build:
    desc: "Build the ISO locally (EDITION=anvil|atelier, default: anvil)"
    dir: foundry-iso
    sources:
      - ../foundry-apt/packages/**/*
      - config/**/*
      - scripts/build-iso.sh
    generates:
      - dist/foundry-{{.EDITION | default "anvil"}}-*.iso
    cmds:
      - EDITION='{{.EDITION | default "anvil"}}' bash scripts/build-iso.sh
```

This means `task iso-build EDITION=atelier` is a no-op unless a package file or ISO config changed — no manual "did I remember to rebuild?" needed.

### B. Composite `atelier-publish` task

Add a new task that chains the full atelier release in order:

```yaml
  atelier-publish:
    desc: "Build atelier ISO (if stale), sign, upload, and update index"
    cmds:
      - task: iso-build
        vars: { EDITION: atelier }
      - task: iso-bump
      - task: iso-sign
        vars: { EDITION: atelier }
      - task: iso-upload
        vars: { EDITION: atelier }
      - task: iso-upload-index
```

`iso-build` is fingerprinted (step A), so if packages haven't changed it's skipped and only the sign+upload steps run. If packages did change, the ISO rebuilds first.

> **Note:** `iso-build` takes ~30–60 min (full livebuild inside Docker). The fingerprinting means you don't pay that cost on every `atelier-publish` run — only when `foundry-apt/packages/**/*` or the ISO config actually changed.

---

## Commit + release

After making the edits above:

```bash
git add foundry-apt/packages/foundry-atelier/debian/control \
        foundry-apt/packages/foundry-atelier/debian/changelog \
        Taskfile.yml
git commit -m "chore(apt): note ppsspp + snes9x-gtk in foundry-atelier description; add atelier-publish task"
```

Then push a tag to trigger the foundry-apt CI publish:

```bash
git tag v<next> && git push origin v<next>
```

CI builds ppsspp, snes9x-gtk, foundry-emulators-consoles-heavy 1.0.3, and foundry-atelier 0.9.1, and publishes all to apt.foundrylinux.org. After that, `task atelier-publish` rebuilds, signs, and uploads the atelier ISO.

---

## Verification

1. After tag push: `docker run --rm ubuntu:26.04 bash -c 'apt-get update && apt-cache policy ppsspp snes9x-gtk'` — both should show `Candidate:` from apt.foundrylinux.org.
2. `dpkg-deb -I dist/foundry-atelier_0.9.1_all.deb` — Description mentions ppsspp and snes9x-gtk.
3. `task iso-build EDITION=atelier` (second run, no changes) — exits immediately: `Task "iso-build" is up to date`.
4. Touch any file under `foundry-apt/packages/ppsspp/` → `task iso-build EDITION=atelier` triggers a full rebuild.
