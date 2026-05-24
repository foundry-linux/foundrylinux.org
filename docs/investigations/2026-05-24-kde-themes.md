# KDE Desktop Theming — What a Foundry Linux Theme Would Take

## Summary

KDE theming is a stack of 11 independent layers. Foundry Linux already has 3 done (Plymouth, SDDM, GRUB). The remaining 8 cover the actual desktop shell — panels, windows, app widgets, icons. A complete theme based on Breeze Dark is roughly 3–4 weeks of design/config work. Color scheme + Plasma Style alone (the most visible parts) is about a week.

Kubuntu, notably, ships **no custom KDE theme** beyond wallpapers — Breeze Dark defaults are enough to feel like a distinct distro once the boot/login/installer experience is branded. Foundry Linux is already at that bar.

---

## What's Already Done

| Layer | Status | Package | Location |
|---|---|---|---|
| GRUB boot menu | ✓ done | `calamares-settings-foundry-linux` | `/usr/share/grub/themes/foundry-linux/` |
| Plymouth boot splash | ✓ done | `calamares-settings-foundry-linux` | `/usr/share/plymouth/themes/foundry-linux/` |
| SDDM login screen | ✓ done | `calamares-settings-foundry-linux` | `/usr/share/sddm/themes/foundry-linux/` |
| Wallpaper | ✓ done | `calamares-settings-foundry-linux` | `/usr/share/backgrounds/foundry-linux-wallpaper.png` |
| **Color Scheme** | **✓ done** | **`foundry-kde-theme`** | **`/usr/share/color-schemes/FoundryLinux.colors`** |

Established color palette (from `FoundryLinux.colors`):
- Background (window bg): `#0a0a0a`
- Surface-hi (raised widgets): `#141414`
- Widget bg (buttons/inputs): `#1a1a1a`
- Text (on-surface): `#f7f7f7`
- Muted (inactive/secondary): `#909090`
- Accent (forge orange): `#ff5b1a`
- Accent-dark (selection alt): `#c8410a`

---

## The Full Stack (8 layers still open)

### 1. Color Scheme — `FoundryLinux.colors`

**Controls:** The master palette consumed by every KDE/Qt app.

**Location:** `/usr/share/color-schemes/FoundryLinux.colors`

**Format:** INI file, ~40 named color definitions across groups:
```ini
[General]
ColorScheme=Foundry Linux
Contrast=4

[Colors:Window]
BackgroundNormal=#0a0a0a
ForegroundNormal=#f7f7f7

[Colors:Button]
BackgroundNormal=#1a1a1a
DecorationFocus=#ff5b1a
```

**Effort:** 1 day. Pure text, start from `BreezeDark.colors`.

---

### 2. Plasma Style (Desktop Shell Theme)

**Controls:** Panel backgrounds, widget popups, system tray, notifications, tooltips, progress bars — everything in the Plasma shell.

**Location:** `/usr/share/plasma/desktoptheme/FoundryLinux/`

**File structure:**
```
FoundryLinux/
├── metadata.desktop          name, inherits=breezedark
├── colors                    INI palette for shell elements
├── dialogs/
│   ├── background.svgz       popup/dialog backgrounds
│   └── tooltip.svgz
└── widgets/
    ├── panel-background.svgz panel bar
    ├── background.svgz       generic widget bg
    ├── button.svgz
    ├── scrollbar.svgz
    ├── slider.svgz
    ├── tab.svgz
    └── (15+ more .svgz files)
```

SVGs use named element IDs that Plasma maps to semantic roles (e.g. `#center`, `#top-left`, `#shadow`). Inherit from Breeze Dark and override only the files where color/shape differs.

**Effort:** 3–5 days. Mostly recolouring existing SVGs.

---

### 3. Look and Feel Package (Meta-Theme)

**Controls:** The master switch that applies all other layers at once via System Settings → Global Theme.

**Location:** `/usr/share/plasma/look-and-feel/org.foundrylinux.desktop/`

```
org.foundrylinux.desktop/
├── metadata.desktop
└── contents/
    ├── defaults              INI pointing at each sub-theme
    ├── plasmarc              shell behavior config
    └── layouts/
        └── org.kde.plasma.desktop-appletsrc   default panel layout
```

`contents/defaults`:
```ini
[kdeglobals]
ColorScheme=FoundryLinux
IconTheme=breeze-dark

[kwinrc]
[org.kde.kdecoration2]
library=org.kde.aurorae
theme=FoundryLinux

[plasmarc]
[Theme]
name=FoundryLinux
```

**Effort:** 1 day. Wires everything else together; no design work.

---

### 4. Window Decoration (Title Bars & Borders)

**Controls:** Window frame, title bar height/buttons/color, shadows, resize handles.

**Best approach: Aurorae** (SVG-based, no C++ required)

**Location:** `/usr/share/aurorae/themes/FoundryLinux/`

```
FoundryLinux/
├── metadata.desktop          border sizes, padding
├── decoration.svgz           SVG sprite: buttons + frame elements
└── FoundryLinuxrc            color overrides (optional)
```

`metadata.desktop`:
```ini
[Decoration]
BorderLeft=1
BorderRight=1
BorderBottom=4
TitleEdgeTop=4
ButtonWidth=18
ButtonHeight=18
```

