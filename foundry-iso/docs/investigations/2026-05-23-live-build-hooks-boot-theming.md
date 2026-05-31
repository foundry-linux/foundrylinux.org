# live-build hooks, boot theming, and ISO patching — lessons learned

Date: 2026-05-23 (updated 2026-05-26)

---

## 1. EDITION env var is not passed into the chroot by live-build

**Problem:** Hook `0030-install-foundry-edition.hook.chroot` fails: `EDITION: EDITION env var required`.

**Root cause:** `lb_chroot_hooks` uses `Chroot chroot "/root/<hookname>"` to run each hook. The `Chroot` function does not forward the caller's environment into the chroot. `EDITION` is set in the Docker container but doesn't survive into the chroot.

**What does work:** `lb_chroot_hooks` bind-mounts `config/` read-only at `/root/config/` inside the chroot before running each hook.

**Fix:** Write `EDITION=<value>` to `config/hook-env.sh` in `config/auto/config`. In each hook that needs it, source that file:

```bash
# In config/auto/config (after lb config noauto ...):
printf 'EDITION=%s\n' "$EDITION" > config/hook-env.sh

# In each hook:
source /root/config/hook-env.sh
```

**Pattern:** Any build-time variable that hooks need should be written to `config/hook-env.sh` and sourced this way.

---

## 3. GRUB timeout: binary hooks are too late; use xorriso on the host

**Problem:** A `.hook.binary` script setting `set timeout=5` in grub.cfg had no effect.

**Root cause:** Binary hooks run during `lb binary`, which is after `lb_binary_grub2` has already written and packed `grub.cfg` into the ISO image.

**Fix:** Extract `grub.cfg` with xorriso on the host after `lb binary` completes, patch it with `sed`, and write it back. Use `-boot_image any keep` to preserve existing El Torito EFI boot entries:

```bash
xorriso -osirrox on -dev binary.hybrid.iso -extract /boot/grub/grub.cfg /tmp/grub.cfg
sed -i 's/^set default=0$/set default=0\nset timeout=5/' /tmp/grub.cfg
xorriso -dev binary.hybrid.iso \
  -map /tmp/grub.cfg /boot/grub/grub.cfg \
  -boot_image any keep \
  -commit
```

**Gotcha — single-quote escaping inside `bash -c '...'`:** All `grep`/`sed` patterns inside a single-quoted `bash -c '...'` Docker block must use double quotes — single-quoted patterns silently break the outer string. `bash -n` and `shellcheck` both miss this. Always manually audit quoting before triggering a build.

**Gotcha — two xorriso commits lose EFI boot entries:** The `-boot_image any keep` flag on the second call preserves existing El Torito entries.

---

## 4. Docker creates ISO as root; host xorriso can't write to it

**Problem:** `xorriso -dev binary.hybrid.iso` on the host gets `Permission denied` — the Docker container runs as root, so the ISO lands with `root:root` ownership.

**Fix:** At the end of the Docker `bash -c '...'` block, before it exits:

```bash
chmod a+rw binary.hybrid.iso
```

---

## 5. `calamares-settings-foundry-linux` must be in the package list

**Problem:** The SDDM Foundry Linux theme and Plymouth theme were absent in the `login-test` edition.

**Root cause (layered):**
1. Hook `0030` ran `apt-get install foundry-login-test`, which failed (no such package) — with `set -euo pipefail`, this aborted hook execution entirely.
2. Even after fixing hook 0030, `calamares-settings-foundry-linux` still wasn't installed because the `login-test` package list didn't include it directly.
3. All hooks were in `config/hooks/live/` (wrong directory — see §1).

**Fix:** Explicitly include `calamares-settings-foundry-linux` in the package list; guard hook 0030 with `apt-cache show` before attempting install.

---

## 6. SDDM and Plymouth theme files installed into wrong subdirectory

**Problem:** SDDM showed the kubuntu theme despite `Current=foundry-linux` in conf.d.

**Root cause:** `debian/install` used bare directory names; `dh_install` copies the directory *itself* (including its name) rather than its contents. Files landed at `foundry-linux/sddm/Main.qml` instead of `foundry-linux/Main.qml`.

**Fix:** Use `/*` glob to copy directory contents:
```
data/config/sddm/*      usr/share/sddm/themes/foundry-linux/
data/config/plymouth/*  usr/share/plymouth/themes/foundry-linux/
```

---

## 7. SDDM QML fails silently with Qt5-style versioned imports on Ubuntu 26.04

**Problem:** SDDM finds the theme directory but renders the breeze greeter instead.

**Root cause:** `Main.qml` used Qt5-style versioned imports (`import QtQuick 2.15`, `import SddmComponents 2.0`). Qt6 QML drops version numbers and `SddmComponents` has no `2.0` declaration — the engine silently rejects the theme.

**Fix:** Remove all version pins and remove `import SddmComponents` entirely — `sddm.login()`, `userModel`, and `sessionModel` are SDDM context properties that need no import:
```qml
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
```

**Also:** `sddm.login()` session arg must be an `int`, not `QModelIndex`. Pass `0` directly, not `sessionModel.index(0, 0)`.

---

## 8. SDDM 0.21 requires `Theme-API=2.0` and `QtVersion=6` in metadata.desktop

**Root cause:** SDDM 0.21 (Ubuntu 26.04) checks `metadata.desktop` for both fields before loading a theme's QML. Without them, SDDM renders the built-in greeter entirely.

```ini
# Required in [SddmGreeterTheme]:
Theme-API=2.0
QtVersion=6
```

### Upstream bug report: SDDM silent theme fallback

