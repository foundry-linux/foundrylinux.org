# TODO — foundrylinux.org

See [`docs/plans/`](docs/plans/) for written plans behind each item, and
[`docs/investigations/2026-05-16-foundry-linux-distro-proposal.md`](docs/investigations/2026-05-16-foundry-linux-distro-proposal.md) for the full roadmap.

## Open

### Phase 1 — package the source-built retro tools — COMPLETE ✓

All five source-built tools from Phase 0's `install-foundry-linux-retro-tools.sh` are now `.deb` packages in the live repo. CI is green. Phase 1 is done.

**Rule, learned the hard way (2026-05-18):** *before* packaging any source-built tool, run `apt-cache policy <pkg>` on a fresh `ubuntu:26.04`. If Ubuntu universe ships it, just add it to the Phase 0 apt-install list and the metapackage `Depends:` — don't duplicate.

- [x] ~~**xa65**~~ — retired. Ubuntu 26.04 universe ships `xa65 2.4.1-0.1build1`. `packages/xa65/` deleted; Phase 0 retro-tools script apt-installs xa65; `foundry-linux-retro-tools` `Depends: xa65` resolves to universe. See [`docs/plans/2026-05-18-retire-xa65.md`](docs/plans/2026-05-18-retire-xa65.md).
- [x] **`/package` skill** ([plan](docs/plans/2026-05-18-package-skill.md)) — reusable Claude Code skill using `dh_make` + `debhelper` + `dpkg-buildpackage`. Iteratively refined through f9dasm, libvgm, and vgmstream packaging runs.
- [x] **ghidra** — packaged and live; `foundry-linux-retro-tools` 1.0.5 `Depends: ghidra`. See done entry below.

### Site

- [ ] **Flesh out foundrylinux.org** — Claude Design landing page is live (v0.2.x); content passes as a real landing page but the design is still iterating. `tweaks-panel.jsx` still present — remove once design is finalised.

### Navbar

Motion graphics candidates for `site/embers.js`. Current: heat shimmer (traveling sine-wave `feDisplacementMap`). Previously tried and retired: breathe glow, grey pinstripes, shockwave rings, ember scatter, sword glint.

**Thermal / forge**
- [x] **Cooling bloom** ★7 — warm orange-white Gaussian luminance blob appears at random x, spreads outward, cools and fades. Purely photometric (no distortion). Like a struck piece of steel.
- [ ] **Molten seam** — 1–2 px bright horizontal line (white-orange core, soft glow) traverses the bar once left→right. The bar is a slab of metal; the seam is a crack showing what's beneath.
- [x] **Edge light** ★4 (bad clipping) — bottom edge of navbar (2–3 px) briefly brightens to near-white orange, spreads upward as a soft gradient, dims. Like looking at hot stock edge-on.

