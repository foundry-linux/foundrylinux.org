# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Architecture

Two sibling directories, two phases:

| Directory | Purpose |
|-----------|---------|
| `foundry-linux-setup/` | **Phase 0** — bash curl installer; runs on a stock Ubuntu box |
| `foundry-apt/` | **Phase 1** — signed APT repo (metapackages + vendored upstreams not in Ubuntu, e.g. `f9dasm`) |

The distro roadmap: Phase 0 (this repo) → Phase 1 (APT repo, this repo) → Phase 2 (`ghcr.io/foundry-linux/devbox:26.04` Distrobox) → Phase 3 (Foundry Linux ISO).

### Phase 0: per-metapackage script pattern

Each `foundry-linux-setup/install-<metapackage>.sh` is the Phase 0 expansion of one APT metapackage: its `Depends:` become `apt-get install` calls; its `Recommends:` that aren't in Ubuntu become source-builds under `~/opt/`. The Phase 1 collapse path is mechanical — replace the script body with a single `apt install <package>` call once `apt.foundrylinux.org` ships.

```
foundry-linux-setup/
  lib.sh                                     shared helpers (colors, logging, run_sudo, apt_update)
  install.sh                                 orchestrator: distro check, role dispatch, summary
  install-foundry-linux-engine-build-deps.sh  build-essential, cmake, libx11, libgl, gdb, xxd …
  install-foundry-linux-blender.sh            blender ≥4.2, python3
  install-foundry-linux-retro-tools.sh        mame, cc65 (sim65+da65), dasm, radare2 + xa65/ghidra/f9dasm/vgmstream/libvgm source-builds → ~/opt/
  install-foundry-linux-android-dev.sh        openjdk-17-jdk, adb, NDK r26c
  install-task.sh                            go-task binary
  install-foundry-linux-dev.sh                orchestrator: calls the above in dependency order
```

`install.sh` dispatches by `--role`: `engine-dev` (engine-build-deps+task+retro), `game-dev` (+blender), `both`/`maintainer` → `install-foundry-linux-dev.sh`. WF repo cloning, Rust install, and wftools build are **not** in these scripts — they live in `setup-wf-workspace.sh` (tracked in the WorldFoundry repo).

### Phase 1: APT repo

Every package in `foundry-apt/packages/` uses the **canonical Debian source-package layout** — the same `debian/{control,changelog,rules,source/format,copyright}` tree that real Debian and Ubuntu archive packages use. `dpkg-buildpackage` consumes it and produces a `.deb`. There is no `DEBIAN/` (uppercase) authored directory — that's a generated artifact inside built `.deb`s, not authored input.

```
foundry-apt/
  packages/<name>/debian/                  Debian source-package format (canonical)
    control                                Source: + Package: stanzas
    changelog                              authoritative version source
    rules                                  one-line "%: dh $@" (executable)
    source/format                          "3.0 (native)" for metapackages
                                           "3.0 (quilt)" for vendored upstreams
    copyright                              DEP-5 format
    [patches/series]                       optional quilt patches for vendored upstreams
    [watch]                                optional uscan tracker for vendored upstreams
  packages/<name>/build.sh                 only for vendored upstreams (e.g. f9dasm) —
                                           fetches sha256-pinned tarball, overlays debian/,
                                           runs dpkg-buildpackage
  scripts/build-all.sh                     dispatch: build.sh wrapper, else dpkg-buildpackage
  scripts/init-repo.sh                     aptly repo create (idempotent)
  scripts/publish-local.sh                 aptly publish → ./public/
  scripts/generate-index.sh                parse packages/*/debian/ → public/index.html
  .github/workflows/publish.yml            tag push → build + sign + sync to Cloudflare R2
```

To package a new upstream as a `.deb`, use the **`/package` skill** (`~/.claude/skills/package/SKILL.md`) — it checks Ubuntu universe first, generates the `debian/` tree via `dh_make`, and patches it with Foundry-customized fields. Don't hand-roll `dpkg-deb --build`.

Hosted on Cloudflare R2 at `apt.foundrylinux.org`. Credentials: GPG signing key + R2 CI tokens in GitHub Actions secrets (CI use); all backed up to a private `foundry-linux-secrets` R2 bucket for disaster recovery. No AWS account required. See `foundry-apt/docs/infra-setup.md` for one-time setup checklist.

