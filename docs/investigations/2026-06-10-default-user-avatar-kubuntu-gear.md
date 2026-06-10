# Investigation: default user avatar is the Kubuntu blue gear

**Date**: 2026-06-10
**ISO**: observed through 0.9.98; fixed in 0.9.100 (avatar source moved to foundry-kde-theme)
**Severity**: cosmetic branding leak — login / logout / lock screens show a *Kubuntu* logo on a
*Foundry* system.

---

## Symptom

The Plasma logout/shutdown screen (and the SDDM login screen and lock screen) shows the
**Kubuntu blue-gear logo** as the avatar for the user, instead of anything Foundry-branded.

---

## Root cause

`kubuntu-settings-desktop` ships the default avatar in skel:

```
$ dpkg -L kubuntu-settings-desktop | grep face
/etc/skel/.face
/etc/skel/.face.icon
$ ls -la /etc/skel/.face*
-rw-r--r--  /etc/skel/.face         # the gear PNG (33 KB)
lrwxrwxrwx  /etc/skel/.face.icon -> .face
```

So **`.face` is the real avatar PNG and `.face.icon` is a symlink to it** — and `.face` is a
**conffile** (it's in `kubuntu-settings-desktop.conffiles`). `.face.icon` is the name KDE/SDDM
look up per user. New accounts are created from `/etc/skel`, so every Foundry user inherits the
Kubuntu gear:

- **Live user** — casper's `casper-bottom/25adduser` builds the live account from `/etc/skel`.
- **Installed user** — Calamares' `users` module copies `/etc/skel` into the new home.

Both therefore get `~/.face` (gear) + `~/.face.icon → .face`, which every greeter/lock surface
renders.

(Other avatar sources exist but weren't in play here: `~/.face.icon` takes precedence over the
SDDM theme fallback `/usr/share/sddm/faces/.face.icon` and over `/usr/share/plasma/avatars/*`
which are the optional "Konqi" picks.)

### Does a kubuntu-settings-desktop upgrade revert it?

No — because `/etc/skel/.face` is a **conffile**. When we overwrite it at build time, its md5
no longer matches the packaged version, so dpkg treats it as locally-modified and **keeps our
version** on upgrade (the `confold` default, including under `apt`/`DEBIAN_FRONTEND=noninteractive`).
So the Foundry avatar survives a future kubuntu-settings-desktop upgrade **without** needing a
`dpkg-divert`. (If it were a regular file, an upgrade *would* restore the gear, and a diversion
would be the right tool — but it isn't.)

---

## Fix (hook 1100)

The avatar asset ships from **`foundry-kde-theme`** (1.0.3) — a survives-install package, same as
the wallpaper and SDDM theme — to `/usr/share/foundry-linux/avatar.png` (the 256×256 anvil mark:
circular, dark warm background, orange anvil + sparks). Hook 1100 overwrites skel's `.face` with
it at build time and repoints the `.face.icon` symlink:

```bash
FOUNDRY_AVATAR=""
for _src in /usr/share/foundry-linux/avatar.png \                         # foundry-kde-theme
            /usr/share/icons/hicolor/256x256/apps/foundry-linux.png \     # foundry-welcome
            /usr/share/calamares/branding/foundry-linux/logo.png; do      # build-time fallback
    [[ -f "$_src" ]] && { FOUNDRY_AVATAR="$_src"; break; }
done
if [[ -n "$FOUNDRY_AVATAR" ]]; then
    cp "$FOUNDRY_AVATAR" /etc/skel/.face        # the real (conffile) avatar
    ln -sf .face /etc/skel/.face.icon           # KDE/SDDM read .face.icon → .face
fi
```

**Why a hook, not a package file:** `foundry-kde-theme` can't simply ship `/etc/skel/.face`
— `kubuntu-settings-desktop` already ships that exact path, so two packages owning it is a dpkg
file conflict. A build-time hook overwrite (after kubuntu-settings is unpacked) sidesteps that,
and the conffile flag (above) keeps it surviving upgrades.

**Why write `.face` not `.face.icon`:** `.face` is the actual file; `.face.icon` is just a symlink
to it. Writing the target and re-creating the symlink mirrors Kubuntu's own layout and keeps the
conffile semantics on `.face`.

---

## Verification

Installed/live login, logout, and lock screens show the Foundry anvil avatar, not the Kubuntu
gear. (Plan item 15 / verification step 16.)
