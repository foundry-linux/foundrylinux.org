# foundry-setup

**Phase 0 of the [Foundry Linux](https://foundrylinux.org) distro plan** — a curl-bash installer that bootstraps a vanilla [Kubuntu 26.04 LTS](https://kubuntu.org/) (or any Ubuntu-family 26.04) into a Foundry Linux game-dev workstation.

## Quick start

```bash
curl -fsSL https://foundrylinux.org/install.sh | bash
```

Or, if you've cloned this repo:

```bash
bash install.sh
```

## What it does

1. **Detects** Ubuntu-family 26.04 LTS (errors on any other release — bypass with `--force`, untested)
2. **Installs apt packages** matching [`Taskfile.yml`'s `dev-setup`](../Taskfile.yml) target (build-essential, cmake, libx11-dev, libgl1-mesa-dev, etc.) plus a few helpers (curl, gnupg, software-properties-common)
3. **Installs [task](https://taskfile.dev/)** (WF's primary command runner) to `~/.local/bin/`
4. **Installs Rust** via [rustup](https://rustup.rs/) + [maturin](https://www.maturin.rs/) (for the `wf_core.so` Blender addon build)
5. **Clones role-appropriate WF repos** into `~/Projects/` using `--depth 1 --filter=blob:none` (~5–25 MB depending on role) — see [Roles](#roles)
6. **Builds [wftools](../wftools/)** (`cargo build --release` — `iffcomp`, `levcomp`, `textile`, `chargrab`)
7. **Installs the [WF Blender addon](../wftools/wf_blender/install.sh)** into the user's Blender extensions directory

Logs everything to `~/.local/state/foundry-install.log`.

**Idempotent** — safe to re-run for upgrades or to add roles later.

## Roles

The installer prompts (or accepts `--role`) for one of these:

| Role | Clones | Use case |
|---|---|---|
| `play` | none | Just want to play WF games |
| `game-dev` | `wf-games` | Authoring WF games / arcade ports |
| `engine-dev` | `WorldFoundry.2026-new-level` (sparse, excludes `engine/vendor`) | Hacking on the engine itself |
| `both` *(default)* | both of the above | Most contributors |
| `maintainer` | adds `foundry-*` repos | Building the distro itself |

## Options

```
--role ROLE       Install role (play, game-dev, engine-dev, both, maintainer)
--skip-rust       Skip Rust toolchain
--skip-blender    Skip Blender addon
--skip-clone      Skip cloning WF repos
--skip-build      Skip building wftools
--force           Bypass distro / version checks
-n, --dry-run     Print the plan without executing anything
-h, --help        Show help
```

## Phase 0 vs Phase 1+

This script is **Phase 0** of the [distro plan](../docs/investigations/2026-05-16-foundry-distro-proposal.md): the lowest-friction onboarding path that works before our APT repo exists.

It's the **only** path where Foundry Linux compiles tooling on the user's machine. **Phase 1** (the APT repo on Cloudflare R2) ships prebuilt `.deb`s for `wftools`, `wf-launcher`, the Blender addon, and everything else — at that point this script collapses to ~20 lines: add the apt source, `apt install foundry-dev`, clone the repos.

## Testing

The repo includes:

- **`test/Dockerfile.ubuntu26.04`** — minimal image for testing the script end-to-end in CI without burning a real VM
- **`test/run-test.sh`** — runs `install.sh --dry-run` inside the test image; extends to a real install with `--real`
- **`.github/workflows/test.yml`** — GitHub Actions runs the dry-run tests on every push

Test locally:

```bash
bash test/run-test.sh                    # dry-run on Ubuntu 26.04
bash test/run-test.sh --real             # full install test (slow, ~5–10 min)
```

## Repo structure

```
foundry-setup/
  install.sh                Main installer
  README.md                 This file
  LICENSE                   GPL-2 (matches the WF engine licence)
  test/
    Dockerfile.ubuntu26.04
    run-test.sh
  .github/
    workflows/
      test.yml              CI: dry-run install on Ubuntu 26.04
```

## Hosting

The canonical URL `https://foundrylinux.org/install.sh` is served from this repo via Cloudflare Pages with a redirect to the latest `install.sh` on `main`. See `docs/hosting.md` (TBD) for the DNS / Pages setup steps.

## License

GPL-2 (matches the WF engine licence).

## Related

- [Foundry Linux distro proposal](../docs/investigations/2026-05-16-foundry-distro-proposal.md) — the full plan this script implements
- [`Taskfile.yml`](../Taskfile.yml) — the `dev-setup`, `blender-install`, and other tasks this script wraps
- [`wftools/wf_blender/install.sh`](../wftools/wf_blender/install.sh) — Blender addon installer reused by this script
- [`engine/build_game.sh`](../engine/build_game.sh) — canonical engine build (run via `task build` after install)
