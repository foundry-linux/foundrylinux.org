# Investigation: getting `apt.foundrylinux.org` tracked by Repology

**Date:** 2026-06-21 · **Status:** research complete, decision pending

## TL;DR

**Technically: easy.** Repology treats APT repos as first-class. Becoming tracked is two
pull requests (one to [`repology-updater`](https://github.com/repology/repology-updater) for
the repo definition, one to [`repology-rules`](https://github.com/repology/repology-rules)
for name normalization) — no cooperation needed from Repology beyond review, and our repo
already publishes everything the fetcher needs.

**Size is *not* the blocker — we fit the derivative pattern cleanly.** The right precedent
isn't the big repos (WakeMeOps 2500, Chromebrew 2000); it's
[**BackBox**](https://github.com/repology/repology-updater/blob/master/repos.d/deb/backbox.yaml)
— an Ubuntu derivative that adds its own security tools on top of Ubuntu, exactly our shape.
BackBox is tracked with **`minpackages: 10`**, pointing Repology at *only its own added
packages* (its Launchpad PPA), **not** a mirror of Ubuntu. Our ~26 distinct vendored
upstreams sit comfortably in that range. `minpackages` is a fetch-sanity floor you set to
your real size — **not** a bar you clear by inflating the package count. (Earlier draft of
this doc cited 400+ as a floor; that was cherry-picked from large repos and is wrong —
BackBox at 10 is the relevant analog.)

**Nice synergy:** the `X-Repology-Project` field we already maintain on every vendored
package (badge work, 2026-06-21) **is exactly the rename/ignore ruleset Repology needs** — a
real project name = a Repology `setname`, `none` = a Repology `ignore`. The mapping is
already authored; it just lives in our `debian/control` instead of in `repology-rules`.

## Producer vs. consumer (what "tracked" actually changes)

Today we **consume** Repology: each vendored package's `X-Repology-Project` makes the apt
index render a badge showing the *upstream-latest* version Repology knows about (from Debian,
Arch, etc.). We pull data **in**.

Being *tracked* is the inverse — we become a **producer**: "Foundry Linux" would appear as a
packaging source **on** each tool's Repology page (e.g. the [`ghidra`](https://repology.org/project/ghidra/versions)
page would gain a "Foundry Linux: 11.x" row), and Repology would version-compare our build
against everyone else's. The two are independent and complementary; neither requires the
other.

## How Repology tracking works

1. **Fetcher + parser per source.** Repology has built-in fetchers/parsers. For an APT repo
   the pattern (from [`repos.d/deb/wakemeops.yaml`](https://github.com/repology/repology-updater/blob/master/repos.d/deb/wakemeops.yaml))
   is `FileFetcher` pulling `…/binary-<arch>/Packages.gz` → `DebianPackagesParser`. (Repos
   that publish a source index instead use `DebianSourcesParser` on `Sources.gz`, like
   [`kde_neon.yaml`](https://github.com/repology/repology-updater/blob/master/repos.d/deb/kde_neon.yaml)
   — **not** us; we publish binary indices only.)
2. **Repo definition** — a YAML stanza in `repology-updater/repos.d/deb/<name>.yaml`.
3. **Normalization rules** — a separate PR to `repology-rules` (numbered YAML files like
   `100.prefix-suffix.yaml`, `700.deversion.yaml`, `800.renames-and-merges`) that map our
   package names → canonical Repology project names and strip our version revisions.
4. **`minpackages`** — a per-source sanity floor: if the fetch yields fewer than this,
   Repology errors the update rather than silently shipping a half-broken index. You set it to
   *your* repo's real size (BackBox uses `10`); it is **not** a minimum you must reach.
5. Submit as PR(s); a Repology maintainer reviews and merges. The
   [`repos.d/deb/README.md`](https://github.com/repology/repology-updater/blob/master/repos.d/deb/README.md)
   points new Debian derivatives at the [Debian Derivatives Census](https://wiki.debian.org/Derivatives/Census)
   — the conventional "front door" for distros like ours.

## Two questions this raised

### "If we mirrored all the (k)ubuntu packages we use, would we qualify?" — No, and don't

1. **The count was never the gate.** BackBox is tracked with 10 packages. `minpackages` is
   set to your real size, not a threshold you reach by adding more — we already clear it.
2. **A mirror adds zero value, and Repology exists to surface *version diversity*.**
   Re-hosting Ubuntu's exact `.deb`s would list thousands of packages at *identical* versions
   to Ubuntu (`Foundry: 1.2.3` == `Ubuntu: 1.2.3`) — pure duplication. That's precisely the
   low-signal "mirror" data Repology has no use for, and it would cost real R2
   bandwidth/storage for nothing. The BackBox model is the right one: **point Repology at only
   your *distinct* packages.** Repology rewards distinctness, not volume — a 26-package set of
   tools Ubuntu *doesn't* have is more interesting to it than a 4 000-package mirror of tools
   it already tracks via Ubuntu.

   *(Mirroring our base might have other justifications — self-containment, version pinning,
   fully-offline ISO builds — but those are independent distro-infra decisions, not a Repology
   lever.)*

### "We should publish source packages too" — Agreed; do it

Every tracked Debian/Ubuntu derivative examined parses a **source** index — BackBox, Parrot,
and KDE neon all use `DebianSourcesParser` on `Sources.{xz,gz}`. We currently publish **binary
only** (`Release` lists `binary-{all,amd64,arm64}`, no `source/Sources`). Publishing source
packages:

- **Matches how comparable repos are tracked** — unlocks `DebianSourcesParser` +
  `allowed_vcs_urls: https://github.com/foundry-linux/`, so each Repology project links back to
  our packaging, not just a binary name.
- **Is good apt-repo hygiene and fits our reproducibility mandate** — `apt-get source <pkg>`
  becomes possible; the repo stops being a binary black box.
- **Is mostly an aptly change** — our builds *already* produce `.dsc` + tarballs (canonical
  `debian/` source layout); we currently discard them. Keep the source artifacts,
  `aptly repo add` the `.dsc`, republish → aptly emits `main/source/Sources.gz`.

This is the one concrete repo change worth doing first — it helps Repology onboarding **and**
the repo on its own merits.

## What we publish today (the fetcher inputs)

From `https://apt.foundrylinux.org/dists/resolute/Release`:

| Field | Value | Note for Repology |
|---|---|---|
| Suite / Codename | `resolute` | the dist path segment |
| Components | `main` | single component |
| Architectures | `all amd64 arm64` | `binary-amd64/Packages` already contains the `all` packages → one source covers everything |
| Origin / Label | **`. resolute`** | aptly default — **should be fixed** to `Foundry Linux` before submitting (Repology may surface Origin/Label) |
| Package count | **55** binary (`binary-amd64`) | ~26 real upstreams + ~29 `foundry-*` metapackages / infra |
| Source index | **none** | we publish binary only → use `DebianPackagesParser` |

## The repo definition we'd submit

Modeled on `wakemeops` (binary-Packages parser) + `family: debuntu` (what Debian/Ubuntu
derivatives use):

```yaml
###########################################################################
# Foundry Linux
###########################################################################
- name: foundry_linux
  type: repository
  desc: Foundry Linux
  statsgroup: Debian+derivs        # same grouping BackBox/derivatives use
  family: debuntu
  ruleset: [debuntu, foundry_linux]
  color: 'e8590c'                  # Foundry forge-orange
  minpackages: 10                  # our real size; BackBox uses 10 — set, not cleared
  sources:
    - name: main
      fetcher:
        class: FileFetcher
        # Preferred: the source index (matches BackBox/Parrot/neon) — requires
        # publishing source packages (see "publish source packages" above).
        # Works TODAY without that: binary-amd64/Packages.gz + DebianPackagesParser.
        url: 'https://apt.foundrylinux.org/dists/resolute/main/source/Sources.gz'
        compression: gz
      parser:
        class: DebianSourcesParser
        allowed_vcs_urls: 'https://github.com/foundry-linux/'
      subrepo: main
  repolinks:
    - desc: Foundry Linux home
      url: https://foundrylinux.org
    - desc: Foundry Linux apt repository
      url: https://apt.foundrylinux.org
    - desc: Foundry Linux packaging sources
      url: https://github.com/foundry-linux/foundrylinux.org
  groups: [ all, production ]
```

## Blockers & prerequisites (ordered by importance)

1. ~~**Size — the real gate.**~~ **Not a blocker** (corrected). BackBox — an add-on-only
   Ubuntu derivative — is tracked at `minpackages: 10`; our ~26 distinct upstreams clear that
   easily. The Census registration (#6) is the credibility step, not a size threshold.
2. **Publish source packages.** Switch the repo from binary-only to also emitting a source
   index (`main/source/Sources.gz`) so we can use the conventional `DebianSourcesParser` +
   VCS links, like every derivative Repology tracks. (Mostly an aptly change — see the
   "publish source packages" section. Optional: the binary parser works today, but source is
   the better target.)
3. **Exclude the metapackages.** `foundry-core/-anvil/-sprite/-atelier/-desktop`,
   the category metas (`foundry-retro-tools`, `foundry-emulators-*`, `foundry-python-gamedev*`,
   …), `calamares-settings-foundry-linux`, `pvsneslib-examples`, and our own apps
   (`foundry-welcome`, `foundry-kde-theme`) are **not** upstream projects — they'd create junk
   Repology projects. These need `ignore` rules.
4. **Origin/Label hygiene.** Set `Origin: Foundry Linux` / `Label: Foundry Linux` in the
   aptly publish (currently the `. resolute` default) so we present cleanly.
5. **Name + version normalization.** Map our binary names → canonical projects and strip our
   `-1foundry1` / `+gitYYYYMMDD` revisions. **We already have this mapping** — it's the
   `X-Repology-Project` field on each vendored package:

   | Our binary package | Repology project (= our `X-Repology-Project`) |
   |---|---|
   | `mesen2` | `mesen` |
   | `snes9x-gtk` | `snes9x` |
   | `task` | `go-task` |
   | `pvsneslib-core` | `pvsneslib` |
   | `python3-{glfw,picire,pydub,…}` | `python:{glfw,picire,pydub,…}` |
   | `asar-snes-assembler` | `asar-snes-assembler` |
   | `halfempty`, `ldtk`, `ruff`, `wla-dx`, … | (same name) |
   | `foundry-*`, `calamares-settings-*`, `*-examples` | **ignore** (= our `none` opt-outs + the un-fielded metapackages) |

6. **Debian Derivatives Census** — register Foundry Linux there; it's the expected path for a
   derivative and lends the PR credibility.

## What we gain / don't gain

**Gain**
- Foundry Linux shows up as a packaging source on ~26 tools' Repology pages → visibility and
  distro credibility in the packaging ecosystem.
- Automatic version-lag detection: Repology would flag where our vendored build trails
  upstream (the same data our badges show, but now *attributed to us*).

**Don't gain**
- The bulk of `foundry-retro-tools` et al. is **Ubuntu-universe** tools we merely `Depends:`
  on (`mame`, `cc65`, `radare2`, `cvise`, …). Those live in Ubuntu's archive, **not** our
  repo, so Repology already tracks them under Ubuntu and would **not** attribute them to
  Foundry Linux. Only our ~26 vendored `.deb`s count.
- It does nothing for the `shrinkray`-via-`pipx` class of tool (not in the apt index at all).

**Caveats**
- Snapshot versions (`libvgm` `g20260121`, git-dated builds) render awkwardly on Repology and
  may show as perpetually "outdated" vs a tagged upstream.
- Maintenance: if the R2 path or dist name ever changes, the Repology fetcher silently breaks
  until we PR an update — we'd own that.

## Recommendation

BackBox de-risks the whole thing: a 10-package add-on Ubuntu derivative is already tracked,
so a ~26-upstream signed derivative with an ISO is squarely in-pattern. The work is mostly our
side, and most of it improves the repo regardless of Repology. Suggested order:

1. **Publish source packages** (the one real repo change) — keep the `.dsc` + tarballs our
   builds already produce, `aptly repo add` them, republish so `Release` gains
   `main/source/Sources.gz`. Good hygiene + reproducibility, and it unlocks the conventional
   `DebianSourcesParser` path.
2. **Fix `Origin`/`Label`** to `Foundry Linux` in the aptly publish (currently `. resolute`).
3. **Generate the ruleset from our own metadata** — add a
   `scripts/generate-repology-ruleset.sh` that emits the `repology-rules` YAML straight from
   each `debian/control` `X-Repology-Project` field (`setname` for real projects, `ignore` for
   `none` + the un-fielded metapackages). Our badge work already authored the mapping.
4. **Register on the [Debian Derivatives Census](https://wiki.debian.org/Derivatives/Census).**
5. **Submit the two PRs** — repo definition to `repology-updater`, ruleset to `repology-rules`.
   A one-line heads-up issue first is courteous but no longer a gate; BackBox is the precedent.

Net: **yes, pursue it.** Steps 1–3 are worth doing on their own merits; 4–5 are the actual
onboarding once those land.

## Sources

- Repology repo definitions: [`repology-updater/repos.d/deb/`](https://github.com/repology/repology-updater/tree/master/repos.d/deb)
  — the decisive analog is [`backbox.yaml`](https://github.com/repology/repology-updater/blob/master/repos.d/deb/backbox.yaml)
  (Ubuntu derivative, add-on-only, `minpackages: 10`, `DebianSourcesParser`); also examined
  `parrot` (full distro, `minpackages: 55000`), `kde_neon`, `wakemeops`, `pacstall`,
  `chromebrew`, `README.md`.
- Normalization rules: [`repology-rules`](https://github.com/repology/repology-rules).
- [Repology — tracked repositories list](https://repology.org/docs/about) ·
  [Debian Derivatives Census](https://wiki.debian.org/Derivatives/Census).
- Our live index: `https://apt.foundrylinux.org/dists/resolute/Release` (55 binary packages,
  `main`, binary-only).
