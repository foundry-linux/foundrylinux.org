# ITP draft: blender-asset-finder-cli

## Upstream packaging audit

Audited the pinned upstream payload on 2026-08-05 with
`foundry-apt/scripts/audit-upstream-packaging.sh`. **No upstream packaging signals found:** no
upstream-maintained `debian/`, `.deb`/`.dsc` builder, `PKGBUILD`/`.spec`, or PPA/Launchpad/OBS/COPR
packaging reference.

## wnpp check

Searched wnpp for "blender-asset-finder-cli" / "wf-asset" (its former name) / "WorldFoundry" CLI. **No existing wnpp bug found.**

## Relationship to blender-asset-finder

See [`blender-asset-finder.md`](blender-asset-finder.md) — this package and `blender-asset-finder` currently ship **duplicate copies** of the same asset-provider search/download library as two separate Debian source packages. **Recommend combining into a single ITP / single source package with two binary packages** before filing with Debian, rather than filing two ITPs for what is one library with two front-ends. Flagging for a packaging-owner decision, not resolving unilaterally here.

## ITP draft (if filed as-is, single binary)

```
Package name    : blender-asset-finder-cli
Version         : 0.1.0+git0a19d26c
Upstream contact: World Foundry <wbnorris@gmail.com>
URL             : https://github.com/wbniv/WorldFoundry
Licence         : GPL-2.0-or-later
Programming lang: Python 3
Description     : CLI companion to the blender-asset-finder Blender add-on
```

Command-line front-end to the blender-asset-finder asset-provider
library — searches and downloads 3D assets from supported CC0/licensed
repositories (Polyhaven, Kenney, AmbientCG, Quaternius, OpenGameArt,
Sketchfab) and filters results by per-blend-directory licence policy.
Subcommands: `search`, `download`, `policy`, `providers`. The same
provider library powers the `blender-asset-finder` Blender add-on
(packaged separately today); both front-ends share their search/download
logic, though the CLI currently bundles its own copy rather than
depending on a shared library package.

Same rationale as `blender-asset-finder`: a scriptable, licence-aware
multi-provider asset downloader is a genuinely useful shell tool for
game/3D-asset pipelines, but the case for Debian inclusion is narrower
(WorldFoundry-originated, not independently demanded elsewhere) than for
a broadly-used dev tool.

## Notes

- `debian/copyright`'s `Upstream-Name: cdpack` looks like a copy-paste
  leftover from another WorldFoundry package's copyright file — worth a
  quick fix in `foundry-apt/packages/blender-asset-finder-cli/debian/copyright`
  before this is packaging-ready for outside review (not fixed here per
  the "reading only, not editing packages/" constraint of this task).
- Architecture: `all`, Depends: `python3 (>= 3.10)`.
- Single license (GPL-2.0-or-later), DFSG-clean.
