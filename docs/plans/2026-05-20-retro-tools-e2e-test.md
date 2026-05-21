---
title: End-to-end fresh-VM test for foundry-retro-tools
date: 2026-05-20
status: in-progress
---

# Retro-tools end-to-end fresh-VM test

## Context

The existing test infrastructure covers two layers but leaves a gap:

| Test | Verifies | Misses |
|---|---|---|
| `foundry-setup/test/run-test.sh` | install script runs end-to-end in Ubuntu 24.04/26.04 container | doesn't assert installed tools actually work |
| `foundry-apt/test/run-test.sh` | each Phase 1 `.deb` apt-installs and lints its man pages | doesn't run binaries — a tool can install with a clean man page but crash on first invocation |

What's missing: a fresh-VM-equivalent run that does the **user-facing flow** — add the apt source, install `foundry-retro-tools`, then invoke each tool the metapackage promised — exactly the path documented in TODO.md's housekeeping list.

## Approach

Add `foundry-setup/test/test-retro-tools-e2e.sh` — a focused harness that:

1. Spins a clean `ubuntu:26.04` container (podman/docker).
2. Runs `setup-foundry-apt-source.sh` to wire `apt.foundrylinux.org`.
3. Runs `install-foundry-retro-tools.sh` (the actual Phase 0 entry point users hit).
4. **For each tool from the metapackage `Depends:`** runs a benign invocation (`--version`, `--help`, or a no-arg usage print) and confirms the binary is on `$PATH` and doesn't crash.

The tool list is parsed from `foundry-apt/packages/foundry-retro-tools/debian/control`'s `Depends:` stanza so the test stays in sync as the metapackage evolves — same discipline as `foundry-apt/test/run-test.sh` already uses.

### Tool → invocation table

| Package | Binary | Invocation | Notes |
|---|---|---|---|
| `mame` | `mame` | `mame -version` | exits 0 |
| `mame-tools` | `chdman` | `chdman --help` | exits 0 |
| `dasm` | `dasm` | `dasm` | no args prints usage; exits 1 — accept any non-signal exit |
| `cc65` | `cc65` / `sim65` / `da65` | `cc65 -V && sim65 -V && da65 -V` | |
| `z80dasm` | `z80dasm` | `z80dasm -v` | |
| `z80asm` | `z80asm` | `z80asm -v` | |
| `radare2` | `r2` | `r2 -v` | binary is `r2`/`radare2` |
| `binwalk` | `binwalk` | `binwalk --help` | |
| `sox` | `sox` | `sox --version` | |
| `binutils-m68k-linux-gnu` | `m68k-linux-gnu-as` / `-objdump` | `--version` each | |
| `xa65` | `xa` | `xa --version` | binary is `xa` not `xa65` |
| `f9dasm` | `f9dasm` | `f9dasm -V` | |
| `libvgm` | `vgm-player` / `vgm2wav` | `--help` each | |
| `vgmstream` | `vgmstream-cli` | `vgmstream-cli` | no args prints usage; non-zero accepted |
| `ghidra` | `ghidra` / `ghidra-headless` | path check only for `ghidra` (GUI launcher); `ghidra-headless` with no args prints usage | GUI launcher would block the test |

Exit-code policy: signal-terminated (≥128) = fail; any other exit = pass (lots of CLI tools exit non-zero on `--help` or no-args usage prints).

### Integration

- Add a `task retro-tools-e2e` target to the top-level Taskfile so it's discoverable.
- Add a "test" section to the new doc and link from TODO.md (replace the housekeeping bullet with a `[verify]` reference to this plan).
- Don't wire into CI yet — it pulls from the live apt repo and takes 3–5 minutes; first iteration is local-only, then promote to CI once the assertion shape is stable.

## Verification

1. `bash foundry-setup/test/test-retro-tools-e2e.sh` exits 0 on a machine with podman or docker; prints `✓ <tool>` for each binary and a final summary `N/N tools verified`.
2. Deliberately mistype one invocation (e.g. `mame --xyz`) and confirm the script exits non-zero and clearly identifies the failed tool.
3. The script uses `mapfile` against the live `debian/control` Depends list — adding a new dep to the metapackage and re-running discovers it without code edits (or fails loudly if no invocation table entry exists).
