# ITP draft: mesen2

## wnpp check

Searched wnpp for "mesen2" / "mesen" (multi-system emulator, SourMesen). **No existing wnpp bug found** — results were all upstream GitHub/forum links (SourMesen/Mesen2, nesdev.org threads), nothing on bugs.debian.org.

## ITP draft

```
Package name    : mesen2
Version         : 2.1.1
Upstream contact: Sour <https://github.com/SourMesen/Mesen2>
URL             : https://github.com/SourMesen/Mesen2
Licence         : GPL-3.0
Programming lang: C# (.NET) with a native C++ core
Description     : high-accuracy multi-system console/handheld emulator
```

Mesen is a cross-platform, high-accuracy emulator covering NES/Famicom
(incl. FDS, Dendy), Super NES/Super Famicom, Game Boy/Game Boy Color,
Game Boy Advance, PC Engine/TurboGrafx-16 (incl. SuperGrafx, CD), Master
System/Game Gear/SG-1000, and WonderSwan/WonderSwan Color. It focuses on
cycle accuracy and ships an extensive debugging/development suite
(debugger, assembler, tile/sprite/memory viewers, event viewer, trace
logger, Lua scripting) — making it well suited to homebrew and
reverse-engineering work in addition to general play.

This would be a strong Debian addition: it's one of the most accurate
multi-system emulators available, actively maintained, GPL-3.0
licensed, and Debian currently has no comparably broad
accuracy-focused multi-system emulator (its closest existing packaged
peers cover single systems each).

## Packaging obstacle — this is not a from-source build

**Our package repackages upstream's self-contained Linux x64 build — a
.NET ahead-of-time-compiled (AOT) single-file binary**, per
`foundry-apt/packages/mesen2/build.sh` and `debian/control`'s own
description: "This package repackages the upstream self-contained Linux
x64 build (a .NET ahead-of-time-compiled single-file binary)." This is
the same class of obstacle as `ghidra`, `ldtk`, and `pvsneslib` in this
batch: Debian main requires building from source, and an AOT single-file
binary is definitionally a shipped, pre-compiled artifact, not a source
build. A real Debian ITP would need to build Mesen2's C#/.NET + native
C++ core from source using Debian's `dotnet` packaging (which exists but
has historically been thin/unstable in Debian main, especially for AOT
compilation support) — a substantially larger undertaking than the
current repack-only approach.

## Verdict

High-value target if the from-source build problem can be solved (real,
broad demand; clean GPL-3.0 license), but **not currently
Debian-ITP-ready** given the AOT-binary packaging shortcut we use today.
Flag alongside `ghidra`/`ldtk`/`pvsneslib` as "needs a dedicated
from-source packaging effort before filing," not a quick win.
