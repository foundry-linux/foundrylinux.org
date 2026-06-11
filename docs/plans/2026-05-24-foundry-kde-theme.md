# foundry-kde-theme — KDE Plasma Theming Package

**Status:** Complete (v1.0.5) — single-package consolidation done; Aurorae/Kvantum pending (future)  
**Package:** `foundry-kde-theme` (`foundry-apt/packages/foundry-kde-theme/`)  
**Repo:** `apt.foundrylinux.org`

---

## Goal

Ship KDE Plasma desktop branding for Foundry Linux as a single `foundry-kde-theme`
package installable from `apt.foundrylinux.org`. `foundry-kde-theme` is the **sole
owner** of all desktop theming — no theming hooks or static includes in `foundry-iso/`.

---

## Palette

| Token | Value | Role |
|---|---|---|
| Background | <span style="display:inline-block;width:12px;height:12px;background:#0a0a0a;border:1px solid #444;vertical-align:middle"></span> `#0a0a0a` | Near-black window/panel base |
| Text | <span style="display:inline-block;width:12px;height:12px;background:#f7f7f7;border:1px solid #444;vertical-align:middle"></span> `#f7f7f7` | Warm-white primary text |
| Accent | <span style="display:inline-block;width:12px;height:12px;background:#ff5b1a;border-radius:2px;vertical-align:middle"></span> `#ff5b1a` | Forge orange — highlight, selection, focus |

---

## What ships in each version

### 1.0.0 — Color scheme

`FoundryLinux.colors` → `/usr/share/color-schemes/`

KDE INI color scheme covering all KDE color groups (Window, Button, View, Selection,
Tooltip, Complementary, Header). Sets the near-black / warm-white / orange palette
across all KDE applications and Plasma shell elements.

### 1.0.1 — Wallpaper + system default

Three additions:

1. **`metadata.json`** → `/usr/share/wallpapers/FoundryLinux-ForgeHorizon/`
   KPackage manifest that makes Plasma recognise the wallpaper in System Settings.

2. **PNG images** → `/usr/share/wallpapers/FoundryLinux-ForgeHorizon/contents/images/`
   `1920x1080.png` and `3840x2160.png`. Plasma downscales the highest available resolution.

3. **`/etc/xdg/plasma-org.kde.plasma.desktop-appletsrc`**
   System-wide KDE default — sets containment 1 wallpaper to `FoundryLinux-ForgeHorizon`.

### 1.0.2 — SDDM login theme

`data/sddm-theme/` → `/usr/share/sddm/themes/foundry-linux/`

`Main.qml`, `background.png`, and `metadata.desktop`. The SDDM greeter uses
`foundry-linux` as its theme. **Must ship from a survives-install package** — shipping
it from `calamares-settings-*` causes the greeter to revert to Kubuntu cones after
install (calamares and its settings package are purged by the installer).

### 1.0.3 — Anvil avatar

`data/avatar.png` → `/usr/share/foundry-linux/avatar.png`

The Foundry anvil logo (square, ~256×256, used as the user avatar on login/logout/lock
screens). Placement in `/etc/skel/.face` is handled by `preinst`/`postinst` via
`dpkg-divert` (see Kubuntu-owned paths below).

### 1.0.4 — Full theming consolidation (LAF, Plasma Style, single-package)

The headline release. Moves all scattered theming into this one package and deletes
the `foundry-iso` theming hooks and static includes:

- **Look-and-Feel package** (`org.foundrylinux.foundry-linux`) — moved from
  `foundry-iso` static include into the package. Gains `contents/defaults` (wires
  color scheme, Plasma Style, splash, wallpaper), `contents/layouts/org.kde.plasma.desktop-layout.js`
  (taskbar pins: settings, software, files, terminal, Blender, browser — Kate omitted),
  and a fixed `Splash.qml` (both dead asset paths corrected to survives-install paths:
  ForgeHorizon wallpaper + anvil avatar). **X-Plasma-APIVersion: 2** added to
  `metadata.json` for the layout.js globals (`applicationExists`, `loadTemplate`).

