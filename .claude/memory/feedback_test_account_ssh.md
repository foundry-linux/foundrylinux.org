---
name: test-account-ssh
description: "For testing installed Foundry systems, Will creates account will / password foundry to SSH in — never bake a credential into the image"
metadata: 
  node_type: memory
  type: feedback
  originSessionId: 786f77e8-30e0-47c9-9b9f-7917fb351bd8
---

To debug an **installed** Foundry Linux test VM, SSH in as the account **`will`** with password **`foundry`** — Will creates that account during the install specifically for testing:

```
sshpass -p foundry ssh -o StrictHostKeyChecking=no -p 2222 will@localhost
```

(The live ISO autologs in and has its own debug SSH, but installs don't — Will's `will:foundry` account is how I reach the installed target.) It doubles as a check that the install account actually has **sudo** (`sudo -n true`).

**Why:** I need installed-system access to verify/debug, but these are **Will's** test credentials set per-install — NOT a credential shipped in the image.

**How to apply:** Use `will:foundry` over SSH (port 2222 in QEMU) for installed-system debugging. **NEVER bake a known credential into the build to get this access** — I once added `setRootPassword: false` so the squashfs `root:foundry` survived onto every install (calamares-settings 1.0.29), which is an inexcusable backdoor; reverted in 1.0.30. Real installs must manage root/passwords normally. The live-ssh debug hook + installed `sshd` are themselves test-only conveniences gated for removal before release (see TODO.md).
