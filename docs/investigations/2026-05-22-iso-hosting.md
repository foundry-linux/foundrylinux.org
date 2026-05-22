# ISO Hosting Investigation

**Date:** 2026-05-22  
**Question:** Where can we host Foundry Linux ISOs for free, given Cloudflare R2's 10 GB free tier is not enough?

---

## Audience context (shapes every decision here)

Foundry Linux is **not** a distro for Linux users. It targets **game developers who may never have used Linux** and don't want to think about it. Linux is just the platform they're building games on — they're coming from Windows or Mac game dev, and the OS is invisible infrastructure.

This matters for hosting because:

- The download experience must feel like downloading Unity or Godot: click button, get file. Clean, unambiguous, from a URL that looks like it belongs to us.
- Our users have **no preconceptions about Linux community hosting norms** — GPG trust chains, "Linux distros always use mirrors," BitTorrent download etiquette. None of that is meaningful to them.
- What they *do* recognise: a sketchy-looking download page with fake "Download" buttons is a red flag. A professional-looking download page on `foundrylinux.org` is fine.
- They never need to know where the bytes actually come from. The canonical URL is `iso.foundrylinux.org`, which we control. The backend is an implementation detail.

---

## Storage requirements

| Artifact | Size | Notes |
|---|---|---|
| `foundry-anvil-1.0-amd64.iso` | ~3.5 GB | Default workstation ISO |
| `foundry-atelier-1.0-amd64.iso` | ~10 GB | Full kit (all metapackages + mobile dev) |
| Third ISO (TBD) | ~5–7 GB | Sprite tier or kiosk; not planned for v1 |
| **ISO total (2 ISOs)** | **~13.5 GB** | Per release; latest + 1 previous = ~27 GB |
| **ISO total (3 ISOs)** | **~20 GB** | If sprite/kiosk ISO is added |
| apt.foundrylinux.org | ~2 GB | Grows slowly as packages are added |
| apt.worldfoundry.org | ~0.5 GB | 12 small CLIs + metapackages |

A single release = 13.5–20 GB ISOs + ~2.5 GB apt repos = **~16–23 GB minimum**, growing to ~35+ GB if we keep 2 versioned releases live.

The plan currently targets Cloudflare R2 for ISOs. **R2's 10 GB free tier covers neither the anvil nor atelier ISO individually.** This investigation finds the correct solution.

---

## Candidate platforms

### 1. Internet Archive (`archive.org`) ⭐ Recommended for ISOs

**Cost:** Free, forever, no storage cap.

**What it is:** A 501(c)(3) non-profit digital library. Their "Software" collection explicitly hosts open-source software — including Linux distributions. Dozens of distros use it.

**Why it works for our audience:** Our users never see archive.org. They click "Download" on `foundrylinux.org`, which resolves via a Cloudflare Worker redirect to the archive.org download URL. The experience is: button → file. The infrastructure is invisible, which is exactly right.

**Limits (practical, not hard):**
- No enforced per-account storage cap.
- Recommended max per _item_: 500 files or 500 GB. One item per release (`foundry-linux-1.0`, `foundry-linux-1.1`, …) stays well under both.
- Web uploader has a 2 GB per-file cap; the CLI (`internetarchive` pip package) and the S3-compatible API (`s3.us.archive.org`) have no file-size limit. The atelier ISO uploads cleanly via `rclone`.

**Upload API (automatable in CI):**
```bash
# pip install internetarchive
ia upload foundry-linux-1.0 \
  dist/foundry-anvil-1.0-amd64.iso \
  dist/foundry-anvil-1.0-amd64.iso.sha256 \
  dist/foundry-anvil-1.0-amd64.iso.asc \
  --metadata="mediatype:software" \
  --metadata="subject:Linux" \
  --metadata="licenseurl:https://www.gnu.org/licenses/gpl-3.0.html"
```

Or via the S3-compatible endpoint using `rclone` — same tool already in `upload-iso.sh`, just swap the remote config.

**Download delivery:** Multi-node (US East, US West, EU, Canada). Not a CDN in the Cloudflare sense, but routinely adequate for popular distro traffic.

**Verdict:** No cost, no size concern, automated upload fits the existing pipeline. The backend is invisible to our users — they get a clean `iso.foundrylinux.org` URL throughout.

---

### 2. SourceForge

**Cost:** Free for open-source projects.

**Storage:** No hard cap; documentation recommends ≤ 20–30 GB and "ask first" for larger. Their total dataset is > 50 TB, so individual projects at this scale are not a concern.

**What it provides:** Global mirror CDN, widely used by Linux distros. Upload via SFTP, rsync, or web UI.

**The real concern for our audience:** SourceForge download pages are cluttered with advertising including large fake "Download" buttons that trigger third-party installers. A user who has never downloaded Linux before, clicking through to a SourceForge download page, has a meaningful chance of clicking the wrong button. This is not a Linux-community-reputation problem — it's a UX problem that affects any non-technical user.

If we use SourceForge, the canonical URL must stay `iso.foundrylinux.org` (redirecting to our specific download link, bypassing the SourceForge landing page entirely). Even then, the risk of the ad-heavy page appearing mid-redirect is non-zero.

**Verdict:** Avoid as primary. Potentially acceptable as a silent backend mirror if the redirect bypasses the landing page entirely — but archive.org has none of these concerns.

---

### 3. Cloudflare R2 (current plan)

**Cost:** 10 GB free, then **$0.015/GB/month** storage, **no egress fees**.

**Free tier math (2 ISOs, 1 release):**

