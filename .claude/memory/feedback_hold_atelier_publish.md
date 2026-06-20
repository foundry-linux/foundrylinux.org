---
name: hold-atelier-publish
description: Do not publish foundry-atelier to the live apt repo until Will explicitly directs it.
metadata: 
  node_type: memory
  type: feedback
  originSessionId: 46ec978a-30b6-4dec-ad4c-b08ff086a415
---

Do not publish `foundry-atelier` — no `task bump`, no `v*` tag, no triggering the CI publish workflow to apt.foundrylinux.org R2 — until Will explicitly says to. Committing/pushing source changes to git is fine; the gate is specifically on the **package going live**.

**Why:** Will directed this on 2026-06-20 while `creduce` was being added to `foundry-atelier` 0.9.6 (the test-case-reducer work). He wants to control when the complete-edition change ships — likely to batch it with other atelier/ISO work rather than push it live piecemeal.

**How to apply:** Make and commit atelier changes normally, but never tag / `task bump` / trigger the publish workflow for atelier without an explicit go-ahead. This is a standing hold until Will lifts it — re-confirm before any atelier publish. This **overrides** [[feedback_start_build_immediately]] for atelier specifically: do not auto-kick an atelier publish/ISO build off the back of this change. (`foundry-retro-tools` is not named in the hold, but still follow the normal "publish only when asked" flow.)
