Provision a new web-hosted signed APT repo on Cloudflare R2.

Usage: `/new-web-apt-repo [gh-org/repo zone-name [subdomain [key-email]]]`

Examples:
- `/new-web-apt-repo` — use existing config in bootstrap.sh (foundry-apt)
- `/new-web-apt-repo worldfoundry/wf-apt worldfoundry.org apt packages@worldfoundry.org`

## Step 0 — Configure for target project

If `$ARGUMENTS` were provided, parse them as:
```
<gh-org/repo>  <zone-name>  [subdomain=apt]  [key-email=packages@<zone-name>]
```

Derive the remaining config values:
- `GH_ORG` = org part of gh-org/repo
- `PKG_NAME` = repo part of gh-org/repo
- `R2_BUCKET` = PKG_NAME
- `CUSTOM_DOMAIN` = subdomain.zone-name
- `DNS_CNAME` = subdomain
- `CF_OPERATOR_TOKEN_NAME` = `<gh-org>-operator`
- `R2_TOKEN_NAME` = `<PKG_NAME>-ci`
- `KEY_NAME` = title-cased PKG_NAME + " Packages"

Then update the config block at the top of `scripts/bootstrap.sh` (lines ~32–51) with these values.

If no arguments were provided, read the config block and confirm the values with the user before proceeding.

## Step 1 — Preflight

```bash
command -v gpg   || echo "missing: install gnupg2"
command -v shred || echo "missing: install util-linux"
command -v curl  || echo "missing"
command -v jq    || echo "missing"
command -v gh    || echo "missing: https://cli.github.com"
gh auth status
```

Stop and help the user install anything missing before continuing.

## Step 2 — Dry run

```bash
bash scripts/bootstrap.sh --dry-run
```

Show the output and ask: "Looks good — run for real?" before proceeding.

## Step 3 — Create the Cloudflare operator token (if needed)

If `CF_API_TOKEN` is not already exported, the script will print instructions and prompt for it:

1. Go to <https://dash.cloudflare.com/profile/api-tokens>
2. Click **+ Create Token** → **Get started** next to **Create Custom Token**
3. Fill in:
   - **Name:** value of `CF_OPERATOR_TOKEN_NAME` from config
   - **Permissions** (exactly three):
     - Account → Workers R2 Storage → Edit
     - Zone → DNS → Edit
     - Zone → Transform Rules → Edit
   - **Account Resources:** Include → select your account
   - **Zone Resources:** Include → Specific zone → `CF_ZONE_NAME` (not "All zones")
4. Continue to summary → **Create Token** → copy the value, paste at the script prompt

`CF_ACCOUNT_ID` and `CF_ZONE_ID` are fetched automatically — no manual lookup needed.

The script validates all three permissions in step 1b before touching anything.

## Step 4 — Run for real

```bash
bash scripts/bootstrap.sh
```

The script caches `CF_API_TOKEN` and R2 credentials to `/tmp/foundry-linux-bootstrap.env`
(mode 600) on first entry and reloads them on subsequent runs — no re-typing on restarts.

When prompted for R2 S3 credentials (step 6), the dashboard page is printed automatically.
Create an **Account API token** (not User — Account tokens survive org membership changes
and are recommended for CI):
- **Token name:** value of `R2_TOKEN_NAME` from config
- **Permissions:** Object Read & Write
- **Bucket:** Apply to specific buckets only → `R2_BUCKET`

## Known failure modes

If step 1b reports missing permissions, edit the token at
<https://dash.cloudflare.com/profile/api-tokens> and re-run (cached token reloads
automatically, no re-paste needed).

| Pattern in output | What it means | Fix |
|---|---|---|
| `[1b] Token is missing required permissions` | Operator token incomplete | Edit token — the output lists which permissions are missing |
| `[1b] Could not retrieve account ID` | Token can't read account | Verify Account Resources → your account is selected |
| `[7.5]` error on transform rule | Zone may already have a conflicting rewrite rule | Check Cloudflare dashboard → `<zone>` → Rules → Transform Rules |

## Step 5 — Push first tag

After the script completes, trigger the first publish workflow:

```bash
task bump    # auto-increments patch version and triggers publish.yml
```

Confirm the `smoke-install` job goes green before calling bootstrap complete.

## Step 5.5 — Verify landing page

Once the publish workflow goes green, confirm the root URL returns HTML:

```bash
curl -sI https://<CUSTOM_DOMAIN>/
# expect: HTTP/2 200  content-type: text/html
```

The root URL is served via a Cloudflare URL rewrite rule (`http_request_transform` phase,
created by bootstrap step 7.5) that transparently maps `/` → `/index.html`. The free plan
does not support the `http_request_redirect` phase, so this is a rewrite, not a 301.

`scripts/generate-index.sh` generates `public/index.html` automatically on every publish run,
pulling package names, versions, and descriptions from `packages/*/DEBIAN/control` — no
manual updates needed as packages evolve.

