# Licences of vendored upstream packages

Each `.deb` we repackage and redistribute via this repo keeps the upstream's licence. Below is the running summary. The full text of each licence ships inside the corresponding `.deb` under `/usr/share/doc/<package>/copyright`.

**Maintenance rule: keep the table sorted alphabetically by package name.** When adding a new row, insert it in the correct sorted position — do not append to the bottom. Use `<span style="white-space:nowrap"><code>pkg-name</code></span>` for the package name cell to prevent hyphenated names from breaking across lines (plain backticks don't render inside a `<span>`).

| Package | Upstream | Licence | Where the text lives in the .deb |
|---|---|---|---|
| <span style="white-space:nowrap"><code>asar-snes</code></span> | [RPGHacker/asar](https://github.com/RPGHacker/asar) | [LGPL-3.0+](https://github.com/RPGHacker/asar/blob/master/license-lgpl.txt) (DLL interface files separately under WTFPL) | `/usr/share/doc/asar-snes/copyright` |
| <span style="white-space:nowrap"><code>blender-asset-finder</code></span> | [wbniv/WorldFoundry](https://github.com/wbniv/WorldFoundry) | GPL-2.0-or-later | `/usr/share/doc/blender-asset-finder/copyright` |
| <span style="white-space:nowrap"><code>blender-asset-finder-cli</code></span> | [wbniv/WorldFoundry](https://github.com/wbniv/WorldFoundry) | GPL-2.0-or-later | `/usr/share/doc/blender-asset-finder-cli/copyright` |
| <span style="white-space:nowrap"><code>bsnes-jg</code></span> | [jgemu/bsnes](https://gitlab.com/jgemu/bsnes) | [GPL-3.0+](https://gitlab.com/jgemu/bsnes/-/blob/master/COPYING) (bundles SameBoy/Expat, libco/ISC, byuuML/Zlib, snes_spc/LGPL-2.1+) | `/usr/share/doc/bsnes-jg/copyright` |
| <span style="white-space:nowrap"><code>drmon</code></span> | [developer-resources-co/drdevtools](https://github.com/developer-resources-co/drdevtools) | [GPL-2.0](https://github.com/developer-resources-co/drdevtools/blob/main/COPYING) | `/usr/share/doc/drmon/copyright` |
| <span style="white-space:nowrap"><code>f9dasm</code></span> | [Arakula/f9dasm](https://github.com/Arakula/f9dasm) | [GPL-2.0+](https://github.com/Arakula/f9dasm) | `/usr/share/doc/f9dasm/copyright` |
| <span style="white-space:nowrap"><code>ghidra</code></span> | [NSA / Ghidra](https://ghidra-sre.org/) | [Apache-2.0](https://github.com/NationalSecurityAgency/ghidra/blob/master/LICENSE) | `/usr/share/doc/ghidra/copyright` |
| <span style="white-space:nowrap"><code>ldtk</code></span> | [deepnight/ldtk](https://github.com/deepnight/ldtk) | [MIT](https://github.com/deepnight/ldtk/blob/master/LICENSE) | `/usr/share/doc/ldtk/copyright` |
| <span style="white-space:nowrap"><code>libvgm</code></span> | [ValleyBell/libvgm](https://github.com/ValleyBell/libvgm) | [GPL-2.0+](https://github.com/ValleyBell/libvgm/blob/master/LICENSE) (bundles BSD-3-Clause / LGPL-2.1+ components) | `/usr/share/doc/libvgm/copyright` |
| <span style="white-space:nowrap"><code>m8te</code></span> | [nesdoug/M8TE](https://github.com/nesdoug/M8TE) | [MIT](https://github.com/nesdoug/M8TE/blob/main/LICENSE) | `/usr/share/doc/m8te/copyright` |
| <span style="white-space:nowrap"><code>mesen2</code></span> | [SourMesen/Mesen2](https://github.com/SourMesen/Mesen2) | [GPL-3.0](https://github.com/SourMesen/Mesen2/blob/master/LICENSE) (pre-built binary bundles Avalonia / SkiaSharp / .NET runtime, MIT) | `/usr/share/doc/mesen2/copyright` |
| <span style="white-space:nowrap"><code>ppsspp</code></span> | [hrydgard/ppsspp](https://github.com/hrydgard/ppsspp) | [GPL-2.0+](https://github.com/hrydgard/ppsspp/blob/master/LICENSE.TXT) (bundles BSD-3-Clause components) | `/usr/share/doc/ppsspp/copyright` |
| <span style="white-space:nowrap"><code>pvsneslib</code></span> | [alekmaul/pvsneslib](https://github.com/alekmaul/pvsneslib) | [Zlib](https://github.com/alekmaul/pvsneslib/blob/master/LICENSE) (bundles a pinned [WLA DX](https://github.com/vhelin/wla-dx) under GPL-2.0+ in `devkitsnes/bin/`) | `/usr/share/doc/pvsneslib-core/copyright` |
| <span style="white-space:nowrap"><code>python3-glfw</code></span> | [FlorianRhiem/pyGLFW](https://github.com/FlorianRhiem/pyGLFW) | [MIT](https://github.com/FlorianRhiem/pyGLFW/blob/master/LICENSE) | `/usr/share/doc/python3-glfw/copyright` |
| <span style="white-space:nowrap"><code>python3-librosa</code></span> | [librosa/librosa](https://github.com/librosa/librosa) | [ISC](https://github.com/librosa/librosa/blob/main/LICENSE.md) | `/usr/share/doc/python3-librosa/copyright` |
| <span style="white-space:nowrap"><code>python3-mss</code></span> | [BoboTiG/python-mss](https://github.com/BoboTiG/python-mss) | [MIT](https://github.com/BoboTiG/python-mss/blob/master/LICENSE.txt) | `/usr/share/doc/python3-mss/copyright` |
| <span style="white-space:nowrap"><code>python3-pydub</code></span> | [jiaaro/pydub](https://github.com/jiaaro/pydub) | [MIT](https://github.com/jiaaro/pydub/blob/master/LICENSE) | `/usr/share/doc/python3-pydub/copyright` |
| <span style="white-space:nowrap"><code>ruff</code></span> | [astral-sh/ruff](https://github.com/astral-sh/ruff) | [MIT](https://github.com/astral-sh/ruff/blob/main/LICENSE) | `/usr/share/doc/ruff/copyright` |
| <span style="white-space:nowrap"><code>snes9x-gtk</code></span> | [snes9xgit/snes9x](https://github.com/snes9xgit/snes9x) | [Snes9x non-commercial](https://github.com/snes9xgit/snes9x/blob/master/LICENSE) ⚠️ (bundles GPL-3+ / LGPL-2.1+ components) | `/usr/share/doc/snes9x-gtk/copyright` |
| <span style="white-space:nowrap"><code>task</code></span> | [go-task/task](https://github.com/go-task/task) | [MIT](https://github.com/go-task/task/blob/main/LICENSE) | `/usr/share/doc/task/copyright` |
| <span style="white-space:nowrap"><code>tilemap-studio</code></span> | [Rangi42/tilemap-studio](https://github.com/Rangi42/tilemap-studio) | [LGPL-3.0](https://github.com/Rangi42/tilemap-studio/blob/master/LICENSE) + [FLTK licence](https://www.fltk.org/doc-1.4/license.html) (statically linked) | `/usr/share/doc/tilemap-studio/copyright` |
| <span style="white-space:nowrap"><code>vgmstream</code></span> | [vgmstream/vgmstream](https://github.com/vgmstream/vgmstream) | [ISC](https://github.com/vgmstream/vgmstream/blob/master/COPYING) | `/usr/share/doc/vgmstream/copyright` |
| <span style="white-space:nowrap"><code>wla-dx</code></span> | [vhelin/wla-dx](https://github.com/vhelin/wla-dx) | [GPL-2.0+](https://github.com/vhelin/wla-dx/blob/master/COPYING) | `/usr/share/doc/wla-dx/copyright` |

> ⚠️ **`snes9x-gtk` ships under the Snes9x licence, which forbids commercial redistribution.** The source may be freely distributed for non-commercial use; binaries are likewise non-commercial. It is fine for Foundry Linux's free, non-commercial apt repo, but it must never be bundled into a paid product. See the [upstream LICENSE](https://github.com/snes9xgit/snes9x/blob/master/LICENSE).

## Retired entries

| Package | Reason |
|---|---|
| `xa65` | Ubuntu 26.04 universe ships it as `xa65 2.4.1-0.1build1` — same upstream code as we briefly vendored under `2.4.1-1foundry1`. Phase 0's `install-foundry-retro-tools.sh` now apt-installs `xa65` directly; `foundry-retro-tools` `Depends: xa65` resolves to the universe entry. Retirement plan: [`../docs/plans/2026-05-18-retire-xa65.md`](../../docs/plans/2026-05-18-retire-xa65.md). |

> **Note on `task`:** earlier retired in favour of the official Cloudsmith apt repo, it was **re-vendored** into this repo (`debian/changelog`, commit `23809cc`) so the `apt.foundrylinux.org` repo is self-contained and `foundry-core` can `Depends: task` without wiring a third apt source.

## Foundry-authored content

- Metapackage source trees (`packages/<name>/debian/`) — MIT for the packaging metadata itself
- Helper scripts (`scripts/*.sh`) — GPL-2.0
- aptly config + GitHub Actions workflows — GPL-2.0

The full project licence is [GPL-2.0](LICENSE), matching the World Foundry engine.