- **Plasma Style** (`FoundryLinux` desktoptheme) — minimal `metadata.json` +
  `colors` file inheriting Breeze SVGs with the forge palette. Panel renders dark.
  Referenced by the LAF defaults (`[plasmarc][Theme] name=FoundryLinux`). Distinctive-shape
  SVG overrides are the follow-up.

- **`/etc/xdg/kdeglobals`** — ships `ColorScheme=Foundry Linux` +
  `LookAndFeelPackage=org.foundrylinux.foundry-linux`. Replaces the old non-executable
  hook `1150-kde-color-scheme.hook.chroot`.

- **`/etc/xdg/kscreenlockerrc`** — ForgeHorizon wallpaper for the lock screen,
  with `WallpaperPlugin=org.kde.image` set (required — lock greeter does not default
  to it; without it the `Image=` path is silently ignored → cones).

- **`/etc/xdg/ksplashrc`** — points at `org.foundrylinux.foundry-linux`.

- **`/etc/xdg/autostart/foundry-wallpaper.desktop`** + `/usr/lib/foundry-kde-theme/foundry-set-wallpaper.sh`
  — per-user wallpaper autostart. Runs `plasma-apply-wallpaperimage` once per login,
  sentinel-gated. The autostart is the load-bearing lever for the wallpaper on the
  **installed** system (LAF defaults + appletsrc are belt-and-suspenders).

- **`dpkg-divert`** for two Kubuntu-owned paths (see below):
  - `/etc/skel/.face` → kubuntu's blue gear diverted to `.face.kubuntu`; postinst
    copies `/usr/share/foundry-linux/avatar.png` to `.face`.
  - `/etc/xdg/kicker-extra-favoritesrc` → kubuntu's Kate-containing list diverted;
    postinst copies Foundry's version (Kate out, Blender in) to the same path.

**foundry-iso deletions (done in this version):** hook `1150-kde-color-scheme.hook.chroot`
(removed), `config/includes.chroot/etc/xdg/kscreenlockerrc` + `ksplashrc` (removed),
`config/includes.chroot/usr/share/plasma/look-and-feel/org.foundrylinux.foundry-linux/`
(removed). Hook `1100` kept for autologin + QEMU virtio EDID; theming sed-loops removed.

### 1.0.5 — LAF preview thumbnail

`data/look-and-feel/org.foundrylinux.foundry-linux/contents/previews/preview.png` →
`/usr/share/plasma/look-and-feel/org.foundrylinux.foundry-linux/contents/previews/`

"Foundry Linux" now shows a live-desktop screenshot thumbnail in System Settings →
Appearance → Global Theme.

### Future layers

| Layer | Status | Location |
|---|---|---|
| Plasma Style (full SVG set) | Partial — forge-palette `colors` + Breeze inherit done; distinctive panel/widget SVGs pending | `/usr/share/plasma/desktoptheme/FoundryLinux/` |
| Aurorae window decoration | Not started | `/usr/share/aurorae/themes/FoundryLinux/` |
| Kvantum application style | Not started | `/usr/share/Kvantum/FoundryLinux/` |

See [`docs/investigations/2026-05-24-kde-themes.md`](../investigations/2026-05-24-kde-themes.md) for design notes on each layer.

---

## Kubuntu-owned paths and dpkg-divert

`foundry-kde-theme` uses `dpkg-divert` to take ownership of two paths that
`kubuntu-settings-desktop` ships:

| Path | Kubuntu file | Divert destination | Foundry replacement |
|---|---|---|---|
| `/etc/skel/.face` | blue-gear avatar PNG (conffile) | `/etc/skel/.face.kubuntu` | anvil avatar copied from `/usr/share/foundry-linux/avatar.png` in `postinst` |
| `/etc/xdg/kicker-extra-favoritesrc` | app-menu Favorites (Kate included) | `/etc/xdg/kicker-extra-favoritesrc.kubuntu` | Foundry's list (Kate out, Blender in) copied from `/usr/share/foundry-kde-theme/kicker-extra-favoritesrc` in `postinst` |

