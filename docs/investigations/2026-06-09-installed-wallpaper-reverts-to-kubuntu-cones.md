# Investigation: installed system reverts to the Kubuntu "cones" wallpaper

**Date**: 2026-06-09
**ISO versions affected**: desktop wallpaper through 0.9.95 (fixed 0.9.96); **lock screen** — a
separate surface — through 0.9.105 (fixed 0.9.106, see [follow-up](#follow-up-2026-06-10-the-lock-screen-was-a-fourth-surface-fixed-in-09106))
**Severity**: high — first impression. A brand-new install of *Foundry* Linux greets the user
with the *Kubuntu* default wallpaper.
**Status**: desktop fixed in hook `1100-live-autologin.hook.chroot` (0.9.96); lock screen fixed in
the `kscreenlockerrc` static include (0.9.106)

---

## Symptom

- **Live session**: correct Foundry wallpaper. ✓
- **Installed system, first boot**: the Kubuntu default "cones" wallpaper. ✗

The live ISO looked right, so the regression hid on the one surface we don't see until the very
end of a multi-minute install: the installed desktop's first boot.

---

## How `build.log` pointed to it

Reviewing the captured build log (the reason `task iso-build` now tees to
`dist/build-<edition>-<version>.log`) showed the wallpaper hook reporting success:

```
Patched 6 look-and-feel defaults file(s)
Patched system appletsrc: /etc/xdg/plasma-org.kde.plasma.desktop-appletsrc
```

…immediately after `Setting up foundry-kde-theme`, which **ships** that same appletsrc pointing
at the *correct* Foundry wallpaper. The hook was overwriting a good value with a path that
only exists while the installer is present. The build log made the "set it, then re-point it
somewhere fragile" sequence visible.

---

## Root cause

The wallpaper hook pointed every wallpaper lever at:

```
/usr/share/sddm/themes/foundry-linux/background.png
```

That file is shipped by **`calamares-settings-foundry-linux`** (the installer's branding
package). Its control file declares:

```
Package: calamares-settings-foundry-linux
Depends: ${misc:Depends}, calamares
```

Calamares removes itself from the freshly installed target. Its
`modules/packages.conf` runs:

```yaml
operations:
  - remove:
    - calamares
    - live-boot
    - live-config
    - ...
```

`apt remove calamares` cannot leave a package installed that `Depends: calamares`, so it also
removes the reverse-dependency **`calamares-settings-foundry-linux`** — and with it
`/usr/share/sddm/themes/foundry-linux/background.png`.

On the installed system the wallpaper config therefore pointed at a **deleted file**. Plasma 6,
unable to load it, fell back to its compiled-in distro default: the Kubuntu cones.

### Why live worked but installed didn't

| | `calamares-settings-foundry-linux` present? | `…/sddm/themes/foundry-linux/background.png` | Wallpaper |
|---|---|---|---|
| Live session | Yes (installer is on the live ISO) | exists | Foundry ✓ |
| Installed first boot | **No** (purged with `calamares`) | **deleted** | Kubuntu cones ✗ |

The bug was invisible on every surface except the installed desktop, which is exactly the
surface we iterate on least.

---

## The fix

Use the wallpaper shipped by **`foundry-kde-theme`**, which is a normal part of the edition and
is **not** removed on install:

```
/usr/share/wallpapers/FoundryLinux-ForgeHorizon/
```

`foundry-kde-theme` already ships `/etc/xdg/plasma-org.kde.plasma.desktop-appletsrc` pointing at
exactly this path — the hook's old job of *overwriting* it with the SDDM path was the entire
regression. Hook 1100 now points all three levers at ForgeHorizon:

1. **LAF defaults** (`/usr/share/plasma/look-and-feel/*/contents/defaults`) — `Image=` →
   `file:///usr/share/wallpapers/FoundryLinux-ForgeHorizon/`
2. **System appletsrc** (`/etc/xdg/plasma-org.kde.plasma.desktop-appletsrc`) — re-asserted to the
   same path (idempotent with what `foundry-kde-theme` ships)
3. **Autostart** (`/etc/xdg/autostart/foundry-wallpaper.desktop` →
   `foundry-set-wallpaper.sh`) — runs `plasma-apply-wallpaperimage
   /usr/share/wallpapers/FoundryLinux-ForgeHorizon/` once per user (sentinel-gated)

Of these, **#3 is the load-bearing lever** — the LAF/appletsrc defaults proved unreliable
across earlier ISOs (Plasma reads them only on a truly first-run config and frequently
re-derives the containment), which is why the per-user `plasma-apply-wallpaperimage` autostart
exists. #1 and #2 are belt-and-suspenders. All three now survive the install.

---

## Lesson

**Any path the installed-system config references must be shipped by a package that survives the
install.** Anything from `calamares-settings-*` is installer-only — it is purged from the target
along with `calamares`. The SDDM theme (login screen art) lives there legitimately, but it must
not be the *source of truth* for the installed desktop. Desktop assets the installed system
depends on belong in a normal edition package (`foundry-kde-theme`), not in the installer's
branding package.

This is the same class of bug as the rsync-error-11 disk-exhaustion lesson: a file present in
the live chroot is not necessarily present on the installed target.

---

## Follow-up 2026-06-10: the lock screen was a fourth surface (fixed in 0.9.106)

The 0.9.96 fix covered the **desktop** wallpaper. Three other "who shows art" surfaces are
separate: the **SDDM login greeter** and the **avatar** (both via `foundry-kde-theme`, which
survives install) — and the **lock screen** (`kscreenlocker`), which this follow-up is about.

**Symptom (observed on installed 0.9.104):** desktop wallpaper correct, SDDM login correct,
avatar (the anvil) correct — but **locking the session** still showed the Kubuntu cones.

**Root cause — same class of bug, a different dead path.** The lock-screen wallpaper is set by a
static include shipped in the ISO:

```
foundry-iso/config/includes.chroot/etc/xdg/kscreenlockerrc
```

It already declared a Foundry wallpaper, but pointed at a path that **does not exist** on the
installed system (`ls` → No such file or directory) — a leftover from an older layout:

```
Image=file:///usr/share/backgrounds/foundry-linux-wallpaper.png
```

With the Image unreadable, the lock greeter fell back to the global Kubuntu cones default,
exactly as the desktop did. (`/etc/xdg/kscreenlockerrc` is owned by **no package** — `dpkg -S`
finds nothing — because the ISO ships it as a static include, not via a deb.)

**Fix (0.9.106):** repoint the include at the real wallpaper shipped by `foundry-kde-theme`
(survives install), using a concrete image file (the greeter is happier with a file than a
package dir):

```
Image=file:///usr/share/wallpapers/FoundryLinux-ForgeHorizon/contents/images/3840x2160.png
```

**Dead end worth recording:** the first attempt shipped `/etc/xdg/kscreenlockerrc` *from*
`foundry-kde-theme` (1.0.4) as a conffile. That **collided** with the static include — the file
already existed in the chroot, so dpkg halted on a conffile prompt (`kscreenlockerrc
(Y/I/N/O/D/Z)`) inside a non-interactive hook and **broke the 0.9.105 build**. Reverted to 1.0.3;
the correct fix is the one-line path correction in the include that already owned the surface.

**Surface inventory — so none is missed again:**

| Surface | Config | Shipped by | Wallpaper / art source |
|---------|--------|-----------|------------------------|
| Desktop | `plasma-org.kde.plasma.desktop-appletsrc` (+ autostart) | foundry-kde-theme + hook 1100 | ForgeHorizon |
| SDDM login | `/etc/sddm.conf.d/*` → `foundry-linux` theme | foundry-kde-theme | theme `background.png` |
| **Lock screen** | `/etc/xdg/kscreenlockerrc` | **foundry-iso static include** | ForgeHorizon `3840x2160.png` |
| Avatar (login/lock/logout) | `/etc/skel/.face` (+ `.face.icon`) | foundry-kde-theme + hook 1100 | anvil avatar |
