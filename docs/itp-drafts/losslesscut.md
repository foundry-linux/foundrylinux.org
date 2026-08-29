# ITP draft: losslesscut

## Upstream packaging audit

Audited the pinned upstream payload on 2026‑08‑29 as part of the `/package` run (Step 2.5).
**No upstream packaging signals found:** no upstream‑maintained `debian/` directory, no
`.deb`/`.dsc` builder, no `PKGBUILD`/`.spec`, and no PPA / Launchpad / OBS / COPR reference in
the README or `docs/`.

Upstream's own Linux distribution channels are:

- `LosslessCut-linux-x86_64.AppImage` and `LosslessCut-linux-x64.tar.bz2` on GitHub Releases
- [Snapcraft](https://snapcraft.io/losslesscut) (`losslesscut`)
- [Flathub](https://flathub.org/apps/details/no.mifi.losslesscut) (`no.mifi.losslesscut`)
- Mac App Store and Microsoft Store (paid, as a way of supporting the author)

There is therefore **no upstream‑preferred Debian package name to defer to**, and no risk of
duplicating a packaging effort upstream already has under way. Our binary package name
`losslesscut` matches upstream's own executable name, Snap name and `StartupWMClass`, so the
naming needs no disambiguation. The Repology project slug is **`lossless-cut`** (not
`losslesscut`, which returns nothing), recorded in `X-Repology-Project`.

## wnpp check — CLEAR, no existing bug

Searched the wnpp bug list on 2026‑08‑29 for both `losslesscut` and `lossless-cut`: **zero
matches**, in any of ITP / RFP / RFA / ITA / O. `sources.debian.org` likewise reports no
source package by that name in any suite.

So this would be a **fresh ITP**, not a retitle of an existing bug — unlike `ghidra`, where two
prior ITP→RFP demotions exist. Re‑check immediately before filing; the search is cheap and the
situation can change.

## ITP draft

```
Package name    : losslesscut
Version         : 3.69.0
Upstream contact: Mikael Finstad <finstaden@gmail.com>
URL             : https://losslesscut.app/
Licence         : GPL-2.0-only (LosslessCut itself)
                  Expat (bundled Electron runtime)
                  BSD-3-clause and others (bundled Chromium content runtime)
                  GPL-3.0+ (bundled FFmpeg 8.0, built --enable-gpl --enable-version3)
Programming lang: TypeScript / JavaScript (Electron); FFmpeg in C
Description     : swiss army knife of lossless video and audio editing
```

LosslessCut trims, cuts, merges and remuxes video and audio files without re‑encoding them.
It performs an almost direct stream copy, so an edit completes in seconds regardless of file
size and the output keeps the exact quality of the source. It is built for quickly pulling the
good parts out of large recordings from a camera, drone, screen capture or gameplay session and
discarding the rest, avoiding both the time cost and the generation loss of a re‑encode.
Alongside trimming it can merge files, extract or delete individual streams, change container
formats, edit track and file metadata, take full‑resolution still snapshots, apply lossless
rotation and cropping through container metadata, and import or export segment lists in CSV,
EDL, CUE and XML.

## Packaging obstacles — a genuinely hard case for Debian main

Our Foundry package repacks upstream's **pre‑built** application bundle. That is acceptable in
our own archive but is disqualifying for Debian main as it stands. Anyone taking this to Debian
faces three obstacles, in increasing order of difficulty:

1. **Pre‑built binary payload.** We ship upstream's published Electron build. Debian main
   requires building from source. A real Debian package would have to build the app from the
   TypeScript sources with `electron-vite`.

2. **Electron is not in Debian.** This is the blocker. Debian has repeatedly declined to carry
   Electron as a shared runtime because of its Chromium‑scale security‑support burden, and
   Chromium itself is a major maintenance commitment. Every Electron application therefore
   either embeds its own ~200 MB runtime (unacceptable duplication) or has nothing to link
   against. Until Debian has a supported Electron story, an Electron GUI app has no clean route
   into main. This is a distribution‑level problem, not something this package can solve.

3. **The npm dependency tree.** LosslessCut's `yarn.lock` pulls a very large transitive
   JavaScript dependency set, essentially none of which is packaged in Debian. Debian's
   `node-*` packaging effort would need to absorb the whole tree.

4. **Bundled FFmpeg, and a licence wrinkle worth flagging.** Upstream bundles its own FFmpeg
   8.0 built `--enable-gpl --enable-version3`, making those binaries **GPL‑3.0+**, while
   LosslessCut's own code is **GPL‑2.0‑only**. Debian would want the package to use the system
   FFmpeg, which sidesteps the wrinkle — but LosslessCut resolves `ffmpeg`/`ffprobe` relative
   to its own resources directory by default, so that needs a patch. (Upstream does expose an
   `LLC_CUSTOM_FFMPEG_PATH` escape hatch, so the change is small.) The combination as shipped
   is upstream's own published arrangement, not one introduced by our packaging.

Realistically this is an **RFP‑shaped** situation rather than an ITP one until Debian's Electron
position changes. Consider filing an RFP that documents obstacle 2 plainly, rather than an ITP
that cannot be discharged.

## Upstream heads‑up

Status: **not yet sent.**

Points to raise when it is:

- We package LosslessCut for Foundry Linux (`apt.foundrylinux.org`) by repacking the official
  AppImage payload; say so plainly, since it is their build we are redistributing.
- Upstream already tags releases cleanly (`v3.69.0`), so the usual "please tag releases" ask
  does not apply — worth noting as a positive.
- Offer to list them as upstream contact and to forward any Debian/Foundry bugs.
- Note that we retain their bundled FFmpeg rather than substituting the system one, so
  behaviour matches their own builds exactly.
- No naming disambiguation is needed here, so nothing to hand them that isn't theirs to decide.

Record the reply — or the absence of one — here once sent.
