# Plan: upstream X11 platform-includes fix to Rangi42/tilemap-studio

**Date**: 2026-06-11
**Goal**: Send the one upstreamable patch from our packaging (`0001-x11-platform-includes.patch`)
back to [Rangi42/tilemap-studio](https://github.com/Rangi42/tilemap-studio) as a PR.
This is step 9 of [docs/plans/2026-06-10-package-tilemap-studio.md](2026-06-10-package-tilemap-studio.md).

## Background

We carry two quilt patches against v4.0.1:

| Patch | Status on `master` (`244378b`) | Action |
|---|---|---|
| `0001-x11-platform-includes.patch` | **Still missing** — `src/utils.h` has no `<FL/platform.H>`/`<X11/*>` | **PR this** |
| `0002-cstring-in-preferences.patch` | **Already fixed** upstream | No PR; set `Forwarded: not-needed` |

`0001` is a genuine portability fix: FLTK 1.4.5 cleaned up its X11 headers and no longer
leaks `fl_display`, `Pixmap`, `XA_CARDINAL` etc. transitively. Every FLTK-1.4.5 X11 build
fails without it — not just ours. The fix is 5 lines, `#ifndef _WIN32`-guarded, and safe on
all platforms.

## The change

File: `src/utils.h`

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

## PR details

**Title**: Fix X11 build against FLTK 1.4.5 (explicit platform/X11 includes)

**Body**:

```
### Problem

Building the Linux (X11) target against FLTK 1.4.5 fails:

    src/main-window.cpp: error: 'fl_display' was not declared in this scope
    src/main-window.cpp: error: 'XA_CARDINAL' was not declared in this scope
    src/main-window.h:  error: 'Pixmap' does not name a type

main-window.{cpp,h}, modal-dialog.cpp and option-dialogs.cpp use
fl_display, fl_xid, Pixmap and XA_CARDINAL under #ifndef _WIN32, but
only <X11/xpm.h> is included. These came in transitively on FLTK <= 1.4.4;
FLTK 1.4.5 cleaned up its headers and no longer leaks them.

### Fix

Add the X11 / FLTK-platform includes to the shared src/utils.h (guarded
#ifndef _WIN32), so every translation unit that needs them gets them.

### Build environment

Ubuntu 26.04, GCC 15, FLTK 1.4.5 built X11-only (FLTK_BACKEND_WAYLAND=0).
Found while packaging Tilemap Studio for Foundry Linux.
```

## Steps

1. **Confirm `0001` still fails on `master`** — build `master` with FLTK 1.4.5 X11-only
   and grep for the `fl_display`/`XA_CARDINAL` errors. (Gating: if upstream already fixed
   it, there's nothing to send.)
2. **Fork + branch** — `gh repo fork Rangi42/tilemap-studio --clone --remote`.
3. **Apply the patch** — strip DEP-3 header, apply only the `utils.h` hunk, preserve CRLF
   line endings (upstream uses CRLF).
4. **Commit + push + open PR** via `gh pr create`.
5. **Record the PR URL** in `0001`'s `Forwarded:` header and in a new `debian/changelog`
   entry for `tilemap-studio`.
6. **Set `0002`'s header** to `Forwarded: not-needed (fixed in upstream master 244378b)`.
7. **Commit the updated `debian/patches/` tree** to foundrylinux.org.

## Commands (ready to run)

~~~bash
# Step 1 — pre-flight: confirm 0001 still fails on master
git clone --depth 1 https://github.com/Rangi42/tilemap-studio /tmp/tms-master
# (build FLTK 1.4.5 X11-only into /tmp/tms-master per debian/rules flags, then:)
cd /tmp/tms-master && PATH="$PWD/bin:$PATH" make 2>&1 | grep -E "fl_display|XA_CARDINAL|Pixmap"
# expect: 'fl_display'/'XA_CARDINAL' not declared -> proceed

# Steps 2–4
gh repo fork Rangi42/tilemap-studio --clone --remote
cd tilemap-studio
git checkout -b fix-fltk145-x11-includes

# Apply utils.h hunk only (strip the DEP-3 header lines from the patch first)
patch -p1 < ~/SRC/foundrylinux.org/foundry-apt/packages/tilemap-studio/debian/patches/0001-x11-platform-includes.patch

git commit -am "Include FLTK platform + X11 headers explicitly for the X11 build

main-window, modal-dialog and option-dialogs use fl_display, fl_xid, Pixmap
and XA_CARDINAL under #ifndef _WIN32 but only included <X11/xpm.h>. These
resolved transitively on FLTK <= 1.4.4 but not on FLTK 1.4.5's cleaned-up
headers, so the X11 build fails with 'fl_display'/'XA_CARDINAL' not declared.
Add the includes to the shared utils.h. Built on Ubuntu 26.04, GCC 15,
FLTK 1.4.5."

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
included. These came in transitively on FLTK <= 1.4.4; FLTK 1.4.5 cleaned up its headers
and no longer leaks them.

### Fix

Add the X11 / FLTK-platform includes to the shared `src/utils.h` (guarded `#ifndef _WIN32`),
so every translation unit that needs them gets them.

### Build environment

Ubuntu 26.04, GCC 15, FLTK 1.4.5 built X11-only (`FLTK_BACKEND_WAYLAND=0`). Found while
packaging Tilemap Studio for Foundry Linux.
BODY
)"
~~~

## Verification

1. Pre-flight grep finds `fl_display`/`XA_CARDINAL` errors on `master` — confirms fix is
   still needed before we open the PR.
2. `gh pr create` exits 0 and returns a URL.
3. `0001`'s `Forwarded:` header updated to the PR URL.
4. `0002`'s `Forwarded:` header updated to `not-needed (fixed in upstream master 244378b)`.
5. `foundrylinux.org` commit contains only `debian/patches/` changes.

## Follow-up (after upstream merges / next release)

When upstream tags a release after v4.0.1:
- If `0001` merged → delete `debian/patches/0001-x11-platform-includes.patch` and its
  `series` entry; bump `TILEMAP_VERSION`/`SHA256` in `build.sh`; rebuild.
- `0002` will ship in the next release by definition → delete it at the same time.
- Bump `debian/changelog` to `4.x.y-1foundry1` and confirm a clean build.
