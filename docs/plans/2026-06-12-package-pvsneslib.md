# Package PVSnesLib as a .deb (foundry-apt)

## Context

PVSnesLib is a small, free SNES homebrew development kit (toolchain + C library +
asset-conversion tools) by Alekmaul. It is **not** in Ubuntu 26.04 universe
(verified — no candidate, no near-match). We want it in `apt.foundrylinux.org` so
Foundry/anvil users can build SNES games out of the box.

PVSnesLib is a **self-contained SDK** keyed off the `PVSNESLIB_HOME` environment
variable. Its build glue (`devkitsnes/snes_rules`) hard-references
`$(PVSNESLIB_HOME)/devkitsnes/bin/816-tcc`, `.../bin/wla-65816`,
`.../tools/gfx2snes`, `.../pvsneslib/include`, `.../pvsneslib/lib/...` — so the
entire tree must install under one root and `PVSNESLIB_HOME` must point at it. The
devkit is used by writing a Makefile that `include`s `snes_rules` and running
`make`; the tools are never invoked directly from `$PATH`.

Upstream's official distribution is a **pre-built amd64 Linux zip** (4.5.0,
2025-12-28). We vendor it exactly as the existing `ghidra` package does.

**Upstream facts (verified):**
- Repo: [alekmaul/pvsneslib](https://github.com/alekmaul/pvsneslib) — release tag `4.5.0`
- Asset: `pvsneslib_450_64b_linux.zip` (11.5 MB), sha256
  `b69ff32ada19895b7ebfe02a1e3c08a44c80bd9c8132de05f5c356f86264ce32`
- Zip top dir `pvsneslib/` contains `devkitsnes/` (13 MB toolchain ELF),
  `pvsneslib/` (6.5 MB lib+include+html), `snes-examples/` (47 MB),
  `vscode-template/` (16 K).
- License: **Zlib** (the `pvsneslib_license.txt` is verbatim zlib text — GitHub's
  "MIT" label is wrong; trust the license file).

## Decisions (confirmed with user)

| Decision | Choice |
|---|---|
| Build approach | Pre-built repack of the official Linux zip (model on `ghidra`). |
| Package split | core / examples / meta — one source, three binaries. |
| PATH exposure | `PVSNESLIB_HOME` only via `/etc/profile.d/pvsneslib.sh`; no `/usr/bin` symlinks. |
| Metapackage wiring | Add `pvsneslib-core` to both `foundry-retro-tools` and `foundry-game-frameworks`. |

## Package layout (one source → three binaries)

```
Source: pvsneslib   (4.5.0-1foundry1, format 3.0 (quilt))
 ├ pvsneslib-core      amd64   /usr/lib/pvsneslib/{devkitsnes,pvsneslib,vscode-template,*.txt,*.png}
 │                             /etc/profile.d/pvsneslib.sh → export PVSNESLIB_HOME=/usr/lib/pvsneslib
 ├ pvsneslib-examples  all     /usr/lib/pvsneslib/snes-examples/   (47 MB)
 └ pvsneslib           all     metapackage → Depends: core, examples
```

## Verification

(Steps run in a fresh `ubuntu:26.04` container — evidence pasted below each.)

### 1. Universe check — PVSnesLib not in Ubuntu 26.04 universe

```
$ docker run --rm ubuntu:26.04 bash -c "apt-get update -qq; apt-cache policy pvsneslib; apt-cache search pvsneslib"
(apt-cache policy pvsneslib: no output — package unknown)
(apt-cache search pvsneslib: no output — no match)
```
PASS — not in universe; no near-match. Packaging is warranted.

### 2. Build all three .debs (`packages/pvsneslib/build.sh` in ubuntu:26.04)

```
OK   /repo/dist/pvsneslib-core_4.5.0-1foundry1_amd64.deb  (1719852 bytes)
OK   /repo/dist/pvsneslib-examples_4.5.0-1foundry1_all.deb  (1594724 bytes)
OK   /repo/dist/pvsneslib_4.5.0-1foundry1_all.deb  (2252 bytes)
=== Moved 3 .debs into dist/ ===
```
PASS — three artifacts produced (core amd64, examples all, meta all). sha256 of
the upstream zip verified by `sha256sum -c` during the fetch.

### 3. Stripped binaries (`file` 816-tcc / wla-65816)

```
816-tcc:   ELF 64-bit LSB pie executable, x86-64, dynamically linked, ..., stripped
wla-65816: ELF 64-bit LSB pie executable, x86-64, dynamically linked, ..., stripped
```
PASS — dh_strip stripped the toolchain ELFs (shipped unstripped upstream); they
remain +x (`-rwxr-xr-x`) after the fixperms/strip permission dance.

### 4. Lintian clean (BLOCKING)

```
pvsneslib-core_4.5.0-1foundry1_amd64.deb      CLEAN
pvsneslib-examples_4.5.0-1foundry1_all.deb    CLEAN
pvsneslib_4.5.0-1foundry1_all.deb             CLEAN
foundry-retro-tools_1.0.12_all.deb            CLEAN   (after wrapping a pre-existing 85-char description line)
foundry-game-frameworks_1.0.3_all.deb         CLEAN
```
Only override that fires (via `pvsneslib-core.lintian-overrides`):
`statically-linked-binary` on the 9 upstream-static converter tools
(816-opt, bin2txt, constify, gfx2snes, gfx4snes, smconv, snesbrr, snestools,
tmx2snes). The 816-tcc/wla-* ELFs are already PIE, so no hardening overrides are
needed (the speculative ones were removed).
PASS — zero E:/W: across all five packages.

### 5. Dependency-chain install + PVSNESLIB_HOME

```
$ apt-get install -y pvsneslib   # from a dpkg-scanpackages local source
ii  pvsneslib           4.5.0-1foundry1
ii  pvsneslib-core      4.5.0-1foundry1
ii  pvsneslib-examples  4.5.0-1foundry1
$ . /etc/profile.d/pvsneslib.sh && echo $PVSNESLIB_HOME
/usr/lib/pvsneslib
```
PASS — the metapackage pulls in core + examples; profile.d exports
`PVSNESLIB_HOME=/usr/lib/pvsneslib`. Resolved Depends: core → `libc6 (>= 2.38)`,
examples → `pvsneslib-core (>= 4.5.0-1foundry1)`, meta → both.

### 6. End-to-end SNES ROM build from an example

```
$ . /etc/profile.d/pvsneslib.sh
$ cp -a $PVSNESLIB_HOME/snes-examples/hello_world /tmp/hw && cd /tmp/hw
$ rm -f hello_world.sfc   # remove the shipped ROM to prove a fresh build
$ make
... Build finished successfully !
$ ls -la hello_world.sfc
-rw-r--r-- 1 root root 262144 ... hello_world.sfc
```
PASS — the full toolchain (816-tcc → wla-65816 → wlalink, plus the gfx/asset
tools) builds a working 262144-byte (.sfc) SNES ROM from a clean install.

### 7. Metapackage wiring (`apt-cache depends`)

```
-- foundry-retro-tools --     Depends: pvsneslib-core
-- foundry-game-frameworks -- Depends: pvsneslib-core
```
PASS — both metapackages list `pvsneslib-core`.

---

**All seven steps PASS.** Ready to publish via `task build` / `task bump`.
