# Investigation: KDE desktop theme packaging for Foundry Linux

**Date:** 2026-05-20  
**Context:** Phase 3 (Foundry Linux ISO) will ship a Kubuntu 26.04 base. We need to decide
how much custom theming to ship — at minimum a branded wallpaper, at most a full KDE
look-and-feel — and how to package it.

---

## Debian/Ubuntu package naming conventions (theme artifacts)

No binding Debian Policy governs theme names. The
[DebianArt/StructureThemePackage](https://wiki.debian.org/DebianArt/StructureThemePackage)
wiki documents a proposed structure; real-world packages mostly follow consistent suffixes.

| Artifact | Pattern | Real examples |
|---|---|---|
| Icon theme | `<name>-icon-theme` | `papirus-icon-theme`, `humanity-icon-theme`, `breeze-icon-theme` |
| GTK theme | `<name>-gtk-theme` or `<name>-theme` | `arc-theme`, `materia-gtk-theme` |
| KDE/Plasma theme | `plasma-theme-<name>` | `plasma-theme-breath` (Manjaro) |
| KDE look-and-feel | `<name>-plasma-look-and-feel` | (Ubuntu uses `yaru-theme-*` split) |
| Cursor theme | `<name>-cursor-theme` or `xcursor-<name>` | `dmz-cursor-theme`, `xcursor-themes` |
| Sound theme | `sound-theme-<name>` | `sound-theme-freedesktop` |
| Wallpapers | `<name>-wallpapers` or `<name>-backgrounds` | `ubuntu-wallpapers`, `plasma-workspace-wallpapers` |
| Full bundle (meta) | `<name>-theme` | (meta-package pulling component debs) |

**Key rule:** lowercase, hyphens, suffix describes the artifact type.
For Foundry Linux, the natural name prefix is `foundry-`.

---

## KDE theming layers (what's actually changeable)

KDE Plasma exposes theming at several independent layers, from least to most effort:

### 1. Desktop wallpaper — trivial
A `.jpg`/`.png` in `/usr/share/wallpapers/<Name>/` with a `metadata.json`. Plasma picks it
up automatically in the wallpaper chooser. Can ship a system-default via
`/etc/xdg/plasma-org.kde.plasma.desktop-appletsrc` (or a KDE `kconf_update` script).

**Package:** `foundry-wallpapers`  
**Architecture:** `all`  
**Effort:** very low

### 2. Color scheme — low effort
A `.colors` file in `/usr/share/color-schemes/`. Just an INI-format palette file.
Plasma's color scheme chooser picks it up immediately; no restart needed.

**Package:** folded into `foundry-plasma-theme` or shipped standalone as
`foundry-color-scheme`  
**Effort:** low — generate from brand colors (<span style="display:inline-block;width:12px;height:12px;background:#E85D00;border-radius:2px;vertical-align:middle"></span> `#E85D00` ember orange etc.)

### 3. Plasma Desktop theme (SVG-based shell chrome)
Controls panel, widgets, tooltips, window decorations chrome. Lives in
`/usr/share/plasma/desktoptheme/<Name>/`. Inherits from `breeze-dark` or `default`;
only override what you want to change (colors, a few SVGs).

**Package:** `foundry-plasma-theme`  
**Effort:** medium — requires SVG editing but can inherit most of Breeze Dark

### 4. Icon theme
Freedesktop spec; `/usr/share/icons/<Name>/` with `index.theme`. Can inherit from
Papirus or Breeze and only ship overrides for app-specific icons.

**Package:** `foundry-icon-theme`  
**Effort:** medium-to-high — even a partial set takes time to do well

### 5. KDE Look-and-Feel package
Bundles wallpaper + color scheme + plasma theme + window decoration + SDDM theme into
a single installable unit. Installed to
`/usr/share/plasma/look-and-feel/<reverse-dns-id>/`. Applied in one click via
System Settings > Global Theme.

**Package:** Could be a single `foundry-plasma-look-and-feel` deb containing
the `contents/` dir, or a meta-package pulling component debs.  
**Effort:** high if building everything; low if it's just a wrapper pointing at
existing components

---

## Recommended approach for Foundry Linux (tiered)

### Tier 1 — ship immediately with the ISO (minimal effort)
- **Branded wallpaper** (`foundry-wallpapers`) — forge/anvil imagery, dark background
- **Color scheme** (folded in, or `foundry-color-scheme`) — ember orange + forge dark palette
- Set wallpaper + color scheme as system default via `/etc/xdg/` config drop-in

Package count: 1–2. Looks intentional without requiring custom SVG work.

### Tier 2 — first proper theme release
- **Plasma Desktop theme** (`foundry-plasma-theme`) — inherits Breeze Dark, overrides
  accent colors and possibly panel/taskbar SVGs
- **Look-and-feel bundle** (`foundry-look-and-feel`) — ties wallpaper + color scheme +
  plasma theme together for one-click apply

### Tier 3 — full brand treatment
- **Icon theme** (`foundry-icon-theme`) — inherits Papirus Dark; custom icons for
  WorldFoundry apps (blender-asset-finder, wf-blender-install, etc.)
- **SDDM login theme** — ✓ done (`calamares-settings-foundry-linux`; `/usr/share/sddm/themes/foundry-linux/`)

---

## File layout (Tier 1 example)

```
foundry-wallpapers/
  debian/
    control       # Architecture: all; Section: graphics
    changelog
    rules         # simple install, no build step
    copyright
  wallpapers/
    FoundryLinux/
      metadata.json          # { "Name": "Foundry Linux", ... }
      contents/
        screenshot.png       # thumbnail shown in chooser
        images/
          foundry-5120x2880.png   # highest res, Plasma downscales
          foundry-1920x1080.png
```

`/etc/xdg/plasma-org.kde.plasma.desktop-appletsrc` drop-in (or `kconf_update` script) sets
`Image=file:///usr/share/wallpapers/FoundryLinux/contents/images/foundry-1920x1080.png`
as default for new users.

---

## References

- [Freedesktop Icon Theme Spec](https://specifications.freedesktop.org/icon-theme-spec/latest/)
- [KDE Plasma Theme Tutorial](https://develop.kde.org/docs/plasma/theme/theme-details/)
- [KDE Look-and-Feel packages](https://develop.kde.org/docs/plasma/lookandfeel/)
- [DebianArt/StructureThemePackage](https://wiki.debian.org/DebianArt/StructureThemePackage)
- [plasma-workspace-wallpapers source](https://invent.kde.org/plasma/plasma-workspace-wallpapers)
  — good reference for `metadata.json` format and deb packaging
