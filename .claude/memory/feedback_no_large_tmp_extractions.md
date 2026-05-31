---
name: no-large-tmp-extractions
description: "Don't extract squashfs/large files to /tmp for inspection — use unsquashfs -l in-place or mount with loop"
metadata: 
  node_type: memory
  type: feedback
  originSessionId: c856fbfb-f86e-456a-bcc0-760a37510a47
---

Never extract a squashfs or other large archive to /tmp just to inspect its contents. /tmp is tmpfs (RAM-backed); a 6.6 GB squashfs fill it instantly and OOM/ENOSPC errors follow.

**Why:** Extracted a 6.6 GB filesystem.squashfs from an ISO to /tmp to grep dpkg/status — filled tmpfs, crashed subsequent commands.

**How to apply:** 
- To list squashfs contents: `unsquashfs -l <file> 'path/glob'`  
- To read a file from squashfs: `unsquashfs -p 1 -cat <file> path/to/file`  
- To inspect ISO contents without extracting: use `xorriso -osirrox on` with `-extract_single` only for small files, never for filesystem.squashfs  
- Always check `df -h /tmp` before any extraction; abort if less than 2× the source file size is available
