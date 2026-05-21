# Licences of vendored upstream packages

Each `.deb` we repackage and redistribute via this repo keeps the upstream's licence. Below is the running summary. The full text of each licence ships inside the corresponding `.deb` under `/usr/share/doc/<package>/copyright`.

| Package | Upstream | Licence | Where the text lives in the .deb |
|---|---|---|---|
| `f9dasm` | [Arakula/f9dasm](https://github.com/Arakula/f9dasm) | [GPL-2.0+](https://github.com/Arakula/f9dasm) | `/usr/share/doc/f9dasm/copyright` |

## Planned future entries (not yet shipped)

| Package | Upstream | Licence | Notes |
|---|---|---|---|
| `ghidra` | [NSA / Ghidra](https://ghidra-sre.org/) | [Apache-2.0](https://github.com/NationalSecurityAgency/ghidra/blob/master/LICENSE) | Big tarball (~400 MB); host on R2. |
| `vgmstream` | [vgmstream/vgmstream](https://github.com/vgmstream/vgmstream) | [ISC](https://github.com/vgmstream/vgmstream/blob/master/COPYING) | Audio decode for legacy formats. |
| `libvgm` | [ValleyBell/libvgm](https://github.com/ValleyBell/libvgm) | [GPL-2.0+](https://github.com/ValleyBell/libvgm/blob/master/LICENSE) | Sound chip emulation library. |

## Retired entries

| Package | Reason |
|---|---|
| `task` | The official Cloudsmith apt repo (`deb.taskfile.dev`) is the easiest source — Phase 0's `install-task.sh` configures it directly, no in-repo vendoring needed. |
| `xa65` | Ubuntu 26.04 universe ships it as `xa65 2.4.1-0.1build1` — same upstream code as we briefly vendored under `2.4.1-1foundry1`. Phase 0's `install-foundry-retro-tools.sh` now apt-installs `xa65` directly; `foundry-retro-tools` `Depends: xa65` resolves to the universe entry. Retirement plan: [`../docs/plans/2026-05-18-retire-xa65.md`](../../docs/plans/2026-05-18-retire-xa65.md). |

## Foundry-authored content

- Metapackage source trees (`packages/<name>/debian/`) — MIT for the packaging metadata itself
- Helper scripts (`scripts/*.sh`) — GPL-2.0
- aptly config + GitHub Actions workflows — GPL-2.0

The full project licence is [GPL-2.0](LICENSE), matching the World Foundry engine.