Button elements in the SVG are named `close`, `maximize`, `minimize`, etc. with `:hover` and `:pressed` variants.

**Effort:** 2–4 days. Copy Breeze Dark Aurorae, recolour buttons and title bar.

---

### 5. Application Style (Qt Widget Rendering)

**Controls:** How buttons, menus, checkboxes, scrollbars, input fields render inside Qt applications (Dolphin, Konsole, KWrite, etc.).

**Best approach: Kvantum** (config-file based, no C++ required)

**Location:** `/usr/share/Kvantum/FoundryLinux/`

```
FoundryLinux/
├── FoundryLinux.kvconfig     color + shape overrides
└── FoundryLinux.svg          modified asset SVG (optional)
```

`FoundryLinux.kvconfig` overrides just the delta from KvGnomeDark or BreezeEvolution:
```ini
[GeneralColors]
window.color=#0a0a0a
base.color=#1a1a1a
highlight.color=#ff5b1a
```

Requires `kvantum` package; user sets it in System Settings → Application Style → Kvantum Manager.

**Effort:** 1–2 days. Config tweaks from an existing dark Kvantum theme.

---

### 6. GTK Theme (GTK2/3/4 Applications)

**Controls:** Blender, GIMP, Firefox, Audacity, and any other GTK app running under KDE.

**Location:** `/usr/share/themes/FoundryLinux/`

```
FoundryLinux/
├── gtk-3.0/
│   ├── gtk.css               full CSS stylesheet
│   ├── settings.ini
│   └── assets/               PNGs for borders, scrollbars, etc.
├── gtk-4.0/
│   ├── gtk.css
│   └── assets/
└── index.theme
```

CSS approach — inherit from Adwaita Dark, override colors:
```css
@define-color accent_color #ff5b1a;
@define-color window_bg_color #0a0a0a;
@define-color view_bg_color #1a1a1a;
```

GTK3 and GTK4 are separate stylesheets with different syntax.

**Effort:** 5–10 days. The most time-consuming component; GTK CSS has many edge cases.

---

### 7. Icon Theme

**Controls:** Every icon in every app and file manager.

**Location:** `/usr/share/icons/FoundryLinuxIcons/`

**Recommended approach: inherit Breeze Dark, add only what differs.**

```ini
# index.theme
[Icon Theme]
Name=Foundry Linux
Inherits=breeze-dark
```

Full custom icon sets run to 200+ icons at 6–8 sizes — 10+ days of design work. Inheriting from Breeze with targeted overrides (e.g. custom app icons) is 1 day.

**Effort:** 1 day (inherit) or 10+ days (custom icons from scratch).

---

### 8. Cursor Theme

**Controls:** All pointer states: arrow, link, text cursor, resize handles, wait spinner, etc.

**Location:** `/usr/share/icons/FoundryLinuxCursors/cursors/`

Cursor files are binary X11 format generated from source PNGs via `xcursorgen`. 40+ cursor types, some animated (e.g. `wait`).

**Recommended approach:** inherit Breeze or Breeze Light.

```ini
# index.theme
[Icon Theme]
Name=Foundry Linux Cursors
Inherits=breeze_cursors
```

**Effort:** 1 day (inherit). Custom cursors from scratch: 5–7 days.

---

## Packaging

One package: `foundry-kde-theme` in `foundry-apt/packages/foundry-kde-theme/`.

```
debian/
  control          Architecture: all; Depends: breeze, plasma-workspace
  install          maps data/ dirs → /usr/share/...
  postinst         kbuildsycoca6, gtk-update-icon-cache
```

Setting the Look and Feel as default for new users:

```bash
# /etc/skel/.config/plasmarc
[Theme]
name=FoundryLinux

# /etc/skel/.config/kdeglobals
[General]
ColorScheme=FoundryLinux
```

Written into `/etc/skel/` via the live-autologin hook (same pattern as the wallpaper autostart).

---

## Effort Summary

| Component | Effort | Approach |
|---|---|---|
| Color Scheme | 1 day | INI file from Breeze Dark |
| Look and Feel | 1 day | Metadata wiring |
| Plasma Style | 3–5 days | Recolour Breeze Dark SVGs |
| Aurorae (window decoration) | 2–4 days | Recolour Breeze Dark Aurorae SVGs |
| Kvantum (app style) | 1–2 days | Config overrides from existing dark theme |
| GTK Theme | 5–10 days | CSS overrides on Adwaita Dark |
| Icon Theme | 1 day | Inherit Breeze Dark |
| Cursor Theme | 1 day | Inherit Breeze cursors |
| **Total** | **~15–25 days** | |

---

## Recommended Sequencing

**Phase A — visible shell (≈1 week):**
Color Scheme → Plasma Style → Look and Feel + `/etc/skel` wiring

This alone replaces the blue Breeze Dark accent with Foundry orange across all panels and widgets. Most impactful single step.

**Phase B — window frames + Qt apps (≈1 week):**
Aurorae → Kvantum

**Phase C — GTK apps (≈1–2 weeks):**
GTK3/4 theme. Lower priority — game dev tools under Foundry Linux are mostly Qt-based.

**Phase D — icons/cursors (≈1 day):**
Inherit Breeze, ship a custom app icon for `foundry-welcome` and any Foundry-specific apps.
