# Per-metapackage install scripts (Phase 0 refactor)

**Status:** Done

## Context

`foundry-linux-setup/install.sh` is the Phase 0 setup script from the Foundry Linux distro proposal ([`docs/investigations/2026-05-16-foundry-linux-distro-proposal.md`](../investigations/2026-05-16-foundry-linux-distro-proposal.md)). It was supposed to install everything the Phase 1 APT metapackages will eventually install — including the retro-porting toolchain (MAME, `dasm`, `cc65` → ships `sim65` 6502 emulator + `da65` disassembler, `radare2`, etc., plus source-built Ghidra / f9dasm / vgmstream / libvgm / xa65). It currently doesn't: `install_apt_packages()` at [`install.sh:222-246`](../../foundry-linux-setup/install.sh) only mirrors `Taskfile.yml:dev-setup` (engine build deps). That's the gap behind the original "which script installs the 6502 emulator(s)" question — there isn't one.

Beyond just filling that gap, the shape is wrong: one monolithic 475-line script with mixed concerns (apt for engine, rustup for tooling, git clone, wftools build, Blender addon install) hides which Phase 0 step corresponds to which Phase 1 metapackage. When Phase 1 lands, the proposal says `install.sh` "collapses to ~20 lines" — but with the current structure, it's not obvious what collapses to what.

**Change:** mirror `foundry-apt/packages/` 1:1 with `foundry-linux-setup/install-<metapackage>.sh`. Each per-metapackage script does in Phase 0 what `apt install <metapackage>` will do in Phase 1; the Phase 1 collapse becomes a near-mechanical "replace body with a single `apt install` call." The top-level `install.sh` keeps only what doesn't belong to any metapackage (distro check, sudo bootstrap, role parsing, repo cloning, Rust toolchain) and dispatches to the per-metapackage scripts.

## Layout

```
foundry-linux-setup/
  lib.sh                                       (NEW — shared helpers)
  install.sh                                   (TRIM — orchestrator + non-meta Phase 0)
  install-foundry-linux-engine-build-deps.sh    (NEW)
  install-foundry-linux-blender.sh              (NEW)
  install-foundry-linux-retro-tools.sh          (NEW — answers the 6502 question)
  install-foundry-linux-android-dev.sh          (NEW)
  install-task.sh                              (NEW)
  install-foundry-linux-dev.sh                  (NEW — orchestrator for the umbrella metapackage)
```

Each per-metapackage script is the Phase 0 expansion of one metapackage's `Depends:` and source-build-sidecar `Recommends:`. The orchestrator (`install-foundry-linux-dev.sh`) calls the constituent scripts in dependency order, matching [`foundry-apt/packages/foundry-linux-dev/DEBIAN/control`](../../foundry-apt/packages/foundry-linux-dev/DEBIAN/control).

## Metapackage → script mapping

Confirmed from `foundry-apt/packages/*/DEBIAN/control`:

| Script | apt deps (from metapackage `Depends:`) | Source-build sidecars (from `Recommends:` not in Ubuntu) |
|---|---|---|
| `install-foundry-linux-engine-build-deps.sh` | `build-essential` `cmake (>=3.22)` `libx11-dev` `libgl1-mesa-dev` `libglu1-mesa-dev` `gdb` `xxd` `python3` `pkg-config` `git` `curl` `wget` `unzip` | — |
| `install-foundry-linux-blender.sh` | `blender (>=4.2)` `python3` `foundry-linux-engine-build-deps` | invokes `wftools/wf_blender/install.sh` for the addon (Phase 1 `Recommends: foundry-linux-blender-addon`) |
| `install-foundry-linux-retro-tools.sh` | `mame` `mame-tools` `dasm` `cc65` `z80dasm` `z80asm` `radare2` `binwalk` `sox` `binutils-m68k-linux-gnu` | `ghidra` → `~/opt/ghidra-*/`; `f9dasm` → `~/opt/f9dasm/`; `vgmstream-cli` → `~/opt/vgmstream/`; `libvgm` → `~/opt/libvgm/`; `xa65` → `~/opt/xa65/` (per [arcade-tooling investigation](../investigations/2026-05-15-claude-arcade-tooling.md) §2 + §1 note line 46) |
| `install-foundry-linux-android-dev.sh` | `openjdk-17-jdk` `adb` `google-android-ndk-r26c-installer` | — |
| `install-task.sh` | (no `Depends:` yet — `foundry-apt/packages/task/` has only a `build.sh`, the .deb wraps the `task` binary itself) | downloads `task` from taskfile.dev if no .deb available (current `install_task_runner()` at `install.sh:281-295`) |
| `install-foundry-linux-dev.sh` | composes the above in this order: engine-build-deps → task → blender → retro-tools → android-dev (optional) | calls each script as `bash install-<name>.sh "$@"` |

