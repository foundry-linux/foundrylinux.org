---
title: /packages page + home-page Forge — generated from live apt indexes
date: 2026-05-21
revised: 2026-05-22
status: done
---

# Plan: `/packages` page, apt-publish-triggered

## Context

foundrylinux.org's landing page still carries the launch-week `Kit` grid — seven hand-curated tool cards (World Foundry GDK, Blender, MAME, f9dasm, 65ax, libvgm, vgmstream). The bottom four are now stale: f9dasm/65ax/libvgm/vgmstream are leaves inside `foundry-retro-tools`, not hero callouts, and "65ax" itself was retired (Ubuntu 26.04 universe ships `xa65`). The site claims a kit it no longer assembles that way.

Meanwhile the apt landscape has grown substantially since the original draft of this plan:

- **apt.foundrylinux.org** now serves a **3-tier editions hierarchy** (`foundry-anvil` ⊆ `foundry-sprite` ⊆ `foundry-atelier`) and **13 per-domain metapackages** (`foundry-retro-tools`, `foundry-emulators` umbrella + 4 sub-metapackages, `foundry-art`, `foundry-pixel-art`, `foundry-trackers`, `foundry-daw`, `foundry-image-cli`, `foundry-game-frameworks`, `foundry-game-reimplementations`, `foundry-free-games`, `foundry-android-development`, `foundry-ios-development`) plus **6 vendored standalone packages** (`f9dasm`, `ghidra`, `libvgm`, `vgmstream`, `blender-asset-finder`, `blender-asset-finder-cli`).
- **apt.worldfoundry.org** serves **5 worldfoundry-* metapackages** plus **9 CLI tools** (cdpack, iffcomp, iffdump, levcomp, lvldump, oaddump, oas2oad, prep, textile).

Total: ~40 packages across both repos, organized in a real two-axis hierarchy (3 editions × ~16 domains). None of this is visible to a user before they install — they have to read CLAUDE.md or `apt-cache depends` on a live box.

This plan does three things:

1. **Replaces the home page's `Kit` section with a `Forge` section** (six domain headliner cards) whose copy is hand-written but whose package counts and sizes are read from a JSON data file generated against the live apt indexes. The seven legacy tool cards (especially the lower four — f9dasm, 65ax, libvgm, vgmstream) are dropped; those are catalogue rows on `/packages`, not hero callouts.
2. **Adds a generated `/packages` page** that resolves both repos' Packages.gz, computes the transitive install footprint from every metapackage and standalone, and presents it as a categorised, size-annotated catalogue grouped by **edition tier** (Anvil base / Sprite layer / Atelier layer) with a per-package `origin` column (`foundry` / `worldfoundry` / `ubuntu-universe` / `ubuntu-multiverse`) and `upstream` column (`vendored` / `in-ubuntu-universe` / `debian-itp` / `n/a-firstparty` / `n/a-proprietary`).
3. **Adds an Upstreaming surface**: a small `data/upstream.yml` records the Debian ITP / Ubuntu MIR status of every vendored or first-party package; `/packages` surfaces it as a column on each row plus a dedicated "Upstreaming" section.

CI wiring (workflow_run, repository_dispatch from worldfoundry.org, nightly cron) is out of scope for this plan revision — landed as a follow-up. This plan focuses on the local generator + the two pages it drives.

## Approach

### 1. New `scripts/build-packages-page.sh` + `scripts/build-packages-data.js`

`build-packages-page.sh` is a thin shell wrapper that runs the heavy lifting inside an `ubuntu:26.04` container with both apt sources wired. The container shells out to `node scripts/build-packages-data.js` for parsing.

```bash
#!/usr/bin/env bash
set -euo pipefail

ROOT="$(git rev-parse --show-toplevel)"
cd "$ROOT"

# Caching — skip if neither Release file has changed.
foundry_sha="$(curl -fsSL https://apt.foundrylinux.org/dists/resolute/Release | sha256sum | awk '{print $1}')"
world_sha="$(curl -fsSL https://apt.worldfoundry.org/dists/stable/Release  | sha256sum | awk '{print $1}')"

if [[ -f site/packages-data.json && "${1:-}" != "--force" ]]; then
  prev_foundry=$(jq -r .foundry_release_sha   site/packages-data.json)
  prev_world=$(  jq -r .worldfoundry_release_sha site/packages-data.json)
  if [[ "$foundry_sha" == "$prev_foundry" && "$world_sha" == "$prev_world" ]]; then
    echo "✓ no change in either apt repo since last generation — skipping"
    exit 0
  fi
fi

docker run --rm -v "$PWD:/work" -w /work ubuntu:26.04 bash <<'EOF'
  set -euo pipefail
  export DEBIAN_FRONTEND=noninteractive
  apt-get update -q
  apt-get install -y --no-install-recommends \
    curl gnupg ca-certificates jq nodejs

  mkdir -p /etc/apt/keyrings
  curl -fsSL https://apt.foundrylinux.org/key.gpg    | gpg --dearmor -o /etc/apt/keyrings/foundry.gpg
  curl -fsSL https://apt.worldfoundry.org/key.gpg    | gpg --dearmor -o /etc/apt/keyrings/worldfoundry.gpg
  echo "deb [signed-by=/etc/apt/keyrings/foundry.gpg]      https://apt.foundrylinux.org resolute main" > /etc/apt/sources.list.d/foundry.list
  echo "deb [signed-by=/etc/apt/keyrings/worldfoundry.gpg] https://apt.worldfoundry.org stable   main" > /etc/apt/sources.list.d/worldfoundry.list

  # Need multiverse enabled so foundry-emulators-vintage / foundry-free-games dependencies resolve
  sed -i 's/^Components: main$/Components: main universe multiverse/' /etc/apt/sources.list.d/ubuntu.sources

  apt-get update -q
  node scripts/build-packages-data.js
EOF
```

