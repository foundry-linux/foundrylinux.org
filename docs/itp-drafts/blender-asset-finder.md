# ITP draft: blender-asset-finder

## Upstream packaging audit

Audited the pinned upstream payload on 2026-08-05 with
`foundry-apt/scripts/audit-upstream-packaging.sh`. **No upstream packaging signals found:** no
upstream-maintained `debian/`, `.deb`/`.dsc` builder, `PKGBUILD`/`.spec`, or PPA/Launchpad/OBS/COPR
packaging reference.

## wnpp check

Searched wnpp for "blender-asset-finder" and "WorldFoundry" Blender add-on. **No existing wnpp bug found.** This is a WorldFoundry-authored project (`wbniv/WorldFoundry` on GitHub), not tracked by any other distro or packager (our own `debian/control` already notes "not on Repology... not in any distro, verified 2026-06-21").

## Relationship to blender-asset-finder-cli — should this be one ITP?

**Yes, worth filing as a single ITP covering both binary packages, or restructuring before filing.** `blender-asset-finder` (the Blender 4.2+ extension) and `blender-asset-finder-cli` (the CLI front-end) are two independently-built Debian **source** packages today (`foundry-apt/packages/blender-asset-finder/` and `foundry-apt/packages/blender-asset-finder-cli/`, each with its own `debian/` tree), but they wrap the **same underlying asset-provider library** — the CLI's `debian/control` says outright: "This package bundles its own copy of that library so it has no runtime dependency on the add-on being installed." That is code duplication across two source packages, which Debian's ftp-master / reviewers will likely flag.

For an actual Debian ITP, the cleaner path is:
1. Restructure upstream (or the Debian packaging layer) so there is **one source package** producing two binary packages (`blender-asset-finder` and `blender-asset-finder-cli`), sharing the provider-library code via a `Multi-Arch: foreign`-safe shared internal module or a small `python3-worldfoundry-assetfinder-common` binary package — not two copies of the same Python source.
2. File **one ITP** for that combined source package, describing both binaries in the Description/long-description the way multi-binary source packages typically do.

Filing two separate ITPs for what is essentially one library with two front-ends invites exactly the kind of duplication Debian's Python/packaging policy discourages. This is a judgment call best made by whoever owns the WorldFoundry packaging relationship — flagging it here rather than resolving it unilaterally.

## ITP draft (if filed as-is, single binary)

```
Package name    : blender-asset-finder
Version         : 0.2.0+git0a19d26c
Upstream contact: World Foundry <wbnorris@gmail.com>
URL             : https://github.com/wbniv/WorldFoundry
Licence         : GPL-2.0-or-later
Programming lang: Python 3
Description     : search/download 3D assets from online repos, in Blender
```

A Blender 4.2+ extension that searches and downloads 3D assets from
multiple licence-aware online repositories — Polyhaven, Kenney, AmbientCG,
Quaternius, OpenGameArt, Sketchfab — and imports them directly into the
active scene. Filters results by per-blend licence policy with provenance
tracking via a `manifest.json` written alongside each download. Distinct
from Blender's built-in Asset Browser (a local asset-library manager):
this add-on's distinguishing feature is multi-provider online search and
on-demand download.

Blender add-ons of this kind (online asset search/import) are not
represented in Debian's existing Blender-adjacent packaging, and a
CC0/licence-aware multi-provider search tool has clear utility for anyone
doing Blender-based game-asset work. Debian-worthiness is more contingent
here than for a generic dev tool, though, given the narrow (WorldFoundry)
origin — this is closer to "useful niche add-on" than "widely-demanded
gap," so the case for filing rests more on functionality than on breadth
of existing demand.

## Notes

- Architecture: `all` (pure Python), Depends: `python3`, Recommends:
  `blender (>= 4.2.0)`.
- Single license (GPL-2.0-or-later) throughout, DFSG-clean.
- See `blender-asset-finder-cli.md` for the paired draft and the
  single-source-package recommendation above.
