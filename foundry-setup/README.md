# foundry-setup

**Phase 0 of the [Foundry Linux](https://foundrylinux.org) distro** — scripts that bootstrap a vanilla Ubuntu 26.04 LTS into a Foundry Linux game-dev workstation.

## Quick start

Wire the apt sources, then install an edition:

```bash
curl -fsSL https://foundrylinux.org/setup.sh | bash
sudo apt install foundry-anvil   # base layer — devbox + KDE
```

`setup.sh` is idempotent: safe to re-run. After it runs, `apt upgrade` keeps everything current.

## Local install (contributors / offline)

If you've cloned this repo:

```bash
bash install.sh                     # default: role "both" (game-dev + engine-dev)
bash install.sh --role anvil        # Phase 1 edition (preferred)
bash install.sh --dry-run           # print the plan without executing
```

> **Note:** `install.sh` sources `lib.sh` and sibling `install-*.sh` scripts from the same
> directory — it requires a local clone. The public curl-bash URL is `setup.sh`, not `install.sh`.

## What it does

**`site/setup.sh`** (the public curl path):

1. Adds `apt.foundrylinux.org` (Foundry metapackages)
2. Adds `apt.worldfoundry.org` (WorldFoundry stack — cross-repo `Depends`)
3. Adds the Cloudsmith `task` repo (go-task runner)
4. Enables Ubuntu `multiverse` (ROM-bundled emulators, NDK, etc.)
5. Runs `apt-get update`

Then `sudo apt install foundry-<edition>` finishes the job.

**`install.sh`** (local clone only) additionally:

- Checks Ubuntu 26.04 LTS (errors on any other release — bypass with `--force`)
- Dispatches to per-metapackage installers for legacy roles
- For Phase 1 editions (`anvil`/`sprite`/`atelier`): just wires sources + runs `apt install`

WF engine workspace setup (Rust, repo clones, wftools build, Blender addon) is handled by
`setup-wf-workspace.sh` (tracked in the WorldFoundry repo).

## Editions and roles

Phase 1 editions (`--role`, preferred):

| Edition | Metapackage | Contents |
|---|---|---|
| `anvil` *(default)* | `foundry-anvil` | Base dev toolkit + KDE |
| `sprite` | `foundry-sprite` | anvil + heavy graphics + audio |
| `atelier` | `foundry-atelier` | sprite + emulators + free games + mobile dev |

Legacy roles (kept for back-compat with pre-edition installs):

| Role | Description |
|---|---|
| `play` | no dev tools (no runtime metapackage yet) |
| `game-dev` | engine-build-deps + Blender + retro-tools + task |
| `engine-dev` | engine-build-deps + retro-tools + task |
| `both` | game-dev + engine-dev |
| `maintainer` | both + android/iOS dev + Foundry repo clone |

## Options

```
--role ROLE       Edition or legacy role (default: both)
--skip-blender    Skip foundry-blender (legacy roles only)
--skip-retro      Skip foundry-retro-tools (legacy roles only)
--apt-only        Forwarded to retro-tools: skip source-build sidecars
--skip-clone      Skip Foundry repo clone (maintainer role only)
--force           Bypass distro / version checks
-n, --dry-run     Print the plan without executing
-h, --help        Show help
```

## Testing

```bash
bash test/run-test.sh              # dry-run in Docker (Ubuntu 26.04)
bash test/run-test.sh --real       # full install test (~5–10 min)
```

CI runs the dry-run test on every push via `.github/workflows/test.yml`.

## Repo structure

```
foundry-setup/
  install.sh                        Local multi-role installer (not a curl URL)
  lib.sh                            Shared helpers (colors, logging, apt wrappers)
  setup-foundry-apt-source.sh       Wire apt.foundrylinux.org
  setup-worldfoundry-apt-source.sh  Wire apt.worldfoundry.org
  install-foundry-*.sh              Per-metapackage installers (used by install.sh)
  install-task.sh
  README.md
  test/
    run-test.sh
    Dockerfile.ubuntu26.04
  .github/workflows/test.yml

site/setup.sh                       Public curl-bash payload → https://foundrylinux.org/setup.sh
```

## License

GPL-2 (matches the WF engine licence).

## Related

- [Foundry Linux distro proposal](../docs/investigations/2026-05-16-foundry-distro-proposal.md)
- [`site/setup.sh`](../site/setup.sh) — the public curl-bash installer
- [`foundry-apt/`](../foundry-apt/) — Phase 1 apt repo (the `.deb` packages that `apt install` resolves)
