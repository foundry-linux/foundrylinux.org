# Live-build hooks, apt, and boot/theming investigation notes

Living reference — append new sections as issues are diagnosed/fixed.

---

## live-build 3.0~a57 preferences.d save/restore cycle (2026-06-04)

`lb_chroot_archives chroot install` (runs before package installation) backs up
**all** files from `chroot/etc/apt/preferences.d/` to a `.save/` directory and
then **removes** them. After ALL hooks finish, live-build restores the backed-up
files. This means:

- `config/apt/preferences.d/*.pref` files are **not active** during hook execution.
  They apply only to the *delivered* live system's apt.
- During hooks, apt sees NO pins unless a hook writes them itself.
- Fix for Mozilla firefox priority: hook 0025 cannot rely on the 1001-priority
  `mozilla-firefox.pref` being present. It must install firefox directly.

---

## gpgv fails in chroot's apt during hook execution (2026-06-04)

**Symptom:** Every `apt-get update` or `apt-get install` inside a hook fails:
```
Could not execute 'gpgv' to verify signature (is gnupg installed?)
```
This applies to ALL repos (Ubuntu, Mozilla, Cloudsmith, foundrylinux, worldfoundry).

**Sequence:**
1. `lb_chroot_install-packages` runs apt **from the host container** acting on the
   chroot. The host's apt/gpgv work fine → packages install cleanly.
2. `lb_chroot_hooks` runs hook scripts **inside the chroot** via `chroot chroot/`.
   The chroot's apt calls `execvp("gpgv", ...)` and fails.

**NOT caused by:**
- gpgv being removed (binary is present; `/usr/bin/gpgv --version` works)
- gpgv being marked auto-installed (apt-mark manual gnupg gpgv shows "already set")
- autoremove removing gnupg (apt-mark manual prevents it)

**Likely cause:** Ubuntu 26.04's apt uses the Sequoia library for verification by
default. In a live-build chroot (no /proc/self/fd, no systemd, restricted
environment), Sequoia's initialization fails. apt falls back to calling the
external `gpgv` binary, but the fallback path's `execvp` fails for an unknown
reason (possibly chroot vs host PATH or exec environment mismatch).

**Effect:** apt marks every repo as unverified for this session. Attempting
`apt-get install -t mozilla firefox --allow-downgrades` silently returns
"already newest version (1:1snap1-0ubuntu8)" because apt won't downgrade to
an unverified package.

**Fix (hook 0025, 0.9.50):** Bypass apt entirely. Use the Mozilla package index
already on disk (cached by the host's apt during lb_chroot_install-packages),
extract the firefox `.deb` URL, download with `curl`, install with
`dpkg --force-downgrade -i`. This completely bypasses apt's gpgv check.

---

## no-snapd.pref pin cleared before package install (2026-06-04)

`config/apt/preferences.d/no-snapd.pref` (priority -1 for snapd) is backed up
and removed before `lb_chroot_install-packages`. Snapd installs anyway because
`firefox (1:1snap1-0ubuntu8)` has `PreDepends: snapd`, which `kubuntu-desktop`
pulls in. The no-snapd pin was meant to block it but is NOT active at install
time.

Correct removal path: hook 0025 swaps firefox for Mozilla's native .deb (no
snapd PreDepend), then `apt-get purge snapd` succeeds because nothing depends
on it anymore.

---

## hook 0020 autoremove removes gnupg/gpgv (2026-06-04, fixed)

`apt-get purge libreoffice*` orphans gnupg/gpgv (libreoffice depends on gnupg
for macro signing; when libreoffice goes, gnupg becomes auto-installed with no
remaining manual consumer).

**Fix:** `apt-mark manual gnupg gpgv` before `apt-get autoremove` in hook 0020.
Added to both hooks 0005 (as first-hook safety net) and 0020 (before autoremove).

