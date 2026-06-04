# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Architecture

**Base: Ubuntu 26.04 LTS ("Resolute Raccoon"), period.** No Debian, no Steam Runtime, no 24.04 fallback.

Phase 0 (bash installer) lives in this repo; Phase 1 (signed apt repos) ships across **two permanently separate** repos with disjoint scopes:

| Repo | Scope | Source tree |
|---|---|---|
| `apt.foundrylinux.org` | Foundry Linux distro toolchain — `foundry-*` edition/category metapackages + **14 vendored upstreams** (`f9dasm`, `ghidra`, `libvgm`, `vgmstream`, `ppsspp`, `snes9x-gtk`, `task`, `ruff`, `python3-{glfw,librosa,mss,pydub}`, `blender-asset-finder{,-cli}` — full list in [`foundry-apt/LICENSES-VENDORED.md`](foundry-apt/LICENSES-VENDORED.md)) | this repo, `foundry-apt/` |
| `apt.worldfoundry.org` | WorldFoundry-specific authoring tools — 9 CLIs (cdpack, iffcomp, iffdump, levcomp, lvldump, oaddump, oas2oad, prep, textile) + 1 Blender add-on (`worldfoundry-blender-editor-exporter`) + 4 umbrella metapackages (`worldfoundry`, `worldfoundry-cli`, `worldfoundry-blender-addons`, `worldfoundry-development`) | sibling repo `../worldfoundry.org/apt/` |

The two repos are **deliberately separate**: foundry-linux is the distribution; WorldFoundry is one tenant's authoring stack. The separation is at the **pipeline** level — neither repo's build/publish depends on the other; they tag and ship from independent namespaces (`v*` vs `apt-v*`). At the **package** level they are intentionally cross‑coupled, though: `foundry-core` `Depends: worldfoundry` and `foundry-atelier` `Depends: worldfoundry-development` (both in apt.worldfoundry.org), while `worldfoundry-cli`/`-blender-addons` `Depend` on `blender-asset-finder*` (in apt.foundrylinux.org). So **both apt sources must be wired for either to resolve** — Phase 0's two `setup-*-apt-source.sh` scripts guarantee that. "Co‑installed, independently published" — not "independently installable."

Distro roadmap: Phase 0 (this repo) → Phase 1 (both apt repos, live) → Phase 2 (`ghcr.io/foundry-linux/devbox:26.04` Distrobox) → Phase 3 (Foundry Linux ISO).

### Editions (the metapackage tiers)

`apt.foundrylinux.org` ships a tiered hierarchy of **edition** metapackages plus à‑la‑carte **category** metapackages (full table in [`foundry-apt/README.md`](foundry-apt/README.md)). The edition nesting:

```
foundry-core ⊆ foundry-anvil ⊆ foundry-sprite ⊆ foundry-atelier        (foundry-desktop adds the KDE layer)

foundry-core      desktop-agnostic dev toolkit (retro-tools, game-frameworks, image-cli,
                  emulators-computers/-consoles, python-gamedev[-extras], worldfoundry [WF],
                  task, btop, firefox)            ← this is what the Phase 2 devbox installs
foundry-desktop   KDE integration (foundry-kde-theme, foundry-welcome)  ← ISO/KDE only, never in the container
foundry-anvil     = foundry-core + foundry-desktop                      ← the default ISO edition
foundry-sprite    = anvil + heavy art/audio (art, pixel-art, trackers, daw, digikam)
foundry-atelier   = sprite + vintage/heavy emulators, game-reimplementations, free-games,
                    android/ios-development, worldfoundry-development [WF], ghidra
                    (ghidra is atelier-only — ~860 MiB of jars; keeps anvil ≤4 GB stick)  ← "complete edition"
```

The devbox installs **`foundry-core`** (one rung below anvil) on purpose — KDE/Plasma has no place in a desktop‑agnostic container. The `foundry-dev` umbrella from the original 2026‑05‑16 proposal no longer exists; it became this `core`/`desktop` split (2026‑05‑28).

### Phase 0: per-metapackage script pattern

Each `foundry-setup/install-<name>.sh` is a thin Phase 0 wrapper that installs from whichever Phase 1 source carries that package. Most scripts are one or two `apt install` calls under the hood.

```
foundry-setup/
  lib.sh                                       shared helpers (colors, logging, run_sudo, apt_update)
  install.sh                                   orchestrator: distro check (26.04 only), role dispatch, summary
  setup-foundry-apt-source.sh                  wire apt.foundrylinux.org (key + sources.list.d)
  setup-worldfoundry-apt-source.sh             wire apt.worldfoundry.org (key + sources.list.d)
  install-foundry-engine-build-deps.sh   build-essential, cmake, libx11-dev, libgl/glu dev, gdb, xxd, python3, pkg-config, git, curl, wget, unzip (Ubuntu universe)
  install-foundry-blender.sh             apt install worldfoundry-blender-addons (apt.worldfoundry.org → Blender + worldfoundry-blender-editor-exporter + blender-asset-finder)
  install-foundry-retro-tools.sh         apt install foundry-retro-tools (apt.foundrylinux.org → mame, cc65, dasm, z80*, radare2, binwalk, sox, m68k binutils, xa65, f9dasm, libvgm, vgmstream; ghidra is atelier-only since 2026-06-04 so anvil fits a 4 GB stick)
  install-foundry-android-development.sh apt install foundry-android-development (apt.foundrylinux.org → JDK 17, adb, NDK r26c; enables Ubuntu multiverse)
  install-foundry-ios-development.sh     apt install foundry-ios-development (apt.foundrylinux.org → libimobiledevice, ideviceinstaller, usbmuxd, ifuse) + pipx install codemagic-cli-tools
  install-task.sh                              go-task (Cloudsmith)
  install-foundry-dev.sh                 apt install worldfoundry-development (apt.worldfoundry.org umbrella — pulls cli + Blender + dev deps) + chains task + retro-tools
```

