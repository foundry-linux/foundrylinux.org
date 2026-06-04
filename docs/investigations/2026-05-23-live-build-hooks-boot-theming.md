# Live-build hooks, apt, and boot/theming investigation notes

Living reference — append new sections as issues are diagnosed/fixed.

---

## live-build 3.0~a57 preferences.d save/restore cycle (2026-06-04)

`lb_chroot_archives chroot install` (runs before package installation) backs up
**all** files from `chroot/etc/apt/preferences.d/` to a `.save/` directory and
then **removes** them. After ALL hooks finish, live-build restores the backed-up
files. This means:

- `config/apt/preferences.d/*.pref` files are **not active** during hook execution.
  They apply only to the *delivered* live system's apt.
- During hooks, apt sees NO pins unless a hook writes them itself.
- Fix for Mozilla firefox priority: hook 0025 cannot rely on the 1001-priority
  `mozilla-firefox.pref` being present. It must install firefox directly.

---

## gpgv fails in chroot's apt during hook execution (2026-06-04)

**Symptom:** Every `apt-get update` or `apt-get install` inside a hook fails:
```
Could not execute 'gpgv' to verify signature (is gnupg installed?)
```
This applies to ALL repos (Ubuntu, Mozilla, Cloudsmith, foundrylinux, worldfoundry).

**Sequence:**
1. `lb_chroot_install-packages` runs apt **from the host container** acting on the
   chroot. The host's apt/gpgv work fine → packages install cleanly.
2. `lb_chroot_hooks` runs hook scripts **inside the chroot** via `chroot chroot/`.
   The chroot's apt calls `execvp("gpgv", ...)` and fails.

**NOT caused by:**
- gpgv being removed (binary is present; `/usr/bin/gpgv --version` works)
- gpgv being marked auto-installed (apt-mark manual gnupg gpgv shows "already set")
- autoremove removing gnupg (apt-mark manual prevents it)

**Likely cause:** Ubuntu 26.04's apt uses the Sequoia library for verification by
default. In a live-build chroot (no /proc/self/fd, no systemd, restricted
environment), Sequoia's initialization fails. apt falls back to calling the
external `gpgv` binary, but the fallback path's `execvp` fails for an unknown
reason (possibly chroot vs host PATH or exec environment mismatch).

**Effect:** apt marks every repo as unverified for this session. Attempting
`apt-get install -t mozilla firefox --allow-downgrades` silently returns
"already newest version (1:1snap1-0ubuntu8)" because apt won't downgrade to
an unverified package.

**Fix (hook 0025, 0.9.50):** Bypass apt entirely. Use the Mozilla package index
already on disk (cached by the host's apt during lb_chroot_install-packages),
extract the firefox `.deb` URL, download with `curl`, install with
`dpkg --force-downgrade -i`. This completely bypasses apt's gpgv check.

---

## no-snapd.pref pin cleared before package install (2026-06-04)

`config/apt/preferences.d/no-snapd.pref` (priority -1 for snapd) is backed up
and removed before `lb_chroot_install-packages`. Snapd installs anyway because
`firefox (1:1snap1-0ubuntu8)` has `PreDepends: snapd`, which `kubuntu-desktop`
pulls in. The no-snapd pin was meant to block it but is NOT active at install
time.

Correct removal path: hook 0025 swaps firefox for Mozilla's native .deb (no
snapd PreDepend), then `apt-get purge snapd` succeeds because nothing depends
on it anymore.

---

## hook 0020 autoremove removes gnupg/gpgv (2026-06-04, fixed)

`apt-get purge libreoffice*` orphans gnupg/gpgv (libreoffice depends on gnupg
for macro signing; when libreoffice goes, gnupg becomes auto-installed with no
remaining manual consumer).

**Fix:** `apt-mark manual gnupg gpgv` before `apt-get autoremove` in hook 0020.
Added to both hooks 0005 (as first-hook safety net) and 0020 (before autoremove).

Note: this fix prevents *autoremove* from removing gnupg, but doesn't fix the
gpgv-fails-in-apt issue above (that's a separate problem).

---

## hook execution order (foundry-iso, confirmed 2026-06-04)

```
0005  disable-packagekit-invoke  → write DPkg::Post-Invoke ""; apt-mark manual gpgv
0010  enable-multiverse          → add-apt-repository multiverse; apt-get update (W: gpgv errors)
0020  strip-kubuntu-bloat        → purge libreoffice, plasma-welcome, snapd (if no preDepend)
0025  mozilla-pin                → dpkg --force-downgrade Mozilla firefox; purge snapd
0030  install-foundry-edition    → apt install foundry-${EDITION} from local-debs
0040  (other hooks)
1000  install-local-debs         → dpkg -i all .debs in /tmp/local-debs/
1010  trim-atelier-only-pkgs     → apt-mark auto ghidra openjdk-21 opencv vtk; autoremove
1050  plymouth-set-theme         → plymouth-set-default-theme foundry
1100  (branding hooks)
1150  (theming hooks)
1200  (final hooks)
```

live-build processes `.hook.chroot` files in lexicographic order.
Numeric prefix ensures deterministic ordering across hook batches.

---

## lb_chroot_package-lists: queues, does not purge (2026-06-04)

`lb_chroot_package-lists` processes `*.list.chroot` files (install queuing).
**`.list.chroot.purge` files do NOT fire in live-build 3.0~a57** — the purge
list mechanism is not implemented. Use a hook script instead.

---

## Cloudsmith task/go-task repo (2026-06-04)

`foundry-iso/config/archives/cloudsmith-task.list.chroot` pins go-task. The
apt source URL changed layout (`any-distro` → per-distro) — watch for 404s.
`task check-apt-repos` will catch this.
