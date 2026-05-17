# foundry-apt infrastructure setup

One-time setup needed before [`publish.yml`](../.github/workflows/publish.yml) can build, sign,
and sync the APT repo to `apt.foundrylinux.org`. Everything is automated via a single script.

## Run bootstrap.sh

From the `linuxfoundry.org` repo root:

```bash
bash scripts/bootstrap.sh --dry-run   # preview all steps
bash scripts/bootstrap.sh             # run for real
```

If `CF_API_TOKEN` is not already exported the script prompts for your Cloudflare Global API Key
(Dash → My Profile → API Tokens → Global API Key) and creates the scoped operator token itself.

See [`docs/plans/2026-05-17-foundry-linux-phase1-bootstrap.md`](../../docs/plans/2026-05-17-foundry-linux-phase1-bootstrap.md)
for the full step-by-step breakdown and status checklist.

## First release

Once `bootstrap.sh` completes, push the first tag to trigger the publish workflow:

```bash
gh repo clone foundry-linux/foundry-apt /tmp/foundry-apt-release
git -C /tmp/foundry-apt-release tag v0.0.1
git -C /tmp/foundry-apt-release push origin v0.0.1
# Watch: https://github.com/foundry-linux/foundry-apt/actions
```

## Status

- [ ] `bootstrap.sh` run to completion
- [ ] First tag `v0.0.1` pushed
- [ ] `publish.yml` workflow green
- [ ] `smoke-install` job passes
