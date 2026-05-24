# ISO autologin debug: QEMU fix + live sshd

Date: 2026-05-24

## Changes from base Kubuntu

Unified diffs of every file we change vs stock `kubuntu-desktop` + `casper`.
Graphics files (SDDM theme, Plymouth theme, wallpaper) are shipped by
`calamares-settings-foundry-linux` and referenced by path only — see
[`foundry-apt/packages/calamares-settings-foundry-linux/`](../../foundry-apt/packages/calamares-settings-foundry-linux/).

### Packages added (`foundry.list.chroot`)
`openbox`, `calamares`, `calamares-settings-foundry-linux`, `casper`, `user-setup`

### Packages removed (`strip.list.chroot.purge`)
`libreoffice-*`, Kontact/KMail/Akonadi, KDE PIM, DigiKam, KDE games, KDEConnect, KRDC/KRFB, KTorrent, `snapd`/`snap-store`, `usb-creator-kde`, `system-config-printer-kde`

### File diffs

> Note: the `Ubuntu Budgie` elif in `15autologin` is a casper 26.04.2 upstream
> addition (vs the 25.10.2 baseline); our patch is the `sddm_session_name` block
> and the `Session=$sddm_session_name` substitution.

```diff
--- a/casper-bottom/15autologin
+++ b/casper-bottom/15autologin
@@ -59,12 +59,20 @@
     # Lubuntu
     elif [ -f /root/usr/share/xsessions/lubuntu-live-environment.desktop ]; then
         sddm_session=lubuntu-live-environment.desktop
+    # Ubuntu Budgie
+    elif [ -f /root/usr/share/wayland-sessions/budgie-desktop-live.desktop ]; then
+        sddm_session=budgie-desktop-live.desktop
     fi
 
+    # Strip .desktop suffix if present; SDDM expects bare session names.
+    # Default to "plasma" when the file-existence checks above all missed
+    # (e.g. none of the known kubuntu/studio/lubuntu/budgie files exist).
+    sddm_session_name="${sddm_session%.desktop}"
+    sddm_session_name="${sddm_session_name:-plasma}"
     cat >>/root/etc/sddm.conf <<EOF
 [Autologin]
 User=$USERNAME
-Session=$sddm_session
+Session=$sddm_session_name
 EOF
 fi
--- a/casper-bottom/16foundry-autologin
+++ b/casper-bottom/16foundry-autologin
@@ -0,0 +1,24 @@
+#!/bin/sh
+PREREQ=""
+DESCRIPTION="Configuring SDDM autologin for Foundry Linux live session..."
+
+prereqs() { echo "$PREREQ"; }
+case $1 in prereqs) prereqs; exit 0 ;; esac
+
+. /scripts/casper-functions
+log_begin_msg "$DESCRIPTION"
+
+# Fix any Session= (empty or wrong) that casper's 15autologin may have written.
+if [ -f /root/etc/sddm.conf ]; then
+    sed -i -e 's/^Session=$/Session=plasma/' \
+           -e 's/^Session=plasma\.desktop$/Session=plasma/' \
+           /root/etc/sddm.conf
+else
+    printf '[Autologin]\nUser=%s\nSession=plasma\nRelogin=false\n' \
+        "${USERNAME:-user}" > /root/etc/sddm.conf
+fi
+
+log_end_msg
--- a/etc/sddm.conf.d/20-kubuntu.conf
+++ b/etc/sddm.conf.d/20-kubuntu.conf
@@ -1,4 +0,0 @@
-[Autologin]
-Relogin=false
-Session=plasma
-User=
--- a/etc/sddm.conf.d/30-foundry-live.conf
+++ b/etc/sddm.conf.d/30-foundry-live.conf
@@ -0,0 +1,10 @@
+[General]
+DisplayServer=wayland
+
+[Theme]
+Current=foundry-linux
+
+[Autologin]
+User=user
+Session=plasma
+Relogin=false
--- a/etc/sddm.conf
+++ b/etc/sddm.conf
@@ -0,0 +1,4 @@
+[Autologin]
+User=user
+Session=plasma
+Relogin=false
--- a/etc/casper.conf
+++ b/etc/casper.conf
@@ -1,14 +1,4 @@
-# This file should go in /etc/casper.conf
-# Supported variables are:
-# USERNAME, USERFULLNAME, HOST, BUILD_SYSTEM, FLAVOUR
-
-export USERNAME="ubuntu"
-export USERFULLNAME="Live session user"
-export HOST="ubuntu"
+export USERNAME="user"
+export USERFULLNAME="Foundry Linux"
+export HOST="foundry-linux"
 export BUILD_SYSTEM="Ubuntu"
-
-# USERNAME and HOSTNAME as specified above won't be honoured and will be set to
-# flavour string acquired at boot time, unless you set FLAVOUR to any
-# non-empty string.
-
-# export FLAVOUR="Ubuntu"
--- a/etc/environment
+++ b/etc/environment
@@ -1 +1,2 @@
 PATH="/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/usr/games:/usr/local/games:/snap/bin"
+LIBGL_ALWAYS_SOFTWARE=1
--- a/etc/skel/.config/autostart/foundry-wallpaper.desktop
+++ b/etc/skel/.config/autostart/foundry-wallpaper.desktop
@@ -0,0 +1,5 @@
+[Desktop Entry]
+Type=Application
+Name=Set Foundry Linux Wallpaper
+Exec=plasma-apply-wallpaperimage /usr/share/sddm/themes/foundry-linux/background.png
+X-KDE-autostart-after=panel
--- a/var/lib/live/config/sddm
+++ b/var/lib/live/config/sddm
@@ -0,0 +1 @@
+(empty — blocks live-config 0085-sddm from overwriting /etc/sddm.conf)
```

