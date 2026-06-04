---
title: State of the Foundry Linux Distro
date: 2026-05-29
status: assessment
author: audit (7 parallel investigations + direct verification)
scope: whole monorepo — structure, apt repos, website, ISO, CI/tooling, docs
---

# State of the Foundry Linux Distro — 2026‑05‑29

**Method.** Seven parallel read‑only audits (one per subsystem) plus direct verification of every load‑bearing claim against the working tree and the live R2 endpoints. Where a finding is relayed but not independently re‑checked, it is flagged in [§13 Confidence & provenance](#13-confidence--provenance).

---

## 1. Executive summary

Foundry Linux is **materially complete through Phase 2 and within striking distance on Phase 3.** The two signed apt repos are live, current, and healthy on R2; the Distrobox image publishes to GHCR; the website is a clean SSR‑static pipeline with a live‑data packages page. Phase 3 (the ISO) has had its hard *technical* blockers (autologin, Calamares, first‑login welcome) **closed and boot‑verified on 2026‑05‑26** — the TODO that still calls them "pending" is stale. What stands between today and a 1.0 ISO is now *operational*, not technical: image size, a never‑built atelier edition, and an unexercised publish loop.

The distribution is in good shape. The risks that matter are **not** in the shipped artifacts — they are in the **scaffolding around them**: a handful of CI gates that are red and being masked by greener sibling jobs, a documented cross‑repo automation that exists nowhere, editor‑history clutter committed to git, and a north‑star proposal that no longer describes the product that shipped.

> ## 🔄 Status refresh — 2026‑06‑04 (6 days on)
>
> Most of the scaffolding risk is now closed; the picture has moved materially. Finding‑by‑finding:
>
> | # | 05‑29 finding | 06‑04 status |
> |---|---|---|
> | 1 | Phase 0 real‑install test broken | ✅ **Fixed** (`5f3cda5`) — `--skip-clone` restored (`install.sh:70`) |
> | 2 | `foundry-apt` PR test red | ✅ **Fixed** (`5f3cda5`) — findings cleared at source; test.yml green |
> | 3 | Cross‑repo dispatch missing | ✅ **Wired** (`9d5dd78`) — `site-deploy.yml` listens for `apt-published`; ⚠️ the `FOUNDRYLINUX_DISPATCH_PAT` secret is still a pending manual step for Will (TODO L33) |
> | 4 | ISO is **15 GB** | 🟢 **Stale by a mile** — anvil rebuilt post‑split to **4.8 GB** (0.9.36); the package‑list reckoning that was "the real Phase 3 blocker" is **done** — full exclusive‑closure sizing analysis + scenario charts landed 2026‑06‑04 (`docs/investigations/2026-06-04-usb-iso-sizing/` + `docs/plans/2026-06-04-usb-sized-iso-editions.md`), with a **SLIM** ~3.1 GiB edition proposed for the 4 GB‑stick target |
> | 5 | No ISO published; atelier never built | 🟡 **Still open** — ISO builds clean locally at 0.9.36 but CI is still `workflow_dispatch`‑only; no atelier build, publish loop still unproven |
> | 6 | North‑star proposal stale | ✅ **Bannered (06‑04)** — moved to `docs/investigations/`, `status:` now reads SUPERSEDED, and a top banner enumerates the divergences (foundry-dev→editions, dropped Steam/kiosk/AWS-SSM, creative-distro scope expansion) |
> | 7 | CLAUDE.md "neither depends" false | ✅ **Reworded (06‑04)** — `CLAUDE.md` now states pipeline‑independent / package‑coupled, "co‑installed, independently published"; also gained an Editions section |
> | 8 | `.history/` clutter committed | 🟢 **Mostly fixed** (`7f5e1ba`) — `.history/` gitignored (`.gitignore:17`), 33/34 snapshots untracked; **1 straggler** `.history/CLAUDE.md` still tracked |
> | 9 | Generated site artifacts committed | 🟡 **Partial** — `serve.json` now tracked (`e39feb0`, closes the deploy‑critical half); but `index.html`/`packages.html`/`packages-data.json` are still tracked, not gitignored |
> | 10 | Node‑24 action‑pin stragglers | ✅ **Fixed (06‑04)** — all 6 stragglers bumped (`site-deploy.yml` upload‑artifact `@v7`; `foundry-iso/publish.yml` upload/download‑artifact `@v7`; `foundry-setup/test.yml` checkout `@v6` ×3). Whole‑tree census is now `checkout@v6` / `setup-node@v6` / `upload`+`download-artifact@v7`, no `<v6` left |
>
> **Package census has also grown:** `apt.foundrylinux.org` is now **40 source packages** (was 32) — **14 vendored upstreams** (was 6+2; +5 Python/ruff on 05‑30, `task` re‑vendored 05‑31), README + `LICENSES-VENDORED.md` rewritten to match (06‑04). Docs: 66 plans · 19 investigations · 14 transcripts.
>
> **What's left from the original action list:** the `serve.json`‑adjacent gitignore of 3 generated artifacts, the `.history/CLAUDE.md` straggler, the `prep`/arm64 TODO (action #10), shellcheck CI breadth (action #14), the investigation/plan consolidation (action #15), and Phase‑3‑to‑1.0 (finding 5 + SLIM go‑ahead). *(Closed in the 06‑04 pass: findings 1‑3, 6, 7, 10; README + CLAUDE.md + LICENSES + proposal docs; action #9 in full.)* **New gap found 06‑04:** `foundry-linux/foundry-setup` child repo **does not exist** and there's no `setup-sync` task — so `foundry-setup/.github/workflows/test.yml` (which the §8 census lists as live CI) actually **runs nowhere**; it only executes once that subtree is mirrored to a repo root. The inline annotations on findings 4–10 below carry the detail.

### Phase scorecard

| Phase | Component | Status | Evidence |
|---|---|---|---|
| **0** | `foundry-setup/` bash installer | ✅ **Live**, conventions strong | All 9 documented scripts present & correct; one CI test path broken (§4) |
| **1a** | `apt.foundrylinux.org` (`foundry-apt/`) | ✅ **Live & current** | 32 packages on R2 match source changelogs exactly; metadata regenerated 2026‑05‑29 |
| **1b** | `apt.worldfoundry.org` (sibling repo) | ✅ **Live & signed** | 14 packages = 9 CLIs + 1 add‑on + 4 metas; last tag `apt‑v0.1.37` |
| **2** | `ghcr.io/foundry-linux/devbox:26.04` | ✅ **Complete** (v0.0.5) | Single‑layer `foundry-core` install + GHCR publish + smoke test |
| **3** | `foundry-iso/` (anvil + atelier ISOs) | 🟡 **In progress** | Tech blockers closed & boot‑verified; **anvil now 4.8 GB @ 0.9.36** (06‑04) + sizing analysis done; atelier/publish/CI still open (§7). *Scorecard ref to "§7" is the original; ISO detail is §6.* |
| — | `foundrylinux.org` website | ✅ **Live** | Cloudflare Pages, SSR static, live packages page; minor defects (§8) |

### The ten findings that matter most

1. ~~**🔴 The Phase 0 real‑install test is broken and silently masked.**~~ **✅ Fixed (`5f3cda5`)** — `--skip-clone` restored as a real flag gating the maintainer-role clone; plus `shellcheck -x` (the SC1091 red it surfaced) and an always-on dry-run regression guard; verified in `ubuntu:26.04`. `install.sh` rejects `--skip-clone` (`install.sh:72`), but both the local `run-test.sh --real` (`:58`) and the CI full‑install job (`test.yml:50`) still pass it — they die on "Unknown option" before doing any work. The dry‑run job has no such flag and stays green, hiding the breakage. Fallout from the editions refactor (the flag was dropped from `parse_args`, the tests never updated).
2. ~~**🔴 `foundry-apt`'s PR test workflow is red.**~~ **✅ Fixed (`5f3cda5`)** — findings cleared at source (`_apt`→`if/then/else`, justified SC2012/SC2064 disables) and `zip python3` added; synced to the child repo where CI run `26675001586` is now **green** (shellcheck ✓, full build ✓). `test.yml:22‑23` runs `shellcheck` with no `-S error` filter, so existing info/warning findings (SC2012/SC2064/SC2015) fail the step; and its build container omits `zip`+`python3`, so the two `blender-asset-finder*` packages can't build there (the tag‑triggered `publish.yml` *does* install them and ships fine — so **shipping works, the branch gate doesn't**).
3. ~~**🔴 A documented cross‑repo automation does not exist.**~~ **✅ Fixed** — receiver `9d5dd78` (`site-deploy.yml` now listens for `apt-published`) + sender WF `540d5c3` (`notify-foundrylinux`, graceful-skip until the secret) + `FOUNDRYLINUX_DISPATCH_PAT` set; fires on the next `apt-v*` tag. `FOUNDRYLINUX_DISPATCH_PAT` / the `repository_dispatch` that should rebuild `/packages` when `apt.worldfoundry.org` publishes was **added then reverted** in the WF repo and is wired on **neither** end. Today the only path is the nightly cron — up to ~24 h stale. TODO already flags it "pending"; the report confirms it is genuinely unimplemented.
4. ~~**🟡 The ISO is 15 GB, not the ~3.5 GB target**~~ **✅ Largely resolved (06‑04).** The 15 GB figure was the pre‑split `0.9.30` artifact; anvil was rebuilt after the 1.0.4 edition split and is now **4.8 GB (~4.47 GiB) at `0.9.36`**. The "package‑list reckoning is the real Phase 3 blocker" call was right — and that reckoning is now **done**: a full exclusive‑dependency‑closure sizing analysis (`docs/investigations/2026-06-04-usb-iso-sizing/`) + plan (`docs/plans/2026-06-04-usb-sized-iso-editions.md`) quantify every metapackage's weight, establish the ~2.44 GiB bare‑KDE floor, and propose a **SLIM ~3.1 GiB** edition for the 4 GB‑stick target. Two new size TODOs fell out of it (snapd‑survives‑strip ~140 MiB; `foundry-python-gamedev-extras` 554 MiB). *Original finding text preserved below for provenance.* — `foundry-anvil` pulls `foundry-core` + `foundry-desktop` + `kubuntu-desktop`; the anvil/atelier size distinction the README and plan promise is currently fictional.
5. **🟡 No ISO has been published anywhere, and atelier has never been built.** *(Still open 06‑04.)* anvil builds clean locally at `0.9.36`, but CI is still `workflow_dispatch`‑only and no atelier ISO has been produced; the build→sign→R2→download→install loop is unproven end‑to‑end. The Internet Archive migration (`upload-iso-ia.sh`) is correctly *deferred* (gated on v1.0.0), not dropped.
6. ~~**🟡 The north‑star proposal is now a historical artifact.**~~ **🟡 Reclassified (06‑04), not yet bannered.** The proposal was moved `docs/plans/` → `docs/investigations/2026-05-16-foundry-linux-distro-proposal.md` — the right filing (it *is* history), but its `status:` frontmatter still reads "Phase 1 (APT repo) next," so a reader still meets a stale status line. The substantive reconciliation (document the creative‑distro scope expansion; retire `foundry-dev`/Steam/kiosk/AWS‑SSM language) is still owed. `foundry-dev` — its centerpiece — no longer exists; it became `foundry-core` + `foundry-desktop`. ~20 art/audio/emulator/games metapackages appeared with no proposal basis.
7. **🟡 CLAUDE.md's "neither repo depends on the other" is false at the package level.** *(Still open 06‑04 — `CLAUDE.md:16` unchanged.)* `foundry-core` `Depends: worldfoundry`; `worldfoundry-cli`/`-blender-addons` `Depend` on `blender-asset-finder*` (which live in foundry‑apt). The repos are *mutually* package‑coupled — true independence holds only for apt‑source configuration. Phase 0's two `setup-*-apt-source.sh` scripts are what actually guarantee both sources are wired.
8. ~~**🟡 Editor `.history/` clutter is committed.**~~ **🟢 Mostly fixed (`7f5e1ba`).** `.history/` is now gitignored (`.gitignore:17`) and 33 of 34 snapshots were `git rm --cached`'d. **One straggler remains tracked: `.history/CLAUDE.md`** (dirty in the working tree) — `git rm --cached` it to fully close this.
9. ~~**🟡 Generated site artifacts are committed**~~ **🟡 Half‑fixed.** The deploy‑critical gap is closed: `site/serve.json` (the `/packages` clean‑URL rewrite) is now **tracked** (`e39feb0`). But the three *generated* artifacts — `site/index.html`, `site/packages.html`, `site/packages-data.json` — are still tracked and still invite merge churn against the "index.html is generated — never edit" stance; gitignore them.
10. ~~**🔴 Node‑24 action‑pin stragglers — now OVERDUE.**~~ **✅ Fixed (06‑04).** All 6 stragglers bumped past the 2026‑06‑02 forced‑off date: `site-deploy.yml:92` (`upload-artifact@v7`), `foundry-iso/publish.yml:68,81` (`upload/download-artifact@v7`), `foundry-setup/test.yml:17,27,52` (`checkout@v6` ×3). Whole‑tree census is now clean (`checkout@v6`, `setup-node@v6`, `upload`/`download-artifact@v7`); all three YAMLs re‑validated.

---

## 2. Structure & contents

### The monorepo + mirror‑out model

`foundry-linux/foundrylinux.org` is a **private monorepo authoring home**. Each shippable component is **git‑archive‑mirrored out** to its own standalone public repo so that (a) per‑component GitHub Actions run scoped to that subtree and (b) each child's tag‑driven `publish.yml` can fire on tags in *that* repo:

```
foundrylinux.org  (private monorepo, authoring)
├── foundry-setup/   ──(task sync)──▶  foundry-linux/foundry-setup
├── foundry-apt/     ──(task sync)──▶  foundry-linux/foundry-apt     ── tag v*  ─▶ R2 apt.foundrylinux.org
├── foundry-devbox/  ──(devbox-sync)─▶ foundry-linux/foundry-devbox  ── tag v*  ─▶ GHCR devbox:26.04
├── foundry-iso/     ──(iso-sync)────▶ foundry-linux/foundry-iso     ── (CI off) ─▶ R2 iso.foundrylinux.org
├── site/ + scripts/ ────────────────────────────────────────────── ── tag v*  ─▶ CF Pages foundrylinux.org
└── docs/  (plans · investigations · transcripts — never mirrored)

apt.worldfoundry.org  ←  ../worldfoundry.org/apt/  (SEPARATE repo, org `wbniv`, tag apt-v*)
```

`task sync` is **load‑bearing, not vestigial**: it archives `HEAD -- <subdir>/`, wipes the clone (preserving `.git`), and pushes — so renames/deletions propagate (added after the `foundry-linux-*`→`foundry-*` rename left orphans, `Taskfile.yml:63‑68`). The child repos' CI assumes files at *their* root (`scripts/build-all.sh`, `./apt/…`), so the mirror is the bridge from monorepo authoring to per‑component CI.

### Component map

| Path | Phase | What it is | State |
|---|---|---|---|
| `foundry-setup/` | 0 | Thin bash wrappers: wire apt sources, `apt install` metapackages | Live; conventions strong; one test path broken |
| `foundry-apt/` | 1a | `apt.foundrylinux.org` source tree (32 pkgs, aptly, R2 CI, index site) | Live & current |
| `../worldfoundry.org/apt/` | 1b | `apt.worldfoundry.org` (14 pkgs; needs a Rust/maturin builder image) | Live & signed |
| `foundry-devbox/` | 2 | Dockerfile installing `foundry-core` → GHCR | Complete (v0.0.5) |
| `foundry-iso/` | 3 | live‑build Kubuntu 26.04 ISO (anvil + atelier) | In progress |
| `site/` + `scripts/` | — | SSR‑static React site + live packages page → CF Pages | Live |
| `docs/` | — | 50 plans · 16 investigations · 11 transcripts | Rich; some drift |
| `.foundry/` | — | gitignored bootstrap.env credential cache | n/a |
| `data/` | — | hand‑curated `categories.json` + `upstream.yml` feeding `/packages` | Current |

### Package census (46 source packages)

**`apt.foundrylinux.org` — 32 packages**, all canonical `debian/` layout (no authored uppercase `DEBIAN/`), live R2 versions matching source changelogs exactly:

- **~24 metapackages** in a tiered edition hierarchy (see graph below).
- **6 vendored upstreams** (sha256‑ or git‑commit‑pinned tarball/clone + `build.sh`): `f9dasm`, `ghidra`, `libvgm`, `vgmstream`, `ppsspp`, `snes9x-gtk`.
- **1 native‑from‑source**: `foundry-welcome` (QML/Kirigami first‑login app).
- **2 git‑vendored Blender tools**: `blender-asset-finder`, `blender-asset-finder-cli`.

> **Update 2026‑06‑04 — the vendored count has since grown to 14.** This 05‑29 snapshot (6 upstreams + 2 Blender tools, `task` retired) was correct *as of audit date*. Added since: `ruff` + `python3-glfw`/`-librosa`/`-mss`/`-pydub` (05‑30, +5) and `task` **re‑vendored** (`23809cc`, 05‑31, +1) so `foundry-apt` is self‑contained for `foundry-core`'s `Depends: task`. The full 14 are now enumerated in `foundry-apt/README.md` and `LICENSES-VENDORED.md` (both rewritten 06‑04). ⚠️ Note `snes9x-gtk` ships under the **Snes9x non‑commercial licence** — fine for this free repo, never for a paid bundle.

**`apt.worldfoundry.org` — 14 packages**: 9 Rust/C CLIs (`cdpack`, `iffcomp`, `iffdump`, `levcomp`, `lvldump`, `oaddump`, `oas2oad`, `prep`, `textile`) + 1 Blender add‑on (`worldfoundry-blender-editor-exporter`) + 4 umbrella metas (`worldfoundry`, `worldfoundry-cli`, `worldfoundry-blender-addons`, `worldfoundry-development`).

#### Metapackage dependency graph (verified from `debian/control`)

```
foundry-atelier (0.9.1)  →  foundry-sprite, emulators-vintage, emulators-consoles-heavy,
                            game-reimplementations, free-games, android-development,
                            ios-development, worldfoundry-development [WF repo]
foundry-sprite  (1.0.0)  →  foundry-anvil, pixel-art, art, trackers, daw
foundry-anvil   (1.0.4)  →  foundry-core, foundry-desktop            (thin umbrella since 05-28 split)
foundry-core    (1.0.1)  →  emulators-computers, emulators-consoles, game-frameworks,
                            image-cli, retro-tools, worldfoundry [WF repo], btop, firefox, task
foundry-desktop (1.0.0)  →  foundry-kde-theme, foundry-welcome       (KDE-only; kept OUT of devbox)
```

The **devbox installs `foundry-core`** (one rung below anvil) precisely so KDE/Plasma never enters a desktop‑agnostic container — the real nesting is `foundry-core ⊆ foundry-anvil ⊆ foundry-sprite ⊆ foundry-atelier`.

> **Note — `foundry-atelier` at `0.9.1`** is an intentional pre‑1.0 marker (downgraded from a prior 1.0.0), signalling the "complete edition" is still settling. Not a bug; R2 correctly serves 0.9.1.

---

## 3. Phase 0 — the bash installer (`foundry-setup/`)

**Verdict: live and well‑built; convention adherence is strong; one CI test path is broken.**

All nine scripts named in CLAUDE.md exist and target exactly what's documented. Across all ten scripts: `set -euo pipefail` present; `-h/--help` short‑circuits *before* any apt/sudo/validation; each standalone script carries a `lib.sh` fallback shim; `apt_update()` (which demotes stale‑repo `apt-get update` failures to warnings) is used over raw `apt-get update`. The orchestrator's 26.04‑only gate, sudo keepalive, and 8‑role dispatch (`anvil`/`sprite`/`atelier`/`game-dev`/`engine-dev`/`both`/`maintainer`/`play`) all check out — every declared role has a matching dispatch arm.

**🔴 Broken: the real‑install test path.** `install.sh:72` dies on any unknown flag; `parse_args` knows `--skip-blender`/`--skip-retro`/`--apt-only` but **not `--skip-clone`** — yet `test/run-test.sh:58` and `.github/workflows/test.yml:50` both pass `--skip-clone --skip-blender`. So `run-test.sh --real` and the CI full‑install job (main/tags) abort immediately; the dry‑run job (no flag) stays green and masks it. Cloning moved into the `maintainer` role during the editions refactor and the flag was dropped, but the tests and `foundry-setup/README.md:49` were never updated. **Fix:** re‑add a no‑op `--skip-clone)` case (it's effectively a no‑op for `engine-dev` now), or drop the flag from the two test invocations.

**🟡 Other gaps:** `foundry-setup/README.md` is badly stale — it documents a pre‑distro Rust/maturin/`wftools`/cargo workflow that exists in no current script, advertises three non‑existent flags, and links to old monorepo paths. CI uses `actions/checkout@v4` (×3, Node‑20) and has a dead `paths:` filter referencing a non‑existent `foundry-setup-test.yml`. The android script calls `add-apt-repository -y multiverse` directly instead of the idempotent `enable_multiverse()` helper. CI shellcheck lints only 2 of ~10 scripts.

---

## 4. Phase 1 — the two apt repos

### 4a. `apt.foundrylinux.org` (`foundry-apt/`) — live, current, healthy

The live R2 repo serves all 32 packages at exactly their source‑changelog versions; `packages.json`/`feed.xml`/`meta/*.json` were regenerated **2026‑05‑29T01:04Z**. GPG‑signed (`Release.gpg` + `InRelease`). The tag‑driven `publish.yml` is exemplary: builds every `.deb` inside `ubuntu:26.04`, signs, two‑pass `rclone` to R2 (data first, `Release` last, `Cache-Control:no-store`), polls for consistency, then re‑installs from the live repo in a clean container. Vendored upstreams are sha256‑pinned (`f9dasm`, `ghidra`, `libvgm`, `vgmstream`, `snes9x-gtk`) or git‑commit‑pinned (`ppsspp`). No build artifacts are tracked (`dist/`, `public/` gitignored).

**🔴 The PR `test.yml` is red** (independent of the green `publish.yml`):
- **shellcheck has no `-S error` filter** (`test.yml:22‑23`) → existing SC2012/SC2064/SC2015 info/warnings fail the step. Same flaw in the `shellcheck` Taskfile target.
- **The build container omits `zip`+`python3`** (`test.yml:38‑40` vs `publish.yml:46‑50`); `build-rust-tool.sh` doesn't self‑install them; `dist/` is empty on fresh checkout so nothing is skipped → the two `blender-asset-finder*` builds fail in the PR test but succeed in publish.

**🟡 Lesser:** the per‑row CVE‑tracker link in the apt index is accidentally gated on `repology` (`generate-index.sh:145`), so only the 4 repology‑tagged packages get a CVE link while metapackages pulling dozens of universe packages show none. `yuzu` (`foundry-emulators-consoles-heavy`) is a hard `Depends:` on a defunct upstream — worth a live `apt-cache policy yuzu` on fresh 26.04. `ppsspp` submodules aren't commit‑pinned. Three different `Maintainer:` strings across one repo. One stray tracked editor file (`foundry-apt/.history/README.md`).

### 4b. `apt.worldfoundry.org` (sibling) — live, signed, spec‑exact

14/14 packages present (nothing missing or extra), last release tag `apt‑v0.1.37` (2026‑05‑22), CI compliant (Node‑24 pins, container builds via a Rust/maturin `apt-builder` image — the reason this repo needs a `Dockerfile` and foundry‑apt doesn't, since the add‑on builds a native `wf_core.so` via maturin), R2 sync with consistency + smoke checks. Version cascade is consistent (`blender-addons 1.1.3` → `worldfoundry 1.1.4` → `development 1.0.4`).

**🟡 Reproducibility violation — `prep` ships a binary blob, not source.** Its `debian/control` admits the lex/yacc grammar "was retired from the repo… until then, the deb is a re‑pack of the tracked binary blob." That breaks the "rebuildable from source" mandate and has **no TODO tracking the grammar restoration**. **🟡 arm64 is advertised but not produced** — aptly publishes `amd64,arm64,all` and `config.py` lists arm64, but every built `.deb` is `_amd64.deb` (1.0.3 changelogs explicitly switched `all`→`amd64` to stop arm64 install‑then‑fail). The `binary-arm64` index is effectively empty. Plus stale local aptly state still indexing the renamed‑away `worldfoundry-blender`, and several stale code comments (Dockerfile says "bookworm"/`publish.yml`; both wrong).

### 4c. Cross‑repo coupling — real, in both directions

| Direction | Edge |
|---|---|
| WF → Foundry | `worldfoundry-cli` `Depends: blender-asset-finder-cli`; `worldfoundry-blender-addons` `Depends: blender-asset-finder` (both live in foundry‑apt) |
| Foundry → WF | `foundry-core` `Depends: worldfoundry`; `foundry-atelier` `Depends: worldfoundry-development` |

This is **intentional and changelog‑documented**, but it means `apt install worldfoundry-cli` hard‑fails unless `apt.foundrylinux.org` is also configured, and vice‑versa. CLAUDE.md's "co‑installed but neither depends on the other being configured" should be reworded to: *"co‑installed; cross‑repo `Depends` mean both sources must be wired, but neither repo's publish pipeline depends on the other."*

---

## 5. Phase 2 — the Distrobox image (`foundry-devbox/`)

**Verdict: complete and live (v0.0.5).** `Dockerfile` is two layers — wire multiverse + both Foundry apt sources + Cloudsmith, then a single `apt install foundry-core` (`Dockerfile:62`). GHCR publish is tag‑driven (`:26.04`/`:<tag>`/`:latest`) and **smoke‑tests the pushed image** via `command -v` over 30+ tools (with `/usr/games` on PATH for MAME). Pins are clean (`checkout@v6`, `build-push@v7`).

**🟢 One stale assertion:** TODO.md:30 and the workflow comment still say the image installs `foundry-anvil`; it installs `foundry-core` (correctly — anvil adds `foundry-desktop`/KDE, which has no place in a container). The whole codebase already says `core`; only these two breadcrumbs lag.

---

## 6. Phase 3 — the ISO (`foundry-iso/`)

**Verdict: least‑complete phase, but substantially further along than TODO implies — the hard technical blockers are closed and boot‑verified.** The framing that "boot verification is pending" is **out of date by three days.**

**Closed & verified (2026‑05‑26):**
- **Autologin** — the real root cause was `live-config`'s `0085-sddm` overwriting `/etc/sddm.conf` with a blank `Session=` (because `username=user` is on the kernel cmdline). Fix is fully in source (`config/hooks/1100-live-autologin.hook.chroot`): bake `30-foundry-live.conf`, block `0085-sddm` via a pre‑touched sentinel, patch `casper-bottom/15autologin` + inject `16foundry-autologin` as defense‑in‑depth. Plan steps 11–14 are PASS (SSH to live machine `192.168.4.32` + visual autologin in QEMU/OVMF).
- **Calamares** — the 3.3.14 fatal‑YAML crash (missing `style:` map + window‑layout keys) is fixed in `calamares-settings-foundry-linux` (branding deb, **1.0.8**, built in foundry‑apt). Verified loading without error.
- **foundry‑welcome first‑login** — user‑confirmed appearing on first login; the bad `X-KDE-autostart-condition` was removed.

**Build tooling is reproducible:** one entry point `scripts/build-iso.sh` runs the whole `lb` pipeline inside a `--privileged ubuntu:26.04` container, fetches/dearmors apt keys, verifies hook output, post‑processes the ISO with `xorriso` to inject a UEFI ESP (live‑build 3.0~a57 doesn't emit EFI images), and patches grub branding. Local `.debs` are staged via `config/includes.chroot/tmp/local-debs/` + hook `1000` (deliberately avoiding `packages.chroot/`, whose local‑signed‑repo path triggers a broken `gpg --gen-key` in gnupg 2.4+). R2 bootstrap + upload + index scripts all exist.

**🟡 Open (operational, not technical):**
- **Size:** the only built artifact is `foundry-anvil-0.9.30-amd64.iso` at **15 GB** — vs a ~3.5 GB target (plan step 3 logged 5.0 GB at 0.9.16; it has since grown). `foundry-anvil` is no longer meaningfully smaller than atelier. **A package‑list/size decision is the true Phase 3 blocker.**
- **No atelier ISO has ever been built locally**; plan steps 4, 7–10 (CI publish, downloadable‑and‑verify, real‑hardware install, atelier sanity) are un‑run. Step 9 is blocked on a QEMU `rsync RERR_FILEIO` reading a 4.7 GB squashfs → needs real hardware.
- **No ISO is published** to R2 or anywhere; the upload→verify→install loop is unproven.
- **Calamares full install walk‑through (step 4)** is still "pending" — it loads, but no end‑to‑end click‑through install is signed off.
- **CI is intentionally disabled** (`workflow_dispatch` only; push/cron commented out, 2026‑05‑22, to conserve Actions minutes). Re‑enabling needs the runner decision: anvil is plausible on GH‑hosted with `maximize-build-space`, but atelier (~10–15 GB, ~75 min) likely needs a self‑hosted runner. CI secrets aren't bootstrapped (§9).
- **Internet Archive migration** (`scripts/upload-iso-ia.sh` + CF Worker redirect) is **not started — correctly deferred to v1.0.0.**

The living investigation doc `foundry-iso/docs/investigations/2026-05-23-live-build-hooks-boot-theming.md` (699 lines) is the canonical "why" reference and is current through 2026‑05‑25. The project even produced a real upstream SDDM patch (`sddm-theme-silent-fallback.patch`) from the theming debug.

---

## 7. Website (`foundrylinux.org`)

**Verdict: in good, current, coherent shape.** Two routes (`/` and `/packages`) rendered to **fully static HTML via esbuild + `renderToStaticMarkup`** — no React/Babel ships to the browser (the "prod React CDN" note is itself stale: there are *no* CDN script tags; React is a dev‑only SSR dependency). The `/packages` page is generated by a pure‑Node pipeline that fetches `Packages.gz` from five archives in parallel (both Foundry repos + Ubuntu main/universe/multiverse), computes display closures, joins per‑package meta sidecars, and fetches Ubuntu Security CVE live‑counts (with a `SKIP_CVE=1` escape hatch). Content is correctly framed for game‑makers‑new‑to‑Linux ("Coming from Windows? Start at Virtual machine or Dual boot"); VM download rows are live (HTTP 200), not placeholders. Deploys to Cloudflare Pages on tag/`workflow_run`/nightly‑cron, with a tags‑only Lighthouse job archiving into `site/lh/`.

**Defects (in priority order):**
1. **🟡 Hero copy‑button gives no feedback.** `ssr-render.js` `homePageScripts()` does `this.lastChild.textContent='COPIED'`, but the hero `.copy` button contains only an SVG — it blanks the icon and shows nothing. Commit `cf43be2` fixed this exact pattern in `copy.js` but missed the inline home‑page script. Use the `data-copied` path.
2. **🟡 Accessibility stuck at 91** — the topbar mark `<a class="topbar-mark" href="/">` has no discernible name at mobile widths (the `<b>` label is CSS‑hidden ≤600px, leaving a title‑less SVG). One `aria-label` likely takes a11y 91→100.
3. **🟡 Performance regression at v1.5.0** — home perf 99 (v1.4.0) → 85 (v1.5.0); TBT 0 ms → **470 ms** over the same span (CLS steady, good). CI runs Lighthouse n=1, so some variance, but the monotonic TBT climb looks real — candidate: embers canvas / scroll‑shrink rAF. Worth a profile.
4. **🟡 `serve.json` untracked but deploy‑critical** — the `/packages` rewrite ships only because `wrangler pages deploy site/` uploads the whole dir; a clean checkout would 404 `/packages` (link‑check resolves it locally and masks the prod dependency). **Commit it.**
5. **🟡 Generated artifacts committed** (`index.html`, `packages.html`, `packages-data.json`) — perpetually‑stale build output and merge‑conflict bait; gitignore them.
6. **🟢 `upload-artifact@v4`** (`site-deploy.yml:88`) should be `@v7`. **DESIGN.md is stale** as a spec (`--bg` #000 vs actual #111; `data-bg="pure"` undocumented/unstyled; eyebrow copy). **Lighthouse never audits `/packages`** — the heaviest page (179 KB, third‑party Repology `<img>` badges) is the perf blind spot.

---

## 8. Workflows, dependencies, ordering, tooling

### Workflow census (7 workflows; all `.deb`/ISO builds in `ubuntu:26.04` containers)

| Workflow | Triggers | Build venue | Node pins | Notes |
|---|---|---|---|---|
| `site-deploy.yml` | tag `v*`, `workflow_run` (after apt publish), nightly cron, dispatch | host (Node 22) | v6/v6/**v4**⚠ | upload‑artifact straggler |
| `foundry-apt/publish.yml` | tag `v*`, dispatch | **container** ✓ | v6/**v7** ✓ | sign + R2 + consistency + smoke‑install — green |
| `foundry-apt/test.yml` | all branches, PR | **container** ✓ | v6/v7 ✓ | **🔴 red** (shellcheck filter + missing zip/python3) |
| `foundry-devbox/publish.yml` | tag `v*`, dispatch | buildx | v6/v4/v3/**v7** ✓ | GHCR + smoke — green |
| `foundry-iso/publish.yml` | **dispatch only — auto DISABLED** | container | v6/**v4**⚠ | matrix anvil+atelier; off since 05‑22 |
| `foundry-setup/test.yml` | push/PR `foundry-setup/**` | container | **v4**⚠ ×3 | dry‑run green, full‑install 🔴 (§4); dead `paths:` ref |
| (sibling) `worldfoundry.org/apt-publish.yml` | tag `apt-v*`, dispatch | **container** ✓ | v6/v7 ✓ | green |

No `FORCE_JAVASCRIPT_ACTIONS_TO_NODE24` anywhere (good). Three workflows lag the Node‑24 pin mandate (forced‑off 2026‑06‑02).

### Dependency ordering — the chain & its hazards

```
Phase 0  install.sh  → wires apt source BEFORE any metapackage → apt installs Phase-1 metas
Phase 1  TWO R2 repos: apt.foundrylinux.org (resolute) + apt.worldfoundry.org (stable)
           foundry-core Depends: worldfoundry  ← CROSS-REPO, resolved at install not build
Phase 2  devbox Dockerfile installs foundry-core → pulls from BOTH live repos at build time
Phase 3  iso-sync-local-debs copies foundry-apt/dist/*.deb → local-debs/ (newest-wins, SHADOWS live repo)
```

**Hazards:** (1) **Cross‑repo publish ordering** — if foundry‑apt ships a `foundry-core` depending on a not‑yet‑published `worldfoundry`, devbox/ISO installs break; the two repos publish from independent tag namespaces (`v*` vs `apt-v*`) with **no pre‑publish gate** (post‑publish smoke tests catch a hard break, but after the fact). (2) **The devbox consumes the *live* repos** (no local shadow), so it can only build after the relevant apt publish propagates; the ISO mitigates this via `local-debs/` shadowing.

### The cross‑repo dispatch gap (the biggest design‑vs‑reality delta)

`FOUNDRYLINUX_DISPATCH_PAT` appears **only in TODO.md and plan docs — in no workflow YAML or script.** `site-deploy.yml` rebuilds `/packages` via `workflow_run` (same‑repo) + nightly cron + tag, but has **no `repository_dispatch`**; and **no WF workflow sends a dispatch** to foundrylinux.org (the WF `deploy.yml` *receives* an unrelated `wf-status-updated`). The dispatch was added then immediately reverted in the WF repo and shipped in no release. **Net:** a WF apt publish only reaches the Foundry packages page via the nightly cron (≤24 h stale). TODO's "pending" is accurate.

### Reproducibility audit

**Strong overall.** `scripts/bootstrap.sh` is a single idempotent entry point that sets `{GPG_PRIVATE_KEY, R2_ACCESS_KEY_ID, R2_SECRET_ACCESS_KEY, R2_ENDPOINT}` on `foundry-apt`, mirrors each to a private `foundry-secrets` R2 bucket for DR, and `shred -u`s the local copies — matching the mandate verbatim. All four names match what `foundry-apt/publish.yml` and the WF `apt-publish.yml` consume.

**🟡 Gap — `foundry-iso` CI secrets are not bootstrapped.** `foundry-iso/publish.yml` references **`R2_ACCOUNT_ID`** and **`GPG_PASSPHRASE`**, which **no script sets** (`bootstrap.sh` sets `R2_ENDPOINT`, never `GPG_PASSPHRASE`; `bootstrap-r2.sh` creates the bucket but `gh secret set`s nothing). Masked today because ISO CI is disabled and local builds read `.foundry/bootstrap.env` — but re‑enabling CI will fail on missing secrets, an un‑scripted manual step. Also: `infra-setup.md`'s status checklist is all‑unchecked despite Phase 1 being live (stale doc, not stale infra); the hook‑integrity wrapper is not wired here (`.claude/settings.json` is `{"hooks":{}}`), so the documented `task md` auto‑preview doesn't fire — run it manually.

### Tooling hygiene

`set -euo pipefail` discipline is clean across every `.sh` checked. shellcheck coverage has holes: **root `scripts/`, all of `foundry-iso/scripts/`, and the per‑metapackage `foundry-setup/install-foundry-*.sh` are linted by no automation.** `.gitignore` covers the multi‑GB scratch (`dist/`, `cache/`, `.build/`, `*.deb`, `node_modules/`, `.wrangler/`) — none accidentally tracked — but **misses `.history/`** (see §10) and `site/serve.json`. Gitignored‑but‑physical clutter (stale `*.deb`/`*.dsc` in `foundry-apt/packages/`) sits next to authored `debian/` dirs; `task dist-prune` only cleans `dist/`.

---

## 9. Documentation

**Census:** ~85 docs — **50 plans · 16 investigations (+1 .tsv) · 11 transcripts · 5 ancillary (apt/iso) · 3 root** (`CLAUDE.md`, `DESIGN.md`, `TODO.md`; no repo‑root README/CONTRIBUTING — those are per‑subproject).

**Verification discipline** was tight during the packaging push (05‑18→05‑22) and on the ISO work — many plans carry the house format (numbered steps → raw output → PASS/FAIL), and the Phase 3 plan is an exemplary multi‑session log. It **slipped on the late‑May site/apt‑index plans**: several (`apt-package-meta`, `foundry-core-desktop-split`, `install-section-redesign`, the 215‑byte `apt-index-two-column-table` stub) are marked done with a `## Verification` header but **no pasted output** — evidence lives in transcripts/commits instead of the plan.

**Doc ↔ reality drift:**
- **`foundry-apt/README.md`** is the worst single drift — its tables list **1 metapackage and 1 vendored upstream** vs the 24 + 6 that exist. A new contributor would massively under‑estimate the project. **🟡 Partially fixed (06‑04):** the **vendored‑upstream table is now complete (1 → 14 rows)** and `LICENSES-VENDORED.md` rewritten to match; the **metapackage table is still a single row** — that half of action #9 remains open.
- **CLAUDE.md** is accurate on infra (two‑repo split, canonical layout, container builds, "no AWS/SSM") but its **architecture section never mentions the editions** or the core/desktop split — it predates 2026‑05‑28.
- **TODO.md** is current and well‑maintained, with two genuinely‑open carryovers (create `FOUNDRYLINUX_DISPATCH_PAT`; flip monorepo public) and one **dangling link** — it points at `2026-05-16-foundry-distro-proposal.md` but the file is `2026-05-16-foundry-linux-distro-proposal.md`.
- **Renamed/retired names are clean** — `foundry-linux-*` and `xa65` references are confined to the historical rename/retire plans.

**Investigations were largely acted on** — dropped‑emulators → packaged same day; iso‑hosting → IA TODO; apt‑listing‑landscape → meta JSON + filter/sort shipped; gh‑actions‑usage → ISO CI disabled. **Duplication** is the main hygiene issue: **four** package‑inventory investigations coexist (the audit script re‑dates a new 1300‑line file each run instead of overwriting one canonical doc), plus four draft/redo plan pairs (`lighthouse-ci`, `version-deb-links`, `per-metapackage-install-scripts`, two node24 drafts) never consolidated.

---

## 10. Other things worth surfacing

- **🟡 The product drifted from "WF game‑dev toolchain" toward "general creative/retro‑gaming distro"** — ~20 art/audio/emulator/games metapackages (Krita, Ardour, Dolphin, PCSX2, 0 A.D., …) appeared with no proposal basis. This may be entirely intentional, but it has never been written down as a deliberate scope expansion, so the *stated* mission (CLAUDE.md, proposal, even the site's framing) and the *shipped* catalogue have quietly diverged. **Decide and document the actual product definition.**
- **🟡 "Anvil" has an identity problem.** It's the website's headline badge and the default ISO, yet at 15 GB it's the *kitchen‑sink* edition, not the lean base its name implies — and the genuinely‑lean rung (`foundry-core`) is the one that ships in the container and is invisible to users. The edition story that's marketed and the edition reality don't line up.
- **🟡 GitHub Actions minutes are the silent constraint behind several decisions** — the `wbniv` org was at 82–100% of the 2,000‑min cap (investigation 05‑22), which is *why* ISO CI is disabled and the Depot/Blacksmith eval exists. The lever that resolves it — **flipping the monorepo public (free CI)** — is an open TODO. This single toggle unblocks ISO CI economics; it deserves to be promoted from housekeeping to a decision.
- **🟡 Editor `.history/` leakage** (34 tracked snapshots, 7 dirs, not gitignored) is the most pervasive low‑grade hygiene issue — it recurs as untracked noise every session and pollutes `git status`. One `.gitignore` line + `git rm -r --cached` closes it.
- **🟢 Uncommitted work in `git status` is legitimate** — the four newest transcripts, the newest package‑inventory, a new memory file, and several plan currency‑fixes (status flips + post‑rename name corrections that *remove* now‑false drift warnings). These should be committed, not ignored.
- **🟢 No security surprises** — single‑key GPG signing via GH Actions secret + DR backup to a private R2 bucket, local copies shredded; no secrets in tracked files; no AWS. The proposal's elaborate AWS‑SSM/OIDC/YubiKey two‑tier signing was dropped in favor of this simpler model (CLAUDE.md is the source of truth, the proposal is stale).

---

## 11. Prioritized action list

> **06‑04 refresh:** strikethrough = done since the audit; ⏳ = still open. See the status table in §1 for the rollup.

**P0 — red CI / correctness (fix before next release):**
1. ~~Re‑add a no‑op `--skip-clone)` case to `foundry-setup/install.sh` parse loop.~~ ✅ `5f3cda5` (`install.sh:70`). [§4]
2. ~~`foundry-apt/test.yml`: fix the shellcheck findings + add `zip python3` to the build container.~~ ✅ `5f3cda5` — test.yml green. [§4a]
3. ~~Decide the cross‑repo `/packages` refresh: wire `FOUNDRYLINUX_DISPATCH_PAT` end‑to‑end.~~ ✅ Wired both ends (`9d5dd78`). ⏳ **One manual step left for Will:** create the PAT + `gh secret set FOUNDRYLINUX_DISPATCH_PAT` (TODO L33). [§8]

**P1 — Phase 3 to 1.0:**
4. ~~Make the anvil package‑list/size decision (15 GB → target)~~ ✅ **Analysis done 06‑04** — anvil rebuilt to 4.8 GB; SLIM ~3.1 GiB edition proposed (`docs/plans/2026-06-04-usb-sized-iso-editions.md`). ⏳ **Decision now = greenlight SLIM + then build atelier.** [§6]
5. ⏳ Exercise the publish loop once by hand (build→sign→R2→download→install) and record evidence in the Phase 3 plan's open steps. [§6]
6. ⏳ Sign off the Calamares full install walk‑through (plan step 4). [§6]
7. ⏳ Before re‑enabling ISO CI, script the `foundry-iso` secrets (`R2_ACCOUNT_ID`, `GPG_PASSPHRASE`) and decide the atelier runner. [§8]

**P1 — docs honesty:**
8. ~~Retire the 2026‑05‑16 proposal as historical (banner it); document the creative‑distro scope expansion.~~ ✅ **Done 06‑04** — `status: SUPERSEDED` + top banner enumerating the divergences (incl. the creative‑distro scope expansion). [§9, §10]
9. ~~Rewrite `foundry-apt/README.md` (1‑row tables → 40 packages); add the editions to CLAUDE.md; reword the "neither depends" claim.~~ ✅ **Done 06‑04** — README vendored table (→14) **and** metapackage table (→ edition tiers + 19 category rows); `LICENSES-VENDORED.md` (→14); CLAUDE.md gained an Editions section, a corrected vendored list, and the pipeline‑vs‑package‑coupling reword. [§4c, §9]
10. ⏳ File a TODO for the `prep` grammar restoration (binary‑blob reproducibility violation); decide arm64 (build it or stop advertising it). [§4b]

**P1 — site:**
11. ⏳ Fix the hero copy‑button feedback (`ssr-render.js:45` still uses the buggy `lastChild.textContent`); ⏳ add the topbar `aria-label` (still missing, `sections.jsx:37`); ~~commit `serve.json`~~ ✅ `e39feb0`; ⏳ gitignore the 3 generated artifacts; ⏳ profile the v1.5.0 TBT regression. [§7]

**P2 — hygiene:**
12. ~~Add `.history/` to `.gitignore` and `git rm -r --cached` the snapshots.~~ 🟢 `7f5e1ba` (33/34); ⏳ **one straggler `.history/CLAUDE.md` still tracked.** [§10]
13. ~~Bump the straggler action pins to Node‑24 majors.~~ ✅ **Done 06‑04** — all 6 bumped to `@v6`/`@v7`; tree census clean. [§8]
14. ⏳ Extend shellcheck CI to root `scripts/`, `foundry-iso/scripts/`, `foundry-setup/install-foundry-*.sh`. [§8]
15. ⏳ Collapse the 4 package‑inventory investigations to one canonical overwrite‑in‑place doc; consolidate the draft/redo plan pairs. [§9]
16. ~~Fix the devbox `foundry-anvil`→`foundry-core` breadcrumb~~ ✅ `49cf608`; ⏳ confirm the dangling proposal link now resolves (proposal moved to `docs/investigations/`). [§5, §9]

---

## 12. Bottom line

The **distribution itself is healthy** — two live signed apt repos, a published container image, a polished website, and an ISO whose hard problems are already solved. The work that remains is concentrated and well‑understood: a **size/package‑list decision for the ISO**, **one clean publish run**, and a **cleanup pass on the scaffolding** — red CI gates, a phantom cross‑repo dispatch, committed build/editor artifacts, and a proposal that no longer matches the product. None of it is architectural; all of it is finishable. The single highest‑leverage non‑code decision is **flipping the monorepo public**, which turns the GitHub‑Actions‑minutes constraint (the hidden reason ISO CI is off) from a budget problem into a non‑issue.

> **06‑04 update to the bottom line.** Three of the four scaffolding worries above are now closed (the red CI gates and the cross‑repo dispatch are fixed; the editor artifacts are gitignored bar one straggler). The **ISO size question moved from "decision" to "decided‑on‑paper"** — anvil is already down to 4.8 GB and the full sizing analysis + a SLIM edition proposal exist; what's left there is a **greenlight + the first real publish run**, not analysis. The remaining genuinely‑open items are small and enumerated in the §1 refresh table: the overdue Node‑24 pins (now past their forced‑off date), the docs‑honesty trio (CLAUDE.md reword + editions section + proposal banner), and Phase‑3‑to‑1.0 execution. **Flipping the monorepo public** remains the single highest‑leverage move.

---

## 13. Confidence & provenance

**Verified directly against the tree / live endpoints during this audit:** the `--skip-clone` break (`install.sh:66‑72` vs `run-test.sh:58`/`test.yml:50`); the `foundry-apt/test.yml` shellcheck filter absence (`:22‑23`); 34 tracked `.history/` files + no gitignore entry; devbox installs `foundry-core` (`Dockerfile:62`); the 3 committed generated site files + untracked `serve.json`; the 15 GB anvil‑only ISO with no atelier; the 32 + 14 package counts; the proposal filename / TODO dangling link.

**Relayed from subsystem audits (high confidence, file:line‑grounded, not independently re‑run here):** the live R2 apt‑repo freshness (audited against the live `Packages` endpoint by the apt agent, timestamp 2026‑05‑29T01:04Z); the specific shellcheck finding codes (SC2012/SC2064/SC2015); the website Lighthouse score trend and the hero copy‑button/aria‑label defects; the WF `prep` binary‑blob and arm64 claims; the ISO boot‑verification PASS records in the Phase 3 plan.

**Not verified (flagged for follow‑up):** whether `yuzu` is still in 26.04 multiverse (needs a live `apt-cache policy` on a fresh container); whether the v1.5.0 TBT regression is real vs n=1 Lighthouse noise (needs a profile); the exact current CVE live‑counts (a moving, build‑time‑fetched number — 47 active / 71 clear of 118 badged at last build, vs the TODO's stale 42/90).
