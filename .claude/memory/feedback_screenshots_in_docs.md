---
name: feedback-screenshots-in-docs
description: "When referencing the visual appearance of a website or webpage in a doc, always include a screenshot AND the URL — forgetting the URL is the common failure"
metadata: 
  node_type: memory
  type: feedback
  originSessionId: 9396d604-fa9d-402b-a207-c647a1945ea7
---

**Primary rule: always link the URL** whenever a doc mentions a website — whether or not a screenshot is present. This is the most commonly forgotten step. Will will notice.

When writing docs (investigations, plans, etc.) that reference the visual presentation of a website or webpage:

1. **Always link the URL first** — as `[display text](url)` inline or as a standalone line above the screenshot. Never mention a site without a link.
2. **Always attempt a screenshot** and embed it with `<img src="screenshots/name.png" width="700">` directly below the URL line.
3. If the screenshot fails (bot detection, 404, etc.), put the URL link in the doc with a note "*(screenshot needed)*" so Will can take it manually and drop it in.

**Why:** Screenshots make investigations immediately useful without requiring readers to open every link; the URL ensures the doc doesn't rot if the screenshot goes stale. And Will will notice if there's no link.

**How to apply:** Every time a doc section describes what a website looks like, add the URL link first, then the screenshot. Headless Chrome command: `google-chrome --headless=new --screenshot=path.png --window-size=1280,900 --hide-scrollbars "URL"`. packages.debian.org blocks headless browsers (Cloudflare); try Firefox headless as fallback; if still blocked, leave the URL link with *(screenshot needed)*.
