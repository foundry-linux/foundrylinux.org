---
name: feedback_test_before_handoff
description: Always run static checks and smoke tests on any artifact before handing off to Will
metadata: 
  node_type: memory
  type: feedback
  originSessionId: 03faeb93-afb4-48a0-b1bd-f38075dca5f6
---

Before handing off any artifact, run every cheap static check available. Don't assume correctness from reading alone — actually invoke the validators.

**Why:** Caught a broken Taskfile task (KEEP=1 silently not exported as env var) only after Will ran it himself. A single test invocation would have caught it immediately. Applies to anything with a fast static checker, not just Taskfiles.

**How to apply — by artifact type:**

- **Before triggering a slow build:** audit every changed script first — `bash -n` + `shellcheck`. Neither catches single-quote-inside-single-quote bugs in `bash -c '...'` blocks; those require a manual read. Specifically: grep/sed patterns inside a `docker run ... bash -c '...'` block must use double quotes, not single quotes. Match the quoting style of passing checks in the same block.
- **Bash scripts:** `bash -n script.sh` (syntax); `shellcheck script.sh` if available
- **Taskfile tasks:** `task <new-task>` with representative args; test both the error path (missing required vars) and happy path (dry-run or --help)
- **JS/Node files:** `node --check file.js`; if it renders HTML, open and visually verify
- **Python:** `python -m py_compile file.py`
- **JSON/YAML:** `python -m json.tool file.json` or `yq . file.yaml`
- **Debian packaging:** `dpkg-deb --info` on the built .deb
- **Docker builds:** at minimum verify the image builds; run with --rm if fast

The bar is: run the fastest check that catches the most likely failure. Judge cost relative to the build cycle, not as an absolute number — a 30-second squashfs inspection against a 25-minute ISO build is 2% overhead and always worth it.

- **ISO / live-build:** After every build, verify critical config landed in the squashfs before booting QEMU:
  ```bash
  unsquashfs -n -cat dist/foundry-*.iso.squashfs etc/sddm.conf.d/30-foundry-live-autologin.conf
  unsquashfs -n -ll dist/foundry-*.iso.squashfs etc/skel/.config/ 2>/dev/null
  ```
  If the expected keys (`DisplayServer=`, `Current=foundry-linux`, skel appletsrc) are absent, the build used a cached chroot and the fix didn't land. Do not boot QEMU until this passes.

**Why the absolute-time framing fails:** "Under 10 seconds" made me skip the squashfs check (takes ~30s) on multiple builds, wasting 3× 25-minute QEMU test cycles trying to debug a problem that a single post-build grep would have surfaced immediately.
