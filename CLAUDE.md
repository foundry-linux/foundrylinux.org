# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Architecture

Two sibling directories, two phases:

| Directory | Purpose |
|-----------|---------|
| `foundry-linux-setup/` | **Phase 0** — bash curl installer; runs on a stock Ubuntu box |
| `foundry-apt/` | **Phase 1** — signed APT repo + `worldfoundry-*` metapackages |

The distro roadmap: Phase 0 (this repo) → Phase 1 (APT repo, this repo) → Phase 2 (`ghcr.io/worldfoundry/devbox:26.04` Distrobox) → Phase 3 (Foundry Linux ISO).

### Phase 0: per-metapackage script pattern

Each `foundry-linux-setup/install-<metapackage>.sh` is the Phase 0 expansion of one APT metapackage: its `Depends:` become `apt-get install` calls; its `Recommends:` that aren't in Ubuntu become source-builds under `~/opt/`. The Phase 1 collapse path is mechanical — replace the script body with a single `apt install <metapackage>` call once `apt.worldfoundry.org` ships.

```
foundry-linux-setup/
  lib.sh                                     shared helpers (colors, logging, run_sudo, apt_update)
  install.sh                                 orchestrator: distro check, role dispatch, summary
  install-worldfoundry-engine-build-deps.sh  build-essential, cmake, libx11, libgl, gdb, xxd …
  install-worldfoundry-blender.sh            blender ≥4.2, python3
  install-worldfoundry-retro-tools.sh        mame, cc65 (sim65+da65), dasm, radare2 + xa65/ghidra/f9dasm/vgmstream/libvgm source-builds → ~/opt/
  install-worldfoundry-android-dev.sh        openjdk-17-jdk, adb, NDK r26c
  install-task.sh                            go-task binary
  install-worldfoundry-dev.sh                orchestrator: calls the above in dependency order
```

`install.sh` dispatches by `--role`: `engine-dev` (engine-build-deps+task+retro), `game-dev` (+blender), `both`/`maintainer` → `install-worldfoundry-dev.sh`. WF repo cloning, Rust install, and wftools build are **not** in these scripts — they live in `setup-wf-workspace.sh` (tracked in the WorldFoundry repo).

### Phase 1: APT repo

```
foundry-apt/
  packages/<metapackage>/DEBIAN/control   one per metapackage
  packages/task/build.sh                  fetch upstream binary + build .deb
  scripts/build-all.sh                    build every .deb → dist/
  scripts/init-repo.sh                    aptly repo create (idempotent)
  scripts/publish-local.sh               aptly publish → ./public/
  .github/workflows/publish.yml          tag push → build + sign + sync to Cloudflare R2
```

Hosted on Cloudflare R2 at `apt.worldfoundry.org`. GPG signing key in AWS SSM SecureString; CI uses GitHub OIDC federation (no long-lived AWS keys). See `foundry-apt/docs/infra-setup.md` for one-time setup checklist.

## Key Commands

```bash
# foundry-apt
task build                       # build all .debs → dist/
task verify                      # dpkg-deb --info each .deb
task shellcheck                  # lint scripts/*.sh + packages/*/build.sh
task publish-local               # build + aptly publish → ./public/ (smoke test)
task apt-test                    # point apt at ./public/ and resolve worldfoundry-dev

# foundry-linux-setup
bash foundry-linux-setup/install.sh --dry-run
bash foundry-linux-setup/install.sh --role engine-dev --dry-run
bash foundry-linux-setup/install-worldfoundry-retro-tools.sh --dry-run
bash foundry-linux-setup/install-worldfoundry-retro-tools.sh --apt-only   # skip ~/opt/ source-builds

# Docker test harness (requires docker)
bash foundry-linux-setup/test/run-test.sh                  # dry-run, Ubuntu 24.04 + 26.04
bash foundry-linux-setup/test/run-test.sh --real           # full install (slow, needs network)
bash foundry-linux-setup/test/run-test.sh --version 26.04 --real
```

To release Phase 1: `git tag v1.0.1 && git push origin v1.0.1` — the publish workflow builds, signs, and syncs to R2 automatically.

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

**Metapackage version bumps:** `1.0.x` for dep changes, `1.x.0` for new packages. After editing `DEBIAN/control`, run `task build` to verify.

**Vendored upstreams** (e.g. `packages/task/`): edit the version + sha256 at the top of `build.sh`, re-pin with `curl -fsSL <url> | sha256sum`, then `task build`.
