# TODO — foundrylinux.org

See [`docs/plans/`](docs/plans/) for written plans behind each item, and
[`docs/investigations/2026-05-16-foundry-linux-distro-proposal.md`](docs/investigations/2026-05-16-foundry-linux-distro-proposal.md) for the full roadmap.

## Open

### Phase 1 — package the source-built retro tools

Phase 0's `install-foundry-linux-retro-tools.sh` source-builds five tools into `~/opt/`. **One (xa65) turned out to already be in Ubuntu 26.04 universe** — see lesson below. The other four are still genuinely missing from Ubuntu, confirmed via `apt-cache policy` on a fresh `ubuntu:26.04`. Each one below needs `packages/<name>/{DEBIAN/control,build.sh}`.

**Rule, learned the hard way (2026-05-18):** *before* packaging any source-built tool, run `apt-cache policy <pkg>` on a fresh `ubuntu:26.04`. If Ubuntu universe ships it, just add it to the Phase 0 apt-install list and the metapackage `Depends:` — don't duplicate.

- [x] ~~**xa65**~~ — already in Ubuntu 26.04 universe as `2.4.1-0.1build1`. We shipped a redundant package as `2.4.1-1foundry1` (works, wins by version sort) — see [`docs/plans/2026-05-18-package-xa65.md`](docs/plans/2026-05-18-package-xa65.md). **Cleanup follow-up:** delete `packages/xa65/`, drop xa65 from the Phase 0 source-build block, add xa65 to the apt-install list. Defer the deletion until after the "Phase 0 configures foundry-apt as a source" TODO lands so the cleanup ships as one atomic change.
- [ ] **`/package` skill** ([plan](docs/plans/2026-05-18-package-skill.md)) — generalize the xa65 packaging effort into a reusable user-level Claude Code skill. Use `dh_make` + `debhelper` + `dpkg-buildpackage` for Debian-policy-compliant packages with auto debug-strip, accurate `${shlibs:Depends}`, build hardening, DEP-5 copyright. **Use this skill to package the four below** instead of hand-rolling `build.sh` like xa65.
- [ ] **libvgm** — chip-register VGM library ([github.com/ValleyBell/libvgm](https://github.com/ValleyBell/libvgm)). Confirmed not in Ubuntu universe.
- [ ] **vgmstream** — VGM/audio stream decoder ([github.com/vgmstream/vgmstream](https://github.com/vgmstream/vgmstream)). Confirmed not in Ubuntu universe.
- [ ] **ghidra** — NSA reverse-engineering suite (currently zip download). Confirmed not in Ubuntu universe. Heavyweight (~400 MB); may need a separate metapackage rather than bundling.
- [ ] After all four ship: have `foundry-linux-retro-tools` Depends on them, then strip the source-build sidecars from `install-foundry-linux-retro-tools.sh` (also strip xa65's then). Plan rehearses this collapse in [`docs/plans/2026-05-17-per-metapackage-install-scripts.md`](docs/plans/2026-05-17-per-metapackage-install-scripts.md) §"Phase 1 collapse rehearsal".

### Phase 2 — Distrobox image

- [ ] Build `ghcr.io/foundry-linux/devbox:26.04` — Containerfile (Kubuntu/Ubuntu 26.04 base) + `apt.foundrylinux.org` configured + `foundry-linux-dev` preinstalled. GHCR workflow for tag-driven publish. Per proposal §"Channel 3 — OCI/container image".

### Phase 3 — Foundry Linux ISO

- [ ] Kubuntu 26.04-based ISO via `livecd-rootfs`. Calamares installer with Foundry Linux branding. Per proposal §"Channel 4 — bootable ISO".

### Housekeeping

- [ ] **Phase 0 configures foundry-apt as an apt source.** `foundry-linux-setup/install.sh` (or a sibling `setup-foundry-apt-source.sh`) should add `https://apt.foundrylinux.org` to `sources.list.d`, import the signing key, and `apt-get update` — before any per-meta script runs. Unblocks stripping the Phase 0 source-build sidecars (`xa65`, then `f9dasm`, `libvgm`, etc., as each gets packaged). Pattern reference: `install-task.sh` (adds Cloudsmith apt repo for `task`).
- [ ] **Worldfoundry → foundry-linux metapackage rename.** `packages/worldfoundry-{android-dev,blender,dev,engine-build-deps}/` are legacy names. The distro is "Foundry Linux"; consider renaming the metapackages and/or shipping `foundry-linux-*` as aliases that `Depends:` on the WF ones.
- [ ] **Fresh-VM retro-tools end-to-end test.** Run `install-foundry-linux-retro-tools.sh` (not the metapackage) on a clean Ubuntu 26.04 VM and confirm all source-builds succeed + binaries appear at expected paths.
- [ ] **Flip monorepo to public** once content is ready: `gh repo edit foundry-linux/foundrylinux.org --visibility public`.

## Done

- 2026-05-18 — [live-test-manpage-assert] `test/run-test.sh` drops `dpkg.cfg.d/excludes`, mandoc-lints every `/usr/bin/<name>.1.gz`; caught real `PP-after-SH` WARNING in f9dasm 1foundry2 → bumped to 1foundry3.
- 2026-05-18 — [package-f9dasm] first `/package` run; 50 KB debhelper-built `.deb` (4.5× smaller than xa65's hand-roll), `Depends: libc6 (>= 2.38)` resolved, retro-tools 1.0.2 now `Depends: f9dasm`.
- 2026-05-18 — [version-deb-links] version numbers in apt index now link to `pool/main/...` `.deb`s; arch-specific code path ready for future arch-split packages.
- 2026-05-18 — [foundry-apt-live-install-tests] `test/run-test.sh` installs each published metapackage in a fresh Ubuntu 26.04 container, 5/5 pass.
- 2026-05-18 — [bootstrap-credential-cache] `BOOTSTRAP_CACHE=/tmp/foundry-linux-bootstrap.env`; step 7.5 URL-rewrite rule replaces the free-plan-disallowed redirect phase.
- 2026-05-17 — [per-metapackage-install-scripts] Phase 0 monolith split into per-metapackage scripts mirroring `foundry-apt/packages/` 1:1; orchestrator + role dispatch.
- 2026-05-17 — [upgrade-github-actions-node24] `actions/checkout@v6` + `actions/upload-artifact@v7`; `FORCE_JAVASCRIPT_ACTIONS_TO_NODE24` removed; shipped as v0.0.8.
- 2026-05-17 — [foundry-linux-phase1-bootstrap] `apt.foundrylinux.org` live on Cloudflare R2; GPG signing key in CI; publish-on-tag workflow operational.
- 2026-05-18 — [repo: monorepo home] monorepo pushed to `foundry-linux/foundrylinux.org` (private); `task sync` mirrors `foundry-apt/` subdir to `foundry-linux/foundry-apt` as before.