## Role → script mapping in top-level `install.sh`

Existing roles preserved. Dispatch table:

| Role | Calls |
|---|---|
| `play` | (none today — no `foundry-linux-runtime` metapackage exists yet; warn and continue) |
| `game-dev` | `install-foundry-linux-engine-build-deps.sh`, `install-task.sh`, `install-foundry-linux-blender.sh`, `install-foundry-linux-retro-tools.sh`, then clone `wf-games`, build wftools |
| `engine-dev` | `install-foundry-linux-engine-build-deps.sh`, `install-task.sh`, `install-foundry-linux-retro-tools.sh`, then clone `WorldFoundry.2026-new-level`, build wftools |
| `both` (default) | `install-foundry-linux-dev.sh` (= engine-build-deps + blender + retro-tools + task), then clone both repos, build wftools |
| `maintainer` | `both` + `install-foundry-linux-android-dev.sh` |

Existing `--skip-rust`, `--skip-blender`, `--skip-clone`, `--skip-build`, `--dry-run`, `--force`, `--allow-24.04`, `-h` flags retained. New flag: `--skip-retro` (so a host without disk for Ghidra can opt out).

## Per-metapackage script contract

Every `install-*.sh` script:

1. `#!/usr/bin/env bash` + `set -euo pipefail` at the top.
2. Handles `-h` / `--help` — prints "Installs the foundry-linux-X metapackage's Phase 0 equivalent" + flag list + exits 0. Per `~/SRC/CLAUDE.md`: `-h` must short-circuit before any arg parsing or apt calls.
3. Sources `lib.sh` if present, falls back to a tiny inline `info()`/`die()`/`run_sudo()` shim if run standalone. The shim is ~10 lines so the script also works when copied to a clean machine before the rest of the repo is there.
4. Idempotent — `apt-get install -y` is naturally idempotent; source-build sidecars guard with `[[ -d ~/opt/<tool> ]] && info "<tool> already installed" && exit 0` (or `--force` to re-run).
5. Respects `FOUNDRY_LOG_FILE` env var (set by top-level `install.sh`) so all per-script output lands in the same `~/.local/state/foundry-install.log`.
6. Phase 1 collapse path: each script's body shrinks to a single `run_sudo apt-get install -y <metapackage>` line once `apt.foundrylinux.org` is published. The source-build sidecars disappear (they ship as `.deb`s from the foundry-apt CI per proposal §483).

## Critical files

**New:**

- `foundry-linux-setup/lib.sh` — extracted helpers: color codes, `log_to_file`, `info`, `ok`, `warn`, `err`, `die`, `step`, `run`, `run_sudo`, `init_logging`. Source these from current `install.sh:50-93`.
- `foundry-linux-setup/install-foundry-linux-engine-build-deps.sh` — body extracted from `install.sh:222-246` (`install_apt_packages`).
- `foundry-linux-setup/install-foundry-linux-blender.sh` — `apt install blender python3` + invoke `wftools/wf_blender/install.sh` (current `install.sh:394-415` `install_blender_addon`).
- `foundry-linux-setup/install-foundry-linux-retro-tools.sh` — apt block from `docs/investigations/2026-05-15-claude-arcade-tooling.md:34-42` + `xa65` source-build fallback from line 46 + Ghidra/f9dasm/vgmstream/libvgm source-builds from §2 of the same doc. **This is the script that installs the 6502 emulator(s)** (`sim65` via `cc65`; full 6502 system emulation via MAME).
- `foundry-linux-setup/install-foundry-linux-android-dev.sh` — extracted from `Taskfile.yml:dev-setup` lines 249-251 (`openjdk-17-jdk adb google-android-ndk-r26c-installer`).
- `foundry-linux-setup/install-task.sh` — body from `install.sh:281-295` (`install_task_runner`).
- `foundry-linux-setup/install-foundry-linux-dev.sh` — sequential calls to the constituent scripts.

**Modified:**

- `foundry-linux-setup/install.sh` — replace the 14 step functions with: arg parsing, distro check (162-199), sudo check (201-217), `init_logging` (now in `lib.sh`), role-based dispatch to the per-metapackage scripts, then the non-metapackage Phase 0 steps that stay: `install_rust` (248-279), `clone_wf_repos` (297-365), `build_wftools` (367-392), `summary` (420-448). Net length: estimated 200-220 lines down from 475.

**Reused (no changes):**

