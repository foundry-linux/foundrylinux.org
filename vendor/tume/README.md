# tUME — the Universal Map Editor (vendored source)

Vendored, latest-and-final source release of **tUME (the Universal Map Editor)** by **Echidna**.

| | |
|---|---|
| **Version** | "Open tUME" source release, **2000-06-18** (the final version — see *Why this is "latest"* below) |
| **License** | **Mozilla Public License 1.1** (file-level weak copyleft) — see [`LICENSE`](LICENSE); every source file carries the MPL header |
| **Upstream** | Echidna — Greg Marquez, John Alvarado, Gregg Tavares ("greggman"). Copyright © 1989–2000 Echidna |
| **Archive** | [`tumesrc.zip`](tumesrc.zip) — sha256 `22c9eb2b7c9229ea62f31893b9b85bc6b2f9b0c48bb4ad888bafed335f83c4ce` |
| **Extracted** | [`src/`](src/) — 179 files: 59 `.c`, 70 `.h`, 8 real-mode `.asm`, `MAKEFILE` + DOS batch build scripts |

## Provenance

The original host `members.aol.com/opentume` (AOL Hometown) is **dead** — shut down 2008. The only
surviving copy is the **Wayback Machine**, from which this was recovered on **2026-06-10**.

- Source archive: `https://web.archive.org/web/20070202224928id_/http://members.aol.com/opentume/download/tumesrc.zip`
- Open tUME landing page: `https://web.archive.org/web/20050421032126id_/http://members.aol.com/opentume/`
- greggman canonical page: `https://web.archive.org/web/20100312033130id_/http://games.greggman.com/game/tume___the_universal_map_editor/`

To re-fetch, find a `200` snapshot via the CDX API, then pull the `id_` (raw) variant:
```sh
curl -s "https://web.archive.org/cdx/search/cdx?url=members.aol.com/opentume/download/tumesrc.zip&filter=statuscode:200&output=text"
curl -sL "https://web.archive.org/web/20070202224928id_/http://members.aol.com/opentume/download/tumesrc.zip" -o tumesrc.zip
echo "22c9eb2b7c9229ea62f31893b9b85bc6b2f9b0c48bb4ad888bafed335f83c4ce  tumesrc.zip" | sha256sum -c
```

### Why this *is* the latest version

tUME development ended in the 1990s; the **2000-06-18 MPL source drop was the one and only
open-source release**. Wayback holds a single non-redirect (`200`) capture of `tumesrc.zip`
(digest `PVZOPK…`, 575 987 B, internal file dates 2000-06-18); all later captures are post-shutdown
`301`s. There is no newer source to find.

Sibling artifacts (not vendored here, recoverable from the same Wayback path) include the DOS binary
`tumeexe.zip`, the tUMEPack/utility *source* (`tp16.zip`, `tpbin.zip`, `dumptume.zip`), and
`tumedemo/tumedocs/samples.zip`.

## Build reality (from `src/BUILD.TXT`)

This is **16-bit DOS code** and does **not** build on a modern Linux toolchain as-is:

- Requires **Borland C Compiler 3.1** + **Borland Turbo Assembler 3.1** (real mode).
- Depends on Echidna's bundled **EGGS** (Echidna Game Generation System) libraries — *not* the
  current version. The modern descendant is [github.com/greggman/elibs](https://github.com/greggman/elibs)
  (New BSD), but the source notes tUME "has NOT yet been updated to work with the most recent EGGS."
- 8 real-mode `.asm` files + near/far pointer assumptions + a DOS graphics/input layer.

A modern Linux port means replacing the asm + EGGS DOS layer (e.g. with SDL) and resolving the 16-bit
memory model. MPL-1.1 permits this (keep the header on modified files). Until then, the authentic
binary runs under **DOSBox**. See the analysis in
[`docs/investigations/2026-06-10-tume-map-editor-port-and-successors.md`](../../docs/investigations/2026-06-10-tume-map-editor-port-and-successors.md).

> **Status**: source preserved & vendored, *not* yet a buildable `.deb`. It is deliberately **not**
> under `foundry-apt/packages/` — that tree is for things that build to a `.deb` on Ubuntu 26.04, and
> tUME does not (yet). Promote it there only once a port produces a Linux binary.
