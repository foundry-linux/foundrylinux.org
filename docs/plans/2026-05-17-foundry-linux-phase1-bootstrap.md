# Foundry Linux Phase 1 bootstrap

**Date:** 2026-05-17
**Status:** In progress
**Scope:** All setup needed before the GitHub Actions `publish.yml` in `foundry-linux/foundry-apt`
can build, sign, and serve the APT repo at `apt.foundrylinux.org`.

Everything here is executed via CLI — no manual console steps except where explicitly called out
as the minimum unavoidable surface (with justification).

---

## ~~Step 1 — Decide the canonical domain~~ (done)

**`apt.foundrylinux.org`** — `foundrylinux.org` is the registered domain; the `apt` subdomain is
self-describing.

---

## Step 1b — Create the Cloudflare operator API token

Create the `foundry-linux-operator` token manually at
<https://dash.cloudflare.com/profile/api-tokens> (one unavoidable manual step — no API path for
creating the first credential). The script prompts for the token value if `CF_API_TOKEN` is not
already exported; `CF_ACCOUNT_ID` and `CF_ZONE_ID` are fetched automatically via the API.

The token requires exactly three permissions:
- **Account | Workers R2 Storage | Edit** — bucket create, object upload, custom domain attach
- **User | API Tokens | Edit** — creates the narrow `foundry-apt-ci` CI token in Step 6
- **Zone | DNS | Edit** (Specific zone: `foundrylinux.org`) — CNAME for `apt.foundrylinux.org`

The script validates all three at the end of step 1b before touching anything.

### CI token credentials (step 6 finding)

The CI token is created via `POST /user/tokens` with the `Workers R2 Storage Bucket Item Write`
permission group. The resulting token's `.result.id` and `.result.value` are the R2 S3
credentials. These work with `rclone --provider Cloudflare` (used by `publish.yml`) but NOT with
the vanilla `aws` CLI (which produces `SignatureDoesNotMatch`). Do not use `aws s3` with R2.

### Key upload (step 8 finding)

`key.gpg` is uploaded via the Cloudflare R2 REST API using Bearer token auth:
`PUT /accounts/{id}/r2/buckets/{bucket}/objects/key.gpg`

No `aws` CLI or S3 credentials required for this step. The `aws` CLI is not a prerequisite.

---

## ~~Step 2a — Create the `foundry-linux` GitHub org~~ (done)

Org exists. GitHub org creation has no public API — the one unavoidable manual step.
All subsequent GitHub operations use `gh` CLI.

---

## ~~Step 2b — Push `foundry-apt/` to its own GitHub repo~~ (done)

`foundry-apt/` in the `linuxfoundry.org` repo is the development working copy;
`foundry-linux/foundry-apt` on GitHub is the CI-facing authoritative source.

---

## Steps 1b–9 — Run `bootstrap.sh`

All remaining setup is fully automated. From the `linuxfoundry.org` repo root:

```bash
bash scripts/bootstrap.sh --dry-run   # preview all steps
bash scripts/bootstrap.sh             # run for real
```

If `CF_API_TOKEN` is not already exported, the script prompts for it (hidden input).
`CF_ACCOUNT_ID` and `CF_ZONE_ID` are fetched automatically.

No AWS account or `aws` CLI required.

What the script does, in order:

| Step | What |
|------|------|
| 1b | Resolve `CF_ACCOUNT_ID` + `CF_ZONE_ID`; validate operator token permissions |
| 2b | Push `foundry-apt/` to `foundry-linux/foundry-apt` on GitHub |
| 3 | Generate 4096-bit RSA GPG signing key (`packages@foundrylinux.org`, 2-year expiry) |
| 4 | Set `GPG_PRIVATE_KEY` GitHub Actions secret; shred local private key copy |
| 6 | Create R2 bucket `foundry-apt`; create scoped `foundry-apt-ci` CI token via `/user/tokens` |
| 7 | Create proxied DNS CNAME `apt.foundrylinux.org`; attach custom domain to R2 bucket |
| 8 | Upload `key.gpg` to R2 via CF REST API (Bearer auth); shred local public key copy |
| 9 | Set `R2_ACCESS_KEY_ID`, `R2_SECRET_ACCESS_KEY`, `R2_ENDPOINT` secrets |

