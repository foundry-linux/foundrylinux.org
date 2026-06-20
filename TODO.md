# TODO — foundrylinux.org

See [`docs/plans/`](docs/plans/) for written plans behind each item, and
[`docs/investigations/2026-05-16-foundry-distro-proposal.md`](docs/investigations/2026-05-16-foundry-distro-proposal.md) for the full roadmap.

## Open

### apt-repo resilience — `task` vendored + source health-check

- [ ] **Phase 2 (gated on the foundry-apt publish):** flip `task` consumers off Cloudsmith — delete `foundry-iso/config/archives/cloudsmith-task.list.chroot` + its key fetch/copy in `build-iso.sh`; drop the `setup.deb.sh | bash` lines from `foundry-devbox/Dockerfile`, `foundry-setup/install-task.sh`, `site/setup.sh`; wire `task check-apt-repos` into the ISO/release preflight. Must follow a foundry-apt publish that serves `task`. [plan](docs/plans/2026-05-31-vendor-task-and-repo-health.md).
### Phase 3 — Foundry Linux ISO

- [ ] **Build `foundry-atelier-1.0-amd64.iso`** — `task iso-build EDITION=atelier`; verify ghidra + full atelier package set present; sign + upload to R2. Target ~10 GB. See [plan](docs/plans/2026-05-22-phase-3-foundry-iso.md).
- [verify] **Guarantee a full KDE config stack** — build-time assertion in `foundry-iso/scripts/build-iso.sh` (after `lb chroot`) checks the KDE config QML modules (`org.kde.kcmutils`, `org.kde.kquickcontrols`, `QtQuick/Dialogs`) are present in the chroot, so a future strip-list edit can't silently break plasmoid config dialogs. Guard comments added to hook 0020 + `strip.list.chroot.purge`; omit/ship rationale in [docs/investigations/2026-05-30-kde-app-kit.md](docs/investigations/2026-05-30-kde-app-kit.md). **Code landed 2026-06-04; verify steps 1–4 PENDING** (require a full `task iso-build`). See [plan](docs/plans/2026-05-30-full-kde-experience.md). _(Surfaced while testing the claude-usage KDE plasmoid live; that empty-config-dialog bug was a claude-usage bug, not a Foundry gap.)_
- [ ] **[at v1.0.0] Migrate ISO hosting to Internet Archive** — ~~provision IA S3 keys~~ ✅ done 2026-06-14 (`scripts/bootstrap-ia.sh` → `IA_S3_ACCESS_KEY`/`IA_S3_SECRET_KEY` in foundry-linux-secrets R2 + GH secrets). Remaining: add `scripts/upload-iso-ia.sh` (rclone → `s3.us.archive.org`), add Cloudflare Worker redirecting `iso.foundrylinux.org/*` → archive.org download URLs, update `publish.yml`. R2 stays for apt repos. See [investigation](docs/investigations/2026-05-22-iso-hosting.md).

- [ ] **Kiosk mode (gamescope + wf-launcher)** — *deferred* from the [phase-3 ISO plan](docs/plans/2026-05-22-phase-3-foundry-iso.md); no plan written yet. A locked-down session that boots straight into a game launcher. Future phase, not blocking v1.


### Deferred follow-ups (surfaced by the 2026-06-04 plan sweep)

Sub-tasks that completed plans explicitly punted/deferred and that weren't tracked anywhere until now.

- [ ] **foundry-kde-theme — remaining theming layers** — Plasma Style SVG set (distinctive panel/widget shapes — partial: forge-palette colors done, Breeze-inherit done; custom SVGs pending), Aurorae window decoration, Kvantum Qt-app style. See [plan](docs/plans/2026-05-24-foundry-kde-theme.md).
- [ ] **Phase 2 devbox — per-game tooling** — `wf-game-create` + per-game Distrobox scaffolding + a `:26.04-maintainer` image tier; deferred from the [devbox execution plan](docs/plans/2026-05-21-phase-2-devbox-execution.md). Companion plan not yet written.
- [ ] **Steam/Sniper release containers** — ship WF games through Steam's [Sniper runtime](https://gitlab.steamos.cloud/steamrt/steam-runtime-tools) (a reproducible release/runtime container), as floated in the original [2026-05-16 proposal](docs/investigations/2026-05-16-foundry-linux-distro-proposal.md). Still wanted — was never tracked anywhere until now; surfaced when the proposal's banner mis-labelled it "dropped" (2026-06-04). Future phase, not blocking v1; no plan written yet.
### Packaging — new upstreams

