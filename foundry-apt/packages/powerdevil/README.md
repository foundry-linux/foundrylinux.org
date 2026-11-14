# PowerDevil battery warning recovery

Version `4:6.6.6-0ubuntu0.1+foundry1` rebuilds Ubuntu's
`4:6.6.6-0ubuntu0.1` source package with the upstream fixes for KDE bug
[500838](https://bugs.kde.org/show_bug.cgi?id=500838).
Ubuntu 26.04 and the checked Kubuntu PPAs offered no fixed drop-in upgrade on
2026-11-14. Upstream 6.6.7 still lacked these fixes; 6.7 and 6.8 require newer
KDE libraries. This package keeps the supported Ubuntu 26.04 desktop stack.

## Changes

Rising battery charge updates persistent notification percentages. Warnings
close above their corresponding thresholds, and the critical-action timer stops
above the critical threshold even if its notification has already been dismissed.
An AC connection still closes warnings through the existing handler. Charging
below the threshold does not itself dismiss a warning.

The three upstream changes are by Kai Uwe Broulik and Ramil Nurmanov:

- `8eac4ca25bf0b6bd957b6fc10b9b1c953110b3d6`
- `e6d348ab2daa08b5333cd6c4ed6ad1d637d30f2d`
- `5c7928c4775492bdcec373bf590afea6b14396c1`

The patch adds ten tests of the real Core charge-change handler, covering
notification updates, threshold boundaries, critical timers, combined laptop
battery charge and peripheral isolation. The original implementation failed nine
cases; the patched implementation passed all ten. These are unit tests, not
physical charger or suspend/resume tests.

## Packaging and maintenance

Ubuntu's Debian packaging, copyright record, package names and dependency minima
are retained. The source payload is SHA256-pinned in `build.sh`. Foundry adds
its changelog revision, regression tests, matching daemon/core dependency, and
CTest execution during packaging. Outputs include all four binary packages
(`powerdevil`, `libpowerdevilcore2`, `powerdevil-data`, `powerdevil-dev`) and a
rebuildable Debian source package.

Build using `task build PKG=powerdevil` from the APT repository, or run
`scripts/build-all.sh powerdevil` inside Ubuntu 26.04 with this repository mounted.
Development and compilation must stay in containers. The wrapper installs its
declared Build-Depends and defaults to two build jobs.

`foundry-desktop` requires the corrected package. When updating this source,
verify that upstream includes the recovery fixes and retain regression coverage;
a higher version number alone does not establish that the bug is fixed.
No Debian ITP is needed: PowerDevil is already packaged by Debian and Ubuntu.

Backport preparation, tests and Foundry packaging were assisted by OpenAI Codex
CLI 0.169.0, model `gpt-6-astra`, reasoning effort `medium`, verified from session
metadata. Upstream runtime authors and original Debian/Ubuntu packaging credits
are preserved.
