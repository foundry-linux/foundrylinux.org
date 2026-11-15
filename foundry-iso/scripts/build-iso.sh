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

# The Docker daemon socket needs the 'docker' group. `task dev-setup` runs
# `usermod -aG docker`, but that does not affect an already-open login session,
# so a fresh interactive shell may not have the group active yet. If the daemon
# is unreachable but this user IS a recorded member, re-exec once under `sg` to
# activate the group for this run -- no password (membership already exists), no
# logout/newgrp. The sentinel prevents an infinite loop if the daemon is truly
# down (member, but sg still can't reach it).
if [[ -z "${_FOUNDRY_SG_REEXEC:-}" ]] \
   && ! docker info >/dev/null 2>&1 \
   && getent group docker | grep -qw "${USER:-$(id -un)}"; then
  echo "=== docker group not active in this shell; activating via sg for this run ==="
  reexec_env=("_FOUNDRY_SG_REEXEC=1" "EDITION=$EDITION")
  for option in FOUNDRY_ISO_TOOLS_IMAGE FOUNDRY_ISO_TOOLS_REVISION FOUNDRY_ISO_THREADS FOUNDRY_ISO_MEMORY_MIB FOUNDRY_ALLOW_OLDER_LOCAL; do
    [[ ! -v "$option" ]] || reexec_env+=("$option=${!option}")
  done
  printf -v reexec_command '%q ' env "${reexec_env[@]}" bash "$0"
  exec sg docker -c "$reexec_command"
fi

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
ISO_VERSION="$(cat "$SCRIPT_DIR/../VERSION")"
DIST_DIR="$REPO_ROOT/dist"
ISO_TOOLS_IMAGE="${FOUNDRY_ISO_TOOLS_IMAGE:-foundry-iso-tools:local}"
# The pinned Dockerfile layer is reused until its digest/revision changes.
# Set this revision when deliberately refreshing APT toolchain packages.
ISO_TOOLS_REVISION="${FOUNDRY_ISO_TOOLS_REVISION:-2026-11-15}"
ISO_THREADS="${FOUNDRY_ISO_THREADS:-2}"
ISO_MEMORY_MIB="${FOUNDRY_ISO_MEMORY_MIB:-512}"
[[ "$ISO_THREADS" =~ ^[1-9][0-9]*$ && "$ISO_MEMORY_MIB" =~ ^[1-9][0-9]*$ ]] || {
  echo "ERROR: FOUNDRY_ISO_THREADS and FOUNDRY_ISO_MEMORY_MIB must be positive integers" >&2
  exit 1
}

# A disconnected or failed external drive can return as read-only after an I/O
# error. Stop before Docker or live-build modifies the workspace.
MOUNT_OPTIONS="$(findmnt -no OPTIONS -T "$REPO_ROOT" 2>/dev/null || true)"
if [[ ",$MOUNT_OPTIONS," == *,ro,* ]]; then
  echo "ERROR: ISO workspace is on a read-only mount: $REPO_ROOT" >&2
  echo "       Check the drive connection and filesystem before retrying." >&2
  exit 1
fi

# A rootless Docker bind mount is always nodev, even when the host filesystem
# permits device nodes. live-build needs working /dev entries in its chroot, so
# run this build against the standard daemon through one desktop auth prompt.
if [[ "$(id -u)" != 0 ]] \
   && docker info --format '{{.SecurityOptions}}' 2>/dev/null | grep -q rootless; then
  echo "=== Rootless Docker cannot build this ISO; switching to the standard daemon ==="
  exec pkexec /usr/bin/env \
    DOCKER_HOST=unix:///var/run/docker.sock \
    FOUNDRY_HOST_UID="$(id -u)" \
    FOUNDRY_HOST_GID="$(id -g)" \
    EDITION="$EDITION" \
    FOUNDRY_ISO_TOOLS_IMAGE="$ISO_TOOLS_IMAGE" \
    FOUNDRY_ISO_TOOLS_REVISION="$ISO_TOOLS_REVISION" \
    FOUNDRY_ISO_THREADS="$ISO_THREADS" \
    FOUNDRY_ISO_MEMORY_MIB="$ISO_MEMORY_MIB" \
    FOUNDRY_ALLOW_OLDER_LOCAL="${FOUNDRY_ALLOW_OLDER_LOCAL:-0}" \
    /bin/bash "$SCRIPT_DIR/build-iso.sh"
fi

