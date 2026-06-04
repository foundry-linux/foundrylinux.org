# Execute Phase 2 — Foundry Linux devbox image

## Context

Phase 2 of the foundrylinux.org roadmap: ship `ghcr.io/foundry-linux/devbox:26.04`, a Distrobox-compatible OCI image with the full WF authoring + retro-porting stack preinstalled. Design plan at `docs/plans/2026-05-21-phase-2-devbox-image.md` (647 lines) is the contract — this file is the execution outline.

**Why now:** Phases 0 and 1 are done; this is the next planned milestone. The plan is complete; nothing about it needs re-debating.

**Current execution state** (verified by exploration):

| Prerequisite | State |
|---|---|
| foundry-apt 16 new metapackages (nested editions) | 0/16 exist (need to author all) |
| `foundry-devbox/` monorepo subdir | doesn't exist (need to create) |
| `github.com/foundry-linux/foundry-devbox` GH repo | doesn't exist (need to bootstrap) |
| Top-level Taskfile sync uses `git status --porcelain` fix | ✅ (commit `3e85f90`) |
| foundry-apt publish.yml template | ✅ (reusable for GHCR workflow shape) |
| `worldfoundry-*` metapackages on apt.worldfoundry.org | ✅ (live, design plan §638 confirms) |
| Phase 0 install-foundry-dev.sh apt sequence | ✅ (at `foundry-setup/install-foundry-dev.sh:85-103`) |

## Execution order

The 16 new metapackages are prerequisites for the Dockerfile (which `apt install foundry-anvil`s). So the image can't build until they're published to apt.foundrylinux.org. Order matters.

### Step 1 — Author 16 metapackages in `foundry-apt/packages/` (3 tiers, nested editions)

Per design plan §"Implementation cost". Each is a tiny native source-package tree using the existing `foundry-retro-tools` template (`debian/{control,changelog,rules,source/format,copyright}`).

**Tier 0 — composable sub-metapackages (6):**
- `foundry-emulators-computers`, `foundry-emulators-consoles`, `foundry-game-frameworks`, `foundry-trackers`, `foundry-pixel-art`, `foundry-image-cli`

(`task` is installed directly from Cloudsmith — no wrapper metapackage.)

**Tier 1 — opt-in extensions (7):**
- `foundry-art`, `foundry-daw`, `foundry-emulators-vintage`, `foundry-emulators-consoles-heavy`, `foundry-emulators` (umbrella), `foundry-game-reimplementations`, `foundry-free-games`

