# Calamares Installer Fix (2026-05-25)

Fix the Calamares installer crashing immediately on launch in the Foundry Linux live ISO.

## Problem

Clicking "Install Foundry Linux" on the live desktop crashed Calamares before any UI appeared.
The `launch-calamares.sh` wrapper ran but `calamares` exited with a fatal branding error:

```
ERROR: FATAL in "/usr/share/calamares/branding/foundry-linux/branding.desc"
 invalid node; first invalid key: "style"
```

## Root cause

Calamares 3.3.14 (shipped in Ubuntu 26.04) requires a `style:` map in `branding.desc`.
Without it, the YAML parser raises "invalid node" and aborts. The map controls installer
sidebar colors. Our `branding.desc` was also missing the window layout keys
(`windowExpanding`, `windowSize`, `windowPlacement`, `sidebar`, `navigation`) which
Calamares 3.3 treats as required-enough to warn about.

Secondary: `settings.conf` was missing two keys that became required in Calamares 3.3:
`hide-back-and-next-during-exec` and `quit-at-end`.

## Fix

### `branding.desc` additions

Added to `foundry-apt/packages/calamares-settings-foundry-linux/data/branding/foundry-linux/branding.desc`:

```yaml
windowExpanding:    normal
windowSize:         800px,520px
windowPlacement:    center

sidebar:    widget
navigation: widget

style:
    SidebarBackground:        "#0a0a0a"
    SidebarText:              "#f7f7f7"
    SidebarTextCurrent:       "#0a0a0a"
    SidebarBackgroundCurrent: "#ff5b1a"
```

### `settings.conf` additions

Added to `data/settings.conf`:

```yaml
hide-back-and-next-during-exec: false
quit-at-end: false
```

### Package version

Bumped `calamares-settings-foundry-linux` from `1.0.5` → `1.0.6`.

## Files changed

- `foundry-apt/packages/calamares-settings-foundry-linux/data/branding/foundry-linux/branding.desc`
- `foundry-apt/packages/calamares-settings-foundry-linux/debian/calamares-settings-foundry-linux/usr/share/calamares/branding/foundry-linux/branding.desc`
- `foundry-apt/packages/calamares-settings-foundry-linux/data/settings.conf`
- `foundry-apt/packages/calamares-settings-foundry-linux/debian/changelog`
- `foundry-iso/local-debs/calamares-settings-foundry-linux_1.0.6_all.deb` (staged)
- `foundry-iso/docs/howto-kubuntu-remix.md` (new Calamares Branding section + pitfalls)

## Verification

1. Build `calamares-settings-foundry-linux_1.0.6_all.deb`:

```
task build PKG=calamares-settings-foundry-linux
```

Output:

```
=== Building calamares-settings-foundry-linux (canonical debian/ source format) ===
SKIP calamares-settings-foundry-linux (dist/calamares-settings-foundry-linux_1.0.6_all.deb already current)
```

PASS — deb built as `1.0.6`.

2. Push to live instance (192.168.4.32) and install:

```
sshpass -p 'foundry' scp foundry-apt/dist/calamares-settings-foundry-linux_1.0.6_all.deb root@192.168.4.32:/tmp/
sshpass -p 'foundry' ssh root@192.168.4.32 "DEBIAN_FRONTEND=noninteractive dpkg -i --force-confnew /tmp/calamares-settings-foundry-linux_1.0.6_all.deb"
```

PASS — installed cleanly, settings.conf updated.

3. Verify Calamares loads without error:

```
sshpass -p 'foundry' ssh root@192.168.4.32 \
  "DISPLAY=:0 WAYLAND_DISPLAY=wayland-0 XDG_RUNTIME_DIR=/run/user/1000 calamares -D8 2>&1 | head -20"
```

Output (key lines):

```
07:53:56 [6]: Calamares::Branding::Branding(…)
    Using Calamares branding file at "/usr/share/calamares/branding/foundry-linux/branding.desc"
07:53:56 [6]: Calamares::Branding::Branding(…)
    Loaded branding component "foundry-linux"
```

PASS — no fatal error, branding loaded.

4. User tests installer UI on live instance at 192.168.4.32 — **pending**.

## Next ISO build

The staged `1.0.6` deb will be picked up automatically by `task iso-build` (via `iso-sync-local-debs` dep). No manual step needed.

```bash
task iso-build   # or: task iso-clean && task iso-build for a full clean rebuild
```