| Object | Size | Over free (10 GB) |
|---|---|---|
| `foundry-anvil-1.0-amd64.iso` | 3.5 GB | — |
| `foundry-anvil-latest-amd64.iso` | 3.5 GB (duplicate pointer) | — |
| `foundry-atelier-1.0-amd64.iso` | 10.0 GB | 7.0 GB |
| `foundry-atelier-latest-amd64.iso` | 10.0 GB (duplicate pointer) | 17.0 GB |
| Checksums + manifests | ~0.1 MB | negligible |
| **Total** | **~27 GB** | **~17 GB over** |

Overage: 17 GB × $0.015 = **$0.255/month** for a 2-ISO release. ~$0.45/month with a third ISO. Cheap but not free.

**For our users** this doesn't change anything — download experience is identical whether bytes come from R2 or archive.org. The difference is purely cost.

**Verdict:** Keep for apt repos (fits free tier comfortably). Switch ISOs to archive.org.

---

### 4. Backblaze B2

**Cost:** 10 GB free, then **$0.005/GB/month** (3× cheaper than R2). Egress is $0.01/GB, but free via Cloudflare CDN (Bandwidth Alliance).

At 17 GB over free tier the storage savings vs R2 are $0.085/month. Not worth the migration complexity.

**Verdict:** Not worth switching at this scale. Not free.

---

### 5. Storj

**Cost:** 25 GB free (new accounts), then ~$4–10/TB/month.

Two ISOs + latest pointers = ~27 GB, which already exceeds the 25 GB free tier. A third ISO pushes to ~40 GB.

**Verdict:** Doesn't solve the problem.

---

### 6. GitLab Releases / GitHub Releases

- **GitHub Releases:** 2 GB per-file limit. Atelier (~10 GB) is impossible. Dead end.
- **GitLab:** 10 GB free per project. Insufficient.

---

### 7. OSDN

Closed April 2025. Not an option.

---

## Recommendation

**Two-tier hosting:**

| Tier | Platform | What goes there |
|---|---|---|
| ISOs | **Internet Archive** | All `.iso`, `.iso.sha256`, `.iso.asc`, `manifest-*.json` per release |
| Apt repos | **Cloudflare R2** (keep current) | `dists/`, `pool/`, `key.gpg` — stays in the 10 GB free tier |

**User-facing URL:** `iso.foundrylinux.org` remains the only URL our users ever see — on the download page, in docs, everywhere. A Cloudflare Worker (free tier: 100,000 requests/day) issues a `302` redirect to the archive.org download URL. The redirect is transparent in the browser; users just get the file.

```javascript
// Cloudflare Worker — update RELEASES on each publish
const RELEASES = {
  "foundry-anvil-latest-amd64.iso":
    "https://archive.org/download/foundry-linux-1.0/foundry-anvil-1.0-amd64.iso",
  "foundry-atelier-latest-amd64.iso":
    "https://archive.org/download/foundry-linux-1.0/foundry-atelier-1.0-amd64.iso",
};

export default {
  async fetch(req) {
    const file = new URL(req.url).pathname.replace(/^\//, "");
    const target = RELEASES[file];
    if (target) return Response.redirect(target, 302);
    return new Response("Not found", { status: 404 });
  },
};
```

This keeps `iso.foundrylinux.org/foundry-anvil-latest-amd64.iso` as the stable canonical URL — future backend changes (archive.org → R2 → anything else) are a one-line update in the Worker, invisible to users and docs.

**Note on torrents:** Archive.org auto-generates `.torrent` and magnet links per item. Gamers are very familiar with torrents for large file downloads — a 10 GB ISO is exactly the kind of file where BitTorrent shines. This is a free bonus when we migrate to archive.org at v1.0.0.

---

## Migration path

The existing pipeline changes minimally:

1. **Add `scripts/upload-iso-ia.sh`** — uploads to archive.org via `rclone` using the S3-compatible endpoint (`s3.us.archive.org`, access key pair from archive.org account settings — free account).
2. **Add Cloudflare Worker** — `iso.foundrylinux.org` → 302 to archive.org download URL. Update `RELEASES` map on each publish.
3. **Update `foundry-iso/.github/workflows/publish.yml`** — replace the R2 upload step with the archive.org upload step.
4. **Keep R2 for apt repos** — no change needed there.

The R2 `foundry-iso` bucket can optionally remain as a hot cache if low-latency direct serving is wanted later (no redirect, bytes come from Cloudflare edge). At 13.5 GB the cost is ~$0.20/month — a judgment call once we have real download traffic to reason about.

---

## Summary table

| Platform | Free storage | $/GB/month over | Egress | Verdict |
|---|---|---|---|---|
| **Internet Archive** | **Unlimited** | **$0** | **$0** | **✓ Use for ISOs** |
| SourceForge | Soft ~20–30 GB | $0 | $0 | Avoid — confusing download pages for non-technical users |
| Cloudflare R2 | 10 GB | $0.015 | $0 | ✓ Keep for apt repos |
| Backblaze B2 | 10 GB | $0.005 | $0.01/GB* | Not worth switching |
| Storj | 25 GB | ~$0.006 | $0.02/GB | Too small for 2+ ISOs |
| GitLab | 10 GB | paid plan | $0 | Too small |
| GitHub Releases | — | — | $0 | 2 GB file limit — unusable |
| OSDN | — | — | — | Closed April 2025 |

*B2 egress is free via Cloudflare CDN (Bandwidth Alliance).

---

## Decision (2026-05-22)

- **Pre-1.0.0:** Host ISOs on Cloudflare R2 as already wired. Small monthly cost (~$0.25–$0.50) is acceptable during development and pre-release when download volume is negligible.
- **At v1.0.0:** Migrate ISO hosting to Internet Archive. R2 stays for apt repos. The `iso.foundrylinux.org` canonical URL is unchanged — a Cloudflare Worker redirect makes the backend swap invisible to users and docs.

Migration TODO tracked in `TODO.md` under Phase 3.
