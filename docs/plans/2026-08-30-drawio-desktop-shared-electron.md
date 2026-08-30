# draw.io Desktop on the shared Electron runtime

**Date:** 2026-08-30

**Status:** proposed

**Scope:** Package draw.io Desktop for Ubuntu 26.04 amd64 as the second
independent application using the versioned shared Electron runtime. Do not
publish it, change an ISO, or claim broader compatibility until the gates below
pass.

**Candidate repository:** `https://apt.wald3n.com/shared-electron`, suite
`resolute`, component `main`. This is separate from `apt.foundrylinux.org`.

---

## TL;DR

Use draw.io Desktop as the first ecosystem-level test of the contract proven by
LosslessCut. Package only draw.io's application code, assets, and desktop
integration; run it with `electron-runtime-42`; retain application-specific
files privately; and verify both GUI and command-line export behavior in a clean
Ubuntu 26.04 environment.

The initial target is upstream draw.io Desktop **31.3.1**, whose release notes
identify Electron **42.9.3**. Our current runtime is 42.3.0, so the first
implementation step is a normal same-major runtime patch update to 42.9.3. The
draw.io package must depend on at least the exact tested patch and remain bounded
below Electron 43. This tests the promised shared-patch maintenance model rather
than silently assuming every Electron 42 patch is interchangeable.

This work is packaging and compatibility validation, not an upstream fork.
Upstream is Apache-2.0 but explicitly says the project is generally closed to
outside contributions. Keep the integration patch small and separately
documented; offer findings upstream as an issue or design note only if useful.

## Why draw.io is next

| Goal | How draw.io tests it |
|---|---|
| Prove the runtime is genuinely shared | It is unrelated to LosslessCut but uses the same Electron 42 major. |
| Exercise a different application shape | draw.io is a diagram editor with file associations, printing, PDF/image export, custom protocols, and CLI export. |
| Test same-major runtime maintenance | Current draw.io needs Electron 42.9.3, requiring the shared runtime to advance from 42.3.0 without breaking LosslessCut. |
| Produce measurable deduplication | A machine with LosslessCut should download only draw.io's app payload, not another Chromium runtime. |
| Validate sustainable packaging | Upstream publishes source plus Linux `.deb`, AppImage, and RPM artifacts under Apache-2.0. |

## Desired package contract

```text
electron-runtime-42 (>= 42.9.3), electron-runtime-42 (<< 43)
  /usr/lib/electron-runtime-42/
    electron
    chrome-sandbox
    Chromium resources, locales, snapshots, and shared libraries

drawio-desktop
  /usr/lib/drawio-desktop/
    app.asar and app-specific resources only
  /usr/bin/drawio
    launcher selecting /usr/lib/electron-runtime-42/electron
  /usr/share/applications/drawio.desktop
  /usr/share/icons/...
  MIME registrations and application documentation
```

The application package must not own `electron`, `chrome-sandbox`, Chromium
packs, generic Electron locales, V8 snapshots, or runtime libraries. A file is
retained in the application only when byte-level inspection or runtime tracing
shows it is draw.io-specific.

## Non-goals

- Supporting Electron 43 merely because intermediate draw.io releases used it;
  the selected current release is back on Electron 42.
- Replacing or weakening Electron sandboxing.
- Modifying draw.io document formats or application behavior.
- Enabling upstream self-update inside an APT-managed installation.
- Supporting RPM, AppImage, Snap, Flatpak, arm64, Debian Stable, or distributions
  beyond Ubuntu 26.04 in this iteration.
- Publishing packages or adding draw.io to any Foundry ISO as part of this plan.
- Claiming upstream endorsement.

## Distribution support

The launch binary is built on and supported for Ubuntu 26.04 amd64. Kubuntu
26.04 and Foundry's Ubuntu-26.04 base consume that same ABI build but must each
pass a clean-machine smoke test. Other derivatives are not automatically
supported. Debian Stable is a later target requiring a native Debian rebuild in
a separate repository suite; Ubuntu 24.04, arm64, and non-Debian package formats
remain out of this iteration.

## Phase 0 — freeze and inspect the upstream baseline

- [ ] Pin the `v31.3.1` source tag/commit and record its archive SHA-256.
- [ ] Pin the official amd64 Linux `.deb` and AppImage digests from the release.
- [ ] Record upstream's Apache-2.0 license and audit bundled third-party notices.
- [ ] Unpack the official `.deb` and AppImage into disposable directories.
- [ ] Generate sorted file manifests, sizes, MIME types, ELF metadata, and hashes.
- [ ] Confirm the embedded runtime reports Electron 42.9.3.
- [ ] Compare generic runtime files byte-for-byte with the official Electron
  42.9.3 Linux x64 archive; explain every mismatch before removing it.
- [ ] Inspect Electron fuses, ASAR integrity metadata, sandbox permissions,
  `resourcesPath` assumptions, updater configuration, and command-line switches.
