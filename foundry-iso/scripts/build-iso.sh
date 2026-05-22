#!/usr/bin/env bash
# Build a Foundry Linux ISO using live-build inside an ubuntu:26.04 container.
#
# Usage:
#   EDITION=anvil bash scripts/build-iso.sh
#   EDITION=atelier bash scripts/build-iso.sh
#
# Output: dist/foundry-<edition>-1.0-amd64.iso

set -euo pipefail

EDITION="${EDITION:?EDITION env var required: anvil or atelier}"
case "$EDITION" in
  anvil|atelier) ;;
  *) echo "EDITION must be one of: anvil, atelier" >&2; exit 1 ;;
esac

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
DIST_DIR="$REPO_ROOT/dist"

mkdir -p "$DIST_DIR"

echo "=== Fetching apt signing keys ==="
curl -fsSL https://apt.foundrylinux.org/key.gpg \
  | gpg --dearmor > "$REPO_ROOT/config/archives/foundry.key"
curl -fsSL https://apt.worldfoundry.org/key.gpg \
  | gpg --dearmor > "$REPO_ROOT/config/archives/worldfoundry.key"
curl -fsSL https://dl.cloudsmith.io/public/task/task/gpg.046FD1186CA342F0.key \
  | gpg --dearmor > "$REPO_ROOT/config/archives/cloudsmith-task.key"

echo "=== Building foundry-${EDITION} ISO (inside ubuntu:26.04 container) ==="
docker run --rm \
  --privileged \
  -e EDITION="$EDITION" \
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
    lb clean --purge 2>/dev/null || true
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
      cp /work/local-debs/*.deb config/includes.chroot/tmp/local-debs/
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
    # Pre-install gnupg so apt-utils postinst finds gpg during lb_chroot_archives.
    # Debootstrap minbase does not include gpg; lb_chroot_archives installs apt-utils
    # whose postinst calls gpg and exits non-zero if not found, aborting the build.
    chroot chroot apt-get install -y --no-install-recommends gnupg
    # Stage 2
    lb chroot
    # lb_binary_iso runs genisoimage and isohybrid inside the chroot via binary.sh.
    # Pre-populate both before lb binary so Check_package picks them up correctly:
    #   - genisoimage wrapper: routes through xorriso --allow-limited-size (squashfs > 4 GiB)
    #   - syslinux-utils: provides isohybrid (lb checks for syslinux pkg which lacks it)
    cp /work/scripts/genisoimage-wrapper.sh chroot/usr/bin/genisoimage
    chmod +x chroot/usr/bin/genisoimage
    # isohybrid patches the ISO MBR for legacy USB boot, but exits non-zero when
    # it cannot recognise the grub2 boot signature, aborting lb_binary_iso before
    # it moves the ISO out of the chroot.  grub2 EFI boot works without it.
    printf "#!/bin/sh\nexit 0\n" > chroot/usr/bin/isohybrid
    chmod +x chroot/usr/bin/isohybrid
    # Stage 3
    lb binary
  '

# live-build writes the ISO to the working directory as binary.hybrid.iso
ISO_SRC="$REPO_ROOT/binary.hybrid.iso"
ISO_DST="$DIST_DIR/foundry-${EDITION}-1.0-amd64.iso"

if [[ -f "$ISO_SRC" ]]; then
  mv "$ISO_SRC" "$ISO_DST"
  echo "=== ISO ready: $ISO_DST ($(du -h "$ISO_DST" | cut -f1)) ==="
else
  echo "ERROR: expected $ISO_SRC not found after lb build" >&2
  exit 1
fi
