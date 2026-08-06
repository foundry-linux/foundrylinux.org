# Audit vendored upstreams for their own Debian packaging

**Date:** 2026-08-05
**Scope:** Close a systematic gap in the `/package` skill, and re-check the 30 packages vendored before
the fix existed.

**No visible surface** — a skill step, an audit script, and its findings. No UI, page, or rendered output,
so no mockup bundle.

---

## What happened

Packaging [`x-emulators`](2026-08-05-package-x-emulators.md) exposed a blind spot. The `/package` skill's
**Step 1** is a blocking universe check that asks:

> *Is this already **in** Debian/Ubuntu?*

For LGB's Xemu the answer was no, correctly, so packaging proceeded. But nothing ever asked the adjacent
and different question:

> *Does **upstream** already package it themselves?*

For Xemu the answer was **yes** — `build/deb-build-simple.sh`, upstream's own binary `.deb` builder, which
sets `PROJECT="xemu"`, installs to `/usr/bin` + `/usr/share/xemu`, names its binaries `xemu-*`, and
declares its own dependency list. It is not *in* Debian, which is exactly why Step 1 sailed past it.

The consequence was not hypothetical. We chose the source/binary name **`x-emulators`** — to avoid
colliding with the unrelated Xbox `xemu` — without knowing upstream already calls his own `.deb` `xemu`.
That surfaced only at ITP time, long after the name had shipped and been published to
apt.foundrylinux.org. The naming decision still stands on its merits, but it was made against a fact we
should have had, not with it. We also diverged on binary naming (`xc65` vs upstream's `xemu-*`) without
ever noticing there was a divergence to reason about.

**Root cause:** "in the archive" and "upstream packages it" are two different questions, and the skill
only asked the first.

## What has already been done

| Change | Commit |
|---|---|
| **Step 2.5 — Check for upstream's own Debian packaging (BEFORE `dh_make`)** added to the skill, placed before `dh_make` because that is when the answer still changes decisions — and because `dh_make` refuses to run over an existing `debian/` | `bf3ebdf` (home repo) |
| Step 6's upstream heads-up now references Step 2.5 rather than duplicating the detection commands | `bf3ebdf` |
| Verification checklist gains the matching item | `bf3ebdf` |
| Retroactive Step 2.5 findings for x-emulators recorded | `b3dbc76` |
| Upstream heads-up made SOP for every ITP draft | `893c420`, `1aaae0d` |

Step 2.5 detects: an upstream `debian/` dir, their own `.deb`/`.dsc` build scripts, `PKGBUILD`/`.spec`
files, and PPA/Launchpad/OBS/COPR references — with a table of what to *do* with each, since the point is
to mine them for package name, install paths and dependencies rather than merely notice them.

## What is left: the other 30

Every vendored package in `foundry-apt/packages/*/` with a `build.sh` went through the skill **before**
Step 2.5 existed, so none of them was checked. That is the remaining exposure.

### Partial data already gathered

A GitHub API probe over the ~14 GitHub-hosted upstreams found **no root `debian/` directory** in any of
them. Useful, but it only clears the most obvious signal, and it is the signal x-emulators would have
*passed*: its script is at `build/deb-build-simple.sh`, not `debian/`. The code-search query that would
find that pattern hit GitHub's rate limit partway through.

**Conclusion: an API-only audit is not sufficient.** The check has to run against the actual source trees.

### Audit design

Every `packages/<name>/build.sh` already knows how to fetch its upstream and pins a sha256, so the audit
can reuse that rather than reinvent it. Add `foundry-apt/scripts/audit-upstream-packaging.sh`:

1. For each `packages/*/build.sh`, extract `UPSTREAM_URL` + `SHA256` (they follow a consistent shape) and
   fetch to a cache dir, skipping anything already cached.
2. Extract, then run the Step 2.5 signal set over the tree.
3. Emit a table: package | upstream `debian/`? | own `.deb` script? | `PKGBUILD`/`.spec`? | distro refs.
4. Compare findings with a committed baseline and exit non-zero on drift. `--strict` retains the original
   "any signal fails" mode; `--inventory-only` cheaply catches package/baseline coverage gaps.

Prebuilt-binary packages (`ghidra`, `ldtk`, `mesen2`, `pvsneslib`, `m8te`) are lower value — there is no
upstream source tree to inspect in the same way — but should still be swept for packaging metadata inside
the zip.

### What to do with findings

Per package, in increasing order of cost:

- **Nothing found** → record the negative in the package's ITP draft. That is a real result: it means the
  ITP heads-up can say "no existing packaging effort found" honestly.
- **Dependency list found** → cross-check against our `${shlibs:Depends}` resolution. For x-emulators
  upstream's declared deps matched ours exactly, which was a useful independent confirmation.
- **Divergent package or binary names** → do not silently "fix". Record the divergence and the reason as a
  decision, in `debian/changelog` and the ITP draft. It resurfaces at ITP time and in any upstream
  conversation.
- **An upstream `debian/` dir** → read it before trusting our generated one; it may encode packaging
  knowledge we inferred worse.
- **An existing distro effort (PPA/OBS/Debian ITP)** → stop and reassess before duplicating maintained
  work. This is the outcome that would actually change what we ship.

## Expected yield

Honestly: **probably low, but not zero.** Most of these upstreams are small tools with no packaging story,
which is why we vendored them. The value is threefold and mostly not the hit rate:

1. It closes a systematic gap rather than the one instance we tripped over.
2. Negative results are worth having — they make the ITP heads-ups accurate instead of assumed.
3. Any *positive* result is high-consequence: a duplicated maintained packaging effort is exactly the
   "stepping on toes" outcome the ITP process is meant to avoid.

## Verification

1. `foundry-apt/scripts/audit-upstream-packaging.sh` runs clean over all 30 packages and emits the table.
2. Its findings for `x-emulators` reproduce the known ground truth: no upstream `debian/`, one
   `.deb` script at `build/deb-build-simple.sh`. If the script does not find that, it is not working.
3. Every package's ITP draft records the audit result, positive or negative.
4. Any divergence found is recorded as a decision, not silently reconciled.

## Audit result — 2026-08-05

Ran the full sweep over all 30 `build.sh` entries (equivalent to the current `--strict` mode). Exit status
was **1, as designed, because signals were found** (status 2 is reserved for audit/fetch errors). All
pinned checksummed downloads verified. Those results are now the committed baseline, so the default mode
returns 0 only when a later run matches them. `foundry-welcome` is the sole native package and therefore
has no upstream payload to inspect.

| Package(s) | Result |
|---|---|
| asar-snes-assembler, blender-asset-finder, blender-asset-finder-cli, bsnes-jg, drmon, f9dasm, flycast, ghidra, halfempty, ldtk, libvgm, m8te, mesen2, ppsspp, pvsneslib, python3-glfw, python3-inators, python3-librosa, python3-mss, python3-picire, python3-pydub, ruff, snes9x-gtk, tilemap-studio, vgmstream, wla-dx | **Negative:** no upstream `debian/`, own `.deb`/`.dsc` builder, `PKGBUILD`/`.spec`, or packaging-service reference found. |
| foundry-welcome | **N/A:** native Foundry package, not a vendored upstream. |
| rpcs3 | **Dependency references only:** no upstream packaging files; `BUILDING.md` references Ubuntu toolchain and Vulkan SDK PPAs for build dependencies, not an RPCS3 packaging effort. |
| xemu-xbox | **Positive:** upstream ships `debian/` and `debian/build_deb.sh`, naming its source and binary package `xemu`. The package plan had already inspected this stale QEMU-derived packaging and records why we replace it; its name independently supports our `xemu-xbox` disambiguation. |
| xemu | **Positive:** no upstream `debian/`, but `build/deb-build-simple.sh` builds a binary package named `xemu`, reproducing the known ground truth that prompted this audit. |

All 26 centralized ITP drafts now carry their individual positive or negative result. Flycast, RPCS3,
and xemu-xbox were packaged after that draft batch and have no files under `docs/itp-drafts/`; their
results are retained in this table (xemu-xbox's package plan also already contains the full analysis).

## Follow-ups

Completed: the baseline-aware audit is wired into CI. Every run checks that all `build.sh` entries have
exactly one baseline row, audits changed package pins, and rechecks `xemu` as the small known-positive
control.
- [ ] `x-emulators`: decide whether the binary-name divergence (`xc65` vs upstream `xemu-*`) is worth
      raising in the upstream heads-up. Current position is to keep bare names — they are what upstream's
      own build, docs and `-h` output use — but say so rather than let him notice.
