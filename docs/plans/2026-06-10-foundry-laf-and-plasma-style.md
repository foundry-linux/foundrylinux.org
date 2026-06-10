# Foundry Linux: build out the Look-and-Feel package + a custom Plasma Style

## Context

The Foundry "global theme" — the Plasma **Look-and-Feel (LAF)** package `org.foundrylinux.foundry-linux` — is a **splash-only stub**. It ships just `metadata.json` + `contents/splash/Splash.qml` as a foundry-iso static include, has no `contents/defaults` and no `contents/layouts/`, and although it's already *activated* (hook `1150-kde-color-scheme.hook.chroot` writes `[KDE] LookAndFeelPackage=org.foundrylinux.foundry-linux` into `/etc/xdg/kdeglobals`), it does almost nothing beyond the splash. Theming is scattered across hooks (1150 color scheme, 1100 wallpaper autostart) and `/etc/xdg` static includes (kscreenlockerrc, ksplashrc) and the `foundry-kde-theme` package (wallpaper, color scheme, SDDM, avatar). Favorites are **inherited from Kubuntu's** `org.kubuntu.desktop.defaultPanel` template.

Will wants the LAF to be a real, coherent Foundry global theme — and (his explicit choice) a **custom Foundry Plasma Style** so the panel/widget *chrome* stops being Breeze. Two latent bugs surfaced during research: the `Splash.qml` **background AND logo paths are both dead on installed systems** (they're shipped only by `calamares-settings-foundry-linux`, which is purged with `calamares` on install — the same mechanism that broke the SDDM theme and the desktop wallpaper), and there's a **`ColorScheme` name mismatch** in-tree (`Foundry Linux` with a space vs `FoundryLinux` without).

**Hard lesson to honor throughout:** "config resolves correctly" ≠ "the surface renders it." The lock screen shipped a correct-looking config but rendered cones for two ISO builds because `WallpaperPlugin` was missing. **Every surface here is verified at the PIXEL level (boot + look), not by `kreadconfig6`/file-existence.**

## Decisions (settled with Will)

- **Scope:** LAF buildout **+ a custom Foundry Plasma Style**.
- **Splash logo:** reuse the existing anvil avatar (`/usr/share/foundry-linux/avatar.png`) — survives install, no new asset.
- **Favorites — BOTH surfaces** (per the 2026-06-10 plan's "likely both"): the **taskbar pins** (`systemsettings`, software center, Dolphin, Konsole, **Blender**, Firefox — Kate omitted) AND the **app-menu Favorites tab** (KActivities-stats DB). **TODO: add a code editor pin later.**
- **Consolidation:** **single package** — `foundry-kde-theme` becomes the *sole owner* of all desktop theming. The foundry-iso theming hooks/includes are **deleted**; the package ships the LAF, Plasma Style, color scheme + activation, wallpaper + its (load-bearing) autostart, `kscreenlockerrc`, `ksplashrc`. The handful of paths Kubuntu's `kubuntu-settings-desktop` also owns (notably `/etc/skel/.face`) use **`dpkg-divert`** in the package's maintainer scripts — still one package, no foundry-iso hook.

---

## Part 1 — Build out the LAF (move it into `foundry-kde-theme`)

The LAF belongs in the survives-install package, not a foundry-iso static include. Create the source tree under `foundry-apt/packages/foundry-kde-theme/data/look-and-feel/org.foundrylinux.foundry-linux/`:

- **`metadata.json`** — keep Id/Name/Version; **add `"X-Plasma-APIVersion": "2"`** (required for the layout.js globals like `applicationExists`/`loadTemplate`).
- **`contents/defaults`** (NEW) — INI mapping the theme surfaces:
  ```ini
  [kdeglobals][KDE]
  widgetStyle=Breeze
  [kdeglobals][General]
  ColorScheme=Foundry Linux          # WITH the space — matches FoundryLinux.colors [General] Name=
  [kdeglobals][Icons]
  Theme=breeze-dark
  [plasmarc][Theme]
  name=FoundryLinux                  # the custom Plasma Style from Part 2 (falls back to breeze-dark if absent)
  [kwinrc][org.kde.kdecoration2]
  library=org.kde.breeze
  theme=Breeze
  [KSplash]
  Theme=org.foundrylinux.foundry-linux
  [Wallpaper]
  Image=FoundryLinux-ForgeHorizon
  ```
- **`contents/layouts/org.kde.plasma.desktop-layout.js`** (NEW) — see Part 3.
- **`contents/splash/Splash.qml`** (MOVED) — fix BOTH dead paths to survives-install assets shipped by `foundry-kde-theme`:
  - background → `file:///usr/share/wallpapers/FoundryLinux-ForgeHorizon/contents/images/1920x1080.png`
  - logo → `file:///usr/share/foundry-linux/avatar.png` (the anvil)
- **`contents/previews/preview.png`** (NEW) — so "Foundry Linux" shows a thumbnail in System Settings → Global Theme.

Wire it in `debian/install` (all under `/usr/share/...` → no conffiles), bump `debian/changelog` to **1.0.4**, and **delete the foundry-iso static include** `foundry-iso/config/includes.chroot/usr/share/plasma/look-and-feel/org.foundrylinux.foundry-linux/` — but only AFTER confirming the rebuilt deb lands in the chroot (sequencing below), since the package installs to the identical path.

Also fix the in-tree `ColorScheme` mismatch: hook `1100`'s skel write uses `FoundryLinux` (no space) — correct it to `Foundry Linux`.

---

## Part 2 — Custom Foundry Plasma Style (`desktoptheme`)  ← the design-heavy, iterative part

Create `foundry-apt/packages/foundry-kde-theme/data/desktoptheme/FoundryLinux/` (`KPackageStructure: Plasma/Theme`), referenced from the LAF defaults (`[plasmarc][Theme] name=FoundryLinux`).

**Honest framing (verify before investing):** Plasma 6 Breeze sets `FollowsSystemColors`, so the panel/widgets **already render in the active color scheme** — i.e. Foundry's dark + `#ff5b1a`. The remaining "Breeze-ness" is **shape/texture, not color**. So:

- **Step 2a — VERIFY the current panel pixels first** (boot 0.9.107, screenshot the panel/system-tray/tooltips). This tells us how much of the gap is shape vs color, so the SVG effort goes where it changes something. (Don't assume "stock Breeze" — it isn't, color-wise.)
- **Step 2b — start minimal:** a `metadata.json` + a Foundry `colors` file (or `FollowsSystemColors`) + **inherit Breeze's SVGs**, overriding only the **highest-impact elements**: `widgets/panel-background.svgz` (the panel itself) first, then `widgets/background.svgz`, `widgets/tooltip.svgz`, the system-tray/clock as warranted. Recolor/restyle toward the forge palette (near-black panel, ember-orange accents/hovers).
- **Step 2c — iterate** on the rest of the SVGs in follow-on passes. A full from-scratch SVG set is explicitly NOT required for v1.

This part needs the actual Breeze `desktoptheme` SVGs to fork — extract them from the `plasma-workspace`/Breeze package during implementation (the chroot was cleaned, so they aren't in-tree right now).

---

## Part 3 — Favorites (panel layout)

Inline the panel build in `contents/layouts/org.kde.plasma.desktop-layout.js` (do NOT ship a separate `org.foundrylinux.defaultPanel` template — one Foundry-owned file, no second KPackage to keep in sync). Mirror Kubuntu's `defaultPanel` logic (kickoff + pager + icontasks + systemtray + clock + showdesktop), set the wallpaper inline (ForgeHorizon), and set the `launchers` list:

```js
const launcher_list = [
  'applications:systemsettings.desktop',
  applicationExists('kubuntu-manage-software')
    ? 'applications:org.kubuntu.manage-software.desktop'
    : 'applications:org.kde.discover.desktop',
  'applications:org.kde.dolphin.desktop',
  'applications:org.kde.konsole.desktop',
  'applications:blender.desktop',   // ADD (blender 5.0.1, ships blender.desktop, in anvil)
  'preferred://browser'
];
```

- **Kate** is not in Kubuntu's template, so an explicit Foundry list simply never includes it — nothing to subtract.

**App-menu "Favorites" tab (IN scope — the second favorites backend).** The Kickoff "Favorites" are stored in the **KActivities-stats SQLite DB** (`~/.local/share/kactivitymanagerd/resources/database`), NOT in `launchers=`. This is the genuinely finicky surface; the exact mechanism must be verified during implementation. Two candidate approaches:
  1. **Pre-port a `favorites=` list** — set the Kickoff applet's `favorites=` (the pre-KAStats list format) in the shipped appletsrc/layout WITHOUT `favoritesPortedToKAstats=true`, so Plasma migrates our list into the KAStats DB on first run. Cleaner if Plasma honors it.
  2. **Seed the DB directly** — a first-login script writes the desired favorites rows into the SQLite DB. Brittle fallback if (1) fails.
  Define the app-menu favorites set (the core taskbar apps + the key game-dev/editor apps — typically fuller than the pins). **Verify at the PIXEL level** (open the app menu → Favorites tab shows the Foundry apps) — "config resolves ≠ rendered" applies doubly to a DB-backed surface. This finishes the 2026-06-10 favorites plan's open half rather than deferring it.

---

## Part 4 — Single-package consolidation (foundry-kde-theme owns ALL desktop theming)

One source of truth, not theming scattered across foundry-iso hooks + static includes. Re-home every desktop-theming surface into `foundry-kde-theme` and **delete the foundry-iso theming hooks/includes**. Re-home the *proven-working mechanism* for each surface (not just LAF defaults) — several surfaces don't render from defaults alone:

| Surface | Today (scattered) | Re-homed into foundry-kde-theme as |
|---|---|---|
| LAF (defaults/layouts/splash/previews) | foundry-iso static include | package files under `/usr/share/plasma/look-and-feel/` (Part 1) |
| Plasma Style | — (new) | package files under `/usr/share/plasma/desktoptheme/` (Part 2) |
| Color scheme + **LAF activation** | hook 1150 writes `/etc/xdg/kdeglobals` | package ships `/etc/xdg/kdeglobals` (`ColorScheme=Foundry Linux`, `LookAndFeelPackage=org.foundrylinux.foundry-linux`) — verify unowned |
| Desktop wallpaper | hook 1100 autostart + appletsrc | package ships the appletsrc **and** the per-user autostart (`/etc/xdg/autostart/foundry-set-wallpaper.desktop` + `/usr/bin/foundry-set-wallpaper.sh`) — the autostart is load-bearing, and is simply a package file |
| Lock screen | foundry-iso `kscreenlockerrc` include | package ships `/etc/xdg/kscreenlockerrc` (WallpaperPlugin + ForgeHorizon) — unowned |
| Splash selection | foundry-iso `ksplashrc` include | package ships `/etc/xdg/ksplashrc` — unowned |
| Avatar | hook 1100 → `/etc/skel/.face` | **dpkg-divert** in postinst (kubuntu-settings-desktop owns `/etc/skel/.face`), then ship Foundry's |
| SDDM theme | already in package | unchanged |

**Deletions in foundry-iso:** hook `1150-kde-color-scheme.hook.chroot` (theming-only → gone), the `kscreenlockerrc` + `ksplashrc` static includes, the LAF static-include dir. Hook `1100` **keeps** its non-theming logic (autologin, the QEMU virtio EDID/display config) but **loses** its theming sed-loops (avatar/wallpaper/color-scheme) — those become the package's job.

**Why this is safe despite the wallpaper fragility:** the wallpaper needed a per-user autostart because theme *defaults* were unreliable. That autostart is a `.desktop` + script — shipping it from the package works identically. No mechanism is lost; it's relocated, not removed.

**Kubuntu-owned-file audit (do this FIRST):** dpkg forbids two packages owning a path. Run `dpkg -S` / inspect `kubuntu-settings-desktop`'s file list for every target path: at minimum `/etc/skel/.face`; check `/etc/xdg/kdeglobals`, `/etc/xdg/kscreenlockerrc`, `/etc/xdg/ksplashrc`, `/etc/skel/.config/*`, the autostart dir. For each genuine conflict, `dpkg-divert --add --rename --divert <path>.kubuntu <path>` in `postinst` (+ undivert in `postrm`). Everything unowned ships as a plain package file. Test install + remove + **upgrade** (the divert must survive a kubuntu-settings upgrade).

---

## Biggest open risk — does the LAF layout actually run on first login?

The 2026-06-10 live read found **no `LookAndFeelPackage` in the running `~/.config/kdeglobals`** (compiled-in default LAF), implying the active LAF on a real session may not be ours despite hook 1150 writing `/etc/xdg`. The favorites (Part 3) only apply if the Foundry LAF's layout.js executes on first login. **Verify empirically** (does the inline wallpaper write / pin list actually appear on a fresh login?). **Fallback:** if it doesn't, ship a full captured `/etc/xdg/plasma-org.kde.plasma.desktop-appletsrc` with the explicit `launchers=` for the favorites specifically (2026-06-10 plan's Option B).

**Consolidation may *fix* this rather than worsen it:** the "no LookAndFeelPackage" symptom may be *because* hook 1150 (file mode 0644, not executable) wasn't actually running, leaving activation unset. Shipping `/etc/xdg/kdeglobals` as a reliable *package* file removes that fragility. Either way, **pixel + `kreadconfig6`-verify the LAF is the active global theme on a fresh first login before trusting the package for favorites.**

---

## Build sequencing

1. Make all `foundry-kde-theme` source changes (Parts 1–3). Do NOT delete the static include yet.
2. `cd foundry-apt && bash scripts/build-all.sh`; `lintian` the new `dist/foundry-kde-theme_1.0.4_all.deb` (clear new tags); `dpkg-deb -c` to confirm the LAF + desktoptheme files are present.
3. `task iso-sync-local-debs` shadows the published deb with the local one (always-local applies; cf. the publish-aware staging).
4. **Now** delete the foundry-iso theming hooks/includes (Part 4: hook 1150, the `kscreenlockerrc`/`ksplashrc` includes, the LAF include) and trim hook 1100's theming sed-loops — confirming each re-homed surface is provided by the package in the chroot first.
5. `task iso-bump` → `task iso-build EDITION=anvil` (Blender is anvil). Test ISO 0.9.x.

---

## Verification (PIXEL level — boot live AND installed)

1. **Splash** (the headline fix) — on an **installed** boot: Foundry splash renders WITH the ForgeHorizon background AND the anvil logo (not bare dots on black). Necessary-but-not-sufficient: `kreadconfig6 --file ksplashrc --group KSplash --key Theme` → `org.foundrylinux.foundry-linux`. **Eyeball it.**
2. **Desktop wallpaper** — ForgeHorizon after first login (confirms the autostart still wins).
3. **Lock screen** — lock → ForgeHorizon (regression check on the 0.9.107 fix).
4. **Color scheme** — KDE app + System Settings show near-black/warm-white/`#ff5b1a`; `ColorScheme` resolves to `Foundry Linux`.
5. **Favorites — BOTH surfaces** — fresh first login: **taskbar pins** show Blender present, Kate absent + the rest (the Blender pin LAUNCHES Blender); **app menu → Favorites tab** shows the Foundry app-menu favorites (the KAStats migration/seed worked). Test a clean user — favorites only apply to fresh profiles.
6. **Plasma Style** — the panel/system-tray/tooltips show the Foundry style (compare to the Step-2a baseline screenshot); "Foundry Linux" Plasma Style selectable in System Settings.
7. **Global Theme** — System Settings → Appearance → Global Theme lists "Foundry Linux" with a preview thumbnail.
8. **Package + divert** — `lintian` clean; installs with no conffile prompts; the `dpkg-divert` paths survive **install → upgrade `kubuntu-settings-desktop` → remove `foundry-kde-theme`** (undivert cleanly restores Kubuntu's file; no orphaned diversions). Confirm no foundry-iso theming hooks/includes remain (grep the deleted paths).

---

## Follow-ups / TODO (to record in TODO.md)

- **Code editor pin** in the favorites (Will wants one added later; pick the editor — VS Code if/when shipped, or another).
- **Plasma Style SVG iteration** — beyond the v1 panel-background/high-impact pass.
- **v2 consolidation** — trim redundant `/etc/xdg/ksplashrc` + hook-1100 LAF-patch loop once pixel-proven.
- This plan supersedes/absorbs the 2026-06-10 favorites plan's "Option A" and the 2026-05-24 "deeper theming layers" LAF/Plasma-Style rows. Create `docs/plans/2026-06-10-foundry-laf-and-plasma-style.md` from this on implementation.

## Critical files

**foundry-kde-theme (the single source of truth):**
- `debian/install` — add ALL re-homed paths: the LAF, the desktoptheme, `/etc/xdg/{kdeglobals,kscreenlockerrc,ksplashrc}`, the wallpaper autostart `.desktop` + script, the avatar
- `debian/postinst` + `debian/postrm` (NEW) — `dpkg-divert` the Kubuntu-owned paths (≥ `/etc/skel/.face`; per the audit) then install Foundry's; undivert on removal
- `data/look-and-feel/org.foundrylinux.foundry-linux/contents/{defaults,layouts/org.kde.plasma.desktop-layout.js,splash/Splash.qml,previews/preview.png}` — new/moved (Part 1)
- `data/desktoptheme/FoundryLinux/` — new Plasma Style (Part 2)
- `data/xdg/{kdeglobals,kscreenlockerrc,ksplashrc}`, `data/autostart/` (wallpaper) — re-homed from foundry-iso
- `debian/changelog` — bump 1.0.4

**foundry-iso (deletions + trims, AFTER the deb is confirmed in the chroot):**
- DELETE hook `1150-kde-color-scheme.hook.chroot`, the `kscreenlockerrc` + `ksplashrc` static includes, and the LAF static-include dir
- `config/hooks/1100-live-autologin.hook.chroot` — REMOVE the theming sed-loops (avatar/wallpaper/color-scheme — incl. the `FoundryLinux`→`Foundry Linux` typo line); KEEP autologin + the QEMU virtio EDID/display config

---

## Verification Results — 0.9.110 anvil live boot (2026-06-11)

Built `foundry-anvil-0.9.110-amd64.iso`, booted headless in QEMU, SSH-corroborated (root:foundry) + screenshotted via the QEMU monitor.

| # | Surface | Result | Evidence |
|---|---|---|---|
| 1 | Splash | ✅ config | `ksplashrc Theme=org.foundrylinux.foundry-linux`; both dead `Splash.qml` paths now point at survives-install assets (boot-brief, not screenshotted) |
| 2 | Desktop wallpaper | ✅ **pixel** | ForgeHorizon forge-glow renders on the live desktop |
| 3 | Lock screen | ✅ **pixel** | locked the session → ForgeHorizon, **not** cones; the 0.9.107 `WallpaperPlugin` fix holds |
| 4 | Color scheme | ✅ **pixel** + config | dark/ember UI; `ColorScheme=Foundry Linux` + applied hash in the user config |
| 5 | Favorites (both) | ✅ **pixel** + config | **the LAF layout RAN** — user appletsrc `launchers=…,blender.desktop,preferred://browser`; kicker `Prepend=…;blender.desktop;…` (Kate out). Blender icon visible in the taskbar. |
| 6 | Plasma Style | ✅ config | `FoundryLinux` desktoptheme referenced by the LAF defaults; panel renders dark (distinctive-shape SVGs are the follow-up) |
| 7 | Global Theme entry | ✅ config | LAF installed; `LookAndFeelPackage=org.foundrylinux.foundry-linux` active |
| 8 | Package + divert | ✅ | build clean (no conffile prompt after the divert-and-replace fix); both diversions active in the booted system; avatar (anvil, 15786 B) inherited to `/home/user/.face` |

**Biggest risk RESOLVED:** the LAF activates and its `layout.js` executes on first login (the Foundry `launchers=` is present in the user's appletsrc), so the taskbar favorites apply — the consolidation fixed it (a reliable `/etc/xdg/kdeglobals` package file instead of the old non-executable hook 1150). The full-appletsrc fallback (Option B) was not needed.
