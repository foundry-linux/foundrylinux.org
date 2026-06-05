---
name: feedback-test-iso-before-handoff
description: Always boot and test ISO installation in QEMU before handing to the user for testing
metadata: 
  node_type: memory
  type: feedback
  originSessionId: 8ee2a7af-0bcb-4b3b-b629-c01a55a83c3b
---

Never hand an ISO to Will for testing without first validating it yourself:
1. Boot the ISO in QEMU
2. Click the "Install Foundry Linux" desktop icon
3. Verify the installation completes successfully

**Why:** Will found a broken install (rsync error 11) in 0.9.54 that should have been caught before handoff. The build completing is not sufficient — installation must work end-to-end.

**How to apply:** After every `task iso-build`, run a QEMU install test before reporting the ISO as ready. Only after the install succeeds should you tell Will it's ready to test.
