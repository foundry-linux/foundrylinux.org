# ITP draft: python3-glfw

## Upstream packaging audit

Audited the pinned upstream payload on 2026-08-05 with
`foundry-apt/scripts/audit-upstream-packaging.sh`. **No upstream packaging signals found:** no
upstream-maintained `debian/`, `.deb`/`.dsc` builder, `PKGBUILD`/`.spec`, or PPA/Launchpad/OBS/COPR
packaging reference.

## wnpp check — naming/duplication risk found, read carefully

No wnpp bug exists for a package named "python3-glfw" or for our
specific upstream, **FlorianRhiem/pyGLFW**. However, search turned up
that Debian **already ships a different, unrelated project with a very
similar name and near-identical purpose**:

- **`python3-pyglfw`** — currently in Debian bookworm (2.5.6+dfsg-1),
  trixie and sid (2.8.0+dfsg-1). Filed as [Bug #1025413](https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=1025413) — "ITP: python-pyglfw -- Python bindings for GLFW", by Étienne Mollier, 2022-12-04. This wraps the **`pyglfw/pyglfw`** GitHub project — a *different* upstream from ours (`FlorianRhiem/pyGLFW`), even though both are "Python ctypes bindings for GLFW3."

This is a real duplication risk, not just a naming coincidence: both
projects independently provide ctypes-based Python bindings to the same
C library (GLFW3). Filing `python3-glfw` for FlorianRhiem's project
alongside Debian's existing `python3-pyglfw` (a different project, same
purpose) would likely draw ftp-master/mentors scrutiny about why Debian
needs two GLFW binding packages. Before filing, whoever owns this should:

1. Confirm FlorianRhiem/pyGLFW has functional differences or API/ecosystem
   reasons (e.g. it's the binding some specific widely-used tool depends
   on) that justify a second package, rather than just packaging it
   because it's what Foundry happens to use internally.
2. If no compelling differentiator exists, consider whether Foundry's
   internal dependency should switch to the already-Debian-packaged
   `python3-pyglfw` instead of packaging a second, functionally
   overlapping binding.

## ITP draft (if a differentiator is found and filing proceeds)

```
Package name    : python3-glfw
Version         : 2.10.0
Upstream contact: Florian Rhiem <florian.rhiem@gmail.com>
URL             : https://github.com/FlorianRhiem/pyGLFW
Licence         : MIT
Programming lang: Python 3
Description     : Python ctypes bindings for GLFW3
```

GLFW is a portable library for OpenGL, Vulkan, and window/input
management. This package provides Python ctypes bindings that wrap the
system GLFW3 shared library, exposing the full GLFW API under Pythonic
naming conventions. Supports X11 and Wayland on Linux (via `libglfw3` or
`libglfw3-wayland`), and is a common foundation for OpenGL game windows,
shader demos, and real-time rendering tools that don't use a
higher-level framework like pygame or pyglet.

## Notes

- Single license (MIT), DFSG-clean — the licensing itself is not the
  concern here, the concern is functional duplication with an existing
  Debian package.
- If filed, the ITP bug should explicitly name and link `python3-pyglfw`
  and #1025413 to pre-empt the "why not just use the existing binding"
  question from Debian reviewers.
