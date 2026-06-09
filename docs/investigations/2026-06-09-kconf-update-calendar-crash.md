# Investigation: kconf_update calendar migration crash on live session

**Date**: 2026-06-09
**ISO versions affected**: first observed in 0.9.90; earlier ISOs on 26.04 may have had it unreported
**Severity**: cosmetic — KDE crash notification appears on first login; zero functional impact
**Status**: upstream `plasma-workspace` bug; Foundry Linux patched in hook 1100 (see [Workaround](#foundry-linux-workaround))

---

## Symptom

On the live desktop and on an installed system's first login, KDE's crash notifier pops up:

> An application has crashed on your system (now or in the past).

Clicking "Details" shows no detail panel. The crash report is at:

```
/var/crash/_usr_share_kconf_update_migrate-calendar-to-plugin-id.py.1000.crash
```

### Crash traceback

```
Package:   plasma-workspace 4:6.6.4-0ubuntu2
Signal:    Traceback (most recent call last):
  File "/usr/share/kconf_update/migrate-calendar-to-plugin-id.py", line 33, in <module>
    with open(appletsrcPath, "r+") as appletsrc:
PermissionError: [Errno 13] Permission denied:
  '/etc/xdg/plasma-org.kde.plasma.desktop-appletsrc'
```

---

## Root cause

### What the script is supposed to do

`kconf_update` is a KDE mechanism that runs migration scripts at session startup when packages
are upgraded. `migrate-calendar-to-plugin-id.py` (written by Nicolas Fella, 2024, GPLv2/v3)
strips full `.so` paths from the `enabledCalendarPlugins=` key in the Plasma desktop appletsrc,
leaving bare plugin IDs. This migrates configs written by Plasma 5 / early Plasma 6 that used
`/usr/lib64/qt5/plugins/plasmacalendarplugins/holidaysevents.so` to the modern
`holidaysevents` form.

### How it finds the file

```python
proc = subprocess.Popen(
    ["/usr/lib/qt6/bin/qtpaths6", "--locate-file", "ConfigLocation",
     "plasma-org.kde.plasma.desktop-appletsrc"],
    stdout=subprocess.PIPE,
)
...
appletsrcPath = line.removesuffix("\n")
```

`qtpaths6 --locate-file ConfigLocation <name>` maps to Qt's
`QStandardPaths::locate(ConfigLocation, name)`, which searches config directories in order:

1. User config dir: `~/.config/plasma-org.kde.plasma.desktop-appletsrc`
2. System config dir: `/etc/xdg/plasma-org.kde.plasma.desktop-appletsrc`

It returns the **first existing** file. On a fresh Foundry Linux install:

- `/etc/xdg/plasma-org.kde.plasma.desktop-appletsrc` **exists** — `kubuntu-settings-desktop`
  ships it with Kubuntu LAF defaults (including `Image=Kubuntu`)
- `~/.config/plasma-org.kde.plasma.desktop-appletsrc` does **not** exist — the user is logging
  in for the first time; Plasma hasn't written their personal copy yet

So `qtpaths6` returns the system path. The script then tries to `open(path, "r+")` — read/write
— on a root-owned `644` file. `EACCES`. Apport catches the exception, writes a `.crash` file,
KDE crash notifier fires.

**This is an upstream `plasma-workspace` bug.** The migration targets user configuration; it
should only ever operate on the user's writable config file, not a system-wide default.

---

## Why this is harmless

1. **The migration is cosmetic.** It strips `.so` paths from `enabledCalendarPlugins=`. A fresh
   Foundry Linux install has no `.so`-form plugin IDs in any config — the migration is a no-op
   even when it succeeds. The calendar applet works fine either way.

2. **Only relevant for users upgrading from Plasma 5 configs.** The old format was written by
   Plasma 5. New installs start with Plasma 6 defaults. There is nothing to migrate.

3. **Fires only once per user.** `kconf_update` records each script's run in
   `~/.config/kconf_updaterc`. After the first (failed) attempt, the script is marked as
   attempted and won't re-run on subsequent logins.

4. **Desktop startup is unaffected.** `kconf_update` scripts run asynchronously. The Python
   exception is caught by apport — the session continues normally. The crash notification is
   purely informational.

---

## When did this start?

The script and its `.upd` rule (`Id=migrate-calendar-plugins`, `Version=6`) were authored in
2024 and landed in `plasma-workspace` as part of Plasma 6.x — they are new in
`plasma-workspace 4:6.6.4-0ubuntu2` (Ubuntu 26.04). The crash requires **both** the script
(from `plasma-workspace`) and the system-wide appletsrc (from `kubuntu-settings-desktop`) to
exist simultaneously in the chroot. This is a recent addition to 26.04's package set, not
something inherited from the 24.04 era; the crash was first **observed** in ISO 0.9.90
(2026-06-09). It is not known whether every 26.04-based build before 0.9.90 had it; the
notification popup is easy to miss or dismiss without investigation.

---

## Upstream patch

The fix is to replace `--locate-file` (which can return a system-owned path) with
`--writable-path` (which always returns the user's writable config directory), then construct
the path manually. If the user has no personal appletsrc, there is nothing to migrate — exit
cleanly.

### `migrate-calendar-to-plugin-id.py` — proposed patch

```diff
--- a/migrate-calendar-to-plugin-id.py
+++ b/migrate-calendar-to-plugin-id.py
@@ -1,10 +1,11 @@
 # SPDX-FileCopyrightText: 2024 Nicolas Fella <nicolas.fella@gmx.de>
 # SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 
+import os
 import re
 import io
 import subprocess
 import sys
 
 # removes the full path from calendar plugin entries and only stores the plugin id, e.g.
 # /usr/lib64/qt5/plugins/plasmacalendarplugins/holidaysevents.so -> holidaysevents
 
@@ -12,21 +13,24 @@ import sys
 proc = subprocess.Popen(
     [
         "/usr/lib/qt6/bin/qtpaths6",
-        "--locate-file",
+        "--writable-path",
         "ConfigLocation",
-        "plasma-org.kde.plasma.desktop-appletsrc",
     ],
     stdout=subprocess.PIPE,
 )
 if proc.wait(3) != 0:
     sys.exit(1)
 
+writableConfigDir = ""
 for line in io.TextIOWrapper(proc.stdout, encoding="utf-8"):
-    appletsrcPath = line.removesuffix("\n")
+    writableConfigDir = line.removesuffix("\n")
+
+appletsrcPath = os.path.join(writableConfigDir, "plasma-org.kde.plasma.desktop-appletsrc")
 
-if len(appletsrcPath) == 0 or not appletsrcPath.endswith("appletsrc"):
-    # something is wrong
-    exit()
+if not writableConfigDir or not os.path.isfile(appletsrcPath):
+    # No personal config — nothing to migrate. This is normal on first login.
+    sys.exit(0)
```

**What changes and why:**

- `--locate-file ConfigLocation <name>` → `--writable-path ConfigLocation`
  `--locate-file` returns the first *existing* file anywhere in Qt's config search path,
  including system dirs. `--writable-path` returns only the user's writable config directory.
- The filename is appended with `os.path.join`, constructing `~/.config/plasma-org…appletsrc`.
- If that file doesn't exist, the script exits cleanly — no permission error, no crash, no
  notification. This is the correct behaviour: if the user has no personal appletsrc, the old
  `.so`-path plugin IDs can't be in their config.

### `.upd` rule (`migrate-calendar-to-plugin-id.upd`)

```ini
Version=6
Id=migrate-calendar-plugins
Script=migrate-calendar-to-plugin-id.py
```

`kconf_update` records this run under `Id=migrate-calendar-plugins` in
`~/.config/kconf_updaterc`. Whether the record key is set on a failed run (i.e. whether
kconf_update catches the Python exception vs the exception bypassing kconf_update's own
error handling) is the open question that determines whether the crash fires on every login
or just the first.

### Where to file upstream

- KDE Bugs: [bugs.kde.org](https://bugs.kde.org) → Product: `plasma-workspace` → Component:
  `general`
- Source: `plasma-workspace/kconf_update/migrate-calendar-to-plugin-id.py` in the
  [KDE Invent plasma-workspace repo](https://invent.kde.org/plasma/plasma-workspace)

---

## Foundry Linux workaround

Until the upstream fix lands in Ubuntu 26.04, Foundry Linux patches the script at ISO build
time. The patch is applied in hook `foundry-iso/config/hooks/1100-live-autologin.hook.chroot`
after `plasma-workspace` is installed into the chroot.

### Patch applied in hook 1100

```bash
# Patch migrate-calendar-to-plugin-id.py to use --writable-path instead of
# --locate-file so it never tries to write to a root-owned system config file.
MIGRATE_SCRIPT=/usr/share/kconf_update/migrate-calendar-to-plugin-id.py
if [[ -f "$MIGRATE_SCRIPT" ]]; then
    python3 - <<'PYEOF'
import sys, os, re

path = '/usr/share/kconf_update/migrate-calendar-to-plugin-id.py'
with open(path) as f:
    src = f.read()

old = '''\
proc = subprocess.Popen(
    [
        "/usr/lib/qt6/bin/qtpaths6",
        "--locate-file",
        "ConfigLocation",
        "plasma-org.kde.plasma.desktop-appletsrc",
    ],
    stdout=subprocess.PIPE,
)
if proc.wait(3) != 0:
    sys.exit(1)

for line in io.TextIOWrapper(proc.stdout, encoding="utf-8"):
    appletsrcPath = line.removesuffix("\\n")

if len(appletsrcPath) == 0 or not appletsrcPath.endswith("appletsrc"):
    # something is wrong
    exit()'''

new = '''\
import os  # noqa: E402 (added by Foundry Linux distro patch)

proc = subprocess.Popen(
    [
        "/usr/lib/qt6/bin/qtpaths6",
        "--writable-path",
        "ConfigLocation",
    ],
    stdout=subprocess.PIPE,
)
if proc.wait(3) != 0:
    sys.exit(1)

writableConfigDir = ""
for line in io.TextIOWrapper(proc.stdout, encoding="utf-8"):
    writableConfigDir = line.removesuffix("\\n")

appletsrcPath = os.path.join(writableConfigDir, "plasma-org.kde.plasma.desktop-appletsrc")

if not writableConfigDir or not os.path.isfile(appletsrcPath):
    sys.exit(0)'''

if old not in src:
    print(f"WARNING: expected block not found in {path} — patch skipped", file=sys.stderr)
    sys.exit(0)  # Don't abort the build; upstream may have fixed it already
src = src.replace(old, new)
with open(path, 'w') as f:
    f.write(src)
print(f"Patched {path}")
PYEOF
fi
```

The patch uses `sys.exit(0)` when the expected block is not found — if upstream ships the fix,
the hook silently no-ops rather than breaking the build.

---

## Occurrence (updated)

| Context | Occurs? | Notes |
|---------|---------|-------|
| Live session (unpatched) | Yes | System appletsrc exists; live user can't write it |
| Installed system first login (unpatched) | Yes, once | Same path returned by qtpaths6; same error |
| Installed system subsequent logins | No | kconf_update marks it as attempted; won't re-run |
| Live or installed (with hook 1100 patch) | No | Script exits cleanly when user has no personal config |
