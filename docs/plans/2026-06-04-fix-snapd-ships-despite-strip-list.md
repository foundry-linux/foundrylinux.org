# Fix: snapd ships despite strip list

## Context

**Bug:** `snapd` (140 MiB) is present in `foundry-anvil-0.9.36` despite `snapd snap-store` being listed in both `config/package-lists/strip.list.chroot.purge` and explicitly purged in `config/hooks/0020-strip-kubuntu-bloat.hook.chroot`.

**Root cause:** live-build installs package lists with `LB_APT_RECOMMENDS="true"` (set via `--apt-recommends true` in `auto/config`). `kubuntu-desktop` Recommends `plasma-discover-backend-snap`, which depends on `snapd`. This happens in the `lb_chroot_package_lists` stage — BEFORE hooks run. Hook 0020 then purges snapd correctly, but hook 0030 runs `apt-get install foundry-anvil` which causes apt to re-evaluate Recommends for the installed base and re-pull snapd. The `autoremove` at the end of hook 0020 doesn't catch the re-pull because it happens after 0020 finishes.

**Why the strip list alone doesn't help:** `.list.chroot.purge` files are also processed in the `lb_chroot_package_lists` stage, before hooks — so even the purge list runs before hook 0030's apt install can re-pull snapd.

**Live-build stage order (relevant excerpt):**
1. `lb_chroot_apt` — copy `config/apt/preferences.d/` into chroot → `/etc/apt/preferences.d/` ← **fix point**
2. `lb_chroot_package_lists` — install `.list.chroot`, then purge `.list.chroot.purge`
3. `lb_chroot_hooks` — run 0020 (purge), then 0030 (install foundry-anvil) ← **re-pull happens here**

## Fix

**Add an apt pin at −1 priority for snapd and its snap-infrastructure packages.** This is applied at stage 1 (before any packages are installed), so snapd can never enter the chroot — not during initial kubuntu-desktop installation, and not during hook 0030's `apt-get install foundry-anvil`.

### File to create

**`foundry-iso/config/apt/preferences.d/no-snapd.pref`** (new file — directory does not yet exist):

```
# Snap infrastructure is not wanted in Foundry Linux.
# Pin at -1 so apt never installs these via Recommends, even when
# --apt-recommends true is active (kubuntu-desktop Recommends plasma-discover-backend-snap
# which Depends on snapd). This pin is applied at lb_chroot_apt, before any packages
# are installed. The hook 0020 explicit purge + autoremove is belt-and-suspenders.
Package: snapd snap-store plasma-discover-backend-snap
Pin: release *
Pin-Priority: -1
```

### Also update hook 0020

In `config/hooks/0020-strip-kubuntu-bloat.hook.chroot`, update the comment block on the `_purge snapd snap-store` line to note that the -1 pin in `config/apt/preferences.d/no-snapd.pref` is now the primary guard (prevents installation entirely), and the `_purge` line is belt-and-suspenders in case of edge cases (e.g. explicit direct install elsewhere in the chain).

No logic changes to the hook — just the explanatory comment.

## Files to modify

| File | Action |
|---|---|
| `foundry-iso/config/apt/preferences.d/no-snapd.pref` | **Create** (new file + new dir) |
| `foundry-iso/config/hooks/0020-strip-kubuntu-bloat.hook.chroot` | Update comment on `_purge snapd` line only |

## Verification

1. Build the ISO: `task build-iso` (or `EDITION=anvil bash foundry-iso/build-iso.sh`)
2. Mount the squashfs and check snapd is absent:
   ```bash
   sudo mount -o loop foundry-iso/.build/binary/live/filesystem.squashfs /mnt/iso
   dpkg-query --root=/mnt/iso -l snapd snap-store plasma-discover-backend-snap 2>&1 | grep -v '^un '
   # Expected: all three show as "no packages found" or status 'un' (unknown/not installed)
   sudo umount /mnt/iso
   ```
3. Confirm ISO size is ~140 MiB smaller than 0.9.36.
4. Boot the ISO in a VM and verify Plasma Discover opens without snap backend errors.
