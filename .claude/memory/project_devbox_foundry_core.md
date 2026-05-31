---
name: devbox-installs-foundry-core
description: "Phase-2 devbox installs foundry-core (NOT foundry-anvil) on purpose — desktop-agnostic + small/fast test harnesses; don't bump to anvil"
metadata: 
  node_type: memory
  type: project
  originSessionId: fc0fa830-d719-4027-a80e-ea40edc4927d
---

`foundry-devbox/Dockerfile` installs **`foundry-core`**, not `foundry-anvil`. Nesting is `foundry-core ⊆ foundry-anvil ⊆ foundry-sprite ⊆ foundry-atelier`, where `foundry-anvil = foundry-core + foundry-desktop` and `foundry-desktop` pulls KDE/Plasma/Kirigami (`foundry-kde-theme`, `foundry-welcome`).

**Why:** Two reasons, both deliberate: (1) a headless Distrobox container has no business carrying a KDE desktop stack; (2) staying at the `core` rung keeps the **test/CI harnesses smaller and quicker to create and boot** — no desktop layer to pull or unpack. The lean base is a feature.

**How to apply:** Never "fix" the devbox image or its smoke tests to install `foundry-anvil` — that bloats the image and slows every container build/boot. Stale breadcrumbs that still say "foundry-anvil" (TODO.md line ~30, the foundry-devbox publish-workflow comment) are wrong and should be corrected to `foundry-core` when next touched.
