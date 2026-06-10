#!/usr/bin/env bash
# Read-only inspection of an installed Foundry Linux target disk (qcow2).
# Mounts the root partition via qemu-nbd --read-only and dumps the wallpaper /
# autologin / branding state, then cleanly disconnects.  Needs sudo (nbd).
#
# Usage:  sudo bash inspect-installed-disk.sh [/path/to/disk.qcow2]
set -euo pipefail

# When run under sudo, $HOME is /root — resolve the invoking user's home so the
# default disk path (~/.cache/...) points at the real user, not root.
_home="${SUDO_USER:+/home/$SUDO_USER}"; _home="${_home:-$HOME}"
DISK="${1:-${XDG_CACHE_HOME:-$_home/.cache}/foundry-test-disk.qcow2}"
NBD=/dev/nbd0
MNT=$(mktemp -d)

cleanup() {
  mountpoint -q "$MNT" && umount "$MNT" 2>/dev/null || true
  qemu-nbd --disconnect "$NBD" >/dev/null 2>&1 || true
  rmdir "$MNT" 2>/dev/null || true
}
trap cleanup EXIT

[[ -f "$DISK" ]] || { echo "ERROR: disk not found: $DISK" >&2; exit 1; }
if pgrep -f "qemu-system.*$(basename "$DISK")" >/dev/null 2>&1; then
  echo "ERROR: a VM is using this disk — shut it down first." >&2; exit 1
fi

# nbd partition scanning needs max_part > 0.  If nbd is already loaded without
# it (max_part=0), /dev/nbd0p* never appear — force a reload.
modprobe nbd max_part=16 2>/dev/null || true
if [[ "$(cat /sys/module/nbd/parameters/max_part 2>/dev/null || echo 0)" == "0" ]]; then
  qemu-nbd --disconnect "$NBD" >/dev/null 2>&1 || true
  rmmod nbd 2>/dev/null || true
  modprobe nbd max_part=16
fi

qemu-nbd --read-only -c "$NBD" "$DISK"
udevadm settle 2>/dev/null || true
partprobe "$NBD" 2>/dev/null || true
partx -a "$NBD" 2>/dev/null || true
udevadm settle 2>/dev/null || true
sleep 1

echo "--- partition table on $NBD ---"
lsblk -f "$NBD" 2>/dev/null || ls -la "${NBD}"* 2>/dev/null
echo

# Find the root partition: the one that has /etc/os-release.
ROOT=""
# -o ro,noload: skip ext4 journal recovery.  The VM is often killed rather than
# cleanly shut down, leaving a dirty journal; plain `mount -o ro` then fails on a
# read-only nbd device because recovery needs to write.  noload mounts anyway.
for part in "${NBD}"p*; do
  [[ -b "$part" ]] || continue
  mount -o ro,noload "$part" "$MNT" 2>/dev/null \
    || mount -o ro "$part" "$MNT" 2>/dev/null \
    || continue
  if [[ -f "$MNT/etc/os-release" ]]; then ROOT="$part"; break; fi
  umount "$MNT" 2>/dev/null || true
done
if [[ -z "$ROOT" ]]; then
  echo "ERROR: could not find root partition on $DISK" >&2
  echo "       partitions seen: $(ls "${NBD}"p* 2>/dev/null | tr '\n' ' ')" >&2
  echo "       (if empty, nbd partition scan failed; if a btrfs @ subvol, tell me)" >&2
  exit 1
fi
echo "root partition: $ROOT"
echo "os-release: $(grep -E '^PRETTY_NAME' "$MNT/etc/os-release" 2>/dev/null)"
echo