WF repo cloning, Rust install, and wftools build are **not** in these scripts — they live in `setup-wf-workspace.sh` (tracked in the WorldFoundry repo).

### Phase 1: apt repos

Every package — in either repo — uses the **canonical Debian source-package layout**: the same `debian/{control,changelog,rules,source/format,copyright}` tree that real Debian and Ubuntu archive packages use. `dpkg-buildpackage` consumes it and produces a `.deb`. There is no `DEBIAN/` (uppercase) authored directory — that's a generated artifact inside built `.deb`s, not authored input.

```
foundry-apt/                                 # apt.foundrylinux.org source tree
  packages/<name>/debian/                    Debian source-package format (canonical)
    control                                  Source: + Package: stanzas
    changelog                                authoritative version source
    rules                                    one-line "%: dh $@" (executable)
    source/format                            "3.0 (native)" for metapackages
                                             "3.0 (quilt)" for vendored upstreams
    copyright                                DEP-5 format
    [patches/series]                         optional quilt patches for vendored upstreams
    [watch]                                  optional uscan tracker for vendored upstreams
  packages/<name>/build.sh                   only for vendored upstreams (e.g. f9dasm) —
                                             fetches sha256-pinned tarball, overlays debian/,
                                             runs dpkg-buildpackage
  scripts/build-all.sh                       dispatch: build.sh wrapper, else dpkg-buildpackage
  scripts/init-repo.sh                       aptly repo create (idempotent)
  scripts/publish-local.sh                   aptly publish → ./public/
  scripts/generate-index.sh                  parse packages/*/debian/ → public/index.html
  .github/workflows/publish.yml              tag push → build (in ubuntu:26.04 container) → sign → R2
```

`apt.worldfoundry.org` is structurally identical (Debian source-package layout, aptly + GitHub Actions + Cloudflare R2) but lives in a separate repository.

**CI builds .debs in `ubuntu:26.04` containers, never on the GitHub host runner.** dpkg-shlibdeps pins to the build host's library sonames; building on the GitHub-hosted noble (24.04) runner produces `.deb`s with ffmpeg-6 sonames that won't install on 26.04. The wrap in `docker run -v $PWD:/work ubuntu:26.04 bash -c 'scripts/build-all.sh'` is mandatory.

To package a new upstream as a `.deb`, use the **`/package` skill** (`~/.claude/skills/package/SKILL.md`) — it checks Ubuntu universe first, generates the `debian/` tree via `dh_make`, and patches it with Foundry-customised fields. Don't hand-roll `dpkg-deb --build`.

Hosting: both repos live on Cloudflare R2. GPG signing keys + R2 CI tokens are in each project's GitHub Actions secrets (CI use); all backed up to a private R2 bucket for disaster recovery. No AWS account required. See `foundry-apt/docs/infra-setup.md` for the one-time setup checklist.

## Key Commands

```bash
# foundry-apt
task build                       # build all .debs → dist/
task verify                      # dpkg-deb --info each .deb
task shellcheck                  # lint scripts/*.sh + packages/*/build.sh
task publish-local               # build + aptly publish → ./public/ (smoke test)
task apt-test                    # point apt at ./public/ and resolve foundry-dev

# foundry-setup
bash foundry-setup/install.sh --dry-run
bash foundry-setup/install.sh --role engine-dev --dry-run
bash foundry-setup/install-foundry-retro-tools.sh --dry-run
bash foundry-setup/install-foundry-retro-tools.sh --apt-only   # skip ~/opt/ source-builds

# Docker test harness (requires docker; Ubuntu 26.04 LTS is the only target)
bash foundry-setup/test/run-test.sh                  # dry-run
bash foundry-setup/test/run-test.sh --real           # full install (slow, needs network)
bash foundry-setup/test/test-retro-tools-e2e.sh      # fresh-VM retro-tools install + tool-invocation
```

To release Phase 1: `git tag v1.0.1 && git push origin v1.0.1` — the publish workflow builds, signs, and syncs to R2 automatically.

## Infrastructure automation mandate

**Everything is scripted — no manual console steps.** All site and infrastructure setup (GitHub repos, Cloudflare DNS/R2, GPG keys) must be executed via scripts, CLI tools (`gh`, `curl`/`wrangler`), or Terraform. Never describe a step as "click X in the console" — translate it into the equivalent CLI command or script block. If a one-time action genuinely has no CLI path, document exactly why and what the minimum manual surface is.

**Credentials and keys for automation only.** API tokens, access keys, and role ARNs are for non-interactive script/CI use. The private GPG signing key and R2 CI tokens go into GitHub Actions secrets (for CI use) and are backed up to the private `foundry-secrets` R2 bucket (for disaster recovery) — the local copy is shredded immediately. No credentials live in local files, `.env`, or are typed interactively more than once. No AWS account or SSM is used by this project.

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
