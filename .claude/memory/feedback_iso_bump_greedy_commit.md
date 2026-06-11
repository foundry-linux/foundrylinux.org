---
name: iso-bump-greedy-commit
description: "task iso-build's iso-bump does a bare `git commit` that sweeps ALL staged changes into the version-bump commit"
metadata: 
  node_type: memory
  type: feedback
  originSessionId: 786f77e8-30e0-47c9-9b9f-7917fb351bd8
---

`task iso-build` → `iso-bump` runs `git add VERSION && git commit`. The bare `git commit` (no pathspec) commits **everything already staged**, not just VERSION. So any staged work (e.g. `git mv` renames, which stage immediately) gets swept into the `chore: bump ISO version to X` commit and mislabeled.

**Why:** Happened on 2026-06-10 (0.9.97): a `git mv` of the SDDM theme files landed inside the bump commit instead of the feature commit.

**How to apply:** Before running `task iso-build`, either commit or unstage your work so nothing is staged when iso-bump fires. Proper fix (TODO): change `bump-version.sh`/the iso-bump task to `git commit -- foundry-iso/VERSION` so it only commits VERSION. Related: [[start-build-immediately]] — committing first also satisfies this.
