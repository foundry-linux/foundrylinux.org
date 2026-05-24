# ISO autologin debug: QEMU fix + live sshd

Date: 2026-05-24

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
