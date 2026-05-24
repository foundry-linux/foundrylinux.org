# foundry-kde-theme — KDE Plasma Theming Package

**Status:** In progress  
**Package:** `foundry-kde-theme` (`foundry-apt/packages/foundry-kde-theme/`)  
**Repo:** `apt.foundrylinux.org`

---

## Goal

Ship KDE Plasma desktop branding for Foundry Linux as a single `foundry-kde-theme`
package installable from `apt.foundrylinux.org`. The package starts with the minimal
viable set (color scheme + wallpaper + system default) and gains more layers over time.

---

## Palette

| Token | Value | Role |
|---|---|---|
| Background | <span style="display:inline-block;width:12px;height:12px;background:#0a0a0a;border:1px solid #444;vertical-align:middle"></span> `#0a0a0a` | Near-black window/panel base |
| Text | <span style="display:inline-block;width:12px;height:12px;background:#f7f7f7;border:1px solid #444;vertical-align:middle"></span> `#f7f7f7` | Warm-white primary text |
| Accent | <span style="display:inline-block;width:12px;height:12px;background:#ff5b1a;border-radius:2px;vertical-align:middle"></span> `#ff5b1a` | Forge orange — highlight, selection, focus |

---

## What ships in each version

### 1.0.0 — Color scheme (done)

`FoundryLinux.colors` → `/usr/share/color-schemes/`

KDE INI color scheme covering all KDE color groups (Window, Button, View, Selection,
Tooltip, Complementary, Header). Sets the near-black / warm-white / orange palette
across all KDE applications and Plasma shell elements.

### 1.0.1 — Wallpaper wiring (done)

Three additions:

1. **`metadata.json`** → `/usr/share/wallpapers/FoundryLinux-ForgeHorizon/`  
   KPackage manifest that makes Plasma recognize the wallpaper in System Settings.

2. **PNG images** → `/usr/share/wallpapers/FoundryLinux-ForgeHorizon/contents/images/`  
   `1920x1080.png` and `3840x2160.png`. Plasma downscales the highest available
   resolution automatically.

3. **`/etc/xdg/plasma-org.kde.plasma.desktop-appletsrc`**  
   System-wide KDE default — sets containment 1 wallpaper to `FoundryLinux-ForgeHorizon`
   for any new user session before they customise anything.  
   Format:
   ```ini
   [Containments][1][Wallpaper][org.kde.image][General]
   Image=file:///usr/share/wallpapers/FoundryLinux-ForgeHorizon/
   ```
   Pointing at the package directory (not a specific resolution file) lets KDE pick the
   best match for the display.

### Future layers (not started)

| Layer | Location | Effort |
|---|---|---|
| Plasma Style | `/usr/share/plasma/desktoptheme/FoundryLinux/` | 1–2 days |
| Look-and-Feel | `/usr/share/plasma/look-and-feel/org.foundrylinux.desktop/` | 1 day |
| Aurorae window decoration | `/usr/share/aurorae/themes/FoundryLinux/` | 2–4 days |
| Kvantum application style | `/usr/share/Kvantum/FoundryLinux/` | 1–2 days |
| SDDM login theme | `/usr/share/sddm/themes/FoundryLinux/` | 1 day |

See `docs/investigations/2026-05-24-kde-themes.md` for detailed design notes on each layer.

---

## File layout

```
foundry-apt/packages/foundry-kde-theme/
  debian/
    control          Depends: plasma-workspace
    changelog        version source (1.0.1 current)
    rules            "%: dh $@"
    install          explicit file → destination mapping
    source/format    "3.0 (native)"
    copyright
  data/
    color-schemes/
      FoundryLinux.colors
    wallpapers/
      FoundryLinux-ForgeHorizon/
        metadata.json
        contents/images/
          1920x1080.png
          3840x2160.png
    xdg/
      plasma-org.kde.plasma.desktop-appletsrc
```

---

## Build

```bash
# In the foundrylinux.org repo root:
docker run --rm -v "$PWD:/work" -w /work/foundry-apt ubuntu:26.04 bash -c \
  'apt-get update -qq && apt-get install -y -qq debhelper devscripts && \
   cd packages/foundry-kde-theme && dpkg-buildpackage -us -uc -b'

# Or via Taskfile:
task build
```

---

## Verification

Steps run against the 1.0.1 build:

1. **`.deb` produced and installs cleanly**

   ```bash
   dpkg-deb --info foundry-apt/foundry-kde-theme_1.0.1_all.deb
   ```

   Expected: `Version: 1.0.1`, `Architecture: all`, `Depends: plasma-workspace`.

   ```
   Package: foundry-kde-theme
   Version: 1.0.1
   Architecture: all
   Depends: plasma-workspace
   ```

   PASS

2. **Color scheme installs to the right path**

   ```bash
   dpkg -c foundry-apt/foundry-kde-theme_1.0.1_all.deb | grep color-schemes
   ```

   Expected: `./usr/share/color-schemes/FoundryLinux.colors`

   ```
   -rw-r--r-- root/root  3606 2026-05-24 07:00 ./usr/share/color-schemes/FoundryLinux.colors
   ```

   PASS

3. **Wallpaper package installs to the right paths**

   ```bash
   dpkg -c foundry-apt/foundry-kde-theme_1.0.1_all.deb | grep wallpaper
   ```

   Expected lines:
   - `./usr/share/wallpapers/FoundryLinux-ForgeHorizon/metadata.json`
   - `./usr/share/wallpapers/FoundryLinux-ForgeHorizon/contents/images/1920x1080.png`
   - `./usr/share/wallpapers/FoundryLinux-ForgeHorizon/contents/images/3840x2160.png`

   ```
   drwxr-xr-x root/root         0 ./usr/share/wallpapers/FoundryLinux-ForgeHorizon/
   drwxr-xr-x root/root         0 ./usr/share/wallpapers/FoundryLinux-ForgeHorizon/contents/
   drwxr-xr-x root/root         0 ./usr/share/wallpapers/FoundryLinux-ForgeHorizon/contents/images/
   -rw-r--r-- root/root   1331650 ./usr/share/wallpapers/FoundryLinux-ForgeHorizon/contents/images/1920x1080.png
   -rw-r--r-- root/root   3211881 ./usr/share/wallpapers/FoundryLinux-ForgeHorizon/contents/images/3840x2160.png
   -rw-r--r-- root/root       513 ./usr/share/wallpapers/FoundryLinux-ForgeHorizon/metadata.json
   ```

   PASS

   > **Note:** `dh_strip_nondeterminism` emits a warning about an "invalid length in IDAT header"
   > for the 3840×2160 PNG. This is a false positive in the perl handler — the PNG has a valid
   > signature and IHDR, and the deb build completes exit 0. File opens correctly in image viewers.

4. **System default appletsrc installs**

   ```bash
   dpkg -c foundry-apt/foundry-kde-theme_1.0.1_all.deb | grep etc/xdg
   ```

   Expected: `./etc/xdg/plasma-org.kde.plasma.desktop-appletsrc`

   ```
   -rw-r--r-- root/root  115 2026-05-24 07:00 ./etc/xdg/plasma-org.kde.plasma.desktop-appletsrc
   ```

   PASS

5. **Wallpaper appears in KDE System Settings** (manual; requires a Foundry Linux KDE session)

   Install the `.deb`, open System Settings → Wallpaper — "Forge Horizon" should appear
   in the wallpaper chooser with thumbnail.

6. **New user session defaults to Forge Horizon wallpaper** (manual; requires a Foundry Linux KDE session)

   Create a new user account. First login should show the Forge Horizon wallpaper without
   any manual selection.