**Tier 2 — nested editions (3):** (names match the site's Editions section + future ISO image names; each a strict superset of the previous)
- `foundry-anvil` — **base layer**; Depends: `worldfoundry` (cross-repo) + `foundry-retro-tools` + `foundry-emulators-computers` + `foundry-emulators-consoles` + `foundry-game-frameworks` + `foundry-image-cli` + `task` (Cloudsmith) + distrobox conveniences. **Devbox image installs this and nothing else.**
- `foundry-sprite` — anvil + heavy graphics + audio; Depends: `foundry-anvil` + `foundry-pixel-art` + `foundry-art` + `foundry-trackers` + `foundry-daw`
- `foundry-atelier` — sprite + everything else; Depends: `foundry-sprite` + `foundry-emulators-vintage` + `foundry-emulators-consoles-heavy` + `foundry-game-reimplementations` + `foundry-free-games` + `foundry-android-development` + `foundry-ios-development` + `worldfoundry-development` (cross-repo)

Full Depends for each in design plan §"Implementation cost".

Add each to `foundry-apt/scripts/build-all.sh` if it doesn't auto-discover (verify first).

### Step 2 — Minimal bootstrap: orchestrator dispatch + 2 lib.sh helpers

User-facing bootstrap stays a single command (`bash install.sh --role <edition>`). All source-wiring lives inside the orchestrator — no per-edition `.sh` scripts to multiply. Opt-in metapackages installed later are a power-user concern, not bootstrap.

- Extend `foundry-setup/lib.sh` with two idempotent helpers:
  - `enable_multiverse()` — wraps `add-apt-repository -y multiverse`, no-ops if already enabled (mirror the inline form at `install-foundry-android-development.sh:58-59`)
  - `setup_worldfoundry_source()` — sources `setup-worldfoundry-apt-source.sh` once, no-op if `/etc/apt/sources.list.d/worldfoundry.list` already exists
- Update `install.sh` orchestrator: extend `--role` dispatch with 4 new cases. Each one wires the sources its metapackage needs, then `apt install foundry-<edition>`:
  - `anvil` → `setup_worldfoundry_source()` + `apt install foundry-anvil`
  - `sprite` → `apt install foundry-sprite` (apt.foundrylinux.org only, no wiring)
  - `atelier` → `setup_worldfoundry_source()` + `apt install foundry-atelier`
  - `atelier` → `setup_worldfoundry_source()` + `enable_multiverse()` + `apt install foundry-atelier`

Test in the existing Docker harness (`foundry-linux-setup/test/run-test.sh` and `test-retro-tools-e2e.sh`) — one role at a time.

### Step 3 — Tag foundry-apt release; CI publishes all 16

```bash
cd foundry-apt
task bump          # bump patch version
git push origin <tag>
gh run watch       # confirm publish.yml goes green
```

Verify the resulting `Packages` index lists all 16 new metapackages. Smoke-install `foundry-anvil` in a fresh `ubuntu:26.04` container (with both apt sources configured) to confirm cross-repo Depends resolve cleanly.

### Step 4 — Front-page Quick Start: author `setup.sh` + update site copy

The front page currently shows a 3-step setup ending in `sudo apt install foundry-base`. After this step the front page becomes a **2-step Quick Start** that stays at 2 steps even as source-wiring grows.

**Author `setup.sh`** — single curl-bash payload that idempotently wires all sources. Lives at `https://foundrylinux.org/setup.sh` (served from `site/public/setup.sh` or equivalent static path; the site is on Cloudflare Pages):

```bash
#!/usr/bin/env bash
set -euo pipefail
# Foundry Linux archive setup — idempotent. https://foundrylinux.org

sudo install -d /etc/apt/keyrings

# apt.foundrylinux.org
curl -fsSL https://apt.foundrylinux.org/key.gpg | sudo gpg --dearmor -o /etc/apt/keyrings/foundry.gpg
echo "deb [signed-by=/etc/apt/keyrings/foundry.gpg] https://apt.foundrylinux.org stable main" \
  | sudo tee /etc/apt/sources.list.d/foundry.list > /dev/null

# apt.worldfoundry.org (for foundry-anvil / -atelier / -everything)
curl -fsSL https://apt.worldfoundry.org/key.gpg | sudo gpg --dearmor -o /etc/apt/keyrings/worldfoundry.gpg
echo "deb [signed-by=/etc/apt/keyrings/worldfoundry.gpg] https://apt.worldfoundry.org stable main" \
  | sudo tee /etc/apt/sources.list.d/worldfoundry.list > /dev/null

# Cloudsmith (for go-task)
curl -1sLf 'https://dl.cloudsmith.io/public/task/task/setup.deb.sh' | sudo bash

# Ubuntu multiverse (for foundry-emulators-vintage / foundry-game-reimplementations / android NDK)
sudo add-apt-repository -y multiverse
sudo apt update

echo "Done. Install an edition: sudo apt install foundry-anvil"
```

**Update site copy** — `site/sections.jsx`, `site/index.html`, `scripts/ssr-render.js` (or wherever the install snippet lives after the SSR render):

- Hero: `sudo apt install foundry-anvil` (replaces `foundry-base`)
- Install section: collapse 3 steps → 2:
  ```
  ① Add the Foundry archive
  curl -fsSL https://foundrylinux.org/setup.sh | bash
  
  ② Install
  sudo apt update && sudo apt install foundry-anvil
  ```
- Optional: an expandable "What does `setup.sh` do?" panel that shows the script body for audit-minded users.

Deploy via the existing `task site-build` + `task site-deploy` (Cloudflare Pages tag-triggered).

### Step 5 — Create `foundry-devbox/` subdir

Layout per design plan §"Layout":

```
foundry-devbox/
  Dockerfile                         # ubuntu:26.04 → 2-layer install per plan §"Dockerfile"
  Taskfile.yml                       # build / run / smoke (plan §"Local Taskfile")
  README.md
  .github/workflows/publish.yml      # GHCR tag-driven publish (plan §"GHCR publish workflow")
  test/smoke-test.sh                 # ~35-tool PATH check
```

The Dockerfile body is fully written out in the design plan (copy verbatim with placeholders filled). Workflow YAML same.

### Step 6 — Add top-level Taskfile.yml entries

Mirror existing `sync` / `release` pattern (Taskfile.yml:19-50, already using `git status --porcelain` per commit `3e85f90`):

- `task devbox-sync` — archive `foundry-devbox/` from HEAD, overlay on fresh clone of `foundry-linux/foundry-devbox`, commit + push if dirty.
- `task devbox-release TAG=v0.x.y` — tag the remote and trigger CI.

### Step 7 — Bootstrap GitHub remote

```bash
gh repo create foundry-linux/foundry-devbox --public \
  --description "Distrobox-compatible WF authoring + retro-porting image for Ubuntu 26.04"
# No initial commit — task devbox-sync seeds it.
```

GHCR push uses the workflow's `GITHUB_TOKEN` (already in scope per `permissions: { packages: write }` in the workflow). No extra secret needed.

### Step 8 — First sync + release

```bash
task devbox-sync
task devbox-release TAG=v0.0.1
gh run watch --repo foundry-linux/foundry-devbox
```

### Step 9 — Run all 7 verification steps from the design plan

Paste raw output below each numbered step in `docs/plans/2026-05-21-phase-2-devbox-image.md`, add PASS/FAIL. Then promote the TODO.md item to `[x]` done.

## Critical files

**Read / reuse:**
- `docs/plans/2026-05-21-phase-2-devbox-image.md` — design contract; full Dockerfile, workflow, Taskfile content
- `Taskfile.yml:19-50` — `sync` / `release` template to mirror for `devbox-sync` / `devbox-release`
- `foundry-apt/packages/foundry-retro-tools/debian/` — metapackage source-tree template
- `foundry-apt/.github/workflows/publish.yml` — tag-trigger + dry_run pattern (shape, not contents)
- `foundry-setup/install-foundry-dev.sh:85-103` — apt-install sequence to mirror in Dockerfile

**Create:**
- `foundry-apt/packages/foundry-{emulators-computers,emulators-consoles,game-frameworks,trackers,pixel-art,image-cli}/debian/{control,changelog,rules,source/format,copyright}` — 6 Tier 0 sub-metapackages
- `foundry-apt/packages/foundry-{art,daw,emulators-vintage,emulators-consoles-heavy,emulators,game-reimplementations,free-games}/debian/...` — 7 Tier 1 opt-in extensions
- `foundry-apt/packages/foundry-{anvil,sprite,atelier}/debian/...` — 3 Tier 2 nested edition umbrellas
- `foundry-devbox/Dockerfile` + `Taskfile.yml` + `README.md` + `.github/workflows/publish.yml` + `test/smoke-test.sh`

**Edit:**
- `Taskfile.yml` (root) — add `devbox-sync` + `devbox-release` tasks
- `TODO.md` — mark Phase 2 item `[x]` after verification passes

## Open questions — resolved

1. **Metapackage scope** → all 16 in one batch (confirmed by user).
2. **GH org access** → confirmed: `wbniv` is a member of `foundry-linux`; token has `repo` + `read:org` scopes, sufficient for `gh repo create foundry-linux/foundry-devbox`.
3. **Naming hierarchy** → 3-tier nested (Tier 0 sub-metapackages, Tier 1 opt-in, Tier 2 nested editions `foundry-{anvil,sprite,atelier}` each a strict superset of the previous). No separate `foundry-everything` umbrella — `foundry-atelier` IS everything. No `foundry-task` wrapper — `task` installs directly from Cloudsmith. `foundry-devbox-base` retired in favor of `foundry-anvil`. Editions cross-Depend on `apt.worldfoundry.org` packages; bootstrap `setup.sh` wires all sources.

## Verification

Use the 7-step verification block from the design plan (`docs/plans/2026-05-21-phase-2-devbox-image.md` §Verification). Steps 1–2 are local (build + smoke); 3–4 are sync + tag-driven CI; 5–6 are pull + distrobox + universe-additions smoke; 7 is the full metapackage audit + mermaid graph render. Append raw output + PASS/FAIL to the design plan file.
