# Plan: confirm the 5 Repology-badge `none` opt-outs in foundry-apt

## Context

The Repology-badge audit tooling ([`2026-06-21-repology-badge-audit-tooling.md`](2026-06-21-repology-badge-audit-tooling.md))
backfilled every vendored package's `X-Repology-Project` field in one sweep (commit
`d59d9f2`), defaulting 5 packages to `none` (the opt-out value). Those 5 were never
individually verified against Repology — so the parent TODO carries an open sub-task:

> (1) Review the remaining 5 `none` opt-outs (`blender-asset-finder{,-cli}`, `drmon`,
> `m8te`, `foundry-welcome`) — set a real `X-Repology-Project` for any that ARE tracked on
> Repology.

This surfaced when Will noticed `m8te` alone has no badge / no "latest packaged version"
row on [apt.foundrylinux.org](https://apt.foundrylinux.org). It isn't a bug — `none` makes
`generate-index.sh` skip the badge **and** the version lookup (the "latest packaged version"
number *comes from* Repology). The question is whether each `none` is *correct*.

This plan closes part (1): verify each of the 5, record evidence, annotate each opt-out with
a one-line reason so future audits don't re-raise it. Part (2) of the parent TODO
(`apt.worldfoundry.org` backfill) stays **out of scope**.

## The 5 packages and expected verdict

| Package | Upstream | Why we expect `none` |
|---|---|---|
| `m8te` | [nesdoug/M8TE](https://github.com/nesdoug/M8TE) | niche GitHub-only Mono/.NET tile editor; not in any distro Repology scans — **already confirmed not tracked** |
| `blender-asset-finder` | [wbniv/WorldFoundry](https://github.com/wbniv/WorldFoundry) | WorldFoundry-authored Blender add-on; not in any distro |
| `blender-asset-finder-cli` | [wbniv/WorldFoundry](https://github.com/wbniv/WorldFoundry) | same WF source, CLI companion |
| `drmon` | [developer-resources-co/drdevtools](https://github.com/developer-resources-co/drdevtools) | niche third-party SNES/Genesis debugger — **verify** (most likely candidate to actually be tracked) |
| `foundry-welcome` | [foundrylinux.org](https://foundrylinux.org) | **our own** distro welcome app — not an upstream, definitionally not on Repology |

If any turns out to be genuinely tracked on Repology, set its real project name via
`task set-badge PKG=<pkg> PROJECT=<repology-project>` instead of annotating it `none`.

## Method

`curl` to the Repology API is **403-blocked** from this host (rate-limit / UA block —
confirmed: even the known-tracked `halfempty` 403s). Use **WebFetch** against
[`repology.org/projects/?search=<term>`](https://repology.org/projects/) (server-side fetch,
not blocked) for each package. For any non-empty match, confirm it's *our* tool (matching
description/upstream), not a name collision — Repology normalizes names, so e.g. an unrelated
"drmon" could exist.

## Changes

1. **Annotate each of the 5** `debian/control` files with a comment directly above
   `X-Repology-Project: none`, e.g.
   `# not on Repology: <one-line reason>` — so `check-repology-badges.sh` / a future audit
   reads the rationale inline.
2. **TODO.md** — strike part (1) of the badge TODO as done (5 opt-outs confirmed), leaving
   part (2) (`apt.worldfoundry.org`) open. Point it at this plan.
3. **This plan** — paste the WebFetch evidence under each verification step (PASS/FAIL).

## Verification

1. **Each of the 5 is absent from Repology** (or, if present, is a name collision / different
   tool — not ours). WebFetch `repology.org/projects/?search=<pkg>` for `m8te`,
   `blender-asset-finder`, `drmon`, `foundry-welcome`; record the result per package.
2. **Annotations land** — `grep -B1 'X-Repology-Project: none' packages/*/debian/control`
   shows a `# not on Repology…` comment above all 5 `none` values, and only those.
3. **Audit still green** — `task check-badges` (a.k.a. `check-repology-badges.sh`) passes:
   all packages declare the field, 5 `none` / rest real, 0 missing. No package was newly
   flagged or had its `none` flipped without a `set-badge` call.
4. **No false badge added** — `git diff` shows no `X-Repology-Project:` *value* changed
   (only comments added); confirms we didn't graft a dead/404 project onto a genuinely
   untracked package.

## Out of scope

- Part (2) of the parent TODO: backfilling the 10 `apt.worldfoundry.org` CLIs and wiring
  `check-repology-badges.sh` into that repo — separate repo, separate pass.
- Re-verifying the 22 packages that already carry a real `X-Repology-Project` (the audit
  reports them green).

## Execution log — 2026-06-21 ✅ done (all 5 stay `none`)

**Step 1 — each of the 5 absent from Repology.** WebFetch against
[`repology.org/projects/?search=<term>`](https://repology.org/projects/) (curl to the JSON
API 403s from this host — confirmed even for the known-tracked `halfempty`):

| Package | Search result |
|---|---|
| `m8te` | "No projects found" → NO MATCHES |
| `blender-asset-finder` (covers `-cli`, same `wbniv/WorldFoundry` source) | NO MATCHES |
| `drmon` | NO MATCHES (no `drmon`/`drdevtools` project — not even a name collision) |
| `foundry-welcome` | NO MATCHES (our own app, expected) |

All 5 genuinely untracked → `none` is correct for every one; no `set-badge` needed. **PASS.**

**Step 2 — annotations land.** `grep -B1 '^X-Repology-Project: none' packages/*/debian/control`
shows a `# not on Repology: …` comment above all 5 `none` values, and only those. **PASS.**

**Step 3 — audit still green.** `task check-badges` →
`PASS: all vendored packages declare X-Repology-Project (22 badged, 5 opt-out)`. **PASS.**

**Step 4 — no false badge added.** `git diff` on the control files shows **no
`X-Repology-Project:` value line changed** — pure comment additions. **PASS.**

**Bonus — `generate-meta.sh` parser hardened.** A `#` line in `debian/control` is valid
deb822, but our hand-rolled `parse_control` would have turned it into a junk
`"# not on Repology…"` stanza key (harmless, but wrong). Added
`if line.startswith("#"): continue`. Proven: `generate-meta.sh m8te` → `repology_project:
"none"`, `generate-meta.sh halfempty` → `repology_project: "halfempty"`, zero `#` keys in
either emitted JSON. (Per Will: "if it does, fix our own script!")

Part (2) of the parent TODO (`apt.worldfoundry.org` backfill) remains open — separate repo.
