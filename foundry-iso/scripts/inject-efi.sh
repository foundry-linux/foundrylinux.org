#!/usr/bin/env bash
# Inject UEFI/EFI boot support into an existing Foundry Linux ISO.
#
# Usage:
#   bash scripts/inject-efi.sh dist/foundry-anvil-1.0-amd64.iso
#
# Modifies the ISO in-place: adds /boot/grub/efi.img (FAT12 EFI System
# Partition containing BOOTX64.EFI) and registers it as the El Torito EFI
# boot entry.  Requires grub-efi-amd64-bin and xorriso on the host.
#
# Approach: live-build 3.0~a57 does not generate EFI boot images in
# binary_grub2.  xorriso -dev + -map creates a pending session so -commit
# writes.  (-indev/-outdev with only -append_partition creates no session
# data and silently produces no output — confirmed 2026-05-23.)

set -euo pipefail

usage() {
  echo "Usage: $(basename "$0") [--output <out-path>] <iso-path>"
  echo "  Without --output: modifies in-place (requires write access to iso-path)"
  echo "  With --output:    writes to out-path (only needs read access to iso-path)"
  exit 0
}

[[ "${1:-}" == "-h" || "${1:-}" == "--help" ]] && usage

OUTPUT_PATH=""
if [[ "${1:-}" == "--output" ]]; then
  OUTPUT_PATH="$2"
  shift 2
fi

[[ $# -lt 1 ]] && { echo "Error: ISO path required" >&2; usage; }

ISO_PATH="$1"
[[ -f "$ISO_PATH" ]] || { echo "Error: not found: $ISO_PATH" >&2; exit 1; }

for cmd in grub-mkimage xorriso mkfs.fat mmd mcopy dd; do
  command -v "$cmd" >/dev/null || { echo "Error: $cmd not found" >&2; exit 1; }
done

EFI_WORK="$(mktemp -d /tmp/foundry-efi.XXXXXX)"
trap 'rm -rf "$EFI_WORK"' EXIT

echo "=== Building GRUB EFI image for $ISO_PATH ==="

mkdir -p "$EFI_WORK/EFI/BOOT"

cat > "$EFI_WORK/grub.cfg" <<'GCFG'
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

echo "  BOOTX64.EFI: $(wc -c < "$EFI_WORK/EFI/BOOT/BOOTX64.EFI") bytes"

EFI_IMG="$EFI_WORK/efi.img"
dd if=/dev/zero of="$EFI_IMG" bs=1k count=1024 2>/dev/null
mkfs.fat -F12 -n "EFIBOOT" "$EFI_IMG"
mmd -i "$EFI_IMG" ::/EFI ::/EFI/BOOT
mcopy -i "$EFI_IMG" "$EFI_WORK/EFI/BOOT/BOOTX64.EFI" ::/EFI/BOOT/BOOTX64.EFI

if [[ -n "$OUTPUT_PATH" ]]; then
  echo "=== Injecting EFI partition: $ISO_PATH → $OUTPUT_PATH ==="
  xorriso \
    -indev "$ISO_PATH" \
    -outdev "$OUTPUT_PATH" \
    -map "$EFI_IMG" /boot/grub/efi.img \
    -boot_image any next \
    -boot_image any bin_path=/boot/grub/efi.img \
    -boot_image any platform_id=0xef \
    -boot_image any emul_type=no_emulation \
    -commit
  echo "=== EFI boot support written to $OUTPUT_PATH ==="
else
  echo "=== Injecting EFI partition into ISO (in-place) ==="
  xorriso \
    -dev "$ISO_PATH" \
    -map "$EFI_IMG" /boot/grub/efi.img \
    -boot_image any next \
    -boot_image any bin_path=/boot/grub/efi.img \
    -boot_image any platform_id=0xef \
    -boot_image any emul_type=no_emulation \
    -commit
  echo "=== EFI boot support injected into $ISO_PATH ==="
fi
