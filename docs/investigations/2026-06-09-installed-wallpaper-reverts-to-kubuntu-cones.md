# Investigation: installed system reverts to the Kubuntu "cones" wallpaper

**Date**: 2026-06-09
**ISO versions affected**: every build through 0.9.95 (fixed in 0.9.96)
**Severity**: high — first impression. A brand-new install of *Foundry* Linux greets the user
with the *Kubuntu* default wallpaper.
**Status**: fixed in hook `1100-live-autologin.hook.chroot` (0.9.96)

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