### Graphics files (not diffed — binary/QML assets in `calamares-settings-foundry-linux`)
- `/usr/share/sddm/themes/foundry-linux/Main.qml`
- `/usr/share/sddm/themes/foundry-linux/background.png`
- `/usr/share/sddm/themes/foundry-linux/metadata.desktop`
- `/usr/share/plymouth/themes/foundry-linux/foundry-linux.plymouth` (+ assets)

---

## Problem

Autologin still fails in live session testing. Investigation of the built ISO confirmed:

- `15autologin` patch IS applied correctly in the initramfs
- `casper.conf` is correct (`USERNAME=user`)
- `plasma.desktop` exists in wayland-sessions
- `/etc/sddm.conf` is absent from squashfs (casper creates it fresh at boot)
- `LIBGL_ALWAYS_SOFTWARE=1` is in `/etc/environment`

Remaining gap: `test/boot-smoke.sh` uses `-vga virtio` (plain VirtIO VGA, no DRM device exposed) but `kwin_wayland --drm` (from kubuntu's `10-wayland.conf`) requires a DRM device. Without DRM, kwin_wayland crashes and SDDM falls back to the greeter. Fix: `-device virtio-vga-gl` (VirtGL), which provides DRM via the `virtio_gpu` kernel driver.

Secondary: no SSH access means every debug cycle requires a full rebuild + re-test.

## What Kubuntu does differently

See §15 in the investigation doc for the full comparison table.

Key: Kubuntu ships `kubuntu-installer-prompt` which provides `kubuntu-live-environment.desktop` — casper's first detection target. We don't have this package (and don't want it — it launches the "Try or Install" dialog). Our casper patch handles the fallback to `plasma.desktop` correctly.

## Changes

1. **Fix `test/boot-smoke.sh`** — `-vga virtio` → `-device virtio-vga-gl`; add SSH port-forward `hostfwd=tcp::2222-:22`
2. **New hook `1200-live-ssh.hook.chroot`** — installs openssh-server, enables sshd, configures for live debug use (password auth, root login)
3. **Update investigation doc** — add §15 Kubuntu vs Foundry Linux comparison table; document QEMU fix

## Verification

1. Rebuild `login-test` ISO
2. Boot with corrected QEMU command (smoke test or manual)
3. Confirm autologin → Plasma desktop (no SDDM greeter)
4. SSH in: `ssh -p 2222 user@localhost` (password: `live`) and confirm login

---

## Addendum — 2026-05-24 17:36 — Root cause found and fixed

**Steps 1–3 remained broken through multiple builds until session 5 (2026-05-24).**

### Actual root cause

`live-config`'s `0085-sddm` script runs at every live boot because `grub.cfg` passes `username=user` on the kernel cmdline. `live-config` reads this as `LIVE_USERNAME=user`, sees sddm is installed, and **overwrites `/etc/sddm.conf`** with:

```ini
[Autologin]
User=user
Session=
```

`Session=` is blank because `x-session-manager` on this Wayland-only image doesn't resolve to `startplasma-x11` or `startlxqt` (the only two entries in `0085-sddm`'s hardcoded case statement). Since `/etc/sddm.conf` is SDDM's highest-priority config (loaded after conf.d), this blank `Session=` defeated every fix applied in conf.d or by casper.

This was invisible until session 5 when we:
1. Extracted the squashfs from the ISO without root (`xorriso + unsquashfs`)
2. Confirmed squashfs configs were all correct
3. Spotted `0085-sddm` in `/usr/lib/live/config/`
4. Extracted `grub.cfg` from the ISO and confirmed `username=user` was on the cmdline
5. Traced through `0085-sddm`'s `Cmdline()` → `Init()` → `Config()` to see the overwrite

### Fix

Pre-create `/var/lib/live/config/sddm` in the squashfs. `live-config`'s `component_was_executed("sddm")` sees the state file and exits `Init()` immediately, never calling `Config()`. One `touch`, no overwrite.

Added to `1100-live-autologin.hook.chroot`:
```bash
mkdir -p /var/lib/live/config
touch /var/lib/live/config/sddm
```

Committed: `12ae8ea fix(iso): block live-config 0085-sddm to fix SDDM autologin`

### Verification result

- **Step 3: PASS** — Plasma desktop appears directly after boot; SDDM greeter never shown. Confirmed visually in QEMU 2026-05-24 17:36.
