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

Handled automatically by `bootstrap.sh` when `CF_API_TOKEN` is not already exported.
The script prompts for the Cloudflare account email and **Global API Key**
(Dash → My Profile → API Tokens → Global API Key), uses them once to mint a scoped
`foundry-linux-operator` token, then they are no longer needed.

The resulting token has:
- **Workers R2 Storage: Write** — bucket create + object upload
- **Zone DNS: Write** — CNAME for `apt.foundrylinux.org`
- **User API Tokens: Edit** — creates the narrow `foundry-apt-ci` CI token in Step 6

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

If `CF_API_TOKEN` is not already exported, the script prompts for the Cloudflare account email
and Global API Key, then creates the operator token automatically.

If `CF_API_TOKEN` is already set (re-run scenario), also export `CF_ACCOUNT_ID` and `CF_ZONE_ID`.

No AWS account is required.

What the script does, in order:

| Step | What |
|------|------|
| 1b | Create `foundry-linux-operator` Cloudflare token (R2 + DNS + user-token:edit) |
| 2b | Push `foundry-apt/` to `foundry-linux/foundry-apt` on GitHub |
| 3 | Generate 4096-bit RSA GPG signing key (`packages@foundrylinux.org`, 2-year expiry) |
| 4 | Set `GPG_PRIVATE_KEY` GitHub Actions secret; shred local private key copy |
| 6 | Create R2 bucket `foundry-apt`; create scoped `foundry-apt-ci` CI token |
| 7 | Create proxied DNS CNAME `apt.foundrylinux.org`; attach custom domain to R2 bucket |
| 8 | Upload `key.gpg` to R2; shred local public key copy |
| 9 | Set `R2_ACCESS_KEY_ID`, `R2_SECRET_ACCESS_KEY`, `R2_ENDPOINT` secrets |

All steps are idempotent — safe to re-run.

---

## Step 10 — Push the first tag

```bash
gh repo clone foundry-linux/foundry-apt /tmp/foundry-apt-release
git -C /tmp/foundry-apt-release tag v0.0.1
git -C /tmp/foundry-apt-release push origin v0.0.1
# Watch: https://github.com/foundry-linux/foundry-apt/actions
```

The `smoke-install` job at the end of `publish.yml` proves a clean Ubuntu 26.04 container can
`apt install foundry-linux-dev` from the live repo.

---

## Step 11 — Create `/new-web-apt-repo` skill

Wrap the bootstrap process as a Claude Code slash command so future runs can be invoked as
`/bootstrap` from within Claude Code.

Skill file: `.claude/commands/new-web-apt-repo.md`

The skill:
- Checks preflight conditions (tools installed, `gh auth status`)
- Runs `bash scripts/bootstrap.sh --dry-run` and shows output
- Prompts for confirmation before running for real
- Monitors for errors and helps diagnose known failure modes
- Reminds the user to push the first tag after completion

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
apt-get install -y --no-install-recommends foundry-linux-dev
```

---

## Status checklist

- [x] Domain decided — `apt.foundrylinux.org`
- [x] `foundry-linux` GitHub org created
- [x] `foundry-linux/foundry-apt` GitHub repo created and pushed
- [ ] Cloudflare operator token `foundry-linux-operator` created
- [ ] GPG signing key generated (`packages@foundrylinux.org`, 4096-bit RSA, 2-year expiry)
- [ ] `GPG_PRIVATE_KEY` secret set on `foundry-linux/foundry-apt`
- [ ] Local copy of private key shredded
- [ ] R2 bucket `foundry-apt` created
- [ ] R2.dev subdomain enabled
- [ ] Scoped R2 CI token `foundry-apt-ci` created
- [ ] Public signing key uploaded to R2 as `key.gpg`
- [ ] DNS CNAME `apt.foundrylinux.org` configured (proxied)
- [ ] Custom domain attached to R2 bucket
- [ ] `R2_ACCESS_KEY_ID`, `R2_SECRET_ACCESS_KEY`, `R2_ENDPOINT` secrets set
- [ ] First tag `v0.0.1` pushed
- [ ] `publish.yml` workflow green
- [ ] `smoke-install` job confirms `apt install foundry-linux-dev` from live repo
- [ ] `/new-web-apt-repo` skill created at `.claude/commands/new-web-apt-repo.md`
