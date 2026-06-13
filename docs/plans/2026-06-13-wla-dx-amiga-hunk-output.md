# Investigation: Amiga Hunk executable output for wlalink (vhelin/wla-dx#589)

**Status: PARKED.** This is a scoped feasibility study + implementation sketch for an upstream wla-dx feature, not active work. Findings below are verified against the wla-dx source at tag `v10.6` (read via `gh api`, June 2026). Pick this up if/when we decide to invest in the upstream PR.

## Context

wla-dx already assembles 68000 code; the gap is that **wlalink only emits flat ROM/cartridge images** (plain binary, Mega Drive SMD/MD, Neo Geo, C64 PRG/CRT). It cannot produce an **AmigaOS Hunk executable** — the relocatable load-file format the AmigaDOS loader expects. Issue [#589](https://github.com/vhelin/wla-dx/issues/589) asks for exactly this. The format is documented in the [ABFS spec](https://web.archive.org/web/20201101014627/http://amiga.rules.no/abfs/abfs.pdf) we vendored at `foundry-apt/packages/wla-dx/docs/abfs.pdf`.

Goal if pursued: add a `-t AMIGAHUNK` linker output mode, land it upstream as a PR to vhelin/wla-dx, and carry it as a quilt patch in our Debian package until released.

## Why this is interesting but non-trivial

wla-dx is fundamentally an **absolute-address assembler**: the linker computes final addresses for everything and bakes them into one flat `g_rom` image. The Amiga Hunk format is the opposite philosophy — **position-independent**: each hunk is emitted as if based at address 0, plus a relocation table telling the loader which 32-bit words to fix up once it places the hunk in RAM. Bridging the two is the whole problem. The good news (verified below) is that WLA's internals expose exactly enough to do it for the common single-hunk case.

---

## Verified source facts (wla-dx v10.6)

These were confirmed by reading the actual source, and they correct several errors in an earlier draft of this doc:

1. **Final bytes live in `g_rom`, not in sections.** `write_rom_file()` (in `wlalink/write.c`) writes the binary by `fwrite(g_rom + g_bankaddress[i], …)`. `struct section`'s `data` array is only meaningful for BANKHEADER sections — after linking, all real content is composited into `g_rom`. **A hunk writer must read `g_rom + section->output_address` for `section->size` bytes.**

2. **Relocation values are loader-ready when the hunk is based at 0.** In `fix_references()`, a `REFERENCE_TYPE_DIRECT_32BIT` reference writes `i = (int)l->address` (the resolved label address) big-endian into `g_rom`. If the referenced section is mapped at base 0 (`.ORG 0`, slot address `$0`), that 32-bit value equals the **hunk-relative offset** — precisely the pre-relocation value the Amiga loader needs. The reference list is **still intact** at `write_rom_file()` time (not freed), so the writer can walk it to build the RELOC32 table.

3. **There is no `[output]` linkfile directive.** The linkfile parser (`wlalink/files.c`) recognizes only: `[objects]`, `[libraries]`, `[header]`, `[footer]`, `[definitions]`, `[ramsections]`, `[sections]`, `[sectionwriteorder]`, `[ramsectionwriteorder]`. **Output format is selected by command-line flag only** (`-t CBMPRG|C64CRT`, `-O BIN|SMD|MD`).

4. **A big-endian longword writer already exists** — `static void _write_u32be(FILE *f, int value)` in `write.c`. Reuse it; do **not** add a new helper, and do **not** use `uint32_t` (the codebase predates `<stdint.h>` — it's `int`/`unsigned char` throughout).

5. **`OUTPUT_TYPE_*` constants are in `wlalink/defines.h`** (`UNDEFINED=0, CBM_PRG=1, C64_CRT=2`). `ROMFORMAT_*` are in `shared.h`. The C64-CRT path is the right architectural model (see below).

## Hunk format — magic numbers (corrected)

All 32-bit **big-endian** longwords. (Earlier draft mis-stated HUNK_SYMBOL as `0x2F0C/12108`; correct value is `0x3F0/1008`.)

| Constant | Dec | Hex | Needed for minimal exe? |
|---|---|---|---|
| HUNK_HEADER | 1011 | 0x3F3 | ✅ yes |
| HUNK_CODE | 1001 | 0x3E9 | ✅ yes |
| HUNK_DATA | 1002 | 0x3EA | for data sections |
| HUNK_BSS | 1003 | 0x3EB | for bss sections |
| HUNK_RELOC32 | 1004 | 0x3EC | ✅ when refs exist |
| HUNK_END | 1010 | 0x3F2 | ✅ yes |
| HUNK_EXT | 1007 | 0x3EF | no (libs only) |
| HUNK_SYMBOL | 1008 | 0x3F0 | no (debug only) |

**Minimal load-file layout:**
```
HUNK_HEADER
  0                  (no resident library names)
  0                  (name-list end marker)
  N                  (table size = hunk count)
  0                  (first hunk index)
  N-1                (last hunk index)
  size_0 … size_{N-1}  (each hunk size in LONGWORDS)
[for each hunk i:]
  <HUNK_CODE|HUNK_DATA|HUNK_BSS>
  size_i             (in longwords; BSS stops here — no payload)
  <4*size_i bytes of code/data>          (omitted for BSS)
  [HUNK_RELOC32 block, if this hunk has 32-bit relocations]
  HUNK_END
```
**HUNK_RELOC32 block:** repeat `{ count, target_hunk, offset_0 … offset_{count-1} }` until a terminating `count = 0`.

---

## Recommended approach: new `OUTPUT_TYPE_AMIGA_HUNK` via `-t`

Mirror the **C64 CRT** precedent, not the Mega Drive `-O` precedent. Rationale: `-O`/ROMFORMAT variants (`_write_megadrive_formatted_rom`) operate on an *already-flattened* image — byte-reorderings of a finished ROM. Hunk output needs **section boundaries and the live reference list**, which only exist at the early dispatch point where `_write_c64_crt_file()` is called. So Hunk belongs alongside C64 CRT as an `OUTPUT_TYPE`, dispatched before flattening.

### Files to change (all in upstream tree; carried as `debian/patches/0001-amiga-hunk-output.patch`)

| File | Change |
|---|---|
| `wlalink/defines.h` | `#define OUTPUT_TYPE_AMIGA_HUNK 3` |
| `wlalink/main.c` | In the `-t` parser (next to `CBMPRG`/`C64CRT`): `else if (!strcmp(flags[count+1], "AMIGAHUNK")) g_output_type = OUTPUT_TYPE_AMIGA_HUNK;` |
| `wlalink/write.c` | New `_write_amiga_hunk_file(FILE *f, char *outname)`; dispatch hook in `write_rom_file()` mirroring the `OUTPUT_TYPE_C64_CRT` branch |
| `doc/wlalink.rst` (+ `doc/linking.rst`) | Document `-t AMIGAHUNK` |
| `tests/68000/amiga_hunk_test/` | New test (see below) |

### Dispatch hook (in `write_rom_file()`, beside the C64 CRT branch)
```c
if (g_output_type == OUTPUT_TYPE_AMIGA_HUNK) {
  int rv = _write_amiga_hunk_file(f, outname);
  fclose(f);
  return rv;
}
```

### `_write_amiga_hunk_file()` sketch (reusing `_write_u32be`)
- **Enumerate hunks:** walk `g_sec_first`, skip BANKHEADER sections, collect `{name, output_address, size}` into an array; assign hunk index 0..N-1. Classify each by name prefix → CODE / DATA / BSS (default CODE).
- **Header:** `_write_u32be` the HUNK_HEADER preamble + per-hunk longword sizes `(size+3)/4`.
- **Body per hunk:** type word, longword size, then `fwrite(g_rom + sec->output_address, 1, sec->size, f)` (pad to longword with zeros); BSS writes no payload.
- **Relocations per hunk:** walk the reference list; select `REFERENCE_TYPE_DIRECT_32BIT` refs whose patch address falls inside this hunk; group by the target hunk that contains the referenced label; emit the HUNK_RELOC32 block. Offset-within-hunk = `ref_address − sec->output_address`.
- **Terminate** each hunk with HUNK_END.

### Test `tests/68000/amiga_hunk_test/`
Minimal, correct AmigaOS-style stub (loader calls your code as a subroutine; return via `rts`, **not** `trap #0`):
```asm
.MEMORYMAP
  DEFAULTSLOT 0
  SLOTSIZE $10000
  SLOT 0 $00000000
.ENDME
.ROMBANKMAP
  BANKSTOTAL 1
  BANKSIZE $10000
  BANKS 1
.ENDRO
.BANK 0 SLOT 0
.ORG 0
  moveq #0, d0      ; return code 0
  rts               ; AmigaOS: return to caller
```
Linker invoked as `wlalink -b linkfile main.out -t AMIGAHUNK` (format is a **flag**, not a linkfile line). Assert the first four output bytes are `00 00 03 F3`. **Open question:** confirm how existing tests assert on binary output — most compare against a committed reference `.bin` via the `testsfile`/`makefile` mechanism; follow whichever pattern `tests/68000/rom_test` uses rather than inventing a shell check.

---

## Open risks to resolve before implementing

1. **Multi-hunk relocation correctness (the hard part).** Fact #2 only guarantees loader-ready values when a hunk is based at **0**. WLA's slot/bank model gives each section a *distinct, generally nonzero* base in one address space, so two hunks can't both sit at 0. For a multi-hunk program the writer must treat each hunk as base-0 and subtract the target hunk's base from each baked value before emitting it — or constrain the supported layout. **Minimal viable scope = a single CODE hunk at `.ORG 0`**, where this problem disappears. Multi-hunk (code + data + bss with cross-hunk refs) is a materially larger effort and should be a follow-up.
2. **Two relocation sources.** WLA resolves fixups via *both* `fix_references()` (simple references) and `compute_pending_calculations()` (pending calcs). A correct RELOC32 table may need entries from **both** paths, not just the reference list. Audit both before trusting the table.
3. **Is `l->address` truly section-relative or absolute?** Fact #2's operational conclusion (base-0 ⇒ value == hunk offset) holds either way, but the multi-hunk math depends on knowing exactly what `l->address` contains. Confirm by single-stepping a 2-section example before generalizing.
4. **Test-harness assertion mechanism** (see test note above).

## Effort estimate

- **Minimal viable** (single CODE hunk, `.ORG 0`, RELOC32 from the reference list, one test): ~1 focused day. Produces a loadable, relocatable AmigaOS executable for simple programs and is a credible upstream PR.
- **Full** (CODE/DATA/BSS, correct cross-hunk relocation, pending-calc coverage, HUNK_SYMBOL debug option): several days; depends on resolving risks 1–3.

## Development workflow (when picked up)

1. Extract source:
   ```bash
   curl -fsSL https://github.com/vhelin/wla-dx/archive/refs/tags/v10.6.tar.gz | tar xz
   ```
2. Edit the C files; build with `cmake .. && make -j$(nproc)`.
3. Verify by hand: `wla-68000 -o main.o main.s && wlalink -b linkfile main.out -t AMIGAHUNK`, then `xxd main.out | head -1` shows `0000 03f3 …` and HUNK_END (`0000 03f2`) markers are present.
4. Add `tests/68000/amiga_hunk_test/`; `make test` must pass.
5. `git diff > 0001-amiga-hunk-output.patch`; add to `debian/patches/series`; confirm it applies under `task build`.
6. Open the PR: `gh pr create --repo vhelin/wla-dx`, referencing issue #589.
