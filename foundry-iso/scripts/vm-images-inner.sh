#!/usr/bin/env bash
# Inner script — runs INSIDE the Docker container during build-vm-images.sh.
# Not intended to be called directly.
# Required env: IMAGE_BASE

set -euo pipefail

ISO="/work/dist/${IMAGE_BASE}.iso"
DIST="/work/dist"
BUILD_TMP="/work/.vm-build-tmp"
mkdir -p "$BUILD_TMP"

cleanup() {
  umount "$BUILD_TMP/vm/run"      2>/dev/null || true
  umount "$BUILD_TMP/vm/sys"      2>/dev/null || true
  umount "$BUILD_TMP/vm/proc"     2>/dev/null || true
  umount "$BUILD_TMP/vm/dev/pts"  2>/dev/null || true
  umount "$BUILD_TMP/vm/dev"      2>/dev/null || true
  umount "$BUILD_TMP/vm/boot/efi" 2>/dev/null || true
  umount "$BUILD_TMP/vm"          2>/dev/null || true
  umount "$BUILD_TMP/iso"         2>/dev/null || true
  qemu-nbd --disconnect /dev/nbd0 2>/dev/null || true
  rm -rf "$BUILD_TMP"
}
trap cleanup EXIT

echo "[0/9] Installing tools..."
apt-get update -qq
apt-get install -y --no-install-recommends \
  qemu-utils squashfs-tools parted dosfstools e2fsprogs rsync ca-certificates kmod

modprobe nbd max_part=8

# No udev in container: find dynamic nbd major and create device node.
NBD_MAJOR=$(awk '$2 == "nbd" {print $1}' /proc/devices)
[[ -b /dev/nbd0 ]] || mknod /dev/nbd0 b "$NBD_MAJOR" 0

# ── 1: Mount ISO ──────────────────────────────────────────────────────────
echo "[1/9] Mounting ISO..."
mkdir -p "$BUILD_TMP/iso"
mount -o loop,ro "$ISO" "$BUILD_TMP/iso"

# ── 2: Extract squashfs ───────────────────────────────────────────────────
echo "[2/9] Extracting filesystem.squashfs (~5 min)..."
unsquashfs -d "$BUILD_TMP/rootfs" "$BUILD_TMP/iso/live/filesystem.squashfs"

# ── 3: Create 20 GB raw disk image ───────────────────────────────────────
echo "[3/9] Creating 20 GB disk image..."
RAW="$BUILD_TMP/foundry.raw"
qemu-img create -f raw "$RAW" 20G

# ── 4: Partition (GPT: 512 MB EFI + rest root) ───────────────────────────
echo "[4/9] Partitioning..."
parted -s "$RAW" mklabel gpt
parted -s "$RAW" mkpart primary fat32   1MiB  513MiB
parted -s "$RAW" mkpart primary ext4  513MiB  100%
parted -s "$RAW" set 1 esp on

qemu-nbd -f raw --connect=/dev/nbd0 "$RAW"
sleep 1
partprobe /dev/nbd0 2>/dev/null || true
sleep 2

# Create partition device nodes (no udev; with max_part=8: p1=minor1, p2=minor2)
[[ -b /dev/nbd0p1 ]] || mknod /dev/nbd0p1 b "$NBD_MAJOR" 1
[[ -b /dev/nbd0p2 ]] || mknod /dev/nbd0p2 b "$NBD_MAJOR" 2

mkfs.fat -F32 -n "FOUNDRY_EFI" /dev/nbd0p1
mkfs.ext4 -F -L "foundry-root" /dev/nbd0p2

# ── 5: Mount and copy filesystem ─────────────────────────────────────────
echo "[5/9] Copying filesystem (~5 min)..."
mkdir -p "$BUILD_TMP/vm"
mount /dev/nbd0p2 "$BUILD_TMP/vm"
mkdir -p "$BUILD_TMP/vm/boot/efi"
mount /dev/nbd0p1 "$BUILD_TMP/vm/boot/efi"

rsync -aHAX \
  --exclude="/proc/*"  --exclude="/sys/*"   --exclude="/dev/*" \
  --exclude="/run/*"   --exclude="/tmp/*" \
  "$BUILD_TMP/rootfs/" "$BUILD_TMP/vm/"

mkdir -p "$BUILD_TMP/vm/proc" "$BUILD_TMP/vm/sys" "$BUILD_TMP/vm/dev" \
         "$BUILD_TMP/vm/run"  "$BUILD_TMP/vm/tmp"
chmod 1777 "$BUILD_TMP/vm/tmp"

# ── 6: Configure for disk install ────────────────────────────────────────
echo "[6/9] Configuring..."

ROOT_UUID=$(blkid -s UUID -o value /dev/nbd0p2)
EFI_UUID=$(blkid  -s UUID -o value /dev/nbd0p1)

