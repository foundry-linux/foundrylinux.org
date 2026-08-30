# Launch Shared Electron at biohack.net/shared-electron

**Date:** 2026-08-30

**Status:** proposed — gated on three installable candidate packages

**Canonical launch URL:** `https://biohack.net/shared-electron/`

**APT repository:** `https://apt.wald3n.com/shared-electron` (new endpoint;
`apt.wald3n.com` does not exist yet)

**Scope:** Publish the initial Shared Electron project page only after the
versioned Electron base, LosslessCut, and draw.io Desktop are all available as
coherent, tested APT candidates. This page is the initial project home; a future
neutral domain may redirect to or replace it without changing package identity.

---

## Launch rule

Do not publish a placeholder that implies packages are available. Launch once
all three rows can be populated from signed repository metadata:

| Component | Required launch state |
|---|---|
| `electron-runtime-42` | Electron 42.9.3 candidate, signed, source-indexed, sandbox tested |
| `losslesscut` | Application-only 3.69.0 package tested on runtime 42.9.3 |
| `drawio-desktop` | Application-only 31.3.1 package tested on runtime 42.9.3, including CLI export |

The runtime and both apps must be promoted as one compatible candidate set. If
any row fails, hold the site launch rather than publishing stale commands.

## Repository and distribution decision

The three packages launch in a **separate Shared Electron APT repository**, not
in the general `apt.foundrylinux.org` package namespace:

```text
deb [arch=amd64 signed-by=/etc/apt/keyrings/shared-electron.gpg] \
  https://apt.wald3n.com/shared-electron resolute main
```

Wald3n is the repository owner because it already maintains the public
open-source/package inventory and is a better software-service identity than a
personal site. As of 2026-08-30, `apt.wald3n.com` has no DNS record and must be
provisioned; `wald3n.com/apt/` is not an existing public APT repository. The
repository has its own signing key, aptly state, immutable snapshots,
publish history, rollback procedure, Release `Origin`/`Label`, and package
metadata. It may reuse the proven `foundry-apt` build/publish tooling during
incubation, but must use a distinct configuration and publish root. This keeps a
Chromium-scale security service from being confused with the Foundry distro
archive and permits a future neutral-domain migration.

The initial supported binary target is **Ubuntu 26.04 LTS amd64**. Test and
document these consumers:

| Consumer | Launch status | Build used |
|---|---|---|
| Ubuntu 26.04 Desktop | Supported | Native Ubuntu 26.04 amd64 build |
| Kubuntu 26.04 | Supported | Same Ubuntu ABI build; separately smoke-tested |
| Foundry's Ubuntu 26.04 base | Supported consumer | Same Ubuntu ABI build; no ISO inclusion implied |
| Other Ubuntu 26.04 flavors/derivatives | Unclaimed until tested | May be compatible; no automatic support claim |
| Debian Stable | Post-launch target | Must be rebuilt and published in a separate Debian suite |
| Ubuntu 24.04 and older | Unsupported in v1 | Do not install or claim compatibility |
| arm64 | Post-launch target | Requires native runtime/app builds and hardware tests |

When Debian Stable is implemented, publish its binaries under a Debian-specific
suite in the same repository only after clean Debian builds and tests. Never
copy or relabel the Ubuntu binaries as Debian packages.

LosslessCut's existing monolithic Foundry package remains available for rollback
during the candidate period. The shared-runtime revision is published only in
the Shared Electron repository. Before production promotion, document APT
version selection for machines that have both repositories and either retire or
supersede the monolithic Foundry revision deliberately—never leave two equal
versions with different payloads.

## Site fit

`biohack.net` is an existing Astro site with a narrow, dark technical visual
language: Space Grotesk body text, JetBrains Mono technical labels, rust-orange
primary accents, blue secondary accents, and compact project sections. Build
the subsite in that repository and reuse its local font assets, tokens, metadata
components, and deployment path. Do not embed a separately generated microsite
or iframe.

The Shared Electron page should feel like a substantial project page within
biohack.net while remaining operationally portable. Keep its content/data in a
self-contained `shared-electron/` feature directory where the site's structure
permits.

### Roles across existing sites

Do not publish four copies of the same operational documentation. During
incubation, assign each existing property one job:

| Surface | Role |
|---|---|
| `indri.studio/apps/shared-electron/` | Polished project/product page: value, supported apps, screenshots, measured savings, and a link to install/security truth |
| `biohack.net/shared-electron/` | Technical origin story, architecture, implementation report, and links to packages/source |
| `wald3n.com/open-source` | Maintained open-source inventory entry for the runtime and enrolled application packaging |
| `apt.wald3n.com/shared-electron` | Signed packages, Release metadata, source indexes, immutable repository snapshots, and machine-readable package state |

`apt.indri.studio` currently resolves but serves the Indri website; it is not an
APT repository and must not be documented as one. The Indri and biohack pages
consume the same generated package/status data or link to it. Neither maintains
a hand-copied signing fingerprint, supported-version matrix, or install command.

## Information architecture

The launch page is one responsive route with anchored sections:

1. **Hero:** “One Electron runtime. Multiple desktop apps.” State clearly that
   this is unofficial third-party packaging for Ubuntu 26.04 amd64.
