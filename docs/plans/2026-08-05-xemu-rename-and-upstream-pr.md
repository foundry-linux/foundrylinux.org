# Rename: `xemu` → the Commodore suite, `xemu-xbox` → the Xbox emulator

**Date:** 2026-08-05
**Scope:** Swap two published package names in `foundry-apt`, drop the old ones from the live repo, port
upstream's `.desktop` generation, and prepare (not open) an upstream PR replacing LGB's own `.deb` script.

**No visible surface** beyond application-menu entries, which are covered by porting upstream's own
`.desktop` template rather than inventing one — so no mockup bundle.

---

## Decision

Two unrelated upstreams are both called "xemu". We package both. The name goes to the project that has
been calling itself that in its own shipped `.deb`:

| package | was | becomes | upstream |
|---|---|---|---|
| LGB's Commodore/Hungarian 8-bit suite | `x-emulators` | **`xemu`** | [lgblgblgb/xemu](https://github.com/lgblgblgb/xemu) |
| Original Xbox emulator | `xemu` | **`xemu-xbox`** | [xemu-project/xemu](https://xemu.app) |

`xemu_xbox` is not an option — Debian Policy §5.6.1 restricts package names to lowercase alphanumerics,
`+`, `-` and `.`.

### Why this way round

LGB's `build/deb-build-simple.sh` has emitted `Package: xemu` since 2016. Ours was the newcomer to that
name. Taking `x-emulators` for his suite meant our repo disagreed with his own package about what his
project is called — and left a live collision: his self-built `.deb` and our published `xemu` (Xbox) share
a dpkg identity while being unrelated software, with his `cdate` version (`20260129235930`) sorting above
our `0.8.136`, so his build would silently replace ours. Giving him the name removes the collision at its
root instead of routing around it.

## No backwards compatibility

**There are no existing consumers.** Confirmed with the user 2026-08-05. So:

- **No transitional dummy packages**, matching the `/package` skill's rule ("pre-1.0 / no users → no
  transitional dummy package").
- **No `Breaks:`/`Replaces:`** migration metadata.
- Old `x-emulators_*` and the old Xbox `xemu_*` artifacts are **deleted from the live repo**, not
  superseded.

This is the one thing that makes the swap simple. It would otherwise be nasty: our published Xbox `xemu` is
`0.8.136-1foundry1` and the suite is `0~git…`, which sorts *lower*, so apt would see a downgrade and refuse
to swap — leaving users stranded on an Xbox emulator named after a Commodore suite.

## Order of operations (the swap cannot be done in either order)

`packages/xemu` is occupied, so the Xbox package must vacate the name before the suite can take it:

1. `git mv packages/xemu packages/xemu-xbox`
2. `git mv packages/x-emulators packages/xemu`
3. Rewrite each tree's identity — per the skill's rename section:
   - `debian/control`: `Source:` + `Package:`
   - `debian/changelog`: new top entry under the **new** name, bumped revision, rename noted; older
     entries stay historical
   - `build.sh`: `NAME=` (drives the `.deb` filename) and help text. **Watch for `NAME` also
     driving the *source directory*** — `xemu-xbox/build.sh` derived
     `SRC_DIR="$WORKDIR/${NAME}-${UPSTREAM_VERSION}"`, but the tarball extracts to upstream's own
     `xemu-0.8.136/`, so the rename broke extraction outright. Fixed by introducing a separate
     `UPSTREAM_NAME` and renaming the extracted tree. Any package whose upstream tarball top-dir
     no longer matches our package name needs this.
   - `debian/rules`: `debian/<new>/…` DESTDIR paths
   - `debian/<new>.manpages`, `debian/<new>.xpm`, `debian/<new>.lintian-overrides` — any per-package
     filename
4. Referrers, in the same commit:
   - `packages/foundry-emulators-consoles/debian/control` → `xemu-xbox` (+ changelog bump)
   - `packages/foundry-emulators-computers/debian/control` → `xemu` (+ changelog bump)
   - `foundry-apt/LICENSES-VENDORED.md`, `foundry-apt/README.md`
   - `docs/itp-drafts/x-emulators.md` → `docs/itp-drafts/xemu.md`, plus its README table row and
     finding #9 (the naming argument inverts: we now *keep* upstream's name and qualify the Xbox one)
   - `TODO.md` ITP line
   - `docs/plans/2026-08-05-package-x-emulators.md` and `…-package-xemu.md` keep their filenames as
     historical records but gain a rename note at the top
5. **Purge `dist/xemu_0.8.136*` by hand before rebuilding.** `prune-dist.sh` keeps the newest
   version *per package name*, and the outgoing Xbox `xemu` (`0.8.136`) sorts **above** the
   incoming suite (`0~git…`). Left in place it is read as the current `xemu` and the freshly
   built suite is deleted as "superseded" — which is exactly what happened on the first
   attempt. Orphan detection cannot catch it: the stale file still carries a live `Package:`
   name.
6. `scripts/prune-dist.sh` then drops genuinely orphaned `.debs` whose package no longer appears in any
   `packages/*/debian/control` — which is exactly the old two names. Verify it actually removes them
   rather than assuming.
7. Republish; verify the old names are **gone** from the live index and the new ones resolve.

## Port upstream's `.desktop` generation

Our seven `.desktop` files are hand-written and static. Upstream generates his from
`targets/*/Makefile`, reading `PRG_TARGET` and `EMU_DESCRIPTION` and substituting into one
`build/xemu.desktop` template. His approach is strictly better and we should adopt it *before* offering
him a PR, so we are contributing something better than what he has rather than merely different:

- **Self-maintaining.** Upstream ships 15 targets; the default build produces 7. A new target gets a menu
  entry automatically instead of silently lacking one.
- **Authoritative names.** Our hand-written strings have already drifted: we say "Commodore LCD
  (prototype)" and "Enterprise 64/128" where upstream says "Commodore LCD" and "Enterprise 128".
- His `Categories=` line was taken verbatim with the rest of the template. **Note this is a deliberate
  inconsistency with the rest of our repo**, which uses plain `Game;Emulator;` (`mesen2`) — his adds
  `X-Debian-Applications-Emulators` (a legacy Debian-menu category; `menu`/`menu-xdg` do still exist in
  26.04 but are vestigial) and `Application` (dropped from the freedesktop spec). Kept as-is because the
  point of porting his template is to stay aligned with upstream, and because we are about to offer the
  packaging back to him — diverging on his own template would be gratuitous. Revisit if we ever
  standardise categories across all our `.desktop` files.

**Confirmed after porting:** the generated entries carry upstream's own names, and two of the
hand-written ones had already drifted — we said "Commodore LCD (prototype)" and "Enterprise 64/128"
where upstream says "Commodore LCD" and "Enterprise 128".

Also worth taking while we are here: a `Bugs:` control field pointing at his issue tracker, `AUTHORS` into
`/usr/share/doc`, and the legacy `/usr/share/pixmaps/` icon alongside the hicolor one.

**Deliberately not taking:** the ROM-fetch helper (upstream disabled it on legal grounds — that judgement
stands and is recorded), the `alien`/RPM step, the `cdate` versioning, and the `dpkg -s libreadline-dev`
build-time dependency shell-out.

## Upstream PR (prepared, NOT opened)

Replace `build/deb-build-simple.sh` with a real `debian/` source tree in his repo. His script is
self-described as "a very lame binary-level DEB package builder" and its own `README.Debian` says
"Ugly, direct binary build, without source package :(". Ours is policy-compliant and lintian-clean on both
the `.deb` and the `.dsc`.

Caveat to state in the PR: Debian-proper packaging normally lives outside the upstream tree, so this does
not pre-empt an eventual ITP — an in-tree `debian/` serves people building their own, which is exactly
what his script does today.

**Posting is user-triggered**, same rule as the ITP and the llvm-mos comment. Draft it, do not open it.

## Execution log — two failures worth recording

**1. `xemu-xbox` build failed on the first attempt.** `NAME=` drove the source-directory path as well as
the `.deb` filename. See the fix noted in step 3 above.

**2. `prune-dist.sh` deleted the freshly built suite.** This is the version-inversion hazard from the
"No backwards compatibility" section, surfacing somewhere unexpected. Prune keeps the newest version
*per package name*; the outgoing Xbox `xemu` (`0.8.136`) sorts **above** the incoming suite (`0~git…`),
so the stale `.deb` was read as the current `xemu` and the new build was removed as "superseded" — with
a message that reads as though it succeeded:

```
prune xemu_0~git20260129.40dfef0d-1foundry3.debian.tar.xz  (source tarball no longer referenced by a kept .dsc)
```

Orphan detection cannot catch this: the stale file still carries a `Package:` name that is live. Fixed by
purging `dist/xemu_0.8.136*` by hand before rebuilding (now step 5), and generalised into the `/package`
skill's rename section as step 5b, since it applies to any name swap or name reuse.

Both failures share a root shape with the collision that motivated this rename in the first place: dpkg
identity is the *name*, and version ordering decides the rest. Reusing a name for different software is
therefore never purely cosmetic.

## Verification

Status as of the first pass. Items 4–6 are gated on the in-flight rebuild and the publish.

1. **Package identity.** ✅ **PASS**

    ```
    xemu/debian/control:1:Source: xemu            xemu/debian/control:16:Package: xemu
    xemu-xbox/debian/control:1:Source: xemu-xbox  xemu-xbox/debian/control:32:Package: xemu-xbox
    ```

    No `x-emulators` remains outside historical plan/transcript files, the ITP draft's fallback
    list (where it is deliberate), and the wnpp-search line recording what was searched for.

2. **Build + lintian.** ✅ **PASS for `xemu`** — builds in a clean `ubuntu:26.04`, lintian clean on
   **both** the `.deb` and the `.dsc`. ⏳ `xemu-xbox` rebuilding after the `NAME`/`SRC_DIR` fix; its
   first attempt failed outright, so this is not yet proven.

3. **Menu entries generated, not hand-written.** ✅ **PASS**

    ```
    Name=Xemu for Commodore 65        Name=Xemu for MEGA65
    Name=Xemu for Commodore LCD       Name=Xemu for Primo
    Name=Xemu for Enterprise 128      Name=Xemu for Videoton TV Computer
    Name=Xemu for Commodore VIC-20
    ```

    Seven entries for seven shipped binaries, names taken from upstream's `EMU_DESCRIPTION`, plus the
    `/usr/share/pixmaps` icon its `Icon=` points at. Two previously hand-written names were wrong.

4. **Metapackage chains resolve** (`foundry-emulators-computers` → `xemu`,
   `foundry-emulators-consoles` → `xemu-xbox`). ⏳ **PENDING** — gated on the rebuild.

5. **Live repo after publish:** `x-emulators` and the old Xbox `xemu_0.8.136*` **absent** from the
   `Packages` index; `xemu` resolves to the suite and `xemu-xbox` to the Xbox emulator. ⏳ **PENDING.**
   Local `dist/` has already been purged of both old names, so this is now about confirming the sync
   deletes them from R2 rather than leaving them stranded.

6. **End-to-end regression:** the renamed `xemu` package's `xc65` still runs the bare-metal 65CE02
   kernel to `0xE0E8` from the live repo. ⏳ **PENDING** — this is the check that would catch the
   `.desktop`/rename work having broken the actual emulators, which no lintian or metadata test can.

## Follow-ups

- [ ] Open the upstream PR once approved; record the URL here.
- [ ] Investigate the 8 upstream targets not built by default (`xcx16`, `xzxspect`, `xrc2014`, `xc900`,
      `xpc`, `xrecpm`, …) — there may be more emulators available at no extra cost.