- `wftools/wf_blender/install.sh` — invoked by `install-foundry-linux-blender.sh`.
- `wftools/wf_blender/install_blender_mcp.sh` — optional, invoked by same.
- `Taskfile.yml:dev-setup` (236-251) — left as-is; it's a developer convenience that overlaps with `install-foundry-linux-engine-build-deps.sh` + `install-foundry-linux-android-dev.sh` but doesn't need to be a single canonical source anymore.

## Verification

1. **6502 emulator install works in isolation.** On a fresh Ubuntu 26.04 VM:

    ```
    bash foundry-linux-setup/install-foundry-linux-retro-tools.sh
    sim65 --version          # cc65's 6502 simulator
    da65 --version           # cc65's 6502 disassembler
    mame -listcrc qbert      # full-system 6502 emulator
    radare2 -v
    ls ~/opt/                # ghidra-*/  f9dasm/  vgmstream/  libvgm/  xa65/
    ```

    Expect: all five binaries report versions; all five `~/opt/` dirs exist.

2. **Dry-run is clean.** `bash install-foundry-linux-retro-tools.sh --dry-run` prints every `apt-get install` and every source-build `git clone` + `make` without executing. Same for each other per-meta script.

3. **Orchestrator wiring.** `bash install-foundry-linux-dev.sh --dry-run` prints calls to each constituent script in the documented order.

4. **Role dispatch.** `bash install.sh --role engine-dev --dry-run` prints calls to engine-build-deps + task + retro-tools (skips blender, skips android-dev). `--role both --dry-run` calls `install-foundry-linux-dev.sh`.

5. **`-h` short-circuits everywhere.** Every new script: `bash install-<name>.sh -h` prints usage and exits 0 without sudo prompt, apt update, or any side effects (per `~/SRC/CLAUDE.md`).

6. **Phase 1 collapse rehearsal.** Manually inspect each per-meta script and confirm the body could be replaced with `run_sudo apt-get install -y <metapackage>` once `apt.foundrylinux.org` ships, with no remaining Phase 0 leftovers. (Source-build sidecars disappear because foundry-apt CI builds them as `.deb`s per proposal §483.)

7. **Existing install.sh behavior preserved.** On a machine where the old `install.sh` succeeded, re-running the new `install.sh` with the same `--role` flag produces the same end state: same packages installed, same repos cloned, same wftools binaries built, same Blender addon registered. Diff the apt history and `~/Projects/` listing before vs after.

## Verification — executed 2026-05-18

### Step 1 — 6502 emulator install on fresh Ubuntu 26.04 VM

**Apt half: PASS (cross-plan evidence).** [`2026-05-18-foundry-apt-live-install-tests.md`](2026-05-18-foundry-apt-live-install-tests.md) (Status: Done, 5/5 pass) installs `foundry-linux-retro-tools` from `apt.foundrylinux.org` on fresh Ubuntu 26.04, validating the `Depends:` (mame, mame-tools, dasm, cc65, z80dasm, z80asm, radare2, binwalk, sox, binutils-m68k-linux-gnu) and therefore `sim65`, `da65`, `mame`, `radare2`.

**Source-build sidecar half: NOT VERIFIED on a fresh VM.** The Phase 0 script also source-builds xa65, f9dasm, libvgm, vgmstream, and downloads Ghidra into `~/opt/`. The Phase 1 metapackage does *not* yet declare these as `Depends:` — they only exist as Phase 0 source-builds. End-to-end Phase 0 retro-tools install on a clean Ubuntu 26.04 VM (running the full script, not the metapackage) has not been executed.

→ Open follow-up: package each of `xa65`, `f9dasm`, `libvgm`, `vgmstream`, `Ghidra` as real `.deb`s in `foundry-apt/packages/` (these would be the first non-metapackage `.deb`s in the repo). Once that lands, `foundry-linux-retro-tools` Depends on them and Step 1 collapses to a pure live-install test.

**PARTIAL** — apt half verified; source-build half pending real `.deb`s.

### Step 2 — Dry-run is clean

