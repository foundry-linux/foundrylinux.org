# Complete LosslessCut publication and upstream handoff

**Date:** 2026-08-30
**Scope:** Finish every non-ISO follow-up for the already-published `losslesscut`
package: public inventory deployment, release verification, project bookkeeping,
the upstream FFmpeg defect report, and Debian WNPP handoff.

---

## Outcome

`losslesscut 3.69.0-1foundry1` is already built, published by foundry-apt
`v1.5.47`, installed from the live repository, and launched successfully on an
Ubuntu 26.04 Wayland desktop. This plan closes the remaining publication loop
without building or publishing a new Foundry ISO.

Completion means:

1. the committed `wald3n.com/open-source` inventory update is versioned,
   deployed, and verified live;
2. the Foundry completion guard has verified both public surfaces;
3. the LosslessCut package plan and `TODO.md` record the production evidence;
4. the malformed bundled-FFmpeg RPATH is reported to `mifi/ffmpeg-builds`; and
5. upstream is notified of the Foundry package and the Debian WNPP RFP is left
   prepared locally; submitting it is excluded because this agent is not
   allowed to send email.

## Constraints

- Do not run `task iso-build`, `task iso-publish`, or otherwise start a new ISO
  build.
- Preserve unrelated dirty work in both `foundrylinux.org` and `wald3n.com`.
- Do not include unrelated files in release commits.
- Re-check live WNPP and upstream issue state immediately before submitting to
  avoid duplicates.
- Do not claim completion until the deployed pages and submitted external
  records are directly observable.

## Execution

### 1. Establish the release baseline

- Confirm foundry-apt `v1.5.47` is the green production run containing
  `losslesscut 3.69.0-1foundry1` and `foundry-sprite 1.0.2`.
- Confirm the local desktop install/launch evidence already recorded in the
  package plan.
- Confirm the wald3n inventory commits contain LosslessCut and are newer than
  the latest deployed/versioned tag.

### 2. Publish the public inventory

- Run the wald3n offline inventory verifier and a production build before
  release.
- Use the repository's normal `task publish` path, which bumps the patch
  version, commits only its declared release inputs, tags and pushes, then
  deploys through the local containerized production pipeline.
- Verify the new tag is on `origin/main` and that the live `/open-source` page
  contains the LosslessCut row.
- Run the Foundry `package-publish:complete` verification. If the historical
  pending marker is absent, record that honestly and independently verify APT,
  the checked-in snapshot, and the live page; do not manufacture a marker after
  publication merely to make the command print success.

### 2a. Restore durable one-command publishing

The first `task publish` attempt exposed an intermittent SSH handoff failure:
the container could read the deploy credentials but its initial `git fetch`
received `Permission denied (publickey)`, despite the host using the same loaded
key successfully. A later direct container probe saw the forwarded agent and
authenticated, so the mount/configuration is structurally correct but the
publisher currently treats one transient authentication failure as terminal.

- Make the publish preflight explicitly prove that the forwarded agent has an
  identity and can authenticate to GitHub in batch mode, producing a focused
  diagnostic when either condition fails.
- Retry the read-only remote fetch a small bounded number of times so a
  momentary agent/socket or GitHub SSH failure does not abort an otherwise safe
  release before mutation.
- Keep the release push fail-closed and single-shot: after a commit/tag exists,
  ambiguous automatic push retries are less desirable than reporting the exact
  state for reconciliation.
- Add a regression test using fake `ssh`, `ssh-add`, and `git` commands to prove
  missing-agent, failed-authentication, transient-fetch, and successful paths.
- Verify the real container can list the forwarded key and run
  `git ls-remote origin HEAD` before calling the publisher fixed.

### 3. Report the FFmpeg build defect upstream

- Search `mifi/ffmpeg-builds` open and closed issues for the malformed RPATH.
- If no duplicate exists, file a concise reproducible report showing the
  literal `$ORIGIN:-Wl:../lib` entry, its current-directory-relative loading
  risk, the over-escaped linker flags that produce it, and the verified
  `$ORIGIN`-only repair.
- Link the issue from the package plan.

### 4. Notify LosslessCut upstream and hand off to Debian

- Search LosslessCut discussions/issues for an existing Foundry/Debian
  packaging thread.
- Send a concise heads-up that Foundry redistributes the official AppImage
  payload, retains upstream's private FFmpeg, uses the upstream package name,
  and will forward relevant bugs.
- Re-check Debian WNPP for `losslesscut` and `lossless-cut`.
- Keep the prepared request as an RFP rather than an ITP: the current package is a
  binary repack and Debian has no supported Electron runtime or packaged npm
  dependency closure, so there is no honest commitment to a main-acceptable
  upload yet.
- Do not submit it by email. Record the clear duplicate check and the prepared
  draft so a human can file it later if desired.

### 5. Close project bookkeeping

- Update the LosslessCut package plan with the APT release, live-install proof,
  wald3n release, completion-guard result, upstream report, upstream heads-up,
  and Debian WNPP result.
- Move LosslessCut publication into `TODO.md` Done and retain only genuinely
  open follow-ups.
- Refresh `docs/plans/README.md` and the deferral ledger as required by the
  repository hooks.
- Run Markdown preview/lint and relevant repository checks, then commit only
  the files belonging to this closure.

## Verification checklist

- [x] `losslesscut 3.69.0-1foundry1` is present in the live APT index.
- [x] A clean Ubuntu 26.04 install from the live repository succeeds.
- [x] wald3n offline inventory verification passes.
- [x] wald3n production build and deployment pass (`v0.0.427`).
- [x] The containerized publisher's SSH preflight and bounded-fetch regression
      tests pass, and a real in-container `git ls-remote` succeeds.
- [x] The live `/open-source` page contains LosslessCut.
- [x] The publication guard is run and its historical missing-marker state is
      recorded accurately.
- [x] The FFmpeg RPATH report has a durable upstream URL:
      [mifi/ffmpeg-builds#1](https://github.com/mifi/ffmpeg-builds/issues/1).
- [x] The LosslessCut upstream heads-up has a durable URL:
      [mifi/lossless-cut#3035](https://github.com/mifi/lossless-cut/issues/3035).
- [x] Debian WNPP was re-checked with no match; the RFP remains prepared but
      unfiled because sending email is prohibited.
- [x] Project plan/TODO/index bookkeeping is current.
- [x] No ISO build or ISO publication command was run.

## Production record

- foundry-apt `v1.5.47` published `losslesscut 3.69.0-1foundry1` and
  `foundry-sprite 1.0.2`; the live Packages index was re-checked on 2026-08-30.
- wald3n `v0.0.426` first deployed the already-committed inventory. The durable
  publisher repair then landed as `d266ae6`, and the repaired one-command
  `task publish` created and deployed `v0.0.427` end to end.
- The live `/open-source` page contains `data-sort-name="losslesscut"` and its
  footer reports `v0.0.427`.
- `task package-publish:complete` reported the historical truth: no pending
  marker existed. No marker was manufactured after the fact; APT, snapshot,
  live page, release tag, and deployment were verified independently.
- Direct upstream handoffs are
  [ffmpeg-builds#1](https://github.com/mifi/ffmpeg-builds/issues/1) and
  [lossless-cut#3035](https://github.com/mifi/lossless-cut/issues/3035).
