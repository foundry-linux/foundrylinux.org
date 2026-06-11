# Package WLA-DX as a .deb for foundry-apt

**Status:** Complete — shipped as [v1.5.16](https://github.com/foundry-linux/foundry-apt/actions)  
**Tracking:** [wla-dx in TODO.md](../../TODO.md) — Packaging — dropped packages to investigate

## Context

WLA-DX is the SNES-family assembler toolchain used by PVSnesLib (65816 + SPC-700 + 17 other
CPUs). Absent from Debian/Ubuntu. Once packaged, it wires into `foundry-retro-tools` so
`apt install foundry-retro-tools` pulls it automatically.

## Decisions

- **Version**: v10.6 (latest tagged release; HEAD is 191 commits ahead — ez80, cx4, sh2
  added post-tag; bump to v10.7 when tagged)
- **sha256**: `010c4d426fd1733b978cbca7530a5e68bdfb6f62976c0d5ff7bff447894e19a8`
- **Build system**: pure cmake, no external deps beyond libc/libm
- **Debian version**: `10.6-1foundry1`
- **Man pages**: 18 total — `wla-6502.1` canonical (full options), 15 other `wla-*` get
  individual short pages with arch description + cross-reference; `wlalink.1` and `wlab.1`
  hand-written from source
- **No upstream patches needed** (clean cmake build, no legacy Makefile/configure.ac)
- **foundry-retro-tools bump**: 1.0.10 → 1.0.11

## Binaries (18 total — v10.6)

All installed to `/usr/bin/`. cx4/ez80/sh2 added in HEAD post-v10.6; will appear in v10.7.

| Binary | CPU |
|---|---|
| `wla-6502` | MOS 6502 |
| `wla-65c02` | WDC 65C02 |
| `wla-65ce02` | CSG 65CE02 |
| `wla-65816` | WDC 65816 (SNES) |
| `wla-spc700` | SPC700 (SNES DSP) |
| `wla-gb` | Game Boy (Sharp SM83) |
| `wla-huc6280` | HuC6280 (PC Engine) |
| `wla-z80` | Zilog Z80 |
| `wla-z80n` | Z80N (ZX Spectrum Next) |
| `wla-68000` | Motorola 68000 |
| `wla-6800` | Motorola 6800 |
| `wla-6801` | Motorola 6801 |
| `wla-6809` | Motorola 6809 |
| `wla-8008` | Intel 8008 |
| `wla-8080` | Intel 8080 |
| `wla-superfx` | SuperFX (SNES) |
| `wlalink` | WLA linker |
| `wlab` | WLA batch assembler |

## cmake flags

```cmake
cmake -B build \
    -D CMAKE_BUILD_TYPE=Release \
    -D CMAKE_INSTALL_PREFIX=/usr \
    -D GEN_DOC=OFF \
    -D STRICT_ANSI_WARNINGS=OFF
cmake --build build -j$(nproc)
cmake --install build --prefix /usr --destdir $(CURDIR)/debian/wla-dx
```

`GEN_DOC=OFF` — sphinx man pages exist upstream as RST but require `sphinx-build` and a
working network to fetch themes; hand-written troff `.1` files are simpler and more
reliable in CI. `STRICT_ANSI_WARNINGS=OFF` — suppresses `-Werror` on ANSI C89 warnings
that may fire with GCC 15.

**cmake version range syntax** — `cmake_minimum_required(VERSION 2.8.12...3.5)` is valid
range notation (policy floor); modern cmake handles it correctly, no override needed.

**Table generators** — cmake builds CPU-table generator binaries first, runs them to produce
lookup tables, then compiles the actual assemblers. Handled internally by cmake; no special
debhelper override needed.

**DESTDIR** — `cmake --install build --destdir` is standard cmake 3.15+ behaviour and works
correctly. Use `override_dh_auto_install` with the explicit form above rather than relying
on `dh_auto_install`'s heuristic, since the build dir is `build/` not the debhelper default.

## Files

```
foundry-apt/packages/wla-dx/
├── build.sh
└── debian/
    ├── control, changelog, copyright, rules
    ├── source/format (3.0 quilt), watch, patches/series
    ├── wla-dx.manpages
    └── man/  (18 × .1 files)
        wla-6502.1  (full options page — all wla-* share the same flags)
        wla-65c02.1 … wla-superfx.1  (15 × short pages: arch desc + SEE ALSO wla-6502(1))
        wlalink.1   (linker options from --help)
        wlab.1      (batch assembler from --help)
```

## debian/control (as shipped)

```
Source: wla-dx
Section: devel
Architecture: any
Build-Depends: debhelper-compat (= 13), cmake
Depends: libc6 (>= 2.34)
Homepage: https://github.com/vhelin/wla-dx
Description: multi-platform cross-assembler package (16 CPU architectures)
 WLA DX is a set of tools for assembling source code to object files
 (wla-ARCH) and linking them together (wlalink). Each wla-ARCH binary
 targets one specific CPU: 6502/65C02/65CE02/65816, SPC700, SuperFX,
 6800/6801/6809, 68000, Z80/Z80N/GB, 8008/8080, HuC6280.
 Includes wlalink (linker) and wlab (binary-to-WLA-DB converter).
```

## Steps

1. ~~Universe check~~ — not in 26.04 universe. ✅
2. ~~Pin tarball sha256~~ — `010c4d42…`. ✅
3. ~~Author the `debian/` tree~~ — 16 CPUs (v10.6); cx4/ez80/sh2 post-date the tag. ✅
4. ~~Build in `ubuntu:26.04` Docker + lintian clean~~ (zero E/W). ✅
5. ~~Wire into `foundry-retro-tools`~~ — changelog 1.0.11, control Depends + description. ✅
6. ~~Local dep-chain smoke test~~ — `dpkg-deb -I` + fresh-container install. ✅
7. ~~Commit + tag~~ — [v1.5.16](https://github.com/foundry-linux/foundry-apt/actions) tagged and CI publishing. ✅

## Verification

1. `bash build.sh` in `ubuntu:26.04` produces `dist/wla-dx_10.6-1foundry1_amd64.deb`.

```
OK   /repo/foundry-apt/dist/wla-dx_10.6-1foundry1_amd64.deb  (923226 bytes)
```

PASS

2. `lintian dist/wla-dx_*.deb` → zero E:/W: lines.

```
(no output)
```

PASS

3. `dpkg-deb -c` lists all 18 files under `usr/bin/` (16 assemblers + wlalink + wlab)
   and 18 man pages under `usr/share/man/man1/`.

```
/usr/bin/wla-6502, /usr/bin/wla-65816, /usr/bin/wla-65c02, /usr/bin/wla-65ce02,
/usr/bin/wla-6800, /usr/bin/wla-68000, /usr/bin/wla-6801, /usr/bin/wla-6809,
/usr/bin/wla-8008, /usr/bin/wla-8080, /usr/bin/wla-gb, /usr/bin/wla-huc6280,
/usr/bin/wla-spc700, /usr/bin/wla-superfx, /usr/bin/wla-z80, /usr/bin/wla-z80n,
/usr/bin/wlab, /usr/bin/wlalink
18 man1/*.gz files
```

PASS

4. `dpkg-deb -I` shows `${shlibs:Depends}` resolved (libc6); no libm (static in glibc on
   modern systems).

```
 Depends: libc6 (>= 2.34)
```

PASS

5. `readelf -h usr/bin/wla-65816` → PIE executable.

```
  Type:    DYN (Position-Independent Executable file)
  Machine: Advanced Micro Devices X86-64
```

PASS

6. Fresh-container `apt install ./wla-dx_*.deb`; `wla-65816` prints usage, `wlalink --help`
   exits 0, `man wlalink` renders.

```
--- WLA W65816 Macro Assembler v10.6 ---
--- WLALINK - WLA DX Macro Assembler Linker v5.21 ---
```

PASS

7. `apt-cache depends foundry-retro-tools` → `Depends: wla-dx`.

```
 Depends: mame, mame-tools, dasm, cc65, 64tass, z80dasm, z80asm, radare2, binwalk,
          sox, binutils-m68k-linux-gnu, xa65, f9dasm, libvgm, vgmstream, m8te,
          tilemap-studio, wla-dx
```

PASS

## When v10.7 is tagged

1. Update `UPSTREAM_VERSION` + `SHA256` in `build.sh`.
2. Add `debian/man/wla-cx4.1`, `wla-ez80.1`, `wla-sh2.1` (short pages, SEE ALSO `wla-6502(1)`).
3. Add those 3 entries to `debian/wla-dx.manpages`.
4. Update `debian/control`: "16 CPU architectures" → "19 CPU architectures"; add cx4/ez80/sh2 lines.
5. Add `debian/changelog` entry.
6. `task bump`.
