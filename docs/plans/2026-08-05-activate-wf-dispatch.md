# Activate the WorldFoundry → foundrylinux.org cross-repo dispatch

**Date:** 2026-08-05
**Tier:** T2
**Closes:** `TODO.md` — *Activate `repository_dispatch` from worldfoundry.org*
**Follows:** [2026-05-21 packages page §6](2026-05-21-packages-page.md)

---

## Context

When `apt.worldfoundry.org` publishes packages, `foundrylinux.org`'s packages page should rebuild —
that page renders **both** apt repos (`scripts/build-packages-page.sh:33` fetches
[apt.worldfoundry.org/dists/stable/Release](https://apt.worldfoundry.org/dists/stable/Release)), so a
WorldFoundry publish genuinely changes it.

**The wiring already existed on both ends.** This was never a "wire it up" task:

| Piece | State on 2026-08-05 |
|---|---|
| `worldfoundry.org/.github/workflows/apt-publish.yml` `notify-foundrylinux` job | committed + pushed (`540d5c3`, after the 2026‑05‑22 revert) |
| `foundrylinux.org/.github/workflows/site-deploy.yml:13` `repository_dispatch: [apt-published]` | present |
| `FOUNDRYLINUX_DISPATCH_PAT` secret on `wbniv/worldfoundry.org` | set 2026‑05‑30 |

What was missing was **a single successful firing**. `apt-publish.yml` had not run since 2026‑05‑21 —
before the notify job landed — and `site-deploy.yml`'s last 25 runs were all `schedule`.

**Why the item stalled for two months:** the job fires only on `github.event_name == 'push'` of an
`apt-v*` tag, so the only way to test it was a full 15‑package build + sign + publish + smoke run. The
cost of checking exceeded the cost of ignoring it. Fixing that feedback loop is the substance of this
change; proving the leg works is then nearly free, and stays free every time the PAT rotates.

## Change

Extract the dispatch into `worldfoundry.org/.github/workflows/notify-foundrylinux.yml`, carrying both
`workflow_call` (for `apt-publish.yml`) and `workflow_dispatch` (for humans). `apt-publish.yml`'s job
becomes a `uses:` call with `secrets: inherit`, keeping its `needs:`/`if:` unchanged.

Behaviour preserved: an **absent** secret still no-ops with a warning rather than reddening the
publish (the reason for the 2026‑05‑22 revert). Behaviour improved: a secret that is **present but
rejected** now reports its HTTP status and the specific remedy, instead of a bare `curl` exit 22.

Landed as `wbniv/worldfoundry.org@b1a3a83`.

## Out of scope — `wald3n.com/open-source`

The standing note *"package publication includes wald3n.com/open-source"* is correct as a **release
checklist** item but cannot be automated by this dispatch, and not merely for effort reasons:

- `wbniv/wald3n.com/.github/workflows/deploy.yml` has its `push: tags:` trigger **commented out**
  (Actions minutes exhausted); deploys run locally via `task publish` / `scripts/redeploy.sh`.
- `scripts/refresh-open-source-data.mjs` reads a **local sibling clone** (`../foundry-apt/packages`,
  per `scripts/open-source-sources.json`) plus `apt-cache show` — **zero** HTTP fetches of either apt
  repo. A CI runner has neither.

A dispatch would land on a disabled workflow that could not compute the data. Re-enabling it and
repointing the refresh at the HTTP `Packages` indexes deserves its own plan.

## No visible surface

CI plumbing only. `foundrylinux.org/packages` is unchanged by this work — only *when* it rebuilds changes.

---

## Verification

1. **Reusable-workflow syntax is valid** — GitHub rejects malformed reusable calls at parse time.

   ```
   $ gh workflow list --repo wbniv/worldfoundry.org
   Build, sign, and publish APT repo	active	279039577
   Deploy	active	276785480
   Notify foundrylinux.org	active	328267523
   ```

   **PASS** — new workflow registered; `apt-publish.yml` still active, so its `uses:` call parsed.

2. **Fire the leg by hand** — the whole point of the extraction.

   ```
   $ gh workflow run notify-foundrylinux.yml --repo wbniv/worldfoundry.org
   $ gh run list --repo wbniv/worldfoundry.org --workflow notify-foundrylinux.yml --limit 1
   31063977817 workflow_dispatch completed failure

   $ gh run view 31063977817 --repo wbniv/worldfoundry.org
   X POST /repos/foundry-linux/foundrylinux.org/dispatches
   ##[error]dispatch failed with HTTP 401
     "message": "Bad credentials",
   ::error::PAT is invalid or expired — mint a new fine-grained PAT and re-run
     'gh secret set FOUNDRYLINUX_DISPATCH_PAT'.
   ```

   **FAIL** — the 2026‑05‑30 `FOUNDRYLINUX_DISPATCH_PAT` is expired or revoked. Not a wiring fault:
   the request was well-formed and reached GitHub, which rejected the credential. Proceed to
   Contingency. Note this is the first evidence the leg was *never* going to work — had it not been
   made testable, this 401 would have surfaced mid-publish on the next `apt-v*` tag.

3. **The far end received it** — expect a `site-deploy` run with `event: repository_dispatch`.

   ```
   $ gh run list --repo foundry-linux/foundrylinux.org --workflow site-deploy.yml --limit 1
   30978492556 schedule 2026-08-05T05:33:49Z
   ```

   **FAIL (blocked by step 2)** — unchanged from the pre-test baseline, consistent with a 401: nothing
   was dispatched. Re-run after Contingency.

4. **The rebuild is a real rebuild, not a crash** — blocked by step 2. Expect `build-packages-page.sh`
   to run and report its cache hit (`no change in either apt repo since last generation — skipping`),
   since no publish preceded the test dispatch; that skip is correct and is itself evidence the chain
   executed. `wrangler pages deploy` then succeeds.

5. **The production path still works** — pending the next real `apt-v*` publish (do not cut one solely
   for this). Expect `notify-foundrylinux` to appear as a nested reusable-workflow call and succeed,
   and the resulting `site-deploy` run to regenerate rather than cache-skip, because
   `apt.worldfoundry.org`'s `Release` sha will have changed.

### Contingency — re-mint the PAT (the one step with no CLI path)

GitHub exposes no API for creating PATs, so this is irreducibly manual — everything downstream of the
token value is scripted.

1. Create a fine-grained PAT at
   [https://github.com/settings/personal-access-tokens/new](https://github.com/settings/personal-access-tokens/new):
   - **Resource owner:** `foundry-linux` (the org — *not* `wbniv`)
   - **Repository access:** Only select repositories → `foundry-linux/foundrylinux.org`
   - **Repository permissions → Contents: Read and write** (Metadata: Read is implicit).
     **Not** Actions — `POST /dispatches` is gated on Contents.
   - **Expiration:** 1 year; record the date below so the next expiry is anticipated rather than
     discovered mid-publish.
2. If the org requires approval for fine-grained PATs, approve it in the org's settings.
3. ```bash
   gh secret set FOUNDRYLINUX_DISPATCH_PAT --repo wbniv/worldfoundry.org
   ```
4. Re-run verification steps 2–4.

**PAT expiry:** _(record on re-mint)_
