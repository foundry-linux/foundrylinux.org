# Plan: Internet Archive ISO Migration Investigation

## Context

At v1.0.0, Foundry Linux ISOs migrate from Cloudflare R2 to Internet Archive.
R2's 10 GB free tier can't hold a single atelier ISO (~10 GB), let alone both
editions + latest pointers (~27 GB). IA is free, unlimited, non-profit, and
already used by major Linux distros. The user-facing URL (`iso.foundrylinux.org`)
never changes — a Cloudflare Worker redirect makes the backend swap invisible.

This plan writes `docs/investigations/2026-06-13-ia-migration.md` containing:
- Full technical background on the IA S3 API
- Complete scripts (ready to commit)
- One-time setup checklist
- Cloudflare Worker code
- CI workflow diff
- Verification steps

---

## Files to create

### 1. `docs/investigations/2026-06-13-ia-migration.md`
Main investigation document (see content below).

### 2. `foundry-iso/scripts/upload-iso-ia.sh`
Parallel to `upload-iso.sh` but targets IA. Uses rclone `internetarchive` backend.
Uploads versioned ISO + sidecars (`.sha256`, `.asc`, `.torrent`) to an IA item
per release. Does NOT create latest-pointer files (IA is immutable per item;
the Worker redirect handles "latest").

### 3. `workers/iso-redirect/index.js`
Cloudflare Worker that routes `iso.foundrylinux.org/{filename}` → 302 to the
correct IA download URL. Reads a JSON `RELEASES` KV key so the map can be
updated without redeploying the Worker.

### 4. `workers/iso-redirect/wrangler.toml`
Wrangler config binding `iso.foundrylinux.org` to the Worker + KV namespace.

### 5. `scripts/bootstrap-ia.sh`
One-time setup: validate IA credentials, set GitHub Actions secrets
(`IA_ACCESS_KEY_ID`, `IA_SECRET_ACCESS_KEY`), verify rclone connectivity.

---

## Key technical decisions

**IA item naming:** One item per major release — `foundry-linux-1.0`,
`foundry-linux-1.1`, etc. All editions (anvil, atelier) go in the same item.
Download URLs: `https://archive.org/download/foundry-linux-1.0/foundry-anvil-1.0-amd64.iso`

**rclone backend:** `internetarchive` type (not generic S3). Supports multipart
natively; the 10 GB atelier ISO uploads cleanly. Config via env vars:
`RCLONE_CONFIG_IA_TYPE=internetarchive`,
`RCLONE_CONFIG_IA_ACCESS_KEY_ID`, `RCLONE_CONFIG_IA_SECRET_ACCESS_KEY`.

**Cloudflare Worker redirect strategy:** Store the `RELEASES` map in a KV
namespace (`ISO_RELEASES`) keyed by filename. The CI publish step writes the new
URLs to KV after a successful IA upload. Worker reads KV and issues a 302.
Fallback: if KV miss, serve a JSON list of known items from IA metadata API.

**No latest-pointer files on IA:** IA items are immutable/append-only. The
"latest" concept lives in the Worker's KV store, not IA. This is cleaner than
duplicating 10 GB files.

**Torrent:** IA auto-generates `{identifier}_archive.torrent` within ~24 h of
upload. The Worker can also expose a `/foundry-anvil-latest-amd64.torrent`
redirect to the auto-generated torrent.

---

## CI workflow change (publish.yml diff summary)

Replace the "Upload ISO to R2" step with:
```yaml
- name: Upload ISO to Internet Archive
  if: ${{ !inputs.dry_run }}
  env:
    EDITION:                 ${{ matrix.edition }}
    IA_ACCESS_KEY_ID:        ${{ secrets.IA_ACCESS_KEY_ID }}
    IA_SECRET_ACCESS_KEY:    ${{ secrets.IA_SECRET_ACCESS_KEY }}
    CF_ACCOUNT_ID:           ${{ secrets.CF_ACCOUNT_ID }}
    CF_API_TOKEN:            ${{ secrets.CF_API_TOKEN }}
    ISO_RELEASES_KV_ID:      ${{ secrets.ISO_RELEASES_KV_ID }}
  run: bash foundry-iso/scripts/upload-iso-ia.sh
```

Keep the R2 upload step for apt repos (separate workflow, unchanged).
Keep the index job but update `generate-iso-index.sh` to point URLs at
`iso.foundrylinux.org` (no change needed — it already does this).

---

## One-time setup (minimal manual surface)

1. Create free archive.org account at https://archive.org/account/login.php
2. Get S3 keys at https://archive.org/account/s3.php
3. Run `bash scripts/bootstrap-ia.sh` — validates keys + stores as GH secrets
4. Create KV namespace in Cloudflare and deploy Worker with `wrangler deploy`
5. Add `ISO_RELEASES_KV_ID` to GH secrets

Everything else is automated by CI.

---

## Verification steps

1. Run `EDITION=anvil DRY_RUN=true bash foundry-iso/scripts/upload-iso-ia.sh`
   → should print upload plan without uploading.
2. Run a real upload with a test item: `foundry-linux-test-{date}`.
3. `curl -I https://archive.org/download/foundry-linux-test-{date}/foundry-anvil-0.9.119-amd64.iso`
   → HTTP 200 from IA.
4. `curl -I https://iso.foundrylinux.org/foundry-anvil-latest-amd64.iso`
   → HTTP 302 to IA download URL.
5. Wait 24 h; check `https://archive.org/download/foundry-linux-test-{date}/foundry-linux-test-{date}_archive.torrent` exists.