- [ ] Inventory native `.node` modules, shared objects, helper executables,
  dictionaries, fonts, codecs, and licenses. Do not assume the app is pure JS.
- [ ] Capture upstream desktop entry, icons, MIME declarations, URL handlers,
  executable names, `StartupWMClass`, and maintainer scripts.
- [ ] Establish the monolithic compressed size, installed size, and cold/warm
  launch measurements for later comparison.

### Phase-0 decision gate

Proceed only if redistribution is clear, the application can run under the
official unfused Electron 42.9.3 runtime without weakening sandboxing, and all
remaining native/application files have attributable ownership. Record a
blocking finding rather than patching around an unexplained security control.

## Phase 1 — update and requalify the shared runtime

- [ ] Bump `electron-runtime-42` from 42.3.0 to 42.9.3 with the official archive
  digest and a new Debian revision.
- [ ] Regenerate binary and Debian source artifacts; run repository license,
  Repology, source-index, manifest, and lint gates.
- [ ] Verify `chrome-sandbox` remains root-owned mode 4755 and the runtime runs as
  an unprivileged user without `--no-sandbox`.
- [ ] Install the updated runtime with the existing split LosslessCut package.
- [ ] Run the LosslessCut headless executable/FFmpeg checks and its available GUI
  smoke tests before using 42.9.3 as draw.io's base.
- [ ] Compare the prior and new runtime manifests and review added/removed files.
- [ ] Retain the 42.3.0 package and repository snapshot as a rollback candidate
  until both applications complete soak testing.

## Phase 2 — build the application-only draw.io package

- [ ] Create `foundry-apt/packages/drawio-desktop/` following the vendored-package
  conventions: digest-pinned input, `--help`, temporary staging, source artifact
  emission, and output under `dist/`.
- [ ] Prefer building application resources from the pinned source and lockfile
  in an Ubuntu 26.04 container. If that cannot be made network-closed and
  reproducible in this iteration, repackage the verified official `.deb` while
  documenting that provenance explicitly.
- [ ] Commit or use the upstream npm lockfile; use `npm ci`, an isolated cache,
  and a pinned Node/npm toolchain. Never use floating `npx` dependencies.
- [ ] Produce an app-only staging tree, removing only files matched by the
  byte-level runtime ownership manifest.
- [ ] Preserve app-specific resources, native modules, third-party notices,
  icons, file associations, and CLI functionality.
- [ ] Disable `electron-updater` for the distro-managed build without disabling
  manual version/help information. APT is the only updater for this package.
- [ ] Add `/usr/bin/drawio`, exporting only proven environment overrides and
  executing `/usr/lib/electron-runtime-42/electron` with the app entry point.
- [ ] Declare `Depends: electron-runtime-42 (>= 42.9.3),
  electron-runtime-42 (<< 43)` plus generated system-library dependencies.
- [ ] Add `X-Repology-Project` only after confirming draw.io's canonical Repology
  slug; use `none` temporarily rather than guessing.
- [ ] Add Debian copyright, changelog, source format, lintian overrides with
  narrow explanations, and a sorted `LICENSES-VENDORED.md` entry.
- [ ] Emit `.deb`, `.dsc`, orig tarball, and Debian tarball. Do not publish them.

### Naming

Use source and binary package name `drawio-desktop` unless repository inspection
finds a collision or the established Repology project maps the official package
differently. Preserve upstream's user-facing “draw.io” name, `/usr/bin/drawio`,
desktop file ID where compatible, MIME associations, and `StartupWMClass`.
Document any unavoidable upgrade/conflict relationship with upstream's official
`.deb`; do not allow both packages to own the same paths silently.

## Phase 3 — automated verification

Add `foundry-apt/test/test-shared-electron-drawio.sh` and a Taskfile entry. Run
the tests in a fresh Ubuntu 26.04 container as an unprivileged desktop user.

- [ ] Install `electron-runtime-42`, LosslessCut, and draw.io together through
  APT dependency resolution—not isolated `dpkg --force-*` operations.
- [ ] Assert all three package states and exact dependency bounds.
- [ ] Assert draw.io owns no generic Electron/Chromium runtime file.
- [ ] Assert runtime files have one owner and application trees do not overlap.
- [ ] Assert Electron reports 42.9.3 as an unprivileged user.
- [ ] Run `drawio --version`/`--help` or the supported equivalent.
- [ ] Launch under Xvfb and capture startup exit status, logs, crash dumps, and a
  screenshot proving the editor reached a usable window.
- [ ] Repeat a launch under a Wayland compositor test harness when available.
- [ ] Fail on sandbox fallback, `--no-sandbox`, missing resource errors, ASAR
  integrity failures, GPU-process crash loops, or unexpected network updater
  traffic.
- [ ] Run lintian on binary and source changes, accepting only narrowly scoped
  documented overrides.

## Phase 4 — functional matrix

Use committed, freely redistributable fixtures and compare outputs with the
official 31.3.1 package where deterministic comparison is possible.

