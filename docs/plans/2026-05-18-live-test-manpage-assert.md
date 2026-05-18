# Plan: live-test asserts man pages install and render

**Date:** 2026-05-18
**Status:** Done (test wired; f9dasm rebuilt as 1foundry3 to fix the real WARNING the assertion caught; live repo will pass once republished)

## Context

The live install test ([`foundry-apt/test/run-test.sh`](../../foundry-apt/test/run-test.sh)) currently confirms only that `apt-get install <pkg>` succeeds against `apt.foundrylinux.org`. It runs inside `ubuntu:26.04`, whose minbase image ships [`/etc/dpkg/dpkg.cfg.d/excludes`](https://github.com/tianon/docker-brew-ubuntu-core/blob/master/excludes) with `path-exclude=/usr/share/man/*` and `path-exclude=/usr/share/doc/*`. That means any man pages our `.deb`s install are silently dropped at install time — the test never sees them.

Two consequences:

1. **f9dasm regression risk.** f9dasm now ships four hand-written man pages ([plan](2026-05-18-package-f9dasm.md) §1foundry2). If a future change to `debian/man/*.1` introduces troff syntax errors or omits a binary, the live test won't catch it.
2. **No regression guard for the lintian gate.** The `/package` skill update (`~/.claude/skills/package/SKILL.md` Step 4) makes `no-manual-page` a blocking lintian warning. The live test should mirror that gate — if a binary in `/usr/bin/` lacks a corresponding `/usr/share/man/man1/<binary>.1.gz`, fail loudly.

Also incidental: line 48's `grep -h "^Package:" packages/*/DEBIAN/control` predates the canonical-debian-layout migration ([commit f1ddbb0](#)) and only matches xa65 (the legacy holdout). Currently `ALL_PACKAGES` reduces to `xa65` alone — the metapackages and f9dasm are silently skipped from the default no-filter run.

## Approach

Patch `run-test.sh` to:

1. **Discover packages from both layouts.** Glob both `packages/*/DEBIAN/control` and `packages/*/debian/control` so xa65 (legacy) and the canonical metapackages + f9dasm all show up.
2. **Drop the dpkg excludes file** before the first `apt-get install`. Single `rm /etc/dpkg/dpkg.cfg.d/excludes` line; cheaper than `unminimize` (no extra download), restores man-page and doc install behavior to match a real desktop.
3. **Install `man-db` alongside `curl gpg ca-certificates`** so `man -l` can render pages.
4. **For each package with `Architecture: any`** (i.e. real binaries, not pure metapackages): after install, derive the binary list from `dpkg -L <pkg> | grep -E '^/usr/bin/'`, and for each binary assert:
   a. A man page exists at `/usr/share/man/man1/<binary>.1.gz`.
   b. `man -l /usr/share/man/man1/<binary>.1.gz` exits 0 — proves the page parses without troff errors.
5. **Pure-metapackage packages** (`Architecture: all`) skip the binary/man-page assertion — they own no `/usr/bin/` files.

`Architecture` is read directly from the in-tree `debian/control` (or `DEBIAN/control` for legacy xa65) — already parsed once for the existing `task`-dep check. Add a single line to read it.

## Why drop the file vs `unminimize`

`unminimize` fetches ~60-100 MB of locale data, full doc trees, info pages, etc. — none of which the test cares about. We just want `path-exclude=/usr/share/man/*` to stop firing. The file is two-dozen lines; deleting it costs 0 bytes and 0 ms. `unminimize` would also work but the simpler option suffices for the assertion we need.

Documented in [the previous turn's investigation](#) — verified by re-installing f9dasm from the live repo with `rm /etc/dpkg/dpkg.cfg.d/excludes` first and seeing all four `.1.gz` files appear at `/usr/share/man/man1/`.

## Files to change

| File | Action |
|---|---|
| `foundry-apt/test/run-test.sh` | **edit** — package discovery fix (both layouts), `rm` the excludes file, install `man-db`, post-install binary+man-page assertion for `Architecture: any` packages |
| `docs/plans/2026-05-18-live-test-manpage-assert.md` | **new** — this file |
| `TODO.md` | **edit** — add a `Done` line once verified |

## Approximate `run-test.sh` shape (delta only)

```bash
# package discovery — both layouts
mapfile -t ALL_PACKAGES < <(
    {
        grep -h "^Package:" packages/*/DEBIAN/control 2>/dev/null || true
        grep -h "^Package:" packages/*/debian/control  2>/dev/null || true
    } | awk '$1 == "Package:" {print $2}' | sort -u
)

# per-package: locate control file in whichever layout, read Architecture
control_file=$(ls packages/${pkg}/DEBIAN/control packages/${pkg}/debian/control 2>/dev/null | head -1)
arch=$(awk -F': ' '/^Architecture:/ {print $2; exit}' "$control_file")

# In the container body, before `apt-get install <pkg>`:
rm -f /etc/dpkg/dpkg.cfg.d/excludes
# (Then `apt-get install` of curl/gpg/ca-certificates ALSO installs man-db.)

# After install, only for Architecture: any:
if [[ "${arch}" == "any" ]]; then
    binaries=$(dpkg -L "${pkg}" | grep -E '^/usr/bin/[^/]+$' || true)
    for bin in $binaries; do
        name=$(basename "$bin")
        page="/usr/share/man/man1/${name}.1.gz"
        [[ -f "$page" ]] || { echo "MISS man page: $page"; exit 1; }
        man -l "$page" >/dev/null 2>&1 || { echo "BAD troff: $page"; exit 1; }
    done
fi
```

## Verification

1. **Discovery works for both layouts.**

    ```
    bash foundry-apt/test/run-test.sh --download-only 2>&1 | grep "Testing:"
    ```

    Expect: seven lines — `f9dasm`, `foundry-linux-retro-tools`, `worldfoundry-{android-dev,blender,dev,engine-build-deps}`, `xa65`. (vs. current behavior: just `xa65`.)

    ```
    === Testing: apt-get install f9dasm ===
    === Testing: apt-get install foundry-linux-retro-tools ===
    === Testing: apt-get install worldfoundry-android-dev ===
    === Testing: apt-get install worldfoundry-blender ===
    === Testing: apt-get install worldfoundry-dev ===
    === Testing: apt-get install worldfoundry-engine-build-deps ===
    === Testing: apt-get install xa65 ===
    Results: 7 passed, 0 failed
    ```

    PASS.

2. **f9dasm assertion passes.**

    ```
    bash foundry-apt/test/run-test.sh --package f9dasm
    ```

    Expect: `[PASS] f9dasm` with no man-page-missing or troff-error output.

    First run against live `1foundry2`: **caught a real bug** — `mandoc -Tlint` flagged `WARNING: skipping paragraph macro: PP after SH` in three of our own sibling pages (hex2bin, mot2bin, cmd2mot). Bumped to `1foundry3`, dropped the `.PP` after `.SH EXAMPLES` and replaced `.RS`/`.RE` with `.IP`. Rebuilt locally and re-checked:

    ```
    $ docker run --rm -v $PWD/foundry-apt:/work ubuntu:26.04 bash -c "... mandoc -Tlint -man on each man page from dist/f9dasm_1.83-1foundry3_amd64.deb"
      ok /tmp/x/usr/share/man/man1/cmd2mot.1.gz
      ok /tmp/x/usr/share/man/man1/f9dasm.1.gz
      ok /tmp/x/usr/share/man/man1/hex2bin.1.gz
      ok /tmp/x/usr/share/man/man1/mot2bin.1.gz
    ```

    PASS for foundry3. Live test will pass once foundry3 is republished — `1foundry2` on R2 still trips the gate (which is the gate doing its job).

3. **Synthetic regression: remove a man page from the published .deb in a side-test.**

    Not run as part of `task live-test` — exercised once during plan verification by hand. Recompress `f9dasm` locally without `debian/man/f9dasm.1`, re-publish to a local aptly repo, run the modified test against that repo, confirm it reports `MISS man page: /usr/share/man/man1/f9dasm.1.gz` and exits 1. (Skip if time-bounded — the assertion logic is self-evidently correct.)

    Did the in-container equivalent (install f9dasm, `rm /usr/share/man/man1/f9dasm.1.gz`, re-run the assertion body):

    ```
      ok /usr/share/man/man1/cmd2mot.1.gz
    MISS man page: /usr/share/man/man1/f9dasm.1.gz
      ok /usr/share/man/man1/hex2bin.1.gz
      ok /usr/share/man/man1/mot2bin.1.gz
    exit code would be: 1
    ✓ ASSERTION FAILED AS EXPECTED
    ```

    PASS.

4. **Synthetic regression: malformed troff in a man page.**

    Same shape — append a deliberately broken `.SH` line missing the closing macro, rebuild, run the test, confirm it reports `BAD troff: /usr/share/man/man1/f9dasm.1.gz` and exits 1. (Also skip if time-bounded.)

    Wrote a bogus page (`.TH FAKE 1` / `.SH` (empty) / `.UNKNOWN_MACRO_NAME` / `.SH NAME`), gzipped into place, re-ran the assertion. First attempt used `man -l` which accepted the bad page (groff is permissive). Switched to `mandoc -Tlint -W warning -man`. Result:

    ```
    BAD mandoc lint: /usr/share/man/man1/f9dasm.1.gz
        mandoc: <stdin>:3:2: ERROR: skipping unknown macro: .UNKNOWN_MACRO_NAME
        mandoc: <stdin>:2:2: WARNING: line scope broken: SH breaks SH
        mandoc: <stdin>:1:2: WARNING: missing date, using "": TH
    syn2 miss=1
    ```

    PASS. (Implementation note: `mandoc -W warning` filters out STYLE-level cosmetic findings — needed because xa65's upstream-vendored pages have plenty of STYLE issues we won't be fixing. xa65 also has real WARNING issues that mandoc catches, so xa65 lives on `MANPAGE_CHECK_SKIP` until it's retired per TODO.md Phase 1.)

5. **All seven packages still pass against the live repo.**

    ```
    bash foundry-apt/test/run-test.sh
    ```

    Expect: `Results: 7 passed, 0 failed`.

    Pre-republish state (live R2 still has `f9dasm 1.83-1foundry2`): expect `Results: 6 passed, 1 failed` (f9dasm fails on the WARNING from `.PP after SH`). The gate is reporting a real bug that's fixed in `1foundry3` locally.

    Post-republish state (after user tags + pushes a new release): expect `Results: 7 passed, 0 failed`. Re-run this step at that point and replace this evidence block.

## Out of scope

- **Doc-file assertion** (`/usr/share/doc/<pkg>/copyright`, `changelog.Debian.gz`). Reasonable next step but not part of this turn — the path-exclude fix already makes them present; we just don't assert on them yet.
- **`unminimize` mode** as an opt-in flag. If we ever want to test against a "full desktop" container (e.g. for testing GUI dependencies), add `--unminimize` later.
- **Adding a `Architecture: any` `apt-cache show`-driven discovery** (i.e. not requiring the in-tree control file). Today's in-tree discovery is simpler and matches what we publish.