All steps are idempotent — safe to re-run. Custom domain 409 (already attached) is treated as
success.

---

## Step 10 — Push the first tag

```bash
gh repo clone foundry-linux/foundry-apt /tmp/foundry-apt-release
git -C /tmp/foundry-apt-release tag v0.0.1
git -C /tmp/foundry-apt-release push origin v0.0.1
# Watch: https://github.com/foundry-linux/foundry-apt/actions
```

Tags v0.0.1–v0.0.4 have been pushed. `build-and-publish` succeeds on every run;
`smoke-install` fails at the `key.gpg` fetch with HTTP 404.

### Blocker — rclone sync deletes key.gpg on every publish

`rclone sync ./public/ R2:foundry-apt/` deletes files in R2 that are not in `./public/`.
`key.gpg` was uploaded to R2 root by the bootstrap script but lives outside the aptly
`public/` tree, so each tag push wipes it.

**Fix:** Extract the public key from `GPG_PRIVATE_KEY` in the workflow and copy it to
`./public/key.gpg` before the rclone sync step. That way `key.gpg` travels in the sync
and is never orphaned.

In `publish.yml`, add after the "Sign Release" step:

```yaml
- name: Export public key for repo consumers
  if: ${{ !inputs.dry_run }}
  env:
    GPG_PRIVATE_KEY: ${{ secrets.GPG_PRIVATE_KEY }}
  run: |
    echo "$GPG_PRIVATE_KEY" | gpg --import
    gpg --export --armor > ./public/key.gpg
```

---

## ~~Step 11 — Create `/new-web-apt-repo` skill~~ (done)

Skill file: `.claude/commands/new-web-apt-repo.md`

---

## Verification

After the workflow goes green, run from any Ubuntu 26.04 machine (or container):

```bash
curl -fsSL https://apt.foundrylinux.org/key.gpg \
  | sudo gpg --dearmor -o /etc/apt/keyrings/foundry.gpg

echo "deb [signed-by=/etc/apt/keyrings/foundry.gpg] https://apt.foundrylinux.org resolute main" \
  | sudo tee /etc/apt/sources.list.d/foundry.list

sudo apt update
apt-cache show task
apt-get install -y --no-install-recommends task
```

---

## Status checklist

- [x] Domain decided — `apt.foundrylinux.org`
- [x] `foundry-linux` GitHub org created
- [x] `foundry-linux/foundry-apt` GitHub repo created and pushed
- [x] Cloudflare operator token `foundry-linux-operator` created with correct permissions
- [x] GPG signing key generated (`packages@foundrylinux.org`, 4096-bit RSA, 2-year expiry)
- [x] `GPG_PRIVATE_KEY` secret set on `foundry-linux/foundry-apt`
- [x] Local copy of private key shredded
- [x] R2 bucket `foundry-apt` created
- [x] R2.dev subdomain enabled
- [x] Scoped R2 CI token `foundry-apt-ci` created (R2 S3 credentials entered manually)
- [x] DNS CNAME `apt.foundrylinux.org` configured (proxied)
- [x] Custom domain attached to R2 bucket
- [x] `R2_ACCESS_KEY_ID`, `R2_SECRET_ACCESS_KEY`, `R2_ENDPOINT` secrets set
- [x] First tag `v0.0.1` pushed (v0.0.1–v0.0.4 pushed; `build-and-publish` green on all)
- [x] `publish.yml` workflow green — v0.0.5 both jobs green
- [x] Public signing key served at `https://apt.foundrylinux.org/key.gpg`
- [x] `smoke-install` job confirms `apt install worldfoundry-engine-build-deps` from live repo
- [x] `/new-web-apt-repo` skill created at `.claude/commands/new-web-apt-repo.md`
