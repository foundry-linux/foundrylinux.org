---
name: feedback-vm-before-launch
description: "Before launching a test VM, check if one is already open to avoid OOM crash"
metadata: 
  node_type: memory
  type: feedback
  originSessionId: fde73f63-c235-451d-b266-05d8a811afe6
---

Before launching any test VM (VirtualBox, QEMU, etc.) for ISO testing, check whether a VM is already running.

**Why:** Launching a second VM while one is already open caused an OOM condition and crashed the computer.

**How to apply:**
- Before booting a new VM instance, check for running VMs (e.g. `VBoxManage list runningvms` or `pgrep -a qemu`).
- If one is already open: if we just need to test something in the existing session, give Will the instructions to run inside that VM — don't launch a new one.
- If we genuinely need a fresh boot (clean state, new ISO), prompt Will to close the existing VM first before proceeding.
- Never silently spin up a second VM.
