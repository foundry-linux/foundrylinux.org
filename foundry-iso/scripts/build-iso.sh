#!/usr/bin/env bash
# Build a Foundry Linux ISO using live-build inside an ubuntu:26.04 container.
#
# Usage:
#   EDITION=anvil bash scripts/build-iso.sh
#   EDITION=atelier bash scripts/build-iso.sh
#
# Output: dist/foundry-<edition>-<version>-amd64.iso  (version from foundry-iso/VERSION)

set -euo pipefail

EDITION="${EDITION:-anvil}"
case "$EDITION" in
  anvil|atelier) ;;
  *) echo "EDITION must be one of: anvil, atelier" >&2; exit 1 ;;
esac

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
ISO_VERSION="$(cat "$SCRIPT_DIR/../VERSION")"
DIST_DIR="$REPO_ROOT/dist"

mkdir -p "$DIST_DIR"

echo "=== Fetching apt signing keys ==="
# Hardened against transient name-resolution / connection failures at this first
# network call: a momentary DNS blip (flaky upstream resolver, VPN tunnel hiccup,
# etc.) otherwise empties the pipe and aborts the whole multi-minute build.
# NOTE: curl does NOT retry name-resolution failures (error 6) unless
# --retry-all-errors is given.
fetch_key() {  # <url> <dest-keyfile>
  curl -fsSL --retry 5 --retry-delay 2 --retry-all-errors --retry-connrefused "$1" \
    | gpg --dearmor > "$2"
  [[ -s "$2" ]] || { echo "ERROR: fetched empty/invalid signing key from $1" >&2; exit 1; }
}
fetch_key https://apt.foundrylinux.org/key.gpg                               "$REPO_ROOT/config/archives/foundry.key"
fetch_key https://apt.worldfoundry.org/key.gpg                              "$REPO_ROOT/config/archives/worldfoundry.key"
fetch_key https://dl.cloudsmith.io/public/task/task/gpg.046FD1186CA342F0.key "$REPO_ROOT/config/archives/cloudsmith-task.key"
# Mozilla key must be binary (dearmored) — apt on 26.04 rejects ASCII-armored
# keys in trusted.gpg.d/, and live-build copies config/archives/*.key verbatim.
fetch_key https://packages.mozilla.org/apt/repo-signing-key.gpg              "$REPO_ROOT/config/archives/mozilla.key"

