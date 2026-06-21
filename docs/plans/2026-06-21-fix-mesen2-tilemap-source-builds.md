# Plan: fix the 2 source-build gaps (mesen2, tilemap-studio)

## Context

The v1.5.35 publish made source packages live, but **51 of 53** packages shipped a `.dsc` —
`mesen2` and `tilemap-studio` hit `lib-source-build.sh`'s best-effort `WARN` path (binary
shipped, no source package). Both binaries are fine; the goal is to close the gap so the
`Sources` index (and Repology tracking) is complete. Reproduced each source pass with stderr
visible (`dpkg-buildpackage -S`) in `ubuntu:26.04` — both are **per-package quirks**, not a
helper bug, so the fixes live in each package's tree.

## Root causes (confirmed)

**mesen2** — `3.0 (quilt)`, vendors an icon under `debian/`:
```
dpkg-source: error: unwanted binary file: debian/mesen2.png
dpkg-source: error: detected 1 unwanted binary file (add it in debian/source/include-binaries …)
```
`dpkg-source -b` refuses binary files under `debian/` unless allowlisted. The binary build
(`-b`) doesn't check this; the source build (`-S`) does. `mesen2/debian/` has exactly one
binary (`mesen2.png`), no `include-binaries`.

**tilemap-studio** — `3.0 (quilt)`, patches applied **in-tree via perl**:
```
dpkg-source: info: the patch has fuzz which is not allowed, or is malformed
dpkg-source: error: … patch … 0001-platform-h-in-main-window.patch … failed with exit status 1
```
`build.sh` applies its 2 build-fix patches with `perl -i` because the upstream is **CRLF** and
GNU `patch`/quilt fuzz on it (documented in the build.sh). But `dpkg-source -b` re-applies the
`debian/patches/series` patches itself with GNU `patch -F 0` against the pristine orig → fuzz →
fail. (`tilemap-studio/debian/` has no binary files, so include-binaries isn't its issue.)

## Fixes (per-package — no `lib-source-build.sh` change)

**mesen2** — create `foundry-apt/packages/mesen2/debian/source/include-binaries`:
```
debian/mesen2.png
```
That's the whole fix — the synthesized orig (the 83 MB binary repack, mesen2 stages no pristine
tarball) is already what `emit_source_package` produces; this just allows the icon.
*(Note: the working tree carries an uncommitted `-1foundry3` bump from prior WIP — leave it
unstaged; commit only the new `include-binaries` file, which applies cleanly to either rev.)*

**tilemap-studio** — in `foundry-apt/packages/tilemap-studio/build.sh`, right after the perl
patch application and **before** `emit_source_package` (~line 127), bake the patches into the
orig and drop the series for the source build:
```bash
# Source package: the patches are already applied in-tree above (perl, because dpkg-source's
# GNU patch fuzzes on the CRLF upstream). Drop the pristine orig + empty the staged series so
# emit_source_package synthesises a self-contained orig from the patched tree and dpkg-source
# -b doesn't try (and fail) to re-apply the CRLF patches. Harmless to the binary build below
# (its dpkg-source --before-build then applies nothing; the tree is already patched).
rm -f "$WORKDIR/${NAME}_${UPSTREAM_VERSION}.orig.tar."*
: > "$SRC_DIR/debian/patches/series"
```
`emit_source_package` then finds no matching orig → synthesises from the patched tree → builds
a clean `.dsc` (patched orig + empty series; the `.patch` files remain in `debian/patches/` as
documentation, unapplied).

## Verification

Reuse the debug harness pattern (mount the repo, run each `build.sh` in `ubuntu:26.04`) but let
the source pass run to completion:

1. **Both source builds succeed** — `dpkg-buildpackage -S` exits 0; `dist/` gets
   `mesen2_*.dsc` + `tilemap-studio_4.0.1-*.dsc` (+ tarballs).
2. **Round-trip** — `dpkg-source -x` each `.dsc` extracts cleanly.
3. **Binaries unaffected** — each `build.sh` still produces its `.deb` (the changes only touch
   the source path / a new `include-binaries` file).
4. **lintian on the `.dsc`** — no new `E:`.

## Publish + skill follow-up

- Commit the two per-package fixes (mesen2 `include-binaries`, tilemap-studio `build.sh`),
  push, then `task bump` (→ v1.5.36) so all **53** source packages go live — or let them ride
  the next publish. Re-run the live-verify (`Sources.gz` count = 53, both present).
- **Iterate the `/package` skill** (per `feedback_package_skill_iterate`): add these two
  gotchas to Step 4's source-build guidance — (a) binaries under `debian/` need
  `debian/source/include-binaries`; (b) packages that apply patches in-tree (CRLF upstreams)
  must synthesise the orig from the patched tree + empty the series for the source pass.