## Key Commands

```bash
# foundry-apt
task build                       # build all .debs → dist/
task verify                      # dpkg-deb --info each .deb
task shellcheck                  # lint scripts/*.sh + packages/*/build.sh
task publish-local               # build + aptly publish → ./public/ (smoke test)
task apt-test                    # point apt at ./public/ and resolve foundry-linux-dev

# foundry-linux-setup
bash foundry-linux-setup/install.sh --dry-run
bash foundry-linux-setup/install.sh --role engine-dev --dry-run
bash foundry-linux-setup/install-foundry-linux-retro-tools.sh --dry-run
bash foundry-linux-setup/install-foundry-linux-retro-tools.sh --apt-only   # skip ~/opt/ source-builds

# Docker test harness (requires docker)
bash foundry-linux-setup/test/run-test.sh                  # dry-run, Ubuntu 24.04 + 26.04
bash foundry-linux-setup/test/run-test.sh --real           # full install (slow, needs network)
bash foundry-linux-setup/test/run-test.sh --version 26.04 --real
```

To release Phase 1: `git tag v1.0.1 && git push origin v1.0.1` — the publish workflow builds, signs, and syncs to R2 automatically.

## Infrastructure automation mandate

**Everything is scripted — no manual console steps.** All site and infrastructure setup (GitHub repos, Cloudflare DNS/R2, GPG keys) must be executed via scripts, CLI tools (`gh`, `curl`/`wrangler`), or Terraform. Never describe a step as "click X in the console" — translate it into the equivalent CLI command or script block. If a one-time action genuinely has no CLI path, document exactly why and what the minimum manual surface is.

**Credentials and keys for automation only.** API tokens, access keys, and role ARNs are for non-interactive script/CI use. The private GPG signing key and R2 CI tokens go into GitHub Actions secrets (for CI use) and are backed up to the private `foundry-linux-secrets` R2 bucket (for disaster recovery) — the local copy is shredded immediately. No credentials live in local files, `.env`, or are typed interactively more than once. No AWS account or SSM is used by this project.

**Scripts are the source of truth.** `foundry-apt/scripts/` and any future `scripts/setup-*.sh` files must be runnable end-to-end (idempotent) to recreate the full stack from a blank account. A step that cannot be reproduced from scripts is not done.

---

## Conventions

**Every install script:**
- `set -euo pipefail` at the top.
- `-h`/`--help` short-circuits before any apt calls, sudo prompts, or argument validation — exits 0.
- Sources `lib.sh` if available; falls back to a tiny inline shim so the script works standalone.
- Idempotent: source-build sidecars guard with `[[ -d ~/opt/<tool> ]]`; skip unless `--force`.
- Respects `FOUNDRY_LOG_FILE` env var (set by top-level `install.sh`).

**Use `apt_update()` from lib.sh**, not `run_sudo apt-get update` directly. `apt-get update` exits non-zero on stale repo entries; `apt_update()` demotes that to a warning so `set -e` doesn't abort the whole install.

**GitHub orgs:**
- `foundry-linux` — this repo and all Foundry Linux distro repos
- `wbniv` — WorldFoundry engine repos (WorldFoundry, wf-games); out of scope here

**Package version bumps:** edit `debian/changelog` (add a new top entry — preferably via `dch -v <NEW> -D resolute "what changed"`). Don't put `Version:` in `debian/control`; the changelog is authoritative. `1.0.x` for dep changes, `1.x.0` for new packages. After bumping, `task build` to verify.

**Vendored upstreams** (e.g. `packages/f9dasm/`): edit `UPSTREAM_VERSION` + `SHA256` at the top of `build.sh`, re-pin with `curl -fsSL <url> | sha256sum`, add a new `debian/changelog` entry, then `task build`. Use the `/package` skill for new vendored packages — see [`docs/plans/2026-05-18-package-skill.md`](docs/plans/2026-05-18-package-skill.md).

**Always check Ubuntu universe first.** Before packaging *any* source-built tool, run `apt-cache policy <pkg>` on a fresh `ubuntu:26.04` container. If universe ships it, add it to the Phase 0 apt-install list and the metapackage `Depends:` — don't duplicate. (Lesson from xa65, which we accidentally re-packaged when it was already in 26.04 universe.)
