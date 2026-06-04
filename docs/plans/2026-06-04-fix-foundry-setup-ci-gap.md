# Fix the foundry-setup CI gap — move the misplaced workflow to the monorepo root

## Context

The 2026-05-29 state-of-the-distro audit (and my 06-04 refresh) flagged: `foundry-linux/foundry-setup` child repo **does not exist** and there's no `setup-sync` task, so `foundry-setup/.github/workflows/test.yml` — which the audit's §8 census lists as live CI — actually **runs nowhere**. GitHub Actions only executes workflows at `<repo-root>/.github/workflows/`; a file at `foundry-setup/.github/workflows/test.yml` (a subdirectory of the private monorepo) is ignored.

My earlier framing ("creating a public repo is your call") was wrong. Two Explore passes proved the workflow was **authored to run from the monorepo root** and was simply placed in the wrong directory:

1. Its `paths:` filter already lists `'.github/workflows/foundry-setup-test.yml'` — the exact root path it *should* live at (a dead self-reference today). Evidence: `foundry-setup/.github/workflows/test.yml:7`.
2. Every job does `cd foundry-setup` / `working-directory: foundry-setup` — which only works when `foundry-setup/` is a **subdir** (monorepo layout) and would **break** in a standalone child repo where those files sit at the root. Evidence: lines 20, 29, 32, 36, 54, 57.

Creating a child repo is the wrong model anyway: the existing child repos (`foundry-apt`, `foundry-iso`, `foundry-devbox`) exist to run **tag-triggered publishes of an artifact** (apt repo → R2, ISO → R2, image → GHCR). `foundry-setup` has **no artifact to publish** — only tests (shellcheck + dry-run + full-install). It belongs in the monorepo, tested in the monorepo.

**Intended outcome:** Phase 0's tests actually run again (they caught the `--skip-clone` regression that was finding #1), with no new public repo and no new sync machinery.

## Approach (the recommended fix — Option A)

### 1. Move the workflow to the monorepo root — no content changes
```
git mv foundry-setup/.github/workflows/test.yml .github/workflows/foundry-setup-test.yml
```
The file already works verbatim at this location:
- `paths:` filter (`foundry-setup/**` + `.github/workflows/foundry-setup-test.yml`) is already correct for root — the previously-dead self-reference becomes accurate.
- All jobs `cd foundry-setup`, so the monorepo checkout (full tree) resolves correctly.
- `actions/checkout@v6` (already Node-24, bumped earlier this session).

Then remove the now-empty `foundry-setup/.github/` tree (git mv leaves the dirs empty/untracked).

### 2. Reconcile the docs the gap touched

- **`docs/investigations/2026-05-29-state-of-the-distro.md`**
  - §2 mirror-out diagram (~L55–60): drop the `foundry-setup/ ──(task sync)──▶ foundry-linux/foundry-setup` line; foundry-setup does **not** mirror out — its CI runs in the monorepo root (it has no artifact).
  - The "New gap found 06-04" note in the §1 refresh banner: flip to **resolved** — workflow moved to `.github/workflows/foundry-setup-test.yml`; foundry-setup intentionally stays in-monorepo (no publishable artifact, so no child repo / no `setup-sync`).
  - §8 workflow census row for `foundry-setup/test.yml`: update path → `.github/workflows/foundry-setup-test.yml`, venue → monorepo root (private), and note the dead `paths:` ref is now correct.
  - §3 mention of the "dead `paths:` filter referencing a non-existent `foundry-setup-test.yml`": the file now lives at that path → no longer dead; update.

- **`CLAUDE.md`** (root project): check the Architecture section for any claim that foundry-setup mirrors out / has a child repo; correct if present. (The Phase 0 script listing itself is fine.)

### 3. Track the *separate* distribution gap (do NOT solve here)

Exploration surfaced a second, distinct gap: `foundry-setup/README.md` tells users to run `curl -fsSL https://foundrylinux.org/install.sh | bash`, but **`site/install.sh` does not exist** (the site only serves `site/setup.sh`, the apt-source wiring script), and `install.sh` isn't self-contained anyway — it's a local dispatcher that sources `lib.sh` + sibling `install-foundry-*.sh`. So Phase 0's full installer has no working public curl-bash path. This needs a product decision (serve a self-contained installer? document git-clone/tarball? rely on `setup.sh` + apt only?) and is out of scope for the CI fix.

**Action:** add a `TODO.md` item under "Housekeeping" (or "Site") capturing it, referencing this finding. Leave the README/site untouched in this pass.

## Critical files

- `foundry-setup/.github/workflows/test.yml` → **move** to `.github/workflows/foundry-setup-test.yml` (no edits)
- `docs/investigations/2026-05-29-state-of-the-distro.md` — reconcile §1 gap note, §2 diagram, §3, §8 census
- `CLAUDE.md` — check/fix any foundry-setup mirror-out claim
- `TODO.md` — add the distribution-gap follow-up item

## Notes / trade-offs

- **GH Actions minutes:** the workflow now runs in the *private* monorepo (consumes minutes), but it's gated by `paths: foundry-setup/**`, and the slow `full-install` job is already `if`-gated to `main`/tags. site-deploy.yml already runs here, so this matches the established model. The open "flip monorepo public" TODO makes it free when actioned.
- **No `Taskfile.yml` change** — deliberately. No `setup-sync` task is added because nothing mirrors out.

## Verification

1. **Placement & validity**
   ```
   test -f .github/workflows/foundry-setup-test.yml && echo MOVED
   test ! -e foundry-setup/.github && echo OLD_DIR_GONE
   python3 -c "import yaml; yaml.safe_load(open('.github/workflows/foundry-setup-test.yml')); print('YAML_OK')"
   ```
   Expect `MOVED`, `OLD_DIR_GONE`, `YAML_OK`.

2. **GitHub recognizes it** — after pushing to a branch, confirm the workflow appears:
   ```
   gh workflow list --repo foundry-linux/foundrylinux.org | grep -i "install.sh tests"
   ```
   (Or trigger by pushing a trivial change under `foundry-setup/` and watch `gh run list`.)

3. **Census is clean** — no workflow remains outside a root `.github/workflows/`:
   ```
   find . -path ./node_modules -prune -o -name '*.yml' -path '*/.github/workflows/*' -print \
     | grep -v '^\./\.github/workflows/' | grep -v node_modules
   ```
   Expect: only the genuine child-repo workflows under `foundry-apt/`, `foundry-iso/`, `foundry-devbox/` (those DO mirror out to their own roots) — and **no** `foundry-setup/.github/...`.

4. **Docs reconciled** — `grep -n "foundry-setup" docs/investigations/2026-05-29-state-of-the-distro.md` shows the gap marked resolved and the mirror diagram no longer lists foundry-setup.
