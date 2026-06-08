# foundry-welcome + Calamares installer polish — 2026-06-09

Small fixes reported after first successful install test on ISO 0.9.82.

## Changes

### foundry-welcome

| # | Issue | Fix |
|---|-------|-----|
| 1 | Won't re-launch explicitly (sentinel blocks all paths) | Pass `--autostart` from XDG autostart entry; sentinel only exits when that flag is present. Direct launches always show the window. |
| 2 | foundrylinux.org link has no hover cursor / not browser-like | Replace `QQC2.Label` rich-text link + `HoverHandler` with a plain `Text` item: `color: Kirigami.Theme.linkColor`, `font.underline: true`, `MouseArea { cursorShape: Qt.PointingHandCursor; onClicked: Qt.openUrlExternally(...) }` |
| 3 | No icon in application menu (`Icon=foundry-linux` resolves nothing) | Ship `foundry-linux.png` (256×256 anvil logo) inside the foundry-welcome package; install to `/usr/share/icons/hicolor/256x256/apps/` via CMakeLists.txt |

### Calamares settings (1.0.17)

| # | Issue | Fix |
|---|-------|-----|
| 4 | Carousel slide text clipped on left | Replace `anchors.leftMargin/rightMargin` on `Column` with explicit `x: 24; width: parent.width - 48` — breaks the circular width dependency |
| 5 | Installer desktop icon shows generic Calamares icon | Hook copies `logo.png` → `/usr/share/pixmaps/foundry-linux-installer.png`; `.desktop` uses `Icon=foundry-linux-installer` |

Items 4 and 5 are already committed (f80d2f0).

## Files changed

- `foundry-apt/packages/foundry-welcome/src/main.cpp` — `--autostart` flag gate
- `foundry-apt/packages/foundry-welcome/src/qml/pages/IntroPage.qml` — browser-like link
- `foundry-apt/packages/foundry-welcome/data/autostart.desktop` — add `--autostart`
- `foundry-apt/packages/foundry-welcome/data/foundry-linux.png` — icon asset (new)
- `foundry-apt/packages/foundry-welcome/CMakeLists.txt` — install icon to hicolor
- `foundry-apt/packages/foundry-welcome/debian/changelog` — bump to 1.0.7

## Verification

1. Boot ISO; foundry-welcome appears on first login.
2. Close it; re-launch from app menu → opens again (no sentinel block).
3. Log out and back in → autostart shows it again (sentinel was written on close, but... wait — should autostart show it again after manual close?)

**Sentinel write policy**: the sentinel is written when the app closes (`Qt.quit()`), regardless of how it was launched. So after any close, subsequent autostart invocations won't show it. Manual launches always show it. This is the correct behaviour: "shows once on login, re-launchable from app menu any time."

4. Hover over foundrylinux.org → pointer cursor, underline visible, click opens browser.
5. Open app menu → Foundry Welcome has the anvil icon.
6. Installer carousel → text properly padded from left edge.
7. Installer desktop icon → glowing anvil, not Calamares blue circle.
