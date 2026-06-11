# Plan: package Tilemap Studio for apt.foundrylinux.org

**Date**: 2026-06-10
**Goal**: Ship Tilemap Studio (v4.0.1, LGPL-3) — the retro-accurate tile/map editor and modern
successor to tUME — in `apt.foundrylinux.org`, wired into `foundry-retro-tools`.
**Linkage decision** (evidence in
[docs/investigations/2026-06-10-tilemap-studio-fltk-linkage.md](../investigations/2026-06-10-tilemap-studio-fltk-linkage.md)):
build **FLTK 1.4.5 X11-only and static-link** it; png/zlib/jpeg come from the system shared libs.

## Why not simpler

- Ubuntu 26.04 ships FLTK **1.4.4** as a **Wayland-hybrid** build; upstream's X11 window code
  (`fl_display`, `XA_CARDINAL`, …) won't compile or run against it. So we vendor FLTK 1.4.5.
- A separate FLTK-1.4.5 apt package would clash with the system `libfltk1.4` SONAME and has no other
  consumer → static-link instead.

## Package shape — `foundry-apt/packages/tilemap-studio/`

- `build.sh` — fetches **two** sha256-pinned tarballs: Tilemap Studio v4.0.1
  (`aba34651…`) and FLTK 1.4.5 (`7715e69c…`), places FLTK in `lib/fltk`, overlays `debian/`, runs
  `dpkg-buildpackage`.
- `debian/rules` — `override_dh_auto_configure/build` builds FLTK X11-only with
  `FLTK_USE_SYSTEM_{LIBPNG,ZLIB,LIBJPEG}=1` + `CMAKE_POSITION_INDEPENDENT_CODE=ON`, installs it into
  the source prefix, then puts `bin/` on `PATH` so the upstream Makefile's bare `fltk-config`
  resolves; `override_dh_auto_install` installs the binary, icons and `.desktop`.
- `debian/patches/` (quilt, both **upstreamable**):
  - `0001-x11-platform-includes.patch` — add `<FL/platform.H>`/`<X11/Xlib.h>`/`<X11/Xatom.h>` to
    `utils.h` (FLTK 1.4.5 headers no longer leak them).
  - `0002-cstring-in-preferences.patch` — `#include <cstring>` for GCC 15.
- `debian/tilemap-studio.lintian-overrides` — one justified `embedded-library [… fltk]` (only FLTK
  is static; png/zlib/jpeg are system libs).
- `debian/man/tilemapstudio.1`, `debian/tilemap-studio.desktop`, DEP-5 `debian/copyright`.

## Steps

1. ~~Universe check~~ — not in 26.04 universe; proceed. ✅
2. ~~Pin both tarballs~~ (sha256 reproducible). ✅
3. ~~Prove the static build recipe in Docker~~ — PIE, 2.1 MB, `ldd` shows no libfltk (static), png/z
   dynamic. ✅
4. ~~Author the `debian/` tree + patches + build.sh~~. ✅
5. ~~**Build via `build.sh` in `ubuntu:26.04` + lintian clean**~~ — zero E/W (lintian didn't even flag FLTK embedded-library; no override entry consumed). PIE + stripped. ✅
6. ~~**Wire into `foundry-retro-tools`**~~ — already wired in 1.0.9; fixed description body + Wed→Thu changelog day (1d2c65c). ✅
7. ~~**Local dependency-chain smoke test**~~ — `apt-cache depends` resolves `tilemap-studio`; fresh-container `apt install` succeeds. ✅
8. ~~**Commit**, then `task sync` / tag to publish; verify live `apt install tilemap-studio`~~ — tagged v1.5.14; live `apt install` confirms `4.0.1-1foundry1` installed. ✅
9. **Upstream the two patches** — open PRs against `Rangi42/tilemap-studio`, record the URLs in each
   patch's `Forwarded:` header and the changelog. (New mandatory last step in the `/package` skill,
   added this session.)

## Verification

1. `bash build.sh` in `ubuntu:26.04` produces `dist/tilemap-studio_4.0.1-1foundry1_amd64.deb`.

```
OK   /repo/foundry-apt/dist/tilemap-studio_4.0.1-1foundry1_amd64.deb  (661804 bytes)
```

PASS

2. `lintian` returns clean apart from the documented FLTK `embedded-library` override.