# live-build creates device nodes inside the chroot. Desktop automounters often
# mount removable ext4 drives with nodev, which turns /dev/null into an unusable
# regular file in the chroot and eventually breaks package triggers.
if [[ ",$MOUNT_OPTIONS," == *,nodev,* && "$(id -u)" == 0 \
      && -n "${FOUNDRY_HOST_UID:-}" ]]; then
  MOUNT_POINT="$(findmnt -no TARGET -T "$REPO_ROOT")"
  if [[ "$MOUNT_POINT" == /run/media/* || "$MOUNT_POINT" == /media/* ]]; then
    echo "=== Enabling device nodes on $MOUNT_POINT for live-build ==="
    mount -o remount,dev "$MOUNT_POINT"
    MOUNT_OPTIONS="$(findmnt -no OPTIONS -T "$REPO_ROOT")"
  fi
fi
if [[ ",$MOUNT_OPTIONS," == *,ro,* ]]; then
  echo "ERROR: ISO workspace became read-only: $REPO_ROOT" >&2
  exit 1
fi
if [[ ",$MOUNT_OPTIONS," == *,nodev,* ]]; then
  echo "ERROR: ISO workspace is on a nodev mount: $REPO_ROOT" >&2
  echo "       Remount it with dev before building (sudo mount -o remount,dev <mountpoint>)." >&2
  exit 1
fi

docker build --pull=false --build-arg "TOOLCHAIN_REVISION=$ISO_TOOLS_REVISION" \
  -t "$ISO_TOOLS_IMAGE" -f "$REPO_ROOT/docker/iso-builder.Dockerfile" "$REPO_ROOT/docker"
ISO_TOOLS_ID=$(docker image inspect --format '{{.Id}}' "$ISO_TOOLS_IMAGE")
mkdir -p "$DIST_DIR"
docker image inspect "$ISO_TOOLS_ID" > "$DIST_DIR/iso-builder-image.json"
printf 'threads=%s\nmemory_mib=%s\n' "$ISO_THREADS" "$ISO_MEMORY_MIB" > "$DIST_DIR/iso-compression-settings.txt"

# Prune old local ISOs for this edition before building to prevent disk exhaustion.
# Keeps build logs (tiny); removes ISO + sidecars (.sha256 .asc .torrent) only.
echo "=== Pruning old dist/ ISOs for ${EDITION} ==="
find "$DIST_DIR" -maxdepth 1 \( \
    -name "foundry-${EDITION}-*-amd64.iso"            \
    -o -name "foundry-${EDITION}-*-amd64.iso.sha256"  \
    -o -name "foundry-${EDITION}-*-amd64.iso.asc"     \
    -o -name "foundry-${EDITION}-*-amd64.iso.torrent" \
\) -exec rm -f {} + && echo "  done"

# lb build runs as root inside Docker on the bind-mounted tree (-v REPO_ROOT:/work)
# and leaves root-owned files in config/ (and dist/). Chown them back to the invoking
# user on exit -- success OR failure -- so later edits and git work without sudo. A
# tiny root container does the chown since the host user cannot chown files it does not
# own. All temporary bootloader files live inside the disposable build container.
_cleanup() {
  # In rootless Docker, container UID 0 maps to the invoking host user.
  # Chowning to the host UID inside that container instead maps files to a
  # subordinate host UID and makes subsequent builds unable to update keys.
  local owner="${FOUNDRY_HOST_UID:-$(id -u)}:${FOUNDRY_HOST_GID:-$(id -g)}"
  if docker info --format '{{.SecurityOptions}}' 2>/dev/null | grep -q rootless; then
    owner="0:0"
  fi
  docker run --rm -v "$REPO_ROOT:/work" "$ISO_TOOLS_ID" \
    chown -R "$owner" /work/config /work/dist >/dev/null 2>&1 || true
}
trap _cleanup EXIT

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

echo "=== Building foundry-${EDITION} ISO (tools $ISO_TOOLS_ID) ==="
docker run --rm \
  --privileged \
  --network host \
  -e EDITION="$EDITION" \
  -e ISO_VERSION="$ISO_VERSION" \
  -e FOUNDRY_ALLOW_OLDER_LOCAL="${FOUNDRY_ALLOW_OLDER_LOCAL:-0}" \
  -e MKSQUASHFS_OPTIONS="-processors $ISO_THREADS -mem ${ISO_MEMORY_MIB}M" \
  -v "$REPO_ROOT:/work" \
  -w /work \
  "$ISO_TOOLS_ID" \
  bash -c '
    set -euo pipefail
    if [[ ",$(findmnt -no OPTIONS -T /work)," == *,nodev,* ]]; then
      echo "ERROR: Docker still sees /work as nodev; live-build cannot use /dev in the chroot" >&2
      exit 1
    fi
    cp /usr/local/share/foundry-iso-toolchain.tsv /work/dist/iso-toolchain.tsv
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
    # Local packages are indexed before the initial edition installation, rather
    # than installed a second time from an includes.chroot payload.
    : > config/package-lists/local-builds.list.chroot
    # Patch bootstrap cache DNS before lb bootstrap runs, but ONLY when the cache
    # actually contains a real Ubuntu base system.  lb_bootstrap_cache restore checks
    # `if [ -d cache/bootstrap ]` — if we unconditionally create that directory (even
    # empty) before lb bootstrap, restore treats it as a cache hit, copies just our DNS
    # stub into chroot/, marks bootstrap done, and exits without running debootstrap.
    # Guard on cache/bootstrap/usr so a fresh or deleted cache does not trigger the bug.
    # Host networking lets the chroot use the host resolver, including a local
    # systemd-resolved stub. Hard-coded public DNS can be blocked by the host VPN.
    if [ -d cache/bootstrap/usr ]; then
      mkdir -p cache/bootstrap/run/systemd/resolve
      cp /etc/resolv.conf cache/bootstrap/run/systemd/resolve/stub-resolv.conf
    fi
    # Stage 1: debootstrap only
    lb bootstrap
    # Fix DNS in the freshly created chroot before any apt calls.  lb bootstrap
    # may restore from cache (preserving stale DNS) or run fresh debootstrap
    # (where systemd writes its own stub-resolv.conf during package configure).
    # Either way, overwrite immediately so the gnupg install below succeeds.
    mkdir -p chroot/run/systemd/resolve
    cp /etc/resolv.conf chroot/run/systemd/resolve/stub-resolv.conf
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
    if [[ -f "config/apt/exclusions/$EDITION.pref" ]]; then
      cp "config/apt/exclusions/$EDITION.pref" chroot/etc/apt/preferences.d/foundry-edition-exclusions.pref
    fi
    # Pre-install gnupg so apt-utils postinst finds gpg during lb_chroot_archives.
    # Debootstrap minbase does not include gpg; lb_chroot_archives installs apt-utils
    # whose postinst calls gpg and exits non-zero if not found, aborting the build.
    chroot chroot apt-get install -y --no-install-recommends gnupg
    if compgen -G "/work/local-debs/*.deb" >/dev/null; then
      mkdir -p chroot/etc/apt/sources.list.d/
      for archive in config/archives/*.list.chroot; do
        cp "$archive" "chroot/etc/apt/sources.list.d/$(basename "$archive" .list.chroot).list"
      done
      chroot chroot apt-get update -o APT::Update::Error-Mode=any
      local_options=()
      [[ "$FOUNDRY_ALLOW_OLDER_LOCAL" != 1 ]] || local_options+=(--allow-older)
      python3 scripts/prepare-local-apt.py --root chroot --archives /work/local-debs \
        --edition "$EDITION" --package-list config/package-lists/local-builds.list.chroot \
        "${local_options[@]}"
    fi
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

    echo "=== Preparing Foundry boot menu ==="
    GRUB_PATCH="$EFI_WORK/boot-menu.cfg"
    xorriso -osirrox on -dev binary.hybrid.iso -extract /boot/grub/grub.cfg "$GRUB_PATCH"
    chmod +w "$GRUB_PATCH"
    python3 /work/scripts/patch-grub.py "$GRUB_PATCH"

    echo "=== Injecting EFI partition and boot menu into ISO ==="
    xorriso \
      -dev binary.hybrid.iso \
      -boot_image any replay \
      -map "$EFI_IMG" /boot/grub/efi.img \
      -map "$GRUB_PATCH" /boot/grub/grub.cfg \
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

ISO_SRC="$REPO_ROOT/binary.hybrid.iso"
ISO_DST="$DIST_DIR/foundry-${EDITION}-${ISO_VERSION}-amd64.iso"

if [[ -f "$ISO_SRC" ]]; then
  mv "$ISO_SRC" "$ISO_DST"
  echo "=== ISO ready: $ISO_DST ($(du -h "$ISO_DST" | cut -f1)) ==="
else
  echo "ERROR: expected $ISO_SRC not found after lb build" >&2
  exit 1
fi
