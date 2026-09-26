# Refresh vendored upstream packages

## Goal

Bring every Foundry-built package with a newer upstream release or tracked
source snapshot up to date, while preserving reproducible, checksum-pinned
builds.

This is the Foundry APT overlay, not the Ubuntu base update stream. Ubuntu's
Updates application lists all installed packages whose candidate version in the
Ubuntu archive is newer, including the kernel, KDE, Mesa, Firefox, libraries,
and firmware. Those packages are intentionally not copied into this repository:
Foundry's 26.04 ISO and devbox receive them when rebuilt from the current
Ubuntu archive.

## Audit

Checked 26 September 2026 against each package's authoritative GitHub, GitLab,
or PyPI release feed. First-party Foundry metapackages and packages already at
their latest upstream version are out of scope.

| Package | Packaged | Upstream | Source |
| --- | --- | --- | --- |
| `bsnes-jg` | 2.1.0 | 2.1.2 | GitLab release tag |
| `drmon` | drdevtools `d28c9d8` | `e26f7a5` | GitHub `main` snapshot |
| `flycast` | 2.6 | 2.7 | GitHub release tag |
| `ghidra` | 12.1 | 12.1.4 | GitHub release asset |
| `libvgm` | `d115188` (2026-04-06) | `c8b998b` (2026-09-05) | GitHub `master` snapshot |
| `pvsneslib` | 4.5.0 | 4.6.0 | GitHub release asset |
| `python3-glfw` | 2.10.0 | 2.10.2 | PyPI |
| `python3-librosa` | 0.11.0 | 1.0.0 | PyPI |
| `ruff` | 0.15.15 | 0.16.9 | PyPI |
| `uv` | 0.12.17 | 0.12.19 | PyPI |
| `vgmstream` | r2083 | r2117 | GitHub release tag |
| `wla-dx` | 10.6 | 10.7 | GitHub release tag |

The remaining release-tracked packages were current: asar 1.91, f9dasm 1.83,
halfempty 0.40, LDtk 1.5.3, M8TE 1.5, Mesen2 2.1.1, PPSSPP 1.20.4,
RPCS3 0.0.42, Snes9x 1.63, Tilemap Studio 4.0.1, and xemu-xbox 0.8.136.
The source-pinned `xemu` checkout also remains at its upstream `master` head.

## Implementation

1. Update each build wrapper's version, source URL, and SHA-256 pin.
2. Keep Flycast's tag and submodule superproject commit in sync with its Debian
   build rules.
3. Add native Debian changelog entries for the twelve package revisions.
4. Build every changed package in Ubuntu 26.04 containers. Resolve any source
   or dependency breakage before publishing.
5. Publish the resulting `.deb` and source packages through the existing
   package-publish workflow, then confirm the public APT index selects each
   new version.

## Full ISO refresh automation

`scripts/refresh-distro.sh` is the local, repeatable refresh entry point. It
captures the simulated upgrade from stock `ubuntu:26.04` to the current Ubuntu
archive, health-checks all ISO APT sources, force-rebuilds the Foundry package
overlay and the selected ISO editions, and QEMU-smoke-tests the results. It
writes evidence to `docs/investigations/<UTC-timestamp>-ubuntu-26.04-refresh.md`,
so a failed refresh can be rerun without overwriting its earlier evidence.

Run the complete local refresh with:

```bash
task distro-refresh
```

The default is both `anvil` and `atelier`. Use `task distro-refresh --
--edition anvil` while iterating, or append `--no-smoke` only when a QEMU boot
test cannot run in the current environment. The script does not sign, upload,
tag, or publish artifacts; release remains the separately reviewed
`task iso-publish` workflow. It does retain the existing `iso-build` behavior
of incrementing `foundry-iso/VERSION` and creating that local version-bump
commit before the new ISO is built.

## Completed in this change

Steps 1 through 3 are complete. The pins were obtained from PyPI file metadata,
GitHub release digests, or by downloading the release/source archive and
calculating SHA-256. The PVSnesLib 4.6.0 Linux asset gained a `_release` suffix,
which is reflected in its fetch URL. The updated PyPI packages now probe their
pinned artifact URL during build; probing the bare `files.pythonhosted.org`
host returns 404 even when the artifact is available. The Ubuntu 26.04 build
image now enables `universe` and `multiverse`, which supplies the declared
`dh-python` and Python build dependencies. The container runner refreshes APT
indexes before package wrappers install their own Build-Depends; the builder
image deliberately removes those indexes after its base layer is created.

## Validation plan

Run targeted Ubuntu 26.04 builds for the twelve changed packages, followed by
`task verify`, `task check-build-deps`, and the APT repository smoke test after
publication. Give `python3-librosa` extra attention because 1.0.0 is a major
release, and test Flycast because its package patches and full submodule clone
are release-sensitive.

For the complete distribution refresh, separately update the Ubuntu base by:

1. Rebuild the devbox and ISO with a fresh 26.04 APT index, without package
   version pins, so Ubuntu's current security and update pockets supply the
   candidate versions.
2. Record the resulting `apt list --upgradable` and installed-version inventory
   from the build container/image as the base-update evidence.
3. Smoke-test the ISO and devbox, then publish their rebuilt artifacts.