```
running with root privileges is not recommended!
```

PASS (zero E: or W: lines; lintian did not even trigger the embedded-library tag — FLTK static link went undetected, so no override was needed)

3. `dpkg-deb -I` shows `${shlibs:Depends}` resolved (libpng16, libz, cairo, pango, X11), no libfltk.

```
Depends: libc6 (>= 2.38), libcairo2 (>= 1.10.0), libgcc-s1 (>= 3.3.1),
 libglib2.0-0t64 (>= 2.12.0), libpango-1.0-0 (>= 1.44.3),
 libpangocairo-1.0-0 (>= 1.14.0), libpng16-16t64 (>= 1.6.46),
 libstdc++6 (>= 14), libx11-6, libxfixes3, libxinerama1 (>= 2:1.1.4),
 libxpm4, zlib1g (>= 1:1.1.4)
```

PASS (libfltk absent; png16/zlib/cairo/pango/X11 all present with version constraints)

4. `file usr/bin/tilemapstudio` reports `pie executable`, stripped after `dh_strip`.

```
/dev/stdin: ELF 64-bit LSB pie executable, x86-64, version 1 (SYSV),
 dynamically linked, interpreter /lib64/ld-linux-x86-64.so.2,
 BuildID[sha1]=3bc04cba5e664fd7efe167c76885211208c46887,
 for GNU/Linux 3.2.0, stripped
```

PASS

5. Fresh-container `apt install ./tilemap-studio_*.deb` succeeds; binary launches `--help`/version.

```
Setting up tilemap-studio (4.0.1-1foundry1) ...
-rwxr-xr-x 1 root root 1.7M Jun 10 06:58 /usr/bin/tilemapstudio
Can't open display:
```

PASS (`Can't open display:` is expected — FLTK launched and hit the headless X11 check; binary executes correctly)

6. `foundry-retro-tools` resolves `tilemap-studio` via `apt-cache depends`.

```
apt-cache depends foundry-retro-tools
  Depends: tilemap-studio
```

PASS

## Upstreaming (step 9 detail)

Both patches are toolchain/portability fixes (GCC 15 / FLTK 1.4.5), not Debian glue, so they're
candidates to go upstream. **But check `master` first** — one of them is already fixed there.

### Step 9.0 — Pre-flight: reproduce on upstream `master` (gating, do NOT skip)

A patch against the **v4.0.1 tag** may already be fixed on `master`. Verified 2026-06-10
(`master` @ `244378b`, ahead of the `v4.0.1` tag):

| Our patch | Status on `master` | Action |
|---|---|---|
| `0002-cstring-in-preferences.patch` | **Already fixed** — `src/preferences.cpp` line 1 is `#include <cstring>`. | **No PR.** It will ship in the next release after v4.0.1; drop our patch then. Set `Forwarded: not-needed (fixed in master 244378b)`. |
| `0001-x11-platform-includes.patch` | **Still missing** — `src/utils.h` has no `<FL/platform.H>`/`<X11/*>` includes. | **PR it** (below), after confirming it still fails to build `master` against FLTK 1.4.5. |

Confirm `0001` still reproduces before opening the PR:

```bash
git clone --depth 1 https://github.com/Rangi42/tilemap-studio /tmp/tms-master
# build FLTK 1.4.5 X11-only into /tmp/tms-master (same cmake flags as debian/rules), then:
cd /tmp/tms-master && PATH="$PWD/bin:$PATH" make 2>&1 | grep -E "fl_display|XA_CARDINAL|Pixmap"
# expect: 'fl_display'/'XA_CARDINAL' not declared  -> the fix is still needed
```

### Step 9.1 — Open the PR for `0001` (X11 platform includes)

The change (CRLF-preserving, as upstream uses CRLF):

```diff
--- a/src/utils.h
+++ b/src/utils.h
@@ -13,6 +13,11 @@
 #pragma warning(push, 0)
 #include <FL/fl_types.h>
+#ifndef _WIN32
+#include <FL/platform.H>
+#include <X11/Xlib.h>
+#include <X11/Xatom.h>
+#endif
 #pragma warning(pop)
```

