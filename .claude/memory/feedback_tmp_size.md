---
name: feedback-tmp-size
description: /tmp is a tmpfs with only ~2.6 GB — never extract large files there
metadata: 
  node_type: memory
  type: feedback
  originSessionId: 9745cedd-955e-4187-95c9-30298d2b1af1
---

`/tmp` had only 2.6 GB free when a 4.4 GB squashfs extraction was attempted there — it filled up and crashed the shell. The main disk's free space was irrelevant.

**Why:** Filled /tmp during ISO verification, killing the shell mid-session.

**How to apply:** Before extracting large files, check `df -h /tmp` specifically. For anything that might exceed available /tmp space, use `/home/will/` instead.
