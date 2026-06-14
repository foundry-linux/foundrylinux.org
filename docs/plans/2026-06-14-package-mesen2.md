# Package Mesen2 (`mesen2`) — pre-built binary repack

**Date:** 2026-06-14
**TODO:** `### Packaging — new upstreams` → "Package `bsnes-jg` and/or `mesen2`"
**Skill:** `/package`

## What

Package the **Mesen** multi-system emulator (repo: [`SourMesen/Mesen2`](https://github.com/SourMesen/Mesen2),
GPL-3.0, latest `2.1.1` 2025-07-06) as `mesen2` on `apt.foundrylinux.org`, and wire it
into `foundry-emulators-consoles-heavy`.

Systems emulated: NES, SNES, Game Boy / GBC, Game Boy Advance, PC Engine / TurboGrafx-16,
SMS / Game Gear, WonderSwan. Accuracy-focused, with a full debugger suite.

## Why repack the pre-built binary (not build from source)

`COMPILING.md` requires the **.NET 8 SDK + SDL2 + clang** and a `dotnet publish` AoT build —
a heavy toolchain that isn't cleanly in Ubuntu universe. Upstream publishes an **official
self-contained Linux x64 release** (`Mesen_2.1.1_Linux_x64.zip`), built clang-AoT on
ubuntu-22.04. This is the Ghidra-style pre-built repack path. The **repo is archived**, so
there is no future source churn to track and **Step 7 (upstreaming patches) is moot** —
nothing to send back.

### Upstream artifact facts (verified 2026-06-14)

- Zip: `Mesen_2.1.1_Linux_x64.zip`, 39 534 900 bytes,
  sha256 `7a9947575cc198209f743fef83fb2b702b786ea705506bdf3f2aea01ab7c1ce9`.
- Contents: a **single** file `Mesen` (83 367 840 bytes).
- `file Mesen` → `ELF 64-bit LSB pie executable, x86-64, dynamically linked, stripped`.
- `ldd Mesen` → only `libz.so.1, libm.so.6, libc.so.6` (NEEDED). Everything else is either
  AoT-compiled in or **bundled** (single-file) and dlopened at runtime.
- It is a **.NET AoT + `PublishSingleFile` bundle**: native deps (`libSkiaSharp.so`,
  `libHarfBuzzSharp.so`) are appended to the ELF host and extracted to a per-user temp dir
  at first run. **Do NOT strip / objcopy** — rewriting the ELF would truncate the appended
  bundle. (Upstream already ships it stripped, so `dh_strip` has nothing to do anyway.)

### Runtime dependencies (dlopen'd — invisible to `dh_shlibdeps`)

`${shlibs:Depends}` only captures libc6/zlib1g from the outer ELF. The real GUI deps come
from the Avalonia X11 backend (P/Invoke) and the bundled Skia. From `strings` + the embedded
`UnableToStartMissingSdl` error string, and confirmed installable on ubuntu:26.04:

| soname (dlopen) | ubuntu 26.04 package | tier |
|---|---|---|
| `libSDL2-2.0.so.0` | `libsdl2-2.0-0` | **Depends** (documented hard requirement) |
| `libX11.so.6` … `libXi/Xrandr/Xcursor/Xext`, `libICE`, `libSM` | `libx11-6 libxi6 libxrandr2 libxcursor1 libxext6 libice6 libsm6` | **Depends** (X11 windowing) |
| `libGL.so.1` | `libgl1` | **Depends** (OpenGL render) |
| (Skia, bundled) → `libfontconfig.so.1` | `libfontconfig1` | **Depends** (pulls libfreetype6) |
| `libgtk-3.so.0` | `libgtk-3-0t64` | Recommends (native file dialogs only) |
| `libvulkan.so.1` | `libvulkan1` | Recommends (optional Vulkan backend) |

ICU / OpenSSL probing strings are .NET's app-local / system fallback list — not hard apt deps.

## Package layout

```
/usr/lib/mesen2/Mesen                                   the 83 MB self-contained binary (0755)
/usr/bin/mesen2                                         wrapper: exec /usr/lib/mesen2/Mesen "$@"
/usr/share/applications/mesen2.desktop                  authored from upstream Linux/appimage/Mesen.desktop
/usr/share/icons/hicolor/48x48/apps/mesen2.png          upstream Linux/appimage/Mesen.48x48.png
/usr/share/man/man1/mesen2.1.gz                          authored (Policy §12.1 — wrapper in /usr/bin)
```

- `Architecture: amd64` (x64 zip only; ARM64 zip exists → future follow-up).
- `Section: otherosfs` (matches `snes9x-gtk`, `ppsspp`).
- Source format `3.0 (quilt)`, `dpkg-buildpackage -b` (binary-only, no orig tarball needed).
- `override_dh_strip:` empty (do not touch the single-file bundle); `override_dh_auto_*` empty.

## Source tree (`foundry-apt/packages/mesen2/`)

```
build.sh                       fetch+sha256 zip → unzip → overlay debian/ → dpkg-buildpackage -b → dist/
debian/control                 amd64, otherosfs, Depends/Recommends per table above
debian/changelog               2.1.1-1foundry1, resolute
debian/copyright               DEP-5, GPL-3.0
debian/rules                   dh $@; empty auto_*/dh_strip overrides; install binary+wrapper+desktop+icon
debian/source/format           3.0 (quilt)
debian/watch                   github tags, numeric version
debian/mesen2.desktop          authored
debian/mesen2.png              vendored 48x48 icon
debian/man/mesen2.1            authored
debian/manpages                lists debian/man/mesen2.1
debian/mesen2.lintian-overrides  only for irreducible pre-built-ELF tags (statically-linked, hardening-*)
```

## Verification steps

1. `bash packages/mesen2/build.sh` inside `ubuntu:26.04` → `dist/mesen2_2.1.1-1foundry1_amd64.deb` produced.
2. `dpkg-deb -I` shows resolved `Depends:` incl. `libsdl2-2.0-0` + X11 stack; `dpkg-deb -c` shows the 5 paths above.
3. `lintian` returns clean — zero `E:`/`W:` except irreducible pre-built tags recorded in `mesen2.lintian-overrides` with justifications.
4. Smoke install in fresh `ubuntu:26.04`; `mesen2 --help` (or headless launch under `xvfb-run`) gets past the SDL2/Avalonia init (no `UnableToStartMissingSdl`).
5. `foundry-emulators-consoles-heavy` (bumped) resolves `mesen2` via local `dpkg-scanpackages` apt source.

## Wiring + release

- Add `mesen2` to `foundry-emulators-consoles-heavy` `Depends:`; `dch` bump.
- `TODO.md`: mark packaging item done, add `ITP: mesen2` (GPL-3.0) tracking entry.
- Commit, `task bump`, watch publish workflow, verify live `apt install mesen2`.
- **No Step 7** — upstream archived; record that in the changelog.

## Post-release fix — no window in VMs (2.1.1-1foundry2)

**Symptom (reported):** on Ubuntu 26.04 in a VM, `mesen2` "doesn't work — no window
ever appears." The process runs but no window maps; no crash, no stderr.

**Root cause:** Mesen's Avalonia UI requires an OpenGL context to open its window.
In a VM without working hardware GL, GL init fails (`failed to load driver: radeonsi`,
`glx: failed to create dri3 screen`) and Mesen never maps a window — it does **not**
auto-fall-back to software rendering. Reproduced: GPU-less env → no window; forcing
Mesa's software rasteriser (`LIBGL_ALWAYS_SOFTWARE=1`, llvmpipe) → window maps and the
config wizard renders correctly (verified on a real display).

**Fix:**
- `/usr/bin/mesen2` is now a wrapper (`debian/mesen2.wrapper`) that forces
  `LIBGL_ALWAYS_SOFTWARE=1` when `systemd-detect-virt` reports a VM/container, so the
  UI always opens there; bare metal keeps the GPU for full-speed emulation. Override:
  `MESEN2_FORCE_SOFTWARE_GL=1|0`.
- `libgl1-mesa-dri` added to `Depends` so the llvmpipe driver is guaranteed present.

**Validation note:** an X-forwarded `ubuntu:26.04` container is *not* a valid bench for
the window check (a root-in-container can't map top-level windows onto the host
XWayland — no Mesen window appears there even with hardware GL). The fix is proven by
(1) software GL rendering a real Mesen window on a real display, and (2) the wrapper
correctly selecting software GL under virtualization (`systemd-detect-virt => docker`).

## Follow-ups (not in this pass)

- ARM64 build from `Mesen_2.1.1_Linux_ARM64.zip` (second `Architecture: arm64` artifact).
- `bsnes-jg` (the other half of the TODO item) — separate `/package` run.