Why: on FLTK 1.4.5's cleaned-up headers, `main-window.{cpp,h}`, `modal-dialog.cpp` and
`option-dialogs.cpp` use `fl_display`, `fl_xid`, `Pixmap` and `XA_CARDINAL` under `#ifndef _WIN32`
but only `<X11/xpm.h>` was included; these no longer come in transitively. Adding the includes to the
shared `utils.h` fixes every TU that needs them. (FLTK ≤1.4.4 leaked them, which is why older builds
compiled.)

```bash
# 1. Fork + clone (gh sets up the 'origin' fork + 'upstream' remote)
gh repo fork Rangi42/tilemap-studio --clone --remote
cd tilemap-studio
git checkout -b fix-fltk145-x11-includes

# 2. Apply just the utils.h hunk from our quilt patch (strip the DEP-3 header / keep CRLF)
patch -p1 < ~/SRC/foundrylinux.org/foundry-apt/packages/tilemap-studio/debian/patches/0001-x11-platform-includes.patch

# 3. Commit
git commit -am "Include FLTK platform + X11 headers explicitly for the X11 build

main-window, modal-dialog and option-dialogs use fl_display, fl_xid, Pixmap
and XA_CARDINAL under #ifndef _WIN32 but only included <X11/xpm.h>. These
resolved transitively on FLTK <= 1.4.4 but not on FLTK 1.4.5's cleaned-up
headers, so the X11 build fails with 'fl_display'/'XA_CARDINAL' not declared.
Add the includes to the shared utils.h. Built on Ubuntu 26.04, GCC 15,
FLTK 1.4.5."

# 4. Push to the fork + open the PR
git push -u origin fix-fltk145-x11-includes
gh pr create --repo Rangi42/tilemap-studio \
  --title "Fix X11 build against FLTK 1.4.5 (explicit platform/X11 includes)" \
  --body "$(cat <<'BODY'
### Problem
Building the Linux (X11) target against **FLTK 1.4.5** fails:

```
src/main-window.cpp: error: 'fl_display' was not declared in this scope
src/main-window.cpp: error: 'XA_CARDINAL' was not declared in this scope
src/main-window.h:  error: 'Pixmap' does not name a type
```

`main-window.{cpp,h}`, `modal-dialog.cpp` and `option-dialogs.cpp` use `fl_display`,
`fl_xid`, `Pixmap` and `XA_CARDINAL` under `#ifndef _WIN32`, but only `<X11/xpm.h>` is
included. These came in transitively on FLTK ≤ 1.4.4; FLTK 1.4.5 cleaned up its headers
and no longer leaks them.

### Fix
Add the X11 / FLTK-platform includes to the shared `src/utils.h` (guarded `#ifndef _WIN32`),
so every translation unit that needs them gets them.

### Build environment
Ubuntu 26.04, GCC 15, FLTK 1.4.5 built X11-only (`FLTK_BACKEND_WAYLAND=0`). Found while
packaging Tilemap Studio for Foundry Linux.
BODY
)"
```

### Step 9.2 — Record the outcome back in our tree

1. Put the PR URL in `0001`'s header: `Forwarded: github.com/Rangi42/tilemap-studio/pull/<n>` (with the `https://` prefix in the actual header).
2. Set `0002`'s header to `Forwarded: not-needed (fixed in upstream master 244378b)`.
3. Note both in `debian/changelog` (e.g. "0001 sent upstream as PR #N; 0002 already fixed upstream,
   carried until next release"). Bump `tilemap-studio` to `4.0.1-1foundry2` if the patch headers
   change after the first build was published.

### Step 9.3 — Follow-up (drop patches on the next release)

When upstream tags a release after v4.0.1:
- If it includes `<cstring>` (it will) → delete `0002` from `debian/patches/series` and the file.
- If PR #N (the X11 includes) merged and shipped → delete `0001` too.
- Bump `TILEMAP_VERSION`/`SHA256` in `build.sh`, rebuild, confirm the now-smaller series still
  applies, and bump the changelog.

> **One-PR-vs-two**: only `0001` needs a PR now, so it's a single focused PR — easiest for the
> maintainer to review and merge. If a future packaging run produces more upstreamable hunks, prefer
> one PR per independent fix so a rejection of one doesn't block the others.
>
> **Gating note**: opening a PR touches a third-party repo (outward-facing) — do it only on explicit
> go-ahead, or hand the user the ready-to-run block above.
