# foundry-welcome Autostart Fix (2026-05-26)

Fix foundry-welcome not appearing on first desktop login in the Foundry Linux live ISO.

## Problem

foundry-welcome never launched on first login despite the autostart entry being
installed at `/etc/xdg/autostart/foundry-welcome.desktop`.

## Root cause

Two completely disconnected "don't show again" mechanisms:

1. **Autostart condition** (`autostart.desktop`):
   ```
   X-KDE-autostart-condition=foundry-welcome:General:FirstRunDone:false
   ```
   KDE's `kde-systemd-start-condition` runs:
   ```
   kreadconfig6 --file foundry-welcome --group General --key FirstRunDone
   ```
   and only launches the app if the result equals `"false"`.

2. **App sentinel** (`main.cpp`):
   ```cpp
   const QString sentinel = ... + "/foundry-welcome-shown";
   if (QFile::exists(sentinel)) return 0;
   ```
   Reads `~/.config/foundry-welcome-shown` (a plain file, not a KConfig key).

On first boot `~/.config/foundry-welcome` doesn't exist, so `kreadconfig6`
returns `""` (empty string), which ≠ `"false"` — condition fails, app never
starts. The sentinel file the app actually checks (`foundry-welcome-shown`)
is never consulted by the condition at all.

A previous session attempted to fix this by pre-seeding
`/etc/skel/.config/foundry-welcome` with `FirstRunDone=false` via the
autologin hook, but that didn't help: the pre-seeded file produces
`FirstRunDone=false` which should match — however the condition was actually
failing for a different reason (`kde-systemd-start-condition` uses a
`--condition` flag form, not space-separated args, making manual testing
misleading).

The real fix: the condition is both wrong and redundant. The app already
handles the re-show guard internally.

## Fix

Remove `X-KDE-autostart-condition` from `data/autostart.desktop`.
Remove the stale skel pre-seeding from `1100-live-autologin.hook.chroot`.
Bump `foundry-welcome` to `1.0.3`.

`autostart.desktop` after fix:
```ini
[Desktop Entry]
Type=Application
Name=Foundry Welcome
Exec=foundry-welcome
X-KDE-autostart-after=panel
NoDisplay=true
```

On every login KDE now starts `foundry-welcome` unconditionally. The app
checks for `~/.config/foundry-welcome-shown` itself and exits immediately
(no UI) if it exists. First login: no sentinel → welcome screen shown →
user dismisses → sentinel written. Subsequent logins: sentinel exists →
app exits in < 1 ms, invisible.

## Files changed

- `foundry-apt/packages/foundry-welcome/data/autostart.desktop` — removed condition
- `foundry-apt/packages/foundry-welcome/debian/changelog` — bumped to 1.0.3
- `foundry-iso/config/hooks/1100-live-autologin.hook.chroot` — removed stale skel block
- `foundry-iso/local-debs/foundry-welcome_1.0.3_amd64.deb` — staged for next ISO build

## Verification

1. Build `foundry-welcome_1.0.3_amd64.deb` in Docker:

```
docker run --rm -v "$PWD:/work" -w /work ubuntu:26.04 bash -c \
  "apt-get update -qq && bash scripts/build-all.sh foundry-welcome"
```

Output:
```
OK   dist/foundry-welcome_1.0.3_amd64.deb  (9704 bytes)
```

PASS

2. Push and install on live instance (192.168.4.32):

```
sshpass -p 'foundry' scp dist/foundry-welcome_1.0.3_amd64.deb root@192.168.4.32:/tmp/
sshpass -p 'foundry' ssh root@192.168.4.32 "dpkg -i /tmp/foundry-welcome_1.0.3_amd64.deb"
```

Output includes:
```
Installing new version of config file /etc/xdg/autostart/foundry-welcome.desktop ...
```

Verified installed desktop file has no `X-KDE-autostart-condition`. PASS

3. Confirmed no sentinel file present for live user:

```
ls /home/user/.config/foundry-welcome-shown   # → no such file
```

PASS

4. Log out and log back in on the VM — foundry-welcome appeared on first login. **PASS** (2026-05-26, confirmed by user)
