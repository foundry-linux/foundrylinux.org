# foundry-apt

The signed APT repo for [Foundry Linux](https://foundrylinux.org), hosted at `apt.foundrylinux.org`.

## Quick start

```bash
curl -fsSL https://apt.foundrylinux.org/key.gpg \
  | sudo gpg --dearmor -o /etc/apt/keyrings/foundry.gpg
echo "deb [signed-by=/etc/apt/keyrings/foundry.gpg] https://apt.foundrylinux.org resolute main" \
  | sudo tee /etc/apt/sources.list.d/foundry.list
sudo apt update
sudo apt install foundry-linux-retro-tools
```

`resolute` is the suite name — Kubuntu 26.04 LTS "Resolute Raccoon", the only supported base.

## Metapackages

| Package | Pulls in | When you want it |
|---|---|---|
| `foundry-linux-retro-tools` | [mame](https://www.mamedev.org/), [dasm](https://dasm-assembler.github.io/), [cc65](https://cc65.github.io/), z80*, [radare2](https://www.radare.org/), [binwalk](https://github.com/ReFirmLabs/binwalk), [sox](http://sox.sourceforge.net/), m68k binutils, [xa65](https://www.floodgap.com/retrotech/xa/), [f9dasm](https://github.com/Arakula/f9dasm), [ghidra](https://ghidra-sre.org/), [vgmstream](https://vgmstream.org/), [libvgm](https://github.com/ValleyBell/libvgm) | Retro/arcade ROM tools |

## Vendored upstream packages

Packages not in the Ubuntu archive, repackaged and shipped from this repo via [`dpkg-buildpackage`](https://manpages.debian.org/dpkg-dev/dpkg-buildpackage.1.html) using the canonical Debian source-package layout:

| Package | Upstream | Notes |
|---|---|---|
| `f9dasm` | [Arakula/f9dasm](https://github.com/Arakula/f9dasm) | Motorola 6800/6809/6309 family disassembler. amd64. |

See [`LICENSES-VENDORED.md`](LICENSES-VENDORED.md) for the running attribution list. Use the [`/package`](https://github.com/anthropics/claude-code) Claude Code skill to add new ones — it generates the Debian source tree via `dh_make` and wires it into this repo.

Two upstreams we *don't* repackage:

- [`task`](https://taskfile.dev) — official Cloudsmith apt repo covers it; Phase 0's `foundry-linux-setup/install-task.sh` configures the repo directly.
- [`xa65`](https://www.floodgap.com/retrotech/xa/) — Ubuntu 26.04 universe ships it as `xa65 2.4.1-0.1build1`; foundry-linux-retro-tools `Depends:` it from there.

## Repo layout

```
foundry-apt/
  packages/<name>/              every package, metapackage or vendored upstream
    debian/                     Debian source-package format (canonical)
      control                   Source: + Package: stanzas
      changelog                 authoritative version (read via dpkg-parsechangelog)
      rules                     one-line "%: dh $@"
      source/format             3.0 (native) for metapackages
                                3.0 (quilt)  for vendored upstreams
      copyright                 DEP-5 format
      [patches/series]          optional quilt patches for vendored upstreams
      [watch]                   optional uscan tracker for vendored upstreams
    build.sh                    only for vendored upstreams: fetches sha256-pinned
                                tarball, overlays debian/, runs dpkg-buildpackage
  aptly/
    aptly.conf                  Local aptly config (rootDir, architectures)
  scripts/
    build-all.sh                dispatch: build.sh wrapper, else dpkg-buildpackage
    init-repo.sh                aptly repo create (idempotent)
    publish-local.sh            aptly publish repo → ./public/
    generate-index.sh           parse packages/*/debian/ → public/index.html
    sign.sh                     CI-side: import GPG key from secret, sign Release
  .github/workflows/
    test.yml                    PR: shellcheck + build all .debs + sanity check
    publish.yml                 tag push: build + sign + sync to Cloudflare R2
  dist/                         build output (gitignored)
  public/                       published apt repo (gitignored)
  Taskfile.yml                  task wrappers around scripts/*
  docs/infra-setup.md           one-time setup: R2 bucket, GPG, DNS
```

## Local development

```bash
bash scripts/build-all.sh
bash scripts/init-repo.sh       # → ~/.aptly/foundry repo
bash scripts/publish-local.sh   # → ./public/ apt tree
apt-cache depends foundry-linux-retro-tools
```

## Adding or upgrading a package

For **metapackages** (just a `Depends:` list — no upstream tarball):

1. Update `packages/<name>/debian/control` (`Depends:`, `Recommends:`, etc.).
2. Add a top entry to `packages/<name>/debian/changelog` — use `dch -v <new-version> -D resolute "what changed"` from inside the package dir, or hand-edit:

   ```
   <name> (<new-version>) resolute; urgency=medium

     * What changed.

    -- Maintainer <email>  <rfc-2822 date>
   ```
3. `task build` to verify.

For **vendored upstream packages** (e.g. `f9dasm`):

1. Bump `UPSTREAM_VERSION` + `SHA256` at the top of `packages/<name>/build.sh`. Re-pin with `curl -fsSL <upstream-url> | sha256sum`.
2. Add a top entry to `packages/<name>/debian/changelog`.
3. `task build`.

For a **new vendored upstream**, use the [`/package`](#) Claude Code skill — it does the universe-check, `dh_make` scaffolding, and template substitution in one go.

See [`CONTRIBUTING.md`](CONTRIBUTING.md) for full instructions.

To release: `task bump` (auto-tags the next patch version) — the [publish workflow](.github/workflows/publish.yml) builds, signs, and syncs to R2 automatically.

## Hosting

- **APT repo:** [Cloudflare R2](https://www.cloudflare.com/developer-platform/products/r2/) (10 GB free tier, zero egress) → `apt.foundrylinux.org`
- **Signing key:** GPG key in GitHub Actions secrets (CI use), backed up to a private `foundry-linux-secrets` R2 bucket for disaster recovery. No AWS account required.

Detailed setup in [`docs/infra-setup.md`](docs/infra-setup.md).

## License

GPL-2.