Note: this fix prevents *autoremove* from removing gnupg, but doesn't fix the
gpgv-fails-in-apt issue above (that's a separate problem).

---

## apt-get purge snapd fails after dpkg --force-downgrade firefox (2026-06-04)

After `dpkg --force-downgrade -i firefox-mozilla.deb` replaces the snap-transitional
firefox, `apt-get purge snapd` still returns "0 to remove" — it silently does nothing.

**Cause:** apt's package cache (from `/var/lib/apt/lists/`) still contains the Ubuntu
archive entry for `firefox 1:1snap1-0ubuntu8` with `PreDepends: snapd`. When apt
resolves whether snapd can be removed, it checks which installed packages depend on it.
But apt uses the CACHE (index) for this lookup in some code paths, not just dpkg status.
The result: apt sees firefox-1:1snap1 as an installed package that needs snapd, even
though dpkg status shows Mozilla's firefox.

**Fix:** Use `dpkg --purge snapd` — dpkg reads only `/var/lib/dpkg/status`, not
the apt cache. Purge in reverse-dependency order (packages that depend on snapd first):
`plasma-discover-backend-snap` → `snap-store` → `snapd`.

**Symptom to diagnose it:** Two `Reading package lists...` blocks in the output with
`0 upgraded, 0 newly installed, 0 to remove`. No error — silent no-op.

---

## dpkg -i upgrade doesn't update apt orphan tracking (2026-06-04)

`hook 1000` installs all local debs with `dpkg -i *.deb`. When
`foundry-python-gamedev-extras` is upgraded from 1.0.0 (which Depends python3-opencv)
to 1.0.1 (which does not), apt's orphan tracking doesn't update — it still thinks
python3-opencv is needed.

**Cause:** `dpkg -i` doesn't update apt's extended states (the `auto-installed` flag
tracking in `/var/lib/apt/extended_states`). When apt-get autoremove runs, it looks at
extended_states to decide what's orphaned. A package upgraded via dpkg -i leaves its
dependents' auto-install tracking stale.

Even `apt-mark auto python3-opencv` followed by `apt-get autoremove` fails — apt's
reverse-dependency resolver still sees the old relationships.

**Fix:** Use `dpkg --purge --force-depends` on the leftover packages directly. This
bypasses apt's dependency resolution entirely. The `--force-depends` is needed because
the opencv packages have interdependencies (e.g., `libopencv-viz` depends on
`libvtk9.5t64`) and dpkg would otherwise refuse to purge them in the wrong order.

---

## hook execution order (foundry-iso, confirmed 2026-06-04)

```
0005  disable-packagekit-invoke  → write DPkg::Post-Invoke ""; apt-mark manual gpgv
0010  enable-multiverse          → add-apt-repository multiverse; apt-get update (W: gpgv errors)
0020  strip-kubuntu-bloat        → purge libreoffice, plasma-welcome, snapd (if no preDepend)
0025  mozilla-pin                → dpkg --force-downgrade Mozilla firefox; purge snapd
0028  jre-preinstall             → apt install openjdk-17-jre-headless
0030  install-foundry-edition    → apt install foundry-${EDITION} from local-debs
0040  firstboot-cleanup          → remove calamares live artifacts
0050  shim-signed                → mkdir /boot/efi/EFI/ubuntu; apt install shim-signed
1000  install-local-debs         → dpkg -i all .debs in /tmp/local-debs/
1010  trim-atelier-only-pkgs     → apt-mark auto ghidra openjdk-21 opencv vtk; autoremove
1050  plymouth-set-theme         → plymouth-set-default-theme foundry
1100  (branding hooks)
1150  (theming hooks)
1200  (final hooks)
```

live-build processes `.hook.chroot` files in lexicographic order.
Numeric prefix ensures deterministic ordering across hook batches.

---

## lb_chroot_package-lists: queues, does not purge (2026-06-04)

`lb_chroot_package-lists` processes `*.list.chroot` files (install queuing).
**`.list.chroot.purge` files do NOT fire in live-build 3.0~a57** — the purge
list mechanism is not implemented. Use a hook script instead.

---

## Cloudsmith task/go-task repo (2026-06-04)

`foundry-iso/config/archives/cloudsmith-task.list.chroot` pins go-task. The
apt source URL changed layout (`any-distro` → per-distro) — watch for 404s.
`task check-apt-repos` will catch this.

---

## shim-signed postinst crashes in live-build chroot (2026-06-06)

`shim-signed` is required for Secure Boot on real hardware (UEFI with Secure Boot
enabled, which is the default on modern PCs). It must be in `foundry.list.chroot`
so `grub-install --uefi-secure-boot` can chain to it on the installed system.

**Symptom:** The `shim-signed` postinst calls:
```
update-alternatives --install /boot/efi/EFI/ubuntu/shimx64.efi.signed shimx64.efi.signed ...
```
In the live-build chroot `/boot/efi/EFI/ubuntu/` does not exist (EFI partition isn't
mounted), so `update-alternatives` exits non-zero:
```
update-alternatives: error: no alternatives for shimx64.efi.signed
```
This causes the package install to fail and `lb_chroot_install-packages` aborts.

**Fix:** Do NOT put `shim-signed` in `foundry.list.chroot`. Instead, install it from a
hook script that creates the directory first. Hooks run AFTER `lb_chroot_install-packages`,
so by hook time apt-get is available and `/boot/efi/EFI/ubuntu` can be created before the
install.

`config/hooks/0050-shim-signed.hook.chroot`:
```bash
#!/bin/bash
set -euo pipefail
mkdir -p /boot/efi/EFI/ubuntu
apt-get install -y --no-install-recommends shim-signed
```

Hook 0028 (JRE preinstall) confirms this pattern works: `apt-get install -y` inside a
`.hook.chroot` succeeds for Ubuntu main/universe packages because the Ubuntu archive GPG
key is present and working inside the chroot. The gpgv failure issue (see above) only
affects third-party repos.

**Hook location:** Hooks must be in `config/hooks/` (NOT `config/hooks/live/`). All
existing hooks live in the parent directory; `config/hooks/live/` is not processed by
live-build 3.0~a57.

---

## Calamares 3.3.14 partitionLayout mandatory keys (2026-06-04 → 06)

`PartitionLayout::init()` in Calamares 3.3.14 checks:
```cpp
if (!pentry.contains("name") || !pentry.contains("size"))
    return false; // silently falls back to default layout
```
Both `name` AND `size` are mandatory in every `partitionLayout` entry. Without `name:`,
Calamares silently switches to a default layout that creates no root partition — then
`unsquashfs` extracts the live filesystem to the live tmpfs (which can't hold 4+ GB)
and the installer exits 1.

**Correct partition.conf excerpt:**
```yaml
partitionLayout:
  - name:        "root"
    filesystem:  "ext4"
    mountPoint:  "/"
    size:        100%
```

Also required: `userSwapChoices: []` to suppress the swap dropdown (otherwise Calamares
inserts a swap partition and the 100% size becomes ambiguous), and an `efi:` block
specifying `mountPoint: "/boot/efi"`.

**Bootloader:** `bootloader.conf` is mandatory. Without it the bootloader module skips
entirely. Minimum viable config:
```yaml
efiBootLoader:    "grub"
grubInstall:      "grub-install"
grubMkconfig:     "grub-mkconfig"
grubCfg:          "/boot/grub/grub.cfg"
grubProbe:        "grub-probe"
efiBootloaderId:  "foundry-linux"
installEFIFallback: false
```

**grub-efi-amd64 must be in squashfs**, not installed via the packages module —
Calamares's `skip_if_no_internet: true` means the packages module skips in the QEMU
test harness (no DNS), and even with `skip_if_no_internet: false` the chroot can't
resolve `archive.ubuntu.com`. Add `grub-efi-amd64` to `foundry.list.chroot`.

**Module order** in `settings.conf` matters: `packages` must come before `bootloader`
so grub-efi-amd64 is installed in the target before `grub-install` runs.

**Calamares debug log:** `/root/.cache/calamares/session.log` — not stdout.
Calamares caches config at startup; after patching `/etc/calamares/`, kill the process
and relaunch with display env set:
```bash
DISPLAY=:0 WAYLAND_DISPLAY=wayland-0 XDG_RUNTIME_DIR=/run/user/1000 \
  DBUS_SESSION_BUS_ADDRESS=unix:path=/run/user/1000/bus \
  calamares -d
```

**Test disk must be at `/var/tmp/`** (real disk), NOT `/tmp/` (7.3 GB tmpfs). Add
`-drive file=/var/tmp/foundry-test.img,...` to the QEMU launch command.

**Wipe test disk between runs** if a previous partial install left partition signatures:
```bash
dd if=/dev/zero of=/dev/sda bs=1M count=10
```
Otherwise Calamares's "Erase disk" option may disappear (it sees an existing layout and
offers only manual partitioning).
