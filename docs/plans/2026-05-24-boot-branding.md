# Boot Branding — Plymouth + UEFI Logo

## Context

The first thing a user sees after the QEMU/GRUB handoff is the **Kubuntu boot
splash** (the kubuntu Plymouth theme). Above it in QEMU, the **TianoCore UEFI
logo** flashes during firmware POST. Neither is Foundry Linux branding.

The `calamares-settings-foundry-linux` package already ships a complete
`foundry-linux` Plymouth theme at
`/usr/share/plymouth/themes/foundry-linux/`, but nothing was calling
`plymouth-set-default-theme` or rebuilding the initramfs, so Kubuntu's theme
won. This plan tracks replacing both logos and understanding the rendering
pipeline so future branding work has a clear map.

---

## Boot sequence — what renders what

| Stage | Renderer | Format | Our control? |
|---|---|---|---|
| UEFI POST | OVMF firmware (baked BMP) | Binary blob in `OVMF_CODE.fd` | No (see §TianoCore) |
| GRUB menu | GRUB2 (`theme.txt` + PNGs) | Declarative text + PNG | ✓ shipped in `calamares-settings-foundry-linux` |
| Boot splash | **Plymouth** (`.script` + PNGs) | Plymouth script mini-language | ✓ theme shipped, activation fix in 1.0.4 |
| Login screen | **SDDM** (`Main.qml`) | Qt Quick / QML | ✓ confirmed working (2026-05-24) |
| Installer slideshow | **Calamares** (`slideshow.qml`) | Qt Quick / QML | ✓ fully ours |

**QML primer:** [QML](https://doc.qt.io/qt-6/qmlapplications.html) (Qt Modeling Language) is the declarative UI language
embedded in Qt. Both SDDM and Calamares are C++ Qt applications that load a
`.qml` file and render it via the Qt Quick GPU pipeline. You describe UI as
nested components (`Rectangle`, `Image`, `Text`, `ListView`) with properties
and signal handlers. SDDM injects a global `sddm` object so QML can call
`sddm.login(user, pass, 0)`.

---

## Problem 1 — Plymouth (Kubuntu logo)

**Root cause:** `calamares-settings-foundry-linux` ships the theme files but
has no `postinst` and no ISO hook calling `plymouth-set-default-theme
foundry-linux`. Kubuntu's theme won because `kubuntu-desktop` sets it at
install time and nothing overrides it afterwards.

### Fix (two-pronged)

**A. ISO hook** — `foundry-iso/config/hooks/1050-plymouth-theme.hook.chroot`
(runs after `1000-install-local-debs` which installs the .deb):

```bash
#!/bin/bash
set -euo pipefail
plymouth-set-default-theme foundry-linux
update-initramfs -u -k all
```

**B. Package postinst** —
`foundry-apt/packages/calamares-settings-foundry-linux/debian/calamares-settings-foundry-linux.postinst`:

```bash
case "$1" in
    configure)
        if command -v plymouth-set-default-theme >/dev/null 2>&1; then
            plymouth-set-default-theme foundry-linux || true
            update-initramfs -u -k all || true
        fi ;;
esac
```

Both files are written. Package version bumped to **1.0.4** in
`debian/changelog`. **Done (2026-05-24):** built in Docker, swapped into
`foundry-iso/config/includes.chroot/tmp/local-debs/` (replaced 1.0.3).

---

## Problem 2 — TianoCore (UEFI logo)

**Root cause:** The TianoCore logo is a BMP compiled into `OVMF_CODE.fd`.
There is no OS-level hook to suppress it — it renders before the kernel loads.

**On real hardware:** users never see TianoCore — they see their PC OEM's logo
(Dell, ASUS, etc.). This is not a Foundry Linux problem for ISO users.

**In QEMU (development/VM distribution only):** The logo appears because OVMF
is used as the firmware. Investigated three suppression paths — all dead ends:

- **Runtime UEFI variable:** `QuietBoot` does not exist in Ubuntu's OVMF VARS
  or CODE binary. The logo is gated by `PcdQuietBoot`, a compile-time PCD, not
  a runtime variable. `python3-virt-firmware` cannot help here.
- **QEMU `-boot splash=`:** Only works with SeaBIOS (legacy BIOS), not OVMF.
- **SeaBIOS fallback:** Loses UEFI / Secure Boot support entirely.

**Verdict: accept it.** The flash is 1–2 s, only visible in QEMU. Real
hardware users see their PC OEM logo instead. If `.qcow2` VM images ship later,
bundle a custom `OVMF_CODE.fd` built from EDK2 with a blank BMP — Phase 4.

---

## Files changed

| File | Change | Status |
|---|---|---|
| `foundry-iso/config/hooks/1050-plymouth-theme.hook.chroot` | New — activates Plymouth theme in ISO chroot | ✓ done |
| `foundry-apt/packages/calamares-settings-foundry-linux/debian/calamares-settings-foundry-linux.postinst` | New — activates Plymouth theme on installed systems | ✓ done |
| `foundry-apt/packages/calamares-settings-foundry-linux/debian/changelog` | Bumped to 1.0.4 | ✓ done |
| `foundry-iso/config/includes.chroot/tmp/local-debs/` | Replaced 1.0.3 with 1.0.4 .deb | ✓ done |

---

## Verification

1. Build the ISO: `task iso-build EDITION=login-test` (fastest turnaround)
2. Launch in QEMU: `task iso-run EDITION=login-test`
3. **Expected:** boot splash shows Foundry Linux logo/background (not Kubuntu gear icon)
4. **Expected:** SDDM login screen shows Foundry dark panel (already working)
5. Confirm Plymouth theme is active inside the running ISO:
   ```bash
   plymouth-set-default-theme   # should print "foundry-linux"
   ```

---

## Out of scope (follow-up)

- GRUB theme is already shipped in `calamares-settings-foundry-linux` but the
  hook to wire it (`update-grub` + `grub-mkconfig`) has not been written yet.
  Tracked separately.
- OVMF custom logo for `.qcow2` VM image distribution — Phase 4 artifact.