2. **Install:** signing-key setup, candidate APT source, `apt update`, and one
   command installing either or both applications.
3. **Available packages:** live cards for the runtime, LosslessCut, and draw.io
   with version, Electron major/patch, compressed size, installed size, status,
   and package-manifest links.
4. **How sharing works:** compact runtime → applications diagram and explicit
   ownership boundary.
5. **Measured savings:** upstream monolithic versus split package sizes, plus
   incremental cost of installing the second app.
6. **Security and updates:** current Electron patch, sandbox state, support
   window, disclosure link, signed-repository fingerprint, and rollback policy.
7. **Compatibility:** tested Ubuntu release/architecture and honest limitations;
   do not imply generic compatibility with arbitrary Electron applications.
8. **Project links:** package sources, build provenance, test results, roadmap,
   and the neutral-organization proposal.

## Data contract

- [ ] Generate package versions, architectures, sizes, hashes, dependency
  bounds, and repository URLs from signed APT metadata.
- [ ] Generate test state from committed compatibility manifests, not prose.
- [ ] Keep descriptive copy in Astro/Markdown; never generate security claims
  from an untrusted package field.
- [ ] Fail the site build when any of the three launch packages is absent,
  unsigned, mismatched to Electron 42.9.3, or marked failing.
- [ ] Display metadata generation time and repository suite.
- [ ] Provide stable fragment links for install, packages, security, and status.

## Implementation

- [ ] Locate and inspect the source repository and standard publisher for
  `biohack.net`; do not duplicate deployment credentials in this repository.
- [ ] Add `/shared-electron/` using the site's existing Astro layout, SEO/Open
  Graph conventions, fonts, responsive breakpoints, and accessibility rules.
- [ ] Translate the high-fidelity site specification in
  `2026-08-30-shared-electron-runtime-repository.md` into the narrower
  biohack.net visual system.
- [ ] Add a “Shared Electron” entry under Highlighted Projects on the home page.
- [ ] Add `indri.studio/apps/shared-electron/` using Indri's existing app-page
  conventions, linking to the same canonical package/status metadata.
- [ ] Add a Shared Electron app card to the Indri home page.
- [ ] Add an install-command copy control with a non-JavaScript fallback.
- [ ] Provision `apt.wald3n.com` and a distinct `/shared-electron` publish root,
  signing key, aptly state, snapshots, and least-privilege deployment identity.
- [ ] Set Release metadata to a distinct Shared Electron `Origin` and `Label`;
  verify it never inherits Foundry Linux branding by accident.
- [ ] Add machine-readable JSON for the three package cards and a visible
  human-readable fallback when metadata is temporarily unavailable.
- [ ] Add canonical, Open Graph, description, and social-card metadata rooted at
  `https://biohack.net/shared-electron/`.
- [ ] Ensure the page works without third-party JavaScript, analytics, remote
  fonts, or browser-side calls to the APT service.
- [ ] Add link, HTML, accessibility, mobile-width, reduced-motion, and screenshot
  tests to the site's existing gate.

## Release sequence

1. Finish and verify `electron-runtime-42` 42.9.3.
2. Requalify LosslessCut against that patch.
3. Finish and verify `drawio-desktop` 31.3.1.
4. Build a candidate APT snapshot at `apt.wald3n.com/shared-electron` containing
   all three source and binary packages; verify it from clean Ubuntu, Kubuntu,
   and Foundry-base 26.04 machines.
5. Generate and review the site metadata and measured-savings table from that
   snapshot.
6. Deploy the Indri app page and biohack.net technical page through each site's
   standard publisher.
7. Add the project and all three packages to `wald3n.com/open-source`, linking
   back to the biohack.net project page and the Wald3n APT repository.
8. Verify the public route, canonical URL, install commands, package links,
   security fingerprint, mobile layout, and home-page discovery link.
9. Record the deployed revision and package snapshot in the release report.

The sequence must not send email. Publication notifications, if any, use only
channels separately and explicitly authorized by the user.

## Acceptance criteria

- [ ] `https://biohack.net/shared-electron/` returns 200 over HTTPS and has the
  correct canonical URL.
- [ ] The page lists exactly the tested runtime, LosslessCut, and draw.io
  candidate versions from signed metadata.
- [ ] A clean Ubuntu 26.04 amd64 system can follow the displayed commands and
  install either app plus the one shared runtime.
- [ ] Package sizes, hashes, dependencies, source links, and signing fingerprint
  match the candidate repository.
- [ ] Security status and unsupported scope are visible without opening another
  page.
- [ ] biohack.net's home page links to the project.
- [ ] Indri's app index links to its Shared Electron page, which displays the
  same package versions and status without duplicating operational ownership.
- [ ] Desktop/mobile, keyboard, reduced-motion, and no-JavaScript checks pass.
- [ ] No ISO was built or changed and no email was sent.

## Later migration

If the project receives a neutral domain and organization, keep this URL as a
permanent project-history/redirect location. Package names, signing identity,
compatibility manifests, and APT paths must not depend on the biohack.net route,
so moving the public home does not force package migrations.

## Related plans

- [Shared Electron runtime repository](./2026-08-30-shared-electron-runtime-repository.md)
- [draw.io Desktop on Shared Electron](./2026-08-30-drawio-desktop-shared-electron.md)
