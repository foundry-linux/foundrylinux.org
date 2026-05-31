---
name: prep-binary-blob-parked
description: "prep (apt.worldfoundry.org) ships a binary blob not buildable source — known and parked, don't re-flag"
metadata: 
  node_type: memory
  type: project
  originSessionId: fc0fa830-d719-4027-a80e-ea40edc4927d
---

The `prep` package on apt.worldfoundry.org (`wbniv/worldfoundry.org`, `apt/packages/prep/`) ships a re-pack of a tracked binary blob, not buildable source — its lex/yacc grammar was retired upstream and `debian/control` notes it "will be restored in a follow-up." Will explicitly **parked** this on 2026-05-29.

**Why:** Audits flag it as a reproducibility-mandate violation (binary not rebuildable from source), but it's a known, accepted deferral — not an oversight. Re-surfacing it wastes a review cycle.

**How to apply:** Don't re-raise it as a finding in state-of-the-distro / reproducibility audits. If/when you touch `prep`, the goal is restoring the grammar so it builds from source; until then leave the binary re-pack as-is. Related: [[feedback_dropped_packages_todo]].