echo "=== Building foundry-${EDITION} ISO (inside ubuntu:26.04 container) ==="
docker run --rm \
  --privileged \
  --dns 8.8.8.8 \
  -e EDITION="$EDITION" \
  -e ISO_VERSION="$ISO_VERSION" \
  -v "$REPO_ROOT:/work" \
  -w /work \
  ubuntu:26.04 \
  bash -c '
    set -euo pipefail
    apt-get update -qq
    apt-get install -y --no-install-recommends \
      live-build curl gpg apt-utils ca-certificates \
      xorriso genisoimage isolinux grub-efi-amd64-bin grub-pc-bin mtools dosfstools
    cp /work/scripts/genisoimage-wrapper.sh /usr/local/bin/genisoimage
    chmod +x /usr/local/bin/genisoimage
    # Wipe sentinels + chroot so every run starts clean.  live-build sets chattr +i
    # on sentinel files; raw rm -rf fails on those even as root.  Strip immutable
    # flags first, then delete.  .cache/ is left intact so the bootstrap tarball
    # and apt packages are not re-downloaded.
    chattr -R -i .build/ chroot/ 2>/dev/null || true
    rm -rf .build/ chroot/
    rm -f .lock
    rm -rf config/includes.chroot/tmp
    bash config/auto/config
    # Inject local .debs (built but not yet published to apt.foundrylinux.org)
    # via config/includes.chroot/ rather than config/packages.chroot/:
    # packages.chroot/ triggers the lb local signed-repo code, which calls
    # "gpg --batch --gen-key" inside the chroot - broken in gnupg 2.4+ containers
    # (agent_genkey fails with "Inappropriate ioctl" and --secret-keyring is
    # obsolete).  includes.chroot/ copies files verbatim; a hook installs them.
    mkdir -p config/includes.chroot/tmp/local-debs
    if ls /work/local-debs/*.deb &>/dev/null; then
      # Deduplicate: for each package name keep only the newest version.
      # local-debs/ can accumulate stale builds; blindly copying all would
      # install an old version when a newer one is also present.
      declare -A _latest_deb
      for _deb in /work/local-debs/*.deb; do
        _pkg=$(dpkg-deb --field "$_deb" Package)
        _ver=$(dpkg-deb --field "$_deb" Version)
        if [[ -n "${_latest_deb[$_pkg]+x}" ]]; then
          _cur=$(dpkg-deb --field "${_latest_deb[$_pkg]}" Version)
          dpkg --compare-versions "$_ver" gt "$_cur" && _latest_deb[$_pkg]="$_deb"
        else
          _latest_deb[$_pkg]="$_deb"
        fi
      done
      for _deb in "${_latest_deb[@]}"; do
        echo "  staging local deb: $(basename "$_deb")"
        cp "$_deb" config/includes.chroot/tmp/local-debs/
      done
      unset _latest_deb _deb _pkg _ver _cur
    fi
    # Stage 1: debootstrap only
    lb bootstrap
    # Install custom apt keys now that chroot/ exists — live-build key
    # handling in ubuntu 3.0~a57 does not reliably copy *.key files before
    # running apt-get update, so we do it explicitly here.
    mkdir -p chroot/etc/apt/trusted.gpg.d/
    cp config/archives/foundry.key        chroot/etc/apt/trusted.gpg.d/foundry.key.gpg
    cp config/archives/worldfoundry.key  chroot/etc/apt/trusted.gpg.d/worldfoundry.key.gpg
    cp config/archives/cloudsmith-task.key chroot/etc/apt/trusted.gpg.d/cloudsmith-task.key.gpg
    cp config/archives/mozilla.key        chroot/etc/apt/trusted.gpg.d/mozilla.key.gpg
    # Copy apt preferences into the chroot (live-build 3.0~a57 does not reliably
    # apply config/apt/preferences.d/ before lb_chroot_package-lists runs).
    # Without this, the mozilla-firefox.pref and no-snapd.pref pins are never
    # seen by apt during package installation and snapd enters the chroot.
    mkdir -p chroot/etc/apt/preferences.d/
    cp config/apt/preferences.d/*.pref chroot/etc/apt/preferences.d/
    # Pre-install gnupg so apt-utils postinst finds gpg during lb_chroot_archives.
    # Debootstrap minbase does not include gpg; lb_chroot_archives installs apt-utils
    # whose postinst calls gpg and exits non-zero if not found, aborting the build.
    chroot chroot apt-get install -y --no-install-recommends gnupg
    # Stage 2
    lb chroot

    # Verify hooks ran — catch cached-chroot silences before packaging starts.
    # Checks the chroot directory directly; no squashfs extraction needed.
    echo "=== Verifying chroot hook output ==="
    SDDM_CONF="chroot/etc/sddm.conf.d/30-foundry-live.conf"
    if [[ ! -f "$SDDM_CONF" ]]; then
      echo "ERROR: $SDDM_CONF absent — hook 1100 did not run" >&2; exit 1
    fi
    grep -q "DisplayServer=" "$SDDM_CONF" || \
      { echo "ERROR: DisplayServer missing from sddm conf — old chroot cached?"; cat "$SDDM_CONF"; exit 1; }
    echo "PASS: $(cat "$SDDM_CONF")"
    CASPER_CONF="chroot/etc/casper.conf"
    if [[ ! -f "$CASPER_CONF" ]]; then
      echo "ERROR: $CASPER_CONF absent — hook 1100 did not write casper.conf" >&2; exit 1
    fi
    grep -q "USERNAME=" "$CASPER_CONF" || \
      { echo "ERROR: USERNAME= missing from casper.conf"; cat "$CASPER_CONF"; exit 1; }
    echo "PASS: $(cat "$CASPER_CONF")"

    # Guard the KDE config QML stack (plan 2026-05-30-full-kde-experience).
    # Plasmoid/KCM config dialogs load these QML modules at runtime; the
    # bloat-strip (hook 0020 + strip.list.chroot.purge) deliberately leaves them
    # in place.  A future over-broad strip-list glob (e.g. a 'kde-*' auto-remove)
    # could silently pull one and ship a distro whose config UIs come up blank.
    # Fail the build here instead.  See docs/investigations/2026-05-30-kde-app-kit.md.
    QML_DIR="chroot/usr/lib/x86_64-linux-gnu/qt6/qml"
    for m in org/kde/kcmutils org/kde/kquickcontrols QtQuick/Dialogs; do
      if [[ ! -d "$QML_DIR/$m" ]]; then
        echo "ERROR: KDE config QML module missing from chroot: $m" >&2
        echo "       (something stripped it — check hook 0020 / strip.list.chroot.purge)" >&2
        exit 1
      fi
    done
    echo "PASS: KDE config QML stack present (kcmutils, kquickcontrols, QtQuick.Dialogs)"

    # lb_binary_iso runs genisoimage and isohybrid inside the chroot via binary.sh.
    # Pre-populate both before lb binary so Check_package picks them up correctly:
    #   - genisoimage wrapper: routes through xorriso --allow-limited-size (squashfs > 4 GiB)
    #   - syslinux-utils: provides isohybrid (lb checks for syslinux pkg which lacks it)
    cp /work/scripts/genisoimage-wrapper.sh chroot/usr/bin/genisoimage
    chmod +x chroot/usr/bin/genisoimage
    # isohybrid patches the ISO MBR for legacy USB boot, but exits non-zero when
    # it cannot recognise the grub2 boot signature, aborting lb_binary_iso before
    # it moves the ISO out of the chroot.  We add UEFI boot ourselves below.
    printf "#!/bin/sh\nexit 0\n" > chroot/usr/bin/isohybrid
    chmod +x chroot/usr/bin/isohybrid
    # Stage 3
    lb binary

    # === UEFI / EFI boot injection ===
    # live-build 3.0~a57 on Ubuntu 26.04 does not generate EFI boot images in
    # binary_grub2.  Post-process the ISO with xorriso to inject an EFI System
    # Partition containing a GRUB EFI image built from the installed modules.
    #
    # Free build dirs first — the xorriso pass needs ~ISO-size of temp space.
    echo "=== Freeing build dirs before EFI injection ==="
    rm -rf binary/ chroot/

    echo "=== Building GRUB EFI image ==="
    EFI_WORK="/tmp/foundry-efi"
    mkdir -p "$EFI_WORK/EFI/BOOT"

    # Embedded grub.cfg: search for the ISO root by a known file, then load the
    # real grub.cfg.  This survives device reordering between machines.
    cat > "$EFI_WORK/grub.cfg" <<'"'"'GCFG'"'"'
search --set=root --file /live/filesystem.squashfs
set prefix=($root)/boot/grub
configfile ($root)/boot/grub/grub.cfg
GCFG

    grub-mkimage \
      --format=x86_64-efi \
      --prefix=/boot/grub \
      --output="$EFI_WORK/EFI/BOOT/BOOTX64.EFI" \
      --config="$EFI_WORK/grub.cfg" \
      iso9660 linux normal all_video search search_fs_file search_fs_uuid \
      search_label cat echo ls boot part_gpt part_msdos \
      fat loopback configfile font gfxterm gfxmenu png jpeg video \
      gfxterm_background

    # Create a 1 MiB FAT12 EFI System Partition image
    EFI_IMG="$EFI_WORK/efi.img"
    dd if=/dev/zero of="$EFI_IMG" bs=1k count=1024 2>/dev/null
    mkfs.fat -F12 -n "EFIBOOT" "$EFI_IMG"
    mmd -i "$EFI_IMG" ::/EFI ::/EFI/BOOT
    mcopy -i "$EFI_IMG" "$EFI_WORK/EFI/BOOT/BOOTX64.EFI" ::/EFI/BOOT/BOOTX64.EFI

    echo "=== Injecting EFI partition into ISO ==="
    xorriso \
      -dev binary.hybrid.iso \
      -map "$EFI_IMG" /boot/grub/efi.img \
      -boot_image any next \
      -boot_image any bin_path=/boot/grub/efi.img \
      -boot_image any platform_id=0xef \
      -boot_image any emul_type=no_emulation \
      -commit
    rm -rf "$EFI_WORK"
    echo "=== EFI boot support injected ==="
    # Make the ISO world-writable so the host user can run xorriso without sudo.
    chmod a+rw binary.hybrid.iso
  '

# Patch grub.cfg on the host (avoids bash -c single-quote escaping issues).
# -boot_image any keep preserves El Torito entries from the EFI injection above.
ISO_SRC="$REPO_ROOT/binary.hybrid.iso"
ISO_DST="$DIST_DIR/foundry-${EDITION}-${ISO_VERSION}-amd64.iso"
GRUB_PATCH="/tmp/foundry-grub-$$.cfg"
trap 'rm -f "$GRUB_PATCH"' EXIT
echo "=== Patching grub.cfg ==="
xorriso -osirrox on -dev "$ISO_SRC" -extract /boot/grub/grub.cfg "$GRUB_PATCH"
chmod +w "$GRUB_PATCH"
sed -i 's/^set default=0$/set default=0\nset timeout=5\nset gfxmode=auto\nset gfxpayload=keep/' "$GRUB_PATCH"
# Rename boot entries from live-build's generic "Debian GNU/Linux" labels.
# The full menuentry line is: menuentry "Debian GNU/Linux - live" {
# so match the quoted title including surrounding quotes, not just the title.
sed -i 's/menuentry "Debian GNU\/Linux - live" {/menuentry "Foundry Linux - Live" {/' "$GRUB_PATCH"
sed -i 's/menuentry "Debian GNU\/Linux - live (fail-safe mode)" {/menuentry "Foundry Linux - Live (safe mode)" {/' "$GRUB_PATCH"
sed -i 's/menuentry "Debian GNU\/Linux - live, kernel/menuentry "Foundry Linux - Live, kernel/' "$GRUB_PATCH"
# Insert "Install Foundry Linux" entry immediately after the first menuentry
# block, reusing its linux/initrd lines with automatic-calamares appended.
# Note: live-build uses tabs (not spaces) between linux/initrd and their args,
# so the regex must use \s+ not ' +' to match whitespace.
python3 - "$GRUB_PATCH" <<'PYEOF'
import sys, re
path = sys.argv[1]
with open(path) as f:
    content = f.read()
linux_match  = re.search(r'^(linux\s+.+)$',  content, re.MULTILINE)
initrd_match = re.search(r'^(initrd\s+.+)$', content, re.MULTILINE)
if not linux_match or not initrd_match:
    print('ERROR: could not find linux/initrd lines in grub.cfg', file=sys.stderr)
    sys.exit(1)
linux_line  = linux_match.group(1)
initrd_line = initrd_match.group(1)
install_entry = (
    '\nmenuentry "Install Foundry Linux" {\n'
    f'{linux_line} automatic-calamares\n'
    f'{initrd_line}\n'
    '}\n'
)
# Inject after the closing brace of the first menuentry block.
idx = content.index('\n}') + 2
content = content[:idx] + install_entry + content[idx:]
with open(path, 'w') as f:
    f.write(content)
print('Injected Install Foundry Linux GRUB entry')
PYEOF
# Strip live-build's default tga background lines — tga.mod is absent from the
# EFI grub image, causing "file not found" noise on every boot.
sed -i '/^insmod tga$/d' "$GRUB_PATCH"
sed -i '/^background_image /d' "$GRUB_PATCH"
xorriso -dev "$ISO_SRC" \
  -map "$GRUB_PATCH" /boot/grub/grub.cfg \
  -boot_image any keep \
  -commit
rm -f "$GRUB_PATCH"
echo "=== grub.cfg patched ==="

if [[ -f "$ISO_SRC" ]]; then
  mv "$ISO_SRC" "$ISO_DST"
  echo "=== ISO ready: $ISO_DST ($(du -h "$ISO_DST" | cut -f1)) ==="
else
  echo "ERROR: expected $ISO_SRC not found after lb build" >&2
  exit 1
fi
