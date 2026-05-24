# ISO autologin debug: QEMU fix + live sshd

Date: 2026-05-24

## Changes from base Kubuntu

Everything below is delta on top of `kubuntu-desktop` in the `live-build` chroot. Graphics files are referenced by path only.

### Packages added (`foundry.list.chroot`)
| Package | Purpose |
|---|---|
| `openbox` | Fallback window manager for installer-only mode |
| `calamares` | Graphical installer |
| `calamares-settings-foundry-linux` | Branding: SDDM theme, Plymouth theme, wallpaper, Calamares config |
| `casper` | Ubuntu live-session framework (creates live user, writes SDDM autologin) |
| `user-setup` | Casper dependency for user account creation |

### Packages removed (`strip.list.chroot.purge`)
`libreoffice-*`, Kontact/KMail/Akonadi suite, KDE PIM, DigiKam, KDE games, KDEConnect, KRDC/KRFB, KTorrent, snapd/snap-store, usb-creator-kde, system-config-printer-kde

### SDDM config delta (`1100-live-autologin.hook.chroot`)
| File | Action | Why |
|---|---|---|
| `/etc/sddm.conf.d/20-kubuntu.conf` | **Removed** | Sets `[Autologin] User=` blank, disables autologin |
| `/etc/sddm.conf.d/30-foundry-live.conf` | **Added** | `DisplayServer=wayland`, Foundry theme, `User=user Session=plasma` |
| `/etc/sddm.conf` | **Baked** | Highest-priority fallback with correct autologin if casper scripts fail |
| `/var/lib/live/config/sddm` | **Pre-created (empty)** | Blocks `live-config`'s `0085-sddm` from overwriting sddm.conf with `Session=` blank |

### Casper initramfs patches (`1100-live-autologin.hook.chroot`)
| Script | Change |
|---|---|
| `casper-bottom/15autologin` | Python patch: strip `.desktop` suffix from session name; default to `plasma` |
| `casper-bottom/16foundry-autologin` | **New**: sed-fixes `Session=` and `Session=plasma.desktop` after 15autologin runs |

### casper.conf
`USERNAME=user`, `USERFULLNAME="Foundry Linux"`, `HOST=foundry-linux`, `BUILD_SYSTEM=Ubuntu`

### Environment
`LIBGL_ALWAYS_SOFTWARE=1` appended to `/etc/environment` — forces llvmpipe in QEMU; ignored on real hardware with GPU driver loaded.

### Wallpaper autostart (`/etc/skel/.config/autostart/foundry-wallpaper.desktop`)
`plasma-apply-wallpaperimage /usr/share/sddm/themes/foundry-linux/background.png` — runs at session start to set the desktop wallpaper. (Image: [`calamares-settings-foundry-linux/.../background.png`](../../foundry-apt/packages/calamares-settings-foundry-linux/))

### Plymouth theme (`1050-plymouth-theme.hook.chroot`, `1100-live-autologin.hook.chroot`)
`update-alternatives` sets `/usr/share/plymouth/themes/foundry-linux/foundry-linux.plymouth` as the default Plymouth theme. (Assets in `calamares-settings-foundry-linux` package.)

### SDDM theme
`/usr/share/sddm/themes/foundry-linux/` — `Main.qml`, `background.png`, `metadata.desktop`. (Shipped by `calamares-settings-foundry-linux`.)

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
