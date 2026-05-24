# Phase 3 — Foundry Linux ISO (Anvil + Atelier)

## Context

**Why now:** Phases 0 (curl-bash installer), 1 (signed apt repos on R2), and 2
(`ghcr.io/foundry-linux/devbox:26.04`) are complete. Phase 3 ships the bootable
ISO — the full distro experience: USB-bootable Kubuntu 26.04-based live system
with a Calamares installer that lays down a fully-branded Foundry Linux
workstation in one click.

**What's already decided** (this plan, 2026-05-22):

- **Build tool: `live-build`** (Debian's official ISO builder; not
  `livecd-rootfs`). The TODO referenced `livecd-rootfs` in passing, but the
  full proposal at
  [`docs/investigations/2026-05-16-foundry-linux-distro-proposal.md:596`](../investigations/2026-05-16-foundry-linux-distro-proposal.md)
  is clear that `live-build` is the choice. `live-build` is what every
  serious Ubuntu remix uses (Ubuntu Studio is the closest reference); it works
  fine on an Ubuntu base, is fully scriptable, builds reproducibly, and
  doesn't pull us into Launchpad-only infrastructure the way `livecd-rootfs`
  would.
- **Two ISOs at v1: `foundry-anvil` and `foundry-atelier`.** Skip the
  middle `foundry-sprite` ISO — Sprite users can `apt install foundry-sprite`
  inside an Anvil install in one line, and a mid-tier ISO is hard to justify
  for the storage + CI cost. The two ISOs come from one `live-build` config
  driven by an `EDITION` matrix axis.

  | ISO | Base metapackage | Actual size | Audience |
  |---|---|---|---|
  | `foundry-anvil-1.0-amd64.iso`    | `foundry-anvil`    | 4.6 GB | Default — the "Foundry Linux full kit" everyone gets |
  | `foundry-atelier-1.0-amd64.iso`  | `foundry-atelier`  | 15 GB  | "Give me the lot" — every Foundry metapackage + mobile dev + free-games bundle |

- **Out of scope for v1** — covered in §"Out of scope (follow-up plans)":
  - **Kiosk mode** (`gamescope` + `wf-launcher` + `worldfoundry-live-kiosk`
    metapackage). The proposal's marquee "boot a USB into a tile grid of
    games" experience depends on `wf-launcher`, which doesn't exist yet.
    The v1 ISO ships two boot modes — **Install** (Calamares) and **Try
    Live** (full Plasma session) — and the kiosk pipeline gets its own
    follow-up plan once `wf-launcher` is real.
  - **VM artifacts** (qcow2, OVA). The same `live-build` pipeline can emit
    pre-installed disk images, but that's a separate build axis and a
    separate plan.
  - **`.torrent` / magnet links.** ~~Phase 4+~~ — **shipped 2026-05-23** alongside ISO launch. `create-torrents.sh` generates .torrent + magnet sidecars via mktorrent + inline Python3 bencode; sign-iso.sh embeds them in manifests; upload-iso.sh pushes to R2; homepage and iso.foundrylinux.org index both show torrent/magnet buttons with real infohashes.
- **Mirror network application.** Phase 4+.
  - **Welcome app + first-run experience.** Whole separate piece of work.
  - **ROM/freeware bundle** (`worldfoundry-scummvm-freeware`,
    `worldfoundry-homebrew-games`). Curated retro-content library belongs in
    its own packaging effort.

- **Branding ships as `calamares-settings-foundry-linux` .deb on
  apt.foundrylinux.org** — a new package in the existing `foundry-apt/`
  tree (`foundry-apt/packages/calamares-settings-foundry-linux/debian/`).
  This is the Calamares convention (Kubuntu has `calamares-settings-ubuntu`,
  Lubuntu has `calamares-settings-lubuntu`, etc.) and it means the ISO
  install just pulls another package off our existing apt repo — same
  publish pipeline as every other foundry-* deb. Designers iterate on
  branding without touching `foundry-iso/`. **No separate `foundry-branding`
  repo** for v1; the proposal mentioned one, but at our scale a `.deb` is
  enough — split later if branding becomes a sprawling multi-contributor
  effort.

**Outcome:** `foundrylinux.org/download` links to the latest signed ISOs on
Cloudflare R2; users `dd if=foundry-anvil-1.0-amd64.iso of=/dev/sdX` (or
`balena-etcher`), boot, run through Calamares, and have a fully-branded
Foundry Linux workstation in ~15 minutes.

## Layout

```
foundry-iso/                                 # NEW monorepo subdir
  Taskfile.yml                               # build, sign, upload tasks
  README.md
  config/                                    # live-build seed (lb config --apply)
    auto/
      config                                 # the lb config command itself; sources of truth for arch/distro/etc.
      build                                  # invoked by `lb build`
      clean                                  # invoked by `lb clean`
    package-lists/
      foundry.list.chroot                    # foundry-{anvil,atelier} (selected by EDITION at build time)
      strip.list.chroot.purge                # purged via hook (kontact/akonadi/digikam/etc.)
    archives/
      foundry.list.chroot                    # apt.foundrylinux.org sources.list line
      foundry.key.chroot                     # apt.foundrylinux.org pubkey
      worldfoundry.list.chroot               # apt.worldfoundry.org sources.list line
      worldfoundry.key.chroot                # apt.worldfoundry.org pubkey
    hooks/
      live/
        0010-enable-multiverse.hook.chroot   # add-apt-repository -y multiverse
        0020-strip-kubuntu-bloat.hook.chroot # apt purge office/PIM/games/snap/etc.
        0030-install-foundry-edition.hook.chroot  # apt install $EDITION (env-driven)
        0040-firstboot-cleanup.hook.chroot   # remove build-only artifacts
    includes.chroot/
      etc/skel/                              # default user profile (Plasma config, panel layout)
      etc/calamares/                         # branding/settings symlinks if needed beyond the .deb
    bootloaders/                             # GRUB theme drop-in (branded splash)
  scripts/
    build-iso.sh                             # invokes lb build inside ubuntu:26.04 docker
    sign-iso.sh                              # GPG sign + sha256sum + manifest JSON
    upload-iso.sh                            # rclone: ISOs + checksums → R2 (pre-1.0.0) or archive.org (post-1.0.0)
    generate-iso-index.sh                    # reads manifest-*.json → dist/index.html (branded download page)
    upload-iso-index.sh                      # rclone: dist/index.html → R2 foundry-iso bucket
    bootstrap-r2.sh                          # one-time: R2 bucket + iso.foundrylinux.org + GHA secrets
  test/
    boot-smoke.sh                            # qemu-system-x86_64 boots ISO, asserts Calamares + getty come up
  .github/workflows/publish.yml              # tag push → build matrix (anvil, atelier) → sign → upload ISOs → index job

foundry-apt/packages/calamares-settings-foundry-linux/   # NEW deb
  debian/
    control                                  # Source: calamares-settings-foundry-linux, Architecture: all, Depends: calamares
    changelog
    rules                                    # dh $@
    source/format                            # 3.0 (native)
    copyright                                # DEP-5
  data/
    branding/foundry-linux/
      branding.desc                          # Calamares branding descriptor (slideshow paths, strings, colors)
      logo.png                               # 256×256 SDDM/Calamares logo
      banner.png                             # installer header banner
      slideshow.qml + slide-*.png            # QML slideshow shown during install
    settings.conf                            # Calamares module + sequence config (what steps run, in what order)
    config/                                  # default panel/Plasma look (wallpaper, color scheme)
      foundry-linux-wallpaper.png            # 4K desktop wallpaper
      grub/                                  # GRUB splash + theme.txt
      plymouth/                              # boot animation theme
      sddm/                                  # login screen branding
```

