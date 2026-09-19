# Foundry Workbench — preliminary name-clearance investigation

**Date:** 2026-08-30

**Status:** technically clear in the searched namespaces; recommended for a
registrar and professional trademark check before adoption

**Candidate:** **Foundry Workbench**

---

## Executive finding

**Foundry Workbench is the strongest candidate found so far.** The exact phrase
did not produce a competing Linux distribution, operating system, game-development
product, repository, package, or container namespace in the checks below. The
candidate also avoids the direct **FoundryOS** collision while retaining the
relationship to World Foundry.

The domain and namespace evidence is encouraging:

- `.com`, `.net`, `.org`, and `.dev` returned **RDAP 404** from their registry
  operators, meaning no domain object was present at lookup time;
- those four names plus `.io` returned no DNS address record;
- the exact GitHub user and organization names returned 404;
- exact-name GitHub and GitLab repository searches returned no result;
- the exact npm and PyPI package URLs returned 404;
- Docker Hub returned a valid namespace query with `count: 0` repositories.

This is a **preliminary clearance screen, not a legal opinion or trademark
clearance**. In particular, an RDAP 404 is not a purchase: a registrar may still
classify a name as reserved or premium, and trademark similarity involves more
than exact-string matches. The next irreversible step should be registering the
domains and ordering a professional search in the jurisdictions where the
distribution will be offered.

## Why this naming shape is necessary

