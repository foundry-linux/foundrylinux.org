# Shared Electron runtime repository and LosslessCut conversion

**Date:** 2026-08-30

**Status:** proposed

**Scope:** Create an intentionally third-party Debian/Ubuntu APT repository of
shared, versioned Electron runtimes; prove it by converting LosslessCut from a
full Electron/Chromium bundle to an application package that depends on the
shared runtime; establish neutral project hosting, security ownership,
documentation, and a public compatibility catalogue.

---

## TL;DR

Electron applications for Debian-family systems usually ship a complete copy of
Electron and Chromium in every `.deb`, AppImage, or tarball. Debian does not
currently provide an Electron runtime, and carrying one in Debian proper has a
Chromium-sized security and maintenance burden. The result is avoidable disk and
download duplication across applications, slow security uptake, and packaging
that cannot share a patched browser engine.

Build an **unofficial, third-party shared Electron runtime repository** that is
explicitly outside Debian proper. Package upstream's Linux Electron distribution
as a major-versioned runtime, initially for Ubuntu 26.04 amd64. Application
packages contain their own application code, assets, native modules, and private
helpers, but depend on the runtime instead of embedding Electron and Chromium.

Do **not** attempt to package every historical Electron major. Start with one
currently supported line selected by the seed application cohort, then move
forward with Electron. Keep an older major only while an enrolled application
still requires it and only while the project can provide security updates for
it. The repository is a shared runtime service, not an Electron museum.

LosslessCut is the reference conversion. Its private FFmpeg remains in the
application package; only the generic Electron/Chromium runtime moves into the
shared dependency. Once the model is proven, evaluate other applications that
currently publish bundled Linux artifacts, beginning with draw.io Desktop and
MarkText because their released sources declare the same Electron 42 major as
LosslessCut 3.69.0.

## Desired outcome

```text
apt.wald3n.com/shared-electron (initial incubator repository)
  └── electron-runtime-<major>
        ├── Electron executable
        ├── Chromium snapshot/resources/locales
        ├── chrome-sandbox
        └── shared Electron libraries
             ▲
             │ exact-major dependency
             │
        app package
        ├── app.asar / application resources
        ├── app-specific native modules
        ├── app-specific private tools (LosslessCut: FFmpeg)
        ├── desktop integration
        └── launcher selecting the declared runtime major
```

Success is not merely a smaller LosslessCut `.deb`. The project must have a
credible answer for browser CVEs, Electron end-of-life, native-module ABI,
application compatibility, repository signing, provenance, rollback, ownership,
and the removal of unsupported runtime lines.

## Non-goals

- Inclusion in Debian proper.
- Rebuilding Chromium and Electron from source in the first release.
- One unversioned runtime that claims to run arbitrary Electron applications.
- Backfilling every previous Electron major.
- Replacing application-specific native modules or helper binaries with system
  copies merely to maximize deduplication.
- Packaging closed-source applications without explicit redistribution rights.
- Supporting Windows, macOS, Flatpak, Snap, RPM, or pacman in v1.
- Treating Fedora support as a prerequisite for multiple package formats. Fedora
  remains out of scope unless a separate community maintainer later adopts it.
- Claiming affiliation with the Electron project, OpenJS Foundation, Debian, or
  Ubuntu.
- Adding any new package to a Foundry ISO until the shared-runtime conversion has
  passed production soak testing.

## Versioned platform scope

### v1: prove the Debian-family service

v1 supports native `.deb` packages and APT only:

- Ubuntu 26.04 is the first production target and build ABI;
- Debian Stable is the first additional distribution target, built separately in
  a Debian container rather than relabeling an Ubuntu binary;
- Ubuntu derivatives may be compatibility-tested against the matching Ubuntu
  build, but are not automatically claimed as supported; and
- Arch is a reference implementation and collaboration target, not another
  runtime repository: validate app patches against Arch's existing versioned
  Electron packages where practical.

The concrete launch repository is
`https://apt.wald3n.com/shared-electron`, with suite `resolute`, component
`main`, and a signing identity distinct from `apt.foundrylinux.org`. Ubuntu
26.04 amd64 is the launch binary target. Kubuntu 26.04 and Foundry's Ubuntu 26.04
base use the same ABI build only after separate smoke tests. Debian Stable is a
post-launch target built natively and published under its own suite; Ubuntu
binaries must never be relabeled as Debian builds.

### v2: require package-format portability

Supporting an additional native package format is a **v2 requirement**, but no
particular distribution is mandatory. v2 is reached only when:

1. `.deb`/APT remains fully supported;
2. at least one second native package format is released for an independently
   maintained distribution family;
3. the runtime/app manifest, compatibility suite, version policy, SBOM, and
   provenance model are shared across formats rather than copied into unrelated
   pipelines; and
4. the new target has demonstrated user demand, a named primary and backup
   maintainer, security-update capacity, CI coverage, and a documented retirement
   policy.

The provisional second-format paths are:

| Path | What it proves | Position |
|---|---|---|
| Arch `PKGBUILD`/pacman app integration | The app/runtime contract works with an independent, already-versioned system Electron implementation | Preferred early cross-validation; do not duplicate Arch's runtime |
| openSUSE Tumbleweed RPM through OBS | The runtime itself and enrolled apps can be expressed outside Debian packaging | Provisional v2 native-format proof, contingent on maintainers and demand |
| Flatpak | The compatibility manifest and tests can inform an immutable, cross-distribution delivery model | Valuable later track, but does not alone satisfy the second **native** package-format requirement |
| Yocto/OpenEmbedded layer | The package-neutral runtime/app contract can be consumed by reproducible embedded product images, cross-toolchains, BSPs, licensing manifests, and SBOM workflows | Preferred embedded integration track after the desktop contract stabilizes; not a desktop package repository and not the v2 second-format proof by itself |

Fedora is not on the project roadmap. RPM support must be designed without
equating the RPM format with Fedora, and Fedora compatibility must never be
claimed from an openSUSE build. Yocto/OpenEmbedded replaces Gentoo as the planned
embedded integration target. NixOS, SteamOS/other immutable systems, Gentoo, and
additional distro families remain community-integration opportunities until they
independently satisfy the admission criteria above.

## Evidence and constraints

### Debian is not the initial destination

Debian's current package search has no `electron` runtime. This project should
not weaken Debian's source, security, or embedded-code policies to force an
upload. It instead creates a clearly labeled third-party repository whose users
know they are accepting upstream binary Electron builds and this project's
security/update policy.

That distinction must remain visible in the package metadata and website:

- `Origin` and `Label` identify this repository, not Debian;
- packages use a repository-specific version suffix;
- documentation never says “available in Debian”;
- the site explains that the initial runtime is an upstream-binary repack; and
- a future from-source build is a separate milestone, not implied by the first
  release.

