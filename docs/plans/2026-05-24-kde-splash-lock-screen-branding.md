# Plan: KDE Plasma Splash Screen + Lock Screen Branding

## Context

After SDDM autologins, KDE Plasma shows a splash screen while the desktop
loads. Currently shows the Kubuntu logo (from the kubuntu look-and-feel).
Similarly, the lock screen wallpaper falls back to Kubuntu's default.
Both need to show Foundry Linux branding.

## Approach

Deliver via `foundry-iso/config/includes.chroot/` overlay — files are merged
into the chroot filesystem at build time, no .deb rebuild needed.

Assets already installed by `calamares-settings-foundry-linux`:
- `/usr/share/backgrounds/foundry-linux-wallpaper.png` — 4K wallpaper
- `/usr/share/plymouth/themes/foundry-linux/logo.png` — Foundry logo

## Files to create

### 1. Look-and-Feel metadata
`includes.chroot/usr/share/plasma/look-and-feel/org.foundrylinux.foundry-linux/metadata.json`

```json
{
    "KPackageStructure": "Plasma/LookAndFeel",
    "KPlugin": {
        "Id": "org.foundrylinux.foundry-linux",
        "Name": "Foundry Linux",
        "Version": "1.0.0"
    }
}
```

### 2. Splash.qml
`includes.chroot/usr/share/plasma/look-and-feel/org.foundrylinux.foundry-linux/contents/splash/Splash.qml`

Dark background + centred logo (fades in at stage 1) + 5-dot progress row
(dots light up ember-orange as stages complete). References assets by
absolute path since they're installed by the branding .deb.

### 3. ksplashrc (system-wide)
`includes.chroot/etc/xdg/ksplashrc`
```ini
[KSplash]
Theme=org.foundrylinux.foundry-linux
```

### 4. kscreenlockerrc (lock screen wallpaper)
`includes.chroot/etc/xdg/kscreenlockerrc`
```ini
[Greeter][Wallpaper][org.kde.image][General]
Image=file:///usr/share/backgrounds/foundry-linux-wallpaper.png
```

### 5. Fix includes.chroot deb (1.0.3 → 1.0.4)
The other agent restored the old .deb; swap back via Docker.

## Verification
Build ISO (`task iso-build EDITION=anvil`), boot in QEMU, observe:
- Plasma splash shows Foundry logo (not Kubuntu gear)
- Lock screen (Super+L) shows Foundry wallpaper
