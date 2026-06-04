# Plan: Flesh out foundrylinux.org landing page

## Context

The landing page is functional but has dead anchors, placeholder VM download links, no Docs landing, and no explanation of the multiple install paths. This pass cleans all of that up and adds the "four ways in" story that the distro proposal describes.

## Changes — `site/sections.jsx`

### 1. Topbar
- `#docs` → `#docs` (real anchor, will exist after change below)

### 2. Hero — no change

### 3. Install section — expand left column with channels grid

Add a 4-card "channels" row **above** the existing `<div className="codeblock">`. Each card is a compact pill/tile showing the install path:

| # | Label | Hook line | Key command |
|---|-------|-----------|-------------|
| 1 | **Existing Ubuntu** | "Already on 26.04? Add the repo." | `curl …/setup.sh \| bash` |
| 2 | **Container** | "Any Linux, macOS, or Windows host." | `distrobox create -i ghcr.io/foundry-linux/devbox:26.04` |
| 3 | **ISO / USB** | "Fresh install — boot and click." | Download button → `#download` anchor |
| 4 | **VM** | "VirtualBox · VMware · QEMU." | Coming soon (greyed, no link) |

The existing code block + ISO/VM download rows stay; the new grid is navigation/orientation above them.

**VM download rows** (VirtualBox, VMware, QEMU): mark as coming-soon — keep the rows, pre-fill the eventual URLs (`iso.foundrylinux.org/foundry-26.04-anvil.{ova,qcow2}`), replace `<a>` with `<span>` on the download icon, add `data-status="soon"` attribute. CSS dims and disables pointer-events. No `href="#"` dead links.

**Install sidebar** — expand the thin "The full ISO is 4.8 GB." line into 2–3 sentences that mention: Kubuntu 26.04 base (Plasma 6), Calamares installer, works on bare metal or VM.

### 4. Editions — no structural change; check blurbs in packages-data.json and update if thin

### 5. New `#docs` section (before `<Foot>`)

Minimal section: number tag "№ 04 · Docs", title "Find the source.", two-column link grid:

- **Source** — `github.com/foundry-linux` (the org)
- **Issues / Bugs** — `github.com/foundry-linux/foundrylinux.org/issues`
- **APT Repository** — `apt.foundrylinux.org`
- **Package catalogue** — `/packages`

No prose beyond a one-line blurb ("Everything is in the open.").

### 6. Footer

- `#docs` → `#docs` (real anchor)
- `#git` → `https://github.com/foundry-linux`
- `#bugs` → `https://github.com/foundry-linux/foundrylinux.org/issues`
- Remove `#mirrors` and `#irc` entries entirely (nothing exists at those destinations)

---

## Changes — `site/styles.css`

- `.vm-row[data-status="soon"]` — `opacity: 0.45; pointer-events: none;` + a tiny `SOON` pill badge on the download icon cell
- `.channels-grid` — 4-col flex row (collapses to 2-col at 700 px, 1-col at 500 px); card style consistent with `.forge-cell` but compact (no icon, just role + one-liner + code snippet)
- `.section#docs` — reuse `.section` + `.section-head` pattern; link grid with same hairline-border card style

---

## Files modified

- `site/sections.jsx` — all JSX changes above
- `site/styles.css` — three new rule blocks
- `site/index.html` — regenerated (never hand-edited)

---

## Build & deploy

```bash
task site-build          # regenerates index.html via ssr-render.js
task md -- site/index.html   # NOT markdown — skip; preview via browser instead
git add site/sections.jsx site/styles.css site/index.html
git commit
git tag vX.Y.Z && git push origin vX.Y.Z   # triggers Cloudflare Pages deploy
```

Preview before tagging: open `site/index.html` directly in browser (`file://…`) or `task site-preview` if that task exists.

---

## Verification

1. Open `foundrylinux.org` — topbar "Docs" and footer "Docs" scroll to `#docs` section.
2. Footer `#git` and `#bugs` open correct GitHub URLs; `#mirrors` and `#irc` are gone.
3. Install section shows 4-channel grid; channel 4 (VM) is visibly dimmed with SOON badge.
4. VM download rows (VirtualBox/VMware/QEMU) have no clickable link — cursor stays default.
5. `#docs` section renders with 4 links; all 4 hrefs are correct.
6. No console errors; no broken anchors (`#mirrors`, `#irc` no longer referenced anywhere).
