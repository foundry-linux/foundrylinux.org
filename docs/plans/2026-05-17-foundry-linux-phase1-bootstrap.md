# Foundry Linux Phase 1 bootstrap

**Date:** 2026-05-17
**Status:** In progress
**Scope:** All setup needed before the GitHub Actions `publish.yml` in `foundry-linux/foundry-apt`
can build, sign, and serve the APT repo at `apt.foundrylinux.org`.

Everything here is executed via CLI — no manual console steps except where explicitly called out
as the minimum unavoidable surface (with justification).

Reference for command-level detail: [`foundry-apt/docs/infra-setup.md`](../../foundry-apt/docs/infra-setup.md).

---

## ~~Step 1 — Decide the canonical domain~~ (done)

**`apt.foundrylinux.org`** — `foundrylinux.org` is the registered domain; the `apt` subdomain is
self-describing. All tooling updated to reflect this.

---

## Step 1b — Create the Cloudflare operator API token

Run once. Uses the **Global API Key** to mint a scoped `foundry-linux-operator` token,
then the Global API Key is no longer needed.

```bash
# Required env vars (one-time use):
export CF_EMAIL="wbnorris@gmail.com"
export CF_GLOBAL_API_KEY="<your-global-api-key>"
# Global API Key: Cloudflare dash → My Profile → API Tokens → Global API Key

bash scripts/step-1b-cf-api-token.sh
# Prints three export lines — copy them into your shell for steps 3–9:
#   export CF_API_TOKEN='...'
#   export CF_ACCOUNT_ID='...'
#   export CF_ZONE_ID='...'
```

The resulting `CF_API_TOKEN` ("foundry-linux-operator") has:
- **Workers R2 Storage: Write** — bucket create + object upload
- **Zone DNS: Write** — CNAME for `apt.foundrylinux.org`
- **User API Tokens: Edit** — creates the narrow `foundry-apt-ci` token in Step 6

---

## ~~Step 2a — Create the `foundry-linux` GitHub org~~ (done)

Org exists. GitHub org creation has no public API — noted here as the one unavoidable manual step.
All subsequent GitHub operations use `gh` CLI.

---

## ~~Step 2b — Push `foundry-apt/` to its own GitHub repo~~ (done)

`foundry-apt/` content lives in this repo (`linuxfoundry.org/foundry-apt/`) but has not been
pushed to a standalone GitHub repo yet. The `publish.yml` OIDC trust policy scopes to
`repo:foundry-linux/foundry-apt:ref:refs/tags/v*`, so it must be a standalone repo.

```bash
# Copy to a temp location outside the linuxfoundry.org git tree
cp -r /home/will/SRC/linuxfoundry.org/foundry-apt /tmp/foundry-apt-push
cd /tmp/foundry-apt-push

git init
git add .
git commit -m "feat: initial foundry-apt import"

gh repo create foundry-linux/foundry-apt \
  --public \
  --description "Foundry Linux signed APT repo and build tooling" \
  --source=. --remote=origin --push

gh repo edit foundry-linux/foundry-apt --enable-discussions

rm -rf /tmp/foundry-apt-push
```

After this, `foundry-apt/` in the linuxfoundry.org repo remains the development working copy;
the standalone repo is the CI-facing authoritative source.

---

## Step 3 — Generate the GPG signing key

Run on any trusted machine (your dev box is fine — the key goes straight into SSM in Step 4
and the local copy is shredded immediately after):

```bash
gpg --full-generate-key
# Type:    RSA (sign only)
# Bits:    4096
# Expires: 2 years
# Name:    Foundry Linux Packages
# Email:   packages@foundrylinux.org

gpg --armor --export packages@foundrylinux.org > /tmp/foundry-packages.pub.gpg
gpg --armor --export-secret-keys packages@foundrylinux.org > /tmp/foundry-packages.sec.gpg
# Never commit the secret key. Shred it immediately after Step 4.
```