- [ ] **Package `bsnes-jg`** — the jgemu-framework bsnes fork (GPL-3.0), an accuracy-focused SNES emulator for `foundry-emulators-consoles-heavy`. Check Ubuntu 26.04 universe first per the always-check-universe rule; use the `/package` skill. (Companion `mesen2` shipped 2026-06-14 — see Done + [plan](docs/plans/2026-06-14-package-mesen2.md).) No plan yet.
- [ ] **Test-case reduction toolchain — Tier 3 (shrinkray)** — Tiers 1+2 ✅ done 2026-06-20: `cvise`+`delta` → `foundry-retro-tools` (anvil, published v1.5.30); `creduce` → `foundry-atelier` (held, not published); vendored `halfempty` (Apache-2.0), `python3-inators`+`python3-picire` (BSD-3) → retro-tools 1.0.15. **Remaining:** `shrinkray` via `pipx` in `install-foundry-retro-tools.sh` (the `textual>=8` + `textual-plotext` cascade blocks a clean `.deb`; revisit a `.deb` once `textual≥8` lands in universe). [plan](docs/plans/2026-06-20-add-reducers-to-retro-tools.md).

### Debian ITP

Check [wnpp.debian.org](https://bugs.debian.org/cgi-bin/pkgreport.cgi?pkg=wnpp) for existing RFP/ITP before filing each. New vendored packages get their own entry here via `/package` skill Step 6.

- [ ] **ITP: `asar-snes-assembler`** — SNES 65816 cross-assembler (LGPL-3.0+)
- [ ] **ITP: `blender-asset-finder`** — Blender asset search add-on (GPL-2.0+)
- [ ] **ITP: `blender-asset-finder-cli`** — CLI for Blender asset finder (GPL-2.0+)
- [ ] **ITP: `bsnes-jg`** — cycle-accurate SNES/Super Famicom emulator (GPL-3.0+)
- [ ] **ITP: `drmon`** — terminal system monitor (GPL-2.0)
- [ ] **ITP: `f9dasm`** — MC6800/6809/68HC12 disassembler (GPL-2.0+)
- [ ] **ITP: `ghidra`** — NSA reverse-engineering framework (Apache-2.0)
- [ ] **ITP: `halfempty`** — parallel file-bisection test-case minimizer (Apache-2.0)
- [ ] **ITP: `ldtk`** — level design toolkit (MIT)
- [ ] **ITP: `libvgm`** — VGM chiptune audio library (GPL-2.0+)
- [ ] **ITP: `m8te`** — SNES 8bpp tile/map editor, Mono runtime (MIT)
- [ ] **ITP: `mesen2`** — multi-system retro emulator, NES/SNES/GB/GBA/PCE/SMS/WS (GPL-3.0)
- [ ] **ITP: `ppsspp`** — PSP emulator (GPL-2.0+)
- [ ] **ITP: `pvsneslib`** — SNES homebrew SDK (Zlib)
- [ ] **ITP: `python3-glfw`** — Python GLFW bindings (MIT)
- [ ] **ITP: `python3-inators`** — utility helpers for picire (BSD-3-Clause)
- [ ] **ITP: `python3-librosa`** — Python audio analysis library (ISC)
- [ ] **ITP: `python3-mss`** — Python multi-platform screenshot library (MIT)
- [ ] **ITP: `python3-picire`** — parallel Delta Debugging reducer (BSD-3-Clause)
- [ ] **ITP: `python3-pydub`** — Python audio manipulation library (MIT)
- [ ] **ITP: `ruff`** — Python linter/formatter written in Rust (MIT)
- [ ] **ITP: `snes9x-gtk`** — SNES emulator ⚠️ Snes9x non-commercial license; likely non-DFSG — research before filing
- [ ] **ITP: `task`** — task runner / Makefile alternative (MIT)
- [ ] **ITP: `tilemap-studio`** — retro tile/map editor (LGPL-3.0)
- [ ] **ITP: `vgmstream`** — VGM stream decoder (ISC)
- [ ] **ITP: `wla-dx`** — multi-CPU cross assembler (GPL-2.0+)

### Housekeeping
- [ ] **VM distribution artifacts (qcow2 / OVA)** — build VirtualBox/VMware/QEMU images alongside ISOs (homepage VM rows are intentional, not placeholders); folds in GRUB-theme activation + OVMF custom-logo work (VM-only). **Start after Internet Archive migration.** See [phase-3 ISO plan](docs/plans/2026-05-22-phase-3-foundry-iso.md), [boot-branding](docs/plans/2026-05-24-boot-branding.md).
- [ ] **Activate `repository_dispatch` from worldfoundry.org** — create a fine-grained PAT scoped to `foundry-linux/foundrylinux.org` (Contents: Read and write), then `gh secret set FOUNDRYLINUX_DISPATCH_PAT --repo wbniv/worldfoundry.org --body <PAT>`; activates site rebuild on the next `apt-v*` tag push. See [plan §6](docs/plans/2026-05-21-packages-page.md).
- [ ] **Restore foundry-iso CI triggers after 1.0 ships** — re-add `push: tags: ['v*']` + monthly cron to `foundry-iso/.github/workflows/publish.yml`; evaluate self-hosted runner for atelier vs GH-hosted for anvil. Disabled 2026-05-22 to conserve GH Actions minutes.
- [ ] **Repology badges: confirm opt-outs + extend to apt.worldfoundry.org** — foundry-apt is fully covered (`task audit-badges` → 22 badge / 5 `none` / 0 missing; Badge column now in the audit inventory). (1) Review the remaining 5 `none` opt-outs (`blender-asset-finder{,-cli}`, `drmon`, `m8te`, `foundry-welcome`) — set a real `X-Repology-Project` for any that ARE tracked on Repology (as done for asar-snes-assembler via `task set-badge`). (2) The audit flagged 10 WorldFoundry CLIs in `apt.worldfoundry.org` (`cdpack`, `iffcomp`, `iffdump`, `levcomp`, `lvldump`, `oaddump`, `oas2oad`, `prep`, `textile`, `worldfoundry-blender-editor-exporter`) lacking the field — backfill them (`none`; they're WF-authored, not on Repology) and wire `check-repology-badges.sh` into that repo. [plan](docs/plans/2026-06-21-repology-badge-audit-tooling.md).

## Watch

Items to check periodically and act on only if something changes.

### go-task Cloudsmith repo

- Already rotated layout once (`any-distro`→per-distro); 404s read as intermittent (CloudFront caches the error). Run `task check-apt-repos` before any ISO/devbox build or release. If `ubuntu resolute` breaks or lags upstream `task`, pull the trigger on Phase 2 flip (own `task` fully from foundry-apt). Current upstream: 3.51.1 (vendored); `ubuntu/resolute` served 3.51.1 as of 2026-05-31.

### PVSnesLib — bundled WLA DX toolchain ([plan](docs/plans/2026-06-12-package-pvsneslib.md))

- Bundles its own pinned WLA DX (`wla-65816`/`wla-spc700`/`wlalink`) + `816-tcc` under `devkitsnes/bin/` — intentional (library `.obj`s are assembled against the pinned WLA). No `/usr/bin` conflict with the standalone `wla-dx` package. If upstream ships a source-buildable toolchain cleanly, revisit building from source for arch portability (currently amd64-only binary zip).

## Parked

Items intentionally on hold — revisit if priorities shift, unpark to `## Open` when ready.

### tUME map editor port ([investigation](docs/investigations/2026-06-10-tume-map-editor-port-and-successors.md))

- Recovered tUME source vendored at `vendor/tume/` (MPL-1.1, from Wayback) — possible future SDL/Qt port; not packaged.

### wla-dx — Amiga Hunk output ([plan](docs/plans/2026-06-13-wla-dx-amiga-hunk-output.md))

- **Amiga Hunk executable output for wlalink** — feasibility study done against wla-dx v10.6 source: add `-t AMIGAHUNK` mode (`OUTPUT_TYPE_AMIGA_HUNK`, mirrors the C64-CRT path). Minimal viable ≈1 day; multi-hunk reloc is the hard follow-up. Revisit if Amiga-target support is wanted.

## Done

- 2026-06-14 — [package-mesen2] packaged Mesen 2.1.1 as `mesen2` (GPL-3.0, NES/SNES/GB/GBA/PCE/SMS/WS) — pre-built .NET-AoT single-file repack (no-strip; bundle appended to ELF), SDL2+libicu78+X11 deps declared, lintian-clean, xvfb headless launch verified; wired into `foundry-emulators-consoles-heavy` 1.0.4 (→ atelier). See [plan](docs/plans/2026-06-14-package-mesen2.md).
- 2026-06-14 — [ia-keys] Provisioned IA S3 keys via `scripts/bootstrap-ia.sh` (liveness-checks against s3.us.archive.org, R2 backup + GH secrets); fixed `foundry-secrets`→`foundry-linux-secrets` bucket bug in backup-secret.sh + bootstrap-r2.sh. Prereq for the v1.0.0 IA migration.
- 2026-06-13 — [python-gamedev-extras-weight] won't do — keeping `foundry-python-gamedev-extras` (554 MiB: `libvtk9.5` 276 MiB + numba/llvmlite + scipy + librosa) in anvil; 4 GB target dropped, 8 GB is the floor.
- 2026-06-13 — [phase-3-iso-anvil] live-build Kubuntu 26.04 ISO pipeline done; anvil-1.0 builds clean at ~4.2 GB; autologin fixed (casper bake + sddm.conf); Calamares + branding wired. See [plan](docs/plans/2026-05-22-phase-3-foundry-iso.md).
- 2026-06-13 — [anvil-drop-games] stale pre-split 15 GB anvil-0.9.30 superseded; current builds shed ~9.9 GiB of games (moved to atelier in foundry-anvil 1.0.4).
- 2026-06-13 — [monorepo-public] foundry-linux/foundrylinux.org flipped to public (required for GH Actions CI to run on PRs). `gh repo view --json isPrivate` confirms `false`.
- 2026-06-12 — [pvs-publish] PVSnesLib 4.5.0 live on apt.foundrylinux.org (pvsneslib-core amd64, pvsneslib-examples all, pvsneslib meta); wired into foundry-retro-tools 1.0.12 + foundry-game-frameworks 1.0.3. See [plan](docs/plans/2026-06-12-package-pvsneslib.md).
- 2026-06-11 — [wla-dx-abfs-pdf] Vendored abfs.pdf (Amiga BFS spec, 2005) to wla-dx/docs/abfs.pdf; posted Wayback Machine URL on vhelin/wla-dx#589. See [plan](docs/plans/2026-06-11-vendor-abfs-pdf-post-to-wla-dx-issue-589.md).
- 2026-06-11 — [add-64tass] 64tass (multi-pass 65816/65xx macro assembler) added to foundry-retro-tools 1.0.10 Depends; resolves from Ubuntu 26.04 universe (64tass 1.60.3243-1).
- 2026-06-04 — [local-deb-staging] task iso-stage-deb PACKAGE=<name> copies latest .deb from foundry-apt/dist/ to foundry-iso/local-debs/, replacing old version.
- 2026-05-31 — [vendor-task] task 3.51.1-1foundry1 vendored into foundry-apt (Cloudsmith layout-rotation fix); scripts/check-apt-repos.sh + task check-apt-repos wired. See [plan](docs/plans/2026-05-31-vendor-task-and-repo-health.md).
- 2026-05-29 — [packages-page-ci] packages-page CI triggers wired: workflow_run on foundry-apt publish + nightly cron + repository_dispatch both ends. Will to activate: create fine-grained PAT (foundry-linux/foundrylinux.org, Contents R/W) and gh secret set in wbniv/worldfoundry.org. See [plan §6](docs/plans/2026-05-21-packages-page.md).
- 2026-05-18 — [package-skill] /package Claude Code skill created: dh_make + debhelper + dpkg-buildpackage; iteratively refined through f9dasm, libvgm, vgmstream packaging runs. See [plan](docs/plans/2026-05-18-package-skill.md).
- 2026-05-18 — [retire-xa65] xa65 retired: Ubuntu 26.04 universe ships xa65 2.4.1-0.1build1; packages/xa65/ deleted; foundry-retro-tools Depends: xa65 resolves from universe. See [plan](docs/plans/2026-05-18-retire-xa65.md).
- 2026-06-13 — [audit-apt-repos-snapshot] `.github/workflows/audit-apt-repos-snapshot.yml`: daily 04:00 UTC cron + `workflow_dispatch`; commits dated `docs/investigations/` snapshots to main; skips if unchanged. See [plan](docs/plans/2026-06-13-audit-apt-repos-scheduled-daily-snapshots.md).
- 2026-06-13 — [iso-prune-local] `build-iso.sh` now prunes old `dist/` ISOs + sidecars (.sha256/.asc/.torrent) for the edition before each build; build logs preserved. Fixes 0.9.104 xorriso disk-full failure.
- 2026-06-13 — [jre-both-in-core] foundry-core 1.0.6: both openjdk-17-jre-headless + openjdk-21-jre-headless in Depends; all tiers (anvil, sprite, atelier, devbox) get both LTS runtimes. foundry-atelier 0.9.5: dropped now-redundant openjdk-21 dep. Published foundry-apt v1.5.26; ISO rebuild in progress.
- 2026-06-13 — [package-asar-snes] packaged asar v1.91 as `asar-snes` (GPL-3.0, SNES 65816/SPC700/SuperFX cross-assembler); named to coexist with @electron/asar npm tool; wired into `foundry-retro-tools`.
- 2026-06-13 — [package-tilemap-studio] packaged Tilemap Studio 4.0.1 (LGPL-3, retro tile/map editor) — hybrid Wayland/X11+GL, FLTK 1.4.5 static-linked; Perl-based patching (2 patches, both already in upstream master); wired into `foundry-retro-tools`. Published `foundry-apt` v1.5.25. See [plan](docs/plans/2026-06-10-package-tilemap-studio.md).
- 2026-06-13 — [drmon-cppdap-fix] drmon cppdap/JsonCpp CMake fix landed upstream in drdevtools `d28c9d8`; upstream now at `6c51085` (multi-window, per-terminal sizes, keyword-shim cleanup).
- 2026-06-11 — [package-wla-dx] packaged wla-dx 10.6-1foundry1 (16-CPU assembler suite: 6502/65816/SPC700/Z80/68000/…); lintian-clean; wired into foundry-retro-tools 1.0.11. See [plan](docs/plans/2026-06-11-package-wla-dx.md).
- 2026-06-11 — [foundry-kde-theme-consolidation] foundry-kde-theme 1.0.4/1.0.5: LAF built out (defaults+layout+splash fixed), Plasma Style (forge palette), panel favorites (Blender in, Kate out), app-menu Favorites (KAStats), dpkg-divert for avatar+kicker, all theming hooks/includes removed from foundry-iso. Pixel-verified on 0.9.110. See [plan](docs/plans/2026-06-10-foundry-laf-and-plasma-style.md).
- 2026-06-11 — [iso-local-debs-verify] step 4 PASS on 0.9.110 install: `apt list --upgradable` shows no foundry packages; foundry-core installed from repo (500), foundry-kde-theme staged+installed from local (100) but Installed==Candidate → no Discover noise. See [plan](docs/plans/2026-06-10-iso-local-debs-publish-aware-staging.md).
- 2026-06-11 — [live-ssh-removed] removed `1200-live-ssh.hook.chroot` (baked `root:foundry` + sshd into live squashfs); replaced with `0035-install-ssh-server.hook.chroot` (openssh on installed system only, ssh.socket enabled by Calamares services-systemd). No credential baked anywhere (live or installed). Commit `b8c7377`.
- 2026-06-10 — [package-m8te] packaged M8TE 1.5 (SNES 8bpp tile/tilemap/palette editor, MIT) — first Mono/.NET WinForms package: repacks the upstream pre-built CIL `M8TE.exe` (Arch: all) behind a `/usr/bin/m8te` mono wrapper; Depends mono-runtime/mono-libraries/libgdiplus. Built + lintian-clean + headless-launch verified. Wired into both `foundry-retro-tools` (1.0.8 → lands in anvil) and `foundry-pixel-art` (1.0.1). Pending publish via `task bump`. See [plan](docs/plans/2026-06-10-package-m8te.md).
- 2026-06-05 — [usb-sizing] anvil 0.9.53 at 3.837 GiB: ghidra → atelier ✅, snapd purged ✅, python3-opencv purged ✅, opencv C++ libs kept (Blender/KDE need them). 4 GB target not reachable without removing intentional content (ibus-data, fonts-noto-cjk, python3-llvmlite, libicu-dev); **minimum stick is 8 GB**. See [plan](docs/plans/2026-06-04-usb-sized-iso-editions.md).
- 2026-06-05 — [snapd-ships-bug] snapd confirmed absent from 0.9.53 via `dpkg --purge` in hook 0025 (apt-get purge silently failed — apt index still showed snap-transitional PreDepending snapd after dpkg replaced firefox). See [plan](docs/plans/2026-06-04-fix-snapd-ships-despite-strip-list.md).
- 2026-06-04 — [create-foundry-usb] `foundry-create-usb.sh` shipped in `foundry-desktop` 1.0.2: GUI (isoimagewriter) + dd mode (lsblk + mount-guard + YES confirm + conv=fsync); desktop entry + `/etc/skel/Desktop` icon; `task iso-write DEVICE=/dev/sdX` for dev.
- 2026-06-04 — [install-sh-curl-gap] `foundry-setup/README.md` pointed to nonexistent `install.sh` curl URL; canonical path is `setup.sh + apt install foundry-anvil`; README rewritten, stale URLs removed from `install.sh`.
- 2026-06-04 — [foundry-setup-ci-gap] `foundry-setup/.github/workflows/test.yml` ran nowhere (GitHub only runs root `.github/workflows/`); moved it to `.github/workflows/foundry-setup-test.yml` — no child repo / no `setup-sync` (foundry-setup has no artifact to publish). See [plan](docs/plans/2026-06-04-fix-foundry-setup-ci-gap.md).
- 2026-06-04 — [plan-sweep] audited all 70 plans; backfilled this Done log, tracked unimplemented/deferred tasks, deleted 5 superseded draft plans. See [full-kde plan](docs/plans/2026-05-30-full-kde-experience.md).
- 2026-05-31 — [xorriso-efi-warning] documented the `/EFI/BOOT` xorriso warning as expected/benign (WONTFIX); no functional fix needed. See [plan](docs/plans/2026-05-31-xorriso-efi-boot-warning-benign.md).
- 2026-05-30 — [python-gamedev-pkgs] packaged python3-glfw, python3-pydub, ruff, python3-librosa, python3-mss; shipped in foundry-apt v1.5.7. See [plan](docs/plans/2026-05-30-package-glfw-pydub-ruff-librosa-mss.md).
- 2026-05-29 — [new-web-apt-repo-skill-v1.1] packages.json + feed.xml + RSS autodiscovery link promoted from docs-only to gen-index.py built-ins; v1.0.0 → v1.1.0; synced to marketplace plugin.
- 2026-05-29 — [cve-live-count-badge] ubuntu.com/security/cves.json fetch at build time; `cve_count` in packages-data.json; `cve-badge--active`/`--clear` spans in packages.jsx; `SKIP_CVE=1` escape hatch.
- 2026-05-29 — [apt-index-json-rss] `packages.json` + `feed.xml` (RSS 2.0, 32 items) generated by `generate-index.sh`; RSS auto-discovery `<link>` in `<head>`; OG tags confirmed already live.
- 2026-05-29 — [packages-page-polish] Repology badges + changelog tooltip on /packages (foundry-origin pkg rows + VendoredStandalones); edition dep expansion `<details>`; CVE shield links (Ubuntu-origin rows + apt index). Data pipeline: build-packages-data.js reads meta/*.json.
- 2026-05-29 — [apt-index-two-column] apt index reflowed to a 2-col table (name+version left, description right); follow-up reflow fixes landed. See [plan](docs/plans/2026-05-29-apt-index-two-column-table.md).
- 2026-05-29 — [site-opengraph] OpenGraph + Twitter Card meta tags on all site pages (index + packages). See [plan](docs/plans/2026-05-29-opengraph-meta-tags.md).
- 2026-05-28 — [apt-package-meta] per-package `public/meta/{name}.json` + `<details>` long-desc/dep-chips/size in apt index; `generate-meta.sh`; Taskfile + CI wired. See [plan](docs/plans/2026-05-28-apt-package-meta.md).
- 2026-05-28 — [apt-index-sort-filter] client-side filter + sortable cols + copy-to-clipboard ported from worldfoundry.org to `foundry-apt/scripts/generate-index.sh` + `gen/static/index.js`; skill updated. See [apt listing landscape](docs/investigations/2026-05-28-apt-listing-landscape.md).
- 2026-05-28 — [foundry-core-desktop-split] split `foundry-core` (tools, no KDE) / `foundry-desktop`; `foundry-anvil` = core+desktop; devbox switched to core. See [plan](docs/plans/2026-05-28-foundry-core-desktop-split.md).
- 2026-05-28 — [install-card-05-hero] site card 05 hero treatment; removed the PATH 04–05 detail block. See [plan](docs/plans/2026-05-28-install-card-05-hero-treatment.md).
- 2026-05-28 — [install-section-redesign] install section reworked to 5 ordered paths + dual-boot callout. See [plan](docs/plans/2026-05-28-install-section-redesign.md).
- 2026-05-26 — [foundry-welcome] QML/Kirigami welcome screen; show-once sentinel; XDG autostart; masks plasma-welcome; packaged as `foundry-welcome` 1.0.5 on apt.foundrylinux.org; integrated into ISO via local-debs. See [plan](docs/plans/2026-05-24-foundry-welcome.md).
- 2026-05-25 — [calamares-installer-fix] calamares-settings-foundry-linux 1.0.6 adds required style/window-layout keys; installer no longer crashes. See [plan](docs/plans/2026-05-25-calamares-installer-fix.md).
- 2026-05-25 — [fix-plasma-welcome-autoboot] purge plasma-welcome in hook 0020 (was kded6rc-suppressed); foundry-welcome replaces it. See [plan](docs/plans/2026-05-25-fix-plasma-welcome-autoboot.md).
- 2026-05-25 — [iso-tasks-edition-all] `iso-build/smoke/sign/upload` all support `EDITION=all` via loop expansion. See [plan](docs/plans/2026-05-25-iso-tasks-edition-all-support.md).
- 2026-05-24 — [autologin-fix] casper autologin root cause fixed (15autologin blank `Session=`); baked `/etc/sddm.conf` + sed in 16foundry-autologin; sshd debug hook. See [plan](docs/plans/2026-05-24-autologin-sshd-debug.md).
- 2026-05-24 — [boot-branding] Plymouth boot-splash theme hook (1050) + postinst set-default-theme; GRUB/OVMF logo deferred to VM phase. See [plan](docs/plans/2026-05-24-boot-branding.md).
- 2026-05-24 — [foundry-kde-theme] `foundry-kde-theme` 1.0.1: color scheme + wallpaper + applet layout; deeper layers deferred. See [plan](docs/plans/2026-05-24-foundry-kde-theme.md).
- 2026-05-24 — [kde-splash-lock] Foundry KDE splash + lock-screen wallpaper via look-and-feel package; shipped in ISO 0.9.30+. See [plan](docs/plans/2026-05-24-kde-splash-lock-screen-branding.md).
- 2026-05-24 — [stale-lock-cleanup] build-iso.sh removes stale `.lock`; confirmed bloat packages absent from final ISO. See [plan](docs/plans/2026-05-24-stale-lock-bloat-package-investigation.md).
- 2026-05-23 — [ppsspp+snes9x-gtk] Both packaged and wired into `foundry-emulators-consoles-heavy`; ppsspp v1.20.4 (git-clone build, 8 submodules), snes9x-gtk 1.63 (tarball, USE_SLANG=OFF). Lintian clean, smoke-tested.
- 2026-05-23 — [flesh-out-site] channels grid (4 install paths), #docs section, dead footer links wired/removed, VM rows marked coming-soon, Install sidebar copy expanded. Shipped v0.3.4.
- 2026-05-23 — [iso-versioning] `foundry-iso/VERSION` + `bump-version.sh` + `task iso-bump`; version stamped into volume label/filenames. See [plan](docs/plans/2026-05-23-iso-versioning.md).
- 2026-05-23 — [link-check-gate] `scripts/check-links.js` + `link-check` task wired into site-deploy CI as a publish gate. See [plan](docs/plans/2026-05-23-link-check-publish-gate.md).
- 2026-05-23 — [list-isos-vms-table] `iso-list` shows R2 + local ISOs/VMs in a columnar table. See [plan](docs/plans/2026-05-23-list-isos-vms-local-table.md).
- 2026-05-23 — [torrent-magnet] `create-torrents.sh` wired into sign/upload; .torrent + magnet buttons on homepage ISO downloads. See [plan](docs/plans/2026-05-23-torrent-magnet-homepage.md).
- 2026-05-23 — [wire-ppsspp-snes9x-iso] ppsspp + snes9x-gtk wired into `foundry-atelier`; atelier-publish task + iso-build fingerprinting. See [plan](docs/plans/2026-05-23-wire-ppsspp-snes9x-into-iso.md).
- 2026-05-22 — [worldfoundry-apt-phase-b] 9 WF CLIs + `worldfoundry-blender-editor-exporter` + 4 metapackages on `apt.worldfoundry.org`; `blender-asset-finder{,-cli}` on `apt.foundrylinux.org` (general Blender tools, not WF-specific); CLAUDE.md updated.
- 2026-05-22 — [worldfoundry-apt-phase-a] `worldfoundry.org/apt/` infra live: GPG-signed empty repo, gen-index site at apt.worldfoundry.org, `new-web-apt-repo` global skill. All 7 verification steps pass. See [plan](docs/plans/2026-05-18-worldfoundry-apt-repo.md).
- 2026-05-22 — [phase-2-devbox] `ghcr.io/foundry-linux/devbox:26.04` live; 16 new metapackages (anvil/sprite/atelier hierarchy + 6 tier-0 sub-meta + 7 opt-in) on apt.foundrylinux.org; 42/42 smoke-test pass; CI green at v0.0.5. See [plan](docs/plans/2026-05-21-phase-2-devbox-image.md).
- 2026-05-22 — [packages-page] `/packages` generated from live Packages.gz of both apt repos + Ubuntu resolute; home Kit grid (stale f9dasm/65ax/libvgm/vgmstream tool cards) replaced with auto-driven 6-card Forge; pure Node generator, ~3 s. CI triggers deferred to follow-up. See [plan](docs/plans/2026-05-21-packages-page.md).
- 2026-05-22 — [move-asset-packages] `blender-asset-finder` + `wf-asset` (→ `blender-asset-finder-cli`) moved from apt.worldfoundry.org to apt.foundrylinux.org; `worldfoundry-cli` Depends + description updated; live on both R2 repos. See [plan](docs/plans/2026-05-20-move-asset-packages.md).
- 2026-05-22 — [drop-linux-from-package-names] `foundry-linux-*` → `foundry-*` across packages, install scripts, and `foundry-linux-setup/` dir; landed in 433a07b, shipped via foundry-apt v0.0.41, live R2 index serves only new names. See [plan](docs/plans/2026-05-21-drop-linux-from-package-names.md).
- 2026-05-22 — [audit-apt-repos] `scripts/audit-apt-repos.sh` + notes.yaml + `audit-apt-repos` task; cross-checks both repos vs inventory. See [plan](docs/plans/2026-05-22-audit-apt-repos-script.md).
- 2026-05-22 — [branding-assets] Calamares/GRUB/Plymouth/SDDM branding assets generated into calamares-settings-foundry-linux/data/. See [plan](docs/plans/2026-05-22-generate-branding-assets.md).
- 2026-05-21 — [retro-tools-e2e-test] `test-retro-tools-e2e.sh` (docker-based) installs metapackage in fresh ubuntu:26.04 + verifies all 15 tools invoke; 15/15 pass. VM not needed for this.
- 2026-05-20 — [mobile-dev-metapackages] `foundry-android-development` + `foundry-ios-development` metapackages + iOS install script (pipx codemagic). See [plan](docs/plans/2026-05-20-mobile-development-metapackages.md).
- 2026-05-20 — [two-apt-repos-spec] CLAUDE.md + scripts updated for two permanent separate repos; `setup-worldfoundry-apt-source.sh` added. See [plan](docs/plans/2026-05-20-two-apt-repos-spec-update.md).
- 2026-05-20 — [vgmstream-libavcodec62] rebuilt ppsspp + vgmstream in ubuntu:26.04 container for ffmpeg 8.x (libavcodec62) sonames. See [plan](docs/plans/2026-05-20-vgmstream-libavcodec62-rebuild.md).
- 2026-05-18 — [canonical-debian-layout] migrated all metapackages from authored `DEBIAN/` to canonical `debian/` source format. See [plan](docs/plans/2026-05-18-canonical-debian-layout.md).
- 2026-05-18 — [post-upload-verify] replaced CI sleep with a 10-attempt post-upload SHA256 consistency check before declaring a publish done. See [plan](docs/plans/2026-05-18-post-upload-consistency-verification.md).
- 2026-05-18 — [lighthouse-ci] Lighthouse CI pipeline + `lighthouse-threshold.sh`; per-tag bundles archived in site-deploy. See [plan](docs/plans/2026-05-18-lighthouse-ci.md).
- 2026-05-18 — [worldfoundry-rename-cleanup] `worldfoundry-*` refs purged from Phase 0 install scripts, README, CONTRIBUTING, Taskfile, test harness; functional installs now use direct deps.
- 2026-05-18 — [apt-index-styling] `apt.foundrylinux.org` index now shares `foundrylinux.org/styles.css`; Space Grotesk + JetBrains Mono; ember orange accent; no duplicate CSS to maintain. Shipped as v0.0.29.
- 2026-05-18 — [worldfoundry-apt-cleanup] `worldfoundry-{android-dev,blender,dev,engine-build-deps}` removed from `foundry-apt/` (moved to `apt.worldfoundry.org`); smoke test scoped to `apt-cache show`; CI green at v0.0.30.
- 2026-05-18 — [site-launch] `foundrylinux.org` live on Cloudflare Pages via tag-push CI; Claude Design bundle imported; branding corrected; `import-claude-design` skill created. See [`docs/plans/2026-05-18-site-launch.md`](docs/plans/2026-05-18-site-launch.md).
- 2026-05-18 — [drop-babel-standalone] Pre-compile `site/*.jsx` → `.js` via esbuild; switch to prod React CDN builds; drop 2.9 MB Babel standalone; update import-claude-design skill. See [`docs/plans/2026-05-18-drop-babel-standalone.md`](docs/plans/2026-05-18-drop-babel-standalone.md).
- 2026-05-18 — [package-ghidra] 541 MB pre-built zip → lintian-clean `.deb` at `12.1-1foundry1`; ghidra + ghidra-headless in `/usr/bin`; retro-tools 1.0.5 now `Depends: ghidra`; sidecar stripped from Phase 0 install script.
- 2026-05-18 — [package-vgmstream] 852 KB lintian-clean `.deb` at `2083-1foundry1`; cmake forced via `--buildsystem=cmake` (legacy Makefile coexists); `vgmstream-cli` statically linked to libvgmstream; retro-tools 1.0.4 now `Depends: vgmstream`.
- 2026-05-18 — [phase-0-foundry-apt-source] `setup-foundry-apt-source.sh` wired into `install.sh`; per-meta scripts collapsed to `apt install <metapackage>`; f9dasm + libvgm sidecars dropped.
- 2026-05-18 — [package-libvgm] 678 KB lintian-clean `.deb` at `0.1+git20260406.d115188-1foundry1`; STATIC libs (upstream pre-stable, no SOVERSION); `/usr/bin/player` renamed to `vgm-player`; retro-tools 1.0.3 now `Depends: libvgm`.
- 2026-05-18 — [live-test-manpage-assert] `test/run-test.sh` drops `dpkg.cfg.d/excludes`, mandoc-lints every `/usr/bin/<name>.1.gz`; caught real `PP-after-SH` WARNING in f9dasm 1foundry2 → bumped to 1foundry3.
- 2026-05-18 — [package-f9dasm] first `/package` run; 50 KB debhelper-built `.deb` (4.5× smaller than xa65's hand-roll), `Depends: libc6 (>= 2.38)` resolved, retro-tools 1.0.2 now `Depends: f9dasm`.
- 2026-05-18 — [version-deb-links] version numbers in apt index now link to `pool/main/...` `.deb`s; arch-specific code path ready for future arch-split packages.
- 2026-05-18 — [foundry-apt-live-install-tests] `test/run-test.sh` installs each published metapackage in a fresh Ubuntu 26.04 container, 5/5 pass.
- 2026-05-18 — [bootstrap-credential-cache] `BOOTSTRAP_CACHE=/tmp/foundry-bootstrap.env`; step 7.5 URL-rewrite rule replaces the free-plan-disallowed redirect phase.
- 2026-05-17 — [per-metapackage-install-scripts] Phase 0 monolith split into per-metapackage scripts mirroring `foundry-apt/packages/` 1:1; orchestrator + role dispatch.
- 2026-05-17 — [upgrade-github-actions-node24] `actions/checkout@v6` + `actions/upload-artifact@v7`; `FORCE_JAVASCRIPT_ACTIONS_TO_NODE24` removed; shipped as v0.0.8.
- 2026-05-17 — [foundry-phase1-bootstrap] `apt.foundrylinux.org` live on Cloudflare R2; GPG signing key in CI; publish-on-tag workflow operational.
- 2026-05-18 — [repo: monorepo home] monorepo pushed to `foundry-linux/foundrylinux.org` (private); `task sync` mirrors `foundry-apt/` subdir to `foundry-linux/foundry-apt` as before.
