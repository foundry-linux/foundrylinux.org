# Plan: "Create a Foundry Linux USB" launcher

**Date:** 2026-05-30
**Status:** draft

---

## Problem

`usb-creator-kde` is installed on Foundry Linux but is unreachable in any
meaningful way. A user who wants to hand a friend a Foundry Linux USB stick
would need to independently:

1. Find the ISO download URL (not obvious — iso.foundrylinux.org has no human landing page yet)
2. Download a 3–5 GB file with no guidance on where to put it
3. Discover `usb-creator-kde` in the system menu
4. Navigate the file picker to the downloaded ISO

That's four undiscoverable steps for a user who may be new to Linux. The
feature exists; the path to it doesn't.

---

## Goal

One click from the installed desktop produces a written Foundry Linux USB stick.
No terminal, no manual ISO hunting.

---

## User journey (target state)

1. User opens **foundry-welcome** → clicks **"Share Foundry Linux"** (new action)
   *or* launches **"Create Foundry Linux USB"** from the application menu.
2. A small dialog confirms: "Download the latest Foundry Linux ISO (~X GB) and
   write it to a USB stick?"
3. ISO downloads to `~/Downloads/foundry-linux-anvil-latest.iso` with a
   progress bar. SHA-256 is verified against the published `.sha256` file.
4. `usb-creator-kde` opens with the ISO pre-selected (passed as a CLI argument).
5. User inserts USB, clicks "Write", done.

---

## Components

### 1. `foundry-create-usb.sh` — download + launch script

Installed to `/usr/local/bin/foundry-create-usb.sh`.

```
foundry-create-usb.sh
  ├── fetch https://iso.foundrylinux.org/foundry-anvil-latest-amd64.iso.sha256
  ├── check if ~/Downloads/foundry-linux-anvil-latest.iso already exists
  │     └── if so, verify sha256 — skip download if it matches
  ├── download ISO to ~/Downloads/foundry-linux-anvil-latest.iso
  │     └── show progress via kdialog --progressbar (or zenity --progress fallback)
  ├── verify sha256 — abort with kdialog --error if mismatch
  └── exec usb-creator-kde ~/Downloads/foundry-linux-anvil-latest.iso
```

The latest ISO is always available at a stable URL (R2 `-latest-` pointer object
written by `scripts/upload-iso.sh`):

```
https://iso.foundrylinux.org/foundry-anvil-latest-amd64.iso
https://iso.foundrylinux.org/foundry-anvil-latest-amd64.iso.sha256
```

**Does `usb-creator-kde` accept a file argument?** Needs verification. If not,
fall back to opening Dolphin at `~/Downloads/` alongside `usb-creator-kde` so
the ISO is one click away in the file picker.

### 2. `install-foundry-linux.desktop` — application menu entry

Installed to `/usr/share/applications/create-foundry-usb.desktop`.

```ini
[Desktop Entry]
Type=Application
Name=Create Foundry Linux USB
GenericName=USB Image Writer
Comment=Download and write a Foundry Linux USB installer
Exec=/usr/local/bin/foundry-create-usb.sh
Icon=drive-removable-media-usb
Terminal=false
StartupNotify=true
Categories=System;
```

This gives the action a home in System → "Create Foundry Linux USB" in the
application launcher, independent of foundry-welcome.

### 3. foundry-welcome integration

Add a **"Share Foundry Linux"** button to the foundry-welcome actions screen
(the same screen that has "Install Foundry Linux", "Explore", etc.). Tapping it
runs `foundry-create-usb.sh`.

This is the highest-visibility entry point for new users who don't yet know
their way around KDE's app menu.

### 4. Packaging

Ship both the script and the `.desktop` file via the **`foundry-desktop`** deb
(already owns `/usr/share/applications/install-foundry-linux.desktop` and
`/usr/local/bin/launch-calamares.sh`). Add to `debian/install`:

```
usr/local/bin/foundry-create-usb.sh
usr/share/applications/create-foundry-usb.desktop
```

`foundry-desktop` already depends on `foundry-welcome` and `foundry-kde-theme`;
add `usb-creator-kde` and `kdialog` as new Depends (both are small and
appropriate for a KDE desktop package).

---

## Out of scope

- Automatic USB ejection / post-write instructions — usb-creator-kde handles this.
- Building an atelier USB — anvil only; atelier is parked.
- In-live-session "write this live USB to another USB" — that's a different
  feature (and needs root, overlayfs complications, etc.).

---

## Open questions

1. **Does `usb-creator-kde` accept a pre-selected ISO path on the CLI?**
   Check `usb-creator-kde --help`. If not, consider `isoimagewriter` (KDE ISO
   Image Writer, available in Ubuntu universe) which does accept a file argument.
   `isoimagewriter` would become the Depends instead of `usb-creator-kde`.

2. **Should we keep `usb-creator-kde` in the purge list or not?**
   Currently not purged (restored after this discussion). If we switch to
   `isoimagewriter`, we can explicitly pull in whichever tool we choose rather
   than relying on kubuntu-desktop's bundled choice.

3. **Is `kdialog` reliable enough for the progress bar, or should we use a
   wrapper terminal (`konsole -e wget …`)?**
   `kdialog --progressbar` requires the calling script to send DBus updates —
   workable but fiddly. A `konsole` window showing `curl --progress-bar` is
   simpler to implement and more transparent to the user.

---

## Verification

1. On an installed Foundry Linux system (not live), confirm "Create Foundry
   Linux USB" appears in the application menu under System.
2. Launch it. Confirm the progress UI appears and the ISO downloads to
   `~/Downloads/foundry-linux-anvil-latest.iso`.
3. Confirm sha256 is checked and mismatches abort with an error dialog.
4. Confirm `usb-creator-kde` (or `isoimagewriter`) opens with the ISO
   pre-selected.
5. Confirm the action appears in foundry-welcome and triggers the same flow.
6. Confirm the `.deb` installs cleanly: `dpkg -L foundry-desktop | grep create-usb`.