`build-packages-data.js` is a single Node script (no deps beyond stdlib + `child_process` for `apt-cache`) that:

1. Reads `data/categories.json` (the ordered category schema — see §2).
2. Reads `data/upstream.yml` (hand-curated ITP/MIR statuses).
3. For each category's metapackage, runs:
   ```
   apt-cache depends --recurse --no-recommends --no-suggests --no-conflicts \
     --no-breaks --no-replaces --no-enhances <pkg>
   ```
   yielding a flat dep closure.
4. For each package in the closure, runs `apt-cache show <pkg>` (Package, Section, Description-en, Installed-Size, Version) + `apt-cache policy <pkg>` (origin — `apt.foundrylinux.org` / `apt.worldfoundry.org` / `archive.ubuntu.com` plus suite/component).
5. Joins per-package upstream status (`upstream.yml` lookup; default `vendored` for foundry-origin packages missing from the yaml; default `in-ubuntu-universe` for universe-origin; default `in-ubuntu-multiverse` for multiverse).
6. Emits `site/packages-data.json`.

**Output shape:**

```json
{
  "generated_at": "2026-05-22T12:34:56Z",
  "foundry_release_sha": "abc…",
  "worldfoundry_release_sha": "def…",
  "editions": [
    {
      "slug": "anvil",
      "title": "Anvil",
      "blurb": "Base edition — the full WorldFoundry stack, retro toolkit, light emulators, game frameworks, image tools. ~5 GB.",
      "install_command": "sudo apt install foundry-anvil",
      "metapackage": "foundry-anvil",
      "installed_size_kb": 5234112,
      "package_count": 87,
      "direct_depends": ["worldfoundry","foundry-retro-tools","foundry-emulators-computers","foundry-emulators-consoles","foundry-game-frameworks","foundry-image-cli","task","sudo"]
    },
    { "slug": "sprite", "title": "Sprite", "blurb": "Anvil + heavy graphics + audio production.", "...": "..." },
    { "slug": "atelier", "title": "Atelier", "blurb": "Sprite + everything else (multiverse emulators, free games, mobile dev, cross-repo dev deps).", "...": "..." }
  ],
  "categories": [
    {
      "slug": "worldfoundry-gdk", "title": "World Foundry GDK",
      "blurb": "Cross-platform engine + 10 CLI authoring tools.",
      "metapackage": "worldfoundry", "in_edition": "anvil",
      "installed_size_kb": 1923456, "package_count": 13,
      "packages": [
        { "name": "worldfoundry-cli", "origin": "worldfoundry", "section": "metapackages",
          "installed_size_kb": 12, "version": "1.0.3",
          "summary": "WorldFoundry authoring CLI tools.",
          "upstream": { "status": "n/a-firstparty" } },
        { "name": "cdpack", "origin": "worldfoundry", "section": "devel",
          "installed_size_kb": 384, "version": "0.3.1-1foundry1",
          "summary": "WorldFoundry CD packer.",
          "upstream": { "status": "n/a-firstparty" } },
        ...
      ]
    },
    { "slug": "blender", "title": "Blender", "metapackage": "worldfoundry-blender-addons", "in_edition": "anvil", "...": "..." },
    { "slug": "retro-tools", "title": "Retro toolkit", "metapackage": "foundry-retro-tools", "in_edition": "anvil", "...": "..." },
    { "slug": "emulators", "title": "Emulators", "metapackage": "foundry-emulators", "in_edition": "anvil+sprite+atelier (split)", "...": "..." },
    { "slug": "game-frameworks", "title": "Game frameworks", "metapackage": "foundry-game-frameworks", "in_edition": "anvil", "...": "..." },
    { "slug": "image-tools", "title": "Image tools", "metapackage": "foundry-art", "in_edition": "sprite", "...": "..." },
    { "slug": "audio-production", "title": "Audio production", "metapackage": "foundry-daw", "in_edition": "sprite", "...": "..." },
    { "slug": "games", "title": "Free games & reimplementations", "metapackage": "foundry-free-games", "in_edition": "atelier", "...": "..." },
    { "slug": "mobile-dev", "title": "Mobile development", "metapackage": "foundry-android-development", "in_edition": "atelier", "...": "..." }
  ],
  "vendored_standalones": [
    { "name": "f9dasm",                   "pulled_by": ["foundry-retro-tools"] },
    { "name": "ghidra",                   "pulled_by": ["foundry-retro-tools"] },
    { "name": "libvgm",                   "pulled_by": ["foundry-retro-tools"] },
    { "name": "vgmstream",                "pulled_by": ["foundry-retro-tools"] },
    { "name": "blender-asset-finder",     "pulled_by": ["worldfoundry-blender-addons"] },
    { "name": "blender-asset-finder-cli", "pulled_by": ["worldfoundry-cli"] }
  ],
  "grand_total_installed_size_kb": 9876543,
  "upstream_summary": {
    "vendored": 4,
    "in-debian-unstable": 0,
    "in-ubuntu-universe": 21,
    "in-ubuntu-multiverse": 7,
    "debian-itp": 0,
    "n/a-firstparty": 11,
    "n/a-proprietary": 0
  },
  "audit": {
    "unowned_packages": [],
    "missing_metapackages": [],
    "out_of_catalogue": []
  }
}
```

