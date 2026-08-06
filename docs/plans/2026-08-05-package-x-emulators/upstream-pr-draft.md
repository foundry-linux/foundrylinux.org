# Upstream PR draft — `SOURCE_DATE_EPOCH` reproducibility fix

**Status: OPENED as [lgblgblgb/xemu#448](https://github.com/lgblgblgb/xemu/pull/448) on
2026-08-05.** The PR is intentionally left untouched pending user review.

## Apply check against current upstream master (2026-08-05)

```
$ git clone --depth 50 https://github.com/lgblgblgb/xemu.git xemu-master-check
$ cd xemu-master-check && git log -1 --format='%H %cd' --date=short
40dfef0d1d5f56be2469492715c12bdb32c75b67 2026-01-29

$ git merge-base --is-ancestor 40dfef0d1d5f56be2469492715c12bdb32c75b67 HEAD && echo YES
YES

$ git apply --check --verbose \
    ../../foundrylinux.org/foundry-apt/packages/x-emulators/debian/patches/0001-honour-SOURCE_DATE_EPOCH.patch
Checking patch build/Makefile.common...
(exit 0 — clean apply)
```

**Result: the patch applies cleanly.** Upstream master has not moved since the commit we pinned
(`40dfef0d`, 2026-01-29) — it is current `HEAD`, not just an ancestor — so there is zero drift to
reconcile before opening the PR. (`git am` itself errored only on the sandbox's missing
`user.name`/`user.email` git config, not on the patch content — `git apply --check` is the
authoritative "does this still apply" signal here and it passed.)

## Debian patch metadata

`debian/patches/0001-honour-SOURCE_DATE_EPOCH.patch` now records
`Forwarded: https://github.com/lgblgblgb/xemu/pull/448`.

---

## Draft PR

**Title:**

```
build: honour SOURCE_DATE_EPOCH for the embedded build date
```

**Body:**

```markdown
## What

`build/Makefile.common` embeds a build timestamp (`build/objs/cdate.data`) into every binary via
`XEMU_BUILDINFO_CDATE`. Today that timestamp comes from `git log -1 --format=%cd`, falling back to
the current wall-clock `date` when the source tree is not a git checkout — which is exactly the
case for a release tarball or any downstream distribution package (Debian, Arch, Homebrew, ...).

This patch makes the date resolution prefer `SOURCE_DATE_EPOCH` when the environment sets it,
before falling back to the existing `git log` / `date` behavior, which is left completely
untouched for normal developer builds.

## Why

`SOURCE_DATE_EPOCH` is the cross-distribution convention for reproducible builds
(https://reproducible-builds.org/specs/source-date-epoch/), understood by dpkg-buildpackage,
rpmbuild, Nix, Guix, and most language build tools. Without it, two builds of byte-identical
source produce different output, because the embedded date falls back to "whatever wall-clock
time the packaging pipeline happened to run at" rather than anything derived from the source
itself. That breaks:

- **Bit-for-bit reproducible builds** — Debian's reproducible-builds effort (and any downstream
  that verifies build reproducibility, e.g. against a second independent build) cannot verify
  Xemu binaries built from a tarball, because the two builds' `cdate.data` differ even though
  every other input is identical.
- **Content-addressed build caches** (Nix, Guix, some CI cache layers) — a non-deterministic
  embedded timestamp means the output hash changes on every rebuild even when nothing changed,
  defeating the cache.

This is exactly the class of fix the Reproducible Builds project recommends: prefer
`SOURCE_DATE_EPOCH`, silently fall back to existing behavior when it's unset, so nothing changes
for anyone not already opting in.

## What changes

One line in `build/Makefile.common`'s build-info generation: if `SOURCE_DATE_EPOCH` is set,
format it (UTC) into the same `%Y%m%d%H%M%S` shape `cdate.data` already uses; otherwise fall
through to the existing `git log` → `date` chain unchanged.

## Testing

Built and ran this patch as part of packaging Xemu for a downstream Debian-based distribution
(Foundry Linux, apt.foundrylinux.org). With `SOURCE_DATE_EPOCH` exported (as `dpkg-buildpackage`
does automatically from `debian/changelog`'s date), the embedded `XEMU_BUILDINFO_CDATE` now
matches the changelog date exactly, and two builds of the same source + same
`SOURCE_DATE_EPOCH` produce byte-identical `cdate.data`. Without `SOURCE_DATE_EPOCH` set, behavior
is unchanged — verified the `git log` and no-git fallbacks both still fire as before.

No functional change to any emulator target; this only affects the diagnostic build-info string
each binary reports (e.g. via `-h`/`--version`).
```

## Notes for whoever sends this

- The patch itself lives at
  `foundry-apt/packages/x-emulators/debian/patches/0001-honour-SOURCE_DATE_EPOCH.patch` in this
  repo — it's a standard unified diff against `build/Makefile.common`, so it can be applied
  directly to a fork with `git apply` or `patch -p1`.
- Suggested flow: fork `lgblgblgb/xemu`, branch off current `master` (already == our pinned
  commit, so no rebase needed), `git apply` the patch, commit with a message matching the title
  above, push, open the PR with the body above.
- Upstream has no CONTRIBUTING.md or PR template as of this check, so no additional
  process/checklist to fill in beyond the description.
