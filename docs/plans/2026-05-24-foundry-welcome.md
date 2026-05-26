# foundry-welcome — Custom Welcome Center

## Context

`plasma-welcome` (KDE's Welcome Center, shown on first login in the Foundry Linux ISO) ships with Kubuntu/KDE branding baked into the binary as Qt resources. Removing or re-theming individual built-in pages requires patching and rebuilding the full C++/KDE Frameworks source — a disproportionate build burden for branding changes.

Instead: ship `foundry-welcome`, a small QML/Kirigami application we fully own. We control every page, all branding, and which panels appear. `plasma-welcome`'s KDED autostart trigger is masked so it never fires.

---

## Pages (v1 — 3 Foundry-specific panels)

| # | Page | Content |
|---|------|---------|
| 1 | **Foundry Linux** | Wordmark/logo, "open-source game dev workstation" pitch, link to foundrylinux.org |
| 2 | **What's Installed** | Tools pre-installed for this edition (retro tools, Blender addons, etc.) |
| 3 | **Community & Docs** | foundrylinux.org/docs, GitHub issues, Discord/Matrix, how to contribute |

KDE feature tutorial pages (KRunner, KDE Connect, Overview …) are v2 — discoverable via System Settings, not needed on day one.

---

## How "run once on first login" works

**XDG autostart** — `/etc/xdg/autostart/foundry-welcome.desktop` fires on every login system-wide:
```ini
[Desktop Entry]
Type=Application
Name=Foundry Welcome
Exec=foundry-welcome
X-KDE-autostart-after=panel
```

**Sentinel file** — `foundry-welcome` checks `~/.config/foundry-welcome-shown` at startup:
- Absent → show window; create sentinel on close
- Present → exit immediately (silent, no window)

**Live session**: home is always re-created fresh from `/etc/skel` each boot → sentinel never exists → welcome always shows. Correct behavior for a live ISO.

**Installed system** (post-Calamares): home persists → sentinel survives reboots → welcome never fires again.

**Suppress plasma-welcome**: mask its KDED module in `/etc/skel/.config/kded6rc` so it never triggers. Done via `1100-live-autologin.hook.chroot`.

---

## Implementation

### 1. New package: `foundry-apt/packages/foundry-welcome/`

```
packages/foundry-welcome/
  CMakeLists.txt
  src/
    main.cpp                     KApplication + QML window launcher (~40 lines)
    qml/
      main.qml                   Window + page list + Skip / Next nav
      pages/
        IntroPage.qml            Foundry Linux intro — logo, pitch, link
        ToolsPage.qml            What's installed — edition-aware bullet list
        CommunityPage.qml        Links: docs, GitHub, Discord
  data/
    foundry-welcome.desktop      App launcher entry
    autostart.desktop            /etc/xdg/autostart entry (system-wide trigger)
  debian/
    control                      Depends: kirigami2, libqt6declarative6; Arch: amd64
    changelog                    1.0.0-1foundry1
    rules                        dh $@ --buildsystem=cmake
    install                      binary → /usr/bin; QML → /usr/share/foundry-welcome/qml/; .desktop files
    source/format                3.0 (native)
    copyright
```

**Style**: Kirigami `ApplicationWindow` + `PageRow` (or simple `StackView`). Page layout matches plasma-welcome conventions (heading, description, optional image/links) for visual continuity.

**Logo**: reference `/usr/share/calamares/branding/foundry-linux/logo.png` (already on-system from `calamares-settings-foundry-linux`). No new asset needed for v1.

### 2. Modify: `foundry-iso/config/hooks/1100-live-autologin.hook.chroot`

Add after existing skel setup:
```bash
# Suppress plasma-welcome; foundry-welcome replaces it
mkdir -p /etc/skel/.config
printf '[Module-plasma_welcome]\nloaded=false\n' \
    >> /etc/skel/.config/kded6rc

# Remove Kubuntu extra-page
rm -f /usr/share/plasma/plasma-welcome/extra-pages/01-Kubuntu.qml
```

### 3. ISO build integration

After building the .deb, drop it into the local-debs bundle:
```
foundry-iso/config/includes.chroot/tmp/local-debs/foundry-welcome_1.0.0_amd64.deb
```
`1000-install-local-debs.hook.chroot` picks it up automatically — same pattern as `calamares-settings-foundry-linux`.

---

## Files to create / modify

| File | Action |
|------|--------|
| `foundry-apt/packages/foundry-welcome/CMakeLists.txt` | new |
| `foundry-apt/packages/foundry-welcome/src/main.cpp` | new |
| `foundry-apt/packages/foundry-welcome/src/qml/main.qml` | new |
| `foundry-apt/packages/foundry-welcome/src/qml/pages/IntroPage.qml` | new |
| `foundry-apt/packages/foundry-welcome/src/qml/pages/ToolsPage.qml` | new |
| `foundry-apt/packages/foundry-welcome/src/qml/pages/CommunityPage.qml` | new |
| `foundry-apt/packages/foundry-welcome/data/autostart.desktop` | new |
| `foundry-apt/packages/foundry-welcome/data/foundry-welcome.desktop` | new |
| `foundry-apt/packages/foundry-welcome/debian/{control,changelog,rules,install,source/format,copyright}` | new |
| `foundry-iso/config/hooks/1100-live-autologin.hook.chroot` | modify (kded6rc mask + rm Kubuntu page) |
| `foundry-iso/config/includes.chroot/tmp/local-debs/foundry-welcome_*.deb` | add after build |

---

## Verification

1. **Build in Docker**: build inside `ubuntu:26.04` container — confirm `.deb` produced with no errors
2. **Install smoke test**: install .deb in container, run `foundry-welcome` — confirm window opens with 3 pages, Skip/Next nav works
3. **Sentinel check**: run a second time — confirm it exits immediately with no window
4. **QEMU ISO test**: boot the ISO, log in — confirm `foundry-welcome` opens (not plasma-welcome), shows 3 Foundry-branded pages

   **Result (2026-05-26, `foundry-anvil-0.9.15-amd64.iso`):** foundry-welcome appeared on first boot. **PASS.**
5. **Re-login test**: log out and back in — confirm welcome does not reappear