The divert is registered in `preinst` (install + upgrade), removed in `postrm`
(remove + purge). Survives `kubuntu-settings-desktop` upgrades: dpkg keeps the
diverted Kubuntu file, installs its new version to `.kubuntu`, and never touches
the Foundry file.

**Why divert, not hook overwrite?** `/etc/skel/.face` is a **conffile** — dpkg
tracks its hash and prompts the user on upgrade if the file has changed. A hook
overwrite dodges the conffile mechanic and works for the initial install, but a
kubuntu-settings upgrade then prompts the user to keep or drop the "modified"
conffile, breaking the silent upgrade path. The divert moves the Kubuntu file
aside entirely so dpkg never sees a conflict.

---

## File layout

```
foundry-apt/packages/foundry-kde-theme/
  debian/
    control            Depends: plasma-workspace, sqlite3
    changelog          version source (1.0.5 current)
    rules              "%: dh $@"
    install            explicit file → destination mapping
    preinst            dpkg-divert --add for .face + kicker-extra-favoritesrc
    postinst           copy avatar + kicker favoritesrc to diverted paths
    postrm             dpkg-divert --remove on remove/purge
    source/format      "3.0 (native)"
    copyright
  data/
    avatar.png                                  → /usr/share/foundry-linux/
    color-schemes/
      FoundryLinux.colors                       → /usr/share/color-schemes/
    wallpapers/
      FoundryLinux-ForgeHorizon/
        metadata.json
        contents/images/
          1920x1080.png
          3840x2160.png
    sddm-theme/
      Main.qml                                  → /usr/share/sddm/themes/foundry-linux/
      background.png
      metadata.desktop
    look-and-feel/
      org.foundrylinux.foundry-linux/
        metadata.json                           → /usr/share/plasma/look-and-feel/…/
        contents/
          defaults
          layouts/
            org.kde.plasma.desktop-layout.js
          splash/
            Splash.qml
          previews/
            preview.png
    desktoptheme/
      FoundryLinux/
        metadata.json                           → /usr/share/plasma/desktoptheme/FoundryLinux/
        colors
    xdg/
      kdeglobals                                → /etc/xdg/
      ksplashrc                                 → /etc/xdg/
      kscreenlockerrc                           → /etc/xdg/
      plasma-org.kde.plasma.desktop-appletsrc  → /etc/xdg/
      kicker-extra-favoritesrc                 → /usr/share/foundry-kde-theme/ (postinst copies to /etc/xdg/)
    autostart/
      foundry-wallpaper.desktop                → /etc/xdg/autostart/
    bin/
      foundry-set-wallpaper.sh                 → /usr/lib/foundry-kde-theme/
```

---

## Build

```bash
# Via Taskfile (builds all packages):
task build

# Single package:
cd foundry-apt && bash scripts/build-all.sh
```

---

## Verification

Full pixel-level verification run against 0.9.110 anvil ISO (2026-06-11). All surfaces
passed. See [docs/plans/2026-06-10-foundry-laf-and-plasma-style.md](2026-06-10-foundry-laf-and-plasma-style.md)
§ "Verification Results" for the complete evidence table.

Summary of what was verified pixel-level:
- ForgeHorizon wallpaper on live desktop and after install
- Lock screen → ForgeHorizon (not cones) — WallpaperPlugin fix holds
- Taskbar pins: Blender present, Kate absent; Blender pin launches Blender
- App-menu Favorites: Foundry set (LAF layout.js ran on first login)
- "Foundry Linux" Global Theme thumbnail in System Settings
- dpkg-divert: avatar (anvil) + kicker Favorites (Blender, no Kate) — both active