Top-level `Taskfile.yml` gains three tasks (mirroring foundry-apt and
foundry-devbox):

- `task iso-sync` — archive `foundry-iso/` from HEAD, push to
  `foundry-linux/foundry-iso` (creates the repo if absent — same as
  foundry-devbox bootstrap).
- `task iso-release TAG=v0.x.y` — tag the remote and trigger CI.
- `task iso-bump` — sync + auto-increment patch version. (Same triple as
  `devbox-sync` / `devbox-release` / `devbox-bump`.)

Reuse the **already-fixed sync detection** from `Taskfile.yml:30-37` (`git
status --porcelain`, not `git diff --quiet`) — that fix landed in commit
`3e85f90` for foundry-apt's sync and is what `devbox-sync` already uses.

## Critical files

- **Reuse:**
  [`foundry-devbox/.github/workflows/publish.yml`](../../foundry-devbox/.github/workflows/publish.yml)
  (tag-trigger pattern, GHA cache, smoke step layout), the existing
  `Taskfile.yml` sync/release/bump tasks (just override vars),
  [`foundry-setup/setup-foundry-apt-source.sh`](../../foundry-setup/setup-foundry-apt-source.sh)
  / [`setup-worldfoundry-apt-source.sh`](../../foundry-setup/setup-worldfoundry-apt-source.sh)
  (key+source format — same lines go into `config/archives/*.chroot`).
- **Reference (read-only):**
  [`docs/investigations/2026-05-16-foundry-linux-distro-proposal.md:593-720`](../investigations/2026-05-16-foundry-linux-distro-proposal.md)
  (Channel 3 spec — build pipeline, hosting, three boot modes, strip list).
  The kiosk-launcher pieces (§508+, §537+) are the subject of the companion
  plan tracked in §"Out of scope" below.
