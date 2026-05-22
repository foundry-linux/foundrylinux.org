# Plan: Homepage update + torrent/magnet support

## Goal

Update foundrylinux.org homepage with the correct atelier ISO size (15 GB actual, not the old ~10 GB estimate), and add `.torrent` and magnet link download options to both the homepage and the iso.foundrylinux.org index page.

## Changes

### New: `foundry-iso/scripts/create-torrents.sh`

- Input: `dist/foundry-{EDITION}-1.0-amd64.iso`
- Output: `.torrent`, `.infohash`, `.magnet` sidecar files in `dist/`
- Uses `mktorrent` (available on host) with 4 MiB pieces, two public trackers, HTTP web seed
- Extracts infohash via inline Python3 bencode parser (no extra deps)

### Updated: `foundry-iso/scripts/sign-iso.sh`

- After writing manifest JSON, call `create-torrents.sh` if `mktorrent` available
- Add `torrent_url`, `infohash`, `magnet_link` fields to `manifest-{EDITION}.json`

### Updated: `foundry-iso/scripts/upload-iso.sh`

- Upload `foundry-{EDITION}-1.0-amd64.iso.torrent` to R2 (both versioned + latest names)

### Updated: `foundry-iso/scripts/generate-iso-index.sh`

- Read `torrent_url` and `magnet_link` from manifest if present
- Render `.torrent` download link and magnet link under each edition's HTTP button

### Updated: `site/icons.jsx`

- Add `TorrentIcon` (simple download-with-arrow SVG)
- Add `MagnetIcon` (magnet U-shape SVG)

### Updated: `site/sections.jsx`

- Line 275: `~10 GB` → `~15 GB` (code comment)
- Line 294: `~10 GB` → `~15 GB` (download table size)
- Add `.torrent` and magnet `<a>` links in ISO download rows

## Immediate steps (already in progress)

1. `create-torrents.sh` — written
2. Update `sign-iso.sh`, `upload-iso.sh`, `generate-iso-index.sh`
3. Add icons + update `sections.jsx`
4. Run `create-torrents.sh` for anvil (ISO in dist/); run for atelier once upload completes
5. Re-sign (to update manifest with torrent fields), upload torrents to R2, regenerate index
6. `task site-build && task site-deploy`

## Verification

- `dist/foundry-anvil-1.0-amd64.iso.torrent` exists, valid bencode
- Magnet link format: `magnet:?xt=urn:btih:{40-char hex}&dn=...&ws=...&tr=...`
- Homepage shows `~15 GB` for atelier, torrent + magnet buttons visible
- `task site-build` exits 0
