# Finish xemu `xpc` / Fake86 integration before packaging it

**Date:** 2026-08-06  
**Priority:** T4  
**Scope:** Reconcile xemu's experimental `targets/pc` with LGB's standalone Fake86 fork, finish a documented PC/XT-compatible baseline, add deterministic boot-image tests, and expose `xpc` only after upstream removes its experimental warning and promotes the target into the normal build.

## Outcome

`xpc` becomes a maintained PC/XT-class emulator in LGB's xemu suite rather than a buildable demonstration. It boots known-good floppy and hard-disk images through a coherent internal BIOS, accepts keyboard input, advances timer interrupts at the expected rate, renders text correctly, reports disk errors without terminating the emulator, and has automated tests that prove those behaviours.

This work is upstream-first. Foundry must not add `xpc` to the `xemu` binary package merely because a local branch passes tests. The release gate is an upstream commit that removes “Unfinished” / “Do not use this (yet)”, adds `pc` to the root `TARGETS`, or otherwise explicitly declares the target supported.

## Current state and reconciliation finding

The package is pinned to xemu commit `40dfef0d1d5f56be2469492715c12bdb32c75b67`. At that commit:

- `targets/pc/README.md` explicitly says the target is unfinished, should not be used, and still needs unification with [LGB's Fake86 fork](https://github.com/lgblgblgb/fake86).
- `xpc` builds only when invoked directly. The root `Makefile` omits `pc` from `TARGETS`.
- `targets/pc/cpu.c` is a substantial 2022 xemu-specific rewrite of the Fake86 CPU core, not a clean copy of the standalone fork's 2020 `src/cpu.c`. Both contain changes absent from the other; replacing either wholesale would lose work.
- The standalone fork has the broader machine model: BIOS disk services, optional ATA, 8253 PIT, 8259 PIC, 8237 DMA, keyboard/mouse input, serial mouse, speaker/audio, AdLib/Sound Blaster, and networking.
- `xpc` has only `cpu`, memory, an internal BIOS, text video, and its xemu frontend. Its `portin*` and `portout*` functions are logging stubs returning all ones, so there is no hardware interrupt or device model behind the CPU.
- `xpc`'s key table is copied from the Primo emulator and is not a PC keyboard mapping. Its generic key callback does nothing.
- Boot media is hard-coded as a 1.44 MB `dos-boot.img`. There is no user-facing drive configuration, media lifecycle, safe read-only mode, or hard-disk path.
- BIOS services are partial. Several functions contain TODOs, unknown BIOS traps can call `exit(1)`/`FATAL`, unsupported INT 13h calls may terminate the emulator, and some video, keyboard, equipment, and timing results are placeholders.
- Video is fixed to BIOS text memory at `B800:0000`; there is no register-level adapter behaviour. That is sufficient for the first supported baseline only if the limitation is explicit and boot tests demonstrate that normal PC/XT software tolerates it.

Before implementation, preserve both inputs as named references: the xemu base commit and an exact Fake86 commit, not the moving `master` branch. Produce a file/function provenance table and a semantic diff covering CPU fixes, BIOS services, disk geometry/error handling, and devices. The table is part of the upstream pull request so future changes do not create a third divergent Fake86 core.

## Supported baseline

The first supported release targets an IBM PC/XT-compatible 8086/8088 environment with 640 KiB conventional RAM and an internal BIOS. It is not an AT/286 emulator.

Required for release:

| Area | Required behaviour |
|---|---|
| CPU | 8086 instruction/flag/interrupt semantics pass an imported or generated conformance corpus; reset, HLT wake-up, and IRQ/NMI delivery are deterministic. V20 extensions, if retained, are an explicit machine option rather than an accidental default. |
| Memory | Conventional RAM, video memory, option-ROM window, BIOS ROM/reset vector, 20-bit wraparound, and read-only ROM writes behave consistently. The A20 gate and extended memory are out of scope for the XT baseline. |
| Interrupts/timing | 8259 PIC and 8253 PIT ports work; IRQ0 updates BIOS ticks at approximately 18.2065 Hz; IRQ1 is delivered for keyboard events; interrupt acknowledge, masking, EOI, and HLT wake-up work. Emulation uses elapsed emulated time, not a fixed number of host operations per video frame. |
| Keyboard | SDL keys translate to XT set-1 scan codes through a keyboard-controller/IRQ path; make/break, modifiers, lock keys, BIOS INT 16h blocking/non-blocking reads, and the BIOS data-area ring buffer work. No Primo mappings remain. |
| Video | 80x25 colour text mode, text VRAM, cursor position/shape, scrolling, teletype output, character/attribute writes, active page, and the BIOS data area agree. Unsupported graphics modes fail cleanly and are documented. |
| Floppy | At least 160/180/320/360/720/1200/1440 KiB raw images are geometry-detected or explicitly configured; reset/status/read/write/verify/parameters services return IBM-compatible carry/AH/AL results; DMA-boundary, CHS, short-image, missing-media, and read-only failures are nonfatal. |
| Hard disk | Raw PC/XT-compatible disks attach at `0x80`; CHS geometry is deterministic and overrideable; boot, read, write, verify, status, and parameter queries work. Decide explicitly whether this is BIOS-level storage or XT-IDE register emulation—do not expose half of each. |
| BIOS/platform | Reset/POST initializes the IVT and BIOS data area, equipment/memory queries are truthful, INT 10h/11h/12h/13h/16h/19h/1Ah cover the baseline, boot order and DL are correct, and an invalid guest request returns a BIOS error rather than aborting the host. |
| Frontend | Repeatable `--floppy`, `--hdd`, `--boot`, `--read-only`, `--headless`, and test-time timeout/exit controls exist; config paths use xemu conventions; drop-file and GUI media changes are optional follow-ups. |

Speaker, AdLib, Sound Blaster, networking, serial mouse, KVM, VGA graphics, and 286 protected mode are explicitly deferred. Their standalone Fake86 implementations may inform interfaces, but importing them is not a condition for the first release. The README must list these omissions so “supported” does not imply a complete late-DOS gaming PC.

## Implementation plan

### 1. Freeze and characterize both lineages

1. Record exact xemu and Fake86 commits, licenses, and file provenance.
2. Build both with sanitizers and capture a baseline using the same small boot sectors where possible.
3. Diff CPU behaviour semantically, separating portability/refactoring changes from opcode fixes. Add tests for every behavioural delta before choosing a winner.
4. Make xemu's `targets/pc/cpu.[ch]` the canonical integrated core unless upstream requests a shared library/subtree. Port proven Fake86 fixes into it as small, reviewable commits; do not bulk-copy the older core.
5. Add a reconciliation note to both READMEs (or deprecate the standalone fork in its README) naming the canonical CPU implementation and the synchronization policy.

### 2. Create testable machine/device boundaries

1. Split the current catch-all memory/I/O code into a machine bus plus focused PIC, PIT, keyboard, disk, BIOS, and video modules.
2. Give each device `init`, `reset`, port read/write, and time/IRQ interfaces with no SDL dependency. Keep SDL/config/rendering in `pc.c`.
3. Replace the frame-coupled `CPU_OPS_PER_SEC` loop with an emulated clock scheduler. Permit a deterministic unlimited/headless mode for tests.
4. Route all guest I/O through a table/range dispatcher. Unknown ports return the documented open-bus value with rate-limited diagnostics; they never flood logs or terminate the process.
5. Run unit tests for PIC priority/masking/EOI, PIT divisor/mode timing, keyboard scan queues, memory wraparound/ROM protection, and CHS/LBA bounds under ASan and UBSan.

### 3. Complete the internal BIOS and boot path

1. Consolidate the duplicated/obsolete BIOS paths in `targets/pc/bios.c`; one implementation owns each interrupt.
2. Initialize the IVT, BIOS data area, equipment word, memory size, disk counts, timer ticks, keyboard ring buffer, video fields, and drive-parameter tables from actual configured devices.
3. Implement the baseline BIOS interrupt matrix above with a shared convention for carry flag and status codes. Unknown functions return an appropriate error or “unsupported” result.
4. Make INT 19h follow configured boot order, validate reads rather than requiring a magic filename, and fall through from absent/unbootable floppy to hard disk. Preserve the BIOS boot-drive number in DL.
5. Provide configuration/CLI media attachment with bounds-checked I/O, writable only when requested, flush/error handling, stable geometry, and clean eject. Never modify a boot fixture in place during tests—copy it to a temporary directory first.
6. Decide the storage abstraction with upstream. For the first release, prefer complete BIOS-level INT 13h storage because it is already the more mature Fake86 path. Add XT-IDE only as a separate, tested phase if software requiring controller ports is an agreed release requirement.

### 4. Finish input and text display

1. Replace `primo_key_map` with a named PC/XT mapping and translate SDL events to set-1 make/break sequences.
2. Feed keyboard data through IRQ1 and the BIOS ring buffer; test modifier state, typematic policy, extended host keys, and focus loss (no stuck keys).
3. Make text rendering derive cursor/page/attribute state from emulated memory/device state rather than private frontend variables.
4. Complete the required INT 10h text calls and verify BIOS data-area values after mode changes, cursor movement, scrolling, page selection, and character output.
5. Add a headless text sink/test port so boot tests assert guest milestones without screenshot OCR. This interface must be compiled only for tests or documented as a stable debug facility.

### 5. Add boot-image and compatibility tests

Create `build/tests/xpc/` (or the location upstream prefers) with source-built fixtures; do not commit proprietary IBM/Microsoft BIOS or DOS images.

The fast suite builds tiny boot sectors with NASM and runs them headlessly with a hard timeout. Each fixture writes a unique result code/signature to the test channel and halts:

- CPU smoke plus an established 8086 instruction/flags corpus.
- Reset-vector, IVT, conventional-memory, equipment-word, and BIOS-data-area checks.
- PIT/IRQ0 cadence, PIC mask/EOI, HLT wake-up, and midnight rollover.
- Keyboard IRQ, make/break/modifier handling, INT 16h peek/read, and ring-buffer wrap.
- INT 10h mode, cursor, page, teletype, scroll, and attribute checks.
- Floppy boot and cross-track/multi-sector CHS reads for every supported geometry.
- Writable temporary floppy, read-only rejection, invalid CHS, DMA-boundary, absent-media, and truncated-image error cases.
- Hard-disk MBR boot, second-stage read, write/readback on a temporary copy, geometry query, bad-drive, and boot fallback.

Add one distributable open-source DOS integration image only after recording its source, license, reproducible build/download procedure, and SHA-256. FreeDOS is the preferred candidate; if its prebuilt image cannot be redistributed cleanly, download a pinned official image in the extended CI job or build a minimal image from source. The integration test must reach a scripted marker from `AUTOEXEC.BAT`, exercise keyboard input and filesystem reads, and exit through the test channel. Keep this extended test separate from the sub-minute fixture suite.

CI runs the fast headless suite on every relevant xemu change and the sanitizer/FreeDOS suite on pull requests or nightly. Failures retain the serial/test log and a screenshot or text-buffer dump. Every emulator invocation has a timeout and treats crashes, sanitizer findings, unknown fatal traps, and missing success markers as failures.

### 6. Upstream review and promotion

Submit the work as a reviewable series rather than one import:

1. test harness and CPU reconciliation;
2. scheduler/PIC/PIT;
3. keyboard;
4. BIOS and text video;
5. floppy/hard-disk media and boot tests;
6. documentation, normal-build promotion, and release metadata.

Ask LGB to confirm the canonical Fake86 lineage, the BIOS-level-versus-XT-IDE decision, and the exact maturity signal that permits distribution. Update `targets/pc/README.md` with the supported machine, CLI examples, ROM/media policy, known limitations, and test commands. Only upstream should remove its warning and add `pc` to `TARGETS`.

### 7. Package only after the upstream gate

After the accepted upstream commit is pinned in `foundry-apt/packages/xemu/`:

1. Re-run the omitted-target audit and record that `xpc` now satisfies its exception criteria.
2. Let the normal upstream build/install path produce `xpc`; do not carry a Foundry-only compile invocation.
3. Install `/usr/bin/xpc`, a man page, desktop entry, icon, and any AppStream metadata. Document that no BIOS or OS image is shipped.
4. Add `xpc` to the package description and update the suite's binary count, copyright/provenance, and installed-file assertions.
5. Build on Ubuntu 26.04, run lintian, smoke-install the `.deb`, run `xpc --help` headlessly, and execute the same fast boot suite against the installed binary.
6. Publish only after the `foundry-emulators-computers -> xemu` dependency chain resolves from the candidate repository and the installed-package boot tests pass.

## Release gates

All gates are mandatory:

- [ ] Exact Fake86 and xemu inputs, provenance, and CPU reconciliation decisions are documented.
- [ ] CPU conformance and all device unit tests pass under ASan/UBSan.
- [ ] Fast boot fixtures pass deterministically in headless CI with no fatal guest-triggerable paths.
- [ ] Floppy and hard-disk boot, read, error, and read-only/write tests pass.
- [ ] The pinned open-source DOS integration test reaches its scripted marker.
- [ ] README accurately states the PC/XT baseline and deferred devices.
- [ ] Upstream accepts the implementation and explicitly promotes `xpc` out of experimental status.
- [ ] `xpc` is built and installed by upstream's normal target list, not a packaging override.
- [ ] Debian package lint, installed-file checks, smoke install, and installed-binary boot tests pass.
- [ ] No proprietary BIOS, DOS, or other boot image enters the source or binary package.

If upstream accepts fixes but keeps the warning or continues omitting `pc` from `TARGETS`, Foundry may carry the tests for upstream development but must continue excluding `xpc` from the user package.

## Completion evidence

The implementation PR should attach:

- the provenance/reconciliation table and exact commits;
- CI links for unit, sanitizer, boot-sector, and open-source DOS jobs;
- logs showing floppy boot, hard-disk fallback, keyboard input, timer ticks, and clean read-only failure;
- the fixture/image license and SHA-256 record;
- upstream's promotion commit;
- `dpkg-deb -c`, lintian, clean-container install, and installed-binary boot-test output for the first package containing `xpc`.