To preview locally before pushing:

```bash
task generate-index   # runs publish-local as a dep, then generates the page
# open public/index.html in a browser
```

To customise branding for a new repo, edit `SITE_TITLE`, `SITE_URL`, and `GITHUB_URL` at
the top of `scripts/generate-index.sh`.

### Package layout — canonical Debian source format only

Every package in this repo uses the canonical **Debian source-package layout**:

```
packages/<name>/
  debian/
    control               # Source: + one or more Package: stanzas
    changelog             # authoritative version source (dpkg-parsechangelog)
    rules                 # one-line "%: dh $@" (executable)
    source/format         # "3.0 (native)" for metapackages
                          # "3.0 (quilt)" for vendored upstreams
    copyright             # DEP-5 format
    [patches/series]      # optional quilt patches for vendored upstreams
    [watch]               # optional uscan tracker for vendored upstreams
  [build.sh]              # only for vendored upstreams — fetches tarball,
                          # overlays debian/, runs dpkg-buildpackage
```

Both pure metapackages and vendored upstreams use this layout. The only difference is:
- **Metapackages**: `3.0 (native)` source format, version like `1.0.1` (no Debian revision), no upstream tarball, no `build.sh`. `build-all.sh` handles them directly.
- **Vendored upstreams** (`/package` skill generates these): `3.0 (quilt)` source format, version like `2.4.1-1foundry1` (upstream-revision), pinned `build.sh` wrapper that fetches + sha256-verifies the upstream tarball.

Hand-rolled vendor (a `build.sh` that calls `dpkg-deb --build` directly with a static
`DEBIAN/control`) is **no longer supported**. The deprecated `packages/xa65/`
(`build.sh` only, uppercase `DEBIAN/control`) still builds and ships a `.deb` via its
own `build.sh`, but it's not on the landing page — `generate-index.sh` only reads
`debian/`. xa65 disappears entirely once the "Phase 0 configures foundry-apt as a
source" TODO lands and Ubuntu universe's xa65 takes over.

## Step 6 — How users consume this repo

Downstream users (and CI smoke-install jobs, fresh containers, etc.) add the repo via:

```bash
sudo install -d /etc/apt/keyrings
curl -fsSL https://<CUSTOM_DOMAIN>/key.gpg \
  | sudo gpg --dearmor -o /etc/apt/keyrings/<SLUG>.gpg
echo "deb [signed-by=/etc/apt/keyrings/<SLUG>.gpg] https://<CUSTOM_DOMAIN> <SUITE> main" \
  | sudo tee /etc/apt/sources.list.d/<SLUG>.list
sudo apt-get update
sudo apt-get install <pkg>
```

Where:
- `<CUSTOM_DOMAIN>` is e.g. `apt.foundrylinux.org` (the repo's web hostname)
- `<SLUG>` is a short identifier for the keyring filename (e.g. `foundry`)
- `<SUITE>` is the distribution codename the repo was published for — look up the value in
  `aptly/aptly.conf` or the publish workflow output. **It is *not* the same as `<CUSTOM_DOMAIN>`.**
  For an Ubuntu 26.04-targeted repo this is `resolute`, not `noble` or `jammy`.

The published key lives at exactly **`/key.gpg`** (`bootstrap.sh` step 8 uploads it).
Do **not** invent paths like `/foundry-archive-keyring.gpg`, `/pubkey.asc`, etc. — those
are 404. The path is canonical.

Add this snippet to the repo's `README.md` so users don't have to guess.

## Credential storage

`bootstrap.sh` automatically stores all secrets to a private `<gh-org>-secrets` R2 bucket
(no public access, no custom domain) using the operator token with Bearer auth:

| Secret | GitHub Actions secret | R2 backup |
|---|---|---|
| GPG signing key | `GPG_PRIVATE_KEY` | `r2://<org>-secrets/GPG_PRIVATE_KEY` |
| R2 access key | `R2_ACCESS_KEY_ID` | `r2://<org>-secrets/R2_ACCESS_KEY_ID` |
| R2 secret | `R2_SECRET_ACCESS_KEY` | `r2://<org>-secrets/R2_SECRET_ACCESS_KEY` |
| CF operator token | (not in GitHub) | `r2://<org>-secrets/CF_API_TOKEN` |

A local session cache at `/tmp/foundry-linux-bootstrap.env` (mode 600, cleared on reboot)
holds `CF_API_TOKEN`, `R2_ACCESS_KEY_ID`, and `R2_SECRET_ACCESS_KEY` so re-runs within the
same session don't re-prompt. Retrieve any value from R2 backup if needed:

```bash
curl -fsSL "https://api.cloudflare.com/client/v4/accounts/<ACCOUNT_ID>/r2/buckets/<org>-secrets/objects/<KEY>" \
  -H "Authorization: Bearer $CF_API_TOKEN"
```

No AWS SSM or other external secrets store is used.
