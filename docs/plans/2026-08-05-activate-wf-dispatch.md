# Activate the WorldFoundry → foundrylinux.org cross-repo dispatch

**Date:** 2026-08-05
**Tier:** T2
**Closes:** `TODO.md` — *Activate `repository_dispatch` from worldfoundry.org*
**Follows:** [2026-05-21 packages page §6](2026-05-21-packages-page.md)
**Status:** ⚠️ **BLOCKED** — code change landed and verified; the leg cannot fire until
`FOUNDRYLINUX_DISPATCH_PAT` is re-minted (see [Remaining work](#remaining-work)). Until then the
nightly 03:00 UTC cron remains the only thing refreshing the packages page after a WorldFoundry
publish — a delay of up to 24 h, not an outage.

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
   [Remaining work](#remaining-work). Note this is the first evidence the leg was *never* going to
   work — had it not been made testable, this 401 would have surfaced mid-publish on the next
   `apt-v*` tag.

3. **The far end received it** — expect a `site-deploy` run with `event: repository_dispatch`.

   ```
   $ gh run list --repo foundry-linux/foundrylinux.org --workflow site-deploy.yml --limit 1
   30978492556 schedule 2026-08-05T05:33:49Z
   ```

   **FAIL (blocked by step 2)** — unchanged from the pre-test baseline, consistent with a 401: nothing
   was dispatched. Re-run after [Remaining work](#remaining-work).

4. **The rebuild is a real rebuild, not a crash** — blocked by step 2. Expect `build-packages-page.sh`
   to run and report its cache hit (`no change in either apt repo since last generation — skipping`),
   since no publish preceded the test dispatch; that skip is correct and is itself evidence the chain
   executed. `wrangler pages deploy` then succeeds.

5. **The production path still works** — pending the next real `apt-v*` publish (do not cut one solely
   for this). Expect `notify-foundrylinux` to appear as a nested reusable-workflow call and succeed,
   and the resulting `site-deploy` run to regenerate rather than cache-skip, because
   `apt.worldfoundry.org`'s `Release` sha will have changed.

---

## Remaining work

Re-mint the PAT. This was written as a contingency; verification step 2 turned it into the only thing
left. GitHub exposes no API for creating PATs, so it is irreducibly manual — everything downstream of
the token value is scripted.

1. Create a fine-grained PAT at
   [https://github.com/settings/personal-access-tokens/new](https://github.com/settings/personal-access-tokens/new):
   - **Token name:** `FOUNDRYLINUX_DISPATCH_PAT (wbniv/worldfoundry.org)` — cosmetic to the API, but
     it is what the expiry-warning email shows. Naming it after the **secret + holding repo** makes
     that email state its own remedy; naming it after the *target* (`foundrylinux.org dispatch`)
     points at where the token aims rather than where you must go to replace it.
   - **Resource owner:** `foundry-linux` (the org — *not* `wbniv`)
   - **Repository access:** Only select repositories → `foundry-linux/foundrylinux.org`
   - **Repository permissions → Contents: Read and write** (Metadata: Read is implicit).
     **Not** Actions — `POST /dispatches` is gated on Contents.
   - **Expiration:** 1 year; record the date below so the next expiry is anticipated rather than
     discovered mid-publish.
2. If the org requires approval for fine-grained PATs, approve it in the org's settings.
3. Store it with the project's own credential path — **not** a bare `gh secret set`, and never by
   pasting the value into a chat transcript:

   ```bash
   task secret-set NAME=FOUNDRYLINUX_DISPATCH_PAT REPO=wbniv/worldfoundry.org
   ```

   That wraps [`scripts/backup-secret.sh`](../../scripts/backup-secret.sh), which reads the value from
   a hidden `/dev/tty` prompt (so it stays out of shell history, the process list, and any transcript),
   PUTs it to the private `foundry-linux-secrets` R2 bucket, **reads it back and compares sha256**, then
   mirrors it to the GitHub Actions secret via stdin rather than argv. A bare `gh secret set` would set
   the secret but skip the disaster-recovery copy, leaving the token single-homed — the thing
   [the infra mandate](../../CLAUDE.md) exists to prevent.

   Prerequisite: `CF_API_TOKEN` + `CF_ACCOUNT_ID`, from the environment or `.foundry/bootstrap.env`.
   That cache is absent on a fresh checkout (it is gitignored); `bash scripts/bootstrap.sh` is
   idempotent and will re-populate it.
4. Re-run verification steps 2–4.

**PAT expiry:** _(record on re-mint)_
