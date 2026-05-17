# foundry-apt

The signed APT repo for [Foundry Linux](https://foundrylinux.org), hosted at `apt.foundrylinux.org`.

## Quick start

```bash
curl -fsSL https://apt.foundrylinux.org/key.gpg \
  | sudo gpg --dearmor -o /etc/apt/keyrings/foundry.gpg
echo "deb [signed-by=/etc/apt/keyrings/foundry.gpg] https://apt.foundrylinux.org resolute main" \
  | sudo tee /etc/apt/sources.list.d/foundry.list
sudo apt update
sudo apt install worldfoundry-dev
```

`resolute` is the suite name (Kubuntu 26.04 "Resolute Raccoon"). A `noble` alias covers 24.04.

## Metapackages

| Package | Pulls in | When you want it |
|---|---|---|
| **`worldfoundry-dev`** | everything below | Default — most contributors |
| `worldfoundry-engine-build-deps` | build-essential, cmake, libx11, libgl, gdb, xxd, pkg-config, git | Just compiling the engine |
| `worldfoundry-blender` | [Blender](https://www.blender.org/) 4.2+ + python3 + engine-build-deps | Authoring levels |
| `worldfoundry-retro-tools` | [mame](https://www.mamedev.org/), [dasm](https://dasm-assembler.github.io/), [cc65](https://cc65.github.io/), z80*, [radare2](https://www.radare.org/), [binwalk](https://github.com/ReFirmLabs/binwalk), [sox](http://sox.sourceforge.net/), m68k binutils; Recommends [ghidra](https://ghidra-sre.org/), [f9dasm](http://www.df.lth.se.orbin.se/~triad/f9dasm/), [vgmstream](https://vgmstream.org/), [libvgm](https://github.com/ValleyBell/libvgm), [xa65](https://www.floodgap.com/retrotech/xa/) | Porting arcade ROMs |
| `worldfoundry-android-dev` | JDK 17, adb, NDK r26c | Cross-compiling for Android (separate because ~3 GB) |

## Vendored upstream packages

Packages not in the Ubuntu archive, repackaged and shipped from this repo:

| Package | Upstream | Pinned version | Notes |
|---|---|---|---|
| `task` | [go-task/task](https://github.com/go-task/task) | 3.51.1 | Repackaged from upstream release binary. Build script: [`packages/task/build.sh`](packages/task/build.sh). amd64 + arm64. |

Planned (v1.1+): [ghidra](https://ghidra-sre.org/), [f9dasm](http://www.df.lth.se.orbin.se/~triad/f9dasm/), [vgmstream](https://vgmstream.org/), [libvgm](https://github.com/ValleyBell/libvgm), [xa65](https://www.floodgap.com/retrotech/xa/). See [`LICENSES-VENDORED.md`](LICENSES-VENDORED.md) for the running attribution list.

## Repo layout

```
foundry-apt/
  packages/
    worldfoundry-dev/DEBIAN/control
    worldfoundry-engine-build-deps/DEBIAN/control
    worldfoundry-blender/DEBIAN/control
    worldfoundry-retro-tools/DEBIAN/control
    worldfoundry-android-dev/DEBIAN/control
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
apt-cache depends worldfoundry-dev
```

## Adding or upgrading a package

For **metapackages** (just a Depends list):

1. Bump `Version:` in `packages/<name>/DEBIAN/control`.
2. Update `Depends:` / `Recommends:` as needed.
3. `bash scripts/build-all.sh` to verify.

For **vendored upstream packages** (e.g. `task`):

1. Edit the version + sha256 at the top of `packages/<name>/build.sh`.
2. Re-pin: `curl -fsSL <url> | sha256sum`.
3. `bash scripts/build-all.sh`.

See [`CONTRIBUTING.md`](CONTRIBUTING.md) for full instructions.

To release: `git tag v0.0.1 && git push origin v0.0.1` — the [publish workflow](.github/workflows/publish.yml) builds, signs, and syncs to R2 automatically.

## Hosting

- **APT repo:** [Cloudflare R2](https://www.cloudflare.com/developer-platform/products/r2/) (10 GB free tier, zero egress) → `apt.foundrylinux.org`
- **Signing key:** [AWS SSM SecureString](https://docs.aws.amazon.com/systems-manager/latest/userguide/sysman-paramstore-securestring.html) + [GitHub OIDC federation](https://docs.github.com/en/actions/deployment/security-hardening-your-deployments/about-security-hardening-with-openid-connect) — no long-lived AWS keys in repo secrets

Detailed setup in [`docs/infra-setup.md`](docs/infra-setup.md).

## License

GPL-2.