printf 'UUID=%s  /          ext4  defaults,noatime  0 1\nUUID=%s   /boot/efi  vfat  umask=0077        0 1\ntmpfs            /tmp       tmpfs defaults,nosuid,nodev  0 0\n' \
  "$ROOT_UUID" "$EFI_UUID" > "$BUILD_TMP/vm/etc/fstab"

echo "foundry-linux" > "$BUILD_TMP/vm/etc/hostname"
sed -i "/127\.0\.1\.1/d" "$BUILD_TMP/vm/etc/hosts" 2>/dev/null || true
echo "127.0.1.1  foundry-linux" >> "$BUILD_TMP/vm/etc/hosts"

# Default user: foundry / foundry
chroot "$BUILD_TMP/vm" useradd -m -s /bin/bash \
  -G "adm,audio,cdrom,dip,floppy,plugdev,sudo,video" foundry 2>/dev/null || true
echo "foundry:foundry" | chroot "$BUILD_TMP/vm" chpasswd

# SDDM autologin
mkdir -p "$BUILD_TMP/vm/etc/sddm.conf.d"
printf '[Autologin]\nUser=foundry\nSession=plasma\nRelogin=false\n' \
  > "$BUILD_TMP/vm/etc/sddm.conf.d/autologin.conf"

# Fresh machine-id on first boot
truncate -s 0 "$BUILD_TMP/vm/etc/machine-id" 2>/dev/null || true

# ── 7: Remove live-boot; install grub-efi; regenerate initramfs ──────────
echo "[7/9] Installing GRUB EFI + removing live-boot (~3 min)..."
rm -f "$BUILD_TMP/vm/etc/resolv.conf"
cp /etc/resolv.conf "$BUILD_TMP/vm/etc/resolv.conf"

mount --bind /dev           "$BUILD_TMP/vm/dev"
mount --bind /dev/pts       "$BUILD_TMP/vm/dev/pts" 2>/dev/null || true
mount -t proc  proc         "$BUILD_TMP/vm/proc"
mount -t sysfs sysfs        "$BUILD_TMP/vm/sys"
mount -t tmpfs tmpfs        "$BUILD_TMP/vm/run"

chroot "$BUILD_TMP/vm" env DEBIAN_FRONTEND=noninteractive bash -c '
  set -euo pipefail
  apt-get update -qq
  apt-get remove --purge -y \
    live-boot live-boot-initramfs-tools live-boot-doc \
    live-config live-config-systemd live-config-doc 2>/dev/null || true
  apt-get autoremove -y --purge 2>/dev/null || true
  apt-get install -y --no-install-recommends grub-efi-amd64
  grub-install \
    --target=x86_64-efi \
    --efi-directory=/boot/efi \
    --bootloader-id="Foundry Linux" \
    --no-nvram \
    --removable
  update-grub
  update-initramfs -u -k all
'

umount "$BUILD_TMP/vm/run"      2>/dev/null || true
umount "$BUILD_TMP/vm/sys"      2>/dev/null || true
umount "$BUILD_TMP/vm/proc"     2>/dev/null || true
umount "$BUILD_TMP/vm/dev/pts"  2>/dev/null || true
umount "$BUILD_TMP/vm/dev"      2>/dev/null || true

# Free squashfs temp space before conversion
umount "$BUILD_TMP/iso" 2>/dev/null || true
rm -rf "$BUILD_TMP/rootfs" "$BUILD_TMP/iso"
umount "$BUILD_TMP/vm/boot/efi"
umount "$BUILD_TMP/vm"
qemu-nbd --disconnect /dev/nbd0
sleep 1

# ── 8: Convert to qcow2 and VMDK ─────────────────────────────────────────
echo "[8/9] Converting to qcow2 and VMDK (~15 min)..."
qemu-img convert -p -f raw -O qcow2 -c \
  "$RAW" "$DIST/${IMAGE_BASE}.qcow2"

qemu-img convert -p -f raw -O vmdk \
  -o subformat=monolithicSparse \
  "$RAW" "$DIST/${IMAGE_BASE}.vmdk"

rm -f "$RAW"

# ── 9: Create OVA (OVF + VMDK tar) ──────────────────────────────────────
echo "[9/9] Creating OVA..."
VMDK_SIZE=$(stat -c %s "$DIST/${IMAGE_BASE}.vmdk")
VMDK_SHA1=$(sha1sum "$DIST/${IMAGE_BASE}.vmdk" | awk '{print $1}')

