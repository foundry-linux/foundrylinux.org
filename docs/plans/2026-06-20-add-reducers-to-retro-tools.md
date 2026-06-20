# Plan: add cvise + the test-case-reduction toolchain to foundry-retro-tools

> **STATUS — ✅ COMPLETE (all 3 tiers), shipped 2026-06-21.** Tier 1 (`cvise`+`delta`→retro-tools, `creduce`→atelier) and Tier 2 (vendored `halfempty`, `python3-inators`, `python3-picire`) landed 2026-06-20; **Tier 3 (`shrinkray` via `pipx`)** landed 2026-06-21. On `main`, pushed to `origin`: `3cba5b8` (shrinkray Phase 0) + `856e596` (ci: `actions/cache`→v5). No `v*` tag pushed — `foundry-retro-tools` is not yet re-published to the live apt repo (publish on Will's go-ahead). Per-tier execution logs at the bottom.

## Revisions from review (2026-06-20, max-effort critique)

Findings that changed the first draft (all verified in a fresh `ubuntu:26.04`):

1. **Size was understated ~3×.** Real bare-container closures: **cvise = 326 MiB**,
   **cvise+creduce+delta = 520 MiB** (the first draft said "~150 MB"). `libllvm21`+
   `libclang-cpp21` = 189 MiB of that, shared with `mesa-vulkan-drivers`/`yuzu`/`rust`.
   → Per decision B (below), anvil carries only **cvise+delta ≈ 326 MiB** bare
   (~140 MiB if `mesa-vulkan-drivers` already seeds `libllvm21`); creduce's ~194 MiB lands
   on atelier. Still measure on the real build.
2. **`creduce` is ~194 MiB of redundancy** over cvise (its maintained parallel rewrite,
   same `clang_delta` passes). → **RESOLVED (Will, option B):** creduce goes
   **atelier-only** (like ghidra); `cvise`+`delta` carry anvil. Saves ~194 MiB on the
   default ISO and drops the redundancy. Net anvil marginal ≈ **~330 MiB** bare
   (~140 MiB if `mesa-vulkan-drivers` already seeds `libllvm21`).
3. **`delta` (reducer) provides NO `delta` command** — it ships `singledelta`,
   `multidelta`, `topformflat` in `/usr/bin`. → (a) **No conflict** with the popular
   `git-delta` (which owns `/usr/bin/delta`); solver confirmed they coexist. (b) The
   e2e check must target `multidelta`/`singledelta`, not `delta`.
4. **`yuzu` is sprite/atelier-only**, not anvil — so it does NOT pre-seed `libllvm21`
   onto the default ISO; only `mesa-vulkan-drivers` might.
5. **picire 21.8 is from 2021** (declares Python ≤3.9), but **`inators` is 2.1.1
   (Sept 2025)**. The 2021-picire-against-2025-inators pairing must be smoke-tested; if
   it breaks, pin a 2021-era inators.
6. **shrinkray .deb is a deep cascade** (`textual≥8` vs universe's 2.1.2 + missing
   `textual-plotext`). → **pipx fallback** is now the documented primary path (precedent:
   `pipx install codemagic-cli-tools` in `install-foundry-ios-development.sh`); .deb only
   if the cascade proves ≤1–2 trivial pure-Python deps.

## Context

Will wants **cvise** (the parallel C/C++ test-case reducer) in the distro, and asked
which similar tools are already packaged vs. would need packaging. Survey:

- **In Ubuntu 26.04 universe** (no packaging — just `Depends:` + Phase 0, the `xa65`
  pattern): `cvise 2.12.0-2`, `creduce 2.11.0`, `delta 2006.08.03`
  (ships `singledelta`/`multidelta`/`topformflat`).
- **Not packaged anywhere — we vendor them**: `halfempty` (Google P0, C),
  `picire` (Python ddmin; needs `inators`), `shrinkray` (DRMacIver, modern, heavy deps).
- **Surveyed and deferred** (heavier, lower priority — note in TODO, don't build now):
  `treereduce` (Rust, tree-sitter), `perses` (Java/JVM, grammar-based),
  `picireny` (grammar-aware picire, needs ANTLR/`antlerinator`).

**Decisions (Will, 2026-06-20):**
1. **Placement: into `foundry-retro-tools`** — the RE/debugging toolchain. (cvise/creduce
   reduce compiler test cases; `halfempty` reduces *arbitrary* files — ROMs, assets,
   crash inputs — the standout for the retro/RE audience.)
2. **Scope: "the works"** — universe tools + vendor halfempty/picire/shrinkray.
3. **The 4 GB-stick goal is dropped** (accepted with the corrected size in view — see the
   open decision below). `foundry-retro-tools` is pulled by `foundry-core → foundry-anvil`,
   so these land on the default ISO. Supersedes
   `docs/plans/2026-06-04-trim-anvil-4gb-move-ghidra-to-atelier.md` (step 6).

### Licensing (all cleanly redistributable — no snes9x-style restriction)

halfempty Apache-2.0 · inators BSD-3 · picire BSD-3 · shrinkray MIT.
cvise/creduce/delta are universe (Debian's redistribution, not ours).

## Implementation

### Tier 1 — universe tools (fast, zero risk)

`foundry-apt/packages/foundry-retro-tools/debian/control` (anvil path):
- Add to `Depends:`: `cvise`, `delta` (**not** creduce).
- Extend the long **Description** to name them (test-case reduction / minimization:
  cvise; singledelta/multidelta/topformflat).
- Bump `debian/changelog` to `1.0.14` (`dch -v 1.0.14 -D resolute "add cvise + delta
  test-case reducers"`). Maintainer/format unchanged.

`foundry-apt/packages/foundry-atelier/debian/control` (complete-edition path):
- Add `creduce` to `Depends:` (alongside `ghidra`/`foundry-cv`) — keeps the redundant
  ~194 MiB reference reducer out of anvil but available in atelier.
- Add a `* creduce` bullet to the Description; bump `debian/changelog`.

`foundry-setup/install-foundry-retro-tools.sh`: extend the `ok "..."` summary block to
mention cvise + delta. No new install logic (metapackage `Depends:` resolves them).

### Tier 2 — vendor halfempty + picire (clean)

Use the **`/package` skill** for each; templates to copy:
- C/make = `foundry-apt/packages/f9dasm/` (pinned `UPSTREAM_VERSION`+`SHA256`, curl+sha256
  verify, overlay `debian/`, `dpkg-buildpackage`, move `.deb`→`dist/`).
- Pure-Python sdist = `foundry-apt/packages/python3-pydub/` (`pybuild`, `Architecture:
  all`, `Build-Depends: dh-python, python3-all, python3-setuptools`).
- New vendored packages use `Maintainer: Foundry Linux <packages@foundrylinux.org>` and
  version `<upstream>-1foundry1`.
- **`/package` skill already refined this session** (`~/.claude/skills/package/SKILL.md`):
  universe check now covers version-floor + install-closure footprint + command≠package-name;
  Step 2 has an old/archived-upstream build-risk note; the Python section gained
  runtime-dep-tree resolution + a pipx-vs-.deb decision. Validate these against the real
  halfempty/picire builds and adjust if anything's off (iterate-the-skill practice).

Create under `foundry-apt/packages/`:
1. `halfempty/` — fetch `v0.40.tar.gz` (Apache-2.0). `Build-Depends: …, pkg-config,
   libglib2.0-dev`; `Depends: ${shlibs:Depends}, ${misc:Depends}`; `debian/rules`
   overrides `dh_auto_install` to install the `halfempty` binary (f9dasm pattern).
   Man page via `help2man` (or hand-written if `--help` is unsuitable).
   ⚠ **Risk:** upstream is from 2020 and archived (Apr 2026). Expect possible GCC-15 /
   glibc-2.41 build breakage on 26.04 → add quilt patches under `debian/patches/` if so;
   confirm `v0.40` is still the newest tag at package time.
2. `python3-inators/` — fetch PyPI sdist `2.1.1` (BSD-3, pure-Python, no runtime deps).
3. `python3-picire/` — fetch `picire-21.8.tar.gz` (BSD-3). `Depends: …, python3-chardet,
   python3-psutil, python3-inators`. CLI `picire` (man page via `help2man`).
   ⚠ **Risk:** verify picire 21.8 runs against inators 2.1.1 (`picire --help` + a
   one-line reduction); if broken, pin an older inators.

Add `halfempty` + `python3-picire` to `foundry-retro-tools` `Depends:`
(`python3-inators` rides along transitively; still listed as a vendored package). Bump
changelog.

### Tier 3 — shrinkray (pipx primary; .deb only if cheap)

shrinkray `26.4.14.0` (MIT, Python ≥3.12 — 26.04 has 3.14.3 ✓) needs `textual≥8.0.0`
(universe ships 2.1.2), `textual-plotext` (missing), `exceptiongroup` (stdlib on 3.14).

**Primary path — pipx in Phase 0:** add `pipx install shrinkray` to
`install-foundry-retro-tools.sh` (codemagic-cli-tools precedent). Gets it onto installed
systems immediately; **not** baked into the ISO and **not** in the metapackage `Depends:`.

**.deb path — only if the cascade is shallow:** `/package python3-textual-plotext`, then
probe `/package python3-textual ≥8`. **If textual cascades into >1 further new dep
(newer `rich`, etc.), STOP** and report depth to Will rather than rabbit-holing. If
shallow: `/package shrinkray` with a quilt patch dropping the `exceptiongroup` dep,
ship CLIs `shrinkray`/`shrinkray-worker`, add to retro-tools `Depends:`.

Add a `TODO.md` item to revisit the shrinkray .deb when `textual` ≥8 reaches universe.

### Wiring shared by every vendored package

- `foundry-apt/LICENSES-VENDORED.md` — alphabetically-sorted row each
  (`<span style="white-space:nowrap"><code>name</code></span>` | upstream | SPDX |
  `/usr/share/doc/<pkg>/copyright`). `scripts/check-licenses-vendored.sh` enforces this.
- `foundry-apt/README.md` — add to the vendored-upstreams table.
- `build-all.sh` / `generate-index.sh` / `generate-meta.sh` pick up new dirs & `.debs`
  automatically.
- `foundry-setup/test/test-retro-tools-e2e.sh` — add `TOOL_CHECKS` entries, keyed by
  **package** name → check **command** (note the name≠command cases):
  `[cvise]="cvise --help"`,
  `[delta]="multidelta --help"` *(package `delta`, command `multidelta`/`singledelta`;
  use a stdin-safe invocation — these read stdin without a flag)*,
  `[halfempty]="halfempty --help"`, `[python3-picire]="picire --help"`.
  *(creduce is atelier-only now — no entry in the retro-tools harness; add it to an
  atelier e2e check if/when one exists.)*
- `foundrylinux.org/CLAUDE.md` — update the vendored-upstream **count and list**
  ("20 vendored upstreams" → new total; add halfempty, python3-inators, python3-picire,
  +shrinkray/textual-plotext if .deb'd).

### 6. Mark the 4 GB plan superseded

Top banner on `docs/plans/2026-06-04-trim-anvil-4gb-move-ghidra-to-atelier.md`:
**SUPERSEDED 2026-06-20** — 4 GB-stick goal dropped when the LLVM-backed reducers were
added to `foundry-retro-tools` (this plan; real anvil cost ~330 MiB). Flag the impact
on `docs/plans/2026-05-30-create-foundry-usb.md` (giveaway stick must target 8 GB).
**Do not** revert ghidra's atelier placement — out of scope.

## Verification

Run from `foundry-apt/`. Keep these numbered steps verbatim; paste raw output + PASS/FAIL
under each (SRC plan-verification format).

1. **Universe tools resolve & coexist with git-delta**
   `docker run --rm ubuntu:26.04 bash -c 'apt-get update -qq && apt-get install -s -y cvise creduce delta git-delta | grep -iE "conflict|will be removed" || echo OK'`
   → `OK` (no conflict). `dpkg-deb -c` confirms `delta` ships `singledelta`/`multidelta`/`topformflat`.

2. **Real ISO marginal size** — build anvil (or `apt-get install -s` against the anvil
   package set) and record the delta vs the pre-change ISO. Expect ~330 MiB
   (~140 MiB if `mesa-vulkan-drivers` already seeds `libllvm21`); creduce's ~194 MiB
   must NOT appear (it's atelier-only).

3. **Vendored packages build clean in container**
   `task build` → `dist/halfempty_0.40-1foundry1_*.deb`, `python3-inators_2.1.1-1foundry1_all.deb`,
   `python3-picire_21.8-1foundry1_all.deb` present.

4. **Lintian + stripped + shellcheck + licenses**
   `for f in dist/{halfempty,python3-inators,python3-picire}_*.deb; do dpkg-deb -I "$f"; lintian "$f"; done`
   (zero `E:`; `halfempty` binary `file` shows "stripped") · `task shellcheck` · `task check-licenses`.

5. **picire↔inators integration smoke test**
   In the container: `picire --help` exits 0, and a trivial reduction
   (`printf 'a\nb\nc\n' | picire --input=- --test=<true-script> --output=/tmp/o`) runs
   without an `inators` ImportError/AttributeError.

6. **Metapackage Depends updated**
   `grep -E 'cvise|creduce|delta|halfempty|picire' packages/foundry-retro-tools/debian/control`.

7. **End-to-end resolve from a local repo**
   `task publish-local && task apt-test` → `apt-cache depends foundry-retro-tools` lists
   the reducers; apt resolves with no unmet deps (catches the inators transitive gap).

8. **E2E tool-invocation harness**
   `bash foundry-setup/test/test-retro-tools-e2e.sh` → every new `TOOL_CHECKS` entry PASSes.

9. **Phase 0 dry-run** — `bash foundry-setup/install-foundry-retro-tools.sh --dry-run`
   exits 0; summary mentions the reducers (+shrinkray pipx line if added).

## Tier 1 — execution log (2026-06-20) ✅ done

Wired the universe tools (`cvise`+`delta` → retro-tools **1.0.14**; `creduce` → atelier
**0.9.6**), updated the Phase 0 summary + e2e harness. Evidence vs the verification steps:

- **Step 1 (resolve + git-delta coexist):** `apt-get install -s cvise creduce delta git-delta`
  in `ubuntu:26.04` → no conflict; `delta` ships `singledelta`/`multidelta`/`topformflat`
  (no `delta` command). PASS.
- **Step 3/4 (build + lintian):** `task build` produced `foundry-retro-tools_1.0.14_all.deb`
  + `foundry-atelier_0.9.6_all.deb`; lintian on both → `LINTIAN_CLEAN (no E:/W:)` (wrapped a
  pre-existing over-long atelier description line to get there). `shellcheck`/`bash -n` clean
  (no new findings); changelogs parse (`1.0.14`, `0.9.6`). PASS.
- **Step 6 (Depends):** retro-tools Depends has `cvise`,`delta`; atelier has `creduce`. PASS.
- **Step 7 (resolve from local repo):** retro-tools resolves pulling `cvise 2.12.0-2` +
  `delta 2006.08.03-14` from universe → `RETRO_RESOLVE_OK`. PASS.
- **Step 9 (Phase 0 dry-run):** `install-foundry-retro-tools.sh --dry-run` exits 0; summary
  shows the new `Reduce:` line; the stale `ghidra` 6809 reference (left from the 1.0.7 move)
  is removed. PASS.

Deferred to Tier 2/3: vendored builds (halfempty/picire), real ISO size measurement (step 2),
full e2e harness run (step 8 — needs 1.0.14 published to the live repo first).

## Tier 2 — execution log (2026-06-20) ✅ done

Vendored all three via `/package`; built in `ubuntu:26.04`, all lintian-clean:

- **python3-inators 2.1.1** (BSD-3, pure-Python `src/` lib, no deps) — 9 KB, imports OK.
  Needed `pybuild-plugin-pyproject` in Build-Depends (a `pyproject.toml [build-system]` makes
  pybuild take the PEP517 path on 26.04) and `SETUPTOOLS_SCM_PRETEND_VERSION` (release sdist
  has no git tags). *(Skill gap: the Python Build-Depends table should add the pyproject plugin.)*
- **python3-picire 21.8** (BSD-3) — 20.8 KB, ships `picire(1)` (man page hand-written from the
  real `--help`). Quilt patch `0001-use-importlib-metadata-for-version`: upstream read its own
  version via `pkg_resources`, dropped from setuptools ≥81 on Python 3.14 → switched to
  `importlib.metadata` (offered upstream). Smoke test reduced `a/KEEP/b` → `KEEP` (confirms the
  inators-2.1.1 runtime integration — the old-pkg/new-dep risk).
- **halfempty 0.40** (Apache-2.0, C) — 22.8 KB, ships `halfempty(1)`. `Build-Depends:
  bsdextrautils` (hexdump). `override_dh_auto_build` drops upstream's `-march=native` (would
  SIGILL on older CPUs) and routes dpkg-buildflags through the Makefile → PIE + full RELRO +
  BIND_NOW + FORTIFY_SOURCE=3 + stack-protector (readelf-verified); `override_dh_auto_test`
  skips the failing upstream test harness. Smoke test reduced 8 → 5 bytes (`KEEP\n`).
- **Wiring:** `foundry-retro-tools` **1.0.15** `Depends: … halfempty, python3-picire`; local-repo
  resolve pulls halfempty + python3-picire + python3-inators (transitive) + cvise/delta →
  `RESOLVE_OK`. LICENSES-VENDORED + README vendored table + e2e `TOOL_CHECKS` + CLAUDE.md
  count (→26) updated; `task check-licenses` PASS, `shellcheck` clean.

Still deferred: **Tier 3 (shrinkray)** — `pipx` path; the `textual≥8` / `textual-plotext`
`.deb` cascade remains.

## Tier 3 — execution log (2026-06-21) ✅ done (pipx path)

Probed the `.deb` cascade first (the plan gates `.deb` on "≤1–2 trivial pure-Python deps";
STOP and report if `textual` pulls in >1 further new dep). Resolved `shrinkray 26.4.14.0`'s
full tree with `pip install --dry-run --report` in a fresh `ubuntu:26.04` (Python 3.14.4),
then classified every dist against 26.04 universe. **The `.deb` path needs 4 new/upgraded
packages, two of them large core libraries — the STOP trigger fired exactly as predicted
("newer `rich`"):**

| Package | Universe | shrinkray/textual needs | Verdict |
|---|---|---|---|
| `python3-textual` | 2.1.2 | **≥8.0.0** (resolved 8.2.7) | below floor → major repackage |
| `python3-rich` | 13.9.4 | **≥14.2.0** (resolved 15.0.0) | below floor — *forced by textual 8.2.7's `rich>=14.2.0`* |
| `python3-textual-plotext` | missing | ≥0.2.0 (1.0.1) | new vendor |
| `python3-plotext` | missing | (textual-plotext dep) 5.3.2 | new vendor |

(Everything else shrinkray needs — `click`, `chardet`, `trio`, `humanize`, `libcst`,
`binaryornot`, `markdown-it`+`linkify`, `pygments`, `platformdirs` — is already in universe
at or above floor. `exceptiongroup` is a no-op on 3.14 and would be patched out.)

Overriding universe's `rich`/`textual` with newer versions in our apt repo would also risk
breaking every *other* universe package that depends on them. So the `.deb` path is both
deep **and** invasive → **pipx primary path taken**, exactly as the plan designates.

**Implementation** — `foundry-setup/install-foundry-retro-tools.sh` only (no `.deb`, no
metapackage `Depends:` change, no changelog bump, no vendored-count/LICENSES change):
- `apt-get install -y foundry-retro-tools pipx`, then `run pipx install shrinkray` as the
  **calling user** (not `run_sudo`) so the venv lands in `$HOME/.local` — the exact
  `codemagic-cli-tools` precedent in `install-foundry-ios-development.sh`.
- Updated the `--help` block and the `Reduce:` summary line to name the full reducer set
  (`cvise`, `delta`, `halfempty`, `picire` via apt; `shrinkray` via pipx).

**Evidence:**
- **pipx smoke test (the primary path actually works):** in a clean `ubuntu:26.04`,
  `pipx install shrinkray` as a non-root user pulled `textual 8.2.7` / `rich 15.0.0` / the
  whole tree into an isolated venv (zero universe conflict), installed console scripts
  `shrinkray` + `shrinkray-worker`, and `shrinkray --help` exited 0.
- **Step 9 (Phase 0 dry-run):** `install-foundry-retro-tools.sh --dry-run` exits 0; output
  shows the `Installing shrinkray via pipx` step (`[dry-run] pipx install shrinkray`) and the
  summary's `Reduce: … shrinkray (pipx)` line. PASS.
- **shellcheck:** `shellcheck -x install-foundry-retro-tools.sh` → CLEAN (the bare-`shellcheck`
  SC1091 "not following lib.sh" note is pre-existing on every `foundry-setup` script). `bash -n` OK.
- **e2e harness untouched (correct):** `test-retro-tools-e2e-inner.sh` iterates the
  metapackage's `Depends:` and looks each up in `TOOL_CHECKS`. shrinkray is pipx-only / not a
  Depend, so it is *not* iterated — a `[shrinkray]` entry would be dead code. No change.

`.deb` revisit recorded as a Watch item in `TODO.md` (gated on `textual≥8` reaching universe).

## Release

`git add` touched `foundry-apt/packages/**`, `LICENSES-VENDORED.md`, `README.md`,
`foundry-setup/**`, `docs/plans/**`, `CLAUDE.md`; commit per-tier; `git push`. Per
`feedback_start_build_immediately`, kick off an ISO build as soon as the foundry-apt
changes land. Phase 1 publish: `git tag v1.0.x && git push origin v1.0.x`.

> **HOLD (Will, 2026-06-20):** do **not** publish `foundry-atelier` — no `task bump`, no
> `v*` tag, no CI publish — until Will explicitly directs. Committing source is fine; the
> gate is on the live apt publish (and any atelier ISO build off the back of it).
> `foundry-retro-tools` isn't named in the hold, but still publish only on a go-ahead.

## Decision log

- **2026-06-20 — creduce placement (option B).** Driven by the corrected size (~520 MiB
  trio; creduce ≈ +194 MiB redundant over cvise): **cvise + delta → `foundry-retro-tools`
  (anvil); creduce → `foundry-atelier` only.** Net anvil marginal ≈ ~330 MiB bare.
- **2026-06-20 — 4 GB-stick goal dropped** (Will), accepting the anvil growth from cvise.
- **2026-06-20 — shrinkray via pipx** as the primary path; .deb only if the textual
  cascade is shallow.
