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
     - User → API Tokens → Edit
     - Zone → DNS → Edit
   - **Account Resources:** Include → select your account
   - **Zone Resources:** Include → Specific zone → `CF_ZONE_NAME` (not "All zones")
4. Continue to summary → **Create Token** → copy the value, paste at the script prompt

`CF_ACCOUNT_ID` and `CF_ZONE_ID` are fetched automatically — no manual lookup needed.

The script validates all three permissions at the end of step 1b before touching anything.

**Note:** Do not add `Account → API Tokens → Edit` — it is not needed and causes confusion
during validation (the script validates `User → API Tokens → Edit` only).

## Step 4 — Run for real

```bash
bash scripts/bootstrap.sh
```

## Known failure modes

If step 1b reports missing permissions, edit the token at
<https://dash.cloudflare.com/profile/api-tokens> and re-run.

| Pattern in output | What it means | Fix |
|---|---|---|
| `[1b] Token is missing required permissions` | Operator token incomplete | Edit token — the output lists which permissions are missing |
| `[1b] Could not retrieve account ID` | Token can't read account | Verify Account Resources → your account is selected |
| `[6] Could not find R2 write permission group ID` | `/user/tokens/permission_groups` returned unexpected data | Verify `User → API Tokens → Edit` is set on the operator token |

## Step 5 — Push first tag

After the script completes, trigger the first publish workflow:

```bash
# values from config block
gh repo clone "${GH_ORG}/${PKG_NAME}" /tmp/apt-repo-release
git -C /tmp/apt-repo-release tag v0.0.1
git -C /tmp/apt-repo-release push origin v0.0.1
```

Confirm the `smoke-install` job goes green before calling bootstrap complete.
