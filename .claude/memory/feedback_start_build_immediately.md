---
name: start-build-immediately
description: "Kick off an ISO build the instant any ISO-affecting change lands; abort+restart in-flight builds when newer changes arrive"
metadata:
  node_type: memory
  type: feedback
  originSessionId: 786f77e8-30e0-47c9-9b9f-7917fb351bd8
---

The moment ANY change that affects the ISO lands, kick off `task iso-build` in the background as the very next action — before committing, before docs, before anything else. This is NOT limited to hooks or calamares-settings: it covers every `foundry-apt/packages/*` package, `foundry-iso/config/**` (hooks, package-lists, includes.chroot, binary/auto/config), and anything else baked into the image.

**Abort-and-restart:** if a build is already running when newer ISO-affecting changes land, abort it (TaskStop the running build) and start a fresh one immediately — a build in flight is already stale, so finishing it wastes the whole window. Always have the latest changes building.

**Don't gate on resources:** do not hold the build back over RAM/VM contention — Will overruled that explicitly (2026-06-10). Start it; if a test VM is competing, note it and suggest closing the VM, but don't delay the build.

**Why:** ISO builds take 30–60 minutes. Docs/commit/verification are free to do while it runs; doing them first wastes the window. Will flagged this twice (0.9.95 and the swap-hide on 0.9.98, 2026-06-10).

**How to apply:** change lands → (TaskStop any running build) → `task iso-build` (background) → commit + docs + investigation while it builds → review build output when notified. See [[iso-bump-greedy-commit]] (commit/clean staging first so the bump commit stays scoped).
