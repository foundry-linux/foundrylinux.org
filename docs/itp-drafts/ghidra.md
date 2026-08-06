# ITP draft: ghidra

## Upstream packaging audit

Audited the pinned upstream payload on 2026-08-05 with
`foundry-apt/scripts/audit-upstream-packaging.sh`. **No upstream packaging signals found:** no
upstream-maintained `debian/`, `.deb`/`.dsc` builder, `PKGBUILD`/`.spec`, or PPA/Launchpad/OBS/COPR
packaging reference.

## wnpp check — EXISTING BUGS FOUND, prominent flag

Two prior wnpp bugs exist for ghidra, both filed as **ITP** and later
**retitled to RFP** (i.e. the original filer gave up / went inactive and
no one is actively packaging it):

- [#923851](https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=923851) — originally "ITP: ghidra -- Ghidra is a software reverse engineering framework", now "RFP: ghidra -- software reverse engineering framework"
- [#973309](https://bugs.debian.org/973309) — same pattern: originally ITP, now "RFP: ghidra -- a reverse engineering framework"

**Do not file a fresh ITP without checking both bugs' current state first.** The correct Debian process here is to comment on one of the existing bugs (most likely the more recent, #973309) offering to take over packaging and asking the BTS to retitle it back to ITP with a new owner, rather than opening bug #3 for the same package — Debian etiquette strongly discourages duplicate wnpp bugs. Whoever picks this up should also check whether "Yangfl" (recorded as an owner in search results) is still active before assuming the slot is free.

## ITP draft (for reference / for retitling an existing bug)

```
Package name    : ghidra
Version         : 12.1
Upstream contact: NSA Research Directorate <https://github.com/NationalSecurityAgency/ghidra/issues>
URL             : https://ghidra-sre.org/
Licence         : Apache-2.0 (core); numerous bundled third-party components
                  under their own licenses (see /usr/lib/ghidra/licenses/
                  in the built package)
Programming lang: Java (decompiler in C++)
Description     : NSA software reverse engineering framework
```

Ghidra is a free and open-source software reverse engineering (SRE)
framework developed by the NSA Research Directorate, supporting analysis
of compiled code across Windows, macOS, and Linux targets. Features
include disassembly, assembly, decompilation, graphing, and scripting,
with hundreds of built-in analysers for ELF, PE, Mach-O, raw binaries,
and cartridge formats, plus a plugin API for custom analysers in
Java or Python (Jython/PyGhidra).

## Packaging obstacles — this is not a simple case

Both prior ITP→RFP demotions are almost certainly explained by these
obstacles, all visible directly in our own `foundry-apt/packages/ghidra/`:

1. **Pre-built binaries, not built from source.** Our package (and every
   upstream release) ships the official pre-built Linux zip. The native
   decompiler (`support/decompile`) is a **pre-compiled x86_64 ELF
   binary** — Debian main categorically requires building from source;
   shipping a prebuilt binary blob is disqualifying as-is. A real Debian
   package would need to build the C++ decompiler and the Java/Gradle
   tree from source, which Ghidra's upstream build system supports but
   is a substantial undertaking (Gradle, a large Java codebase, native
   JNI glue).
2. **~860 MiB of bundled jars.** Our `LICENSES-VENDORED.md` already
   tracks Ghidra as one of our heaviest vendored packages. A from-source
   Debian build would need every one of Ghidra's third-party Java
   dependencies either already packaged in Debian or newly packaged
   themselves — a large transitive dependency chain (Flat Laf, various
   Apache Commons libs, ANTLR, and more), each needing its own
   DFSG/licensing check.
3. **Architecture restriction.** Our own `debian/control` restricts to
   `Architecture: amd64` because of the prebuilt native decompiler —
   any real Debian package would need per-architecture decompiler
   builds (Debian supports far more than amd64).
4. **openjdk-21-jdk dependency** — fine, already in Debian, not itself
   an obstacle.

## Verdict

Legitimate long-term ITP target (real demand: two independent people
already tried), but **not a T1-shippable ITP** — it requires an actual
from-source build effort, likely spanning weeks and multiple sub-ITPs
for missing Java dependencies, before a bug can responsibly be filed (or
re-claimed). Recommend treating "package Ghidra from source for Debian"
as its own multi-week project, separate from the rest of this batch,
if pursued at all.
