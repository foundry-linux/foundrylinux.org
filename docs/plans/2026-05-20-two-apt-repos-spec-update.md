---
title: Update spec for two separate apt repos; collapse Phase 0 to worldfoundry-* metapackages
date: 2026-05-20
status: in-progress
---

# Spec update — two apt repos; Phase 0 collapses to worldfoundry-*

## Context

The architecture in `CLAUDE.md` lists `foundry-apt/` as **the** Phase 1 repo, but in reality Phase 1 is now split across two **permanently separate** apt repos with disjoint scopes:

| Repo | Scope | Source tree |
|---|---|---|
| `apt.foundrylinux.org` | Foundry Linux distro toolchain: retro-porting metapackage + vendored upstreams (`f9dasm`, `ghidra`, `libvgm`, `vgmstream`) | this repo, `foundry-apt/` |
| `apt.worldfoundry.org` | WorldFoundry-specific authoring tools: 12 binaries (cdpack, iffcomp, iffdump, levcomp, lvldump, oaddump, oas2oad, prep, textile, wf-asset, wf-blender, blender-asset-finder) + 4 metapackages (worldfoundry, worldfoundry-cli, worldfoundry-blender, worldfoundry-development) | `../worldfoundry.org/apt/` |

User direction (2026-05-20):

1. **Permanently separate** — no consolidation planned.
2. **Collapse blender + dev umbrella to worldfoundry-***: `install-foundry-blender.sh` → `apt install worldfoundry-blender`; `install-foundry-dev.sh` → `apt install worldfoundry-development`.
3. **Keep architecture inline in CLAUDE.md** — don't redirect to the inventory.

## Approach

### 1. CLAUDE.md architecture section rewrite

- Replace the single "Phase 1 = foundry-apt/" framing with a two-repo table (scope, source tree, target).
- Update the Phase 0 script table to point each script at its Phase 1 target (universe / apt.foundrylinux.org / apt.worldfoundry.org).
- Add a one-line note that the two repos are deliberately separate.

### 2. New helper script: `setup-worldfoundry-apt-source.sh`

Mirror of `setup-foundry-apt-source.sh`, but for `apt.worldfoundry.org`. Adds the GPG key + sources.list.d entry idempotently. Called by the install scripts below.

### 3. `install-foundry-blender.sh` collapse

Currently: `apt install blender python3` (Ubuntu universe).

After: setup apt.worldfoundry.org source → `apt install worldfoundry-blender`. That transitively brings Blender + the WF Blender add-on (`wf-blender`) + the asset-finder add-on (`blender-asset-finder`) — strictly more than the bare `blender` install.

### 4. `install-foundry-dev.sh` collapse

Currently: composes engine-build-deps + task + blender + retro-tools by calling sub-installers in order.

After: setup apt.worldfoundry.org source → `apt install worldfoundry-development` (umbrella that pulls in worldfoundry → cli + blender + Blender, plus build-essential, cmake, X11/GL dev, python3). Then chain `install-task.sh` and `install-foundry-retro-tools.sh` because task (Cloudsmith) and retro-tools (apt.foundrylinux.org) aren't part of worldfoundry-development.

`install.sh`'s role dispatch logic stays the same — `both` / `maintainer` still call `install-foundry-dev.sh`; engine-dev / game-dev still compose individual sub-installers (which now route correctly to whichever apt source they need).

## Verification

1. `bash foundry-setup/install.sh --dry-run --role both` prints the new plan with both apt sources getting configured.
2. `bash foundry-setup/install-foundry-blender.sh --dry-run` shows `apt install worldfoundry-blender`, not the old `blender python3`.
3. After publishing: in a clean ubuntu:26.04 container, `bash install-foundry-blender.sh` resolves and installs Blender + wf-blender + blender-asset-finder via the worldfoundry metapackage.
4. CLAUDE.md `grep -c 'apt.worldfoundry.org'` returns ≥1.