Both §7 and §8 share the same failure mode: SDDM silently falls back to the built-in greeter with no log output, no console warning, and no indication to the user or developer that anything went wrong. The fallback behaviour itself is correct — a broken theme should not crash the login screen — but the silence makes it nearly impossible to diagnose without reading the SDDM source.

**Proposed report (file against [KDE SDDM](https://bugs.kde.org/enter_bug.cgi?product=sddm)):**

---

**Title:** SDDM silently falls back to built-in greeter on theme load failure — no diagnostic output

**Environment:** SDDM 0.21 / Qt 6 / Ubuntu 26.04 (applies to any Qt6 build)

**Steps to reproduce:**

1. Install an SDDM theme whose `Main.qml` uses Qt5-style versioned imports (`import QtQuick 2.15`, `import SddmComponents 2.0`), or whose `metadata.desktop` is missing `Theme-API=2.0` / `QtVersion=6`.
2. Set `Current=<that-theme>` in `/etc/sddm.conf.d/`.
3. Start SDDM.

**Expected:** SDDM logs a warning at startup — e.g. `sddm: theme "foo" failed to load: QML parse error at Main.qml:3` — then falls back to the built-in greeter.

**Actual:** SDDM silently renders the built-in greeter. No message appears in the journal (`journalctl -u sddm`), on stdout, or in any log file. The configured theme name is still shown in `sddm.conf.d` — there is no indication from SDDM itself that the theme was rejected.

**Impact:** A theme author porting from Qt5 to Qt6 has no way to know their theme was rejected short of reading the SDDM source. Diagnosing the failure required: noticing the wrong greeter was showing, diffing working vs. broken theme QML, and cross-referencing Qt6 release notes about unversioned imports. A single log line would have surfaced the cause immediately.

**Proposed fix:** In the theme-loading code path, emit a `qWarning()` (or equivalent structured log) when:
- The QML engine fails to parse or resolve imports in `Main.qml`
- `metadata.desktop` is missing required fields (`Theme-API`, `QtVersion`)
- Any other condition that causes SDDM to reject the configured theme and fall back

The log should name the theme, the file, and the specific failure reason.

---

---

## 9. SDDM conf.d ordering: use `30-` prefix

**Root cause:** `kubuntu-desktop` installs `20-kubuntu.conf` (sets `Current=kubuntu`, blanks `User=`). SDDM merges conf.d files lexicographically — last writer wins per key. A `10-` prefix loses to `20-kubuntu.conf`.

**Fix:** Name the Foundry live conf `30-foundry-live.conf` so it sorts after both `10-wayland.conf` and `20-kubuntu.conf`.

**Also:** `10-wayland.conf` sets `DisplayServer=wayland`, so SDDM reads sessions from `/usr/share/wayland-sessions/` — session detection must look there first.

---

## 10. UEFI boot for ISOs >4 GiB

SeaBIOS fails with `code 0009` on ISOs >4 GiB (FAT/ISO9660 32-bit LBA limit). Use OVMF for testing:

```bash
qemu-system-x86_64 \
  -enable-kvm -m 4G \
  -drive if=pflash,format=raw,readonly=on,file=/usr/share/OVMF/OVMF_CODE_4M.fd \
  -drive if=pflash,format=raw,file=/tmp/OVMF_VARS_copy.fd \
  -cdrom foundry-login-test-0.9.0-amd64.iso \
  -display gtk,gl=on -device virtio-vga-gl
```

Always make a writable copy of `OVMF_VARS_4M.fd` per run.

live-build 3.0~a57 on Ubuntu 26.04 does not emit EFI boot images from `lb_binary_grub2`. EFI boot must be injected manually via `grub-mkimage` + `mkfs.fat` + `xorriso` after `lb binary`.

### The `/EFI/BOOT will emerge` warning is expected and benign (WONTFIX)

The EFI injection logs:

```
xorriso : WARNING : EFI boot equipment is provided but no directory /EFI/BOOT
xorriso : WARNING : will emerge in the ISO filesystem. A popular method to
xorriso : WARNING : prepare a USB stick on MS-Windows relies on having in the
xorriso : WARNING : ISO filesystem a copy of the EFI System Partition tree.
```

It fires because we attach EFI **only** as the El Torito ESP entry (`-map "$EFI_IMG" /boot/grub/efi.img`, `platform_id=0xef`) — there is no loose `/EFI/BOOT/BOOTX64.EFI` tree in the ISO9660 filesystem. It is a plain libisofs tree-node check, not a boot failure. **Intentionally not fixed:**

- Every USB writer we support — `isoimagewriter` / `usb-creator-kde` (the `create-foundry-usb` path), `dd`, Ventoy, balenaEtcher, Rufus DD-mode — **raw byte-copies the whole `.iso`** and boots via El Torito. They never touch `/EFI/BOOT`, so the warning does not affect them; a multi-GB ISO is no problem.
- The only method the warning targets — extract the ISO and copy its files onto a **FAT32** USB (some Windows tools / Rufus "ISO mode") — is impractical for our editions regardless: the squashfs is >4 GiB and FAT32 caps a single file at 4 GiB. Adding `/EFI/BOOT` would not make that method work.

So the fix would be purely cosmetic (silence the warning + match Ubuntu's hybrid layout) with no working USB path unblocked. **If** a sub-4 GiB edition ever ships and Windows file-copy support is wanted, add the standard loose tree with one line in the EFI-injection xorriso call — the `grub-mkimage` binary already exists at `$EFI_WORK/EFI/BOOT/BOOTX64.EFI`, El Torito path unchanged:

```
-map "$EFI_WORK/EFI/BOOT/BOOTX64.EFI" /EFI/BOOT/BOOTX64.EFI \
```

Until then, ignore it — don't re-flag in audits.

---

## 11. Plymouth theme activation

`update-alternatives --install` and `update-alternatives --set` must both be called to activate a Plymouth theme. Hook 1100 then calls `update-initramfs -u` unconditionally (also required to bundle `casper.conf` — see §12).

---

## 12. Live session framework: use `casper`, not `live-config`

### Background: two separate Ubuntu live frameworks

Ubuntu's live ISO ecosystem has two competing implementations:

| Package | Origin | Mechanism | User creation timing |
|---|---|---|---|
| `live-config` + `live-config-systemd` | Debian Live | systemd service at runtime | After systemd starts |
| `casper` | Ubuntu / Canonical | initramfs scripts before systemd | Before systemd starts |

**Kubuntu, Ubuntu, Ubuntu Studio, and Lubuntu all ship `casper`.** live-build `--mode ubuntu` does not automatically choose one — if neither is in the package list, the live user is never created.

We started with `live-config` because it was what live-build's own documentation referenced. That was the wrong choice for an Ubuntu base.

### How casper works

Casper ships initramfs scripts in `/usr/share/initramfs-tools/scripts/casper-bottom/`. These run **before systemd PID 1 starts**, operating on the squashfs root mounted at `/root`:

- `casper-bottom/25adduser` — calls `chroot /root /usr/lib/user-setup/user-setup-apply` to create the live user (`$USERNAME`) with home directory and sudoers entry.
- `casper-bottom/15autologin` — detects the display manager (SDDM, GDM, LightDM) and appends `[Autologin]` to `/root/etc/sddm.conf`. For KDE Plasma, detects `plasma.desktop` in wayland-sessions.

Because both run in the initramfs, **the live user exists and SDDM's autologin config is written before `display-manager.service` starts**. No race condition.

### How casper.conf gets into the initramfs

Casper's initramfs hook (`/usr/share/initramfs-tools/hooks/casper`) copies `/etc/casper.conf` into the initrd at `update-initramfs` time. Casper's own postinst runs `update-initramfs` before our hooks do, so hook 1100 writes `casper.conf` first, then unconditionally re-runs `update-initramfs -u`:

```bash
cat > /etc/casper.conf <<'EOF'
export USERNAME="user"
export USERFULLNAME="Foundry Linux"
export HOST="foundry-linux"
export BUILD_SYSTEM="Ubuntu"
EOF
# ... Plymouth setup ...
update-initramfs -u   # re-runs so casper's hook bundles our casper.conf
```

### SDDM config priority: `/etc/sddm.conf` beats conf.d

SDDM loads config files in this order (lowest → highest priority), overwriting per-key:

1. `/usr/lib/sddm/sddm.conf.d/*.conf` (system defaults)
2. `/etc/sddm.conf.d/*.conf` (alphabetical — `30-` beats `20-`)
3. `/etc/sddm.conf` ← **highest priority, processed last**

Casper's `15autologin` appends to `/etc/sddm.conf` at initramfs time. Whatever it writes there wins over every conf.d file, including `30-foundry-live.conf`. A blank `Session=` from casper therefore silently overrides our `Session=plasma` in conf.d.

### Casper session detection fails → `Session=` blank → autologin aborted

Casper's `15autologin` sets `$sddm_session` by checking for a fixed list of session `.desktop` files:

```sh
if   [ -f /root/usr/share/wayland-sessions/kubuntu-live-environment.desktop ]; then
    sddm_session=kubuntu-live-environment.desktop   # Kubuntu
elif [ -f /root/usr/share/wayland-sessions/plasma.desktop ]; then
    sddm_session=plasma.desktop                     # Ubuntu Studio
elif ...
fi
cat >>/root/etc/sddm.conf <<EOF
[Autologin]
User=$USERNAME
Session=$sddm_session
EOF
```

Two bugs:
1. If none of the known files are found, `$sddm_session` is **unset** → writes `Session=` (blank) → SDDM logs `Unable to find autologin session entry ""` and skips autologin.
2. When `plasma.desktop` **is** found, the session is written as `plasma.desktop` (with extension). SDDM looks up sessions by bare name (`plasma`), so `plasma.desktop` never matches.

Kubuntu avoids both bugs by shipping `kubuntu-live-environment.desktop` — the first file casper checks — as a specially crafted live session. We use `kubuntu-desktop` but not the kubuntu live-environment packages, so that file is absent.

**Fix (in hook 1100, before `update-initramfs -u`):** Patch casper's `15autologin` to strip the `.desktop` suffix and default to `plasma` when detection falls through. The patched script is then bundled into the initramfs by `update-initramfs -u`:

```python
# Strip .desktop suffix; SDDM expects bare session names.
# Default to "plasma" when the file-existence checks all miss.
sddm_session_name="${sddm_session%.desktop}"
sddm_session_name="${sddm_session_name:-plasma}"
```

### Casper LIVE_MEDIA_PATH mismatch: `live-media-path=live` required

**Symptom:** Boot halts with "Unable to find a medium containing a live file system."

**Root cause:** Casper's initramfs script hardcodes `LIVE_MEDIA_PATH=casper` — it looks for `*.squashfs` in `/casper/` on the ISO. live-build places the squashfs at `/live/filesystem.squashfs`.

**Fix:** Add `live-media-path=live` to `--bootappend-live` in `config/auto/config`:
```
--bootappend-live "boot=casper live-media-path=live quiet splash"
```

### What changed vs live-config

| Item | live-config (old) | casper (current) |
|---|---|---|
| Package list | `live-config`, `live-config-systemd`, `user-setup` | `casper`, `user-setup` |
| Boot cmdline | `boot=live components quiet splash` | `boot=casper live-media-path=live quiet splash` |
| User creation | `live-config.service` at systemd runtime | initramfs `25adduser`, before systemd |
| SDDM autologin | written to conf.d by hook 1100 | written to `sddm.conf` by `15autologin`; conf.d overlays `[General]` + `[Theme]` only |
| systemd ordering | `After=live-config.service` drop-in | not needed |
| PAM patches | `pam_loginuid.so`, `pam_nologin.so` → optional | not needed |
| `USERFULLNAME` | kernel cmdline `live-config.user-fullname=` | `/etc/casper.conf` → initramfs |

---

## 13. `kwin_wayland` crashes with `virtio-vga` in QEMU → use VirtGL

**Root cause:** `kwin_wayland` requires DRM/KMS. Plain `virtio-vga` provides no DRM device — the compositor crashes immediately on the autologin path and SDDM shows a blank greeter.

**Fix for QEMU testing:** Use VirtGL (`-device virtio-vga-gl -display gtk,gl=on`) which provides DRM via the `virtio-gpu` kernel driver. Also set `LIBGL_ALWAYS_SOFTWARE=1` in `/etc/environment` (written by hook 1100) so `kwin_wayland` uses llvmpipe instead of trying to acquire a hardware GPU. This has no effect on real hardware where the GPU driver is loaded and preferred.

```bash
qemu-system-x86_64 \
  -enable-kvm -m 4G \
  -drive if=pflash,format=raw,readonly=on,file=/usr/share/OVMF/OVMF_CODE_4M.fd \
  -drive if=pflash,format=raw,file=/tmp/OVMF_VARS_copy.fd \
  -cdrom foundry-login-test-0.9.0-amd64.iso \
  -display gtk,gl=on \
  -device virtio-vga-gl
```

---

## 14. Plasma desktop wallpaper via `/etc/skel/` — pre-seeded before first login

**Mechanism:** Casper's `25adduser` calls `user-setup-apply`, which creates the live user via `useradd -m`. The `-m` flag copies `/etc/skel/` into `/home/user/` before Plasma starts its first session.

**Fix:** Hook 1100 writes a `plasma-apply-wallpaperimage` autostart entry into `/etc/skel/`:

```bash
if [[ -f /usr/share/sddm/themes/foundry-linux/background.png ]]; then
  mkdir -p /etc/skel/.config/autostart
  cat > /etc/skel/.config/autostart/foundry-wallpaper.desktop <<'EOF'
[Desktop Entry]
Type=Application
Name=Set Foundry Linux Wallpaper
Exec=plasma-apply-wallpaperimage /usr/share/sddm/themes/foundry-linux/background.png
X-KDE-autostart-after=panel
EOF
fi
```

`plasma-apply-wallpaperimage` is preferred over static `appletsrc` because Plasma 6 assigns containment IDs dynamically on first run — a static config targeting containment `[1]` fails if the ID differs.

---

## 15. Kubuntu 26.04 reference: what they do vs. our current state

**Does Kubuntu produce a live ISO?** Yes — every Kubuntu release ships as a live ISO. It boots into a live KDE Plasma session backed by casper, with Calamares available for installation. There is no separate "server install" ISO for Kubuntu.

### Package that unlocks casper autologin: `kubuntu-installer-prompt`

Casper's `15autologin` (v 25.10.2) checks session `.desktop` files in this order:

1. `/root/usr/share/wayland-sessions/kubuntu-live-environment.desktop` — provided by **`kubuntu-installer-prompt`** (Kubuntu only)
2. `/root/usr/share/wayland-sessions/plasma.desktop` — provided by `plasma-workspace` (ships with kubuntu-desktop)
3. `/root/usr/share/xsessions/lubuntu-live-environment.desktop` — Lubuntu
4. `/root/usr/share/wayland-sessions/budgie-desktop-live.desktop` — Ubuntu Budgie

Kubuntu hits case 1 because they install `kubuntu-installer-prompt`. We hit case 2 because we install `kubuntu-desktop` (which pulls `plasma-workspace`) but not `kubuntu-installer-prompt`.

The `kubuntu-installer-prompt` package also launches the "Try or Install Kubuntu" dialog in the live session. We deliberately exclude it — Foundry Linux autologins straight to Plasma.

### `.desktop` suffix bug (casper + SDDM mismatch)

Whichever branch matches, casper writes the value WITH the `.desktop` extension (e.g. `Session=plasma.desktop`). SDDM 0.21 looks up sessions by bare name (`plasma`), so `plasma.desktop` never matches → autologin aborted.

Kubuntu is insulated from this bug because SDDM finds a session named `kubuntu-live-environment` (the bare name of `kubuntu-live-environment.desktop`) and that session file exists — but casper still writes the extension and SDDM must strip it internally, or the Kubuntu-specific session is matched by filename.

Our fix: patch `15autologin` before `update-initramfs -u` in hook 1100 to strip the `.desktop` suffix and default to `plasma`. **Confirmed present in the built initramfs** (2026-05-24 extraction from `foundry-login-test-0.9.0-amd64.iso`).

### Full comparison table

| Item | Kubuntu 26.04 | Foundry Linux (current) | Status |
|---|---|---|---|
| Live session package | `kubuntu-installer-prompt` → `kubuntu-live-environment.desktop` | (absent — not wanted) | ✓ patch handles fallback |
| casper session detection | Hits branch 1 (kubuntu-live-environment.desktop) | Hits branch 2 (plasma.desktop) | ✓ |
| `.desktop` suffix in `Session=` | `Session=kubuntu-live-environment.desktop` written | `Session=plasma.desktop` written | ✓ patched to strip |
| Patched 15autologin | No patch needed (package provides the file) | Patched in hook 1100 | ✓ in initramfs |
| casper.conf `USERNAME=user` | Yes | Yes | ✓ |
| Live user creation | casper `25adduser` (initramfs, before systemd) | casper `25adduser` (initramfs, before systemd) | ✓ |
| SDDM `[Autologin]` in sddm.conf | Written by casper `15autologin` at boot | Written by casper `15autologin` at boot | ✓ |
| SDDM conf.d theme override | 20-kubuntu.conf (kubuntu theme) | 30-foundry-live.conf (foundry-linux theme) | ✓ 30- > 20- |
| `LIBGL_ALWAYS_SOFTWARE=1` | Not set (hardware GPU assumed) | Set in `/etc/environment` | ✓ |
| QEMU test device | Physical HW / VirtGL `-device virtio-vga-gl` | **`-vga virtio` (WRONG — no DRM)** | **⚠ fixed 2026-05-24** |
| SSH access for debugging | N/A (test on real HW) | **`openssh-server` (added 2026-05-24)** | ✓ new hook 1200 |

### Root cause of persistent autologin failure in smoke tests

`test/boot-smoke.sh` used `-vga virtio` (legacy VirtIO VGA). This device does **not** expose a DRM device to the guest. `kwin_wayland --drm` (from kubuntu's `10-wayland.conf`) requires DRM — without it, kwin_wayland crashes immediately and SDDM falls back to the greeter, making it appear as if autologin config is wrong.

**Fix:** use `-device virtio-vga-gl -display gtk,gl=on` (VirtGL). The `virtio-gpu` kernel driver then exposes `/dev/dri/card0` and kwin_wayland can start. `LIBGL_ALWAYS_SOFTWARE=1` ensures llvmpipe is used for GL rendering (no host GPU needed).

Also added `hostfwd=tcp::2222-:22` to the QEMU netdev so SSH is available for live debugging without a full rebuild cycle:

```bash
ssh -p 2222 user@localhost   # password: live
ssh -p 2222 root@localhost   # password: foundry
```

---

## 16. plasma-welcome: purge it rather than suppressing via kded6rc

**Problem:** plasma-welcome launched at first boot despite two layers of kded6rc suppression:
- `/etc/xdg/kded6rc` → `[Module-plasma_welcome] autoload=false` (system-wide, hook 0020)
- `/etc/skel/.config/kded6rc` → `[Module-plasma_welcome] loaded=false` (user skel, hook 1100)

**Root cause (two compounding mistakes):**

1. **Wrong key in user skel.** `loaded` tracks whether a kded6 module is currently active in a session; `autoload` is what kded6 reads at startup to decide whether to load the module. Setting `loaded=false` has no suppression effect.

2. **System-level kded6rc not consulted for module autoload.** kded6 reads the `autoload` key from the user-level `~/.config/kded6rc`. When `autoload` is absent from the user config, kded6 falls back to the value embedded in the plugin's metadata (`X-KDE-Kded-autoload=true`), not to `/etc/xdg/kded6rc`. The system config is therefore ineffective for module suppression.

**Root cause of the original wrong comment:** Hook 0020 originally said "purging plasma-welcome fails because kubuntu-desktop depends on it." This was incorrect — `apt-cache show kubuntu-desktop` and `apt-cache show kubuntu-settings-desktop` both list `plasma-welcome` under `Recommends`, not `Depends`. Purging it is clean.

**Fix (2026-05-25):** Purge `plasma-welcome` in hook 0020 instead of suppressing via config. Removed all kded6rc suppression and QML page removal from both hooks — none of it is needed once the package is gone.

```bash
# In hook 0020:
apt-get purge -y plasma-welcome >/dev/null 2>&1 || true
```

---

## 17. Live session NIC stays DOWN — NM ignores keyfile without uuid

**Problem:** In QEMU (and on real hardware with non-eth0 NIC names like `ens2`, `enp3s0`), the Ethernet interface stays DOWN even after hook 1200 creates `/etc/NetworkManager/system-connections/live-ethernet.nmconnection`. `ssh -p 2222 root@localhost` is unreachable.

**Root cause:** NetworkManager 1.48+ (Ubuntu 26.04) silently ignores keyfile connection profiles that are missing the `uuid` field. The file is present, permissions are correct, but NM never loads the profile at boot.

**Symptoms to distinguish this from other NM issues:**
- `ip link show` shows the interface (e.g., `ens2`) in `DOWN` state with no IP address.
- `nmcli connection show` does not list the `Live-Ethernet` connection at all.
- `journalctl -u NetworkManager | grep keyfile` shows a warning like `plugin keyfile: ignored: missing uuid`.

**Fix:** Add `uuid` to the `[connection]` stanza in the nmconnection file:

```ini
[connection]
id=Live-Ethernet
uuid=4a6b8c2d-1e3f-4a5b-9c0d-7e8f1a2b3c4d
type=ethernet
autoconnect=true
autoconnect-priority=-100
```

Any valid RFC 4122 UUID works; a static value is fine for a live-session-only profile.

**QEMU SSH workflow:** Launch QEMU with port forwarding, then SSH with password auth only:

```bash
# Copy OVMF vars and launch
cp /usr/share/OVMF/OVMF_VARS_4M.fd /tmp/OVMF_VARS_copy.fd
qemu-system-x86_64 \
  -enable-kvm -m 4G \
  -drive if=pflash,format=raw,readonly=on,file=/usr/share/OVMF/OVMF_CODE_4M.fd \
  -drive if=pflash,format=raw,file=/tmp/OVMF_VARS_copy.fd \
  -cdrom foundry-iso/dist/foundry-anvil-<version>-amd64.iso \
  -display gtk,gl=on \
  -device virtio-vga-gl \
  -netdev user,id=n0,hostfwd=tcp::2222-:22 \
  -device e1000,netdev=n0

# SSH (disable pubkey auth to avoid "Too many authentication failures"):
ssh -p 2222 -o IdentitiesOnly=yes -o PubkeyAuthentication=no root@localhost   # password: foundry
ssh -p 2222 -o IdentitiesOnly=yes -o PubkeyAuthentication=no user@localhost    # password: live

# Or use sshpass:
sshpass -p 'foundry' ssh -o IdentitiesOnly=yes -o PubkeyAuthentication=no -p 2222 root@localhost
```

**Real hardware SSH (live machine on LAN):**
```bash
sshpass -p 'foundry' ssh -o IdentitiesOnly=yes -o PubkeyAuthentication=no root@<ip>
```

**Artifact verification before QEMU test:** Always confirm the squashfs contains the expected versions using `unsquashfs` before testing:

```bash
# Mount ISO and check dpkg status + NM connection file
sudo mount -o loop,ro foundry-anvil-X.Y.Z-amd64.iso /mnt/iso
sudo unsquashfs -d /tmp/sq /mnt/iso/live/filesystem.squashfs \
  var/lib/dpkg/status \
  etc/NetworkManager/system-connections
grep -A3 "Package: calamares-settings-foundry-linux" /tmp/sq/var/lib/dpkg/status
grep -A3 "Package: foundry-welcome" /tmp/sq/var/lib/dpkg/status
cat /tmp/sq/etc/NetworkManager/system-connections/live-ethernet.nmconnection
```

## 18. Build aborts at the host-side key fetch on a transient DNS failure

**Symptom.** `task iso-build` dies immediately, before the container even starts:

```
=== Fetching apt signing keys ===
curl: (6) Could not resolve host: apt.foundrylinux.org
gpg: no valid OpenPGP data found.
task: Failed to run task "iso-build": exit status 2
```

**Root cause — a transient name-resolution failure, not a build bug or a dead
repo.** `build-iso.sh` fetches the four apt signing keys with `curl` on the
*host* (before the `ubuntu:26.04` container). `getaddrinfo()` timing out surfaces
to curl as error 6 ("couldn't resolve host"); the empty pipe then makes
`gpg --dearmor` print "no valid OpenPGP data", and `set -euo pipefail` aborts the
whole build. The domain resolved and served HTTP 200 on recheck, so it was a
one-off — and `resolvectl statistics` on the build host showed **886 query
timeouts over a week of uptime** (~5/hr), i.e. this resolver does intermittently
time out and the build's first network call unluckily landed on one. (On the
affected host, public DNS happened to be routed through a VPN tunnel's resolver —
the most timeout-prone path — but the lesson is general.)

**Fix.** Wrap the fetches in a retrying helper:

```sh
fetch_key() {  # <url> <dest-keyfile>
  curl -fsSL --retry 5 --retry-delay 2 --retry-all-errors --retry-connrefused "$1" \
    | gpg --dearmor > "$2"
  [[ -s "$2" ]] || { echo "ERROR: fetched empty/invalid signing key from $1" >&2; exit 1; }
}
```

**Gotcha:** plain `curl --retry N` does **not** retry name-resolution failures
(error 6) — you must add `--retry-all-errors`. The `[[ -s ]]` assert also catches
a 200-but-garbage response writing an empty key file.

**Pattern:** every network fetch in a multi-minute build (signing keys here; also
consider `apt-get` inside the container via `Acquire::Retries`) should be
retry-wrapped — a one-second resolver blip must never nuke a 20-plus-minute build.

---

<details>
<summary><strong>Historical: live-config dead ends (superseded by §12)</strong></summary>

These sections document the investigation path taken while using `live-config` before the root cause was identified. The fixes described here are all removed from the current codebase.

---

### H0. Hook directory: `config/hooks/` not `config/hooks/live/`

We put hooks in `config/hooks/live/` because that's what live-config documentation and community examples showed. They silently never ran.

**Root cause:** `lb_chroot_hooks` only searches `config/hooks/*.chroot` — no subdirectory recursion. The `config/hooks/live/` and `config/hooks/normal/` directories are ignored by live-build 3.0~a57 on Ubuntu 26.04.

```sh
# From /usr/lib/live/build/lb_chroot_hooks:
for _HOOK in config/hooks/*.chroot
do ...
done
```

**Fix:** All chroot hooks must live directly in `config/hooks/*.hook.chroot`. Read the actual live-build scripts in `/usr/lib/live/build/` rather than trusting docs that may describe older versions.

---

### H1. live-config.user-fullname via kernel cmdline

`live-config` supports `LIVE_USER_FULLNAME` via `/etc/live/config.conf.d/user-setup.conf` or the kernel cmdline `live-config.user-fullname="Foundry Linux"`. The kernel cmdline approach is more reliable since it doesn't depend on the hook chain. Both were set; neither is used now (replaced by `USERFULLNAME` in `casper.conf`).

---

### H2. `display-manager.service` ordering: `After=live-config.service`

`display-manager.service` started before `live-config.service` finished creating the `user` account — SDDM autologin failed because the user didn't exist in NSS/PAM yet.

Attempted fix — a systemd drop-in from hook 1100:
```bash
mkdir -p /etc/systemd/system/display-manager.service.d
cat > /etc/systemd/system/display-manager.service.d/after-live-config.conf <<'EOF'
[Unit]
After=live-config.service
Wants=live-config.service
EOF
```

This was fragile: `live-config.service` is `Type=oneshot` and exits when its script exits, but the NSS/PAM state may not be consistent by the time SDDM reads it. Eliminated entirely by switching to casper, which creates the user in the initramfs before systemd starts.

---

### H3. SDDM autologin config written to conf.d (live-config era)

During the live-config era, hook 1100 wrote a full `[Autologin]` stanza to `/etc/sddm.conf.d/30-foundry-live-autologin.conf`:

```ini
[General]
DisplayServer=wayland

[Theme]
Current=foundry-linux

[Autologin]
User=user
Session=plasma
Relogin=false
```

With casper, `[Autologin]` is written to `/etc/sddm.conf` by `casper-bottom/15autologin` in the initramfs. Our conf.d now only carries `[General]` and `[Theme]`, renamed to `30-foundry-live.conf`.

---

### H4. `live-config` + `user-setup` both required; `user-setup` was missing

`live-config`'s `0030-user-setup` component guards: `pkg_is_installed "user-setup" || exit 0` — if `user-setup` is absent, `user-setup-apply` is never called and the `user` account is never created.

**Default password:** `live-config` 11.0.5 sets the live user password to `live` (hash `8Ab05sVQ4LLps`), not empty.

---

### H5. `live-config` missing entirely → user account never created

`live-build --mode ubuntu` does NOT automatically inject `live-config` into the squashfs when using a custom package list. Without it, `live-config.service` never runs, and the live `user` account is never created — both autologin and manual login fail.

**Symptom signature:** SDDM greeter appears despite `[Autologin]` config; `user` + empty password rejected; no `user` entry in `/etc/passwd` at runtime.

---

### H6. Session mismatch: `Session=openbox` vs `DisplayServer=wayland`

Hook 1100 had a login-test special case scanning only `/usr/share/xsessions/*.desktop`. Alphabetical ordering yielded `openbox`. But `10-wayland.conf` sets `DisplayServer=wayland`, so SDDM looks in `/usr/share/wayland-sessions/` — `openbox` has no wayland session entry, autologin silently fails.

Fix at the time: scan wayland-sessions first. With casper, session detection is handled by `casper-bottom/15autologin` which correctly finds `plasma.desktop` in wayland-sessions.

---

### H7. `pam_nologin.so requisite` in sddm-autologin blocks autologin

`/etc/pam.d/sddm-autologin` has `auth requisite pam_nologin.so`. If `/etc/nologin` exists transiently during early systemd startup, the autologin auth chain is aborted. Manual login with `user`/`live` succeeded; autologin fell back to greeter.

Attempted fix — sed patch in hook 1100:
```bash
sed -i 's/^auth\s\+requisite\s\+pam_nologin\.so/auth optional pam_nologin.so/' \
  /etc/pam.d/sddm-autologin
```

Removed: Kubuntu live doesn't patch PAM, and casper's initramfs user-creation approach doesn't trigger this window.

---

### H8. casper `15autologin` appends `/root/etc/sddm.conf` with empty Session= (root cause, 2026-05-24)

**Confirmed via SDDM journal** (dumped from live VM via serial port):
```
Unable to find autologin session entry ""
```

SDDM's `[Autologin] Session` was empty string at boot time.

**Root cause chain:**

1. `casper-bottom/15autologin` runs in the initramfs and writes `[Autologin]` to `/root/etc/sddm.conf` via `cat >>` (append).
2. `/etc/sddm.conf` is the **highest-priority** SDDM config — it overrides all `/etc/sddm.conf.d/` entries including our `30-foundry-live.conf`.
3. Our hook `1100-live-autologin` already patched `15autologin` to fix two bugs (Session name with `.desktop` extension; empty Session fallback). The patch IS applied in the initramfs and IS correctly generating `Session=plasma` per static analysis.
4. Despite the patch, SDDM still saw `Session=""`. The exact path in 15autologin that produces the empty value is unclear — `/usr/share/wayland-sessions/plasma.desktop` exists in the squashfs and the `${sddm_session_name:-plasma}` default should fire. Unable to trace further without live debugging of initramfs execution.

**Key findings from squashfs inspection:**
- No `/etc/sddm.conf` baked into squashfs — casper creates it at boot (correct).
- `/etc/sddm.conf.d/30-foundry-live.conf`: `Session=plasma User=user` ✓
- `/etc/sddm.conf.d/20-kubuntu.conf`: `Session=plasma User=` (User blank, overridden by 30-)
- `/usr/share/wayland-sessions/plasma.desktop` exists in squashfs ✓
- `casper-bottom/15autologin` in built initramfs: patch confirmed present ✓
- `casper-bottom/ORDER` confirms 15autologin runs before 25adduser

**Fix (first attempt):** Added `casper-bottom/16foundry-autologin` — a properly-structured casper-bottom script numbered to run immediately after `15autologin` in the ORDER. It **overwrites** (not appends) `/root/etc/sddm.conf` with definitive values:
```sh
printf '[Autologin]\nUser=%s\nSession=plasma\nRelogin=false\n' "$USERNAME" \
    > /root/etc/sddm.conf
```
This wins regardless of what 15autologin wrote because it runs after in the same initramfs chain.

---

### H9. Two writes to sddm.conf both produce 31 bytes — 16foundry-autologin not firing (2026-05-24)

**Observation:** After booting the ISO with `16foundry-autologin` in place, `stat /etc/sddm.conf` in the live VM shows:

```
Size=31        Birth: 2026-05-24 07:20:27  Modify: 2026-05-24 07:20:44
```

31 bytes = exactly `[Autologin]\nUser=user\nSession=\n` — the unpatched `15autologin` output with empty Session.

**Key deduction:** Birth ≠ Modify (17 second gap) → two separate writes happened. Both produced 31 bytes. `16foundry-autologin`'s `printf` should produce ~51 bytes (includes `Session=plasma\nRelogin=false\n`). Therefore **`16foundry-autologin` did not run** — the second write came from elsewhere.

**Most likely cause:** The `if [ -f /root/usr/bin/sddm ]` guard in the original `16foundry-autologin` was evaluating to false in the initramfs environment, or `15autologin` is running a second time through a path not visible in the ORDER file. The second 31-byte write remains unattributed.

**Also confirmed:**
- `plasma.desktop` IS present in `/usr/share/wayland-sessions/` in the live session — so 15autologin's detection path should have worked. The patched `${sddm_session_name:-plasma}` default fires correctly per static analysis, but actual initramfs execution is not producing the patched output.
- `/etc/sddm.conf.d/30-foundry-live.conf` is present with `Session=plasma` but is overridden by `/etc/sddm.conf` (conf.d loses to the top-level file).
- No `/etc/sddm.conf` baked in squashfs (confirmed via `unsquashfs -l`).

**Fix (second attempt — 2026-05-24):**

Two-pronged approach in `1100-live-autologin.hook.chroot`:

1. **Bake `/etc/sddm.conf` into the squashfs** with correct `Session=plasma`. This is the highest-priority SDDM config. `casper/15autologin` will overwrite it with `cat >>` (append), but `16foundry-autologin` then corrects it. The squashfs copy handles edge cases where casper-bottom doesn't run at all.

2. **Rewrite `16foundry-autologin` to use `sed -i`** instead of `printf >` (overwrite), and **remove the `if [ -f /root/usr/bin/sddm ]` guard:**
   ```sh
   if [ -f /root/etc/sddm.conf ]; then
       sed -i -e 's/^Session=$/Session=plasma/' \
              -e 's/^Session=plasma\.desktop$/Session=plasma/' \
              /root/etc/sddm.conf
   else
       printf '[Autologin]\nUser=%s\nSession=plasma\nRelogin=false\n' \
           "${USERNAME:-user}" > /root/etc/sddm.conf
   fi
   ```
   The `sed` is surgical — if `/etc/sddm.conf` already has `Session=plasma`, it's a no-op. Handles both the empty-Session case and the `.desktop`-suffix case.

</details>

---

## 18. live-build sanitises `system-connections/` — write NM config from initramfs instead

**Problem:** Hook 1200 creates `/etc/NetworkManager/system-connections/live-ethernet.nmconnection` in the chroot (with uuid, type=ethernet, method=auto) but the file is missing from the 0.9.19 squashfs. The live session has no automatic Ethernet — the NIC stays DOWN.

**Evidence:** Build log line 8923 shows `ls: cannot access '/etc/NetworkManager/system-connections': No such file or directory` during the package installation phase (before hooks run). Lines 10745-10749 show NM's postinst creating its service symlinks and `WARNING: NetworkManager could not reload connections`. After hook 1200 completes (confirmed by ssh.service symlinks being created), the file is absent in the squashfs (verified via Docker + unsquashfs inspection).

**Root cause:** live-build's network-sanitisation step (part of `lb_chroot_hacks` or a system hook that runs AFTER all `.hook.chroot` files) removes the contents of `/etc/NetworkManager/system-connections/` before compressing the squashfs. This is intentional — live-build doesn't want build-machine network credentials leaking into the live image. Our NM profile gets caught in this sweep even though it's not a build-machine credential.

**Fix (0.9.20):** Write the NM connection profile from the initramfs via a `casper-bottom/17foundry-nm` script, added in hook 1100 before `update-initramfs -u`. casper-bottom scripts run after the overlayfs is mounted (so writes go to the tmpfs writable layer), and after the squashfs is already read-only — build-time cleanup can never remove them.

```sh
# In hook 1100, before update-initramfs -u:
cat > /usr/share/initramfs-tools/scripts/casper-bottom/17foundry-nm << 'EOF'
#!/bin/sh
# ... standard casper-bottom boilerplate ...
mkdir -p /root/etc/NetworkManager/system-connections
cat > /root/etc/NetworkManager/system-connections/live-ethernet.nmconnection << 'NMEOF'
[connection]
id=Live-Ethernet
uuid=4a6b8c2d-1e3f-4a5b-9c0d-7e8f1a2b3c4d
type=ethernet
autoconnect=true
autoconnect-priority=-100
[ethernet]
[ipv4]
method=auto
[ipv6]
method=auto
addr-gen-mode=stable-privacy
NMEOF
chmod 600 /root/etc/NetworkManager/system-connections/live-ethernet.nmconnection
EOF
chmod +x /usr/share/initramfs-tools/scripts/casper-bottom/17foundry-nm
```

**Pattern:** Any file that live-build might sanitise from the chroot (network config, machine-specific files) should be written via a `casper-bottom` initramfs script rather than a `.hook.chroot` file. `.hook.chroot` files are correct for persistent distro config; live-session transient config belongs in casper-bottom.