| Area | Required checks |
|---|---|
| Documents | Create, open, edit, save, save-as, reopen, and recover a `.drawio` file. |
| Imports | Open representative SVG, PNG-with-embedded-diagram, and supported Visio fixture. |
| Exports | GUI and CLI export to PNG, SVG, PDF, and JPEG; validate type, dimensions/pages, and embedded diagram data where applicable. |
| Multi-page | Create and export a multi-page diagram; verify page selection and PDF page count. |
| Desktop | Icon, menu entry, `StartupWMClass`, recent files, file picker, drag/drop, clipboard, and MIME double-click. |
| Printing | Open print preview and render to PDF in the test desktop. |
| Offline | Start, edit, and export with networking disabled. |
| Protocols | Exercise declared URL/custom protocol handlers with safe fixtures. |
| Security | Confirm sandboxed renderer processes and no updater installation path. |
| Coexistence | Run draw.io and LosslessCut concurrently against the same runtime. |

## Phase 5 — upgrade, rollback, and measurements

- [ ] Test a clean install of the app-only package.
- [ ] Test upgrade from upstream's official 31.3.1 `.deb`, recording conffile,
  desktop database, MIME database, and user-profile behavior.
- [ ] Test removal and purge without deleting user diagrams or preferences.
- [ ] Test runtime 42.9.3 rollback to the retained candidate only if dependency
  bounds permit it; otherwise verify APT refuses the unsafe downgrade cleanly.
- [ ] Test coordinated rollback to the monolithic draw.io package and runtime
  snapshot in a disposable repository.
- [ ] Measure standalone `.deb` and installed sizes for upstream and split builds.
- [ ] Measure incremental download and disk use for: draw.io alone; LosslessCut
  alone; and both installed together.
- [ ] Measure cold/warm launch and CLI export time against upstream, treating
  meaningful regressions as blockers rather than hiding them in averages.

## Phase 6 — candidate handoff (separate authorization required)

- [ ] Write the compatibility manifest: app version, tested runtime patch,
  dependency bounds, native modules, known limitations, and test results.
- [ ] Update the shared-runtime site mock/data model with a draw.io candidate
  page and measured savings, clearly marked unofficial.
- [ ] Prepare—but do not execute—a coordinated candidate publication of runtime
  42.9.3 and draw.io Desktop to `apt.wald3n.com/shared-electron`.
- [ ] Define a soak period, telemetry-free user feedback route, rollback owner,
  and severity thresholds.
- [ ] Only after explicit publication approval, promote the runtime and app
  together. ISO inclusion remains a separate decision.
- [ ] Once the runtime, LosslessCut, and draw.io form a verified installable
  candidate set, launch `https://biohack.net/shared-electron/` according to the
  [biohack.net launch plan](./2026-08-30-biohack-shared-electron-launch.md).

## Acceptance criteria

- [ ] draw.io 31.3.1 runs on `electron-runtime-42` 42.9.3 with normal sandboxing.
- [ ] Existing LosslessCut behavior passes on the upgraded runtime.
- [ ] The draw.io package contains no second generic Electron runtime.
- [ ] GUI editing plus PNG, SVG, PDF, and JPEG export pass; CLI export passes.
- [ ] Desktop/MIME integration, offline use, printing, and multi-page behavior
  pass on clean Ubuntu 26.04.
- [ ] Package/source metadata, licenses, lintian, repository gates, and clean APT
  installation pass.
- [ ] Upgrade, removal, concurrent use, and rollback behavior are documented and
  tested.
- [ ] Combined installation demonstrates a material disk/download saving versus
  two monolithic upstream packages.
- [ ] No ISO or production repository was changed without separate approval.

## Stop conditions

Stop and report rather than shipping if any of these remain true:

- the runtime requires `--no-sandbox`, root execution, or weakened Electron
  fuses;
- draw.io relies on a modified Electron binary not reproducible in the shared
  runtime;
- native ABI requirements cannot be represented by the package dependency;
- updater removal breaks startup or normal manual workflows;
- core editing/export behavior differs materially from upstream;
- redistribution or third-party license ownership is unclear; or
- advancing Electron 42 to 42.9.3 breaks LosslessCut.

## Deliverables

- `foundry-apt/packages/drawio-desktop/`
- updated `electron-runtime-42` 42.9.3 package and source artifacts
- automated three-package clean-install/sandbox test
- committed functional fixtures and export checks
- file-ownership and compatibility manifests
- size/performance comparison report
- publication/rollback runbook, left unexecuted

## Primary references

- [draw.io Desktop repository](https://github.com/jgraph/drawio-desktop)
- [draw.io Desktop releases](https://github.com/jgraph/drawio-desktop/releases)
- [draw.io Desktop package manifest](https://github.com/jgraph/drawio-desktop/blob/dev/package.json)
- [Shared Electron runtime repository and LosslessCut conversion](./2026-08-30-shared-electron-runtime-repository.md)