---

## Step 4 — Store the private key in AWS SSM

```bash
aws ssm put-parameter \
  --name /foundry-apt/signing-key \
  --type SecureString \
  --value "$(cat /tmp/foundry-packages.sec.gpg)" \
  --description "GPG signing key for foundry-apt CI"

shred -u /tmp/foundry-packages.sec.gpg
```

Uses the free AWS-managed `aws/ssm` KMS key — no $1/month customer-managed key needed.

---

## Step 5 — Create GitHub OIDC identity provider and IAM role

No long-lived AWS credentials in GitHub — OIDC federation only. All via AWS CLI:

```bash
set -euo pipefail

AWS_ACCOUNT_ID=$(aws sts get-caller-identity --query Account --output text)

# 1. Add GitHub as OIDC identity provider (idempotent — errors if already exists; that's fine)
aws iam create-open-id-connect-provider \
  --url https://token.actions.githubusercontent.com \
  --client-id-list sts.amazonaws.com \
  --thumbprint-list 6938fd4d98bab03faadb97b34396831e3780aea1 \
  2>/dev/null || echo "OIDC provider already exists, continuing"

OIDC_ARN="arn:aws:iam::${AWS_ACCOUNT_ID}:oidc-provider/token.actions.githubusercontent.com"

# 2. Write trust policy scoped to tag pushes on foundry-linux/foundry-apt only
cat > /tmp/foundry-apt-trust.json <<EOF
{
  "Version": "2012-10-17",
  "Statement": [{
    "Effect": "Allow",
    "Principal": { "Federated": "${OIDC_ARN}" },
    "Action": "sts:AssumeRoleWithWebIdentity",
    "Condition": {
      "StringEquals": {
        "token.actions.githubusercontent.com:aud": "sts.amazonaws.com",
        "token.actions.githubusercontent.com:sub": "repo:foundry-linux/foundry-apt:ref:refs/tags/v*"
      }
    }
  }]
}
EOF

# 3. Create the role
aws iam create-role \
  --role-name foundry-apt-publish \
  --assume-role-policy-document file:///tmp/foundry-apt-trust.json \
  --description "OIDC role — foundry-linux/foundry-apt tag-push CI only"

# 4. Attach minimal inline policy: SSM read on /foundry-apt/* only
cat > /tmp/foundry-apt-policy.json <<'EOF'
{
  "Version": "2012-10-17",
  "Statement": [{
    "Effect": "Allow",
    "Action": ["ssm:GetParameter", "kms:Decrypt"],
    "Resource": "arn:aws:ssm:*:*:parameter/foundry-apt/*"
  }]
}
EOF

aws iam put-role-policy \
  --role-name foundry-apt-publish \
  --policy-name foundry-apt-ssm-read \
  --policy-document file:///tmp/foundry-apt-policy.json

ROLE_ARN=$(aws iam get-role --role-name foundry-apt-publish --query Role.Arn --output text)
echo "Role ARN: ${ROLE_ARN}"

rm /tmp/foundry-apt-trust.json /tmp/foundry-apt-policy.json
```

---

## Step 6 — Create Cloudflare R2 bucket and scoped API token

Requires `CF_API_TOKEN` (a Cloudflare token with Account R2 + DNS edit permissions) and
`CF_ACCOUNT_ID`. These come from the one-time Cloudflare account setup already done for
`foundrylinux.org`.

