# ITP draft: python3-mss

## wnpp check

Searched wnpp for "python-mss" / "python3-mss" (BoboTiG's screenshot library). **No existing wnpp bug found** — search returned generic ITP examples and the PyPI page for `mss`, nothing on bugs.debian.org for this package specifically.

## ITP draft

```
Package name    : python3-mss
Version         : 10.2.0
Upstream contact: Mickaël Schoentgen <contact@tiger-222.fr>
URL             : https://github.com/BoboTiG/python-mss
Licence         : MIT
Programming lang: Python 3
Description     : ultra-fast cross-platform screenshot library for Python
```

MSS (Multiple ScreenShots) is a pure-Python ctypes library that captures
screenshots at maximum speed without any external dependencies. On
Linux it accesses X11 and XRandR directly via ctypes; no PIL or OpenCV
is required for capture. Optional Pillow integration converts raw pixel
buffers to PNG or other image formats. Useful for game recording tools,
automated QA harnesses, and screen-capture overlays. Also ships the
`mss(1)` command-line tool for one-shot screenshot capture.

Debian has screenshot tools (`scrot`, `gnome-screenshot`, `flameshot`)
but no lightweight, dependency-free Python screenshot *library* for
scripting/automation use cases — a legitimate, narrow gap. Given mss's
wide PyPI usage (a common building block in QA/automation/game-dev
tooling), this is a clean, low-risk ITP.

## Notes

- Single license (MIT), DFSG-clean.
- Architecture `all` (pure Python + ctypes against system X11/XRandR —
  no compiled extension to build), about as simple a Python ITP as this
  batch gets.
- Since this search came back with no existing package or ITP (unlike
  `python3-pydub` and `python3-librosa` elsewhere in this batch), this
  one looks genuinely available — worth prioritizing if only one or two
  of the python3-* items in this batch get filed.
