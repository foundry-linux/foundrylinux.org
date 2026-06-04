# Plan: Fix plasma-welcome autobooting instead of foundry-welcome

## Context

The Foundry Linux 0.9.6 ISO boots the KDE Welcome Center (`plasma-welcome`) alongside `foundry-welcome`. The existing kded6rc suppression is not working, and there is no real reason to keep `plasma-welcome` installed at all.

**Root cause confirmed via SSH into the running VM:**

- `plasma-welcome` ships only a kded6 plugin. No XDG autostart file, no systemd unit.
- Journal confirms the module ran: `kded6[1802]: org.kde.plasma.welcome.kded: Launching Welcome Center with no last seen version`
- The kded6rc suppression (`autoload=false` in system config, `loaded=false` in user skel) is wrong: kded6 reads `autoload` from the user-level kded6rc, not the system-level one. The skel set `loaded=false` (wrong key — tracks active state, not startup behaviour).

**Why we still ship it:**

The original hook comment ("purging it fails because kubuntu-desktop depends on it") was wrong. Both `kubuntu-desktop` and `kubuntu-settings-desktop` list `plasma-welcome` only as a `Recommends`, not `Depends`. There is no dependency blocker — we can purge it cleanly with `apt-get purge`.

**Fix:** purge `plasma-welcome` in hook 0020. Remove all the kded6rc suppression cruft — it's no longer needed.

---

## Changes

### 1. `foundry-iso/config/hooks/0020-strip-kubuntu-bloat.hook.chroot`

Add `plasma-welcome` to the purge call at the top (or add as a separate `apt-get purge` line near it):

```bash
apt-get purge -y plasma-welcome
```

Remove the now-obsolete kded6rc suppression block (lines 18–27):
```bash
# DELETE these lines — no longer needed once plasma-welcome is purged:
# mkdir -p /etc/xdg
# cat >> /etc/xdg/kded6rc <<'EOF'
# [Module-plasma_welcome]
# autoload=false
# EOF
```

### 2. `foundry-iso/config/hooks/1100-live-autologin.hook.chroot`

Remove the kded6rc skel block (lines 142–149):
```bash
# DELETE these lines — no longer needed:
# mkdir -p /etc/skel/.config
# printf '[Module-plasma_welcome]\nloaded=false\n' >> /etc/skel/.config/kded6rc
# rm -f /usr/share/plasma/plasma-welcome/extra-pages/01-Kubuntu.qml
```

### 3. `foundry-iso/docs/investigations/2026-05-23-live-build-hooks-boot-theming.md`

Add a new section documenting:
- The kded6 `autoload` vs `loaded` distinction
- Why system-level kded6rc doesn't work for module suppression
- The correct fix: purge the package (and that the "depends on it" comment was wrong — it was only `Recommends`)

---

## Rebuild

```bash
cd foundry-iso
task iso-build   # ~10-15 min, builds in ubuntu:26.04 Docker container
```

---

## Verify in QEMU

```bash
task iso-smoke   # boots ISO with VirtGL; wait for "PASS: ISO booted past early-boot stage"
```

Visual check in QEMU window: `foundry-welcome` appears, KDE Welcome Center does not.

SSH sanity check once the desktop loads:
```bash
ssh -p 2222 user@localhost   # password: live
dpkg -l plasma-welcome        # must show 'rc' (removed) or not present
pgrep -a plasma-welcome       # must return nothing
```

---

## Files to modify

- `foundry-iso/config/hooks/0020-strip-kubuntu-bloat.hook.chroot` — purge `plasma-welcome`; remove kded6rc suppression block
- `foundry-iso/config/hooks/1100-live-autologin.hook.chroot` — remove skel kded6rc block and QML page removal
- `foundry-iso/docs/investigations/2026-05-23-live-build-hooks-boot-theming.md` — new section on the root cause and fix
