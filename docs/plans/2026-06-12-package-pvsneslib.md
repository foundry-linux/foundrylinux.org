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

## Publish-time fallout: pre-existing packages that had never hit CI

Publishing pvsneslib via `task bump` syncs the whole committed `foundry-apt/`
tree to the mirror and tags it, which built every package in CI for the first
time since several were committed. Two packages had been committed but **never
tag-published**, so CI met them fresh and they blocked the publish until fixed:

1. **shellcheck gate (v1.5.17):** `asar`, `wla-dx`, `drmon` `build.sh` had
   SC2015 (`A && B || C`) / SC2016 (literal-`$` sed) findings. Fixed: switched
   to explicit `if-then-else` `_apt`, added an SC2016 disable on drmon's literal
   `${CMAKE_CURRENT_SOURCE_DIR}` sed. (commit `c81dcfa`)
2. **drmon could not build in CI (v1.5.18) — see the dedicated section below.**

Publish landed on **v1.5.19** (green, incl. the post-publish live-repo
smoke-check). pvsneslib + all three binaries are live at `4.5.0-1foundry1`.

## drmon: making it CI-buildable + the upstream cppdap/JsonCpp bug

drmon lives in the public **`developer-resources-co/drdevtools`** repo (ours;
GPL-2.0), not as a standalone release. Three layers had to be fixed for it to
build on a clean CI runner; the third is a genuine upstream bug.

**(a) Local-only source → fetch.** `build.sh` copied from `$HOME/SRC/drdevtools`
and aborted when absent. Rewritten to fetch the **sha256-pinned repo tarball**
(`drdevtools @ 61e1303`, sha256 `c8e90d57…`) when no local checkout is present,
keeping `DRDEVTOOLS=/path` as a local-iteration override.

**(b) Undeclared build deps.** drmon needs `libcppdap-dev` and `libjsoncpp-dev`
— present on the author's box, never declared. Added to `debian/control`
Build-Depends + the `build.sh` apt line.

**(c) Upstream CMake bug — cppdap never links JsonCpp.** This is the one worth
fixing at the source. `devsys/tools/drmon/CMakeLists.txt` pulls cppdap via:

```cmake
FetchContent_Declare(cppdap GIT_REPOSITORY https://github.com/google/cppdap.git
                            GIT_TAG dap-1.58.0-a)
FetchContent_MakeAvailable(cppdap)
```

cppdap keeps its JSON backend (**JsonCpp**) as a **git submodule** under
`third_party/json`. `FetchContent` clones cppdap but does **not** populate its
submodules, so cppdap builds with an empty JSON backend and every
`Json::Value::…` symbol is undefined at link time (≈60 undefined references in
the `drmon-dap-snes`/`drmon-dap-gen` targets). It only ever "worked" on a box
where an earlier checkout had populated the submodule in the FetchContent cache;
on a clean machine it can never link.

**The upstream fix (in drdevtools, validated — drmon builds clean with it):**

```cmake
# before FetchContent_MakeAvailable(cppdap):
set(CPPDAP_USE_EXTERNAL_JSONCPP_PACKAGE ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(cppdap)
find_package(jsoncpp REQUIRED)
# in drmon_dap_target():
target_link_libraries(${TARGET} PRIVATE cppdap jsoncpp_lib Threads::Threads)
```

i.e. tell cppdap to use the **system** JsonCpp (Ubuntu 26.04 ships
`libjsoncpp-dev` at `1.58.0a`, matching the pinned cppdap tag `dap-1.58.0-a`)
and link `jsoncpp_lib` into the dap targets (drmon's `session.cpp` uses
`Json::Value` directly too). Alternative considered: switch drmon entirely to
the **system** `libcppdap-dev` (`find_package(cppdap)` instead of FetchContent)
— Ubuntu's `1.58.0a` matches the pinned tag exactly, so it would drop the
network fetch and the submodule problem together. Cleaner for a distro but a
bigger change (loses FetchContent portability on non-Debian hosts); not yet
validated. Recommended: ship the minimal external-JsonCpp fix now, consider the
system-cppdap switch later.

**Carry-forward in foundry-apt today:** `packages/drmon/build.sh` applies the
same two CMake edits via `sed` after fetching the source — so drmon builds in CI
**right now** without waiting on the upstream commit. This is a patch we carry,
and a smell.

**Follow-up tasks (tracked in TODO.md):**

1. Land the CMake fix in `drdevtools` `main` (branch + diff prepared at
   `/tmp/drdevtools-pr`, `fix/drmon-cppdap-jsoncpp-link`; we own the repo, push
   pending Will's go-ahead — the auto-classifier blocked an unprompted push).
2. Re-pin `packages/drmon/build.sh` `DRDEVTOOLS_SHA` + `SHA256` to the fixed
   commit and **delete the two jsoncpp seds** (keep the `libs/` sed and the
   `libjsoncpp-dev` build-dep). Re-verify drmon builds, then re-publish.
3. (Optional) Evaluate switching drmon to system `libcppdap-dev`.

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
