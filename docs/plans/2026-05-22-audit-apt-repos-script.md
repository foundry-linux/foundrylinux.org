# Plan: refresh package-inventory doc + automate via script

> Canonical copy once approved: `~/SRC/foundrylinux.org/docs/plans/2026-05-22-audit-apt-repos-script.md`.

## Context

`foundrylinux.org/docs/investigations/2026-05-19-package-inventory.md` snapshots both Foundry/WF apt repos + the Phase 0 `foundry-setup/` installers. Three days of changes (today's worldfoundry-blender→worldfoundry-blender-addons rename, Blender-pin widenings, foundrylinux.org going from 🚧 planned → live, new foundry-* creative-bundle metapackages, etc.) have moved it materially stale.

User wants three steps:

1. **Refresh** the inventory doc to today's live state.
2. **Automate** via a script so future refreshes are one command.
3. **Compare** script output vs the manual refresh — improve the script for the gaps.

The expected payoff: a `task audit-apt-repos` or `bash scripts/audit-apt-repos.sh > docs/investigations/$(date +%F)-package-inventory.md` that anyone can run to drop a fresh snapshot.

## Critical files

| Path | Change |
|---|---|
| `~/SRC/foundrylinux.org/docs/investigations/2026-05-22-package-inventory.md` | **New** — manual refresh. Sibling to (not replacement for) the 05-19 snapshot; the older doc is the historical baseline. |
| `~/SRC/foundrylinux.org/scripts/audit-apt-repos.sh` | **New** — bash + python3 standard-library, fetches live Packages from both repos + reads local source trees, emits markdown matching the doc's shape. `set -euo pipefail`, `-h/--help`. |
| `~/SRC/foundrylinux.org/scripts/audit-apt-repos.notes.yaml` | **New** *(probably)* — per-package narrative the script can't infer from metadata (the "What it does" / "Why vendored" / "Open issue" prose). Pure-data sidecar; the script merges it into output. |
| `~/SRC/foundrylinux.org/Taskfile.yml` | Add a `task audit-apt-repos` target wrapping the script. |

## What's stale in the 2026-05-19 doc (the diff the refresh produces)

**Repository 1: `apt.worldfoundry.org`**

- `worldfoundry-blender` → renamed to `worldfoundry-blender-addons`; constituent `wf-blender` → `worldfoundry-blender-editor-exporter`.
- Versions: `worldfoundry 1.1.0 → 1.1.4`, `worldfoundry-cli 1.0.0 → 1.0.3`, `worldfoundry-blender-addons 1.0.0 → 1.1.3`, `worldfoundry-development 1.0.1 → 1.0.4`, `wf-blender 0.2.1+git...-1foundry1 → worldfoundry-blender-editor-exporter 0.2.1+git3fa94cbe-2foundry2`.
- All CLIs (cdpack/iffcomp/iffdump/levcomp/lvldump/oaddump/oas2oad/prep/textile) rebuilt to `-1foundry2` for ubuntu:26.04.
- `worldfoundry-blender-addons` Blender pin: `(>= 4.2.0)` → `(>= 4.0.2), (<< 5.1)`. Validated against 4.0.2/4.2.21/4.5.10/5.0.1 via the WF-wbniv blender-addon-tests CI matrix.
- `blender-asset-finder` + `wf-asset` (now `blender-asset-finder-cli`) **moved out** of this repo into apt.foundrylinux.org.
- Binary package count: 12 → 9.

**Repository 2: `apt.foundrylinux.org`**

- Status: 🚧 planned → ✅ **live** (verified end-to-end today via fresh ubuntu:26.04 container — `apt install worldfoundry-blender-addons` resolves `blender-asset-finder` from this repo and installs cleanly).
- Suite: `resolute` (matches Ubuntu 26.04 codename).
- Now ships **~24 packages**: `blender-asset-finder`, `blender-asset-finder-cli`, `f9dasm`, `ghidra`, `libvgm`, `vgmstream`, `foundry-retro-tools`, plus a new family of creative-software metapackages: `foundry-anvil`, `foundry-art`, `foundry-atelier`, `foundry-daw`, `foundry-emulators` (+ `-computers`, `-consoles`, `-consoles-heavy`, `-vintage`), `foundry-free-games`, `foundry-game-frameworks`, `foundry-game-reimplementations`, `foundry-image-cli`, `foundry-pixel-art`, `foundry-sprite`, `foundry-trackers`, `foundry-android-development`, `foundry-ios-development`.

**Phase 0: `foundry-setup/`**

- `install-foundry-blender.sh` now targets `worldfoundry-blender-addons` (was `worldfoundry-blender`).
- New scripts the 05-19 doc didn't have: `setup-foundry-apt-source.sh`, `setup-worldfoundry-apt-source.sh`, `install-foundry-ios-development.sh`, `install-foundry-android-development.sh` (per the CLAUDE.md file tree).

**Cross-repo dep picture**

- `worldfoundry-blender-addons` (apt.worldfoundry.org) now `Depends: blender-asset-finder` (apt.foundrylinux.org). The two repos are no longer entirely disjoint — there's a real cross-repo dep edge. The diagram needs a horizontal arrow between them.

## Steps

### Phase 1 — manual refresh (Step 1)

Write `2026-05-22-package-inventory.md` from scratch, mirroring the 05-19 shape but reflecting today's state. Sections:
1. Header table (both repos now ✅ live).
2. Repo 1 details (suite, key, etc.) + metapackages table + binary packages table + arm64 coverage note.
3. Repo 2 details (suite=`resolute`) + ~24 packages, grouped: vendored upstreams, retro toolchain meta, creative-software metas, dev-bundle metas.
4. Phase 0 installer table (today's set per CLAUDE.md).
5. Cross-repo dep diagram with the new `blender-asset-finder` arrow.
6. Conventions recap (preserved from 05-19; only the date/links change).
7. See-also (updated with today's plan + the matrix-tests plan in WF-wbniv).

`task md -- docs/investigations/2026-05-22-package-inventory.md` to preview before commit.

### Phase 2 — automate via script (Step 2)

`scripts/audit-apt-repos.sh` — bash wrapper around an embedded python3 script. Also wired as `task audit-apt-repos` (the canonical invocation surface — `bash scripts/audit-apt-repos.sh` remains valid for ad-hoc / piping). Inputs:

- **Live apt metadata** (HTTP):
  - `https://apt.worldfoundry.org/dists/stable/main/binary-{amd64,arm64,all}/Packages`
  - `https://apt.foundrylinux.org/dists/resolute/main/binary-{amd64,arm64,all}/Packages`
  - Each repo's `/dists/<suite>/Release` for suite metadata (codename, architectures, signing fingerprint via the `SignedBy:` field if present, otherwise from `/key.gpg`).
- **Local source trees**:
  - `~/SRC/worldfoundry.org/apt/packages/*/debian/control` → cross-check against live.
  - `~/SRC/foundrylinux.org/foundry-apt/packages/*/debian/control` → same.
- **Phase 0 install scripts**: grep `apt install …` / `apt-get install …` lines in `foundry-setup/install-*.sh` to extract each script's install list.
- **Narrative sidecar**: `scripts/audit-apt-repos.notes.yaml` keyed by package name, with fields like `description_long`, `why_vendored`, `open_issues`. Optional — script renders gracefully when entries are missing.

Output: markdown to stdout (or `-o <file>`), matching the doc's shape so it can `>` directly into `docs/investigations/$(date +%F)-package-inventory.md`. Frontmatter date = today.

Behaviour requirements (per `~/SRC/CLAUDE.md`):
- `set -euo pipefail`
- `-h/--help` short-circuits before any fetching
- Errors out clearly if a repo is unreachable (don't silently emit a partial doc).
- No host deps beyond `curl`, `python3` (stdlib), `bash`.

**Task wrapper** (per `~/SRC/CLAUDE.md` §Commands & Tooling — "Use `task <name>` over raw commands"). Add to `foundrylinux.org/Taskfile.yml`:

```yaml
audit-apt-repos:
  desc: "Refresh docs/investigations/<today>-package-inventory.md by polling both apt repos + local source trees"
  cmds:
    - bash scripts/audit-apt-repos.sh -o docs/investigations/{{ now | date "2006-01-02" }}-package-inventory.md
    - echo "Wrote docs/investigations/{{ now | date \"2006-01-02\" }}-package-inventory.md"
```

(go-task's `{{ now | date }}` syntax is fine here; or use a shell expansion via `cmd: bash -c 'bash scripts/audit-apt-repos.sh -o docs/investigations/$(date +%F)-package-inventory.md'`. Whichever the existing Taskfile pattern uses.)

Invocation surface: `task audit-apt-repos` is the canonical route — `bash scripts/audit-apt-repos.sh > …` remains valid for ad-hoc / piping use. Both call the same `.sh`.

### Phase 3 — compare + improve (Step 3)

Run the script:

```
bash scripts/audit-apt-repos.sh > /tmp/inventory-script.md
diff docs/investigations/2026-05-22-package-inventory.md /tmp/inventory-script.md
```

Categorise the diff:

- **Metadata-derivable but missing from v1 script** → fix the script.
- **Narrative I want preserved** → add to `audit-apt-repos.notes.yaml`.
- **Truly hand-curated prose** (conventions recap, see-also, the cross-repo paragraph) → put in template constants the script emits verbatim.

Iterate until `diff` is small + only contains stuff that legitimately can't be auto-derived. End state: the script's output IS the canonical doc going forward; the manual refresh becomes the seed.

## Verification

1. `task md -- docs/investigations/2026-05-22-package-inventory.md` renders cleanly in the browser.
2. `bash scripts/audit-apt-repos.sh -h` prints usage and exits 0.
3. `bash scripts/audit-apt-repos.sh > /tmp/inv-script.md` produces a valid markdown doc, no errors on stderr.
4. End-to-end re-verification of cross-repo install in fresh ubuntu:26.04 still works (we already did this today; mention it in the doc, don't re-run).
5. `diff` between script output and the manual doc fits on one screen.

## Out of scope

- Backfilling the 2026-05-19 doc with auto-script content. The older snapshot has historical value; preserve it untouched.
- Wiring this into CI on a schedule (cron-driven daily inventory snapshots). Possible follow-up, but tonight the script is good as a manual `task audit-apt-repos`.
- Inventorying Phase 2 (Distrobox devbox image) / Phase 3 (Foundry Linux ISO) — neither exists yet.
- A second-level breakdown per creative-software metapackage at apt.foundrylinux.org (what each pulls in). Add later if useful; v1 just lists them.
