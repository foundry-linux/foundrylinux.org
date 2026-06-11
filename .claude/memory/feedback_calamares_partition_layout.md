---
name: feedback-calamares-partition-layout
description: Calamares 3.3 partitionLayout mandatory keys and bootloader.conf requirement
metadata: 
  node_type: memory
  type: feedback
  originSessionId: 786f77e8-30e0-47c9-9b9f-7917fb351bd8
---

Calamares 3.3.14 `PartitionLayout::init()` requires BOTH `name` AND `size` as mandatory keys in each `partitionLayout` entry (checked with `!pentry.contains("name") || !pentry.contains("size")`). Without `name`, it silently switches to a default layout that creates no root partition — unsquashfs then extracts to the live tmpfs and fails with exit code 1.

Correct format:
```yaml
partitionLayout:
  - name:        "root"
    filesystem:  "ext4"
    mountPoint:  "/"
    size:        100%
```

Also mandatory: `bootloader.conf` must set `efiBootLoader: "grub"` — without it the bootloader module skips entirely and the installed system won't boot.

Both `grub-efi-amd64` AND `shim-signed` must be in the squashfs (`foundry.list.chroot`). `shim-signed` is required for Secure Boot on real hardware (the default on modern PCs). `shim-signed`'s postinst calls `update-alternatives` for `shimx64.efi.signed` which fails in the live-build chroot because `/boot/efi` isn't mounted — fix with a chroot hook that pre-creates `/boot/efi/EFI/ubuntu/` before the package is installed (`config/hooks/live/0050-shim-signed.hook.chroot`).

**Why:** Discovered by reading `PartitionLayout.cpp` source from GitHub after multiple failed attempts where `eraseModeFilesystem`, `defaultFileSystemType`, and `partitionLayout` without `name` all failed silently.

**How to apply:** Always include `name:` in partitionLayout entries and always ship `bootloader.conf`.