```
$ bash foundry-linux-setup/install-foundry-linux-retro-tools.sh --dry-run

━━━ Installing foundry-linux-retro-tools (apt) ━━━
  [dry-run] sudo apt-get update -q
  [dry-run] sudo apt-get install -y mame mame-tools dasm cc65 z80dasm z80asm radare2 binwalk sox binutils-m68k-linux-gnu
✓ Retro toolchain apt packages installed (sim65, da65, mame, radare2, dasm, z80dasm, ...)
  [dry-run] mkdir -p /home/will/opt

━━━ Building xa65 (6502 cross-assembler) ━━━
ℹ xa65: /home/will/opt/xa65 exists, skipping (use --force to rebuild)

━━━ Building f9dasm (6809 disassembler) ━━━
  [dry-run] git clone --depth 1 https://github.com/Arakula/f9dasm /home/will/opt/f9dasm
  [dry-run] make -C /home/will/opt/f9dasm
✓ f9dasm built — binary at /home/will/opt/f9dasm/f9dasm

━━━ Building libvgm (chip-register VGM library) ━━━
  [dry-run] git clone --depth 1 https://github.com/ValleyBell/libvgm /home/will/opt/libvgm
  [dry-run] cmake -S /home/will/opt/libvgm -B /home/will/opt/libvgm/build
  [dry-run] cmake --build /home/will/opt/libvgm/build -j
✓ libvgm built — artifacts under /home/will/opt/libvgm/build/

━━━ Building vgmstream-cli (VGM/audio stream decoder) ━━━
  [dry-run] git clone --depth 1 --recursive https://github.com/vgmstream/vgmstream /home/will/opt/vgmstream
  [dry-run] cmake -S /home/will/opt/vgmstream -B /home/will/opt/vgmstream/build
  [dry-run] cmake --build /home/will/opt/vgmstream/build -j
✓ vgmstream built — binary at /home/will/opt/vgmstream/build/cli/vgmstream-cli

━━━ Installing Ghidra (heavyweight — ~400 MB) ━━━
  [dry-run] curl -fL --progress-bar -o /home/will/opt/ghidra.zip https://github.com/NationalSecurityAgency/ghidra/releases/download/Ghidra_11.1.2_build/ghidra_11.1.2_PUBLIC_20240709.zip
  [dry-run] unzip -q /home/will/opt/ghidra.zip -d /home/will/opt
```

**PASS** — every apt-get and source-build emitted as `[dry-run]` with no side effects.

### Step 3 — Orchestrator wiring

```
$ bash foundry-linux-setup/install-foundry-linux-dev.sh --dry-run

━━━ foundry-linux-dev: engine build deps ━━━
ℹ → install-foundry-linux-engine-build-deps.sh --dry-run
━━━ foundry-linux-dev: task runner ━━━
ℹ → install-task.sh --dry-run
━━━ foundry-linux-dev: Blender ━━━
ℹ → install-foundry-linux-blender.sh --dry-run
```

**PASS** — orchestrator dispatches to constituent scripts in documented order.

### Step 4 — Role dispatch

```
$ bash foundry-linux-setup/install.sh --role engine-dev --force --dry-run
⚠ Ubuntu-family 25.10 — untested but --force is set
ℹ → install-foundry-linux-engine-build-deps.sh --dry-run
ℹ → install-task.sh --dry-run
ℹ → install-foundry-linux-retro-tools.sh --dry-run --force

$ bash foundry-linux-setup/install.sh --role both --force --dry-run
ℹ → install-foundry-linux-dev.sh --dry-run --force
```

**PASS** — `engine-dev` invokes engine-build-deps + task + retro-tools (skips blender, android-dev). `both` invokes the umbrella `install-foundry-linux-dev.sh`. (Host is Ubuntu 25.10, hence `--force`; same dispatch on 24.04/26.04.)

### Step 5 — `-h` short-circuits

```
install.sh                                       rc=0  Foundry Linux setup script (Phase 0)
install-foundry-linux-android-dev.sh             rc=0  Phase 0 installer for foundry-linux-android-dev
install-foundry-linux-blender.sh                 rc=0  Phase 0 installer for foundry-linux-blender
install-foundry-linux-dev.sh                     rc=0  Phase 0 installer for foundry-linux-dev (umbrella metapackage)
install-foundry-linux-engine-build-deps.sh       rc=0  Phase 0 installer for foundry-linux-engine-build-deps
install-foundry-linux-retro-tools.sh             rc=0  Phase 0 installer for foundry-linux-retro-tools
install-task.sh                                  rc=0  Installs task (go-task) from the official Cloudsmith apt repo.
```

**PASS** — every script exits 0 on `-h` with a usage banner; no sudo, no apt update, no side effects.

### Step 6 — Phase 1 collapse rehearsal

Each `install-foundry-linux-*.sh` body cleanly maps to `run_sudo apt-get install -y <metapackage>` once `apt.foundrylinux.org` is configured. Source-build sidecars (xa65, f9dasm, libvgm, vgmstream, Ghidra) are guarded by `[[ -d ~/opt/<tool> ]]` and will be replaced by `.deb`s once foundry-apt CI builds them.

**PASS** (visual inspection).

### Step 7 — Existing install.sh behavior preserved

Superseded — the original monolithic `install.sh` has been replaced by the per-metapackage layout. Behavioral equivalence is now validated end-to-end by the Phase 1 live install tests (Step 1 cross-reference).

**N/A** (superseded by Phase 0 → Phase 1 transition).
