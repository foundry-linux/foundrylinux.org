Provision a new web-hosted signed APT repo on Cloudflare R2.

Wraps `scripts/bootstrap.sh` — which is configured for `foundry-apt` by default but can be
re-targeted for any project by editing the config block at the top of the script.

## Step 0 — Confirm or update config

Read the config block at the top of `scripts/bootstrap.sh` (lines ~32–51). Key values:

| Variable | Current default | What it controls |
|---|---|---|
| `GH_ORG` | `foundry-linux` | GitHub org that owns the repo |
| `PKG_NAME` | `foundry-apt` | Repo name and R2 bucket name |
| `KEY_EMAIL` | `packages@foundrylinux.org` | GPG signing key identity |
| `R2_BUCKET` | `foundry-apt` | Cloudflare R2 bucket |
| `CUSTOM_DOMAIN` | `apt.foundrylinux.org` | Public APT repo URL |
| `CF_ZONE_NAME` | `foundrylinux.org` | Cloudflare zone for DNS |

If the user is bootstrapping a different project, update these values before proceeding.

## Step 1 — Preflight

Check each prerequisite:

```bash
command -v gpg   || echo "missing: install gnupg2"
command -v shred || echo "missing: install util-linux"
command -v curl  || echo "missing"
command -v jq    || echo "missing"
command -v aws   || echo "missing: install awscli (used as S3-compat client — no AWS account needed)"
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

If `CF_API_TOKEN` is not already exported, the script will print these instructions and prompt:

1. Go to <https://dash.cloudflare.com/profile/api-tokens>
2. Click **+ Create Token** → **Get started** next to **Create Custom Token**
3. Fill in:
   - **Name:** `<CF_OPERATOR_TOKEN_NAME from config>`
   - **Permissions:**
     - Account → Workers R2 Storage → Edit
     - Account → API Tokens → Edit
     - Zone → DNS → Edit
   - **Account Resources:** Include → select your account
   - **Zone Resources:** Include → Specific zone → `<CF_ZONE_NAME>` (not "All zones")
4. Continue to summary → **Create Token** → copy the value
5. Paste it at the script prompt (input is hidden)

`CF_ACCOUNT_ID` and `CF_ZONE_ID` are fetched automatically from the API — no manual lookup needed.

## Step 4 — Run for real

```bash
bash scripts/bootstrap.sh
```

## Monitor for known failure modes

| Pattern in output | What it means | Fix |
|---|---|---|
| `[1b] Could not retrieve account ID` | Token lacks Account read access | Check R2 Storage and API Tokens permissions are set to Account scope |
| `[6] R2 CI token … already exists` warning | Prior partial run; secret value is gone | Delete the CI token at <https://dash.cloudflare.com/profile/api-tokens> and re-run |
| `[8] R2_SECRET_ACCESS_KEY empty` | Step 6 hit the existing-token path | See above |

## Step 5 — Push first tag (Step 10)

After the script completes, push the first release tag to trigger the publish workflow:

```bash
GH_REPO="<GH_ORG>/<PKG_NAME>"   # from config block
gh repo clone "${GH_REPO}" /tmp/apt-repo-release
git -C /tmp/apt-repo-release tag v0.0.1
git -C /tmp/apt-repo-release push origin v0.0.1
```

Watch the Actions run and confirm the `smoke-install` job goes green.
