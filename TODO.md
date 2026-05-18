# TODO — foundrylinux.org

See [`docs/plans/`](docs/plans/) for written plans behind each item, and
[`docs/investigations/2026-05-16-foundry-linux-distro-proposal.md`](docs/investigations/2026-05-16-foundry-linux-distro-proposal.md) for the full roadmap.

## Open

### Phase 1 — package the source-built retro tools

Phase 0's `install-foundry-linux-retro-tools.sh` source-built five tools into `~/opt/`. xa65 (Universe), f9dasm, libvgm, and vgmstream are now resolved via apt. Only ghidra remains as an open item. Use the `/package` skill — each new package needs `packages/<name>/debian/` + `build.sh`.

**Rule, learned the hard way (2026-05-18):** *before* packaging any source-built tool, run `apt-cache policy <pkg>` on a fresh `ubuntu:26.04`. If Ubuntu universe ships it, just add it to the Phase 0 apt-install list and the metapackage `Depends:` — don't duplicate.

- [x] ~~**xa65**~~ — retired. Ubuntu 26.04 universe ships `xa65 2.4.1-0.1build1`. `packages/xa65/` deleted; Phase 0 retro-tools script apt-installs xa65; `foundry-linux-retro-tools` `Depends: xa65` resolves to universe. See [`docs/plans/2026-05-18-retire-xa65.md`](docs/plans/2026-05-18-retire-xa65.md).
- [x] **`/package` skill** ([plan](docs/plans/2026-05-18-package-skill.md)) — reusable Claude Code skill using `dh_make` + `debhelper` + `dpkg-buildpackage`. Iteratively refined through f9dasm, libvgm, and vgmstream packaging runs.

### Phase 2 — Distrobox image

- [ ] Build `ghcr.io/foundry-linux/devbox:26.04` — Containerfile (Kubuntu/Ubuntu 26.04 base) + `apt.foundrylinux.org` configured + `foundry-linux-dev` preinstalled. GHCR workflow for tag-driven publish. Per proposal §"Channel 3 — OCI/container image".

### Phase 3 — Foundry Linux ISO

- [ ] Kubuntu 26.04-based ISO via `livecd-rootfs`. Calamares installer with Foundry Linux branding. Per proposal §"Channel 4 — bootable ISO".

### Housekeeping

- [ ] **Worldfoundry → foundry-linux metapackage rename.** `packages/worldfoundry-{android-dev,blender,dev,engine-build-deps}/` are legacy names. The distro is "Foundry Linux"; consider renaming the metapackages and/or shipping `foundry-linux-*` as aliases that `Depends:` on the WF ones.
- [ ] **Fresh-VM retro-tools end-to-end test.** Run `install-foundry-linux-retro-tools.sh` (not the metapackage) on a clean Ubuntu 26.04 VM and confirm all source-builds succeed + binaries appear at expected paths.
- [ ] **Flip monorepo to public** once content is ready: `gh repo edit foundry-linux/foundrylinux.org --visibility public`.

## Done

- 2026-05-18 — [package-ghidra] 541 MB pre-built zip → lintian-clean `.deb` at `12.1-1foundry1`; ghidra + ghidra-headless in `/usr/bin`; retro-tools 1.0.5 now `Depends: ghidra`; sidecar stripped from Phase 0 install script.
- 2026-05-18 — [package-vgmstream] 852 KB lintian-clean `.deb` at `2083-1foundry1`; cmake forced via `--buildsystem=cmake` (legacy Makefile coexists); `vgmstream-cli` statically linked to libvgmstream; retro-tools 1.0.4 now `Depends: vgmstream`.
- 2026-05-18 — [phase-0-foundry-apt-source] `setup-foundry-apt-source.sh` wired into `install.sh`; per-meta scripts collapsed to `apt install <metapackage>`; f9dasm + libvgm sidecars dropped.
- 2026-05-18 — [package-libvgm] 678 KB lintian-clean `.deb` at `0.1+git20260406.d115188-1foundry1`; STATIC libs (upstream pre-stable, no SOVERSION); `/usr/bin/player` renamed to `vgm-player`; retro-tools 1.0.3 now `Depends: libvgm`.
- 2026-05-18 — [live-test-manpage-assert] `test/run-test.sh` drops `dpkg.cfg.d/excludes`, mandoc-lints every `/usr/bin/<name>.1.gz`; caught real `PP-after-SH` WARNING in f9dasm 1foundry2 → bumped to 1foundry3.
- 2026-05-18 — [package-f9dasm] first `/package` run; 50 KB debhelper-built `.deb` (4.5× smaller than xa65's hand-roll), `Depends: libc6 (>= 2.38)` resolved, retro-tools 1.0.2 now `Depends: f9dasm`.
- 2026-05-18 — [version-deb-links] version numbers in apt index now link to `pool/main/...` `.deb`s; arch-specific code path ready for future arch-split packages.
- 2026-05-18 — [foundry-apt-live-install-tests] `test/run-test.sh` installs each published metapackage in a fresh Ubuntu 26.04 container, 5/5 pass.
- 2026-05-18 — [bootstrap-credential-cache] `BOOTSTRAP_CACHE=/tmp/foundry-linux-bootstrap.env`; step 7.5 URL-rewrite rule replaces the free-plan-disallowed redirect phase.
- 2026-05-17 — [per-metapackage-install-scripts] Phase 0 monolith split into per-metapackage scripts mirroring `foundry-apt/packages/` 1:1; orchestrator + role dispatch.
- 2026-05-17 — [upgrade-github-actions-node24] `actions/checkout@v6` + `actions/upload-artifact@v7`; `FORCE_JAVASCRIPT_ACTIONS_TO_NODE24` removed; shipped as v0.0.8.
- 2026-05-17 — [foundry-linux-phase1-bootstrap] `apt.foundrylinux.org` live on Cloudflare R2; GPG signing key in CI; publish-on-tag workflow operational.
- 2026-05-18 — [repo: monorepo home] monorepo pushed to `foundry-linux/foundrylinux.org` (private); `task sync` mirrors `foundry-apt/` subdir to `foundry-linux/foundry-apt` as before.