**Upstream status values** (closed set):
- `vendored` — Foundry-built, not in Debian/Ubuntu (no ITP filed)
- `debian-itp` — open ITP bug in Debian; `note` contains bug URL
- `in-debian-unstable` — landed in Debian unstable, awaiting Ubuntu autosync
- `in-ubuntu-universe` — already in Ubuntu 26.04 universe (auto-detected from `apt-cache policy`)
- `in-ubuntu-multiverse` — already in Ubuntu 26.04 multiverse (auto-detected)
- `n/a-firstparty` — own software (worldfoundry-*, blender-asset-finder-*)
- `n/a-proprietary` — non-free, not a candidate for upstreaming

If a foundry-origin package is missing from `upstream.yml`, the generator defaults to `vendored` and prints a warning.

### 2. Categorisation: 9 categories grouped by edition tier

The categories below mirror the actual current metapackage hierarchy. There is no longer a "planned metapackage" column — every metapackage in this catalogue exists in apt today. Categories are not disjoint (MAME appears under `retro-tools` and `emulators`; image tools appears under `image-tools` for CLI use and again as part of `worldfoundry-blender`'s recommended pairings).

| # | Slug | Title | Metapackage | Edition tier | Contents |
|---|---|---|---|---|---|
| 1 | `worldfoundry-gdk` | World Foundry GDK | `worldfoundry` | Anvil | 10 CLIs (cdpack, iffcomp, iffdump, levcomp, lvldump, oaddump, oas2oad, prep, textile, blender-asset-finder-cli) |
| 2 | `blender` | Blender | `worldfoundry-blender-addons` | Anvil | Blender + worldfoundry-blender-editor-exporter add-on + blender-asset-finder |
| 3 | `retro-tools` | Retro toolkit | `foundry-retro-tools` | Anvil | mame, mame-tools, dasm, cc65, z80*, radare2, binwalk, sox, m68k binutils, xa65, f9dasm, libvgm, vgmstream, ghidra |
| 4 | `emulators` | Emulators | `foundry-emulators` (umbrella) | spans 3 tiers | computers (Anvil), consoles (Anvil), consoles-heavy (Atelier), vintage (Atelier, multiverse) |
| 5 | `game-frameworks` | Game frameworks | `foundry-game-frameworks` | Anvil | tiled, SDL2/3, SFML, Allegro5, libtcod, glslang, spirv-cross, spirv-tools |
| 6 | `image-tools` | Image tools | `foundry-art` + `foundry-image-cli` | Sprite (art), Anvil (image-cli) | krita, gimp, inkscape, mypaint, mtpaint, grafx2, imagemagick, graphicsmagick + Sprite-only: tuxpaint |
| 7 | `audio-production` | Audio production | `foundry-daw` + `foundry-trackers` | Sprite | ardour, audacity, lmms, rosegarden, hydrogen, qtractor, mixxx, milkytracker, schism, furnace, openmpt123 |
| 8 | `games` | Free games & reimplementations | `foundry-free-games` + `foundry-game-reimplementations` | Atelier | 0ad, supertuxkart, supertux, widelands, warzone2100, openttd, freeciv, freedoom, freedm, hedgewars + vcmi, openrct2, fheroes2, exult, sauerbraten |
| 9 | `mobile-dev` | Mobile development | `foundry-android-development` + `foundry-ios-development` | Atelier | openjdk-17, adb, NDK r26c + libimobiledevice, ideviceinstaller, usbmuxd, ifuse |

`data/categories.json` is the hand-curated ordered list driving the page. Per category: `slug`, `title`, `blurb`, `icon`, `metapackages[]` (one or more — categories #6, #7, #8, #9 each cover two metapackages), `in_edition_tier` (anvil / sprite / atelier / "spans").

### 2b. Audit & alignment section

The generator emits three findings sets:

1. **`unowned_packages`** — packages reachable in apt but pulled by *no* category's metapackage list (and not in `out_of_catalogue`). Indicates a forgotten Depends or a missing category.
2. **`missing_metapackages`** — categories in `categories.json` whose declared metapackage doesn't actually exist in either apt repo. Should be empty (the plan no longer envisions any "planned" metapackages); if not empty, the build errors.
3. **`out_of_catalogue`** — hand-listed packages that exist in apt but intentionally aren't in the catalogue (e.g. `worldfoundry-blender-editor-exporter` — it's a build artifact, not for end-user install). Rendered as a small "Also in apt, not in this catalogue" footer.

The Alignment section on the `/packages` page renders these three lists with a brief explanation.

**Failure modes:**
- **Errors** if `unowned_packages` is non-empty or `missing_metapackages` is non-empty.
- **Warns** if a foundry-origin package is missing from `upstream.yml`.

### 3. New SSR page: `site/packages.jsx`

New component module that imports `packages-data.json` and renders the full catalogue. Layout:

```
┌─ Topbar (shared) ────────────────────────────────────────────────┐
│                                                                  │
│  Packages                                                        │
│  Everything Foundry Linux installs on top of Kubuntu 26.04.      │
│  Grand total: 9.9 GB · regenerated 2026-05-22 12:34 UTC          │
│                                                                  │
│  ╔══ Editions ════════════════════════════════════════════════╗  │
│  ║ Three nested install profiles. Pick one — atelier pulls in ║  │
│  ║ sprite, which pulls in anvil.                              ║  │
│  ║                                                            ║  │
│  ║ ┌─ Anvil ───── 5.2 GB ─┐ ┌─ Sprite ───── 7.8 GB ┐ ┌─ Atelier ─ 9.9 GB ─┐ ║  │
│  ║ │ sudo apt install      │ │ sudo apt install     │ │ sudo apt install  │ ║  │
│  ║ │   foundry-anvil       │ │   foundry-sprite     │ │   foundry-atelier │ ║  │
│  ║ │ Base — full WF stack, │ │ Anvil + heavy art    │ │ Sprite + ROM-     │ ║  │
│  ║ │ retro toolkit, light  │ │ + audio production.  │ │ bundled emulators,│ ║  │
│  ║ │ emulators, frameworks.│ │                      │ │ free games, mobile│ ║  │
│  ║ │ 87 packages           │ │ 142 packages         │ │ 218 packages      │ ║  │
│  ║ └───────────────────────┘ └──────────────────────┘ └───────────────────┘ ║  │
│  ╚════════════════════════════════════════════════════════════╝  │
│                                                                  │
│  ╔══ 01 · World Foundry GDK ──────── Anvil ─── 1.4 GB ════════╗  │
│  ║  sudo apt install worldfoundry                             ║  │
│  ║  Cross-platform engine + 10 CLI authoring tools.           ║  │
│  ║  ┌──────────────────────────────────────────────────────┐  ║  │
│  ║  │ Package      │ Origin    │ Size  │ Upstream │ Summary│  ║  │
│  ║  ├──────────────┼───────────┼───────┼──────────┼────────┤  ║  │
│  ║  │ worldfoundry │ worldfoundry │ 4 KiB │ n/a-firstparty │ … │ ║  │
│  ║  │ cdpack       │ worldfoundry │ 384 KiB│ n/a-firstparty│ … │ ║  │
│  ║  │ ...          │ ...       │ ...   │ ...      │ ...    │  ║  │
│  ║  └──────────────┴───────────┴───────┴──────────┴────────┘  ║  │
│  ╚════════════════════════════════════════════════════════════╝  │
│                                                                  │
│  … 02 Blender · 03 Retro toolkit · 04 Emulators · 05 Game        │
│       frameworks · 06 Image tools · 07 Audio production ·        │
│       08 Free games & reimplementations · 09 Mobile dev …        │
│                                                                  │
│  ╔══ Vendored standalones ════════════════════════════════════╗  │
│  ║ Packages we build ourselves because Ubuntu doesn't ship    ║  │
│  ║ them. Each one is pulled in transitively by one of the     ║  │
│  ║ categories above; this list is the at-a-glance vendoring   ║  │
│  ║ surface.                                                   ║  │
│  ║                                                            ║  │
│  ║ · f9dasm                  — pulled by foundry-retro-tools  ║  │
│  ║ · ghidra                  — pulled by foundry-retro-tools  ║  │
│  ║ · libvgm                  — pulled by foundry-retro-tools  ║  │
│  ║ · vgmstream               — pulled by foundry-retro-tools  ║  │
│  ║ · blender-asset-finder    — pulled by worldfoundry-blender-addons ║  │
│  ║ · blender-asset-finder-cli — pulled by worldfoundry-cli    ║  │
│  ╚════════════════════════════════════════════════════════════╝  │
│                                                                  │
│  ╔══ Upstreaming ═════════════════════════════════════════════╗  │
│  ║ Where we ship vendored tools to Ubuntu/Debian when it      ║  │
│  ║ makes sense to.                                            ║  │
│  ║                                                            ║  │
│  ║ Open Debian ITPs (0)                                       ║  │
│  ║   (none currently — first ITP planned for f9dasm)          ║  │
│  ║                                                            ║  │
│  ║ Already in Ubuntu universe (21)                            ║  │
│  ║   mame · cc65 · dasm · z80asm · radare2 · binwalk · sox    ║  │
│  ║   · krita · gimp · inkscape · ardour · audacity · …        ║  │
│  ║                                                            ║  │
│  ║ Already in Ubuntu multiverse (7)                           ║  │
│  ║   vice · atari800 · mame-extra · …                         ║  │
│  ║                                                            ║  │
│  ║ Vendored, no ITP yet (4)                                   ║  │
│  ║   ghidra · libvgm · vgmstream · f9dasm                     ║  │
│  ╚════════════════════════════════════════════════════════════╝  │
│                                                                  │
│  ╔══ Alignment ═══════════════════════════════════════════════╗  │
│  ║ Unowned packages (0):  (empty — every package is reached)  ║  │
│  ║ Missing metapackages (0):  (empty — all 16 exist)          ║  │
│  ║ Out of catalogue (1):  worldfoundry-blender-editor-exporter║  │
│  ╚════════════════════════════════════════════════════════════╝  │
│                                                                  │
│  Foot (shared)                                                   │
└──────────────────────────────────────────────────────────────────┘
```

Reuses existing `site/styles.css` chrome (`.section`, `.section-head`, the existing `.editions` grid) plus three new utility classes: `.pkg-table`, `.pkg-origin-badge`, `.pkg-upstream-badge`.

### 3b. Home-page reshape: `Kit` → `Forge`

The current `Kit` section (`site/sections.jsx:88-199`) renders seven hand-curated tool cards. The top three (World Foundry GDK, Blender, MAME) stand in for whole categories; the bottom four (f9dasm, 65ax, libvgm, vgmstream) are leaves within `foundry-retro-tools`.

**Replace `Kit` with `Forge`**: six category headliner cards in a 3×2 grid, each card auto-driven from `packages-data.json`:

```
Section header
  № 01 · The Forge

  We package the retro, game-dev, RE, and authoring tools Ubuntu
  doesn't ship — and push the ones worth shipping upstream into Debian.

  Built for Ubuntu 26.04. Resigned, rebuilt, and re-tested on every push.

Top row (creative loops)
  ┌──────────────────────┐  ┌──────────────────────┐  ┌──────────────────────┐
  │ World Foundry GDK    │  │ Blender              │  │ Retro toolkit        │
  │ apt install          │  │ apt install          │  │ apt install          │
  │   worldfoundry       │  │   worldfoundry-      │  │   foundry-retro-     │
  │                      │  │     blender          │  │     tools            │
  │ 10 CLIs · 21 MiB     │  │ Blender + add-ons    │  │ MAME · Ghidra ·      │
  │                      │  │ ~1.9 GiB             │  │ dasm · z80 · cc65    │
  │ See category →       │  │ See category →       │  │ See category →       │
  └──────────────────────┘  └──────────────────────┘  └──────────────────────┘

Bottom row (rest of catalogue)
  ┌──────────────────────┐  ┌──────────────────────┐  ┌──────────────────────┐
  │ Emulators            │  │ Audio & art          │  │ Games & mobile dev   │
  │ apt install          │  │ apt install          │  │ apt install          │
  │   foundry-emulators  │  │   foundry-sprite     │  │   foundry-atelier    │
  │                      │  │                      │  │                      │
  │ 4 sub-metapackages · │  │ DAW · trackers ·     │  │ 30 free games · WF   │
  │ MAME + 28 emulators  │  │ Krita · GIMP …       │  │ engines · NDK · iOS  │
  │ See category →       │  │ See category →       │  │ See full catalogue → │
  └──────────────────────┘  └──────────────────────┘  └──────────────────────┘

Footer strip on this section
  + N open Debian ITPs · M tools already in Ubuntu universe →
  + Browse the full catalogue → /packages
```

**Card data binding.** Each card has hand-written `title`, `role`, `icon`, `category_slug` (the link target). Package count, install command, headline contents string, and the install command come from `packages-data.json` lookups by `category_slug`. If a card references a `category_slug` not in the JSON, the build fails — guaranteeing no card can silently claim a category that doesn't exist.

**Updates to other home-page sections:**
- `Hero` — keep `apt install foundry-anvil` (already current).
- `Install` — keep the 2-step Quick Start (curl-bash + apt install).
- `Editions` (site/sections.jsx:270) — keep, but update the per-edition contents copy to reflect actual Depends. Currently the Anvil card describes "World Foundry GDK, Blender, MAME, the assembler suite, the audio libraries — everything ready on first boot," which is still accurate at category level; the per-edition contents bullet list (if any) should be auto-driven from `packages-data.json`'s `editions[].direct_depends`.
- `Topbar` — `#kit` link → `#forge`; add a top-level `Packages` nav item linking to `/packages`.
- `Foot` — add `Packages` link in the same row as Docs/Install/Editions.

### 4. SSR renderer extension: `scripts/ssr-render.js`

Generalise the existing single-page renderer into a small loop over a `PAGES` table:

```js
const PAGES = [
  { entry: 'site/app.jsx',      out: 'site/index.html',
    title: 'foundrylinux.org · FOUNDRY LINUX',
    description: 'Foundry Linux — a Linux distribution for game development, reverse engineering, and retro tooling.' },
  { entry: 'site/packages.jsx', out: 'site/packages.html',
    title: 'Packages · FOUNDRY LINUX',
    description: 'Every package Foundry Linux installs on top of Kubuntu 26.04 — categorised by edition tier, with sizes and upstream status.' },
];
```

Wrapper HTML factored into a `wrap(body, {title, description, extraHead, extraBody})` template function. `embers.js` and the copy-button + scroll-shrink inline scripts stay on the home page only; `/packages` is static content.

### 5. New Taskfile entries

```yaml
packages-page:
  desc: "Fetch live Packages.gz from both apt repos, regenerate site/packages-data.json"
  dir: "{{.ROOT_DIR}}"
  cmds:
    - bash scripts/build-packages-page.sh {{.CLI_ARGS}}

site-build:
  desc: SSR-render site/*.jsx → fully-static site/index.html + site/packages.html
  dir: "{{.ROOT_DIR}}"
  deps: [packages-page]
  cmds:
    - node scripts/ssr-render.js
```

`packages-page` is cheap on cache-hit (skips the docker step entirely when both Release SHAs are unchanged), so making it a `site-build` dep is safe.

### 6. CI wiring — DEFERRED to a follow-up plan

The original draft of this plan included `workflow_run` (foundry-apt publish → site rebuild), cross-repo `repository_dispatch` from `wbniv/worldfoundry.org` requiring a new fine-grained `FOUNDRYLINUX_DISPATCH_PAT` secret, and a nightly cron. That work is **out of scope for this revision** — landed as `docs/plans/YYYY-MM-DD-packages-page-ci-triggers.md` once the local pipeline is verified end-to-end.

For now, regeneration happens manually (`task site-build`) or on tag push (existing `site-deploy.yml`). Manual is fine while the schema settles.

## Critical files

**Create:**
- `scripts/build-packages-page.sh` — shell entry, wraps the container
- `scripts/build-packages-data.js` — runs inside the container; `apt-cache depends --recurse` + `apt-cache show` + `apt-cache policy` → `site/packages-data.json`
- `data/categories.json` — ordered category schema (slug, title, blurb, icon, metapackages[], in_edition_tier)
- `data/upstream.yml` — per-package upstream status (ITP bugs, n/a-firstparty markers, etc.)
- `site/packages.jsx` — new SSR page
- `site/forge.jsx` — new home-page section (or inline in `sections.jsx`; pick whichever keeps `sections.jsx` under ~400 lines)
- `site/icons.jsx` additions — 3 new icons (retro toolkit, emulators-multi, games-controller) if existing GamepadIcon/AssemblerIcon don't fit; reuse existing where possible

**Modify:**
- `scripts/ssr-render.js` — generalise to render N pages from `PAGES` table; extract HTML wrapper into `wrap(body, …)`
- `Taskfile.yml` — add `packages-page` task, add `deps: [packages-page]` to `site-build`
- `site/app.jsx` — remove `<Kit/>`, insert `<Forge/>` in its place
- `site/sections.jsx` — delete `Kit` component + `KIT = [...]` array; extract `Topbar` + `Foot` so `packages.jsx` can import them; minor Hero copy tweak; update `Editions` contents copy if needed
- `site/styles.css` — add `.pkg-table`, `.pkg-origin-badge`, `.pkg-upstream-badge`, `.forge-grid`
- `TODO.md` — add this plan as the new active item; ship checkbox for items in §6 (CI wiring) and §2c (future audit-driven follow-ups)

**Do NOT modify in this revision:**
- `.github/workflows/site-deploy.yml` — CI trigger wiring deferred to follow-up plan
- `worldfoundry.org/.github/workflows/apt-publish.yml` — cross-repo dispatch deferred
- `worldfoundry.org/apt/docs/infra-setup.md` — PAT documentation deferred

## Verification

1. **Local generation, first run**
   ```bash
   task packages-page
   jq '.editions[] | {slug, installed_size_kb, package_count}' site/packages-data.json
   jq '.categories[] | {slug, installed_size_kb, package_count}' site/packages-data.json
   ```
   Expected: 3 editions (sizes increasing anvil < sprite < atelier), 9 categories each with `installed_size_kb > 0` and `package_count > 0`.

2. **Cache hit on second run**
   ```bash
   task packages-page
   ```
   Expected: prints `✓ no change in either apt repo since last generation — skipping` and exits ~instantly (~50 ms — just the two Release fetches).

3. **Force regenerate**
   ```bash
   task packages-page -- --force
   ```
   Expected: container spins up, ~30 s, refreshed `generated_at`.

4. **SSR builds both pages**
   ```bash
   task site-build
   ls -la site/index.html site/packages.html
   ```
   Expected: both files non-empty; `site/packages.html` references all 9 category titles + all 3 edition titles.

5. **Home page lost the lower-4 per-tool cards**
   ```bash
   grep -Eo '(f9dasm|65ax|libvgm|vgmstream)' site/index.html | sort -u
   ```
   Expected: empty — none of those four tool names appear on the home page anymore.

6. **Home page Forge has six category headliners**
   ```bash
   grep -Eo '(World Foundry GDK|Blender|Retro toolkit|Emulators|Audio &amp; art|Games &amp; mobile dev)' site/index.html | sort -u | wc -l
   ```
   Expected: 6.

7. **Home page gained Forge framing + catalogue link**
   ```bash
   grep -E '(The Forge|push the ones worth shipping upstream|/packages)' site/index.html
   ```
   Expected: Forge section header, upstream positioning sentence, and a link to `/packages`.

8. **Audit is empty / clean**
   ```bash
   jq '.audit' site/packages-data.json
   ```
   Expected: `unowned_packages: []`, `missing_metapackages: []`, `out_of_catalogue` may have 1 entry (`worldfoundry-blender-editor-exporter`).

9. **Upstream summary sanity**
   ```bash
   jq '.upstream_summary' site/packages-data.json
   ```
   Expected: `vendored: 4` (f9dasm, ghidra, libvgm, vgmstream), `n/a-firstparty: 11` (worldfoundry-* + blender-asset-finder*), `in-ubuntu-universe` and `in-ubuntu-multiverse` non-zero, `debian-itp: 0` (until we file one).

10. **Vendored standalones surface**
    ```bash
    jq -r '.vendored_standalones[] | .name' site/packages-data.json
    ```
    Expected (6 names): `f9dasm`, `ghidra`, `libvgm`, `vgmstream`, `blender-asset-finder`, `blender-asset-finder-cli`.

11. **Preview locally**
    ```bash
    cd site && python3 -m http.server 8080
    # open http://localhost:8080/
    # open http://localhost:8080/packages.html
    ```
    Expected: home page renders with 6 Forge cards (top: GDK, Blender, Retro toolkit; bottom: Emulators, Audio & art, Games & mobile dev) — counts and sizes filled in; `/packages` renders Editions block + 9 category sections + Vendored standalones + Upstreaming + Alignment.

12. **Card → category link integrity**
    ```bash
    grep -Eo 'href="/packages#[a-z-]+"' site/index.html | sort -u
    ```
    Expected: 6 anchors, each matching a `categories[].slug` value in `packages-data.json`. Build fails loudly if a Forge card references a slug not present in the JSON.

## Follow-ups (separate plans)

- **packages-page CI triggers** — `workflow_run` on foundry-apt publish, `repository_dispatch` from worldfoundry.org with new `FOUNDRYLINUX_DISPATCH_PAT` secret, nightly cron fallback. See §6.
- **File first Debian ITP** — `f9dasm` is the natural candidate (small, self-contained, debhelper build, no patches). Once landed, `data/upstream.yml` flips `f9dasm.status: vendored` → `debian-itp` with the bug number.
- **Repology badges** — add a `<img src="https://repology.org/badge/latest-versions/{name}.svg">` badge per vendored package row on `/packages` (and optionally on `apt.foundrylinux.org`). Shows at a glance whether our vendored package (ghidra, f9dasm, libvgm, vgmstream) is current against upstream. See [apt listing landscape investigation](../investigations/2026-05-28-apt-listing-landscape.md).
- **Copy-to-clipboard for install commands** — add a small clipboard button beside each `apt install` snippet on `/packages` (edition cards + category headers) and on `apt.foundrylinux.org`'s quick-install block. Pure JS, no server dependency. See [apt listing landscape investigation](../investigations/2026-05-28-apt-listing-landscape.md).
- **Changelog popover on hover** — show the latest `debian/changelog` entry (first stanza) as a `<details>` or CSS tooltip on the version cell of each package row on `/packages` and `apt.foundrylinux.org`. No page navigation required; first stanza is always short. See [apt listing landscape investigation](../investigations/2026-05-28-apt-listing-landscape.md).
- **Metapackage dependency expansion** — for umbrella packages (`foundry-retro-tools`, `foundry-emulators`, edition metapackages), show the resolved direct-depends list inline (collapsed by default) so users know what they're getting before running `apt install`. Data already in `packages-data.json`; rendering is a client-side toggle. See [apt listing landscape investigation](../investigations/2026-05-28-apt-listing-landscape.md).
- **Update `new-web-apt-repo` skill** — fold the enhanced listing patterns (Repology badges, copy-to-clipboard, changelog popover, metapackage expansion) into the skill's generated `generate-index.sh` template so new repos start with the full set. See [apt listing landscape investigation](../investigations/2026-05-28-apt-listing-landscape.md).
- **Port sortable columns + client-side filter from `worldfoundry.org` `generate-index.sh`** — the sister repo's apt index already has both; bring them into `foundry-apt/scripts/generate-index.sh` and fold into the `new-web-apt-repo` skill template so all future repos start with them. See [apt listing landscape investigation](../investigations/2026-05-28-apt-listing-landscape.md).
- **`packages.json` index** — generate `public/packages.json` alongside `index.html` from `generate-index.sh`; same data (name, version, arch, description, homepage), different serialiser. Zero extra cost; future Repology ingestor or downstream tool can consume without scraping HTML. See [apt listing landscape investigation](../investigations/2026-05-28-apt-listing-landscape.md).
- **OpenGraph meta tags** — add `<meta property="og:title">`, `<meta property="og:description">`, `<meta property="og:image">` to `apt.foundrylinux.org/index.html` (and per-package detail pages if/when Option B ships). Link preview cards on Discord/Mastodon show package name + description instead of bare domain. See [apt listing landscape investigation](../investigations/2026-05-28-apt-listing-landscape.md).
- **RSS/Atom feed** — generate `public/feed.xml` at publish time: one `<item>` per package, title = "name version — description", pubDate from `debian/changelog`. Users subscribe once and get notified on version bumps without email or account. Emit the current top-of-changelog entry per package on every publish; RSS readers deduplicate by `<guid>` (use `name@version`). See [apt listing landscape investigation](../investigations/2026-05-28-apt-listing-landscape.md).
- **CVE tracker links (passive)** — add a shield icon column to each package row on `/packages` linking to `ubuntu.com/security/cves?package={name}` for Ubuntu-origin packages, and to the upstream GitHub Security Advisories page for vendored packages (ghidra → NationalSecurityAgency/ghidra; f9dasm/libvgm/vgmstream → ubuntu tracker as fallback). No build-time fetch; always live data when clicked. See [apt listing landscape investigation](../investigations/2026-05-28-apt-listing-landscape.md).
- **CVE badge with live count (option #2, deferred)** — at `task packages-page` build time, fetch `ubuntu.com/security/cves.json?package={name}&limit=1` per package (~100 requests, ~10–20 s extra) and cache active CVE count in `packages-data.json`; render `0 CVEs ✓` / `N active CVEs ⚠` badge. Only worth doing if passive links prove insufficient. Implement after option #1 ships.

---

## Verification results (2026-05-22)

Implementation landed in `db702ef feat(site): /packages page + Forge home reshape, generated from live apt`. All 12 steps PASS.

Pivot during implementation: original §1 design used an `ubuntu:26.04` container with `apt-cache depends --recurse / show / policy` per package — ~1,700 sequential shell calls totalling ~15 minutes. Killed and rewrote to fetch the 5 Packages.gz files in parallel and traverse Depends in Node (with virtual-package resolution via Provides). New runtime: **~3 seconds** (cache hit: ~1 s). No docker dependency.

Three minor adjustments during verification:
1. Categories.json initially listed a non-existent `worldfoundry-blender` metapackage; the real package is `worldfoundry-blender-addons`. Fixed.
2. Categories.json missed `worldfoundry-development` ownership; added to the WF GDK category (it's the engine build-deps layer).
3. Vendored-standalones filter was too narrow (excluded `n/a-firstparty`). Widened to "every leaf in `origin === 'foundry'`" so the blender-asset-finder pair shows up (the cross-repo first-party bridge). WF CLIs in apt.worldfoundry.org stay in the GDK category, not the standalones list.

### 1. Local generation — PASS
```
$ task packages-page -- --force
· fetching Packages.gz from 5 archives (parallel)
  · merged 74500 packages, 71343 virtual provides
· resolving editions
  · anvil      745 pkgs  3.72 GiB
  · sprite    1061 pkgs  5.00 GiB
  · atelier   1281 pkgs  18.27 GiB
· resolving categories
  · worldfoundry-gdk        21 pkgs  208 MiB
  · blender                  3 pkgs  156 MiB
  · retro-tools             15 pkgs  1.33 GiB
  · emulators               23 pkgs  597 MiB
  · game-frameworks          9 pkgs  58 MiB
  · image-tools              9 pkgs  246 MiB
  · audio-production        11 pkgs  179 MiB
  · games                   35 pkgs  9.43 GiB
  · mobile-dev               7 pkgs  2.04 GiB
  audit: unowned=0 missing-metapackages=0
```

### 2. Cache hit on second run — PASS (~1 s)
```
$ task packages-page
✓ no change in apt repos or local config — skipping
```

### 3. Force regenerate — PASS (~3 s)

### 4. SSR builds both pages — PASS
```
✓ site/index.html      rendered (22 KB of markup, no React runtime)
✓ site/packages.html   rendered (65 KB of markup, no React runtime)
```

### 5. Home page lost the lower-4 per-tool cards — PASS
```
$ grep -Eo '(f9dasm|65ax|libvgm|vgmstream)' site/index.html | sort -u
(empty)
```
(The Retro toolkit blurb originally enumerated those tool names; tightened the blurb so the names live only on /packages.)

### 6. Home page Forge has 6 categories — PASS
```
$ grep -Eo '(World Foundry GDK|Blender|Retro toolkit|Emulators|Audio &amp; trackers|Games &amp; reimplementations)' site/index.html | sort -u
Audio &amp; trackers · Blender · Emulators · Games &amp; reimplementations · Retro toolkit · World Foundry GDK
```

### 7. Forge framing + catalogue link — PASS
```
$ grep -Eo '(The Forge|push the ones worth shipping upstream|/packages)' site/index.html | sort -u
/packages · The Forge · push the ones worth shipping upstream
```

### 8. Audit clean — PASS
```
$ jq '.audit' site/packages-data.json
{ "unowned_packages": [], "missing_metapackages": [], "out_of_catalogue": [] }
```

### 9. Upstream summary — PASS
```
{ "vendored": 4, "debian-itp": 0, "in-debian-unstable": 0,
  "in-ubuntu-universe": 94, "in-ubuntu-multiverse": 12, "in-ubuntu-main": 6,
  "n/a-firstparty": 14, "n/a-proprietary": 0, "unknown": 0 }
```

### 10. Vendored standalones — PASS (6 names, mix of vendored + first-party)
```
blender-asset-finder — n/a-firstparty
blender-asset-finder-cli — n/a-firstparty
f9dasm — vendored
ghidra — vendored
libvgm — vendored
vgmstream — vendored
```

### 11. Preview locally — PASS

Both pages render via `cd site && python3 -m http.server 8080`. Home has the 6-card Forge grid + Editions ladder; /packages has the Editions header + 9 category sections + Vendored standalones + Upstreaming + Alignment.

### 12. Card → category link integrity — PASS
```
$ grep -Eo 'href="/packages#[a-z-]+"' site/index.html | sort -u
href="/packages#audio-production"
href="/packages#blender"
href="/packages#emulators"
href="/packages#games"
href="/packages#retro-tools"
href="/packages#worldfoundry-gdk"
```
All 6 slugs exist in `packages-data.json` `.categories[].slug`.
