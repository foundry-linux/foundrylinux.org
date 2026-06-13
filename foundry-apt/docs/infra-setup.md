# foundry-apt infrastructure setup

One-time setup needed before [`publish.yml`](../.github/workflows/publish.yml) can build, sign,
and sync the APT repo to `apt.foundrylinux.org`. Everything is automated via a single script.
No AWS account required.

## Run bootstrap.sh

From the `foundrylinux.org` repo root:

```bash
bash scripts/bootstrap.sh --dry-run   # preview all steps
bash scripts/bootstrap.sh             # run for real
```

If `CF_API_TOKEN` is not already exported, the script prompts you to create a scoped operator
token at <https://dash.cloudflare.com/profile/api-tokens> (two permissions: Workers R2 Storage
Edit + Zone DNS Edit), then paste the value. The token is stored immediately to a private
`foundry-linux-secrets` R2 bucket so subsequent runs can skip the prompt if re-exported from
there.

See [`docs/plans/2026-05-17-foundry-phase1-bootstrap.md`](../../docs/plans/2026-05-17-foundry-phase1-bootstrap.md)
for the full step-by-step breakdown and status checklist.

## First release

Once `bootstrap.sh` completes, push the first tag to trigger the publish workflow:

```bash
task bump    # auto-increments patch version and triggers publish.yml
# Watch: https://github.com/foundry-linux/foundry-apt/actions
```

## Status

- [ ] `bootstrap.sh` run to completion
- [ ] First tag `v0.0.1` pushed
- [ ] `publish.yml` workflow green
- [ ] `smoke-install` job passes
