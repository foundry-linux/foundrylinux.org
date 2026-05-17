Run the Foundry Linux Phase 1 bootstrap (`scripts/bootstrap.sh`) — provisions all one-time
infrastructure for `foundry-linux/foundry-apt`: Cloudflare operator token, GitHub repo, GPG
signing key, GitHub secrets, R2 bucket, DNS, and public key upload.

## Preflight

Check each prerequisite before running:

```bash
command -v gpg   # gnupg2
command -v shred # util-linux
command -v curl
command -v jq
command -v aws   # AWS CLI used as S3-compat client for R2 — no AWS account needed
command -v gh
gh auth status
```

If any are missing, tell the user which package to install and stop.

## Dry run first

Run the dry-run pass and show the full output so the user can confirm what will happen:

```bash
bash scripts/bootstrap.sh --dry-run
```

Ask: "Looks good — run for real?" before proceeding.

## Run for real

```bash
bash scripts/bootstrap.sh
```

The script prompts interactively for Cloudflare credentials if `CF_API_TOKEN` is not exported.
Do not attempt to pre-supply them — let the script prompt.

## Monitor for known failure modes

| Pattern in output | What it means | Fix |
|---|---|---|
| `[error] Cloudflare Global API Key auth failed` | Wrong email or key | Re-enter credentials |
| `[1b] Token 'foundry-linux-operator' already exists` | Prior partial run | Export `CF_API_TOKEN` if you have it, or delete the token at <https://dash.cloudflare.com/profile/api-tokens> |
| `[6] R2 CI token … already exists` warning | Secret value is gone | Delete `foundry-apt-ci` token at <https://dash.cloudflare.com/profile/api-tokens> and re-run |
| `[8] R2_SECRET_ACCESS_KEY empty` | Step 6 hit the existing-token path | See above |

## After success — Step 10

Remind the user to push the first tag to trigger the publish workflow:

```bash
gh repo clone foundry-linux/foundry-apt /tmp/foundry-apt-release
git -C /tmp/foundry-apt-release tag v0.0.1
git -C /tmp/foundry-apt-release push origin v0.0.1
# Watch: https://github.com/foundry-linux/foundry-apt/actions
```

Verify the `smoke-install` job goes green before calling bootstrap complete.