USER_HOME=$(find "$MNT/home" -maxdepth 1 -mindepth 1 -type d | head -1)
UNAME=$(basename "$USER_HOME")
echo "===== installed user: $UNAME ====="
echo
echo "===== WALLPAPER ====="
echo "--- user appletsrc Image (what the desktop actually shows) ---"
grep -E "^Image=" "$USER_HOME/.config/plasma-org.kde.plasma.desktop-appletsrc" 2>/dev/null || echo "  (no user appletsrc)"
echo "--- system appletsrc Image (default) ---"
grep -E "^Image=" "$MNT/etc/xdg/plasma-org.kde.plasma.desktop-appletsrc" 2>/dev/null || echo "  (no system appletsrc)"
echo "--- wallpaper autostart present? ---"
ls -la "$MNT/etc/xdg/autostart/foundry-wallpaper.desktop" 2>/dev/null || echo "  MISSING from /etc/xdg/autostart"
ls -la "$USER_HOME/.config/autostart/foundry-wallpaper.desktop" 2>/dev/null || echo "  (not in user autostart)"
echo "--- wallpaper set-script present? ---"
ls -la "$MNT/usr/local/bin/foundry-set-wallpaper.sh" 2>/dev/null || echo "  MISSING"
echo "--- sentinel (if present in user home, the script SKIPS) ---"
ls -la "$USER_HOME/.config/foundry-wallpaper-set" 2>/dev/null && echo "  >>> SENTINEL PRESENT — script exits without setting wallpaper <<<" || echo "  (no sentinel — script should run)"
echo "--- sentinel wrongly seeded into /etc/skel? ---"
ls -la "$MNT/etc/skel/.config/foundry-wallpaper-set" 2>/dev/null && echo "  >>> SENTINEL IN SKEL — every new user skips wallpaper <<<" || echo "  (not in skel — good)"
echo "--- ForgeHorizon wallpaper present (foundry-kde-theme survived)? ---"
ls "$MNT/usr/share/wallpapers/FoundryLinux-ForgeHorizon/contents/images/" 2>/dev/null || echo "  MISSING — foundry-kde-theme was removed!"
echo "--- SDDM background present (calamares-settings survived)? ---"
ls -la "$MNT/usr/share/sddm/themes/foundry-linux/background.png" 2>/dev/null || echo "  absent (calamares-settings purged, as expected)"
echo "--- is calamares-settings still installed on target? ---"
ls "$MNT/var/lib/dpkg/info/calamares-settings-foundry-linux.list" 2>/dev/null && echo "  calamares-settings STILL on target" || echo "  calamares-settings purged from target"
echo "--- any cones/Kubuntu wallpaper refs in user config? ---"
grep -rEl "wallpapers/Kubuntu|Image=Kubuntu" "$USER_HOME/.config" 2>/dev/null || echo "  (none in user config)"
echo
echo "===== AUTOLOGIN ====="
echo "--- sddm autologin config on target (note the Session= value) ---"
for f in "$MNT/etc/sddm.conf" "$MNT"/etc/sddm.conf.d/*.conf; do
  [[ -f "$f" ]] || continue
  echo "  # $f"
  grep -nE "autologin|^\[Autologin\]|User=|Session=|Relogin" "$f" 2>/dev/null | sed 's/^/    /' || true
done
echo "--- session names SDDM can actually launch (Session= must match one bare name) ---"
ls "$MNT"/usr/share/wayland-sessions/*.desktop "$MNT"/usr/share/xsessions/*.desktop 2>/dev/null \
  | sed "s|$MNT||; s|^|    |" || echo "    (none found)"
echo "--- is sddm even enabled on target? ---"
ls -la "$MNT/etc/systemd/system/display-manager.service" 2>/dev/null | sed "s|$MNT||" || echo "    (display-manager.service not linked)"
echo "--- did our live sddm config leak to the target? (User=user from live) ---"
grep -rnE "User=user|Current=foundry" "$MNT"/etc/sddm.conf.d/ "$MNT/etc/sddm.conf" 2>/dev/null | sed "s|$MNT||; s/^/    /" || echo "    (no live sddm leakage)"
echo
echo "--- FULL content of each sddm.conf.d file (precedence: later wins) ---"
for f in "$MNT/etc/sddm.conf" "$MNT"/etc/sddm.conf.d/*.conf; do
  [[ -f "$f" ]] || continue
  echo "  ### ${f#"$MNT"}"
  sed 's/^/    /' "$f"
done
echo "--- autologin/nopasswdlogin groups + whether '$UNAME' is a member ---"
grep -E "^(autologin|nopasswdlogin):" "$MNT/etc/group" 2>/dev/null | sed 's/^/    /' || echo "    (no autologin/nopasswdlogin groups defined)"
echo "    secondary groups for $UNAME: $(awk -F: -v u="$UNAME" '$4 ~ ("(^|,)" u "(,|$)") {print $1}' "$MNT/etc/group" 2>/dev/null | tr '\n' ' ')"
echo "--- /etc/pam.d/sddm-autologin (does it gate on a group?) ---"
grep -vE "^#|^$" "$MNT/etc/pam.d/sddm-autologin" 2>/dev/null | sed 's/^/    /' || echo "    (no sddm-autologin PAM file)"

echo
echo "===== SDDM THEME (login screen background) ====="
echo "--- is the foundry-linux SDDM theme present on target? ---"
ls -d "$MNT/usr/share/sddm/themes/foundry-linux" 2>/dev/null && echo "    present" || echo "    >>> MISSING — purged with calamares-settings; Theme.Current=foundry-linux points at nothing → default cones login bg <<<"
echo "--- SDDM themes actually available on target ---"
ls "$MNT/usr/share/sddm/themes/" 2>/dev/null | sed 's/^/    /'

echo
echo "===== first-login session / LAF ====="
echo "--- active look-and-feel for user ---"
grep -E "LookAndFeelPackage|^ColorScheme" "$USER_HOME/.config/kdeglobals" 2>/dev/null || echo "  (no kdeglobals LAF set)"
echo
echo "done."
