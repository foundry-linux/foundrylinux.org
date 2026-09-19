# MarkText `electron-log` integration fix

**Date:** 2026-08-31

**Status:** source-build revision in progress; repository publication pending approval

**Package repository:** `/home/will/foundrylinux.org/foundry-apt`

**Website:** `/home/will/sharedpair.dev`

## Objective

Repair MarkText's packaged-mode adaptation so its bundled `electron-log` code remains intact under `electron-runtime-42`, strengthen the behavioral test to prove that the editor renders a real document, and publish captured evidence only after the corrected package passes.

## Confirmed cause

The current `marktext` build performs an equal-length, in-place ASAR substitution:

- from `electron.app.isPackaged`;
- to `process.env.MT_PACKAGED`.

The substitution is global and expects three matches. Two matches are direct MarkText packaged-mode checks that need adaptation. The third is a substring within `electron-log`'s `this.electron.app.isPackaged` expression and must not be changed.

The global replacement currently produces `this.process.env.MT_PACKAGED` inside `electron-log`'s `ElectronExternalApi.isDev()`. `this.process` is undefined, so MarkText throws during main-process initialization:

`TypeError: Cannot read properties of undefined (reading 'env')`

The failure was exposed by attempting to open and capture a real Markdown document. The existing smoke test can report success after brief process survival without proving that MarkText's editor window rendered.

## Phase 1 — Narrow the packaged-mode substitution

1. Change `packages/marktext/build.sh` so it replaces only direct, standalone `electron.app.isPackaged` expressions.
2. Exclude property chains such as `this.electron.app.isPackaged` from the match, using an explicit contextual pattern rather than a global substring replacement.
3. Preserve the equal-length substitution requirement so ASAR offsets remain unchanged.
4. Replace exactly two direct MarkText references with `process.env.MT_PACKAGED`.
5. Assert after substitution that:
   - exactly two direct references were replaced;
   - `electron-log` still contains `this.electron.app.isPackaged`;
   - no `this.process.env.MT_PACKAGED` corruption exists; and
   - the two intended `process.env.MT_PACKAGED` checks remain in MarkText application code.
6. Keep the existing `MT_PACKAGED=1` launcher environment and application-private resource-path adaptation unchanged.

## Phase 2 — Package revision and build

1. Add `marktext (0.19.1-1foundry2)` to `debian/changelog` with the precise integration fix.
2. Build in the repository's Ubuntu 26.04 container; do not build against host libraries.
3. Verify the resulting package metadata, Electron 42 dependency bounds, installed file ownership, native ABI 146 modules, and absence of bundled Electron or Chromium.
4. Inspect the built ASAR bytes around every packaged-mode reference to confirm the intended two-to-one split between adapted MarkText code and preserved `electron-log` code.

## Phase 3 — Strengthen the behavioral test

Update `test/test-shared-electron-marktext.sh` so success requires more than a live process:

1. Create a non-personal Markdown fixture containing headings, lists, emphasis, and a block quotation.
2. Launch MarkText as an unprivileged user with an isolated home, configuration, and cache.
3. Wait for a real MarkText editor window associated with the fixture.
4. Require the window to remain alive after the document has had time to render.
5. Reject logs containing `App threw an error`, `Uncaught Exception`, `Cannot read properties of undefined`, native-module ABI failures, initialization failures, or fatal errors.
6. Retain the package-boundary and native-module assertions from the current test.
7. Run the test in a fresh Ubuntu 26.04 container against the exact `0.19.1-1foundry2` artifact and `electron-runtime-42 42.9.3-1foundry1`.

## Phase 4 — Captured evidence

1. Reuse the clean-container fixture from the behavioral test.
2. Capture the maximized MarkText application window only after the rendered headings, lists, and quotation are visible.
3. Reject blank editors, loading states, JavaScript error dialogs, desktops, and unrelated windows.
4. Store the image under `sharedpair.dev/public/images/applications/marktext/`.
5. Record the exact package, runtime, platform, fixture, capture method, and date in screenshot provenance.
6. Add the image as “Captured evidence” on `/applications/marktext/`; retain the official upstream image separately as “Interface reference.”

## Phase 5 — Shared Pair regeneration and validation

1. Update `shared-electron/status.json` to the corrected MarkText revision only after the package and fresh-container test pass.
2. Regenerate the Shared Pair package registry and measurement manifest from the new `.deb`.
3. Run the complete Astro check, production build, rendered-route verification, image-provenance checks, and local-path hygiene checks under pinned Node 26.8.1.
4. Confirm that MarkText remains in the tested cohort only when the stronger behavioral gate passes.

## Acceptance criteria

- `electron-log` retains `this.electron.app.isPackaged` in the built ASAR.
- The built ASAR contains no `this.process.env.MT_PACKAGED` corruption.
- Only MarkText's two direct packaged-mode checks use `process.env.MT_PACKAGED`.
- MarkText opens the Markdown fixture as an unprivileged user on shared Electron 42 without a main-process error.
- The fresh-container test proves a rendered editor window and rejects JavaScript error dialogs.
- The website shows a genuine captured MarkText editor with complete provenance.
- All generated-data and site validation commands pass.
- No repository promotion or external deployment occurs without Will's approval.

## Implementation record

- `marktext 0.19.1-1foundry2` replaces only the two direct MarkText packaged-mode checks and preserves `electron-log`'s `this.electron.app.isPackaged` reference.
- Build-time assertions reject the prior `this.process.env.MT_PACKAGED` corruption.
- The Ubuntu 26.04 container build completed successfully.
- The strengthened fresh-container test opened a real Markdown document window and passed with ABI 146 native modules on `electron-runtime-42 42.9.3-1foundry1`.
- A maximized MarkText editor rendering the non-personal Shared Pair fixture was captured with exact package/runtime provenance.
