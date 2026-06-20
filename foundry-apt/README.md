# foundry-apt

The signed APT repo for [Foundry Linux](https://foundrylinux.org), hosted at `apt.foundrylinux.org`.

## Quick start

```bash
curl -fsSL https://apt.foundrylinux.org/key.gpg \
  | sudo gpg --dearmor -o /etc/apt/keyrings/foundry.gpg
echo "deb [signed-by=/etc/apt/keyrings/foundry.gpg] https://apt.foundrylinux.org resolute main" \
  | sudo tee /etc/apt/sources.list.d/foundry.list
sudo apt update
sudo apt install foundry-core   # full WF game-authoring toolkit (desktop-agnostic)
```

> **Note:** `foundry-core` cross-depends on `apt.worldfoundry.org` — wire that source too,
> or use [`setup.sh`](https://foundrylinux.org/setup.sh) to add both in one step.
> On a fresh Kubuntu 26.04 install (or the Foundry ISO), use `foundry-anvil` instead
> — it adds the KDE theming layer on top of core.

`resolute` is the suite name — Kubuntu 26.04 LTS "Resolute Raccoon", the only supported base.

## Metapackages

Foundry Linux ships as a tier of **edition** metapackages — each builds on the one below — plus **category** metapackages you can install à la carte. (`[WF]` marks a dep that lives in [`apt.worldfoundry.org`](https://apt.worldfoundry.org), so both apt sources must be wired — Phase 0's two `setup-*-apt-source.sh` scripts do this.)

### Editions (the ISO tiers)

The nesting is `foundry-core ⊆ foundry-anvil ⊆ foundry-sprite ⊆ foundry-atelier`, with `foundry-desktop` adding the KDE layer:

| Edition | Builds on | What it adds |
|---|---|---|
| `foundry-core` | — (base toolkit) | Desktop‑agnostic dev toolkit — `foundry-retro-tools`, `foundry-game-frameworks`, `foundry-image-cli`, `foundry-emulators-computers`, `foundry-emulators-consoles`, `foundry-python-gamedev`(+`-extras`), `worldfoundry`&nbsp;`[WF]`, plus `task`, `btop`, `firefox`. **This is what the devbox container installs.** |
| `foundry-desktop` | — | KDE desktop integration: `foundry-kde-theme` + `foundry-welcome`. ISO / KDE hosts only — kept **out** of the container. |
| `foundry-anvil` | `core` + `desktop` | The developer toolkit on a themed KDE desktop. **Default ISO edition.** |
| `foundry-sprite` | `anvil` | + heavy graphics/audio: `foundry-art`, `foundry-pixel-art`, `foundry-trackers`, `foundry-daw`, `digikam`, `showfoto`. |
| `foundry-atelier` | `sprite` | **Complete edition — everything:** + `foundry-emulators-vintage`, `foundry-emulators-consoles-heavy`, `foundry-game-reimplementations`, `foundry-free-games`, `foundry-android-development`, `foundry-ios-development`, `worldfoundry-development`&nbsp;`[WF]`, [`ghidra`](https://ghidra-sre.org/) (atelier-only — see below). |

### Category metapackages (install à la carte)

| Package | Pulls in |
|---|---|
| `foundry-retro-tools` | [mame](https://www.mamedev.org/), [dasm](https://dasm-assembler.github.io/), [cc65](https://cc65.github.io/), z80*, [radare2](https://www.radare.org/), [binwalk](https://github.com/ReFirmLabs/binwalk), [sox](http://sox.sourceforge.net/), m68k binutils, [xa65](https://www.floodgap.com/retrotech/xa/), [f9dasm](https://github.com/Arakula/f9dasm), [vgmstream](https://vgmstream.org/), [libvgm](https://github.com/ValleyBell/libvgm), [cvise](https://github.com/marxin/cvise) + delta (C/C++ test-case reducers; 1.0.14) — **ghidra moved to `foundry-atelier`** (1.0.7, 2026-06-04: its ~860 MiB of jars kept anvil off a 4 GB USB stick) |
| `foundry-game-frameworks` | 2D/3D game‑dev frameworks, headers, and shader tools |
| `foundry-image-cli` | command‑line image manipulation utilities |
| `foundry-python-gamedev` / `-extras` | Python game‑dev base + extras stack |
| `foundry-art` | raster, vector, and pixel‑art suites |
| `foundry-pixel-art` | pixel‑art paint programs |
| `foundry-daw` | Digital Audio Workstation stack and trackers |
| `foundry-trackers` | chiptune and module trackers |
| `foundry-emulators` | umbrella over the four emulator metas below |
| `foundry-emulators-computers` | vintage home‑computer emulators (no bundled ROMs) |
| `foundry-emulators-consoles` | light console and adventure‑engine emulators |
| `foundry-emulators-consoles-heavy` | heavy console and handheld emulators |
| `foundry-emulators-vintage` | vintage home‑computer emulators **with** bundled ROMs (multiverse) |
| `foundry-game-reimplementations` | FOSS engines for commercial games (bring your own data) |
| `foundry-free-games` | bundled FOSS games with multi‑GB data packs |
| `foundry-android-development` | Android dev tools (JDK 17, adb, NDK r26c) |
| `foundry-ios-development` | iOS device tools (libimobiledevice, ideviceinstaller, ifuse) |
| `foundry-kde-theme` | KDE Plasma desktop theme |
| `calamares-settings-foundry-linux` | Calamares installer branding + settings (shipped on the ISO) |

> `foundry-welcome` (the QML/Kirigami first‑login app) is **not** a metapackage — it's built native‑from‑source and pulled in by `foundry-desktop`.

## Vendored upstream packages

Packages not in the Ubuntu archive, repackaged and shipped from this repo via [`dpkg-buildpackage`](https://manpages.debian.org/dpkg-dev/dpkg-buildpackage.1.html) using the canonical Debian source-package layout:

| Package | Upstream | Notes |
|---|---|---|
| `f9dasm` | [Arakula/f9dasm](https://github.com/Arakula/f9dasm) | Motorola 6800/6809/6309 family disassembler. `any`. |
| `ghidra` | [ghidra-sre.org](https://ghidra-sre.org/) (NSA) | Software reverse-engineering framework. `amd64`. |
| `halfempty` | [googleprojectzero/halfempty](https://github.com/googleprojectzero/halfempty) | Parallel, language-agnostic file-bisection test-case minimizer. `any`. |
| `libvgm` | [ValleyBell/libvgm](https://github.com/ValleyBell/libvgm) | Chip-register VGM playback library + player utilities. `any`. |
| `vgmstream` | [vgmstream.org](https://vgmstream.org/) | Video-game audio decoder (CLI). `any`. |
| `ppsspp` | [ppsspp.org](https://www.ppsspp.org) | Fast, portable PSP emulator. `any`. |
| `snes9x-gtk` | [snes9x.com](https://www.snes9x.com) | Portable Super Nintendo (SNES) emulator. `any`. |
| `task` | [taskfile.dev](https://taskfile.dev) | Task runner / build tool written in Go. `amd64`. |
| `ruff` | [astral.sh/ruff](https://docs.astral.sh/ruff) | Extremely fast Python linter and code formatter. `amd64`. |
| `python3-glfw` | [FlorianRhiem/pyGLFW](https://github.com/FlorianRhiem/pyGLFW) | Python ctypes bindings for GLFW3. `all`. |
| `python3-librosa` | [librosa.org](https://librosa.org) | Audio and music analysis library for Python. `all`. |
| `python3-mss` | [BoboTiG/python-mss](https://github.com/BoboTiG/python-mss) | Ultra-fast cross-platform screenshot library for Python. `all`. |
| `python3-pydub` | [pydub.com](http://pydub.com) | High-level audio manipulation library for Python. `all`. |
| `python3-inators` | [renatahodovan/inators](https://github.com/renatahodovan/inators) | Utility helpers for picire (test-case reduction). `all`. |
| `python3-picire` | [renatahodovan/picire](https://github.com/renatahodovan/picire) | Parallel Delta Debugging test-case reducer (CLI). `all`. |
| `blender-asset-finder` | [wbniv/WorldFoundry](https://github.com/wbniv/WorldFoundry) | Blender add-on: search/download 3D assets from online repos. `all`. |
| `blender-asset-finder-cli` | [wbniv/WorldFoundry](https://github.com/wbniv/WorldFoundry) | CLI companion to the `blender-asset-finder` add-on. `all`. |

See [`LICENSES-VENDORED.md`](LICENSES-VENDORED.md) for the running attribution list. Use the [`/package`](https://github.com/anthropics/claude-code) Claude Code skill to add new ones — it generates the Debian source tree via `dh_make` and wires it into this repo.

One upstream we *don't* repackage:

- [`xa65`](https://www.floodgap.com/retrotech/xa/) — Ubuntu 26.04 universe ships it as `xa65 2.4.1-0.1build1`; foundry-retro-tools `Depends:` it from there.

## Repo layout

```
foundry-apt/
  packages/<name>/              every package, metapackage or vendored upstream
    debian/                     Debian source-package format (canonical)
      control                   Source: + Package: stanzas
      changelog                 authoritative version (read via dpkg-parsechangelog)
      rules                     one-line "%: dh $@"
      source/format             3.0 (native) for metapackages
                                3.0 (quilt)  for vendored upstreams
      copyright                 DEP-5 format
      [patches/series]          optional quilt patches for vendored upstreams
      [watch]                   optional uscan tracker for vendored upstreams
    build.sh                    only for vendored upstreams: fetches sha256-pinned
                                tarball, overlays debian/, runs dpkg-buildpackage
  aptly/
    aptly.conf                  Local aptly config (rootDir, architectures)
  scripts/
    build-all.sh                dispatch: build.sh wrapper, else dpkg-buildpackage
    init-repo.sh                aptly repo create (idempotent)
    publish-local.sh            aptly publish repo → ./public/
    generate-index.sh           parse packages/*/debian/ → public/index.html
    sign.sh                     CI-side: import GPG key from secret, sign Release
  .github/workflows/
    test.yml                    PR: shellcheck + build all .debs + sanity check
    publish.yml                 tag push: build + sign + sync to Cloudflare R2
  dist/                         build output (gitignored)
  public/                       published apt repo (gitignored)
  Taskfile.yml                  task wrappers around scripts/*
  docs/infra-setup.md           one-time setup: R2 bucket, GPG, DNS
```

## Local development

```bash
bash scripts/build-all.sh
bash scripts/init-repo.sh       # → ~/.aptly/foundry repo
bash scripts/publish-local.sh   # → ./public/ apt tree
apt-cache depends foundry-retro-tools
```

## Adding or upgrading a package

For **metapackages** (just a `Depends:` list — no upstream tarball):

1. Update `packages/<name>/debian/control` (`Depends:`, `Recommends:`, etc.).
2. Add a top entry to `packages/<name>/debian/changelog` — use `dch -v <new-version> -D resolute "what changed"` from inside the package dir, or hand-edit:

   ```
   <name> (<new-version>) resolute; urgency=medium

     * What changed.

    -- Maintainer <email>  <rfc-2822 date>
   ```
3. `task build` to verify.

For **vendored upstream packages** (e.g. `f9dasm`):

1. Bump `UPSTREAM_VERSION` + `SHA256` at the top of `packages/<name>/build.sh`. Re-pin with `curl -fsSL <upstream-url> | sha256sum`.
2. Add a top entry to `packages/<name>/debian/changelog`.
3. `task build`.

For a **new vendored upstream**, use the [`/package`](#) Claude Code skill — it does the universe-check, `dh_make` scaffolding, and template substitution in one go.

See [`CONTRIBUTING.md`](CONTRIBUTING.md) for full instructions.

To release: `task bump` (auto-tags the next patch version) — the [publish workflow](.github/workflows/publish.yml) builds, signs, and syncs to R2 automatically.

## Hosting

- **APT repo:** [Cloudflare R2](https://www.cloudflare.com/developer-platform/products/r2/) (10 GB free tier, zero egress) → `apt.foundrylinux.org`
- **Signing key:** GPG key in GitHub Actions secrets (CI use), backed up to a private `foundry-linux-secrets` R2 bucket for disaster recovery. No AWS account required.

Detailed setup in [`docs/infra-setup.md`](docs/infra-setup.md).

## License

GPL-2.
