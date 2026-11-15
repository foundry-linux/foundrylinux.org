#!/usr/bin/env bash
# Tiny synthetic ISO: validates combined menu/EFI edit, not a distro boot test.
set -euo pipefail
mkdir -p /tmp/menu-test/tree/boot/grub
cd /tmp/menu-test
cat > tree/boot/grub/grub.cfg <<'EOF'
set default=0
insmod tga
background_image /boot/grub/splash.tga
menuentry "Debian GNU/Linux - live" {
linux /live/vmlinuz boot=casper quiet splash
initrd /live/initrd.img
}
menuentry "Debian GNU/Linux - live (fail-safe mode)" {
linux /live/vmlinuz boot=casper nosplash
initrd /live/initrd.img
}
EOF
cp /usr/lib/ISOLINUX/isolinux.bin tree/
xorriso -as mkisofs -quiet -o fixture.iso -b isolinux.bin -c boot.cat \
    -no-emul-boot -boot-load-size 4 -boot-info-table tree
xorriso -osirrox on -dev fixture.iso -extract /boot/grub/grub.cfg menu.cfg
chmod +w menu.cfg
python3 /work/scripts/patch-grub.py menu.cfg
test "$(grep -c 'menuentry ' menu.cfg)" = 3
grep -q 'automatic-calamares' menu.cfg
grep -q 'Foundry Linux - Live (safe mode)' menu.cfg
if grep -q tga menu.cfg; then exit 1; fi
truncate -s 1M efi.img
mkfs.fat -F12 efi.img
xorriso -dev fixture.iso -boot_image any replay -map efi.img /boot/grub/efi.img \
    -map menu.cfg /boot/grub/grub.cfg -boot_image any next \
    -boot_image any bin_path=/boot/grub/efi.img -boot_image any platform_id=0xef \
    -boot_image any emul_type=no_emulation -commit
xorriso -osirrox on -dev fixture.iso -extract /boot/grub/grub.cfg actual.cfg
cmp menu.cfg actual.cfg
xorriso -indev fixture.iso -report_el_torito plain > boot-record.txt 2>&1
grep -q BIOS boot-record.txt
grep -q UEFI boot-record.txt
printf 'bad menu\n' > invalid.cfg
if python3 /work/scripts/patch-grub.py invalid.cfg 2>/dev/null; then exit 1; fi
echo 'PASS: combined ISO commit preserves BIOS, adds EFI, installs menu and rejects malformed input'
