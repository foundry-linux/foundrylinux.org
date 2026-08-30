# Package LosslessCut as a .deb for apt.foundrylinux.org

**Date:** 2026‑08‑29
**Package:** `losslesscut` 3.69.0‑1foundry1
**Upstream:** [mifi/lossless-cut](https://github.com/mifi/lossless-cut) — GPL‑2.0‑only
**Skill:** `/package`

---

## Why

LosslessCut trims and cuts video/audio by stream copy rather than re‑encode. For a
game‑development distro it is the tool for cutting gameplay capture, trailers and devlog
footage without generation loss — a real workflow gap in the current catalogue, which has
`foundry-daw` for audio production and `foundry-art` for images but nothing for video.

## Visible surface

No newly designed surface. The package ships upstream's own GUI, upstream's icon, and a
`.desktop` entry derived from upstream's. The only Foundry‑authored user‑visible text is the
package description and the man page. No mockups warranted.

---

## Findings that shaped the design

### 1. Not in Ubuntu 26.04 universe (Step 1, blocking)

`apt-cache policy losslesscut` returns an empty `Candidate:`. The near‑matches
(`gnome-video-trimmer`, `gopchop`) are unrelated tools. No duplication of upstream effort.

### 2. Upstream ships no Debian packaging of its own (Step 2.5)

No `debian/` directory, no `.deb` build script, no PPA / Launchpad / OBS / COPR reference.
Upstream's Linux channels are the AppImage, the `.tar.bz2`, Snap and Flathub
(`no.mifi.losslesscut`). Nobody's toes to step on, and no upstream‑preferred package name to
defer to. Our binary name `losslesscut` matches upstream's own binary, Snap name and
`StartupWMClass`.

### 3. Building from source is not viable in CI

The upstream build needs yarn 4, `electron-vite`, `electron-builder`, a network fetch of the
Electron runtime, and a separate download of a prebuilt FFmpeg from
`mifi/ffmpeg-builds`. This is a pre‑built‑binary repack, per the skill's pre‑built flow.

### 4. The AppImage is **not** self‑contained — the load‑bearing correction

The initial plan was to ship the AppImage as a zero‑dependency standalone bundle. Extracting
it disproved that. `--appimage-extract` yields **the same 577 MB payload as the `.tar.bz2`**
plus only six small legacy libraries:

```
usr/lib/libnotify.so.4      usr/lib/libappindicator.so.1
usr/lib/libindicator.so.7   usr/lib/libgconf-2.so.4
usr/lib/libXtst.so.6        usr/lib/libXss.so.1
```

It bundles **no** GTK, NSS, CUPS, Pango or Cairo — `find` for those names returns nothing. The
Electron binary still links the host's `libgtk-3`, `libnss3`, `libnspr4`, `libcups`,
`libdbus-1`, `libasound`, `libatk*`, `libatspi`, `libgbm`, `libxkbcommon`, X11 and glibc.

Two consequences:

- Shipping the raw `.AppImage` would **add** a dependency, not remove one: a type‑2 AppImage
  needs `libfuse2t64` to self‑mount, which Ubuntu no longer installs by default. The
  FUSE‑free fallback (`--appimage-extract-and-run`) re‑extracts 577 MB on every launch.
- A genuinely dependency‑free package would mean vendoring the GTK/NSS closure ourselves —
  measured at **170 packages / 353 MiB** on a bare `ubuntu:26.04`. Beyond size, Mesa/GPU
  drivers must match the host kernel and so cannot be safely bundled, which would force
  software rendering in a *video* editor, and desktop theme/portal integration would degrade.

**Decision:** ship a normal `.deb` with `${shlibs:Depends}`. Every dependency is a stock
desktop library already present on any Ubuntu 26.04 desktop and on the Foundry ISO, so in
practice nothing extra is pulled in, while GPU acceleration and native theming keep working.

### 5. The AppImage is still the better *source* artifact

Same payload as the tarball, but it additionally carries upstream's own 512×512 PNG icon and
a desktop entry, which the `.tar.bz2` omits. So `build.sh` fetches the AppImage and unpacks it
at **build** time — nothing mounts an AppImage at runtime, so no FUSE dependency is incurred.

### 6. Bundled FFmpeg contributes nothing to `Depends:`

`resources/` carries a private FFmpeg 8.0 (275 MB). Its libraries link **only** against glibc:

```
libavcodec.so.62  -> libm libpthread libc ld-linux libdl libgcc_s libmvec
libavformat.so.62 -> libm libdl libmvec libpthread libc ld-linux libgcc_s
```

Every codec (x264, x265, aom, vpx, opus, …) is statically linked in. So no system `ffmpeg` is
required or consulted, and the media stack really is self‑contained. Verified by running
`resources/ffmpeg -version` with no `LD_LIBRARY_PATH` — it works, because the binaries carry
`$ORIGIN` in their RPATH. That also makes the tree fully relocatable to `/usr/lib/losslesscut`.

Licensing note: the bundled build is configured `--enable-gpl --enable-version3`, so the
FFmpeg binaries are GPL‑3.0+ while LosslessCut's own code is GPL‑2.0‑only. That combination is
upstream's own published arrangement, not one introduced here; recorded in `debian/copyright`.

### 7. The AppDir's six bundled libraries are dropped

The AppImage carries six compatibility shims under `usr/lib`. Checking each against Ubuntu
26.04 shows they are shims for distributions far older than our target, and that half of them
are **unsatisfiable** here:

| Bundled shim | Ubuntu 26.04 status | Verdict |
|---|---|---|
| `libgconf-2.so.4` | `libgconf-2-4` **absent** from the archive | drop — GConf is gone |
| `libappindicator.so.1` | needs `libdbusmenu-gtk4`, **no installation candidate** | drop — GTK 2 tray |
| `libindicator.so.7` | same, and GTK 2 | drop |
| `libnotify.so.4` | `libnotify4` 0.8.8 in universe | drop, use archive |
| `libXtst.so.6` | `libxtst6` in universe | drop, use archive |
| `libXss.so.1` | `libxss1` in universe | drop, use archive |

Shipping them would be dead weight whose own dependencies cannot be resolved. Electron
*dlopens* the latter three rather than linking them, so `dh_shlibdeps` cannot see them — they
are named in `Depends:` explicitly instead. Dropping the directory also removes the need for
any `LD_LIBRARY_PATH` in the launcher, which becomes a single `exec` line.

### 8. `dh_shlibdeps` needs the GUI libraries present at build time

A minimal `ubuntu:26.04` build container has no GTK/NSS/CUPS, and `dpkg-shlibdeps` then fails
outright with `cannot find library libgtk-3.so.0 …` — a hard error, not the soft
missing‑information warning that `--ignore-missing-info` suppresses. The 23 runtime library
packages are therefore listed in `Build-Depends:` and installed by `build.sh`. `-l` still
points at the app root and `resources/` so the *bundled* private libraries resolve internally
rather than being looked up in the archive.

### 9. Upstream's bundled ffmpeg has a malformed RPATH (upstreamable)

`objdump -p resources/ffmpeg` reports the RPATH as the literal string:

```
$ORIGIN:-Wl:../lib
```

Upstream's FFmpeg build passes `--extra-ldexeflags='-pie -Wl,-rpath=\$$ORIGIN -Wl,-rpath=\$$ORIGIN/../lib'`,
and the over-escaping means the second `-Wl` was never parsed as a flag — it and a bare
relative `../lib` ended up as RPATH *path components*. A non-`$ORIGIN` relative RPATH entry is
resolved against the **current working directory**, so running the binary from a directory
containing a crafted `../lib/libavcodec.so.62` could load it. Minor, but a real defect, and
lintian flags it as an error.

Fixed at package build time with `patchelf --set-rpath '$ORIGIN'` on `ffmpeg` and `ffprobe` —
`$ORIGIN` alone is sufficient because the `libav*` objects sit beside the binaries. **This is
worth reporting upstream**, in [mifi/ffmpeg-builds](https://github.com/mifi/ffmpeg-builds)
(which produces the binaries) rather than in `lossless-cut` itself. Not yet sent; there is no
quilt patch to forward, since the fix is a post-build ELF rewrite rather than a source change.

### 10. `chrome-sandbox` must be setuid root

Electron's SUID sandbox helper arrives mode 0755. Upstream's AppImage sidesteps the problem by
launching with `--no-sandbox`. We instead install it `4755 root:root` so the renderer sandbox
stays enabled — the same thing VS Code's official `.deb` does. `Rules-Requires-Root: no` plus
dh's `--root-owner-group` gives the required root ownership. Set in `override_dh_strip`,
because `dh_strip`'s `objcopy` runs *after* `dh_fixperms` and would otherwise reset the mode.

---

## Layout

| Path | Contents |
|---|---|
| `/usr/lib/losslesscut/` | Electron payload + private FFmpeg, verbatim |
| `/usr/bin/losslesscut` | one-line `sh` launcher: `exec /usr/lib/losslesscut/losslesscut "$@"` |
| `/usr/share/applications/losslesscut.desktop` | from upstream's repo entry, `Exec`/`Icon` fixed |
| `/usr/share/icons/hicolor/512x512/apps/losslesscut.png` | upstream's icon, from the AppImage |
| `/usr/share/metainfo/no.mifi.losslesscut.appdata.xml` | upstream AppStream data |
| `/usr/share/man/man1/losslesscut.1.gz` | Foundry‑authored (Policy §12.1) |

## Catalogue placement

There was no video category, and putting a video editor under "Audio & trackers" would have
mislabelled it. A new **`video`** category was added to `data/categories.json`
(`in_edition_tier: sprite`) carrying `losslesscut` as a `standalone_packages` entry — the same
shape `ghidra` uses under `retro-tools`. It appears only on `/packages`; the home page renders a
fixed six‑card list that was left untouched.

`losslesscut` is also added to `foundry-sprite`'s `Depends:` (bumped to 1.0.2), the multimedia
content‑creation edition, which already depends on non‑metapackage leaves (`digikam`,
`showfoto`).

Note the catalogue audit in `scripts/build-packages-data.js` only counts a package as *owned*
if a **category** reaches it — being in an edition metapackage's `Depends:` is not enough. Had
the category not been added, `task site-build` would have failed with
`✗ unowned packages — build FAIL`.

---

## Verification

Run 2026‑08‑29. Steps are the spec; raw output is the evidence.

**1. Universe check on `ubuntu:26.04` finds no `losslesscut`.**

```
=== apt-cache policy losslesscut ===
=== search losslesscut ===
=== search lossless (cut/video) ===
gnome-video-trimmer - Simple GUI application for lossless cutting of video files
gopchop - fast, lossless cuts-only editor for MPEG2 video files
```

PASS — empty `Candidate:`; both near-matches are unrelated tools.

**2. `build.sh` fetches the AppImage and the sha256 matches the pin and GitHub's own digest.**

```
=== Verifying sha256 ===
LosslessCut-linux-x86_64.AppImage: OK

$ gh api .../releases/tags/v3.69.0 --jq '.assets[]|select(.name|test("AppImage"))|.digest'
sha256:179eaae27bffbe25a6549a4a714474126b57c288cefc307046fc9c6313a12676
```

PASS — local sha256 equals GitHub's independently published asset digest.

**3. `dpkg-buildpackage` produces `losslesscut_3.69.0-1foundry1_amd64.deb`.**

```
dpkg-deb: building package 'losslesscut' in '../losslesscut_3.69.0-1foundry1_amd64.deb'.
OK   /repo/dist/losslesscut_3.69.0-1foundry1_amd64.deb  (164323976 bytes)
OK   dist/losslesscut_3.69.0-1foundry1.dsc
OK   dist/losslesscut_3.69.0-1foundry1.debian.tar.xz
OK   dist/losslesscut_3.69.0.orig.tar.gz
```

PASS — 156 MB binary package, plus the source package for the `Sources` index.

**4. `lintian` returns zero `E:` and zero `W:` lines beyond the root-privileges advisory.**

```
===== LINTIAN (deb) =====
running with root privileges is not recommended!
===== LINTIAN (dsc) =====
running with root privileges is not recommended!
```

PASS — both artifacts clean. The first build had 29 `E:` and 3 `W:`; the RPATH, ldconfig-trigger
and FSF-address findings were **fixed** rather than overridden, and only the irreducible
`embedded-library` and deliberate `elevated-privileges` tags carry justified overrides.

**5. `dpkg -s` shows a resolved `${shlibs:Depends}` with version constraints.**

```
Depends: libasound2t64 (>= 1.0.17), libatk-bridge2.0-0t64 (>= 2.5.3),
 libatk1.0-0t64 (>= 2.32.0), libatspi2.0-0t64 (>= 2.9.90), libc6 (>= 2.34),
 libcairo2 (>= 1.6.0), libcups2t64 (>= 1.7.0), libdbus-1-3 (>= 1.10),
 libexpat1 (>= 2.1~beta3), libgbm1 (>= 21.1.0), libgcc-s1 (>= 4.2),
 libglib2.0-0t64 (>= 2.40.0), libgtk-3-0t64 (>= 3.9.10), libnspr4 (>= 2:4.9-2~),
 libnss3 (>= 2:3.30), libpango-1.0-0 (>= 1.14.0), libudev1 (>= 183),
 libx11-6 (>= 2:1.4.99.1), libxcb1 (>= 1.9.2), libxcomposite1 (>= 1:0.4.6),
 libxdamage1 (>= 1:1.1), libxext6, libxfixes3, libxkbcommon0 (>= 0.5.0),
 libxrandr2, libnotify4, libxss1, libxtst6
Section: video
Installed-Size: 587995
Architecture: amd64
Version: 3.69.0-1foundry1
```

PASS — version constraints resolved; the three dlopen-only libraries appear as explicit deps.

**6. `chrome-sandbox` is setuid root.**

```
-rwsr-xr-x root:root /usr/lib/losslesscut/chrome-sandbox
```

PASS — mode 4755, owner root:root.

**7. Smoke install in a clean `ubuntu:26.04`; the bundled ffmpeg runs and all libraries resolve.**

```
===== 4. RPATH rewritten to $ORIGIN only (no ../lib, no -Wl) =====
  RUNPATH              $ORIGIN

===== 5. bundled ffmpeg still runs after patchelf =====
ffmpeg version n8.0-23-gd1f31a829d-20251022 Copyright (c) 2000-2025 the FFmpeg developers
ffprobe version n8.0-23-gd1f31a829d-20251022 Copyright (c) 2007-2025 the FFmpeg developers

===== 6. every runtime library of the Electron binary resolves =====
PASS — no unresolved shared libraries

===== 7. launcher, desktop entry, icon, man page =====
desktop entry VALID
/usr/share/icons/hicolor/512x512/apps/losslesscut.png
/usr/share/man/man1/losslesscut.1.gz
/usr/share/metainfo/no.mifi.losslesscut.appdata.xml

===== 8. no system ffmpeg pulled in =====
PASS — no system ffmpeg package
```

PASS — installs from a local apt source with every dependency satisfied from the archive; the
patched RPATH did not break the bundled ffmpeg; no system ffmpeg is dragged in.

**8. Repo-level checks still pass.**

```
$ task check-badges
PASS: all vendored packages declare X-Repology-Project (25 badged, 6 opt-out)
$ task check-build-deps
PASS: known Build-Depends companion-package pairs are complete
$ task check-licenses
PASS: all vendored packages are listed in LICENSES-VENDORED.md
$ task shellcheck
(no output — clean)
$ task gen-repology-ruleset
✓ wrote repology/foundry-linux.yaml  (10 remap, 26 metapackage-ignore, 6 none-ignore)
```

PASS. `build-all.sh` iterates `packages/*/` and dispatches to `build.sh` when present, so it
picks up `losslesscut` with no change.

**9. Real desktop install and GUI launch** (added 2026‑08‑30, Ubuntu 26.04 Wayland session).

`apt install` of the built `.deb` on a live desktop pulled **no additional packages** —
confirming the "every dependency is already present on a desktop" premise behind choosing
normal deps over vendoring:

```
Summary:
  Upgrading: 0, Installing: 1, Removing: 0, Not Upgrading: 4
Processing triggers for hicolor-icon-theme / man-db / desktop-file-utils
```

Launching it:

```
info: LosslessCut version 3.69.0 { isDev: false }
info: Initialized config store
info: Current version 3.69.0 / Newest version 3.69.0
info: ffmpeg -f lavfi -i 'nullsrc=s=256x256:d=1' -f null -
info: ffprobe -v 0 -of json -show_program_version

$ pgrep -af losslesscut
1205199 /usr/lib/losslesscut/losslesscut
1205203 /usr/lib/losslesscut/chrome-sandbox /usr/lib/losslesscut/losslesscut
```

PASS, and it settles the two items that were previously reasoned rather than observed:

- **The setuid sandbox is genuinely in use** — `chrome-sandbox` runs as a live process spawning
  the zygote, across 10 Electron processes, with no "SUID sandbox helper … not configured
  correctly" error. Installing it `4755 root:root` rather than launching `--no-sandbox` (as
  upstream's AppImage does) works as intended.
- **The rewritten RPATH survives in the real application.** LosslessCut runs its own startup
  self-test against the bundled binaries — an `ffmpeg` null-encode and an `ffprobe` version
  probe — and proceeds past both, exercising `patchelf --set-rpath '$ORIGIN'` in situ rather
  than just under `ffmpeg -version`.

One benign line, not a defect: `'--ozone-platform=wayland' is not compatible with Vulkan`.
Chromium noting it cannot use Vulkan under Wayland and falling back; every Electron app on
Wayland logs it, and it is unrelated to packaging.

## Not verified

- **`StartupWMClass=LosslessCut`** was not confirmed against a live window, as neither `wmctrl`
  nor `xdotool` is installed; the value is taken from upstream's own AppImage desktop entry,
  which launches the same binary the same way. A mismatch would only cost taskbar icon
  grouping.
- **The full `Depends:` chain of `foundry-sprite`** could not be resolved locally, since that
  needs every other Foundry package present in `dist/`. CI resolves it on the full build.

## Follow-ups

- [mifi/ffmpeg-builds#1](https://github.com/mifi/ffmpeg-builds/issues/1) reports the
  malformed RPATH from finding 9 with the reproducer and verified `$ORIGIN`-only repair.
- [mifi/lossless-cut#3035](https://github.com/mifi/lossless-cut/issues/3035) gives upstream the
  Foundry Linux packaging heads-up and offers to forward relevant bugs.
- Debian WNPP was re-checked on 2026-08-30 with no `losslesscut`/`lossless-cut` match. The
  prepared request remains an **RFP**, not an ITP, because Electron is not packaged in Debian.
  It is intentionally unfiled: this agent is not allowed to send email.

## Publication completion (2026-08-30)

- foundry-apt `v1.5.47` published `losslesscut 3.69.0-1foundry1` and
  `foundry-sprite 1.0.2` in a green 7m09s production run.
- The live APT Packages index contains LosslessCut; a real Ubuntu 26.04 desktop install pulled
  no additional dependencies and the GUI, setuid Electron sandbox, bundled ffmpeg, and ffprobe
  startup probes all passed.
- The refreshed public inventory was deployed as wald3n `v0.0.426`, then the publisher SSH
  handoff was hardened and the repaired one-command workflow created/deployed `v0.0.427`.
- The live `/open-source` page contains LosslessCut and reports `v0.0.427`.
- The historical publication marker was already absent. That state was recorded rather than
  manufacturing a retrospective marker; APT, snapshot, live page, release tag, and deploy were
  verified independently.

All packaging and publication work is complete. No ISO build was started.