### Electron compatibility is versioned

Electron's official policy uses an eight-week major cadence and supports the
latest three stable major lines. As of this investigation, Electron's release
site lists stable majors 44, 43, and 42. That policy creates two requirements:

1. security support is a moving window, not a permanent promise; and
2. packages must identify the Electron major they were tested against.

Native Node modules are an additional hard boundary. Electron documents that its
Node ABI differs from ordinary Node because of components such as BoringSSL, and
native modules must be rebuilt for the target Electron version. Application
packages containing `.node` modules therefore belong to an exact runtime-major
compatibility set and must be rebuilt/tested when moving majors.

### The first cohort already clusters

Released upstream `package.json` files inspected on 2026-08-30:

| Application | What it does | Released version inspected | Declared Electron | Linux bundled artifacts | Initial fit |
|---|---|---:|---:|---|---|
| [LosslessCut](https://github.com/mifi/lossless-cut) | Cuts, joins, and remuxes video/audio without re-encoding | 3.69.0 | `^42.3.0` | AppImage + tarball | Reference conversion |
| [draw.io Desktop](https://github.com/jgraph/drawio-desktop) | Creates flowcharts, architecture diagrams, and other technical diagrams | 31.3.2 | `^42.8.0` | `.deb`, RPM, AppImage | Best second proof |
| [MarkText](https://github.com/marktext/marktext) | Edits Markdown in a visual, distraction-free desktop editor | 0.19.1 | `^42.1.0` | `.deb`, RPM, Snap, AppImage, tarball | Same-major candidate |
| [Signal Desktop](https://github.com/signalapp/Signal-Desktop) | Provides end-to-end encrypted messaging and calls from the desktop | 8.25.0 | `43.4.1` | Signal-operated APT repository for Debian-based systems + AppImage | Current-major candidate, high complexity |
| [Joplin](https://github.com/laurent22/joplin) | Stores and synchronizes Markdown notes, notebooks, and to-do lists | 3.6.16 | `40.8.3` | `.deb` + AppImage | Candidate only after an upstream Electron move |
| [Element Desktop](https://github.com/element-hq/element-desktop) | Chats and collaborates over the decentralized Matrix network | 1.12.13 | `40.6.1` | upstream Linux packaging | Candidate only after an upstream Electron move |
| [Beekeeper Studio](https://github.com/beekeeper-studio/beekeeper-studio) | Browses, queries, and edits SQL databases through a desktop GUI | 6.0.5 | `39.8.10` | `.deb`, RPM, AppImage | Candidate only after an upstream Electron move |
| [balenaEtcher](https://github.com/balena-io/etcher) | Flashes operating-system images to USB drives and SD cards | 2.1.6 | `37.2.4` | `.deb`, RPM, Linux zip | Too old for initial runtime support |

The table identifies prospects, not permission to redistribute them. Each app
still needs a license, trademark, build, native-module, updater, and upstream
packaging audit before enrollment.

## Version-support policy

### Launch policy: one line, forward-only

Do not launch with all eight majors shown in the research table.

1. **Attempt the newest supported stable major first.** At implementation time,
   pin the newest Electron stable and test LosslessCut against it. Today that is
   major 44; it must be re-checked when work begins.
2. If LosslessCut needs source changes to move from its declared major 42, carry
   a small forwardable compatibility patch and open it upstream.
3. Only if the port cannot be completed in the phase's time box may the first
   release use Electron 42, which is still in the current three-major support
   window at plan time. Record an expiry date and an Electron-44 migration issue
   before publishing it.
4. Add future majors as the supported window advances or as a real application
   requires them.
5. Do not add an old major merely because an interesting app still bundles it.
   The app must first move to a supported major upstream or through a maintainable
   forward patch.

### Retention and retirement

- Runtime packages are parallel-installable by major:
  `electron-runtime-44`, `electron-runtime-45`, and so on.
- Applications depend on exactly one major range, for example:
  `electron-runtime-44 (>= 44.0), electron-runtime-44 (<< 45)`.
- An optional `electron-runtime` convenience metapackage points to the preferred
  current major. Application packages never depend on that moving metapackage.
- Support at most the current upstream-supported majors, and preferably fewer.
- Retain an older supported major only while at least one published application
  depends on it.
- When upstream ends support for a major, publish a dated end-of-support notice.
  Either migrate every dependent app or remove those apps and the runtime
  together. Never leave a browser runtime silently frozen.
- Repository metadata exposes `X-Electron-Major`, `X-Electron-Chromium-Version`,
  `X-Electron-Upstream-Version`, and `X-Support-Ends` for automation and the site.

This policy bounds storage and work. A major costs one runtime payload plus
symbols/source artifacts, not one runtime per app; historical majors do not
accumulate indefinitely.

## Package design

### Runtime source package

Provisional source package: `electron-runtime-44` (major substituted at build
time only in generation tooling; each published source package is explicit).

Binary packages:

| Package | Contents |
|---|---|
| `electron-runtime-44` | Runtime executable, resources, locales, sandbox, libraries |
| `electron-runtime-44-dbgsym` | Automatically generated detached debug symbols where useful |
| `electron-runtime` | Optional moving metapackage depending on the preferred major |

Install layout:

```text
/usr/lib/electron-runtime/44/electron
/usr/lib/electron-runtime/44/chrome-sandbox
/usr/lib/electron-runtime/44/chrome_*.pak
/usr/lib/electron-runtime/44/icudtl.dat
/usr/lib/electron-runtime/44/locales/
/usr/lib/electron-runtime/44/resources/
/usr/lib/electron-runtime/44/snapshot_blob.bin
/usr/lib/electron-runtime/44/v8_context_snapshot.bin
/usr/bin/electron44 -> ../lib/electron-runtime/44/electron
```

The real file inventory must be derived from and tested against the pinned
upstream Electron zip. Do not assume the sample list remains constant.

### Build provenance

Phase 1 may repackage Electron's official signed/published Linux archive because
the repository is explicitly third-party. The build must:

- pin the exact upstream version and SHA-256;
- record the Electron, Chromium, Node, V8, and module ABI versions;
- verify upstream checksums/signatures where Electron publishes them;
- generate an SPDX or CycloneDX SBOM;
- record the upstream release URL and Git commit;
- build the Debian source and binary package in Ubuntu 26.04 containers;
- emit in-toto/SLSA-compatible provenance from CI where practical;
- publish the original archive and Debian packaging source references; and
- never describe the result as a Debian-from-source build.

A later source-build track may investigate Electron's official build process,
Chromium toolchain size, codecs, and reproducibility. It must not block the
shared-runtime proof.

### Container-build goals and Signal comparison

Signal Desktop's public Linux reproducible-build harness is the closest useful
reference for the application side of this project. It pins a dated Ubuntu
image by digest, derives `SOURCE_DATE_EPOCH` from the source commit, pins Node
and pnpm, installs with a frozen lockfile, builds as the invoking user, and
documents SHA-256 comparison with Signal's official `.deb`. Signal explicitly
labels Linux reproducibility experimental. Its harness targets one complete app;
it does not design or publish a shared runtime.

Our current Foundry build has a different strength: it builds against the real
Ubuntu 26.04 ABI and performs a guarded multi-package APT publication with a
complete-dist gate, separate signing step, staged metadata upload, durable
artifact mirror, and live repository consistency check. It is containerized but
not yet reproducible: the `ubuntu:26.04` tag and APT packages float, timestamps
are not normalized, and the LosslessCut proof resolves ASAR tooling through
`npx` without a committed dependency lockfile.

The full goal-by-goal table and evidence are in
[`docs/investigations/2026-08-30-signal-vs-shared-electron-container-builds.md`](../investigations/2026-08-30-signal-vs-shared-electron-container-builds.md).
The resulting requirements are:

- pin the Ubuntu 26.04 toolchain image by digest while retaining Resolute ABI
  generation;
- construct a reviewed, versioned build image instead of installing the latest
  archive packages inside every package build;
- derive and pass `SOURCE_DATE_EPOCH` from the packaging commit;
- replace build-time ad hoc `npx` resolution with a committed lockfile and
  verified/offline tool cache;
- run two isolated clean builds and require byte-identical outputs, using
  `diffoscope` on failure;
- run builds as the caller UID/GID where possible rather than repairing
  root-owned outputs;
- publish image/input/output hashes, SBOM, provenance, and test evidence;
- keep signing and promotion behind a protected job/environment; and
- preserve the existing repository completeness, staged upload, live
  consistency, durable mirror, and rollback gates.

### Sandbox ownership

`chrome-sandbox` is shared runtime state, not application state. The runtime
package owns it as `4755 root:root`, with lintian justification and an install
test that proves the SUID sandbox is actually selected. Application packages do
not ship their own sandbox helper and do not launch with `--no-sandbox`.

### Dependencies and codecs

Use `dpkg-shlibdeps` against the runtime executable and libraries to produce
versioned system-library dependencies. Audit:

- GTK, NSS, NSPR, CUPS, DBus, ALSA, ATK/AT-SPI, GBM, X11, Wayland, and glibc;
- proprietary-codec status and whether the official upstream binary enables
  anything the project cannot redistribute;
- crash reporter behavior and endpoints;
- update mechanisms that belong to upstream app bundles rather than the shared
  runtime; and
- Widevine or other separately licensed components, which must not be bundled by
  default.

## Application package contract

Every enrolled app gets a machine-readable manifest, for example:

```yaml
application: losslesscut
upstream_version: 3.69.0
electron_major: 44
electron_modules: 149
entrypoint: /usr/lib/losslesscut/app.asar
native_modules: []
private_helpers:
  - ffmpeg
  - ffprobe
runtime_flags: []
tested:
  - x11
  - wayland
```

The packaging linter verifies:

- the exact runtime dependency matches the manifest;
- no second Electron executable or generic Chromium payload remains;
- all `.node` files report the expected ABI or are rebuilt for it;
- `process.resourcesPath`, `app.getAppPath()`, updater paths, crash paths, and
  helper discovery work in the split layout;
- desktop files and URL handlers invoke the wrapper, not a bundled executable;
- the runtime's sandbox is used; and
- the app does not silently download another Electron runtime on first launch.

Provide a small `electron-app-launch` helper only if it adds enforceable behavior
(manifest validation, runtime selection, diagnostics). A shell wrapper that
merely saves one line is not a new framework.

## LosslessCut reference conversion

### Current state

`losslesscut 3.69.0-1foundry1` repacks the official AppImage payload. Its binary
package is about 164 MB (157 MiB), with an installed size of about 588 MB
(574 MiB). It bundles both:

- the generic Electron/Chromium application runtime; and
- LosslessCut's private FFmpeg 8 build and application resources.

The FFmpeg payload is application-specific and remains private to LosslessCut.
The shared-runtime work removes only Electron's generic payload.

### Conversion sequence

1. Produce a byte-level inventory of the current AppDir and classify every file
   as runtime, application, private helper, native module, desktop integration,
   license, or unknown.
2. Obtain the official Electron archive for the app's declared version and
   compare hashes/ELF build IDs against the files in the AppImage. Record
   upstream modifications instead of assuming the embedded runtime is vanilla.
3. Attempt to build/run LosslessCut's application resources against the newest
   supported runtime major. Patch and upstream API incompatibilities where
   practical.
4. Install only `app.asar`, `app.asar.unpacked`, application assets, licenses,
   and private FFmpeg/ffprobe under `/usr/lib/losslesscut/`.
5. Change `Depends:` from the large `${shlibs:Depends}` runtime closure alone to
   include the exact `electron-runtime-<major>` package. Keep direct dependencies
   only for libraries loaded by LosslessCut-specific native code/helpers.
6. Launch with the runtime executable and explicit application entrypoint. Do
   not set `LD_LIBRARY_PATH` globally.
7. Remove LosslessCut's copy of `chrome-sandbox`, Chromium packs/locales,
   Electron executable, snapshots, SwiftShader/runtime libraries, and other
   files proven to be owned by the runtime package.
8. Disable or redirect electron-builder's self-updater if it assumes upstream's
   monolithic artifact. APT owns updates for both packages.
9. Preserve the existing `$ORIGIN`-only FFmpeg RPATH repair and verify the
   private FFmpeg remains isolated from the runtime.
10. Bump the Foundry package revision, add `Breaks/Replaces` only if a path
    transition actually requires them, and test upgrade from
    `3.69.0-1foundry1` without leftover runtime files.

### LosslessCut verification

- Compare old/new `.deb` download and installed sizes.
- Install on clean Ubuntu 26.04 from only the new third-party runtime repo plus
  the Foundry app repo.
- Launch on Wayland and X11.
- Verify renderer sandbox processes and absence of `--no-sandbox`.
- Open media; mark in/out; export by stream copy; merge segments; snapshot a
  frame; and exercise metadata editing.
- Run LosslessCut's startup FFmpeg and ffprobe probes.
- Verify file dialogs, drag/drop, notifications, clipboard, portals, GPU
  acceleration, audio, keyboard shortcuts, URL/file associations, and taskbar
  grouping.
- Run with a hostile current directory to repeat the RPATH security test.
- Upgrade the runtime within the same major and rerun the suite without
  rebuilding LosslessCut.
- Remove LosslessCut while retaining another app on the runtime, then remove the
  last app and confirm APT can autoremove the runtime normally.

Do not replace the live Foundry package until the split package passes all of
these checks. Keep the current monolithic package as the rollback artifact.

## Candidate application programme

### Tier 1: same-major proof candidates

After LosslessCut, evaluate **draw.io Desktop** first and **MarkText** second.
Their released sources declare Electron 42, giving immediate evidence about
whether one runtime can serve unrelated applications. The evaluation includes:

- source and redistribution license;
- exact embedded Electron version and modifications;
- native `.node` modules and ABI;
- updater assumptions;
- app protocol/file handlers;
- codecs/private helpers;
- current Ubuntu/Debian/third-party packaging;
- upstream willingness to accept split-runtime patches; and
- size saved by deduplication.

If the project launches on Electron 44, test these applications on 44 rather
than adding 42 automatically. A Tier-1 app that cannot move must justify a
second supported major or remain bundled upstream.

### Tier 2: future-current candidates

- **Signal Desktop** is already on Electron 43 but has security-sensitive native
  modules and its own update/repository model. Treat it as a compatibility and
  governance partnership, not a quick repack.
- **Joplin** and **Element Desktop** are on Electron 40 in the inspected releases.
  Revisit after they move into the supported window.
- **Beekeeper Studio** (39) and **balenaEtcher** (37) are not reasons to resurrect
  unsupported majors. Track upstream upgrades only.

#### How to leverage Signal's existing Debian-package work

“Signal-operated APT repository” does **not** mean Signal Desktop is in Debian's
official archive. Signal signs and publishes its own repository at
`updates.signal.org` for Debian-based distributions. Its source config tells
`electron-builder` to produce an amd64 `.deb`, and the official install flow
adds Signal's signing key and APT source before installing `signal-desktop`.

This is still unusually useful to the shared-runtime project:

1. **Reproducible upstream baseline.** Signal publishes a containerized
   reproducible-build procedure and documents comparing its output byte-for-byte
   with the official `.deb` obtained through APT. Use that procedure to establish
   an upstream-identical baseline before changing runtime ownership.
2. **Machine-readable package recipe.** Its public `package.json` records the
   Electron version, `electron-builder` Debian target, minimum glibc, system
   dependencies, desktop/protocol metadata, extra polkit policy resources, and
   native-module rebuild step. Translate those fields into the candidate
   manifest rather than reverse-engineering the package from scratch.
3. **Authoritative bundle oracle.** Download—but do not republish—the official
   `.deb` in CI, verify it with Signal's repository signature, inventory the
   embedded Electron tree, and compare that tree with the corresponding official
   Electron runtime. This detects Signal-specific runtime changes before a split
   is attempted.
4. **High-value compatibility fixture.** Produce an unpublished experimental
   package from the reproducible build, remove only the generic Electron files,
   inject the exact versioned runtime dependency, and run Signal's tests plus
   launch, link/device, notification, protocol-handler, polkit, sandbox, upgrade,
   and encrypted-database migration tests. Never exercise real user accounts or
   copy user data into CI.
5. **Upstream-ready evidence.** If the experiment passes, send Signal a small,
   auditable proposal: measured savings, exact file split, reproducible diffs,
   native-module ABI results, security-update implications, and rollback design.
   The best outcome is an upstream build mode that emits an app-only package;
   maintaining a permanently divergent Signal package is the fallback, not the
   goal.

What cannot be reused automatically:

- Signal's signing key, APT origin, release authority, trademarks, or updater;
- the official package name for a public third-party build without naming and
  coexistence review;
- an official `.deb` with its Electron files deleted—the signature and package
  checksums would no longer describe the artifact;
- native `.node` modules across Electron majors without rebuilding and testing;
  or
- Signal's security assurances for a package released by this project.

Accordingly, classify Signal first as a **non-published compatibility target**.
Only enroll it publicly after Signal cooperates or after legal/trademark review,
a distinct third-party package identity, an independent update policy, and a
security test programme are in place. Its APT repository is a model and a source
of verified inputs, not a dependency repository that can be composed with ours.

### Candidate discovery automation

Create a scheduled scanner that reads public release metadata and source
manifests for an allowlisted set of applications, recording:

- application and release version;
- declared and embedded Electron versions;
- Linux artifact formats and sizes;
- license;
- native module count/ABI;
- last release/activity;
- existing distro packages; and
- runtime support-window status.

The scanner proposes candidates; it never republishes software automatically.

## Repository and organization design

### Neutral project identity

The runtime should not live permanently inside `foundry-linux/foundry-apt`.
That would make a general dependency appear specific to one distribution and
discourage other packagers from relying on it.

Provisional organization: **`electron-packaging`**. Public GitHub API checks on
2026-08-30 returned 404 for both the user and organization handles, and exact
repository searches found no established project. Availability must be checked
again at creation time. Because “Electron” is an existing project/trademark,
obtain naming guidance and display **Unofficial community packaging** prominently.

Provisional repositories:

```text
electron-packaging/runtime-deb       runtime packaging + CI + tests
electron-packaging/apps              app manifests, compatibility data, helpers
electron-packaging/website           public site and documentation
electron-packaging/infra             DNS, R2, signing, release automation
```

Prefer a monorepo for the first proof if four repositories would create ceremony
without independent release needs:

```text
electron-packaging/repository
  runtime/
  apps/
  site/
  infrastructure/
```

Split only when permissions, release cadence, or contributor ownership justify
it. Never call the organization `electron`, `electronjs`, `debian-electron`, or
anything implying official Electron or Debian ownership.

### Incubation and transfer

If a neutral organization cannot be created immediately, incubate the proof in
`foundry-linux/electron-runtime` with an explicit planned transfer. GitHub
repository transfer preserves issue and clone redirects, but APT origins,
domains, signing policy, and user trust are harder to move. Therefore create the
neutral signing identity and domain before the first public repository release,
even if early code review happens in Foundry infrastructure.

## APT repository and infrastructure

Provisional public endpoint: `apt.electronpackaging.org`. Confirm the parent
domain through RDAP/registrar and trademark review before use.

Repository properties:

- Ubuntu 26.04 (`resolute`) first; amd64 first, arm64 after a real app test;
- signed `InRelease`, `Release`, `Packages`, and `Sources` indexes;
- `Origin: Electron Packaging` and `Label: Electron Packaging (Unofficial)`;
- separate stable and candidate channels, not a single mutable pool;
- Cloudflare R2 or equivalent object storage with versioning and immutable
  release snapshots;
- declarative bucket, DNS, key, secret, and CI setup;
- no credentials in local files or repository history;
- per-package targeted publishing with a complete-index gate and durable mirror;
- signed checksums, SBOMs, provenance, and retained rollback artifacts;
- a documented repository-removal procedure; and
- machine-readable support/EOL metadata consumed by the website.

Do not reuse the Foundry APT signing key. This runtime repository has a different
trust boundary, security policy, and potential contributor group.

## Security programme

This is the actual product. Deduplication without faster security updates is not
worth creating a new browser-runtime supplier.

### Required ownership

- Named primary and backup security maintainers.
- Private vulnerability-reporting channel with published response expectations.
- GitHub private vulnerability reporting/security advisories enabled.
- Dependabot-style version detection is insufficient; monitor Electron releases,
  Chromium CVEs, Electron advisories, and upstream support dates.
- A patch/release SLA, proposed as 48 hours for actively exploited/high-severity
  runtime fixes and seven days for ordinary supported-line updates.
- A documented emergency yanking/rollback process.
- No runtime major remains published as “supported” without an owner.

### Automated gates

- Daily upstream version and support-window check.
- CVE/advisory ingestion mapped to Electron and Chromium versions.
- Rebuild and regression-test all enrolled apps on every runtime patch update.
- ABI and file-manifest diff against the previous runtime.
- Sandbox mode, GPU, Wayland, X11, portal, audio, and smoke tests.
- Malware scanning and secret scanning of upstream archives and produced files.
- SBOM diff and license-policy check.
- Canary publication before stable promotion.
- Website status changes atomically with repository promotion.

## Website design

Build a standalone Astro + Tailwind 4 site. Its purpose is trust and operational
clarity, not generic marketing.

### Canonical home and the existing sites

A public release warrants a new, neutral domain. The runtime repository is a
general ecosystem service with its own signing key, security response, support
window, and failure modes; making either an app studio or a distribution site
canonical would blur that trust boundary. The provisional
`electronpackaging.org` name is a working label only. Before registration,
perform domain, organization-name, trademark, and search-confusion checks and
prefer a distinct name if using “Electron” would imply upstream affiliation.

The sites should have deliberately different roles:

| Surface | Role | Must not become |
| --- | --- | --- |
| New neutral domain | Canonical install instructions, signing-key fingerprint, security policy, runtime/app support matrix, status, and advisories | A generic studio portfolio |
| `apt.<new-domain>` | Signed APT repository and repository metadata | An alias of the Foundry or Indri repository |
| `indri.studio/apps/shared-electron/` | Polished project/product page with screenshots, app catalogue, and measured savings, linking to canonical package/security state | A second copy of installation or security documentation |
| `biohack.net/shared-electron/` | Technical origin story, architecture, and implementation report | The package origin or operational status source |
| `wald3n.com/open-source` | Contribution/project listing and optional case study, linking to the canonical site | The package origin or operational status source |
| `foundrylinux.org` (and any successor distribution site) | Consumer-facing integration notes and edition/package availability | The canonical home for a cross-distribution dependency |
| Neutral GitHub organization | Source, issues, releases, advisories, governance, and CI | A repository whose identity implies official Electron or Debian ownership |

All discovery pages should consume or link to machine-readable canonical
metadata rather than hand-copying versions, support status, keys, or commands.
Use redirects for any defensive/alternate domains acquired; never operate two
apparently equal project homes.

Do not buy or launch a new domain during the feasibility proof. Make it a gate
before the first public candidate release: if the runtime and converted
LosslessCut package pass the technical proof and the service has credible
security ownership, clear and register the neutral identity, establish its
independent signing key, and then publish the site. If the proof stops as an
internal experiment, retain it under Foundry infrastructure and describe it on
the existing sites without creating another abandoned public trust endpoint.

### Information architecture

```text
/
  What the shared runtime is, current supported major, aggregate space saved,
  latest security update, and an explicit Unofficial badge

/install
  Key fingerprint, deb822 source setup, channel choice, removal and rollback

/runtimes
  Electron/Chromium/Node versions, architectures, status, EOL, changelog, SBOM

/apps
  Compatibility matrix: app version → runtime major → test status → package repo

/apps/losslesscut
  Reference conversion, size comparison, test evidence, known deviations

/security
  Threat model, update SLA, advisories, reporting process, supported window

/developers
  Packaging contract, manifest schema, native-module handling, enrollment guide

/governance
  Maintainers, decision process, funding, trademark/non-affiliation statement

/status
  Last repository publish, index integrity, CI health, current incidents
```

### Visual direction

Do not borrow Electron's atom logo, cyan palette, or site composition. Use a
neutral package-infrastructure identity: runtime layers, dependency graphs, and
deduplicated blocks as the visual language. The most important homepage visual
is a small, honest before/after relationship:

```text
Before:  App A + Electron | App B + Electron | App C + Electron
After:   App A | App B | App C  →  shared Electron runtime
```

Show exact measured bytes, not illustrative “up to” claims. Every compatibility
badge links to test evidence and a timestamp.

### Homepage mockups

The desktop homepage should expose trust and current state before explanation or
branding. This is a functional wireframe, not a final name or visual identity:

```text
┌──────────────────────────────────────────────────────────────────────────────┐
│ [layer mark] PROJECT NAME     Runtimes  Apps  Security  Docs  Status  GitHub │
│               UNOFFICIAL COMMUNITY PACKAGING                                │
├──────────────────────────────────────────────────────────────────────────────┤
│                                                                              │
│  One Electron runtime.                    ┌─ CURRENT STATUS ────────────────┐ │
│  More than one application.               │ ● Repository operational       │ │
│                                           │ Electron 42 · amd64            │ │
│  Versioned, shared Electron packages      │ Updated 30 Aug 2026            │ │
│  for Debian-family systems, with tested   │ 1 enrolled app · 0 incidents  │ │
│  application compatibility.               └────────────────────────────────┘ │
│                                                                              │
│  [ Install the repository ]  [ How it works ]                                │
│                                                                              │
│  Not affiliated with Electron or Debian.                                     │
├──────────────────────────────────────────────────────────────────────────────┤
│  BEFORE                              AFTER                                   │
│  ┌───────┐ ┌───────┐ ┌───────┐      ┌─────┐ ┌─────┐ ┌─────┐                │
│  │ App A │ │ App B │ │ App C │      │ A   │ │ B   │ │ C   │                │
│  │ + 42  │ │ + 42  │ │ + 42  │      └──┬──┘ └──┬──┘ └──┬──┘                │
│  └───────┘ └───────┘ └───────┘         └────────┼────────┘                   │
│                                             ┌────▼────────────┐              │
│  000 MB installed                           │ Electron 42     │              │
│  per measured package set                   │ shared runtime  │              │
│                                             └─────────────────┘              │
├──────────────────────────────────────────────────────────────────────────────┤
│  TESTED APPLICATIONS                                                          │
│  LosslessCut 3.69.0  → Electron 42  [PASS · evidence]  [package details]     │
│  Draw.io             → evaluating          MarkText → evaluating             │
├──────────────────────────────────────────────────────────────────────────────┤
│  SECURITY FIRST                                                               │
│  Supported versions, patch SLA, advisories, SBOM and reproducible provenance │
│  are part of the package—not an afterthought.            [Security policy →] │
└──────────────────────────────────────────────────────────────────────────────┘
```

On mobile, the current security state and non-affiliation notice remain above
the fold; the dependency diagram becomes vertical instead of shrinking:

```text
┌──────────────────────────────┐
│ [mark] PROJECT       [menu]  │
│ UNOFFICIAL COMMUNITY PROJECT │
├──────────────────────────────┤
│ One Electron runtime.        │
│ More than one application.   │
│                              │
│ [ Install repository ]       │
│ [ How it works ]             │
│                              │
│ ● OPERATIONAL                │
│ Electron 42 · amd64          │
│ Updated 30 Aug 2026          │
│                              │
│ Not affiliated with          │
│ Electron or Debian.          │
├──────────────────────────────┤
│ BEFORE                       │
│ [App + 42] [App + 42]        │
│                              │
│ AFTER                        │
│ [App] [App]                  │
│    ╲   ╱                     │
│ [shared Electron 42]         │
│ Exact measured saving: 000MB │
├──────────────────────────────┤
│ LosslessCut 3.69.0           │
│ Electron 42  [PASS]          │
│ [Test evidence →]            │
└──────────────────────────────┘
```

The runtime detail page should use a compact release ledger rather than a
marketing card grid:

```text
Electron 42  [SUPPORTED]   Chromium …   Node …   V8 …
Published …  Security patch …  EOL …    amd64 [PASS]  arm64 [PLANNED]

Enrolled apps       Version       Result       Tested        Evidence
LosslessCut         3.69.0        PASS         timestamp     logs / manifest

[Install 42] [Changelog] [SBOM] [Provenance] [Report a vulnerability]
```

Replace every placeholder version, date, byte count, and component version from
signed generated metadata. A stale placeholder must fail the production build.

### High-fidelity visual specification

Treat the canonical site as sober package infrastructure with a tactile,
layered identity. It should feel closer to a release console than a startup
landing page, while remaining readable to people who only want to install an
application.

```text
Canvas          #0B0D10    near-black graphite
Panel           #12161C    raised surface
Panel strong    #181E26    table headers and active rows
Rule            #29313D    1px separators
Text            #F3F6FA    primary
Text muted      #9BA7B5    secondary; never below WCAG AA
Healthy         #6EE7A8    status only
Warning         #F4C76B    candidate/EOL warning only
Critical        #FF7575    advisories/incidents only
Action          #8CB4FF    links and primary actions
Layer accents   #A995FF / #70D6FF / #6EE7A8

Display face    IBM Plex Sans Condensed, 700, -0.025em
Body/interface  Inter, 400/500/650
Data/code       IBM Plex Mono, 400/600
Content width   1180px; 24px desktop gutters; 16px mobile gutters
Radius          6px controls, 10px panels; no pill-shaped marketing cards
Shadow          0 18px 50px rgb(0 0 0 / 24%); panels also retain a visible rule
Motion          140–180ms; no ambient animation; respect reduced-motion
```

The provisional layer mark is three offset, outlined rectangles sharing one
solid lower layer. It conveys deduplication without borrowing Electron's atom.
Use it as an inline SVG generated in the site repository, not an image asset.

#### Canonical homepage — desktop, 1440 × 1100 crop

```text
┌────────────────────────────────────────────────────────────────────────────────────┐
│  ▱  COMMON RUNTIME     Runtimes  Applications  Security  Developers  Status   GitHub│ 72
│     UNOFFICIAL                                                                    │
├────────────────────────────────────────────────────────────────────────────────────┤
│                                                                                    │
│  VERSIONED ELECTRON PACKAGING                           LIVE REPOSITORY              │
│                                                                                    │
│  Stop shipping the browser                 ╭────────────────────────────────────╮  │
│  with every desktop app.                   │  ● OPERATIONAL          STABLE      │  │
│                                            │                                    │  │
│  A security-maintained shared Electron     │  ELECTRON 42                       │  │
│  runtime for applications that Debian      │  42.3.0 · amd64                    │  │
│  cannot carry cleanly today.               │                                    │  │
│                                            │  Chromium 142…   Node 22…           │  │
│  [ Add repository → ] [ Read the model ]   │  Updated 2h ago · no incidents     │  │
│                                            │  [View runtime ledger →]            │  │
│  Not affiliated with Electron or Debian.   ╰────────────────────────────────────╯  │
│                                                                                    │
├────────────────────────────────────────────────────────────────────────────────────┤
│  ONE RUNTIME, MEASURED                                      VERIFIED 30 AUG 2026   │
│                                                                                    │
│  BEFORE                                  AFTER                                      │
│  ┌──────────────────────┐                ┌─────────┐ ┌─────────┐ ┌─────────┐         │
│  │ LosslessCut          │                │ App A   │ │ App B   │ │ App C   │         │
│  │ app 000 MB           │                └────┬────┘ └────┬────┘ └────┬────┘         │
│  │ Electron 000 MB      │                     ╰───────────┼───────────╯              │
│  └──────────────────────┘                           ╭──────▼────────────╮             │
│  repeated for every app                             │ Electron 42      │             │
│                                                     │ 000 MB once      │             │
│  ███████████████████████  old                       ╰───────────────────╯             │
│  ████████████             shared      000 MB saved across tested set                 │
├────────────────────────────────────────────────────────────────────────────────────┤
│  COMPATIBILITY LEDGER                                      [All applications →]     │
│  ┌──────────────────┬────────────┬─────────────┬──────────────┬───────────────────┐  │
│  │ APPLICATION      │ VERSION    │ RUNTIME     │ RESULT       │ LAST VERIFIED     │  │
│  ├──────────────────┼────────────┼─────────────┼──────────────┼───────────────────┤  │
│  │ LosslessCut      │ 3.69.0     │ Electron 42 │ ● PASS       │ 30 Aug · evidence │  │
│  │ draw.io Desktop  │ evaluating │ —           │ ◌ RESEARCH   │ candidate report  │  │
│  │ MarkText         │ evaluating │ —           │ ◌ RESEARCH   │ candidate report  │  │
│  └──────────────────┴────────────┴─────────────┴──────────────┴───────────────────┘  │
├────────────────────────────────────────────────────────────────────────────────────┤
│  SECURITY IS THE PRODUCT                                                            │
│  48h critical patch target       Signed metadata       SBOM + provenance            │
│  [Security policy] [Advisories] [Status history] [Repository signing key]            │
└────────────────────────────────────────────────────────────────────────────────────┘
```

The hero's status panel is populated from the same signed release manifest as
`/status`. On stale or failed metadata it turns warning/critical and replaces
the install CTA with “Read incident”; presentation must never mask repository
state.

#### Install page — command and trust flow

```text
ADD THE STABLE REPOSITORY                                      Step 1 of 3

1  VERIFY IDENTITY          2  ADD SOURCE           3  INSTALL AN APP
●───────────────────────────○───────────────────────○

Signing key fingerprint
8F3A 0000 0000 0000  0000 0000 0000 0000 0000 0000    [Copy]
[Download key] [Independent fingerprint locations]

Ubuntu 26.04 · amd64 · stable
╭─ shell ─────────────────────────────────────────────────────────── [Copy] ╮
│ curl … | gpg --dearmor …                                                │
│ sudo install …                                                          │
│ sudo tee /etc/apt/sources.list.d/…                                      │
╰──────────────────────────────────────────────────────────────────────────╯

What this grants trust to  ·  How to remove it  ·  Candidate channel
```

Do not present an unreviewable one-line `curl | sudo sh` installer. Separate
key acquisition, fingerprint verification, source creation, and package
installation, with an explanation beside each copyable block.

#### LosslessCut application page

```text
Applications / LosslessCut                                      ● VERIFIED

[LosslessCut icon]  LosslessCut 3.69.0
Swiss-army knife for lossless video and audio editing

PACKAGE                         SHARED DEPENDENCY
losslesscut 3.69.0-1…     ───▶  electron-runtime-42 42.3.0-1…
318 MB installed                000 MB installed once

TEST MATRIX
Launch  PASS   Open media  PASS   Preview  PASS   Cut/export  PASS
FFmpeg 8 private payload  PASS   Wayland  PASS   X11  PASS   Sandbox  PASS

Measured 30 Aug 2026 on Ubuntu 26.04 amd64
[Full test evidence] [Package manifest] [SBOM] [Known differences]

Why FFmpeg is not shared
LosslessCut's codec build is application-specific; this project deduplicates
Electron without silently replacing application media capabilities.
```

#### Existing-site discovery treatments

These are native components on existing sites, not miniature copies of the
canonical homepage.

`indri.studio` may carry a full portfolio tile and project detail lead-in:

```text
┌──────────────────────────────────────────────────────────────┐
│ OPEN INFRASTRUCTURE                                          │
│ ▱  Common Runtime                                            │
│    Shared, versioned Electron packaging for desktop apps.    │
│                                                              │
│    ● Repository operational   Electron 42   1 verified app   │
│    [Visit project ↗]  [View source ↗]                         │
└──────────────────────────────────────────────────────────────┘
```

`wald3n.com/open-source` should use its existing contribution/project-row
language and remain compact:

```text
Common Runtime   Maintained project · Packaging infrastructure
Versioned shared Electron runtime; LosslessCut reference package.
[Project ↗] [GitHub ↗]                                      ● operational
```

The distro site should show only consumer integration where LosslessCut is
listed:

```text
LosslessCut 3.69.0       Foundry Sprite
Uses electron-runtime-42 from the Common Runtime repository.  [Why two repos?]
[Install Sprite] [Package details ↗]
```

Each component fetches a tiny cacheable status JSON from the canonical domain.
If it cannot be fetched, show no health assertion—display “Status unavailable”
and retain the link. Never let an old green dot survive a failed refresh.

### Build and deployment

- Astro and Tailwind 4 from the first commit.
- Content generated from signed repository metadata and app manifests, not a
  second hand-maintained database.
- Static deployment unless authenticated/security workflows create a real SSR
  need.
- Accessibility, responsive layout, RSS/Atom security feed, and JSON status API.
- Cloudflare Pages/Workers or equivalent managed through declarative infra.
- Preview deployments for changes; tag-driven production release; live smoke
  test verifies package metadata and security status agree.

## Governance and sustainability

Before inviting application maintainers, publish:

- charter and scope;
- maintainer and reviewer roles;
- runtime-major admission and retirement policy;
- application enrollment/removal criteria;
- security embargo handling;
- trademark/non-affiliation policy;
- code of conduct and contribution guide;
- financial sponsorship policy with no pay-to-delay-security option;
- bus-factor/backup-key plan; and
- archival/sunsetting procedure.

Budget storage, CI, bandwidth, signing-key custody, CVE monitoring, and maintainer
time. Track cost per runtime major and per architecture. The support policy must
shrink before the maintainer team becomes overloaded, never after updates stop.

## Implementation phases

### Phase 0 — feasibility and naming gate

- [ ] Re-check Electron current supported majors and choose the one launch line.
- [ ] Confirm LosslessCut on the newest major or document the blocking APIs.
- [ ] Compare its embedded runtime to official upstream Electron byte-for-byte.
- [ ] Inventory native modules and resource-path assumptions.
- [ ] Build an unpacked proof outside APT: system runtime + split LosslessCut app.
- [ ] Measure download/installed savings and cold/warm startup differences.
- [ ] Review Electron trademark/naming guidance.
- [ ] Clear and register the project domain and GitHub organization.
- [ ] Write a go/no-go decision. Stop if the security ownership cannot be staffed.

### Phase 1 — runtime repository

- [ ] Create neutral organization/repository and governance files.
- [ ] Build and publish a digest-pinned Ubuntu 26.04 toolchain image with a
      reviewed dependency manifest; record its digest in the repository.
- [ ] Derive `SOURCE_DATE_EPOCH` from the packaging commit and make all archive,
      package, SBOM, and provenance generation consume it.
- [ ] Commit the JavaScript packaging-tool lockfile and make stable builds use a
      verified, offline dependency store rather than resolving through `npx`.
- [ ] Package the chosen Electron major from a pinned official Linux archive.
- [ ] Add dependency generation, file manifest, SBOM, provenance, sandbox tests,
      and lintian policy.
- [ ] Rebuild twice with isolated caches, require identical hashes, and retain a
      `diffoscope` report when equality fails.
- [ ] Bootstrap a separately signed candidate APT repository declaratively.
- [ ] Implement current-major detection, patch-update automation, canary tests,
      support/EOL metadata, and emergency rollback.
- [ ] Publish candidate runtime and verify clean Ubuntu 26.04 installation.

### Phase 2 — LosslessCut conversion

- [ ] Split the current payload by proven ownership.
- [ ] Port LosslessCut to the chosen supported Electron major.
- [ ] Build `losslesscut` with an exact runtime dependency and no embedded
      Electron/Chromium files.
- [ ] Run the full functional, sandbox, GPU, resource-path, FFmpeg, upgrade, and
      removal test matrix.
- [ ] Publish to candidate, soak, then promote both repositories in a coordinated
      release.
- [ ] Retain and document rollback to the monolithic package.
- [ ] Submit the generic runtime integration upstream to LosslessCut.

### Phase 3 — public site and first external app

- [ ] Launch the Astro/Tailwind site from signed metadata.
- [ ] Publish install, security, developer, governance, status, and LosslessCut
      evidence pages.
- [ ] Evaluate draw.io Desktop as the second independent app.
- [ ] Require a second app to pass before claiming ecosystem-level compatibility.
- [ ] Contact MarkText only after the second-app contract is stable.

### Phase 4 — forward maintenance

- [ ] Add the next Electron stable major before the current line leaves support.
- [ ] Migrate enrolled apps, retaining the older major only where justified.
- [ ] Run and publish the first major-retirement exercise.
- [ ] Add arm64 only after runtime and application hardware tests pass.
- [ ] Reassess source-building Electron when staffing and CI capacity make it
      credible.

### Phase 5 — v2 package-format portability

- [ ] Extract package-neutral runtime/app manifests, generated file ownership,
      compatibility tests, SBOM, provenance, and lifecycle metadata from the
      Debian-specific implementation.
- [ ] Validate LosslessCut's app-only payload and launcher contract with Arch's
      existing versioned Electron package; contribute packaging or patches to
      Arch/AUR instead of publishing a redundant Arch runtime.
- [ ] Select the second native package format only after its distribution target
      has demand, primary and backup maintainers, and security/CI capacity.
- [ ] Provisional path: package the supported runtime and LosslessCut as native
      openSUSE Tumbleweed RPMs through OBS, with openSUSE-specific dependency,
      sandbox, signing, install, upgrade, and removal tests.
- [ ] Prove that both native formats are generated from the same compatibility
      contract and runtime-major lifecycle state.
- [ ] Document that Fedora is unsupported; do not infer Fedora compatibility
      from RPM output or make Fedora a v2 release gate.
- [ ] Investigate Flatpak separately for immutable/cross-distribution desktops;
      it supplements but does not replace the second native-format proof.
- [ ] After the desktop package-neutral contract stabilizes, design a
      Yocto/OpenEmbedded layer with recipes for versioned Electron runtimes and
      app-only payloads, pinned source/binary inputs, license manifests, SBOM and
      CVE integration, cross-architecture builds, image tests, and runtime-major
      retirement metadata. Treat this as the embedded product-image track, not
      as another public desktop package repository or a substitute for v2's
      second native package format.

## Verification gates

### Runtime

- [ ] Exact pinned upstream archive and metadata verified.
- [ ] No unowned/unexplained runtime files.
- [ ] `dpkg-shlibdeps`, lintian, SBOM, license and malware scans pass.
- [ ] Sandbox, Wayland, X11, GPU, audio, portals and crash behavior pass.
- [ ] Same-major patch upgrade passes every enrolled app.
- [ ] Candidate rollback restores the previous known-good runtime.

### LosslessCut

- [ ] No embedded Electron executable, Chromium packs/locales, snapshots, or
      sandbox helper remain in the app package.
- [ ] Exact runtime dependency and manifest agree.
- [ ] Private FFmpeg remains functional and isolated.
- [ ] Full editing/export smoke matrix passes on clean Ubuntu 26.04.
- [ ] Upgrade from the existing Foundry package is clean and reversible.
- [ ] Size saving is measured and published.

### Service

- [ ] Clean-machine repository setup and removal pass.
- [ ] Index completeness, signing, source packages, provenance and live hash
      checks pass.
- [ ] Security monitor detects a simulated new Electron patch/advisory.
- [ ] Website runtime/app/security state matches signed repository metadata.
- [ ] Two unrelated applications run on the shared runtime before the project
      claims general compatibility.

## Risks and explicit mitigations

| Risk | Mitigation |
|---|---|
| Shared runtime breaks app assumptions | Exact-major manifests, candidate channel, per-app regression tests |
| Native ABI mismatch | Scan/rebuild `.node` modules; exact Electron module ABI gate |
| Runtime CVE becomes centralized blast radius | Fast SLA, canary, rollback, named security ownership |
| Too many major versions | Forward-only policy; demand-driven majors; retirement with dependents |
| Upstream app updater reinstalls bundle | Disable/patch updater; APT owns updates |
| Official binary differs from app's embedded build | Hash/build-ID comparison and recorded exceptions |
| Trademark implies official Electron project | Neutral branding, “Unofficial” everywhere, naming review |
| Repository disappears and strands apps | Sunset plan, signed snapshots, exportable packaging/provenance |
| Savings are smaller than expected | Phase-0 byte measurement and go/no-go threshold |
| Maintainer burden exceeds Foundry's scope | Independent org/governance; cap supported majors/apps |

## Go/no-go criteria

Proceed to public candidate release only if:

1. LosslessCut runs fully on a currently supported Electron major;
2. the split saves at least 100 MiB installed without degrading sandboxing, GPU,
   portals, or FFmpeg behavior;
3. same-major patch upgrades can be tested automatically;
4. a named maintainer owns security releases and a backup can publish safely;
5. the project identity, domain, and non-affiliation language are cleared; and
6. rollback is tested before the first stable promotion.

If those conditions fail, keep LosslessCut's existing monolithic package. A
duplicated runtime is preferable to a shared runtime that is stale, incompatible,
or ambiguously maintained.

## Primary references

- [Electron release schedule and support policy](https://www.electronjs.org/docs/latest/tutorial/electron-timelines)
- [Electron release versions and embedded Chromium/Node versions](https://releases.electronjs.org/)
- [Electron native Node module ABI guidance](https://www.electronjs.org/docs/latest/tutorial/using-native-node-modules)
- [Arch Linux Electron package](https://archlinux.org/packages/extra/x86_64/electron/)
- [Debian package search for Electron](https://packages.debian.org/search?keywords=electron)
- [Yocto Project technical overview](https://www.yoctoproject.org/development/technical-overview/)
- [LosslessCut 3.69.0 release](https://github.com/mifi/lossless-cut/releases/tag/v3.69.0)
- [draw.io Desktop 31.3.2 release](https://github.com/jgraph/drawio-desktop/releases/tag/v31.3.2)
- [MarkText 0.19.1 release](https://github.com/marktext/marktext/releases/tag/v0.19.1)
- [Signal Desktop 8.25.0 release](https://github.com/signalapp/Signal-Desktop/releases/tag/v8.25.0)
- [Joplin 3.6.16 release](https://github.com/laurent22/joplin/releases/tag/v3.6.16)
- [Element Desktop](https://github.com/element-hq/element-desktop)
- [Beekeeper Studio 6.0.5 release](https://github.com/beekeeper-studio/beekeeper-studio/releases/tag/v6.0.5)
- [balenaEtcher 2.1.6 release](https://github.com/balena-io/etcher/releases/tag/v2.1.6)