```bash
set -euo pipefail
# CF_API_TOKEN and CF_ACCOUNT_ID must be set in the environment

# 1. Create the R2 bucket
curl -fsSL -X POST \
  "https://api.cloudflare.com/client/v4/accounts/${CF_ACCOUNT_ID}/r2/buckets" \
  -H "Authorization: Bearer ${CF_API_TOKEN}" \
  -H "Content-Type: application/json" \
  -d '{"name":"foundry-apt","locationHint":"auto"}'

# 2. Enable the r2.dev subdomain for smoke testing before DNS is wired
curl -fsSL -X PUT \
  "https://api.cloudflare.com/client/v4/accounts/${CF_ACCOUNT_ID}/r2/buckets/foundry-apt/domains/managed" \
  -H "Authorization: Bearer ${CF_API_TOKEN}" \
  -H "Content-Type: application/json" \
  -d '{"enabled":true}'
# Note the pub-xxxxx.r2.dev hostname from the response — used in Step 7

# 3. Get R2 Write permission group ID
R2_WRITE_ID=$(curl -fsSL \
  "https://api.cloudflare.com/client/v4/user/tokens/permission_groups" \
  -H "Authorization: Bearer ${CF_API_TOKEN}" \
  | jq -r '.result[] | select(.name == "Workers R2 Storage Bucket Item Write") | .id')

# 4. Create a scoped R2 token for CI use only
R2_TOKEN_RESPONSE=$(curl -fsSL -X POST \
  "https://api.cloudflare.com/client/v4/user/tokens" \
  -H "Authorization: Bearer ${CF_API_TOKEN}" \
  -H "Content-Type: application/json" \
  -d "{
    \"name\": \"foundry-apt-ci\",
    \"policies\": [{
      \"effect\": \"allow\",
      \"resources\": { \"com.cloudflare.api.account.${CF_ACCOUNT_ID}\": \"*\" },
      \"permission_groups\": [{ \"id\": \"${R2_WRITE_ID}\", \"name\": \"Workers R2 Storage Bucket Item Write\" }]
    }]
  }")

echo "${R2_TOKEN_RESPONSE}" | jq -r '.result.value'
# Save this value — it is the R2_SECRET_ACCESS_KEY shown only once.

# R2 S3-compat credentials are derived:
#   Access Key ID  — visible in Cloudflare dashboard under the token (or from the create response)
#   Secret Key     — the token value above
#   Endpoint       — https://${CF_ACCOUNT_ID}.r2.cloudflarestorage.com
```

---

## Step 7 — Configure DNS and attach the custom domain

```bash
set -euo pipefail
# CF_API_TOKEN, CF_ACCOUNT_ID, CF_ZONE_ID (foundrylinux.org zone) must be set
# R2_DEV_HOSTNAME — the pub-xxxxx.r2.dev value from Step 6

# 1. Add proxied CNAME record
curl -fsSL -X POST \
  "https://api.cloudflare.com/client/v4/zones/${CF_ZONE_ID}/dns/records" \
  -H "Authorization: Bearer ${CF_API_TOKEN}" \
  -H "Content-Type: application/json" \
  -d "{
    \"type\": \"CNAME\",
    \"name\": \"apt\",
    \"content\": \"${R2_DEV_HOSTNAME}\",
    \"proxied\": true,
    \"comment\": \"foundry-apt R2 bucket\"
  }"

# 2. Attach the custom domain to the R2 bucket
curl -fsSL -X PUT \
  "https://api.cloudflare.com/client/v4/accounts/${CF_ACCOUNT_ID}/r2/buckets/foundry-apt/domains/custom" \
  -H "Authorization: Bearer ${CF_API_TOKEN}" \
  -H "Content-Type: application/json" \
  -d '{"domains":[{"domain":"apt.foundrylinux.org","enabled":true}]}'

# 3. Verify — 404 on Release is expected (bucket is empty); we're just checking TLS + routing
curl -I https://apt.foundrylinux.org/
```

---

## Step 8 — Upload the public signing key to R2

```bash
set -euo pipefail
# R2_ACCESS_KEY_ID, R2_SECRET_ACCESS_KEY, CF_ACCOUNT_ID must be set

AWS_ACCESS_KEY_ID="${R2_ACCESS_KEY_ID}" \
AWS_SECRET_ACCESS_KEY="${R2_SECRET_ACCESS_KEY}" \
aws s3 cp /tmp/foundry-packages.pub.gpg \
  s3://foundry-apt/key.gpg \
  --endpoint-url "https://${CF_ACCOUNT_ID}.r2.cloudflarestorage.com"

shred -u /tmp/foundry-packages.pub.gpg

# Verify it's reachable
curl -fsSL https://apt.foundrylinux.org/key.gpg | gpg --show-keys
```

