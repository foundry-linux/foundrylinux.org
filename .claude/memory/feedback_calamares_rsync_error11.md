---
name: feedback_calamares_rsync_error11
description: Calamares rsync error 11 = target disk exhausted; never add large packages to foundry.list.chroot
metadata: 
  node_type: memory
  type: feedback
  originSessionId: c46277d8-b172-4e3b-8a54-9e7d39080107
---

rsync error code 11 during Calamares installation means the target filesystem ran out of space while unpacking the squashfs. It does NOT indicate a corrupted squashfs or a bug in Calamares itself.

**Why:** Adding `openjdk-17-jre-headless` to `foundry.list.chroot` in 0.9.54 added ~270 MiB to the installed footprint and caused install failures on tighter target disks (confirmed 2026-06-05).

**How to apply:**
- Never add large packages to `foundry.list.chroot` as a shortcut. Use a chroot hook (`config/hooks/`) instead — the hook gets the same squashfs footprint but the intent is documented and reviewable.
- Packages that are `Depends:` of a foundry metapackage don't need a package-list entry; hook 0030's `apt-get install foundry-${EDITION}` satisfies them transitively.
- Hook `0028-jre-preinstall.hook.chroot` is the canonical home for the JRE 17 install, with a comment warning not to move it back to the package list.
- When a user reports rsync error 11 after a JRE/Ghidra/large-package addition, check the package-list diff first.

**Related:** [[feedback_test_before_handoff]] — the 0.9.54 install failure could have been caught by a pre-release smoke install.