The existing public project
[`23sonics/foundryos-2`](https://github.com/23sonics/foundryos-2) is a direct
collision with **FoundryOS**. GitHub's API reports that repository as created on
2026-02-26 and last pushed on 2026-07-31. Its own description calls it customized
Fedora Atomic images built with BlueBuild. It is small and personal-use, but it
occupies the exact operating-system name and Linux-image namespace.

[Whitewater Foundry](https://www.whitewaterfoundry.com/) is also an established
Linux-distribution vendor: its WSL distribution is Pengwin. It does not appear to
use the exact product name “Foundry Workbench,” but its presence makes bare
**Foundry**, **Foundry Linux**, and other weakly distinguished constructions
unnecessarily risky.

The appropriate pattern remains:

```text
Foundry + distinctive product word
```

“Workbench” does meaningful identifying work and describes this distribution as
a prepared place for making games. It remains valid if the kernel, Ubuntu base,
or delivery mechanism changes.

## Exact-name product and web search

Searches performed on 2026-08-30:

- `"Foundry Workbench"`
- `"Foundry Workbench" software`
- `"Foundry Workbench" game development`
- `"Foundry Workbench" operating system`
- `"Foundry Workbench" trademark`

No exact-name competing product was found in the returned results. Broad searches
for the two individual words are noisy and are not evidence of collision:
“Foundry” is widely used by Palantir, Foundry VTT, the Ethereum Foundry toolchain,
Cloud Foundry, and many unrelated companies; “Workbench” is a common software
noun. The protectable and searchable unit is the full phrase.

The absence of an indexed exact match is favorable, not dispositive. Search
engines do not cover unindexed products, pending marks, local businesses, or
private prerelease projects.

## Domain checks

The following registry RDAP endpoints were queried directly rather than relying
on a domain reseller's search UI.

| Domain | Registry result | DNS address | Interpretation |
|---|---:|---:|---|
| `foundryworkbench.com` | Verisign RDAP `404` | none | No registered domain object observed |
| `foundryworkbench.net` | Verisign RDAP `404` | none | No registered domain object observed |
| `foundryworkbench.org` | PIR RDAP `404` | none | No registered domain object observed |
| `foundryworkbench.dev` | Google Registry RDAP `404` | none | No registered domain object observed |
| `foundryworkbench.io` | RDAP host did not resolve | none | Inconclusive; check through a registrar |

Relevant authoritative endpoints:

- [Verisign `.com` RDAP](https://rdap.verisign.com/com/v1/domain/FOUNDRYWORKBENCH.COM)
- [Verisign `.net` RDAP](https://rdap.verisign.com/net/v1/domain/FOUNDRYWORKBENCH.NET)
- [Public Interest Registry `.org` RDAP](https://rdap.publicinterestregistry.org/rdap/domain/FOUNDRYWORKBENCH.ORG)
- [Google Registry `.dev` RDAP](https://pubapi.registry.google/rdap/domain/FOUNDRYWORKBENCH.DEV)

An HTTP 404 from an RDAP registry means that registry returned no domain object.
It does not guarantee a normal-price registration. Domain availability must be
confirmed and completed at an actual registrar immediately before public naming;
availability can change at any time.

## Repository and organization namespaces

### GitHub

The following public API paths returned 404:

- `github.com/foundryworkbench`
- GitHub organization `foundryworkbench`
- `github.com/foundryworkbench/foundryworkbench`

GitHub repository search for the exact phrase returned no repository. These
results indicate that the obvious public handle was unused at lookup time, but
GitHub can reserve names or withhold them from registration independently of the
public API.

### GitLab

GitLab's public project search for `foundry workbench` returned no project. User
and group creation availability was not claimed because GitLab's public search
is not an authoritative reservation API.

## Package and image registries

| Registry | Exact identifier | Result |
|---|---|---|
| npm | `foundry-workbench` | package endpoint `404` |
| PyPI | `foundry-workbench` | project JSON endpoint `404` |
| Docker Hub | `foundryworkbench/*` | repository query `200`, `count: 0` |
| crates.io | `foundry-workbench` | public API rejected automated lookup (`403`); inconclusive |

Search results containing one word but not the full candidate—such as Ethereum
Foundry's Anvil packages, Sanity Workbench, or Cloud Foundry tools—were treated as
neighborhood noise rather than exact collisions.

The distro does not need to claim all of these package identifiers. Their value
here is as evidence that no software project appears to have established the
exact phrase across common developer namespaces.

## Trademark screen

Exact-phrase web searches with `trademark`, `software`, `operating system`, and
`game development` found no apparent **Foundry Workbench** mark or product.

This is the least conclusive part of the investigation. USPTO and EUIPO trademark
clearance requires more than an exact-name web query:

- pending applications may not be well indexed;
- confusing similarity can exist without an exact textual match;
- goods/services classes and actual market use matter;
- **Foundry** alone is crowded in software and technology;
- Whitewater Foundry, Foundry VTT, Palantir Foundry, and other established users
  deserve a professional similarity review even though none was found using the
  full phrase.

Before launch, search at minimum:

- [USPTO Trademark Search](https://tmsearch.uspto.gov/)
- [EUIPO eSearch plus](https://euipo.europa.eu/eSearch/)
- [WIPO Global Brand Database](https://branddb.wipo.int/)

The search should cover exact and phonetic variants and the relevant software,
downloadable operating-system, developer-tool, and education/support classes.

## Brand architecture

Recommended public form:

```text
Foundry Workbench
A game-development workstation by World Foundry
```

Recommended edition forms:

```text
Foundry Workbench — Anvil
Foundry Workbench — Sprite
Foundry Workbench — Atelier
```

Recommended technical identifiers if adopted:

| Surface | Identifier |
|---|---|
| Public name | `Foundry Workbench` |
| Short conversational name | `Workbench` |
| Repository/machine slug | `foundry-workbench` |
| ISO stem | `foundry-workbench-{anvil,sprite,atelier}` |
| Primary domain | `foundryworkbench.com` if registrar-confirmed |
| Existing domain | retain `foundrylinux.org` as a long-lived redirect |
| APT package prefix | retain `foundry-*` |
| Edition names | retain Anvil, Sprite, Atelier |

Keeping `foundry-*` packages avoids a migration that offers no user-facing
benefit. The brand change and the package namespace solve different problems.

## Recommendation

Proceed with **Foundry Workbench** as the preferred candidate, subject to two
gates before announcing it:

1. register `foundryworkbench.com`, `.org`, `.net`, and preferably `.dev`, plus
   the relevant repository/organization handles; and
2. obtain a professional trademark search covering software and downloadable
   operating-system goods in the intended markets.

Do not use **FoundryOS**. Do not announce **Foundry Workbench** until the domains
are actually registered; publishing the candidate while the names merely appear
available creates an avoidable squatting risk.

## Lookup limitations

- All results are point-in-time observations from 2026-08-30.
- `.io` RDAP was unavailable from the lookup environment.
- crates.io returned 403 to the automated exact-name request.
- No registrar purchase was attempted.
- No social-media account was created or modified.
- No trademark application database result was interpreted as legal advice.
- This investigation made no naming, DNS, repository, package, or branding
  changes.

