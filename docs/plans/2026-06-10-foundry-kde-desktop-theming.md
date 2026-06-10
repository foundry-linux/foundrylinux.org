# Foundry-owned KDE desktop theming — 2026-06-10

Consolidate the Foundry KDE desktop defaults (panel favorites, wallpaper, login greeter, avatar,
colors, splash) into **`foundry-kde-theme`** — our survives-install package — instead of the
current patchwork of build-time hook-patches against Kubuntu files. Immediate trigger: Will wants
to change the taskbar favorites (**remove Kate, add Blender**) and asked that it live in our theme.

---

## Why

Today the desktop look is assembled from three different places:

| Surface | Where it's set now | In our package? |
|---------|--------------------|-----------------|
| Desktop wallpaper | `foundry-kde-theme` appletsrc + hook 1100 LAF patch | partly |
| SDDM login greeter | `foundry-kde-theme/data/sddm-theme/` | ✅ yes |
| User avatar | `foundry-kde-theme/data/avatar.png` + hook 1100 → `/etc/skel/.face` | ✅ yes |
| Color scheme | `foundry-kde-theme` + hook 1100 → `/etc/skel/.config/kdeglobals` | partly |
| **Panel favorites** | **Kubuntu's `org.kubuntu.desktop.defaultPanel` layout template** | ❌ inherited |
| Plymouth / GRUB | `calamares-settings` (installer-only) | n/a (boot) |

The favorites are the first thing we want to change that we *don't* own at all. Rather than
hook-patch Kubuntu's template, move the default to a Foundry-owned mechanism.

---

## How panel favorites actually work (investigated 2026-06-10)

1. The active **Look-and-Feel** runs `contents/layouts/org.kde.plasma.desktop-layout.js` on a
   user's **first login** to build the default panel.
2. Kubuntu's LAF JS just calls `loadTemplate('org.kubuntu.desktop.defaultPanel')` (+ sets the
   Kubuntu wallpaper inline).
3. That **layout template** (`/usr/share/plasma/layout-templates/org.kubuntu.desktop.defaultPanel/contents/layout.js`,
   shipped by `kubuntu-settings-desktop`) builds the panel and writes the Task Manager pins:

   ```js
   const icontasks_obj = arg_panel_obj.addWidget( 'org.kde.plasma.icontasks' );
   const launcher_list = [
     'applications:systemsettings.desktop',
     app_software_str,                         // org.kubuntu.manage-software.desktop | org.kde.discover.desktop
     'applications:org.kde.dolphin.desktop',
     'applications:org.kde.konsole.desktop',
     'preferred://browser'
   ];
   icontasks_obj.writeConfig( 'launchers', launcher_list );
   ```

**Note — Kate is NOT in the Kubuntu default list.** And on a live 0.9.103 session (read via
SSH 2026-06-10) the running config is *not* Kubuntu's template at all:

- `~/.config/kdeglobals` has **no `LookAndFeelPackage`** (compiled-in default LAF, not org.kubuntu).
- The icontasks applet in `plasma-org.kde.plasma.desktop-appletsrc` has **no `launchers=` line**
  → the taskbar pins are the **applet's compiled-in defaults**, not the Kubuntu template's list.
- The kickoff applet has **`favoritesPortedToKAstats=true`** → the **app-menu "Favorites" tab is
  stored in a KActivities-stats SQLite DB** (`~/.local/share/kactivitymanagerd/resources/database`),
  not in any config file.

**So "favorites" is two separate backends:**

| Surface | Stored in | To set a distro default |
|---------|-----------|-------------------------|
| Taskbar pins | icontasks `launchers=` in the desktop appletsrc | ship a default appletsrc / panel layout with the `launchers` list |
| App-menu Favorites tab | KActivities-stats **SQLite DB** (`favoritesPortedToKAstats`) | seed the DB, or pre-port a `favorites=` list before KAStats migration — finicky |

**Implication:** we do **not** need to reverse-engineer where the current "Kate" comes from — for a
distro we just **define** the Foundry set and ship it. But which surface Will means matters,
because the taskbar-pins path is straightforward (config) and the menu-favorites path is a DB seed.
**Open: which surface?** (app-menu Favorites tab vs taskbar pins — likely both.)

Favorites only apply to a **fresh** user session (first login); existing profiles keep their panel.

