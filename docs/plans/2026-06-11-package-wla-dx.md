# Package WLA-DX as a .deb for foundry-apt

**Status:** In progress  
**Tracking:** [wla-dx in TODO.md](../../TODO.md) — Packaging — dropped packages to investigate

## Context

WLA-DX is the SNES-family assembler toolchain used by PVSnesLib (65816 + SPC-700 + 17 other
CPUs). Absent from Debian/Ubuntu. Once packaged, it wires into `foundry-retro-tools` so
`apt install foundry-retro-tools` pulls it automatically.

## Decisions

- **Version**: v10.6 (latest tagged release; v10.7 exists in HEAD but untagged)
- **sha256**: `010c4d426fd1733b978cbca7530a5e68bdfb6f62976c0d5ff7bff447894e19a8`
- **Build system**: pure cmake, no external deps beyond libc/libm
- **Debian version**: `10.6-1foundry1`
- **Man pages**: 21 total — `wla-6502.1` canonical (full options), 18 other `wla-*` get
  individual short pages with arch description + cross-reference; `wlalink.1` and `wlab.1`
  hand-written from source
- **No upstream patches expected** (clean cmake build, no legacy Makefile/configure.ac)
- **foundry-retro-tools bump**: 1.0.10 → 1.0.11

## Files

```
foundry-apt/packages/wla-dx/
├── build.sh
└── debian/
    ├── control, changelog, copyright, rules
    ├── source/format (3.0 quilt), watch, patches/series
    ├── wla-dx.manpages
    └── man/  (21 × .1 files)
```

## Verification steps

1. `lintian dist/wla-dx_*.deb` → zero E:/W: lines
2. `file debian/wla-dx/usr/bin/wla-6502` → "stripped"
3. `dpkg-deb -I dist/wla-dx_*.deb` → `Depends:` has resolved libc6/libm
4. Smoke install in `ubuntu:26.04`: `wla-65816` prints usage; `wlalink --help` works;
   `man wlalink` and `man wla-65816` render correctly
5. `apt install foundry-retro-tools` in clean container with both repos wired resolves
   wla-dx successfully
