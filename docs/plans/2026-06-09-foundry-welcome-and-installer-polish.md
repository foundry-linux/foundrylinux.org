# foundry-welcome + Calamares installer polish — 2026-06-09

Polish pass after first successful install test (ISO 0.9.82). Covers foundry-welcome UX fixes,
Calamares slideshow text clipping (5-attempt debug), live/installed wallpaper, and Plymouth messages.

---

## Items

| # | Status | Issue | Package / hook |
|---|--------|-------|---------------|
| 1 | ✅ done | foundry-welcome won't re-launch explicitly | foundry-welcome 1.0.7 |
| 2 | ✅ done | foundrylinux.org link: no hover cursor, not browser-like | foundry-welcome 1.0.7 |
| 3 | ✅ done | No icon in app menu (`Icon=foundry-linux` resolves nothing) | foundry-welcome 1.0.7 |
| 4 | ✅ done | Installer desktop icon shows generic Calamares icon | hook 1110 |
| 5 | ✅ done | Plymouth shutdown messages invisible | calamares-settings 1.0.19 |
| 6 | ✅ done | Carousel heading clipped on left — **6 attempts; takes 1–5 fixed the wrong element** (clipped text baked into the slide PNG). **Verified good by Will (0.9.96).** | calamares-settings 1.0.22 |
| 7 | ✅ done | Wallpaper reverts to Kubuntu default on live session | hook 1100 |
| 8 | ✅ done | **Desktop** wallpaper on installed first boot — ForgeHorizon path. **Verified via Image #2 (0.9.96): installed desktop shows ForgeHorizon, not cones.** The earlier "cones" was the *login screen* (item 11), not the desktop. | hook 1100 (ForgeHorizon path) |
| 9 | ✅ done | `kconf_update` calendar-migration crash popup on first login | hook 1100 (patch upstream script) |
| 10 | ✅ done | Build log not captured / build warnings unreviewed | Taskfile `iso-build` tees to `dist/build-*.log`; Plymouth hook 1050 cleaned |
| 11 | 🔄 verify | **SDDM login screen** shows cones on installed system — foundry SDDM theme purged with calamares-settings (same family as item 8). Move theme → foundry-kde-theme (survives install). | foundry-kde-theme 1.0.2 + calamares-settings 1.0.23 |
| 12 | 👀 monitor | Autologin intermittently prompts on installed first boot — worked on 2nd boot ("didn't have to do anything"). Likely `Relogin=false` (20-kubuntu.conf): autologin fires once per boot, a bounced first session strands you on the greeter. Not reliably reproducible; watch. | — |
| 13 | 🔄 verify | Swap chooser dropdown appeared in partition page despite `userSwapChoices: []`. Hide it + keep no-swap default. **See [Swap / hibernate research](#swap--hibernate-research).** | calamares-settings 1.0.24 |

---

## Fixes

### 1 — foundry-welcome re-launch (`--autostart` flag gate)

**Root cause**: sentinel (`~/.config/foundry-welcome-shown`) was checked unconditionally on
every launch, so the app exited immediately on any invocation after first run.

**Fix**: parse `--autostart` from `argv` in `main.cpp`. Only check the sentinel when that flag
is present. Direct launches (app menu, terminal) bypass the sentinel and always show the window.
XDG autostart `.desktop` updated: `Exec=foundry-welcome --autostart`.

Sentinel write policy: written on `Qt.quit()` regardless of launch path. Shows on each live
login (skel recreated → no sentinel), shows exactly once on installed system, re-launchable
from app menu at any time.

### 2 — foundrylinux.org link hover cursor

**Fix**: replaced `QQC2.Label` rich-text link + `HoverHandler` with a plain `Text` item:
`color: Kirigami.Theme.linkColor`, `font.underline: true`,
`MouseArea { cursorShape: Qt.PointingHandCursor; onClicked: Qt.openUrlExternally(...) }`.

### 3 — App menu icon

**Fix**: ship `foundry-linux.png` (256×256 anvil) inside the package; install to
`/usr/share/icons/hicolor/256x256/apps/` via `CMakeLists.txt`.

### 4 — Installer desktop icon (glowing anvil)

Hook 1110 copies `logo.png` from the calamares-settings branding dir to
`/usr/share/pixmaps/foundry-linux-installer.png`. `.desktop` uses `Icon=foundry-linux-installer`.
`shellprocess.conf` removes the installer icon from the installed target before `users` runs.

### 5 — Plymouth messages

Plymouth `script` module silently drops all messages unless `Plymouth.SetMessageFunction(callback)`
is registered. Added `message_callback(text)` → `Image.Text` sprite centred horizontally at 82%
screen height. Registered with `Plymouth.SetMessageFunction`. "Remove the media and press enter
to continue" now visible on shutdown screen.

---

## Carousel text clipping — failure log

**The actual clipped text was baked into the slide PNGs, not rendered by QML.** Takes 1–5 all
edited a QML `Text` caption that overlaid the *same* heading at the bottom of the slide — a real
Qt Quick anchor quirk, but on the wrong element. The heading that visibly clipped
("Where"→"Vhere") is typeset into the 800×440 slide artwork ~32 px from the left, and
`fillMode: Image.PreserveAspectCrop` cropped ~77 px off each side when the slide was scaled to
fill the taller installer pane.

| Attempt | Version | Approach | Why it failed |
|---------|---------|----------|---------------|
| 1 | 1.0.17 | `anchors.leftMargin/rightMargin` on Column + children `width: parent.width` | Wrong element (QML caption, not the clipped PNG text). Also circular implicit-width → Column at x=0. |
| 2 | 1.0.18 | `x: 24; width: parent.width - 48; anchors.verticalCenter` on Column | Wrong element. Qt Quick ignores `x`/`width` when an anchor is set; Column at x=0. |
| 3 | 1.0.18 | `Item { anchors.fill + leftMargin/rightMargin }` wrapping Column | Wrong element. Still x=0 in Calamares host. |
| 4 | 1.0.20 | Pure-anchor Column: `anchors.left + leftMargin + anchors.right + rightMargin + anchors.verticalCenter` | Wrong element. All-anchor, still x=0 — confirms the anchor quirk on `Column`/positioners in `ListView` delegates, but the caption was never the clipped text. |
| 5 | 1.0.21 | `x: 24; y: Math.round((parent.height-height)/2); width: parent.width-48` — NO anchors on Column | **Did** correctly position the QML caption (ISO 0.9.90/0.9.91) — but the *baked PNG heading* still clipped, so the screen looked unchanged. |
| **6** | **1.0.22** | **`Image.PreserveAspectFit` + dark backdrop; delete the redundant QML caption** | **Whole slide shown, nothing cropped; the PNG's own heading is the only text. ISO 0.9.96.** |

**Root cause**: `fillMode: Image.PreserveAspectCrop` on the slide `Image`. The slides are 1.818:1;
the installer pane is taller, so Crop scaled to fill the height and cropped the width, eating the
near-left-edge heading baked into each PNG. The five QML-caption patches chased an unrelated (if
genuine) anchor quirk in Calamares's `QQuickWidget` host. **Lesson logged in the howto**: when
text is "clipped", first decide whether it is QML text or raster-baked — `PreserveAspectCrop`
crops the long axis and eats near-edge content.

---

## Wallpaper

### Live session (DONE)

1. `sleep 5` — rejected (no bare sleep fixes).
2. Poll for plasmashell DBus registration — fires too early; LAF overwrites afterward.
3. Poll for `[Containments]` in appletsrc — confirmed working on live ISO (0.9.88+).

**Active fix**: autostart script polls every second until `plasma-apply-wallpaperimage` exits 0.

### Installed system first boot — **real root cause found in 0.9.96 (the "cones" bug)**

**Symptom**: installed first boot shows the Kubuntu 26.04 "cones" wallpaper; live is fine.

**Real root cause** (from `build.log`): every wallpaper lever pointed at
`/usr/share/sddm/themes/foundry-linux/background.png`, shipped by
**`calamares-settings-foundry-linux`**, which `Depends: calamares`. Calamares's
`modules/packages.conf` runs `remove: calamares` on the installed target; apt drops the
reverse-dependency too, **deleting the SDDM theme + background.png**. The installed system's
wallpaper config then referenced a dead path → Plasma fell back to the Kubuntu default. Live
worked only because the installer (and its background.png) is still present there.

The earlier theories (DBus-timing, `[Containments]` poll, LAF `kconf_update` ordering, the
skel-vs-system-wide autostart) were all secondary — the wallpaper image file simply wasn't on
the installed system. Full write-up:
[`docs/investigations/2026-06-09-installed-wallpaper-reverts-to-kubuntu-cones.md`](../investigations/2026-06-09-installed-wallpaper-reverts-to-kubuntu-cones.md).

**Fix** (hook 1100, ISO 0.9.96): point all three levers at the wallpaper shipped by
**`foundry-kde-theme`** — `/usr/share/wallpapers/FoundryLinux-ForgeHorizon/` — which is a normal
edition package and is **not** purged on install:

1. LAF `defaults` (6 files) → `Image=file:///usr/share/wallpapers/FoundryLinux-ForgeHorizon/`
2. System appletsrc (`/etc/xdg/plasma-org.kde.plasma.desktop-appletsrc`, shipped by
   foundry-kde-theme) → re-asserted to the same path
3. System-wide autostart `plasma-apply-wallpaperimage <ForgeHorizon>` (load-bearing lever),
   sentinel-gated, polling ≤60 s

**Lesson**: any path the *installed* system references must come from a package that survives the
install — never `calamares-settings-*` (installer-only, purged with `calamares`).

---

## Files changed

```
foundry-apt/packages/calamares-settings-foundry-linux/
  data/branding/foundry-linux/slideshow.qml        — carousel: PreserveAspectFit + drop caption (take 6 → 1.0.22)
  data/config/plymouth/foundry-linux.script        — Plymouth message callback (1.0.19)
  data/config/shellprocess.conf                    — remove installer icon from target
  debian/changelog                                 — 1.0.17 → 1.0.22

foundry-iso/config/hooks/
  1050-plymouth-theme.hook.chroot                  — use update-alternatives (no more "command not found")
  1100-live-autologin.hook.chroot                  — wallpaper → ForgeHorizon (survives install); kconf_update patch

Taskfile.yml                                       — iso-build tees build to dist/build-<edition>-<version>.log
docs/investigations/2026-06-09-kconf-update-calendar-crash.md                    (new)
docs/investigations/2026-06-09-installed-wallpaper-reverts-to-kubuntu-cones.md   (new)

foundry-apt/packages/foundry-welcome/
  src/main.cpp                                     — --autostart flag gate
  src/qml/pages/IntroPage.qml                      — browser-like link
  data/autostart.desktop                           — Exec=foundry-welcome --autostart
  data/foundry-linux.png                           — icon asset (new)
  CMakeLists.txt                                   — install icon to hicolor
  debian/changelog                                 — 1.0.5 → 1.0.7

foundry-iso/config/hooks/
  1100-live-autologin.hook.chroot                  — wallpaper LAF patch + improved autostart
  1110-live-install-button.hook.chroot             — anvil icon for installer desktop file

foundry-iso/docs/howto-kubuntu-remix.md            — pitfalls updated (wallpaper, carousel)
```

---

## ISO version history

| ISO | Change |
|-----|--------|
| 0.9.82 | baseline — first successful grub install |
| 0.9.83–0.9.87 | grub, unpackfsc, mount.conf fixes |
| 0.9.88 | wallpaper [Containments] poll; carousel attempts 2–3; Plymouth messages; foundry-welcome 1.0.7 |
| 0.9.89 | wallpaper LAF patch (6 files); carousel attempt 4 (pure anchors — still broken) |
| 0.9.90 | carousel attempt 5 (pure positional x/y/width) — fixed the QML caption, PNG still clipped |
| 0.9.91–0.9.95 | system-wide wallpaper autostart + sentinel; kconf_update crash investigated |
| 0.9.96 | carousel take 6 (PreserveAspectFit, drop caption) — **verified**; desktop wallpaper → ForgeHorizon — **verified (Image #2)**; kconf_update patch; plymouth hook cleanup; build-log capture |
| **0.9.97** | **SDDM login-screen theme → foundry-kde-theme (survives install; fixes login-screen cones); calamares-settings 1.0.23 stops shipping it** |

---

## Login screen (SDDM) theme — installed system

**Symptom**: after install, the SDDM **login screen** shows the Kubuntu cones background
(only seen when autologin doesn't fire — see item 12). The installed **desktop** is correct
(ForgeHorizon, item 8).

**Root cause** (same family as item 8): the `foundry-linux` SDDM greeter theme was shipped by
`calamares-settings-foundry-linux`, which `Depends: calamares` and is purged from the installed
target. `[Theme] Current=foundry-linux` (hook 1100's `30-foundry-live.conf`) then pointed at a
missing theme → SDDM fell back to the Kubuntu breeze greeter → cones.

**Fix** (0.9.97): `git mv` the SDDM theme (`Main.qml` + `background.png` + `metadata.desktop`)
from `calamares-settings-foundry-linux` → `foundry-kde-theme` (which survives the install).
`Main.qml` uses `Qt.resolvedUrl("background.png")` (relative), so no path edits. Hook 1100's
`Theme.Current=foundry-linux` now resolves on the installed system too. calamares-settings 1.0.23
stops shipping the theme (no file conflict — verified with `dpkg-deb -c`).

**General rule**: any asset the *installed* system renders — desktop wallpaper, SDDM greeter,
lock screen — must ship from a survives-install package, never `calamares-settings-*`.

---

## Swap / hibernate research

**Trigger**: Will noticed a swap dropdown (No swap / Swap with Hibernate) on the partition page
and asked why no-swap is the default and whether the dropdown was new.

### Why the dropdown appeared (and how to hide it)

`partition.conf` had `userSwapChoices: []`. Calamares 3.3.14 treats an **empty** list as
*unconfigured* and backfills its built-in choices (none + suspend), so a dropdown shows. The
combo box is shown only when `userSwapChoices` has **> 1** entry — so a **single-entry** list
(`- none`) leaves nothing to choose and Calamares hides it. Fix in calamares-settings 1.0.24:
`userSwapChoices: [ none ]`, `initialSwapChoice: "none"` unchanged. Confirmed via live SSH the
running installer used our config; `git blame` shows the swap lines unchanged since 2026-06-06
(`1f4d540`) — so the dropdown was **not** a Foundry-side change, it's Calamares' empty-list
fallback (possibly surfaced by a Calamares package update; not confirmed without an old-ISO A/B).

### Why no-swap (no hibernate) is the right default — three reasons

1. **Hibernate is blocked under Secure Boot.** Foundry ships Secure Boot support (`shim-signed`).
   Ubuntu's kernel enters *lockdown* mode when Secure Boot is on, and lockdown **disables
   hibernation** (the resume image isn't validated → treated as a code-injection vector). So on a
   Secure-Boot-**on** machine, "Swap (with Hibernate)" yields a large swap partition but hibernate
   still won't work; it only functions with Secure Boot **off**.
2. **Hibernate swap is RAM-sized.** The `suspend` choice sizes swap ≥ RAM — e.g. 32 GB of disk on
   a 32 GB box, costly when storing large game assets.
3. **Plasma 6 / Wayland hibernate-resume is historically flaky** (black screens, failed resumes).

The genuinely weak part of the *old* setup was defaulting to **zero** swap (no OOM protection
during heavy compiles/asset processing), not the lack of hibernate. Options were offered
(swapfile / small-swap default, hibernate-by-default, swapfile-no-hibernate); **Will chose to
keep no-swap as the default and hide the chooser** (2026-06-10). Revisit if OOM-during-build
becomes a real complaint — a swapfile (`initialSwapChoice: "file"`) is the low-cost mitigation
(resizable, no partition planning, no Secure Boot interaction).

`SwapChoice` values for reference: `none`, `small` (swap, no hibernate), `suspend` (RAM-sized,
hibernate), `file` (swapfile), `reuse` (existing swap).

---

## Verification

Run once 0.9.97 boots. **Do not upload to R2 until all pass.**

1. Boot live ISO → Foundry Linux wallpaper (not Kubuntu raccoon/cones).
2. ~~Installer carousel → heading and body fully visible, no left-edge clipping ("Where", not "Vhere").~~ **PASS (0.9.96, Will confirmed)**
3. Carousel → all 4 slides cycle; whole slide shown (dark letterbox top/bottom is fine, no cropping).
4. Shut down/reboot from installer → Plymouth shows "remove the media and press enter to continue".
5. ~~Boot installed system → Foundry ForgeHorizon **desktop** wallpaper (NOT cones).~~ **PASS (0.9.96, Image #2)**
6. foundry-welcome appears on first login; close; relaunch from app menu → opens again.
7. foundrylinux.org link: hover shows pointer cursor; click opens browser.
8. App menu → foundry-welcome shows anvil icon.
9. First login → **no `kconf_update` crash-notification popup** (patched migrate-calendar script).
10. `build.log` (dist/build-anvil-0.9.97.log) → no "command not found" / no unexpected hook failures.
11. **Installed system SDDM login screen → Foundry greeter background, NOT Kubuntu cones** (new in 0.9.97). If autologin skips the greeter, log out to reach it.
12. Autologin → on installed first boot, lands at desktop without a login prompt (watch for the `Relogin=false` once-per-boot behaviour).
13. **Partition page → NO swap dropdown** (calamares-settings 1.0.24, next build). Just "Erase disk" with no swap chooser; no-swap forced.
14. Installed desktop → no "Install Foundry Linux" icon.
15. grub-install succeeds — installed system boots without live ISO.