---

## Options to own the favorites (and the rest) in foundry-kde-theme

### A. Foundry Look-and-Feel (recommended direction)

Build out the existing stub LAF `org.foundrylinux.foundry-linux` (currently only ships a
`Splash.qml`) into a complete LAF shipped by `foundry-kde-theme`:

- `contents/defaults` — color scheme, icon theme, wallpaper, splash, widget style
- `contents/layouts/org.kde.plasma.desktop-layout.js` — loads a **Foundry** panel template
- a Foundry panel template `org.foundrylinux.defaultPanel` (a copy of Kubuntu's with the Foundry
  `launcher_list`: drop `org.kde.kate.desktop`, add `applications:blender.desktop`)

Then set this LAF as the **system default** (`/etc/xdg/kdeglobals` `[KDE] LookAndFeelPackage=…`,
or `kde-config-look-and-feel-default`). One package owns the entire desktop default; nothing
patches Kubuntu.

- **Pro:** the clean, "in our theme" answer; consolidates wallpaper + colors + favorites + splash.
- **Con:** biggest change; a LAF sets up a lot (must replicate what Kubuntu's LAF provides) and
  switching the active LAF risks regressing surfaces we've already fixed. Needs careful testing.

### B. Ship a complete default appletsrc in foundry-kde-theme (lighter)

`foundry-kde-theme` already ships `/etc/xdg/plasma-org.kde.plasma.desktop-appletsrc` (currently
just the wallpaper `Image=`). Extend it to a **full** captured panel (containments for desktop +
panel + the icontasks `launchers` with the Foundry set + wallpaper).

- **Pro:** stays in our package; no new LAF.
- **Con / open question:** does a present `/etc/xdg` appletsrc actually **seed new users instead
  of** the LAF running its template? Plasma's precedence here is unverified — must test. The
  appletsrc must be captured from a real, fully-configured Foundry session (containment IDs,
  screen mapping) or it won't apply cleanly.

### C. Foundry panel template + minimal LAF JS swap (middle)

Ship `org.foundrylinux.defaultPanel` (customised template) in foundry-kde-theme, and a Foundry LAF
whose *only* job is a layout JS that `loadTemplate('org.foundrylinux.defaultPanel')` + reuses
Kubuntu's defaults for everything else. Smaller than a full LAF, still ours.

**Recommendation:** start by confirming the real current favorites + active LAF (open item above),
then prototype **B** (cheapest if Plasma honours the system appletsrc); fall back to **C/A** if it
doesn't. Decide before implementing.

---

## The favorites change itself

Target `launcher_list` (whichever mechanism wins):

```
applications:systemsettings.desktop
applications:org.kde.discover.desktop      (or org.kubuntu.manage-software.desktop)
applications:org.kde.dolphin.desktop
applications:org.kde.konsole.desktop
applications:blender.desktop               ← ADD (blender 5.0.1, ships blender.desktop, in anvil)
preferred://browser
# org.kde.kate.desktop                     ← REMOVE (confirm it's actually pinned first)
```

Blender's id confirmed from the cached deb: `/usr/share/applications/blender.desktop` →
`applications:blender.desktop`.

---

## Risks / open questions

1. **Where does Kate come from?** Kubuntu's default list doesn't include it — confirm the active
   LAF + real `launchers=` from a running Foundry install before assuming the mechanism.
2. **LAF template vs system appletsrc precedence** — the crux of option B; verify empirically.
3. **First-login only** — favorites won't change for existing profiles; fine for a fresh distro.
4. **Switching active LAF (option A)** could regress the wallpaper/colors/greeter we've already
   landed — regression-test all of them if we go that route.

---

## Verification

1. Fresh install → taskbar pins = the Foundry set (Blender present, Kate absent).
2. Live session → same pins.
3. Wallpaper, login greeter, avatar, colors all still correct (no regression from the new mechanism).
4. `blender.desktop` pin launches Blender (not a dead icon).

---

## Status

- Mechanism investigated (above). Approach **not yet chosen** — pending the "real current
  favorites / active LAF" read and the appletsrc-precedence test.
- Unrelated in-flight: native 1600×400 welcome banner committed (calamares-settings 1.0.28), ISO
  rebuild pending; SDDM login could later use the metallic wordmark in place of the flat text.