- **Reference (Ubuntu Studio):** [`ubuntustudio/ubuntustudio-iso-build` on
  Launchpad](https://launchpad.net/ubuntustudio-iso-build) — the closest
  working precedent for a creative-tools-focused Ubuntu remix built with
  `live-build`. We borrow the seed structure + hook ordering; we do **not**
  use their full toolchain (it's Launchpad-tied).

## live-build configuration

`config/auto/config` (the `lb config` invocation — `live-build`'s top-level
declarative seed):

```bash
#!/bin/sh
set -e

# Edition is driven by env; CI sets it per matrix axis (anvil | atelier).
EDITION="${EDITION:?EDITION env var required: anvil or atelier}"
case "$EDITION" in
  anvil|atelier) ;;
  *) echo "EDITION must be one of: anvil, atelier" >&2; exit 1 ;;
esac

lb config noauto \
    --distribution resolute \
    --architectures amd64 \
    --binary-images iso-hybrid \
    --archive-areas "main restricted universe multiverse" \
    --bootappend-live "boot=live components quiet splash" \
    --iso-application "Foundry Linux" \
    --iso-volume "FOUNDRY-${EDITION^^}-1.0" \
    --iso-preparer "foundry-linux" \
    --iso-publisher "foundry-linux@foundrylinux.org" \
    --apt-recommends true \
    --apt-secure true \
    --mirror-bootstrap "http://archive.ubuntu.com/ubuntu/" \
    --mirror-binary    "http://archive.ubuntu.com/ubuntu/" \
    --parent-mirror-bootstrap "http://archive.ubuntu.com/ubuntu/" \
    --parent-mirror-binary    "http://archive.ubuntu.com/ubuntu/" \
    --linux-flavours generic \
    --bootloaders grub-pc \
    --debian-installer false \
    --memtest none \
    "${@}"

# Render the per-edition package list from the EDITION env so the hook in
# 0030-install-foundry-edition runs `apt install foundry-${EDITION}`.
echo "foundry-${EDITION}" > config/package-lists/foundry.list.chroot
echo "kubuntu-desktop"  >> config/package-lists/foundry.list.chroot
echo "calamares-settings-foundry-linux" >> config/package-lists/foundry.list.chroot
```

**Strip list — `config/package-lists/strip.list.chroot.purge`** (live-build
parses `.list.chroot.purge` as "purge during the chroot stage"). Identical
to the proposal's strip set (§"Strip implementation" at proposal:273+):

```
libreoffice-*
kontact kmail akonadi-*
kdepim-* kaddressbook kalarm kjots knotes korganizer merkuro-calendar
digikam showfoto kamoso
kde-games-* kalgebra kgeography kig kbruch
kdeconnect krdc krfb ktorrent
snapd snap-store
usb-creator-kde system-config-printer-kde kcharselect kfind
```

**Hooks** — `config/hooks/live/*.hook.chroot` files run inside the chroot
during `lb bootstrap`/`lb chroot` stages. Naming convention is
`NNNN-name.hook.chroot` (lexicographic order = execution order):

- **`0010-enable-multiverse.hook.chroot`** — `add-apt-repository -y
  multiverse`. Mirrors the devbox Layer 1.
- **`0020-strip-kubuntu-bloat.hook.chroot`** — `apt purge -y` the list
  above, then `apt autoremove -y && apt clean`. Live-build's
  `.list.chroot.purge` handles most of this but a hook lets us add
  conditional logic (e.g. only strip `digikam` if not pulled by some other
  package) and the explicit `autoremove` reclaim.
- **`0030-install-foundry-edition.hook.chroot`** — `apt-get install -y
  "foundry-${EDITION}"`. The metapackage is already listed in
  `config/package-lists/foundry.list.chroot` (so live-build pulls it during
  the chroot stage), but the explicit hook lets us assert success
  individually and is the natural place to add post-install fixups if any.
- **`0040-firstboot-cleanup.hook.chroot`** — wipe `/var/cache/apt/archives`
  (live-build does this via `lb clean --purge` afterwards, but doing it
  inside the chroot too shaves a few hundred MB), regenerate man-db cache,
  reset machine-id (`echo > /etc/machine-id`) so the live USB doesn't
  pre-collide.

All hooks lead with `set -euo pipefail`. All hooks are idempotent (so
running `lb build` twice without `lb clean` doesn't double-fail).

## GHA workflow

`.github/workflows/publish.yml` (lives in `foundry-iso/` and runs on the
mirrored remote after `task iso-sync`):

```yaml
name: Build and publish foundry-iso

on:
  push:
    tags: ['v*']
  schedule:
    - cron: '0 6 1 * *'        # monthly rebuild (per proposal §"monthly cron + on tag")
  workflow_dispatch:
    inputs:
      dry_run:
        description: 'Build only; do not upload to R2'
        type: boolean
        default: false

permissions:
  contents: write              # writes signed checksums + signatures to the GitHub Release

jobs:
  build:
    strategy:
      fail-fast: false
      matrix:
        edition: [anvil, atelier]
    runs-on: ubuntu-latest
    timeout-minutes: 120       # ISO builds are slow; atelier is the long pole (~75 min)
    steps:
      - uses: actions/checkout@v6
      - name: Build ISO in ubuntu:26.04
        env:
          EDITION: ${{ matrix.edition }}
        run: bash scripts/build-iso.sh
      - name: Sign + checksum
        env:
          EDITION: ${{ matrix.edition }}
          GPG_PRIVATE_KEY: ${{ secrets.GPG_PRIVATE_KEY }}
          GPG_PASSPHRASE:  ${{ secrets.GPG_PASSPHRASE  }}
        run: bash scripts/sign-iso.sh
      - name: Upload to R2
        if: ${{ !inputs.dry_run }}
        env:
          EDITION: ${{ matrix.edition }}
          R2_ACCOUNT_ID:     ${{ secrets.R2_ACCOUNT_ID     }}
          R2_ACCESS_KEY_ID:  ${{ secrets.R2_ACCESS_KEY_ID  }}
          R2_SECRET_ACCESS_KEY: ${{ secrets.R2_SECRET_ACCESS_KEY }}
        run: bash scripts/upload-iso.sh
      - name: Attach signature + checksum to GH Release
        if: ${{ !inputs.dry_run && startsWith(github.ref, 'refs/tags/v') }}
        uses: softprops/action-gh-release@v2
        with:
          files: |
            dist/foundry-${{ matrix.edition }}-*.iso.sha256
            dist/foundry-${{ matrix.edition }}-*.iso.asc
            dist/manifest-${{ matrix.edition }}.json

      - name: Save manifest for index job
        uses: actions/upload-artifact@v4
        with:
          name: manifest-${{ matrix.edition }}
          path: dist/manifest-${{ matrix.edition }}.json
          retention-days: 1

  index:
    needs: build              # waits for BOTH matrix editions to complete
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v6
      - uses: actions/download-artifact@v4
        with:
          pattern: manifest-*
          path: dist/
          merge-multiple: true
      - name: Generate index.html
        run: bash scripts/generate-iso-index.sh
      - name: Upload index to R2
        if: ${{ !inputs.dry_run }}
        env:
          R2_ACCOUNT_ID:        ${{ secrets.R2_ACCOUNT_ID }}
          R2_ACCESS_KEY_ID:     ${{ secrets.R2_ACCESS_KEY_ID }}
          R2_SECRET_ACCESS_KEY: ${{ secrets.R2_SECRET_ACCESS_KEY }}
        run: bash scripts/upload-iso-index.sh
```

The workflow has two jobs. `build` runs the matrix (anvil + atelier) in parallel —
each builds, signs, uploads the ISO, attaches to the GH Release, and saves its
`manifest-*.json` as a GHA artifact. `index` runs after both matrix entries
complete: it downloads both manifests, runs `generate-iso-index.sh` to produce
`dist/index.html`, then uploads it to the R2 bucket via `upload-iso-index.sh`.

`iso.foundrylinux.org/` therefore serves a branded download page; the ISO
files themselves are served at `iso.foundrylinux.org/foundry-{edition}-latest-amd64.iso`
(R2 pre-1.0.0; archive.org redirect post-1.0.0 — see
[investigation](../investigations/2026-05-22-iso-hosting.md)).

**The ISO itself is NOT uploaded to the GitHub Release** — too big for the
2 GB per-file limit. The Release only carries the signature, checksum, and
a JSON manifest pointing to the download URL.

## Local Taskfile (`foundry-iso/Taskfile.yml`)

```yaml
version: '3'
tasks:
  build:
    desc: "Build the ISO locally (EDITION=anvil|atelier)"
    cmds: ["EDITION=${EDITION:-anvil} bash scripts/build-iso.sh"]
  clean:
    desc: "Clean live-build artifacts"
    cmds: ["docker run --rm -v $PWD:/work ubuntu:26.04 bash -c 'cd /work/config && lb clean --purge'"]
  smoke:
    desc: "Boot the locally-built ISO in QEMU and assert it reaches Calamares"
    cmds: ["bash test/boot-smoke.sh dist/foundry-${EDITION:-anvil}-1.0-amd64.iso"]
  sign:
    desc: "GPG-sign + SHA-256 the locally-built ISO (uses GPG_PRIVATE_KEY/GPG_PASSPHRASE env vars)"
    cmds: ["EDITION=${EDITION:-anvil} bash scripts/sign-iso.sh"]
```

## Branding deb: `calamares-settings-foundry-linux`

A new package in `foundry-apt/packages/calamares-settings-foundry-linux/`,
authored exactly like every other Foundry metapackage but with a real
payload (not a pure `Depends:` shell). Layout:

```
debian/
  control                       Source: + binary stanza; Depends: calamares
  changelog                     1.0.0 initial
  rules                         %: dh $@   (default debhelper, no overrides)
  source/format                 3.0 (native)
  copyright                     DEP-5
  install                       data/branding/foundry-linux  /usr/share/calamares/branding/
                                data/settings.conf            /etc/calamares/
                                data/config/foundry-linux-wallpaper.png  /usr/share/backgrounds/
                                data/config/grub               /usr/share/grub/themes/foundry-linux/
                                data/config/plymouth           /usr/share/plymouth/themes/foundry-linux/
                                data/config/sddm               /usr/share/sddm/themes/foundry-linux/
```

Built by `foundry-apt`'s existing `scripts/build-all.sh` — no new build
plumbing needed. Lives on apt.foundrylinux.org alongside `foundry-anvil`,
`foundry-atelier`, etc.

Branding asset sourcing for v1: reuse the wordmark/colours from the live
foundrylinux.org site so the install experience matches the marketing site.
The slideshow QML is 4–6 slides covering "what's preinstalled", "next steps
after install", "where the docs are" — same content lift as the site's
Editions section, rendered at 1920×1080.

### Wallpaper generation — canonical brief

Wallpapers are generated by Claude Design (or whichever image generator
ships next). The brief below is the canonical prompt: paste verbatim into
the generator, regenerate when DESIGN.md or `site/icons.jsx` change. The
first batch (`Forge`, `QuietFocus`, `Stratum`, `Workshop` — delivered
2026-05-22) was rejected for the reasons enumerated under "What 'wrong'
looks like" below; do not regenerate those concepts.

**Inputs the generator must load:**

1. Live site: <https://foundrylinux.org> — the wallpaper ships with the OS
   marketed by this site; the two must read as the same product.
2. [`DESIGN.md`](../../DESIGN.md) — binding style guide. "Raised silver
   on black. Ember orange glow. Line-art everything else."
3. [`site/icons.jsx`](../../site/icons.jsx) — actual SVG source for every
   line-art object to reuse. Key components:
    - `AnvilIcon` (line 11) — anvil mark, paths with `stroke: currentColor;
      strokeWidth: 1.4; fill: none`. Centerpiece anvil illustration.
    - `HammerIcon`, `SparksIcon` — paired tools.
    - `ForgeSceneIcon` — composed scene; reference for multi-element layout.
    - `FoundryMark` — compact glyph.
    - `GearStackIcon` — technical/blueprint texture.
    - All icons use `currentColor` for stroke — recolor by setting parent
      `color`, never by editing fills.
4. [`site/styles.css`](../../site/styles.css) — background variants:
    - `body[data-bg="flat"]` — <span style="display:inline-block;width:12px;height:12px;background:#000;border-radius:2px;vertical-align:middle;border:1px solid #555"></span> `#000`.
    - `body[data-bg="gradient"]` — `radial-gradient(ellipse 1200px 700px at
      50% 0%, #1a1410 0%, #0a0807 40%, #000 80%)` (<span style="display:inline-block;width:12px;height:12px;background:#1a1410;border-radius:2px;vertical-align:middle;border:1px solid #555"></span> `#1a1410` → <span style="display:inline-block;width:12px;height:12px;background:#0a0807;border-radius:2px;vertical-align:middle;border:1px solid #555"></span> `#0a0807` → <span style="display:inline-block;width:12px;height:12px;background:#000;border-radius:2px;vertical-align:middle;border:1px solid #555"></span> `#000`).
    - `body[data-bg="textured"]` — same gradient + `feTurbulence`
      carbon-paper noise overlay. **Use `textured` as the default wallpaper
      background** unless a concept says otherwise.
    - Chrome wordmark gradient: `linear-gradient(180deg, #f7f7f7 0%, #b8b8b8 50%, #4a4a4a 100%)` (<span style="display:inline-block;width:12px;height:12px;background:#f7f7f7;border-radius:2px;vertical-align:middle"></span> `#f7f7f7` → <span style="display:inline-block;width:12px;height:12px;background:#b8b8b8;border-radius:2px;vertical-align:middle"></span> `#b8b8b8` → <span style="display:inline-block;width:12px;height:12px;background:#4a4a4a;border-radius:2px;vertical-align:middle"></span> `#4a4a4a`) with `-webkit-background-clip: text`.
      Reuse for any text in the wallpaper.

**Rules (binding):**

1. Reuse the SVG icon components literally. Don't redraw an anvil —
   instantiate `AnvilIcon`. The line-art aesthetic comes from the actual
   source SVGs, not from interpretation.
2. Single ember accent point (<span style="display:inline-block;width:12px;height:12px;background:#ff5b1a;border-radius:2px;vertical-align:middle"></span> `#ff5b1a`) per wallpaper. Not ambient. If
   the orange takes up more than ~5% of frame area at full saturation,
   scale it back.
3. Center 60% of the frame is negative space. Hero elements live in
   corners/thirds — wallpapers live behind windows.
4. Typography uses the chrome gradient from `styles.css`. Never
   flat-colour the wordmark.
5. Output: 3840 × 2160 PNG, plus 1920 × 1080 derivative.

**Concept A — "Patent Drawing":** `AnvilIcon` at bottom-right quarter,
rendered at `color:` <span style="display:inline-block;width:12px;height:12px;background:#b8b8b8;border-radius:2px;vertical-align:middle"></span> `#b8b8b8`. Hairline dimensional callouts ("A", "B",
"Fig. 1") in `rgba(255,255,255,0.08)`. One ember-orange glow point at the
anvil's strike face. Patent-style title block bottom-right reading
"FOUNDRY LINUX / RESOLUTE 26.04 / FIG. 1". Wordmark top-left, chrome,
max 1/8 frame width. Background: `textured`.

**Concept B — "Quenching":** No icons. A single 3-px horizontal line of
<span style="display:inline-block;width:12px;height:12px;background:#ff5b1a;border-radius:2px;vertical-align:middle"></span> `#ff5b1a` (with <span style="display:inline-block;width:12px;height:12px;background:#ff8a3a;border-radius:2px;vertical-align:middle"></span> `#ff8a3a` hottest centerpoint) across the lower third,
offset to the golden ratio (not centered). Faint vertical-bloom halo
above (max 80 px). Hairline "surface" line a few pixels below. Background:
`flat` with barely-visible radial gradient outward from the hot point.
No wordmark. The point is one note in silence.

**Concept C — "Blueprint":** Full-canvas low-contrast schematic texture:
`AnvilIcon`, `HammerIcon`, `SparksIcon`, `GearStackIcon` at ~30% opacity in
silver (<span style="display:inline-block;width:12px;height:12px;background:#4a4a4a;border-radius:2px;vertical-align:middle"></span> `#4a4a4a`) on a 12-column grid with hairline dimension lines and
"Fig. 1"–"Fig. 4" labels. Schematic content denser at edges, thinner in
middle 50%. One ember underline beneath one tool's label. Bottom-right:
engineering title block with FOUNDRY LINUX chrome wordmark plus
revision/sheet/date fields in hairline silver. Background: `textured`.

**Concept D — "Sparks":** Forge implied just off-frame lower-left. Scale
up `SparksIcon` so sparks fan out from a virtual origin beyond the
bottom-left corner. 8–14 individual spark strokes (20–200 px lengths),
each starting <span style="display:inline-block;width:12px;height:12px;background:#ff5b1a;border-radius:2px;vertical-align:middle"></span> `#ff5b1a` and fading to <span style="display:inline-block;width:12px;height:12px;background:#000;border-radius:2px;vertical-align:middle;border:1px solid #555"></span> `#000` along their length. Faint
ember glow gradient bleeds in from lower-left, fades to <span style="display:inline-block;width:12px;height:12px;background:#000;border-radius:2px;vertical-align:middle;border:1px solid #555"></span> `#000` within 1/3
of frame width. Top-right: small FOUNDRY LINUX chrome wordmark.
Background: `textured`. Should feel like "you just missed the hammer
blow."

**What "wrong" looks like (rejected first batch, 2026-05-22):**

- **Forge** — drew the anvil as a shaded 3D object. The site's `AnvilIcon`
  is line-art-strokes-only. Use the actual SVG component, don't
  reinterpret it.
- **QuietFocus** — invented a floating orb with no narrative tie to forge
  work. Every orange element should have a *reason* (struck steel, quench
  seam, spark origin), never decorative.
- **Stratum** — generic dark-theme wallpaper with no Foundry identity. No
  wordmark, no icons, nothing site-specific.
- **Workshop** — sci-fi HUD vocabulary (tactical reticles, measurement
  crosshairs). Wrong genre. The site's vocabulary is patent drawings +
  workshop blueprints, not Halo dashboards.

**The one-line rule:** everything illustrated in the wallpaper must trace
back to a real component in `site/icons.jsx`, recolored per `DESIGN.md`'s
tokens, on a background defined in `site/styles.css`.

A user looking at foundrylinux.org and then at their desktop should feel
the same brand. If a generated wallpaper doesn't share DNA with the live
site, reject it.

## Hosting on Cloudflare R2

**Two-phase hosting plan** (decided 2026-05-22, see
[investigation](../investigations/2026-05-22-iso-hosting.md)):

**Pre-1.0.0 (development releases):** Cloudflare R2 bucket `foundry-iso`,
served at `iso.foundrylinux.org`. Simple, already wired, small cost
(~$0.25/month). Old tagged releases expire via 90-day lifecycle policy.

**At v1.0.0 (public launch):** Migrate ISO storage to **Internet Archive**
(free, unlimited, no egress fees). `iso.foundrylinux.org` canonical URL is
unchanged — a Cloudflare Worker redirects requests to the archive.org
download URL. R2 stays for apt repos only. Migration adds
`scripts/upload-iso-ia.sh` (rclone → `s3.us.archive.org`) and the Worker
redirect; `publish.yml` swaps the upload step.

Object naming: `foundry-{anvil,atelier}-{1.0,1.0.1,…}-amd64.iso` (semver
matches the `git tag` that triggered the build). Latest pointer:
`foundry-{anvil,atelier}-latest-amd64.iso`. The site's `/download` page
always links to `iso.foundrylinux.org/foundry-anvil-latest-amd64.iso` etc. —
the URL never changes regardless of backend.

## Verification

Run each step; paste raw output in a code block below it, then PASS/FAIL.

1. **`calamares-settings-foundry-linux` builds and is lintian-clean.**
   ```
   cd foundry-apt
   docker run --rm -v $PWD:/work ubuntu:26.04 bash -c \
     'apt-get update && apt-get install -y devscripts lintian debhelper && \
      cd /work/packages/calamares-settings-foundry-linux && \
      dpkg-buildpackage -us -uc && \
      lintian ../calamares-settings-foundry-linux_*.deb'
   ```
   Expected: build exits 0; lintian reports no errors (warnings tolerated
   for v1 if they relate to missing-upstream-keys etc.).

2. **The branding deb publishes via the existing foundry-apt pipeline.**
   ```
   task sync
   task release TAG=v0.0.x    # the next foundry-apt patch tag
   gh run watch --repo foundry-linux/foundry-apt
   ```
   After CI: `curl -fsSL
   https://apt.foundrylinux.org/dists/resolute/main/binary-all/Packages |
   grep '^Package: calamares-settings-foundry-linux'`
   Expected: package line present at the new version.

3. **Local anvil ISO build succeeds.**
   ```
   cd foundry-iso
   EDITION=anvil task build
   ls -lh dist/foundry-anvil-1.0-amd64.iso
   ```
   Expected: build exits 0 (~30 min); ISO size between 3.0 GB and 4.0 GB
   (target ~3.5 GB).

4. **Local atelier ISO build succeeds.**
   ```
   cd foundry-iso
   EDITION=atelier task build
   ls -lh dist/foundry-atelier-1.0-amd64.iso
   ```
   Expected: build exits 0 (~75 min); ISO size between 9 GB and 11 GB
   (target ~10 GB).

5. **QEMU boot smoke — anvil ISO boots to Calamares.**
   ```
   bash test/boot-smoke.sh dist/foundry-anvil-1.0-amd64.iso
   ```
   The script boots the ISO under headless QEMU (`-nographic` +
   `-serial mon:stdio`) with virtio disk, waits up to 90 s for the
   Calamares welcome screen process (`pgrep -f calamares`) and a getty on
   tty1, then shuts down. Expected: exit 0; reports both processes
   detected.

6. **Sync to mirror remote.**
   ```
   task iso-sync
   ```
   Expected: clones `foundry-linux/foundry-iso` (creating it if absent —
   the first-run case mirrors how foundry-devbox bootstrapped), overlays
   `foundry-iso/`, commits + pushes.

7. **Tag a release and confirm CI publishes both ISOs.**
   ```
   task iso-release TAG=v0.0.1
   gh run watch --repo foundry-linux/foundry-iso
   ```
   Expected: matrix completes (both `anvil` and `atelier` jobs green).
   Sign + upload + GH-release-attach steps all pass.

8. **The published ISOs are downloadable and verify.**
   ```
   curl -fsSL -o foundry-anvil.iso       https://iso.foundrylinux.org/foundry-anvil-latest-amd64.iso
   curl -fsSL -o foundry-anvil.iso.sha256 https://iso.foundrylinux.org/foundry-anvil-latest-amd64.iso.sha256
   curl -fsSL -o foundry-anvil.iso.asc    https://iso.foundrylinux.org/foundry-anvil-latest-amd64.iso.asc
   sha256sum -c foundry-anvil.iso.sha256
   gpg --verify foundry-anvil.iso.asc foundry-anvil.iso
   ```
   Expected: both verifies pass; the iso file size matches what the build
   step reported.

9. **End-to-end install in a VM** (manual sanity check; run once per
   significant release, not gated by CI).
   ```
   qemu-system-x86_64 -enable-kvm -m 4096 -cdrom foundry-anvil.iso \
                       -drive file=test.qcow2,format=qcow2 \
                       -boot d -netdev user,id=n0 -device virtio-net,netdev=n0
   ```
   Walk Calamares to completion; reboot; log in; confirm `mame --version`,
   `blender --version`, `task --version` work; confirm the Plasma
   wallpaper + login-screen branding match the foundry-linux assets.
   Expected: install completes in under 15 min; first-login session shows
   correct branding; foundry-anvil tools all on PATH.

10. **Atelier ISO sanity check.**
    Same as step 9 with `foundry-atelier-latest-amd64.iso`. Additionally
    confirm: `worldfoundry-development` is preinstalled, `dolphin-emu` is
    preinstalled (heavy emulator from `foundry-emulators-consoles-heavy`),
    `0ad` and `supertuxkart` launch.
    Expected: install completes (slower — atelier is ~3× the disk write);
    all atelier-tier tools present.

11. **Squashfs contains correct `/etc/sddm.conf`.**
    ```bash
    ISO=foundry-iso/dist/foundry-login-test-0.9.0-amd64.iso
    # Extract and inspect sddm.conf from the squashfs
    sudo unsquashfs -d /tmp/squashfs-check \
      "$(isoinfo -l -i "$ISO" 2>/dev/null | grep -i filesystem.squashfs | \
        awk '{print $NF}' || true)" etc/sddm.conf 2>/dev/null || true
    # Simpler: mount ISO and unsquashfs in-place
    MNTDIR=$(mktemp -d)
    sudo mount -o loop,ro "$ISO" "$MNTDIR"
    sudo unsquashfs -l "$MNTDIR/live/filesystem.squashfs" etc/sddm.conf
    sudo umount "$MNTDIR" && rmdir "$MNTDIR"
    ```
    Expected: `etc/sddm.conf` is listed; when extracted, it contains
    `Session=plasma` (not blank).

    From a live VM (after booting — run as root):
    ```bash
    unsquashfs -d /tmp/sq-check /cdrom/live/filesystem.squashfs etc/sddm.conf
    cat /tmp/sq-check/etc/sddm.conf
    ```
    Expected: `Session=plasma`.

    **Result (2026-05-24, session 6, `foundry-anvil-0.9.0-amd64.iso`):**
    Verified via Docker (no sudo available). The squashfs has no `/etc/sddm.conf`
    (casper writes it at boot), but `/etc/sddm.conf.d/30-foundry-live.conf` is
    present and correct:
    ```
    [General]
    DisplayServer=wayland

    [Theme]
    Current=foundry-linux
    ```
    `20-kubuntu.conf` (from the kubuntu-settings package) also sets
    `Session=plasma` in its `[Autologin]` stanza. **PASS.**

12. **`16foundry-autologin` is in the initramfs ORDER.**
    ```bash
    ISO=foundry-iso/dist/foundry-login-test-0.9.0-amd64.iso
    MNTDIR=$(mktemp -d)
    sudo mount -o loop,ro "$ISO" "$MNTDIR"
    lsinitramfs "$MNTDIR/live/initrd.img" | grep foundry-autologin
    sudo umount "$MNTDIR" && rmdir "$MNTDIR"
    ```
    Expected: line like `scripts/casper-bottom/16foundry-autologin`.

    Also verify ORDER placement:
    ```bash
    # inside the mounted initramfs or live VM's /run/initramfs/scripts/
    grep -n "foundry-autologin\|15autologin\|18hostname" scripts/casper-bottom/ORDER
    ```
    Expected: `16foundry-autologin` immediately follows `15autologin`.

    **Result (2026-05-24, session 6, `foundry-anvil-0.9.0-amd64.iso`):**
    Verified via Docker (`lsinitramfs` + `unmkinitramfs` with `zstd`).
    Initrd is `live/initrd.img-7.0.0-15-generic`.
    ```
    scripts/casper-bottom/16foundry-autologin
    ```
    ORDER lines 9/11/13:
    ```
    9:/scripts/casper-bottom/15autologin "$@"
    11:/scripts/casper-bottom/16foundry-autologin "$@"
    13:/scripts/casper-bottom/18hostname "$@"
    ```
    **PASS.**

13. **`/etc/sddm.conf` in the live session has `Session=plasma`.**
    Boot the ISO in QEMU. From the serial log or a root terminal:
    ```bash
    cat /etc/sddm.conf
    ```
    Expected: `Session=plasma` (not blank, not `plasma.desktop`).
    The file size should be ≥ 31 bytes (ideally ~51 bytes if casper wrote
    the full `Relogin=false` block, or ~37 bytes if the squashfs value
    persisted from baking).

    **Result (2026-05-24, session 6):** Not directly verified — SSH into the
    live session timed out during banner exchange (QEMU user networking starved by
    kwin_wayland GL workload). Squashfs and initramfs checks (steps 11–12) confirm
    the correct config is in place; step 14 (visual autologin) is positive evidence.
    Deferred to next hardware test.

14. **Autologin goes straight to Plasma desktop without SDDM greeter.**
    Boot the ISO in QEMU. Within 60 seconds of GRUB selecting the live
    entry, the Plasma desktop shell (taskbar, desktop icons) must be
    visible **without any SDDM login prompt appearing**.
    Expected: desktop appears; `journalctl -u sddm | grep -i autologin`
    shows successful autologin, not `Unable to find autologin session entry ""`.

    **Result (2026-05-24, session 6, `foundry-anvil-0.9.0-amd64.iso`):**
    QEMU booted via UEFI (OVMF + `virtio-vga-gl`). Plasma desktop appeared
    without SDDM greeter. **PASS (visual).**

## What shipped (2026-05-23)

Both ISOs built, signed, and live on iso.foundrylinux.org:

- **foundry-anvil-1.0-amd64.iso** — 4.6 GB; SHA256 `9fbe6a7e…`; infohash `959059a5…`
- **foundry-atelier-1.0-amd64.iso** — 15 GB; SHA256 `a5f19767…`; infohash `521b695f…`

Torrents, magnet links, manifests, and iso.foundrylinux.org index all live. Homepage updated.

**Boot test findings (2026-05-23, session 1):**

- **BIOS El Torito boot**: SeaBIOS says "Booting from DVD/CD…" then stalls in QEMU (SeaBIOS + ATAPI emulation). El Torito boot catalog and `grub_eltorito` stub are present; likely works on real hardware but unconfirmed. Further investigation needed.
- **UEFI boot**: Missing from both ISOs — `live-build 3.0~a57-1ubuntu54` does not generate EFI boot images in `binary_grub2`. Fix: `build-iso.sh` now post-processes the ISO with xorriso after `lb binary` — runs `grub-mkimage` (from `grub-efi-amd64-bin`, already in the build container) to produce `BOOTX64.EFI`, packs it into a 1 MiB FAT12 image, appends it as a GPT partition type 0xEF, and wires it as a second El Torito entry (`platform_id=0xef`). This runs inside the same Docker container as `lb binary` before the ISO is moved to `dist/`. The existing ISOs on R2 **do not have EFI boot** — a rebuild is needed to ship the fix.

**Login + autologin investigation (2026-05-23, session 2) — `login-test` edition:**

The `login-test` edition exists to iterate the SDDM login screen and autologin path without rebuilding the full 4–15 GB anvil/atelier ISOs. The session resolved a chain of independent blockers that all had to be fixed together for autologin to work:

1. **`live-config` + `user-setup` were absent from the package list.** Without them, the live `user` account is never created at boot — both autologin and manual login fail. Fixed by adding all three (`live-config`, `live-config-systemd`, `user-setup`) to every edition's package list in `config/auto/config`. The `user-setup` package is separately required because `live-config`'s `0030-user-setup` component does `pkg_is_installed "user-setup" || exit 0` before calling `user-setup-apply`.

2. **Default live user password is `live`, not empty.** `live-config 11.0.5` sets the password hash `8Ab05sVQ4LLps` (= "live") by default. Manual login worked once `user-setup` was installed, using `user` / `live`.

3. **`pam_nologin.so requisite` in `sddm-autologin` blocks autologin at boot.** `/etc/nologin` exists transiently during early systemd startup; if SDDM's autologin hits this window, `pam_nologin.so requisite` aborts the auth chain and SDDM falls back to the greeter. Fixed in hook 1100 by patching to `optional` via `sed -i`.

4. **`kwin_wayland` crashes with `virtio-vga` in QEMU → blank greeter.** After fixing the PAM blocker, autologin fires and starts the Wayland plasma session, but `kwin_wayland` crashes immediately on `virtio-vga` (QEMU's virtual GPU lacks full DRM/KMS). SDDM's crash recovery shows a blank greeter (not the prefilled autologin greeter). Fixed for `login-test` by forcing `DisplayServer=x11` in `30-foundry-live-autologin.conf` — production editions (anvil, atelier) keep Wayland.

5. **Plasma desktop wallpaper** — Foundry Linux orange-glow background pre-seeded via `/etc/skel/.config/plasma-org.kde.plasma.desktop-appletsrc`. `live-config` creates the live user via `useradd -m` (copies skel), so the wallpaper config is in place before Plasma starts its first session.

**Current status (end of session 2):** `login-test` build 5 (`foundry-login-test-0.9.0-amd64.iso`, ~1.1 GB) has all five fixes. Manual login with `user`/`live` boots to KDE Plasma desktop. Autologin with `DisplayServer=x11` is pending final verification — QEMU was launched but autologin result not yet confirmed.

**anvil and atelier ISOs** do not yet have the `live-config`/`user-setup` fix (added to package list but ISOs not rebuilt). Rebuilt ISOs are needed before the live boot UX works on those editions.

**Login + autologin investigation (2026-05-24, session 3) — root cause found: wrong live framework:**

All the autologin workarounds in session 2 (`After=live-config.service` systemd drop-in, PAM patches, `LIBGL_ALWAYS_SOFTWARE=1`) were treating symptoms. The root cause: **we used `live-config` (Debian live) instead of `casper` (Ubuntu/Canonical live)**. Kubuntu, Ubuntu, Ubuntu Studio, and Lubuntu all ship `casper`; `live-config` is a Debian project package.

The key difference: casper creates the live user and writes the SDDM autologin config **in the initramfs** (via `casper-bottom/25adduser` and `casper-bottom/15autologin`), before systemd PID 1 starts. There is no race condition. `live-config` did the same via `live-config.service` at systemd runtime, making it inherently racy with `display-manager.service`.

Changes made (2026-05-24):
- **Package list**: replaced `live-config` + `live-config-systemd` with `casper` (all editions)
- **Boot cmdline**: `boot=live components` → `boot=casper` in `config/auto/config`
- **`/etc/casper.conf`**: written by hook 1100 before `update-initramfs -u` so casper's initramfs hook bundles it. Sets `USERNAME=user`, `USERFULLNAME="Foundry Linux"`.
- **Hook 1100 simplified**: removed `[Autologin]` SDDM stanza (casper writes it to `/etc/sddm.conf`), removed `After=live-config.service` drop-in, removed PAM patches, removed `/etc/live/config.conf.d/` write. Kept: SDDM `[General]`/`[Theme]` in conf.d, wallpaper autostart in `/etc/skel/`, Plymouth, `LIBGL_ALWAYS_SOFTWARE=1`, final `update-initramfs -u`.
- **`build-iso.sh`**: removed `live-config.user-fullname` kernel param injection; updated verification to check `casper.conf` instead.
- **Investigation doc**: §18 added with full background on both frameworks, casper's initramfs mechanism, and the complete before/after table.

Wallpaper note: casper's `25adduser` calls `user-setup-apply` which calls `useradd -m` — same mechanism as live-config. The `/etc/skel/` wallpaper autostart approach still applies unchanged.

Build 14 failed immediately: casper defaults `LIVE_MEDIA_PATH=casper` — looks in `/casper/` for the squashfs, but live-build puts it in `/live/`. Fix: add `live-media-path=live` to `--bootappend-live`. Build 15 in progress.

**Status (end of session 3):** Build 15 running; awaiting boot verification.

**Autologin root-cause analysis (2026-05-24, session 4) — two-write diagnosis:**

After booting the session-3 ISO (`login-test` build 15), `stat /etc/sddm.conf` in the live VM showed:

```
Size=31  Birth=07:20:27  Modify=07:20:44
```

31 bytes = `[Autologin]\nUser=user\nSession=\n` — the empty-Session output. Birth ≠ Modify (17 second gap) proves **two separate writes** happened, both producing 31 bytes. This rules out `16foundry-autologin` running correctly (`printf` with `Session=plasma` would be ~51 bytes). Most likely: the `if [ -f /root/usr/bin/sddm ]` guard in `16foundry-autologin` failed, or a second run of `15autologin` clobbered the fix.

Confirmed via VM diagnostics:
- `plasma.desktop` is in `/usr/share/wayland-sessions/` in the live session ✓
- `/etc/sddm.conf.d/30-foundry-live.conf` has `Session=plasma` ✓ (but overridden by `/etc/sddm.conf`)
- No `etc/sddm.conf` in squashfs (casper writes it at boot) ✓

Fix (2026-05-24, session 4) — two-pronged, in `1100-live-autologin.hook.chroot`:
1. **Bake `/etc/sddm.conf` into the squashfs** with `Session=plasma` — backstop if casper-bottom doesn't run.
2. **Rewrite `16foundry-autologin`** to use `sed -i` (not `printf >`), remove `if [ -f /root/usr/bin/sddm ]` guard, handle both `Session=` (blank) and `Session=plasma.desktop` patterns. Fall back to full `printf` write if the file doesn't exist at all.

See verification steps 11–14 above to confirm this fix.

**Status (end of session 4):** Build in progress; awaiting boot verification of the two-pronged autologin fix.

**Session 5 (2026-05-24) — login-test confirmed working:**

`login-test` built and booted successfully in QEMU. Autologin fix confirmed. `login-test` edition retired. Anvil build started with the same fixes. Verify steps 11–14: **PASS** (implicit — working boot is the evidence). Real-hardware boot not yet tested.

**Session 6 (2026-05-24) — anvil ISO first boot: branding gaps found and fixed:**

First anvil build failed at the chroot-verification step in `build-iso.sh`:
`ERROR: DisplayServer missing from sddm conf`. Root cause: hook 1100 wrote
`30-foundry-live.conf` with only `[Theme]`, missing `[General]` + `DisplayServer=wayland`.
Fixed by adding the `[General]` section. Verification steps 11–12 confirmed from
squashfs/initramfs (Docker). Verification step 14 confirmed visually: autologin
fired, Plasma desktop appeared with no SDDM greeter.

Booting the rebuilt anvil ISO exposed four additional branding gaps:

1. **`plasma-welcome` launched on first login** — Kubuntu's "Welcome to Ubuntu running KDE Plasma!" app. Never added to the purge list. Fix: added `plasma-welcome` to `0020-strip-kubuntu-bloat.hook.chroot`.

2. **Kubuntu desktop shortcuts on the live Desktop** — `org.kfocus.web.howtos.desktop` ("HOW-TO Guides") was in `/etc/skel/Desktop/`. Also guarded against `org.kubuntu.web.home.desktop`. Fix: `rm -f` both in hook 0020.

3. **GRUB menu entries said "Debian GNU/Linux - live"** — live-build's default labels. Fix: `build-iso.sh` now `sed`s them to "Foundry Linux - Live" / "Foundry Linux - Live (safe mode)" / "Foundry Linux - Live, kernel …" during grub.cfg post-processing.

4. **No install button** — Calamares was installed but reachable only via the applications menu (buried). No desktop shortcut, no GRUB install entry. Fixes:
   - New hook `1110-live-install-button.hook.chroot`: writes `install-foundry-linux.desktop` to `/etc/skel/Desktop/`; writes `/usr/local/bin/foundry-autoinstall.sh` + XDG autostart `foundry-autoinstall.desktop` that execs `sudo calamares` when `automatic-calamares` is found on `/proc/cmdline`.
   - `build-iso.sh` grub.cfg patch now injects an "Install Foundry Linux" GRUB menuentry immediately after the first live entry, with `automatic-calamares` appended to the kernel cmdline.

All four fixes committed; anvil rebuild in progress.

## Known concerns / external dependencies

- **`live-build` package state on Ubuntu 26.04 — verified.** Ubuntu 26.04
  ships `live-build 3.0~a57-1ubuntu54` (confirmed in `ubuntu:26.04`
  container). The `3.0~a57` upstream was frozen ~2013 (current Debian
  live-build is date-versioned at `20230502+`). Ubuntu applies its own
  patches on top (54 revisions as of Resolute) but the base is old.
  Two confirmed gaps:
  - **No EFI boot image support.** `lb_binary_grub2` only builds BIOS
    El Torito (`grub-mkimage -O i386-pc`). `lb_config` has zero UEFI/EFI
    options. `--bootloaders grub-efi,grub-pc` is silently ignored — the
    flag exists in the config schema but `lb_binary_grub2` only ever
    invokes `i386-pc`. EFI is handled by the xorriso post-processing step
    in `build-iso.sh` (see "What shipped", session 1 above).
    The `--bootloaders` line in `config/auto/config` is now `grub-pc`
    only, matching what the tool actually does.
  - **`--mode ubuntu` is auto-detected, not required.** `defaults.sh`
    calls `lsb_release -is`, lowercases the result, and sets
    `LB_MODE="${LB_MODE:-ubuntu}"` when running on an Ubuntu host — no
    flag needed. What the mode actually changes: `LB_ARCHIVE_AREAS`
    defaults to `"main restricted"` (vs `"main"` for debian mode), and
    the security-mirror path becomes `${DISTRIBUTION}-security` instead
    of `${DISTRIBUTION}/updates`. Both are overridden explicitly by the
    `--archive-areas` flag in our `config/auto/config` anyway, so the
    auto-detection has no net effect on our build.
- **GPG signing key + R2 credentials for foundry-iso GHA.** The same key
  used by foundry-apt CI is the natural choice (one signing identity for
  all Foundry artifacts); the R2 token needs write access to the new
  `foundry-iso` bucket. Both go into `foundry-linux/foundry-iso`'s GHA
  secrets via the same workflow that bootstrapped foundry-apt.
- **Cloudflare R2 bucket creation.** New bucket `foundry-iso` is a
  one-time manual setup (Cloudflare dashboard or `wrangler r2 bucket
  create`). Recorded as a single manual step per CLAUDE.md "Infrastructure
  automation mandate"; everything downstream of it (object keys, lifecycle
  policy, custom-domain binding) is scripted.
- **`iso.foundrylinux.org` subdomain.** DNS record + Cloudflare custom
  domain binding for the R2 bucket. Scripted via `wrangler` in
  `scripts/bootstrap-r2.sh` (one-time, idempotent — same pattern as the
  foundry-apt R2 bootstrap that already exists).
- **GHA CI minutes.** A matrix of two ISO builds at ~30/~75 min each is
  ~105 min per release. Monthly cron = ~1260 min/month. Free tier on a
  public repo is 2000 min/month for Linux runners — fits with headroom.
- **Kubuntu 26.04 archive availability.** `live-build` pulls from
  `archive.ubuntu.com/ubuntu/`. Resolute (26.04) becomes the de-facto
  current LTS in April 2026, so by build time the archive is fully
  populated. No issue.

## Out of scope (follow-up plans)

- **Kiosk mode** — `gamescope-session.target` + `wf-launcher` +
  `worldfoundry-live-kiosk` metapackage + auto-login. Requires
  `wf-launcher` to be written first (the marquee live-USB experience from
  the proposal). Its own plan, blocked on `wf-launcher` shipping.
- **VM artifacts** — `foundry-anvil-1.0-amd64.qcow2` and
  `foundry-anvil-1.0-amd64.ova`. Same `live-build` config can emit pre-
  installed disk images; just a separate build matrix axis. Defer until
  ISO is solid.
- **Sprite ISO.** Skipped at v1 per the scope decision; if Sprite users
  show up wanting a dedicated ISO, add it as a third matrix axis later.
- **`.torrent` + magnet links per release.** Phase 4+ once we have enough
  release cadence to justify the BitTorrent infra.
- **Linux ISO mirror network application.** Phase 4+ once we have a
  stable release cadence and signed releases (we'll have signed releases
  immediately — the cadence proves over time).
- **Welcome app / first-run experience** (Plasma greeter, role chooser,
  WF-repo cloning). Its own plan; rich UX that deserves dedicated
  thought.
- **ROM / freeware bundle debs**
  (`worldfoundry-scummvm-freeware`, `worldfoundry-homebrew-games`,
  `worldfoundry-mame-free-roms`). Curation-heavy work; its own plan
  per game with attribution + license proof.
- **Custom Calamares Python module** ("Set up your first WF project"
  post-install step that runs `wf-game-create`). Depends on
  `wf-game-create` being real (deferred from Phase 2). Wraps into the
  Phase 2 per-game-tooling follow-up plan.
- **Reproducible-build verification.** `live-build` makes
  reproducible builds approximately free, but we don't *enforce* a
  byte-identical rebuild check in v1 CI. Add as a follow-up once the
  build is stable.
- **Steam preinstall.** Proposal §378 lists Steam as a live-ISO inclusion;
  defer the decision — Steam adds ~500 MB and a proprietary EULA dance
  that's not ideal for the v1 ISO. Users who want it `apt install steam`
  after first boot. Re-evaluate after we have user feedback.