**Mechanical / percussive**
- [ ] **Anvil resonance** — single circular brightness ring expanding from a random point *within* the bar (not from below). One ring, very slow expansion, fades. The navbar surface is the anvil face.
- [ ] **Steel flex** — the bar subtly bows: a slow vertical-only displacement wave passes through once, as if the bar is a thin plate flexing. Near-imperceptible at low amplitude.
- [ ] **Weld flash** — at a random x, a 200–300 ms burst of blue-white overbrightness (welder's arc). No ramp-in: nothing → blinding dot → nothing. Brevity is the effect.

**Retro / digital**
- [ ] **Scanline roll** — a band of faint horizontal scanlines (2 px on / 2 px off, low opacity) drifts downward through the bar once. CRT monitor refresh. Connects to the MAME/arcade side of the brand.
- [ ] **Bit corruption** — a traveling zone of mild pixel-level brightness jitter (canvas `ImageData`) sweeps left→right like a slow data error crossing a screen. Restrained glitch.
- [ ] **Current flow** — extremely sparse, nearly invisible points drifting in one direction (organized, not random). DC current / electron drift visualization. Distinguish from ember scatter by directionality and faintness.

**Abstract / mathematical**
- [ ] **Interference rings** — two simultaneous circular wave systems from two random x positions; brightness adds at intersections (constructive interference). Wave-tank demo aesthetic.
- [ ] **Grain pulse** — film grain (Gaussian-distributed per-pixel brightness noise via `ImageData`) ramps up ~0.5 s, holds briefly, fades. Organic, photographic.
- [ ] **Chromatic slip** — R channel shifts 1 px left, B channel shifts 1 px right for 1–2 s, then snaps back. Near-invisible RGB split; felt more than seen. SVG `feOffset` + `feBlend`.
- [ ] **Standing wave** — the bar behaves as a plucked string: a standing wave (fixed nodes, oscillating antinodes) appears across the full width at 3–5 wavelengths, then decays in amplitude like a struck string ringing out. Acoustic physics made visible.
- [ ] **Magnetic field lines** — faint Bézier arcs (like iron filings around a bar magnet) materialize across the bar — curving from one end, looping over, curving back — fade in slowly, hold, dissolve. Cool-toned and mathematical; completely different character from every thermal/light effect.

**Thermal / forge** (batch 2)
- [ ] **Thermal gradient creep** — one edge of the bar is very subtly warm-tinted (faint orange haze); that warmth conducts slowly across to the other side over 3–4 s, then cools. Unlike cooling bloom (point source expanding outward), this is directional edge-to-edge conduction.
- [ ] **Shadow pass** — the only subtractive effect in the list: a soft Gaussian shadow drifts across via `globalCompositeOperation = 'multiply'`, slightly darkening and desaturating as it passes. The absence of light rather than its presence; suggests something heavy moving overhead.

**Mechanical / percussive** (batch 2)
- [ ] **Mercury bead** — a single small bright specular dot (radial gradient, silvery-white, 4–6 px) rolls across the bottom of the bar end to end, leaving a brief luminous trail that fades immediately. Like a ball bearing rolling across a polished surface. One subject, tracking shot.
- [ ] **Ferrofluid spikes** — along the bottom edge, 4–8 thin vertical spikes of varying heights (10–20 px) shoot upward simultaneously, hold ~200 ms, retract. Like ferrofluid snapping to a magnet. Canvas lines drawn from bottom up; distinctive silhouette behavior unlike anything else in the list.

**Retro / digital** (batch 2)
- [ ] **Letterpress ghost** — a watermark of "ANVIL" or a single glyph materializes at very low opacity (~0.05), holds, fades. No motion; purely the ghostly presence of something barely legible. Like a printing plate impression bleeding through the surface.
- [ ] **Phosphor afterimage** — the navbar leaves a faint ghost copy at its previous painted position, drifts upward a few pixels, and dissolves over ~0.6 s. CRT phosphor persistence. The effect is temporal — the memory of where the bar just was.

**Material / optical**
- [ ] **Oil slick** — a very subtle iridescent sheen (desaturated spectral colors — pale blue, violet, green — at ~0.03 opacity) drifts slowly across the bar. Thin-film interference, like the surface of a freshly quenched blade or motor oil on water. Soft, continuous, never distracting.
- [ ] **Spangle** — a brief overlay of delicate crystalline geometry (Voronoi cell edges, thin and bright) flashes onto the bar and fades. Like zinc spangle on galvanized steel: industrial, geometric, beautiful. Unique in revealing a mathematical structure rather than emitting or distorting light.

### Phase 2 — Distrobox image

- [ ] Build `ghcr.io/foundry-linux/devbox:26.04` — Containerfile (Kubuntu/Ubuntu 26.04 base) + `apt.foundrylinux.org` configured + `foundry-linux-dev` preinstalled. GHCR workflow for tag-driven publish. Per proposal §"Channel 3 — OCI/container image".

### Phase 3 — Foundry Linux ISO

- [ ] Kubuntu 26.04-based ISO via `livecd-rootfs`. Calamares installer with Foundry Linux branding. Per proposal §"Channel 4 — bootable ISO".

### apt.worldfoundry.org [deferred]

- [ ] **apt.worldfoundry.org Phase A** [deferred] — `worldfoundry.org/apt/` infrastructure + gen-index site; migrate `new-web-apt-repo` command → global skill with SKILL.md + templates (Tailwind v4, design-mode prompting). See [plan](docs/plans/2026-05-18-worldfoundry-apt-repo.md).
- [ ] **apt.worldfoundry.org Phase B** [deferred] — five `worldfoundry-*` metapackages + `worldfoundry-blender-addon`; remove worldfoundry-* from `foundry-apt/`.

### Housekeeping
- [ ] **Fresh-VM retro-tools end-to-end test.** Run `install-foundry-linux-retro-tools.sh` on a clean Ubuntu 26.04 VM and confirm all tools are available (`ghidra`, `vgmstream-cli`, `f9dasm`, `vgm2wav`, `mame`, etc.).
- [ ] **Flip monorepo to public** once content is ready: `gh repo edit foundry-linux/foundrylinux.org --visibility public`.

## Done

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
- 2026-05-18 — [bootstrap-credential-cache] `BOOTSTRAP_CACHE=/tmp/foundry-linux-bootstrap.env`; step 7.5 URL-rewrite rule replaces the free-plan-disallowed redirect phase.
- 2026-05-17 — [per-metapackage-install-scripts] Phase 0 monolith split into per-metapackage scripts mirroring `foundry-apt/packages/` 1:1; orchestrator + role dispatch.
- 2026-05-17 — [upgrade-github-actions-node24] `actions/checkout@v6` + `actions/upload-artifact@v7`; `FORCE_JAVASCRIPT_ACTIONS_TO_NODE24` removed; shipped as v0.0.8.
- 2026-05-17 — [foundry-linux-phase1-bootstrap] `apt.foundrylinux.org` live on Cloudflare R2; GPG signing key in CI; publish-on-tag workflow operational.
- 2026-05-18 — [repo: monorepo home] monorepo pushed to `foundry-linux/foundrylinux.org` (private); `task sync` mirrors `foundry-apt/` subdir to `foundry-linux/foundry-apt` as before.
