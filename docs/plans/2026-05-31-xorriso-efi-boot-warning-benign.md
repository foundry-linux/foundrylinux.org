# Plan: mark the xorriso `/EFI/BOOT` warning as expected/benign (docs only — no code change)

## Context

`build-iso.sh` logs a libisofs warning: *"EFI boot equipment is provided but no directory /EFI/BOOT will emerge in the ISO filesystem…"*. It fires because the build provides UEFI boot **only** as the El Torito ESP entry (`-map "$EFI_IMG" /boot/grub/efi.img` + `platform_id=0xef`), with no loose `/EFI/BOOT/` tree in the ISO9660 filesystem.

**Decision (2026-05-31): do NOT add `/EFI/BOOT`; document the warning as benign.** Rationale:
- Every real USB writer — `isoimagewriter` / `usb-creator-kde` (what `docs/plans/2026-05-30-create-foundry-usb.md` uses), plus `dd`, Ventoy, balenaEtcher, Rufus DD-mode — **raw byte-copies the whole `.iso`** and boots via El Torito. Size- and filesystem-agnostic; **already works**. The 4.8 GB ISO is not too big for USB.
- The *only* method the missing `/EFI/BOOT` affects is the niche "extract ISO → copy files onto a FAT32 USB" trick. That is **independently impossible** for our editions because the squashfs is >4 GiB and FAT32 caps a single file at 4 GiB. Adding `/EFI/BOOT` would not rescue it.
- So the fix would be **purely cosmetic** (silence the warning + match Ubuntu's hybrid layout) with **no working USB path unblocked**. Not worth the rebuild/verification churn now.

## Change (docs only)

`foundry-iso/docs/investigations/2026-05-23-live-build-hooks-boot-theming.md`, §10 ("UEFI boot for ISOs >4 GiB", ~L163-180): append a short note —

- The libisofs `/EFI/BOOT will emerge` warning is **expected and benign / WONTFIX**. EFI is provided as the El Torito ESP entry only; no loose `/EFI/BOOT/` tree.
- Why benign: image writers raw-copy the ISO and boot via El Torito (unaffected); the copy-to-FAT32 method the warning targets is impractical anyway (squashfs >4 GiB > FAT32's 4 GiB file limit).
- Escape hatch for the future: if a sub-4 GiB edition ever ships and Windows file-copy support is wanted, add a loose tree with one line in the EFI-injection xorriso call — `-map "$EFI_WORK/EFI/BOOT/BOOTX64.EFI" /EFI/BOOT/BOOTX64.EFI` (the grub-mkimage binary already exists on disk; El Torito path stays unchanged).
- Don't re-flag the warning in audits.

## Critical files
- `foundry-iso/docs/investigations/2026-05-23-live-build-hooks-boot-theming.md` (only)

## Verification
- Doc-only; no rebuild. `task md -- foundry-iso/docs/investigations/2026-05-23-live-build-hooks-boot-theming.md` to preview, then commit.
