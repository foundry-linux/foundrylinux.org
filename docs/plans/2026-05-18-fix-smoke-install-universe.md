# Plan — Fix smoke-install: enable universe in Ubuntu 26.04 container

## Context

The `smoke-install` job in `.github/workflows/publish.yml` (in `foundry-linux/foundry-apt`) fails when trying to install `foundry-linux-retro-tools` because `vgmstream` (one of its `Depends:`) requires `libavcodec60 (>= 7:6.0)`, which lives in Ubuntu's `universe` repository. The bare `ubuntu:26.04` Docker container used by the CI job only has `main` (and `restricted`) enabled — `universe` is absent, so `libavcodec60` and several other deps are unresolvable.

The fix is to enable `universe` in the container before the `apt-get update` that resolves Foundry repo dependencies.

---

## Fix

**File:** `foundry-apt/.github/workflows/publish.yml` (monorepo copy, then synced to `foundry-linux/foundry-apt`)

In the `smoke-install` job's single step, add universe enablement **before** the `apt-get update` that reads the Foundry repo:

```yaml
      - name: Install from the freshly-published repo
        run: |
          apt-get update -q
          apt-get install -y curl gnupg ca-certificates software-properties-common
          add-apt-repository -y universe
          curl -fsSL https://apt.foundrylinux.org/key.gpg \
            | gpg --dearmor -o /etc/apt/keyrings/foundry.gpg
          echo "deb [signed-by=/etc/apt/keyrings/foundry.gpg] https://apt.foundrylinux.org resolute main" \
            > /etc/apt/sources.list.d/foundry.list
          apt-get update -q
          apt-cache show foundry-linux-retro-tools
          apt-get install -y --no-install-recommends --download-only foundry-linux-retro-tools
```

`software-properties-common` + `add-apt-repository -y universe` is the canonical Ubuntu way to enable universe; it works on both the old `sources.list` format and the DEB822 format used in 24.04+.

---

## Execution order

1. Edit `foundry-apt/.github/workflows/publish.yml` in the monorepo.
2. Commit and push to `foundrylinux.org` main.
3. Sync the change to `/tmp/foundry-apt-sync` (already cloned), commit, push.
4. Push tag `v0.0.28` on `foundry-linux/foundry-apt` to trigger the publish + smoke-install.
5. Watch the run — both `build-and-publish` and `smoke-install` jobs should go green.

---

## Critical files

| Path | Action |
|------|--------|
| `foundry-apt/.github/workflows/publish.yml` | Edit — add universe enablement to smoke-install step |
| `/tmp/foundry-apt-sync/` | Sync + tag to trigger CI |

---

## Verification

1. `gh run list --repo foundry-linux/foundry-apt --limit 2` → both jobs `success`
2. `smoke-install` job log shows `apt-cache show foundry-linux-retro-tools` and successful `--download-only` resolution