---

## Step 9 — Set repo secrets on `foundry-linux/foundry-apt`

```bash
set -euo pipefail
# All variables must be set from the steps above

ROLE_ARN=$(aws iam get-role --role-name foundry-apt-publish --query Role.Arn --output text)
R2_ENDPOINT="https://${CF_ACCOUNT_ID}.r2.cloudflarestorage.com"

gh secret set AWS_ROLE_ARN         --repo foundry-linux/foundry-apt --body "${ROLE_ARN}"
gh secret set R2_ACCESS_KEY_ID     --repo foundry-linux/foundry-apt --body "${R2_ACCESS_KEY_ID}"
gh secret set R2_SECRET_ACCESS_KEY --repo foundry-linux/foundry-apt --body "${R2_SECRET_ACCESS_KEY}"
gh secret set R2_ENDPOINT          --repo foundry-linux/foundry-apt --body "${R2_ENDPOINT}"

# Verify all four secrets exist
gh secret list --repo foundry-linux/foundry-apt
```

---

## Step 10 — Push the first tag

```bash
gh repo clone foundry-linux/foundry-apt /tmp/foundry-apt-release
git -C /tmp/foundry-apt-release tag v0.0.1
git -C /tmp/foundry-apt-release push origin v0.0.1
# Watch: https://github.com/foundry-linux/foundry-apt/actions
```

The `smoke-install` job at the end of `publish.yml` proves a clean Ubuntu 26.04 container can
install a metapackage from the live repo.

---

## Verification

After the workflow goes green, run from any Ubuntu 26.04 machine (or container):

```bash
curl -fsSL https://apt.foundrylinux.org/key.gpg \
  | sudo gpg --dearmor -o /etc/apt/keyrings/foundry.gpg

echo "deb [signed-by=/etc/apt/keyrings/foundry.gpg] https://apt.foundrylinux.org resolute main" \
  | sudo tee /etc/apt/sources.list.d/foundry.list

sudo apt update
apt-cache show foundry-linux-dev
```

`apt-get install -y --no-install-recommends foundry-linux-dev` in a container is the definitive test.

---

## Status checklist

- [x] Domain decided — `apt.foundrylinux.org`
- [ ] Cloudflare operator token `foundry-linux-operator` created (`CF_API_TOKEN`)
- [x] `foundry-linux` GitHub org created
- [x] `foundry-linux/foundry-apt` GitHub repo created and pushed
- [ ] GPG signing key generated (`packages@foundrylinux.org`, 4096-bit RSA, 2-year expiry)
- [ ] Private key stored in AWS SSM at `/foundry-apt/signing-key`
- [ ] Local copy of private key shredded
- [ ] GitHub OIDC identity provider added to AWS IAM
- [ ] `foundry-apt-publish` IAM role created with scoped trust + minimal SSM policy
- [ ] R2 bucket `foundry-apt` created
- [ ] R2.dev subdomain enabled
- [ ] Scoped R2 CI token created
- [ ] Public signing key uploaded to R2 as `key.gpg`
- [ ] DNS CNAME `apt.foundrylinux.org` configured (proxied)
- [ ] Custom domain attached to R2 bucket
- [ ] `AWS_ROLE_ARN`, `R2_ACCESS_KEY_ID`, `R2_SECRET_ACCESS_KEY`, `R2_ENDPOINT` secrets set on repo
- [ ] First tag `v0.0.1` pushed
- [ ] `publish.yml` workflow green
- [ ] `smoke-install` job confirms `apt install foundry-linux-dev` from live repo