cat > "$DIST/${IMAGE_BASE}.ovf" << OVFEOF
<?xml version="1.0" encoding="UTF-8"?>
<Envelope xmlns="http://schemas.dmtf.org/ovf/envelope/1"
          xmlns:ovf="http://schemas.dmtf.org/ovf/envelope/1"
          xmlns:vssd="http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/CIM_VirtualSystemSettingData"
          xmlns:rasd="http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/CIM_ResourceAllocationSettingData">
  <References>
    <File ovf:href="${IMAGE_BASE}.vmdk" ovf:id="file1" ovf:size="${VMDK_SIZE}"/>
  </References>
  <DiskSection>
    <Info>Virtual disk information</Info>
    <Disk ovf:diskId="vmdisk1" ovf:fileRef="file1"
          ovf:capacity="21474836480" ovf:capacityAllocationUnits="byte"
          ovf:format="http://www.vmware.com/interfaces/specifications/vmdk.html#monolithicSparse"/>
  </DiskSection>
  <NetworkSection>
    <Info>Logical networks</Info>
    <Network ovf:name="NAT"><Description>NAT</Description></Network>
  </NetworkSection>
  <VirtualSystem ovf:id="${IMAGE_BASE}">
    <Info>Foundry Linux Anvil Edition — Ubuntu 26.04 LTS</Info>
    <Name>Foundry Linux Anvil</Name>
    <OperatingSystemSection ovf:id="101">
      <Info>Guest OS</Info>
      <Description>Ubuntu Linux (64-bit)</Description>
    </OperatingSystemSection>
    <VirtualHardwareSection>
      <Info>Virtual hardware requirements</Info>
      <System>
        <vssd:ElementName>Virtual Hardware Family</vssd:ElementName>
        <vssd:InstanceID>0</vssd:InstanceID>
        <vssd:VirtualSystemType>vmx-17</vssd:VirtualSystemType>
      </System>
      <Item>
        <rasd:Caption>2 virtual CPUs</rasd:Caption>
        <rasd:InstanceID>1</rasd:InstanceID>
        <rasd:ResourceType>3</rasd:ResourceType>
        <rasd:VirtualQuantity>2</rasd:VirtualQuantity>
      </Item>
      <Item>
        <rasd:AllocationUnits>MegaBytes</rasd:AllocationUnits>
        <rasd:Caption>4096 MB RAM</rasd:Caption>
        <rasd:InstanceID>2</rasd:InstanceID>
        <rasd:ResourceType>4</rasd:ResourceType>
        <rasd:VirtualQuantity>4096</rasd:VirtualQuantity>
      </Item>
      <Item>
        <rasd:Caption>SCSI controller</rasd:Caption>
        <rasd:InstanceID>3</rasd:InstanceID>
        <rasd:ResourceSubType>VirtualSCSI</rasd:ResourceSubType>
        <rasd:ResourceType>6</rasd:ResourceType>
      </Item>
      <Item>
        <rasd:AddressOnParent>0</rasd:AddressOnParent>
        <rasd:Caption>Disk image</rasd:Caption>
        <rasd:HostResource>ovf:/disk/vmdisk1</rasd:HostResource>
        <rasd:InstanceID>4</rasd:InstanceID>
        <rasd:Parent>3</rasd:Parent>
        <rasd:ResourceType>17</rasd:ResourceType>
      </Item>
      <Item>
        <rasd:AddressOnParent>0</rasd:AddressOnParent>
        <rasd:AutomaticAllocation>true</rasd:AutomaticAllocation>
        <rasd:Caption>Ethernet adapter</rasd:Caption>
        <rasd:Connection>NAT</rasd:Connection>
        <rasd:InstanceID>5</rasd:InstanceID>
        <rasd:ResourceSubType>E1000</rasd:ResourceSubType>
        <rasd:ResourceType>10</rasd:ResourceType>
      </Item>
    </VirtualHardwareSection>
  </VirtualSystem>
</Envelope>
OVFEOF

OVF_SHA1=$(sha1sum "$DIST/${IMAGE_BASE}.ovf" | awk '{print $1}')
printf 'SHA1 (%s.ovf)  = %s\nSHA1 (%s.vmdk) = %s\n' \
  "$IMAGE_BASE" "$OVF_SHA1" "$IMAGE_BASE" "$VMDK_SHA1" \
  > "$DIST/${IMAGE_BASE}.mf"

cd "$DIST"
tar --create --file="${IMAGE_BASE}.ova" \
  "${IMAGE_BASE}.ovf" \
  "${IMAGE_BASE}.vmdk" \
  "${IMAGE_BASE}.mf"

rm -f "${IMAGE_BASE}.ovf" "${IMAGE_BASE}.mf"

sha256sum "${IMAGE_BASE}.qcow2" > "${IMAGE_BASE}.qcow2.sha256"
sha256sum "${IMAGE_BASE}.vmdk"  > "${IMAGE_BASE}.vmdk.sha256"
sha256sum "${IMAGE_BASE}.ova"   > "${IMAGE_BASE}.ova.sha256"

echo "=== VM images ready ==="
ls -lh "${IMAGE_BASE}".{qcow2,vmdk,ova}
