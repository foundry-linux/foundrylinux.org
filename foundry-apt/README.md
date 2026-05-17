# foundry-apt

The signed APT repo for [Foundry Linux](https://foundrylinux.org), hosted at `apt.foundrylinux.org`.

## Quick start

```bash
curl -fsSL https://apt.foundrylinux.org/key.gpg \
  | sudo gpg --dearmor -o /etc/apt/keyrings/foundry.gpg
echo "deb [signed-by=/etc/apt/keyrings/foundry.gpg] https://apt.foundrylinux.org resolute main" \
  | sudo tee /etc/apt/sources.list.d/foundry.list
sudo apt update
```

`resolute` is the suite name (Kubuntu 26.04 "Resolute Raccoon"). A `noble` alias covers 24.04.

## Packages

| Package | Upstream | Notes |
|---|---|---|
| `task` | [go-task/task](https://github.com/go-task/task) | Repackaged from upstream release binary. amd64 + arm64. |

See [`LICENSES-VENDORED.md`](LICENSES-VENDORED.md) for upstream attribution.

## Repo layout

```
foundry-apt/
  packages/
    task/build.sh               Fetches upstream binary + builds .deb
  aptly/
    aptly.conf                  Local aptly config (rootDir, architectures)
  scripts/
    build-all.sh                Runs build.sh if present, else dpkg-deb --build
    init-repo.sh                aptly repo create (idempotent)
    publish-local.sh            aptly publish repo → ./public/
    sign.sh                     CI-side: fetch GPG key from AWS SSM, sign Release
  .github/workflows/
    test.yml                    PR: shellcheck + build all .debs + sanity check
    publish.yml                 tag push: build + sign + sync to Cloudflare R2
  dist/                         build output (gitignored)
  public/                       published apt repo (gitignored)
  Taskfile.yml                  task wrappers around scripts/*
  docs/infra-setup.md           one-time setup: R2 bucket, AWS SSM, GPG, DNS
```

## Local development

```bash
bash scripts/build-all.sh
bash scripts/init-repo.sh       # → ~/.aptly/foundry repo
bash scripts/publish-local.sh   # → ./public/ apt tree
```

To release: `git tag v0.0.1 && git push origin v0.0.1` — the [publish workflow](.github/workflows/publish.yml) builds, signs, and syncs to R2 automatically.

## Adding a vendored upstream package

1. `mkdir packages/<name>/`
2. Write `packages/<name>/build.sh` — fetch upstream artefact (pin SHA256), stage files, write `staging/DEBIAN/control`, run `dpkg-deb --build`.
3. `bash scripts/build-all.sh` to verify.
4. Add upstream licence to `LICENSES-VENDORED.md`.

## Hosting

- **APT repo:** [Cloudflare R2](https://www.cloudflare.com/developer-platform/products/r2/) (10 GB free tier, zero egress) → `apt.foundrylinux.org`
- **Signing key:** [AWS SSM SecureString](https://docs.aws.amazon.com/systems-manager/latest/userguide/sysman-paramstore-securestring.html) + [GitHub OIDC federation](https://docs.github.com/en/actions/deployment/security-hardening-your-deployments/about-security-hardening-with-openid-connect) — no long-lived AWS keys in repo secrets

Detailed setup in [`docs/infra-setup.md`](docs/infra-setup.md).

## License

GPL-2.
