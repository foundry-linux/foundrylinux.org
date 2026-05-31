# Appendix: Installed Package Manifest (foundry-anvil 0.9.33)

Every package in the built **anvil** ISO's root filesystem (`/live/filesystem.squashfs`), grouped by source apt repo, then sorted by installed size (largest first) within each repo. Post-prune — packages stripped by `strip.list.chroot.purge` and hook `0020` are already gone. Metapackages (dependency-list only, no files of their own) are shown in *italic* with a <span style="font-size:0.8em;background:#e0e0e0;color:#555;border-radius:3px;padding:1px 4px">meta</span> badge.

- **Source:** `dpkg` status + `/var/lib/apt/lists/` from `foundry-anvil-0.9.33-amd64.iso` (ISO 4.6 GB, squashfs 4.3 GiB)
- **Packages installed:** 2,599 (16 metapackages)
- **Total installed size:** 8.62 GiB (9,043,157 KiB) — uncompressed on-disk footprint

> Generated from a specific build — regenerate after any package-list or metapackage change. Installed-Size is the uncompressed footprint reported by dpkg (not download size). Repo origin resolved by matching each installed `(package, version)` against the ISO's own apt index.

## By repo (rollup)

| Repo | Packages | Metapackages | Installed Size |
|------|---------:|-------------:|---------------:|
| Ubuntu universe | 1,158 | 2 | 4.17 GiB |
| Ubuntu main | 1,407 | 3 | 3.46 GiB |
| apt.foundrylinux.org | 19 | 8 | 923.6 MiB |
| Cloudsmith (go-task) | 1 | — | 47.9 MiB |
| Ubuntu multiverse | 1 | — | 40.8 MiB |
| apt.worldfoundry.org | 13 | 3 | 4.3 MiB |
| **Total** | **2,599** | **16** | **8.62 GiB** |

## Full manifest (grouped, size desc within each repo)

<details>
<summary><strong>Ubuntu universe</strong> — 1,158 packages · 2 meta · 4.17 GiB</summary>

<table>
<thead><tr><th align="right">#</th><th align="left">Package</th><th align="left">Version</th><th align="left">Description</th><th align="right">Installed Size</th></tr></thead>
<tbody>
<tr><td align="right">1</td><td><code>mame</code></td><td>0.285+dfsg1-1</td><td>Multiple Arcade Machine Emulator (MAME)</td><td align="right">433.7 MiB</td></tr>
<tr><td align="right">2</td><td><code>plasma-workspace-wallpapers</code></td><td>4:6.6.4-0ubuntu1</td><td>Wallpapers for Plasma 6</td><td align="right">217.1 MiB</td></tr>
<tr><td align="right">3</td><td><code>libqt6webenginecore6</code></td><td>6.10.2+dfsg-1</td><td>Qt 6 WebEngine Core library</td><td align="right">197.9 MiB</td></tr>
<tr><td align="right">4</td><td><code>openjdk-21-jre-headless</code></td><td>21.0.11+10-1~26.04.2</td><td>OpenJDK Java runtime, using Hotspot JIT (headless)</td><td align="right">194.1 MiB</td></tr>
<tr><td align="right">5</td><td><code>blender</code></td><td>5.0.1+dfsg-1ubuntu1</td><td>Very fast and versatile 3D modeller/renderer</td><td align="right">155.0 MiB</td></tr>
<tr><td align="right">6</td><td><code>mame-data</code></td><td>0.285+dfsg1-1</td><td>Multiple Arcade Machine Emulator (MAME) -- data files</td><td align="right">119.0 MiB</td></tr>
<tr><td align="right">7</td><td><code>scummvm</code></td><td>2026.1.0+dfsg-1build1</td><td>engine for several graphical adventure games</td><td align="right">116.7 MiB</td></tr>
<tr><td align="right">8</td><td><code>openjdk-21-jdk-headless</code></td><td>21.0.11+10-1~26.04.2</td><td>OpenJDK Development Kit (JDK) (headless)</td><td align="right">92.0 MiB</td></tr>
<tr><td align="right">9</td><td><code>scummvm-data</code></td><td>2026.1.0+dfsg-1build1</td><td>engine for several graphical adventure games (data files)</td><td align="right">84.8 MiB</td></tr>
<tr><td align="right">10</td><td><code>blender-data</code></td><td>5.0.1+dfsg-1ubuntu1</td><td>Very fast and versatile 3D modeller/renderer - data package</td><td align="right">66.1 MiB</td></tr>
<tr><td align="right">11</td><td><code>python3-scipy</code></td><td>1.16.3-4build1</td><td>scientific tools for Python 3</td><td align="right">65.3 MiB</td></tr>
<tr><td align="right">12</td><td><code>kf6-breeze-icon-theme</code></td><td>6.24.0-0ubuntu1</td><td>Default Plasma icon theme</td><td align="right">57.7 MiB</td></tr>
<tr><td align="right">13</td><td><code>libqt6webengine6-data</code></td><td>6.10.2+dfsg-1</td><td>Qt 6 Web content engine library - data</td><td align="right">52.1 MiB</td></tr>
<tr><td align="right">14</td><td><code>python3-matplotlib</code></td><td>3.10.7+dfsg1-2build1</td><td>Python based plotting system in a style similar to Matlab</td><td align="right">44.9 MiB</td></tr>
<tr><td align="right">15</td><td><code>libopenblas0-pthread</code></td><td>0.3.32+ds-5</td><td>Optimized BLAS (linear algebra) library (shared lib, pthread)</td><td align="right">44.0 MiB</td></tr>
<tr><td align="right">16</td><td><code>unicode-data</code></td><td>16.0.0-1build1</td><td>Property data for the Unicode character set</td><td align="right">43.0 MiB</td></tr>
<tr><td align="right">17</td><td><code>libembree4-4</code></td><td>4.4.0+dfsg-1</td><td>High Performance Ray Tracing Kernels - runtime</td><td align="right">38.8 MiB</td></tr>
<tr><td align="right">18</td><td><code>breeze-wallpaper</code></td><td>4:6.6.4-0ubuntu1</td><td>Default Plasma wallpaper</td><td align="right">38.3 MiB</td></tr>
<tr><td align="right">19</td><td><code>libgdal38</code></td><td>3.12.2+dfsg-1build2</td><td>Geospatial Data Abstraction Library</td><td align="right">35.9 MiB</td></tr>
<tr><td align="right">20</td><td><code>plasma-workspace-data</code></td><td>4:6.6.4-0ubuntu2</td><td>Plasma Workspace data files</td><td align="right">33.4 MiB</td></tr>
<tr><td align="right">21</td><td><code>libopenvdb10.0t64</code></td><td>10.0.1-3</td><td>Sparse Volume Processing toolkit - lib</td><td align="right">31.6 MiB</td></tr>
<tr><td align="right">22</td><td><code>cc65</code></td><td>2.19-2build1</td><td>complete cross development package for 65(C)02 systems</td><td align="right">31.3 MiB</td></tr>
<tr><td align="right">23</td><td><code>cryfs</code></td><td>1.0.3-1</td><td>encrypt your files and store them in the cloud</td><td align="right">30.6 MiB</td></tr>
<tr><td align="right">24</td><td><code>breeze-cursor-theme</code></td><td>4:6.6.4-0ubuntu1</td><td>Default Plasma cursor theme</td><td align="right">29.8 MiB</td></tr>
<tr><td align="right">25</td><td><code>libdcmtk19</code></td><td>3.7.0+really3.6.9-1</td><td>OFFIS DICOM toolkit runtime libraries</td><td align="right">29.3 MiB</td></tr>
<tr><td align="right">26</td><td><code>plasma-desktop-data</code></td><td>4:6.6.4-0ubuntu1</td><td>Tools and widgets for the desktop data files</td><td align="right">28.7 MiB</td></tr>
<tr><td align="right">27</td><td><code>kate-data</code></td><td>4:25.12.3-0ubuntu1</td><td>shared data files for Kate text editor</td><td align="right">27.3 MiB</td></tr>
<tr><td align="right">28</td><td><code>libavcodec62</code></td><td>7:8.0.1-3ubuntu2</td><td>FFmpeg library with de/encoders for audio/video codecs - runtime files</td><td align="right">27.0 MiB</td></tr>
<tr><td align="right">29</td><td><code>libflite1</code></td><td>2.2-7build1</td><td>Small run-time speech synthesis engine - shared libraries</td><td align="right">26.9 MiB</td></tr>
<tr><td align="right">30</td><td><code>python3-sympy</code></td><td>1.14.0-2</td><td>Computer Algebra System (CAS) in Python (Python 3)</td><td align="right">26.3 MiB</td></tr>
<tr><td align="right">31</td><td><code>plasma-workspace</code></td><td>4:6.6.4-0ubuntu2</td><td>Plasma Workspace for KF6</td><td align="right">25.1 MiB</td></tr>
<tr><td align="right">32</td><td><code>libkf6breezeicons6</code></td><td>6.24.0-0ubuntu1</td><td>Default Plasma icon theme</td><td align="right">24.9 MiB</td></tr>
<tr><td align="right">33</td><td><code>proj-data</code></td><td>9.7.1-1</td><td>Cartographic projection filter and library (datum package)</td><td align="right">23.4 MiB</td></tr>
<tr><td align="right">34</td><td><code>spirv-tools</code></td><td>2026.1-1</td><td>API and commands for processing SPIR-V modules (tools)</td><td align="right">22.5 MiB</td></tr>
<tr><td align="right">35</td><td><code>libradare2-6.0.0t64</code></td><td>6.0.7+ds-1</td><td>libraries from the radare2 suite</td><td align="right">20.8 MiB</td></tr>
<tr><td align="right">36</td><td><code>kde-style-oxygen-qt6</code></td><td>4:6.6.4-0ubuntu1</td><td>Qt 6 decoration for the Oxygen desktop theme</td><td align="right">20.6 MiB</td></tr>
<tr><td align="right">37</td><td><code>mednafen</code></td><td>1.32.1+dfsg-2build2</td><td>multi-platform emulator, including NES, GB/A, Lynx, PC Engine</td><td align="right">19.8 MiB</td></tr>
<tr><td align="right">38</td><td><code>kio</code></td><td>5.116.0-2</td><td>resource and network access abstraction</td><td align="right">19.2 MiB</td></tr>
<tr><td align="right">39</td><td><code>libqt6webenginecore6-bin</code></td><td>6.10.2+dfsg-1</td><td>Qt 6 WebEngine Core binaries</td><td align="right">18.2 MiB</td></tr>
<tr><td align="right">40</td><td><code>mame-tools</code></td><td>0.285+dfsg1-1</td><td>Tools for MAME</td><td align="right">18.1 MiB</td></tr>
<tr><td align="right">41</td><td><code>hatari</code></td><td>2.6.1+dfsg-2</td><td>Emulator for the Atari ST, STE, TT, and Falcon computers</td><td align="right">18.1 MiB</td></tr>
<tr><td align="right">42</td><td><code>python3-pyqt6</code></td><td>6.10.2-2build5</td><td>Python bindings for Qt 6</td><td align="right">16.7 MiB</td></tr>
<tr><td align="right">43</td><td><code>libx265-215</code></td><td>4.1-4</td><td>H.265/HEVC video stream encoder (shared library)</td><td align="right">16.4 MiB</td></tr>
<tr><td align="right">44</td><td><code>intel-media-va-driver</code></td><td>26.1.2+dfsg1-1</td><td>VAAPI driver for the Intel GEN8+ Graphics family</td><td align="right">16.2 MiB</td></tr>
<tr><td align="right">45</td><td><code>libcodec2-1.2</code></td><td>1.2.0-4</td><td>Codec2 runtime library</td><td align="right">16.1 MiB</td></tr>
<tr><td align="right">46</td><td><code>libpyside6-py3-6.10</code></td><td>6.10.2-6ubuntu1</td><td>Python 3 bindings for Qt 6 (base files)</td><td align="right">15.9 MiB</td></tr>
<tr><td align="right">47</td><td><code>libkf6i18n-data</code></td><td>6.24.0-0ubuntu1</td><td>Advanced internationalization framework</td><td align="right">15.8 MiB</td></tr>
<tr><td align="right">48</td><td><code>libkf5i18n-data</code></td><td>5.116.0-1ubuntu4</td><td>Advanced internationalization framework.</td><td align="right">15.7 MiB</td></tr>
<tr><td align="right">49</td><td><code>python3-pyqt5</code></td><td>5.15.11+dfsg-3build3</td><td>Python 3 bindings for Qt5</td><td align="right">15.7 MiB</td></tr>
<tr><td align="right">50</td><td><code>qt6-translations-l10n</code></td><td>6.10.2-1</td><td>translations for Qt 6</td><td align="right">15.3 MiB</td></tr>
<tr><td align="right">51</td><td><code>libavfilter11</code></td><td>7:8.0.1-3ubuntu2</td><td>FFmpeg library containing media filters - runtime files</td><td align="right">15.0 MiB</td></tr>
<tr><td align="right">52</td><td><code>qttranslations5-l10n</code></td><td>5.15.18-1</td><td>translations for Qt 5</td><td align="right">14.9 MiB</td></tr>
<tr><td align="right">53</td><td><code>docbook-xsl</code></td><td>1.79.2+dfsg-8</td><td>stylesheets for processing DocBook XML to various output formats</td><td align="right">14.5 MiB</td></tr>
<tr><td align="right">54</td><td><code>kwin-data</code></td><td>4:6.6.4-0ubuntu1</td><td>KDE window manager data files</td><td align="right">14.4 MiB</td></tr>
<tr><td align="right">55</td><td><code>kio6</code></td><td>6.24.0-0ubuntu1</td><td>resource and network access abstraction</td><td align="right">14.4 MiB</td></tr>
<tr><td align="right">56</td><td><code>fs-uae</code></td><td>3.2.35-2</td><td>Cross-platform Amiga emulator based on UAE/WinUAE</td><td align="right">14.3 MiB</td></tr>
<tr><td align="right">57</td><td><code>plasma-nm</code></td><td>4:6.6.4-0ubuntu1</td><td>Plasma network connections management</td><td align="right">13.6 MiB</td></tr>
<tr><td align="right">58</td><td><code>neochat</code></td><td>25.12.3-0ubuntu1</td><td>Matrix client for desktop and mobile made by KDE</td><td align="right">13.3 MiB</td></tr>
<tr><td align="right">59</td><td><code>libqt5gui5t64</code></td><td>5.15.18+dfsg-1ubuntu1</td><td>Qt 5 GUI module</td><td align="right">12.9 MiB</td></tr>
<tr><td align="right">60</td><td><code>plasma-desktop</code></td><td>4:6.6.4-0ubuntu1</td><td>Tools and widgets for the desktop</td><td align="right">12.8 MiB</td></tr>
<tr><td align="right">61</td><td><code>vlc-plugin-base</code></td><td>3.0.23-1</td><td>multimedia player and streamer (base plugins)</td><td align="right">12.1 MiB</td></tr>
<tr><td align="right">62</td><td><code>dosbox-x-data</code></td><td>2026.01.02+dfsg-2</td><td>DOS emulator with accurate hardware emulation - data</td><td align="right">11.9 MiB</td></tr>
<tr><td align="right">63</td><td><code>dosbox-x</code></td><td>2026.01.02+dfsg-2</td><td>DOS emulator with complete, accurate hardware emulation</td><td align="right">11.7 MiB</td></tr>
<tr><td align="right">64</td><td><code>libqt6quickcontrols2-6</code></td><td>6.10.2+dfsg-3</td><td>Qt 6 Quick Controls 2 library</td><td align="right">11.4 MiB</td></tr>
<tr><td align="right">65</td><td><code>libkf6unitconversion-data</code></td><td>6.24.0-0ubuntu1</td><td>Support for unit conversion</td><td align="right">11.3 MiB</td></tr>
<tr><td align="right">66</td><td><code>libqt6gui6</code></td><td>6.10.2+dfsg-7</td><td>Qt 6 GUI module</td><td align="right">11.2 MiB</td></tr>
<tr><td align="right">67</td><td><code>yt-dlp</code></td><td>2026.03.17-1</td><td>downloader of videos from YouTube and other sites</td><td align="right">11.0 MiB</td></tr>
<tr><td align="right">68</td><td><code>openmsx</code></td><td>21.0+dfsg-3</td><td>MSX emulator that aims for perfection</td><td align="right">11.0 MiB</td></tr>
<tr><td align="right">69</td><td><code>python3-fonttools</code></td><td>4.61.1-3build1</td><td>Converts OpenType and TrueType fonts to and from XML (Python 3 Library)</td><td align="right">11.0 MiB</td></tr>
<tr><td align="right">70</td><td><code>libgdcm3.0t64</code></td><td>3.0.24-9ubuntu1</td><td>Grassroots DICOM runtime libraries</td><td align="right">10.9 MiB</td></tr>
<tr><td align="right">71</td><td><code>libcapstone-dev</code></td><td>5.0.7-2</td><td>lightweight multi-architecture disassembly framework - devel files</td><td align="right">10.7 MiB</td></tr>
<tr><td align="right">72</td><td><code>tesseract-ocr-osd</code></td><td>1:4.1.0-2build1</td><td>tesseract-ocr language files for script and orientation</td><td align="right">10.1 MiB</td></tr>
<tr><td align="right">73</td><td><code>libopenimageio2.5</code></td><td>2.5.19.1+dfsg-2</td><td>Library for reading and writing images - runtime</td><td align="right">9.9 MiB</td></tr>
<tr><td align="right">74</td><td><code>kate</code></td><td>4:25.12.3-0ubuntu1</td><td>powerful text editor</td><td align="right">9.9 MiB</td></tr>
<tr><td align="right">75</td><td><code>libradare2-common</code></td><td>6.0.7+ds-1</td><td>arch independent files from the radare2 suite</td><td align="right">9.9 MiB</td></tr>
<tr><td align="right">76</td><td><code>gocryptfs</code></td><td>2.6.1-1</td><td>Encrypted overlay filesystem written in Go</td><td align="right">9.8 MiB</td></tr>
<tr><td align="right">77</td><td><code>aptitude-common</code></td><td>0.8.13-7ubuntu5</td><td>architecture independent files for the aptitude package manager</td><td align="right">9.8 MiB</td></tr>
<tr><td align="right">78</td><td><code>libsdl3-dev</code></td><td>3.4.2+ds-1ubuntu1</td><td>Simple DirectMedia Layer, version 3 development files</td><td align="right">9.6 MiB</td></tr>
<tr><td align="right">79</td><td><code>libkf6texteditor-data</code></td><td>6.24.0-0ubuntu1</td><td>provide advanced plain text editing services</td><td align="right">9.5 MiB</td></tr>
<tr><td align="right">80</td><td><code>libplacebo360</code></td><td>7.360.0-3</td><td>GPU-accelerated video/image rendering primitives (shared library)</td><td align="right">9.5 MiB</td></tr>
<tr><td align="right">81</td><td><code>stella</code></td><td>7.0+dfsg-2build1</td><td>Atari 2600 Emulator for SDL &amp; the X Window System</td><td align="right">9.4 MiB</td></tr>
<tr><td align="right">82</td><td><code>libshaderc1</code></td><td>2026.1-1</td><td>Library API for accessing glslc functionality - shared libraries</td><td align="right">9.0 MiB</td></tr>
<tr><td align="right">83</td><td><code>libtiled1</code></td><td>1.11.90-1</td><td>library for general purpose tile map editor</td><td align="right">9.0 MiB</td></tr>
<tr><td align="right">84</td><td><code>libcapstone5</code></td><td>5.0.7-2</td><td>lightweight multi-architecture disassembly framework - library</td><td align="right">8.9 MiB</td></tr>
<tr><td align="right">85</td><td><code>python-matplotlib-data</code></td><td>3.10.7+dfsg1-2build1</td><td>Python based plotting system (data package)</td><td align="right">8.9 MiB</td></tr>
<tr><td align="right">86</td><td><code>libkwin6</code></td><td>4:6.6.4-0ubuntu1</td><td>KDE window manager library</td><td align="right">8.7 MiB</td></tr>
<tr><td align="right">87</td><td><code>glslang-tools</code></td><td>16.2.0-2</td><td>OpenGL and OpenGL ES shader front end and validator -- tools</td><td align="right">8.5 MiB</td></tr>
<tr><td align="right">88</td><td><code>kdegames-card-data</code></td><td>4:25.12.3-0ubuntu1</td><td>card decks for KDE games</td><td align="right">8.5 MiB</td></tr>
<tr><td align="right">89</td><td><code>netpbm</code></td><td>2:11.10.02-1build1</td><td>Graphics conversion tools between image formats</td><td align="right">8.4 MiB</td></tr>
<tr><td align="right">90</td><td><code>khelpcenter-data</code></td><td>4:25.12.3-0ubuntu1</td><td>search and read system documentation data</td><td align="right">8.3 MiB</td></tr>
<tr><td align="right">91</td><td><code>libqt6widgets6</code></td><td>6.10.2+dfsg-7</td><td>Qt 6 widgets module</td><td align="right">8.3 MiB</td></tr>
<tr><td align="right">92</td><td><code>libqt6quick6</code></td><td>6.10.2+dfsg-3</td><td>Qt 6 Quick library</td><td align="right">8.3 MiB</td></tr>
<tr><td align="right">93</td><td><code>calamares</code></td><td>3.3.14-0ubuntu25</td><td>distribution-independent installer framework</td><td align="right">7.8 MiB</td></tr>
<tr><td align="right">94</td><td><code>libsvtav1enc2</code></td><td>2.3.0+dfsg-1build1</td><td>Scalable Video Technology for AV1 (libsvtav1enc shared library)</td><td align="right">7.7 MiB</td></tr>
<tr><td align="right">95</td><td><code>libspatialite8t64</code></td><td>5.1.0-3ubuntu2</td><td>Geospatial extension for SQLite - libraries</td><td align="right">7.6 MiB</td></tr>
<tr><td align="right">96</td><td><code>okular-data</code></td><td>4:25.12.3-0ubuntu1</td><td>universal document viewer - data files</td><td align="right">7.6 MiB</td></tr>
<tr><td align="right">97</td><td><code>dolphin-data</code></td><td>4:25.12.3-0ubuntu1</td><td>file manager - data</td><td align="right">7.5 MiB</td></tr>
<tr><td align="right">98</td><td><code>libqt5widgets5t64</code></td><td>5.15.18+dfsg-1ubuntu1</td><td>Qt 5 widgets module</td><td align="right">7.4 MiB</td></tr>
<tr><td align="right">99</td><td><code>konsole</code></td><td>4:25.12.3-0ubuntu1</td><td>X terminal emulator</td><td align="right">7.3 MiB</td></tr>
<tr><td align="right">100</td><td><code>plasma-widgets-addons</code></td><td>4:6.6.4-0ubuntu1</td><td>additional widgets for Plasma 6</td><td align="right">7.3 MiB</td></tr>
<tr><td align="right">101</td><td><code>libopencolorio2.5</code></td><td>2.5.1+dfsg-1</td><td>complete color management solution - runtime</td><td align="right">7.2 MiB</td></tr>
<tr><td align="right">102</td><td><code>python3-opengl</code></td><td>3.1.10+dfsg-1</td><td>Python bindings to OpenGL (Python 3)</td><td align="right">7.1 MiB</td></tr>
<tr><td align="right">103</td><td><code>libmagickcore-7.q16-10</code></td><td>8:7.1.2.18+dfsg1-1</td><td>low-level image manipulation library -- quantum depth Q16</td><td align="right">7.1 MiB</td></tr>
<tr><td align="right">104</td><td><code>partitionmanager</code></td><td>25.12.3-0ubuntu1</td><td>file, disk and partition management for KDE</td><td align="right">7.1 MiB</td></tr>
<tr><td align="right">105</td><td><code>libgnustep-base1.31</code></td><td>1.31.1-4ubuntu2</td><td>GNUstep Base library</td><td align="right">6.7 MiB</td></tr>
<tr><td align="right">106</td><td><code>elisa</code></td><td>25.12.3-0ubuntu1</td><td>Simple music player with a focus on Plasma desktop integration and privacy</td><td align="right">6.5 MiB</td></tr>
<tr><td align="right">107</td><td><code>ksystemlog</code></td><td>4:25.12.3-0ubuntu1</td><td>system log viewer</td><td align="right">6.5 MiB</td></tr>
<tr><td align="right">108</td><td><code>7zip</code></td><td>26.00+dfsg-1</td><td>7-Zip file archiver with a high compression ratio</td><td align="right">6.5 MiB</td></tr>
<tr><td align="right">109</td><td><code>kde-spectacle</code></td><td>4:6.6.4-0ubuntu1</td><td>Screenshot capture utility</td><td align="right">6.4 MiB</td></tr>
<tr><td align="right">110</td><td><code>libqt6core6t64</code></td><td>6.10.2+dfsg-7</td><td>Qt 6 core module</td><td align="right">6.4 MiB</td></tr>
<tr><td align="right">111</td><td><code>libqt6qml6</code></td><td>6.10.2+dfsg-3</td><td>Qt 6 QML library</td><td align="right">6.4 MiB</td></tr>
<tr><td align="right">112</td><td><code>libsdl2-dev</code></td><td>2.32.10+dfsg-6</td><td>Simple DirectMedia Layer development files</td><td align="right">6.3 MiB</td></tr>
<tr><td align="right">113</td><td><code>libqalculate23</code></td><td>5.9.0-1</td><td>Powerful and easy to use desktop calculator - library</td><td align="right">6.2 MiB</td></tr>
<tr><td align="right">114</td><td><code>okular-doc</code></td><td>4:25.12.3-0ubuntu1</td><td>universal document viewer - manual</td><td align="right">6.2 MiB</td></tr>
<tr><td align="right">115</td><td><code>kwin-common</code></td><td>4:6.6.4-0ubuntu1</td><td>KDE window manager, common files</td><td align="right">6.2 MiB</td></tr>
<tr><td align="right">116</td><td><code>libqt5quick5</code></td><td>5.15.18+dfsg-2</td><td>Qt 5 Quick library</td><td align="right">6.1 MiB</td></tr>
<tr><td align="right">117</td><td><code>kdeconnect</code></td><td>25.12.3-0ubuntu1</td><td>connect smartphones to your desktop devices</td><td align="right">6.1 MiB</td></tr>
<tr><td align="right">118</td><td><code>libqt5core5t64</code></td><td>5.15.18+dfsg-1ubuntu1</td><td>Qt 5 core module</td><td align="right">6.0 MiB</td></tr>
<tr><td align="right">119</td><td><code>gwenview-doc</code></td><td>4:25.12.3-0ubuntu1</td><td>documentation for Gwenview</td><td align="right">6.0 MiB</td></tr>
<tr><td align="right">120</td><td><code>plasma-desktoptheme</code></td><td>6.6.4-0ubuntu1</td><td>Plasma Runtime components</td><td align="right">6.0 MiB</td></tr>
<tr><td align="right">121</td><td><code>openmsx-data</code></td><td>21.0+dfsg-3</td><td>datafiles for openMSX, an MSX emulator</td><td align="right">6.0 MiB</td></tr>
<tr><td align="right">122</td><td><code>libopenexr-3-1-30</code></td><td>3.1.13-2build1</td><td>runtime files for the OpenEXR image library</td><td align="right">6.0 MiB</td></tr>
<tr><td align="right">123</td><td><code>kpat</code></td><td>4:25.12.3-0ubuntu1</td><td>solitaire card games</td><td align="right">6.0 MiB</td></tr>
<tr><td align="right">124</td><td><code>timgm6mb-soundfont</code></td><td>1.3-5build1</td><td>TimGM6mb SoundFont from MuseScore 1.3</td><td align="right">5.9 MiB</td></tr>
<tr><td align="right">125</td><td><code>kio-extras-data</code></td><td>4:25.12.3-0ubuntu1</td><td>Extra functionality for kioslaves data files</td><td align="right">5.9 MiB</td></tr>
<tr><td align="right">126</td><td><code>libplasma5support-data</code></td><td>4:6.6.4-0ubuntu1</td><td>support components for porting from KF5/Qt5 to KF6/Qt6 - data files</td><td align="right">5.9 MiB</td></tr>
<tr><td align="right">127</td><td><code>libqt6designer6</code></td><td>6.10.2-1</td><td>Qt 6 Designer library</td><td align="right">5.8 MiB</td></tr>
<tr><td align="right">128</td><td><code>unar</code></td><td>1.10.8+ds1-9build1</td><td>Unarchiver for a variety of file formats</td><td align="right">5.8 MiB</td></tr>
<tr><td align="right">129</td><td><code>gwenview</code></td><td>4:25.12.3-0ubuntu1</td><td>image viewer by KDE</td><td align="right">5.8 MiB</td></tr>
<tr><td align="right">130</td><td><code>graphicsmagick</code></td><td>1.4+really1.3.46-2</td><td>collection of image processing tools</td><td align="right">5.7 MiB</td></tr>
<tr><td align="right">131</td><td><code>qml-module-qtquick-controls2</code></td><td>5.15.18+dfsg-1</td><td>Qt 5 Qt Quick Controls 2 QML module</td><td align="right">5.6 MiB</td></tr>
<tr><td align="right">132</td><td><code>ark</code></td><td>4:25.12.3-0ubuntu1</td><td>archive utility</td><td align="right">5.5 MiB</td></tr>
<tr><td align="right">133</td><td><code>kmahjongg</code></td><td>4:25.12.3-0ubuntu1</td><td>mahjongg solitaire game</td><td align="right">5.5 MiB</td></tr>
<tr><td align="right">134</td><td><code>libkf5widgetsaddons-data</code></td><td>5.116.0-1ubuntu1</td><td>add-on widgets and classes for applications that use the Qt Widgets module</td><td align="right">5.4 MiB</td></tr>
<tr><td align="right">135</td><td><code>libqt5designer5</code></td><td>5.15.18-1</td><td>Qt 5 designer module</td><td align="right">5.4 MiB</td></tr>
<tr><td align="right">136</td><td><code>binutils-m68k-linux-gnu</code></td><td>2.46-3ubuntu2</td><td>GNU binary utilities, for m68k-linux-gnu target</td><td align="right">5.3 MiB</td></tr>
<tr><td align="right">137</td><td><code>libopencv-imgproc410</code></td><td>4.10.0+dfsg-7ubuntu5</td><td>computer vision Image Processing library</td><td align="right">5.3 MiB</td></tr>
<tr><td align="right">138</td><td><code>libze1</code></td><td>1.28.2-2</td><td>oneAPI Level Zero -- share libraries</td><td align="right">5.1 MiB</td></tr>
<tr><td align="right">139</td><td><code>tiled</code></td><td>1.11.90-1</td><td>general purpose tile map editor</td><td align="right">5.1 MiB</td></tr>
<tr><td align="right">140</td><td><code>libqt6pdf6</code></td><td>6.10.2+dfsg-1</td><td>Qt 6 PDF library</td><td align="right">5.0 MiB</td></tr>
<tr><td align="right">141</td><td><code>libqt5qml5</code></td><td>5.15.18+dfsg-2</td><td>Qt 5 QML module</td><td align="right">4.9 MiB</td></tr>
<tr><td align="right">142</td><td><code>libqt6shadertools6</code></td><td>6.10.2-1</td><td>Qt 6 shader tools module</td><td align="right">4.8 MiB</td></tr>
<tr><td align="right">143</td><td><code>grub-theme-breeze</code></td><td>6.6.4-0ubuntu1</td><td>Breeze theme for GRUB 2</td><td align="right">4.7 MiB</td></tr>
<tr><td align="right">144</td><td><code>dcmtk-data</code></td><td>3.7.0+really3.6.9-1</td><td>OFFIS DICOM toolkit data files</td><td align="right">4.7 MiB</td></tr>
<tr><td align="right">145</td><td><code>haruna</code></td><td>1.7.1-1ubuntu3</td><td>Video player built with Qt/QML on top of libmpv</td><td align="right">4.7 MiB</td></tr>
<tr><td align="right">146</td><td><code>qt6-base-dev-tools</code></td><td>6.10.2+dfsg-7</td><td>Qt 6 base development programs</td><td align="right">4.6 MiB</td></tr>
<tr><td align="right">147</td><td><code>fceux</code></td><td>2.6.5+dfsg1-2build4</td><td>all-in-one NES/Famicom Emulator</td><td align="right">4.5 MiB</td></tr>
<tr><td align="right">148</td><td><code>libkpmcore13</code></td><td>25.12.3-0ubuntu1</td><td>KDE Partition Manager Core</td><td align="right">4.5 MiB</td></tr>
<tr><td align="right">149</td><td><code>kdeplasma-addons-data</code></td><td>4:6.6.4-0ubuntu1</td><td>locale files for kdeplasma-addons</td><td align="right">4.5 MiB</td></tr>
<tr><td align="right">150</td><td><code>dolphin-doc</code></td><td>4:25.12.3-0ubuntu1</td><td>file manager - documentation</td><td align="right">4.5 MiB</td></tr>
<tr><td align="right">151</td><td><code>libqt6quick3druntimerender6</code></td><td>6.10.2-1</td><td>Qt 6 Quick 3D Runtime Renderer library</td><td align="right">4.3 MiB</td></tr>
<tr><td align="right">152</td><td><code>libproj25</code></td><td>9.7.1-1</td><td>Cartographic projection library</td><td align="right">4.3 MiB</td></tr>
<tr><td align="right">153</td><td><code>libcrypto++8t64</code></td><td>8.9.0-2build1</td><td>General purpose cryptographic library - shared library</td><td align="right">4.2 MiB</td></tr>
<tr><td align="right">154</td><td><code>kubuntu-settings-desktop</code></td><td>1:26.04.13</td><td>Settings and artwork for the Kubuntu (Desktop)</td><td align="right">4.2 MiB</td></tr>
<tr><td align="right">155</td><td><code>okular</code></td><td>4:25.12.3-0ubuntu1</td><td>universal document viewer</td><td align="right">4.2 MiB</td></tr>
<tr><td align="right">156</td><td><code>plasma-discover-common</code></td><td>6.6.4-0ubuntu1</td><td>Discover software manager suite (common data files)</td><td align="right">4.1 MiB</td></tr>
<tr><td align="right">157</td><td><code>python3-pyqtgraph</code></td><td>0.14.0-5</td><td>Scientific Graphics and GUI Library for Python 3</td><td align="right">4.1 MiB</td></tr>
<tr><td align="right">158</td><td><code>libhdf5-310</code></td><td>1.14.6+repack-2</td><td>HDF5 C runtime files - serial version</td><td align="right">4.1 MiB</td></tr>
<tr><td align="right">159</td><td><code>libshiboken6-py3-6.10</code></td><td>6.10.2-6ubuntu1</td><td>CPython bindings generator for C++ libraries (Python3 shared library)</td><td align="right">4.1 MiB</td></tr>
<tr><td align="right">160</td><td><code>libkf6texteditor6</code></td><td>6.24.0-0ubuntu1</td><td>provide advanced plain text editing services</td><td align="right">4.1 MiB</td></tr>
<tr><td align="right">161</td><td><code>libquotientqt6-0.9</code></td><td>0.9.6.1-1ubuntu1</td><td>Qt6 library to write cross-platform clients for Matrix</td><td align="right">4.0 MiB</td></tr>
<tr><td align="right">162</td><td><code>qml6-module-qtquick-dialogs</code></td><td>6.10.2+dfsg-3</td><td>Qt 6 Quick Dialogs QML module</td><td align="right">4.0 MiB</td></tr>
<tr><td align="right">163</td><td><code>ksudoku</code></td><td>4:25.12.3-0ubuntu1</td><td>Sudoku puzzle game and solver</td><td align="right">3.9 MiB</td></tr>
<tr><td align="right">164</td><td><code>python3-pycryptodome</code></td><td>3.20.0+dfsg-3build1</td><td>cryptographic Python library (Python 3)</td><td align="right">3.9 MiB</td></tr>
<tr><td align="right">165</td><td><code>tesseract-ocr-eng</code></td><td>1:4.1.0-2build1</td><td>tesseract-ocr language files for English</td><td align="right">3.9 MiB</td></tr>
<tr><td align="right">166</td><td><code>dolphin</code></td><td>4:25.12.3-0ubuntu1</td><td>file manager</td><td align="right">3.9 MiB</td></tr>
<tr><td align="right">167</td><td><code>libopencv-core410</code></td><td>4.10.0+dfsg-7ubuntu5</td><td>computer vision core library</td><td align="right">3.9 MiB</td></tr>
<tr><td align="right">168</td><td><code>radare2</code></td><td>6.0.7+ds-1</td><td>free and advanced command line hexadecimal editor</td><td align="right">3.8 MiB</td></tr>
<tr><td align="right">169</td><td><code>drkonqi</code></td><td>6.6.4-0ubuntu1</td><td>Crash handler for Qt applications</td><td align="right">3.7 MiB</td></tr>
<tr><td align="right">170</td><td><code>kinfocenter</code></td><td>4:6.6.4-0ubuntu1</td><td>system information viewer</td><td align="right">3.6 MiB</td></tr>
<tr><td align="right">171</td><td><code>kde-cli-tools-data</code></td><td>4:6.6.4-0ubuntu1</td><td>tools to use kioslaves from the command line</td><td align="right">3.6 MiB</td></tr>
<tr><td align="right">172</td><td><code>kscreen</code></td><td>4:6.6.4-0ubuntu1</td><td>KDE monitor hotplug and screen handling</td><td align="right">3.6 MiB</td></tr>
<tr><td align="right">173</td><td><code>aptitude</code></td><td>0.8.13-7ubuntu5</td><td>terminal-based package manager</td><td align="right">3.6 MiB</td></tr>
<tr><td align="right">174</td><td><code>libxerces-c3.2t64</code></td><td>3.2.4+debian-1.3build2</td><td>validating XML parser library for C++</td><td align="right">3.5 MiB</td></tr>
<tr><td align="right">175</td><td><code>openconnect</code></td><td>9.12-3.3</td><td>open client for various network vendors SSL VPNs</td><td align="right">3.5 MiB</td></tr>
<tr><td align="right">176</td><td><code>libgraphicsmagick-q16-3t64</code></td><td>1.4+really1.3.46-2</td><td>format-independent image processing - C shared library</td><td align="right">3.5 MiB</td></tr>
<tr><td align="right">177</td><td><code>desmume</code></td><td>0.9.13-4build1</td><td>Nintendo DS emulator</td><td align="right">3.5 MiB</td></tr>
<tr><td align="right">178</td><td><code>powerdevil-data</code></td><td>4:6.6.4-0ubuntu1</td><td>data files for the KDE power management service for Plasma</td><td align="right">3.4 MiB</td></tr>
<tr><td align="right">179</td><td><code>kdoctools6</code></td><td>6.24.0-0ubuntu1</td><td>Tools to generate documentation in various formats from DocBook</td><td align="right">3.4 MiB</td></tr>
<tr><td align="right">180</td><td><code>libqt6designercomponents6</code></td><td>6.10.2-1</td><td>Qt 6 Designer Components library</td><td align="right">3.4 MiB</td></tr>
<tr><td align="right">181</td><td><code>libtesseract5</code></td><td>5.5.0-1build1</td><td>Tesseract OCR library</td><td align="right">3.3 MiB</td></tr>
<tr><td align="right">182</td><td><code>dolphin-plugins</code></td><td>4:25.12.3-0ubuntu1</td><td>plugins for Dolphin</td><td align="right">3.3 MiB</td></tr>
<tr><td align="right">183</td><td><code>kcalc</code></td><td>4:25.12.3-0ubuntu1</td><td>simple and scientific calculator</td><td align="right">3.3 MiB</td></tr>
<tr><td align="right">184</td><td><code>libgeos3.14.1</code></td><td>3.14.1-2</td><td>Geometry engine for Geographic Information Systems - C++ Library</td><td align="right">3.3 MiB</td></tr>
<tr><td align="right">185</td><td><code>librav1e0.8</code></td><td>0.8.1-7</td><td>Fastest and safest AV1 encoder - shared library</td><td align="right">3.2 MiB</td></tr>
<tr><td align="right">186</td><td><code>kwalletmanager</code></td><td>4:25.12.3-0ubuntu1</td><td>secure password wallet manager</td><td align="right">3.2 MiB</td></tr>
<tr><td align="right">187</td><td><code>libsdl3-0</code></td><td>3.4.2+ds-1ubuntu1</td><td>Simple DirectMedia Layer, version 3</td><td align="right">3.2 MiB</td></tr>
<tr><td align="right">188</td><td><code>qml6-module-org-kde-desktop</code></td><td>6.24.0-0ubuntu1</td><td>Qt Quick Controls 2: Desktop Style</td><td align="right">3.2 MiB</td></tr>
<tr><td align="right">189</td><td><code>network-manager-openconnect</code></td><td>1.2.10-4.1</td><td>network management framework (OpenConnect plugin core)</td><td align="right">3.1 MiB</td></tr>
<tr><td align="right">190</td><td><code>libavformat62</code></td><td>7:8.0.1-3ubuntu2</td><td>FFmpeg library with (de)muxers for multimedia containers - runtime files</td><td align="right">3.1 MiB</td></tr>
<tr><td align="right">191</td><td><code>qml6-module-org-kde-kirigamiaddons-formcard</code></td><td>1.11.0-2ubuntu2</td><td>formcard module for QML</td><td align="right">3.1 MiB</td></tr>
<tr><td align="right">192</td><td><code>qml6-module-org-kde-breeze</code></td><td>6.6.4-0ubuntu1</td><td>Breeze inspired QQC2 Style</td><td align="right">3.1 MiB</td></tr>
<tr><td align="right">193</td><td><code>libmpv2</code></td><td>0.41.0-2ubuntu4</td><td>video player based on MPlayer/mplayer2 (client library)</td><td align="right">3.0 MiB</td></tr>
<tr><td align="right">194</td><td><code>go-mtpfs</code></td><td>1.0.0+git20200111.42254b1-1build3</td><td>Mount MTP devices over FUSE</td><td align="right">3.0 MiB</td></tr>
<tr><td align="right">195</td><td><code>libqt6quicktemplates2-6</code></td><td>6.10.2+dfsg-3</td><td>Qt 6 Quick Templates 2 library</td><td align="right">3.0 MiB</td></tr>
<tr><td align="right">196</td><td><code>kio-extras</code></td><td>4:25.12.3-0ubuntu1</td><td>Extra functionality for kioslaves</td><td align="right">3.0 MiB</td></tr>
<tr><td align="right">197</td><td><code>kde-config-tablet</code></td><td>6.6.4-0ubuntu1</td><td>implements a KDE configuration GUI for the Wacom drivers</td><td align="right">3.0 MiB</td></tr>
<tr><td align="right">198</td><td><code>mednaffe</code></td><td>0.9.3-1build1</td><td>front-end for the Mednafen multi-system emulator</td><td align="right">3.0 MiB</td></tr>
<tr><td align="right">199</td><td><code>bluedevil</code></td><td>4:6.6.4-0ubuntu1</td><td>KDE Bluetooth stack</td><td align="right">2.9 MiB</td></tr>
<tr><td align="right">200</td><td><code>kio-audiocd</code></td><td>4:25.12.3-0ubuntu1</td><td>transparent audio CD access for applications using the KDE Platform</td><td align="right">2.9 MiB</td></tr>
<tr><td align="right">201</td><td><code>synaptic</code></td><td>0.91.7build1</td><td>Graphical package manager</td><td align="right">2.9 MiB</td></tr>
<tr><td align="right">202</td><td><code>plasma-systemmonitor</code></td><td>6.6.4-0ubuntu1</td><td>System monitor for the Plasma desktop</td><td align="right">2.9 MiB</td></tr>
<tr><td align="right">203</td><td><code>print-manager</code></td><td>6:6.6.4-0ubuntu1</td><td>printer configuration and monitoring tools</td><td align="right">2.8 MiB</td></tr>
<tr><td align="right">204</td><td><code>gdal-data</code></td><td>3.12.2+dfsg-1build2</td><td>Geospatial Data Abstraction Library - Data files</td><td align="right">2.7 MiB</td></tr>
<tr><td align="right">205</td><td><code>libleptonica6</code></td><td>1.86.0-1</td><td>image processing library</td><td align="right">2.7 MiB</td></tr>
<tr><td align="right">206</td><td><code>ffmpeg</code></td><td>7:8.0.1-3ubuntu2</td><td>Tools for transcoding, streaming and playing of multimedia files</td><td align="right">2.7 MiB</td></tr>
<tr><td align="right">207</td><td><code>libqt6network6</code></td><td>6.10.2+dfsg-7</td><td>Qt 6 network module</td><td align="right">2.7 MiB</td></tr>
<tr><td align="right">208</td><td><code>libkf6widgetsaddons6</code></td><td>6.24.0-0ubuntu1</td><td>add-on widgets and classes for applications that use the Qt Widgets module</td><td align="right">2.7 MiB</td></tr>
<tr><td align="right">209</td><td><code>xdg-desktop-portal-kde</code></td><td>6.6.4-0ubuntu1</td><td>backend implementation for xdg-desktop-portal using Qt</td><td align="right">2.7 MiB</td></tr>
<tr><td align="right">210</td><td><code>libkf5xmlgui-data</code></td><td>5.116.0-1ubuntu4</td><td>User configurable main windows.</td><td align="right">2.7 MiB</td></tr>
<tr><td align="right">211</td><td><code>kimageformat6-plugins</code></td><td>6.24.0-0ubuntu1</td><td>additional image format plugins for Qt Gui</td><td align="right">2.6 MiB</td></tr>
<tr><td align="right">212</td><td><code>konsole-kpart</code></td><td>4:25.12.3-0ubuntu1</td><td>Konsole plugin for Qt applications</td><td align="right">2.6 MiB</td></tr>
<tr><td align="right">213</td><td><code>spirv-cross</code></td><td>2021.01.15+1.4.335.0-1</td><td>Convert SPIR-V to other shader languages (CLI tool)</td><td align="right">2.5 MiB</td></tr>
<tr><td align="right">214</td><td><code>libkf6widgetsaddons-data</code></td><td>6.24.0-0ubuntu1</td><td>add-on widgets and classes for applications that use the Qt Widgets module</td><td align="right">2.5 MiB</td></tr>
<tr><td align="right">215</td><td><code>libqt5network5t64</code></td><td>5.15.18+dfsg-1ubuntu1</td><td>Qt 5 network module</td><td align="right">2.5 MiB</td></tr>
<tr><td align="right">216</td><td><code>libqt6multimedia6</code></td><td>6.10.2-2</td><td>Qt 6 Multimedia library</td><td align="right">2.5 MiB</td></tr>
<tr><td align="right">217</td><td><code>fonts-hack</code></td><td>3.003-4</td><td>Typeface designed for source code</td><td align="right">2.4 MiB</td></tr>
<tr><td align="right">218</td><td><code>libqalculate-data</code></td><td>5.9.0-1</td><td>Powerful and easy to use desktop calculator - data</td><td align="right">2.4 MiB</td></tr>
<tr><td align="right">219</td><td><code>powerdevil</code></td><td>4:6.6.4-0ubuntu1</td><td>KDE power management service for Plasma</td><td align="right">2.4 MiB</td></tr>
<tr><td align="right">220</td><td><code>libkf6xmlgui-data</code></td><td>6.24.0-0ubuntu1</td><td>User configurable main windows</td><td align="right">2.4 MiB</td></tr>
<tr><td align="right">221</td><td><code>baloo6</code></td><td>6.24.0-0ubuntu1</td><td>framework for file search and metadata management</td><td align="right">2.3 MiB</td></tr>
<tr><td align="right">222</td><td><code>kdeconnect-libs</code></td><td>25.12.3-0ubuntu1</td><td>KDE Connect internal libs</td><td align="right">2.3 MiB</td></tr>
<tr><td align="right">223</td><td><code>kmenuedit</code></td><td>4:6.6.4-0ubuntu1</td><td>XDG menu editor</td><td align="right">2.3 MiB</td></tr>
<tr><td align="right">224</td><td><code>plasma-discover</code></td><td>6.6.4-0ubuntu1</td><td>Discover software management suite</td><td align="right">2.2 MiB</td></tr>
<tr><td align="right">225</td><td><code>libqt6waylandcompositor6</code></td><td>6.10.2-4</td><td>Qt 6 Wayland Compositor library</td><td align="right">2.2 MiB</td></tr>
<tr><td align="right">226</td><td><code>plasma-pa</code></td><td>4:6.6.4-0ubuntu1</td><td>Plasma 6 Volume controller</td><td align="right">2.2 MiB</td></tr>
<tr><td align="right">227</td><td><code>libqt6location6</code></td><td>6.10.2-1</td><td>Qt 6 Location library</td><td align="right">2.2 MiB</td></tr>
<tr><td align="right">228</td><td><code>kup-backup</code></td><td>0.10.0-1ubuntu2</td><td>backup tool for KDE&#x27;s Plasma desktop</td><td align="right">2.2 MiB</td></tr>
<tr><td align="right">229</td><td><code>oxygen-sounds</code></td><td>4:6.6.4-0ubuntu1</td><td>Sounds for the Oxygen desktop theme</td><td align="right">2.2 MiB</td></tr>
<tr><td align="right">230</td><td><code>libkirigamicontrols6</code></td><td>6.24.0-0ubuntu1</td><td>set of QtQuick components targeted for mobile use</td><td align="right">2.2 MiB</td></tr>
<tr><td align="right">231</td><td><code>ocean-sound-theme</code></td><td>6.6.4-0ubuntu1</td><td>Ocean Sound Theme for the Plasma desktop</td><td align="right">2.1 MiB</td></tr>
<tr><td align="right">232</td><td><code>libplasma5support6</code></td><td>4:6.6.4-0ubuntu1</td><td>support components for porting from KF5/Qt5 to KF6/Qt6 - shared library</td><td align="right">2.1 MiB</td></tr>
<tr><td align="right">233</td><td><code>libxapian30</code></td><td>1.4.31-2</td><td>Search engine library</td><td align="right">2.1 MiB</td></tr>
<tr><td align="right">234</td><td><code>kdegames-mahjongg-data-kf6</code></td><td>4:25.12.3-0ubuntu1</td><td>tilesets and backgrounds for Mahjongg games</td><td align="right">2.1 MiB</td></tr>
<tr><td align="right">235</td><td><code>mesa-utils</code></td><td>9.0.0-2build1</td><td>Miscellaneous Mesa utilities -- symlinks</td><td align="right">2.1 MiB</td></tr>
<tr><td align="right">236</td><td><code>sddm</code></td><td>0.21.0+git20250502.4fe234b-2ubuntu3</td><td>modern display manager for X11</td><td align="right">2.1 MiB</td></tr>
<tr><td align="right">237</td><td><code>libboost-chrono1.90.0</code></td><td>1.90.0-6ubuntu1</td><td>C++ representation of time duration, time point, and clocks</td><td align="right">2.1 MiB</td></tr>
<tr><td align="right">238</td><td><code>qml6-module-qtquick-virtualkeyboard</code></td><td>6.10.2+dfsg-1</td><td>Qt 6 Virtual Keyboard QML module</td><td align="right">2.0 MiB</td></tr>
<tr><td align="right">239</td><td><code>libkf6syntaxhighlighting6</code></td><td>6.24.0-0ubuntu1</td><td>Syntax highlighting Engine</td><td align="right">2.0 MiB</td></tr>
<tr><td align="right">240</td><td><code>libqt5waylandcompositor5</code></td><td>5.15.18-1</td><td>QtWayland compositor library</td><td align="right">2.0 MiB</td></tr>
<tr><td align="right">241</td><td><code>python3-mpmath</code></td><td>1.3.0-2</td><td>library for arbitrary-precision floating-point arithmetic (Python3)</td><td align="right">1.9 MiB</td></tr>
<tr><td align="right">242</td><td><code>libksysguard-data</code></td><td>4:6.6.4-0ubuntu1</td><td>library for system monitoring - data files</td><td align="right">1.9 MiB</td></tr>
<tr><td align="right">243</td><td><code>kmines</code></td><td>4:25.12.3-0ubuntu1</td><td>minesweeper game</td><td align="right">1.9 MiB</td></tr>
<tr><td align="right">244</td><td><code>libqt6quick3d6</code></td><td>6.10.2-1</td><td>Qt 6 Quick 3D library</td><td align="right">1.9 MiB</td></tr>
<tr><td align="right">245</td><td><code>keditbookmarks</code></td><td>25.12.3-0ubuntu1</td><td>bookmarks editor utility for KDE</td><td align="right">1.9 MiB</td></tr>
<tr><td align="right">246</td><td><code>libx264-165</code></td><td>2:0.165.3222+gitb35605ac-3build1</td><td>x264 video coding library</td><td align="right">1.8 MiB</td></tr>
<tr><td align="right">247</td><td><code>libqt5quicktemplates2-5</code></td><td>5.15.18+dfsg-1</td><td>Qt 5 Quick Templates 2 library</td><td align="right">1.8 MiB</td></tr>
<tr><td align="right">248</td><td><code>python3-pyqt6.qtquick</code></td><td>6.10.2-2build5</td><td>Python bindings for Qt 6 Quick module</td><td align="right">1.8 MiB</td></tr>
<tr><td align="right">249</td><td><code>libopenmpt0t64</code></td><td>0.8.4-1</td><td>module music library based on OpenMPT -- shared library</td><td align="right">1.8 MiB</td></tr>
<tr><td align="right">250</td><td><code>qdoc-qt6</code></td><td>6.10.2-1</td><td>Qt 6 qdoc tool</td><td align="right">1.8 MiB</td></tr>
<tr><td align="right">251</td><td><code>fastfetch</code></td><td>2.57.1+dfsg-1ubuntu1</td><td>neofetch-like tool for fetching system information</td><td align="right">1.8 MiB</td></tr>
<tr><td align="right">252</td><td><code>ksystemstats</code></td><td>6.6.4-0ubuntu1</td><td>plugin based system monitoring daemon</td><td align="right">1.8 MiB</td></tr>
<tr><td align="right">253</td><td><code>libopenal1</code></td><td>1:1.25.1-2</td><td>Software implementation of the OpenAL audio API (shared library)</td><td align="right">1.8 MiB</td></tr>
<tr><td align="right">254</td><td><code>libqt6waylandclient6</code></td><td>6.10.2+dfsg-7</td><td>Qt 6 Wayland client module</td><td align="right">1.8 MiB</td></tr>
<tr><td align="right">255</td><td><code>i965-va-driver</code></td><td>2.4.1+dfsg1-2build1</td><td>VAAPI driver for Intel G45 &amp; HD Graphics family</td><td align="right">1.8 MiB</td></tr>
<tr><td align="right">256</td><td><code>qml-module-org-kde-kirigami2</code></td><td>5.116.0-1ubuntu4</td><td>set of QtQuick components targeted for mobile use</td><td align="right">1.8 MiB</td></tr>
<tr><td align="right">257</td><td><code>sleuthkit</code></td><td>4.12.1+dfsg-3build1</td><td>tools for forensics analysis on volume and filesystem data</td><td align="right">1.7 MiB</td></tr>
<tr><td align="right">258</td><td><code>libkf6wallet-data</code></td><td>6.24.0-0ubuntu1</td><td>safe desktop-wide storage for passwords - data files</td><td align="right">1.7 MiB</td></tr>
<tr><td align="right">259</td><td><code>vulkan-tools</code></td><td>1.4.341.0+dfsg1-1</td><td>Miscellaneous Vulkan utilities</td><td align="right">1.7 MiB</td></tr>
<tr><td align="right">260</td><td><code>btop</code></td><td>1.4.6-2</td><td>Modern and colorful command line resource monitor that shows usage and stats</td><td align="right">1.7 MiB</td></tr>
<tr><td align="right">261</td><td><code>libdav1d7</code></td><td>1.5.3-1</td><td>fast and small AV1 video stream decoder (shared library)</td><td align="right">1.7 MiB</td></tr>
<tr><td align="right">262</td><td><code>assistant-qt6</code></td><td>6.10.2-1</td><td>Qt 6 Assistant</td><td align="right">1.7 MiB</td></tr>
<tr><td align="right">263</td><td><code>plasma-theme-oxygen</code></td><td>4:6.6.4-0ubuntu1</td><td>Look-and-feel for the Oxygen desktop theme</td><td align="right">1.7 MiB</td></tr>
<tr><td align="right">264</td><td><code>qml6-module-qt5compat-graphicaleffects</code></td><td>6.10.2-1</td><td>Qt 6 Qt5Compat QML module</td><td align="right">1.7 MiB</td></tr>
<tr><td align="right">265</td><td><code>libcfitsio10t64</code></td><td>4.6.3-1</td><td>shared library for I/O with FITS format data files</td><td align="right">1.7 MiB</td></tr>
<tr><td align="right">266</td><td><code>libkf6networkmanagerqt6</code></td><td>6.24.0-0ubuntu1</td><td>Qt wrapper for NetworkManager</td><td align="right">1.7 MiB</td></tr>
<tr><td align="right">267</td><td><code>libkf5wallet-data</code></td><td>5.116.0-1ubuntu1</td><td>Secure and unified container for user passwords.</td><td align="right">1.7 MiB</td></tr>
<tr><td align="right">268</td><td><code>libnetcdf22</code></td><td>1:4.9.3-1build2</td><td>Interface for scientific data access to large binary data</td><td align="right">1.7 MiB</td></tr>
<tr><td align="right">269</td><td><code>openjdk-21-jdk</code></td><td>21.0.11+10-1~26.04.2</td><td>OpenJDK Development Kit (JDK)</td><td align="right">1.6 MiB</td></tr>
<tr><td align="right">270</td><td><code>libkf6kiocore6</code></td><td>6.24.0-0ubuntu1</td><td>resource and network access abstraction (KIO core library)</td><td align="right">1.6 MiB</td></tr>
<tr><td align="right">271</td><td><code>libewf2</code></td><td>20140816-2build1</td><td>library with support for Expert Witness Compression Format</td><td align="right">1.6 MiB</td></tr>
<tr><td align="right">272</td><td><code>libqt6opengl6</code></td><td>6.10.2+dfsg-7</td><td>Qt 6 OpenGL module</td><td align="right">1.6 MiB</td></tr>
<tr><td align="right">273</td><td><code>libkf5kiocore5</code></td><td>5.116.0-2</td><td>resource and network access abstraction (KIO core library)</td><td align="right">1.6 MiB</td></tr>
<tr><td align="right">274</td><td><code>liballegro5.2t64</code></td><td>2:5.2.11.3+dfsg-1</td><td>portable library for cross-platform game and multimedia development</td><td align="right">1.6 MiB</td></tr>
<tr><td align="right">275</td><td><code>libkf6userfeedback-data</code></td><td>6.24.0-0ubuntu1</td><td>user feedback for applications - localization files</td><td align="right">1.6 MiB</td></tr>
<tr><td align="right">276</td><td><code>qt6-l10n-tools</code></td><td>6.10.2-1</td><td>Qt 6 translation tools</td><td align="right">1.5 MiB</td></tr>
<tr><td align="right">277</td><td><code>plasma-keyboard</code></td><td>6.6.4-0ubuntu1</td><td>On-screen virtual keyboard for KDE Plasma</td><td align="right">1.5 MiB</td></tr>
<tr><td align="right">278</td><td><code>plasma-firewall</code></td><td>6.6.4-0ubuntu1</td><td>Plasma configuration module for firewalls</td><td align="right">1.5 MiB</td></tr>
<tr><td align="right">279</td><td><code>kwin-wayland</code></td><td>4:6.6.4-0ubuntu1</td><td>KDE window manager, Wayland version</td><td align="right">1.5 MiB</td></tr>
<tr><td align="right">280</td><td><code>libqt5waylandclient5</code></td><td>5.15.18-1</td><td>QtWayland client library</td><td align="right">1.5 MiB</td></tr>
<tr><td align="right">281</td><td><code>libkf5widgetsaddons5</code></td><td>5.116.0-1ubuntu1</td><td>add-on widgets and classes for applications that use the Qt Widgets module</td><td align="right">1.5 MiB</td></tr>
<tr><td align="right">282</td><td><code>systemsettings</code></td><td>4:6.6.4-0ubuntu1</td><td>System Settings interface</td><td align="right">1.5 MiB</td></tr>
<tr><td align="right">283</td><td><code>libkimageannotator-qt6-0</code></td><td>0.7.2-2</td><td>Image Annotating Library (Qt6 lib)</td><td align="right">1.5 MiB</td></tr>
<tr><td align="right">284</td><td><code>qtwayland5</code></td><td>5.15.18-1</td><td>QtWayland platform plugin</td><td align="right">1.5 MiB</td></tr>
<tr><td align="right">285</td><td><code>libokular6core4</code></td><td>4:25.12.3-0ubuntu1</td><td>libraries for the Okular document viewer</td><td align="right">1.5 MiB</td></tr>
<tr><td align="right">286</td><td><code>python3-pyqt6.qtqml</code></td><td>6.10.2-2build5</td><td>Python bindings for Qt 6 QML module</td><td align="right">1.4 MiB</td></tr>
<tr><td align="right">287</td><td><code>libradare2-dev</code></td><td>6.0.7+ds-1</td><td>devel files from the radare2 suite</td><td align="right">1.4 MiB</td></tr>
<tr><td align="right">288</td><td><code>libkf5kiowidgets5</code></td><td>5.116.0-2</td><td>resource and network access abstraction (KIO widgets library)</td><td align="right">1.4 MiB</td></tr>
<tr><td align="right">289</td><td><code>libmagickwand-7.q16-10</code></td><td>8:7.1.2.18+dfsg1-1</td><td>image manipulation library -- quantum depth Q16</td><td align="right">1.4 MiB</td></tr>
<tr><td align="right">290</td><td><code>plasma-vault</code></td><td>6.6.4-0ubuntu1</td><td>Plasma applet and services for creating encrypted vaults</td><td align="right">1.4 MiB</td></tr>
<tr><td align="right">291</td><td><code>kdenetwork-filesharing</code></td><td>4:25.12.3-0ubuntu1</td><td>network filesharing configuration module</td><td align="right">1.4 MiB</td></tr>
<tr><td align="right">292</td><td><code>libkf6kiowidgets6</code></td><td>6.24.0-0ubuntu1</td><td>resource and network access abstraction (KIO widgets library)</td><td align="right">1.4 MiB</td></tr>
<tr><td align="right">293</td><td><code>breeze-gtk-theme</code></td><td>6.6.4-0ubuntu1</td><td>GTK theme built to match KDE&#x27;s Breeze</td><td align="right">1.4 MiB</td></tr>
<tr><td align="right">294</td><td><code>qml6-module-qtquick-controls</code></td><td>6.10.2+dfsg-3</td><td>Qt 6 Quick Controls QML module</td><td align="right">1.4 MiB</td></tr>
<tr><td align="right">295</td><td><code>python3-unicodedata2</code></td><td>16.0.0+ds-1build2</td><td>Python unicodedata backport/updates</td><td align="right">1.3 MiB</td></tr>
<tr><td align="right">296</td><td><code>libswscale9</code></td><td>7:8.0.1-3ubuntu2</td><td>FFmpeg library for image scaling and various conversions - runtime files</td><td align="right">1.3 MiB</td></tr>
<tr><td align="right">297</td><td><code>libvmdk1</code></td><td>20240510-2build1</td><td>VMWare Virtual Disk format access library</td><td align="right">1.3 MiB</td></tr>
<tr><td align="right">298</td><td><code>libkf5configwidgets-data</code></td><td>5.116.0-2</td><td>Extra widgets for easier configuration support.</td><td align="right">1.3 MiB</td></tr>
<tr><td align="right">299</td><td><code>libqca-qt6-2</code></td><td>2.3.10-2</td><td>libraries for the Qt Cryptographic Architecture [Qt 6]</td><td align="right">1.3 MiB</td></tr>
<tr><td align="right">300</td><td><code>libweather-ion7</code></td><td>4:6.6.4-0ubuntu1</td><td>weather data from multiple sources</td><td align="right">1.3 MiB</td></tr>
<tr><td align="right">301</td><td><code>libkf6textwidgets-data</code></td><td>6.24.0-0ubuntu1</td><td>Advanced text editing widgets</td><td align="right">1.3 MiB</td></tr>
<tr><td align="right">302</td><td><code>libkf5textwidgets-data</code></td><td>5.116.0-1ubuntu1</td><td>Advanced text editing widgets.</td><td align="right">1.3 MiB</td></tr>
<tr><td align="right">303</td><td><code>libkf6kiofilewidgets6</code></td><td>6.24.0-0ubuntu1</td><td>resource and network access abstraction (KIO file widgets library)</td><td align="right">1.3 MiB</td></tr>
<tr><td align="right">304</td><td><code>libavutil60</code></td><td>7:8.0.1-3ubuntu2</td><td>FFmpeg library with functions for simplifying programming - runtime files</td><td align="right">1.3 MiB</td></tr>
<tr><td align="right">305</td><td><code>libkquickimageeditor1</code></td><td>0.6.0-2build1</td><td>Image editing components -- shared library</td><td align="right">1.3 MiB</td></tr>
<tr><td align="right">306</td><td><code>libmysofa1</code></td><td>1.3.3+dfsg-1ubuntu2</td><td>library to read HRTFs stored in the AES69-2015 SOFA format</td><td align="right">1.2 MiB</td></tr>
<tr><td align="right">307</td><td><code>libqt6qmlmodels6</code></td><td>6.10.2+dfsg-3</td><td>Qt 6 QML Models library</td><td align="right">1.2 MiB</td></tr>
<tr><td align="right">308</td><td><code>libvlccore9</code></td><td>3.0.23-1</td><td>base library for VLC and its modules</td><td align="right">1.2 MiB</td></tr>
<tr><td align="right">309</td><td><code>libkf6newstuff-data</code></td><td>6.24.0-0ubuntu1</td><td>Support for downloading application assets from the network</td><td align="right">1.2 MiB</td></tr>
<tr><td align="right">310</td><td><code>python3-zstandard</code></td><td>0.25.0-1build1</td><td>Python bindings for interfacing with Zstandard library</td><td align="right">1.2 MiB</td></tr>
<tr><td align="right">311</td><td><code>libkf6config-data</code></td><td>6.24.0-0ubuntu1</td><td>configuration settings framework for Qt</td><td align="right">1.2 MiB</td></tr>
<tr><td align="right">312</td><td><code>libkf6configwidgets-data</code></td><td>6.24.0-0ubuntu1</td><td>Extra widgets for easier configuration support</td><td align="right">1.2 MiB</td></tr>
<tr><td align="right">313</td><td><code>libqt6bluetooth6</code></td><td>6.10.2-1</td><td>Qt 6 Connectivity Bluetooth library</td><td align="right">1.2 MiB</td></tr>
<tr><td align="right">314</td><td><code>libsfml-dev</code></td><td>3.0.2+dfsg-2</td><td>Simple and Fast Multimedia Library - Development Files</td><td align="right">1.2 MiB</td></tr>
<tr><td align="right">315</td><td><code>python3-sentry-sdk</code></td><td>2.22.0-1</td><td>New Python SDK for Sentry.io (Python 3)</td><td align="right">1.2 MiB</td></tr>
<tr><td align="right">316</td><td><code>libgav1-2</code></td><td>0.20.0-2build1</td><td>AV1 decoder developed by Google -- runtime library</td><td align="right">1.2 MiB</td></tr>
<tr><td align="right">317</td><td><code>libvhdi1</code></td><td>20240509-2build2</td><td>Virtual Hard Disk image format access library</td><td align="right">1.2 MiB</td></tr>
<tr><td align="right">318</td><td><code>libddcutil5</code></td><td>2.2.5-1</td><td>Control monitor settings - shared library</td><td align="right">1.2 MiB</td></tr>
<tr><td align="right">319</td><td><code>libtaskmanager6</code></td><td>4:6.6.4-0ubuntu2</td><td>Plasma Workspace task manager library</td><td align="right">1.2 MiB</td></tr>
<tr><td align="right">320</td><td><code>libkf5waylandclient5</code></td><td>4:5.116.0-0ubuntu7</td><td>Qt library wrapper for Wayland libraries</td><td align="right">1.2 MiB</td></tr>
<tr><td align="right">321</td><td><code>libkf6coreaddons-data</code></td><td>6.24.0-0ubuntu1</td><td>KDE Frameworks 6 addons to QtCore - data files</td><td align="right">1.1 MiB</td></tr>
<tr><td align="right">322</td><td><code>libkf6texttemplate6</code></td><td>6.24.0-0ubuntu1</td><td>library used for text processing</td><td align="right">1.1 MiB</td></tr>
<tr><td align="right">323</td><td><code>libwvstreams4.6t64-extras</code></td><td>4.6.1-19</td><td>C++ network libraries for rapid application development</td><td align="right">1.1 MiB</td></tr>
<tr><td align="right">324</td><td><code>libzxing3</code></td><td>2.3.0-5</td><td>C++ port of ZXing library (library files)</td><td align="right">1.1 MiB</td></tr>
<tr><td align="right">325</td><td><code>libkf6sonnet-data</code></td><td>6.24.0-0ubuntu1</td><td>spell checking library for Qt, data files</td><td align="right">1.1 MiB</td></tr>
<tr><td align="right">326</td><td><code>libtsk19t64</code></td><td>4.12.1+dfsg-3build1</td><td>library for forensics analysis on volume and filesystem data</td><td align="right">1.1 MiB</td></tr>
<tr><td align="right">327</td><td><code>libkf5coreaddons-data</code></td><td>5.116.0-1ubuntu1</td><td>KDE Frameworks 5 addons to QtCore - data files</td><td align="right">1.1 MiB</td></tr>
<tr><td align="right">328</td><td><code>tesseract-ocr</code></td><td>5.5.0-1build1</td><td>Tesseract command line OCR tool</td><td align="right">1.1 MiB</td></tr>
<tr><td align="right">329</td><td><code>libkf6solid-data</code></td><td>6.24.0-0ubuntu1</td><td>Qt library to query and control hardware</td><td align="right">1.1 MiB</td></tr>
<tr><td align="right">330</td><td><code>libkf6i18nlocaledata6</code></td><td>6.24.0-0ubuntu1</td><td>Advanced internationalization framework (locale data)</td><td align="right">1.1 MiB</td></tr>
<tr><td align="right">331</td><td><code>fonts-katex</code></td><td>0.16.10+~cs6.1.0-5ubuntu1</td><td>Fast math typesetting for the web (fonts)</td><td align="right">1.1 MiB</td></tr>
<tr><td align="right">332</td><td><code>qml6-module-org-kde-newstuff</code></td><td>6.24.0-0ubuntu1</td><td>Support for downloading application assets from the network</td><td align="right">1.1 MiB</td></tr>
<tr><td align="right">333</td><td><code>libkf5kiofilewidgets5</code></td><td>5.116.0-2</td><td>resource and network access abstraction (KIO file widgets library)</td><td align="right">1.1 MiB</td></tr>
<tr><td align="right">334</td><td><code>libgvc7</code></td><td>14.1.2-1ubuntu1</td><td>rich set of graph drawing tools - gvc library</td><td align="right">1.1 MiB</td></tr>
<tr><td align="right">335</td><td><code>linguist-qt6</code></td><td>6.10.2-1</td><td>Qt 6 Linguist</td><td align="right">1.1 MiB</td></tr>
<tr><td align="right">336</td><td><code>libqt6webenginequick6</code></td><td>6.10.2+dfsg-1</td><td>Qt 6 WebEngine Quick library</td><td align="right">1.1 MiB</td></tr>
<tr><td align="right">337</td><td><code>libjs-katex</code></td><td>0.16.10+~cs6.1.0-5ubuntu1</td><td>Fast math typesetting for the web (for browsers)</td><td align="right">1.1 MiB</td></tr>
<tr><td align="right">338</td><td><code>libkf5sonnet5-data</code></td><td>5.116.0-1ubuntu2</td><td>spell checking library for Qt, data files</td><td align="right">1.0 MiB</td></tr>
<tr><td align="right">339</td><td><code>libkwaylandclient6</code></td><td>4:6.6.4-0ubuntu1</td><td>Qt library wrapper for Wayland libraries</td><td align="right">1.0 MiB</td></tr>
<tr><td align="right">340</td><td><code>gnutls-bin</code></td><td>3.8.12-2ubuntu1.1</td><td>GNU TLS library - commandline utilities</td><td align="right">1.0 MiB</td></tr>
<tr><td align="right">341</td><td><code>libkf6bluezqt6</code></td><td>6.24.0-0ubuntu1</td><td>Qt wrapper for bluez</td><td align="right">1.0 MiB</td></tr>
<tr><td align="right">342</td><td><code>skanpage</code></td><td>25.12.3-0ubuntu1</td><td>Multi-page scanning and saving of documents and images</td><td align="right">1.0 MiB</td></tr>
<tr><td align="right">343</td><td><code>libkf6coreaddons6</code></td><td>6.24.0-0ubuntu1</td><td>KDE Frameworks 6 addons to QtCore</td><td align="right">1.0 MiB</td></tr>
<tr><td align="right">344</td><td><code>libkf5solid5-data</code></td><td>5.116.0-1ubuntu2</td><td>Qt library to query and control hardware</td><td align="right">1.0 MiB</td></tr>
<tr><td align="right">345</td><td><code>qml6-module-org-kde-kirigami</code></td><td>6.24.0-0ubuntu1</td><td>set of QtQuick components targeted for mobile use (QML module)</td><td align="right">1.0 MiB</td></tr>
<tr><td align="right">346</td><td><code>kdialog</code></td><td>4:25.12.3-0ubuntu1</td><td>Dialog display utility</td><td align="right">1.0 MiB</td></tr>
<tr><td align="right">347</td><td><code>libnotificationmanager1</code></td><td>4:6.6.4-0ubuntu2</td><td>Plasma Workspace notification manager library</td><td align="right">1.0 MiB</td></tr>
<tr><td align="right">348</td><td><code>gnustep-base-runtime</code></td><td>1.31.1-4ubuntu2</td><td>GNUstep Base library - daemons and tools</td><td align="right">1023 KiB</td></tr>
<tr><td align="right">349</td><td><code>libkcddb6-5</code></td><td>4:25.12.3-0ubuntu1</td><td>CDDB library for KDE Platform (runtime)</td><td align="right">1017 KiB</td></tr>
<tr><td align="right">350</td><td><code>libosdcpu3.6.0</code></td><td>3.6.0-3build1</td><td>high performance subdivision surface (subdiv) library - CPU runtime</td><td align="right">1007 KiB</td></tr>
<tr><td align="right">351</td><td><code>kcharselect</code></td><td>4:25.12.3-0ubuntu1</td><td>special character utility</td><td align="right">1006 KiB</td></tr>
<tr><td align="right">352</td><td><code>libkf6solid6</code></td><td>6.24.0-0ubuntu1</td><td>Qt library to query and control hardware</td><td align="right">1006 KiB</td></tr>
<tr><td align="right">353</td><td><code>libscim8v5</code></td><td>1.4.18+git20211204-0.5</td><td>library for SCIM platform</td><td align="right">1001 KiB</td></tr>
<tr><td align="right">354</td><td><code>libglu1-mesa-dev</code></td><td>9.0.2-1.1build2</td><td>Mesa OpenGL utility library -- development files</td><td align="right">993 KiB</td></tr>
<tr><td align="right">355</td><td><code>kamera</code></td><td>4:25.12.3-0ubuntu1</td><td>digital camera support for KDE applications</td><td align="right">990 KiB</td></tr>
<tr><td align="right">356</td><td><code>kactivitymanagerd</code></td><td>6.6.4-0ubuntu1</td><td>System service to manage user&#x27;s activities</td><td align="right">989 KiB</td></tr>
<tr><td align="right">357</td><td><code>libqt6dbus6</code></td><td>6.10.2+dfsg-7</td><td>Qt 6 D-Bus module</td><td align="right">988 KiB</td></tr>
<tr><td align="right">358</td><td><code>okular-extra-backends</code></td><td>4:25.12.3-0ubuntu1</td><td>additional document format support for Okular</td><td align="right">983 KiB</td></tr>
<tr><td align="right">359</td><td><code>libsnapd-qt-2-1</code></td><td>1.72-0ubuntu3</td><td>Qt6 snapd library</td><td align="right">980 KiB</td></tr>
<tr><td align="right">360</td><td><code>libkf6purpose-data</code></td><td>6.24.0-0ubuntu1</td><td>abstraction to provide and leverage actions of a specific kind, translations</td><td align="right">972 KiB</td></tr>
<tr><td align="right">361</td><td><code>libtcod-dev</code></td><td>1.24.0+dfsg-2</td><td>development files for the libtcod roguelike library</td><td align="right">968 KiB</td></tr>
<tr><td align="right">362</td><td><code>vlc-data</code></td><td>3.0.23-1</td><td>common data for VLC</td><td align="right">960 KiB</td></tr>
<tr><td align="right">363</td><td><code>libkf6xmlgui6</code></td><td>6.24.0-0ubuntu1</td><td>User configurable main windows</td><td align="right">947 KiB</td></tr>
<tr><td align="right">364</td><td><code>libcalamares3.3</code></td><td>3.3.14-0ubuntu25</td><td>Shared object files for Calamares</td><td align="right">945 KiB</td></tr>
<tr><td align="right">365</td><td><code>kirigami-addons-data</code></td><td>1.11.0-2ubuntu2</td><td>data files for kirigami-addons</td><td align="right">943 KiB</td></tr>
<tr><td align="right">366</td><td><code>libglew2.2</code></td><td>2.2.0-4build2</td><td>OpenGL Extension Wrangler - runtime environment</td><td align="right">933 KiB</td></tr>
<tr><td align="right">367</td><td><code>libkf6newstuffcore6</code></td><td>6.24.0-0ubuntu1</td><td>Support for downloading application assets from the network</td><td align="right">928 KiB</td></tr>
<tr><td align="right">368</td><td><code>libtomcrypt1</code></td><td>1.18.2+dfsg-7build2</td><td>public domain open source cryptographic toolkit</td><td align="right">926 KiB</td></tr>
<tr><td align="right">369</td><td><code>kwallet6</code></td><td>6.24.0-0ubuntu1</td><td>safe desktop-wide storage for passwords - kwalletd daemon</td><td align="right">925 KiB</td></tr>
<tr><td align="right">370</td><td><code>qt6-tools-dev-tools</code></td><td>6.10.2-1</td><td>Qt 6 development tools</td><td align="right">913 KiB</td></tr>
<tr><td align="right">371</td><td><code>libkf5xmlgui5</code></td><td>5.116.0-1ubuntu4</td><td>User configurable main windows.</td><td align="right">902 KiB</td></tr>
<tr><td align="right">372</td><td><code>libqca-qt6-plugins</code></td><td>2.3.10-2</td><td>QCA plugins for libqca2 [Qt 6]</td><td align="right">898 KiB</td></tr>
<tr><td align="right">373</td><td><code>kde-cli-tools</code></td><td>4:6.6.4-0ubuntu1</td><td>tools to use KDE services from the command line</td><td align="right">891 KiB</td></tr>
<tr><td align="right">374</td><td><code>python3-pyqt5.qtopengl</code></td><td>5.15.11+dfsg-3build3</td><td>Python 3 bindings for Qt5&#x27;s OpenGL module</td><td align="right">888 KiB</td></tr>
<tr><td align="right">375</td><td><code>python3-brotli</code></td><td>1.2.0-3build1</td><td>lossless compression algorithm and format (Python 3 version)</td><td align="right">883 KiB</td></tr>
<tr><td align="right">376</td><td><code>libigdgmm12</code></td><td>22.9.0+ds1-1</td><td>Intel Graphics Memory Management Library -- shared library</td><td align="right">879 KiB</td></tr>
<tr><td align="right">377</td><td><code>mesa-utils-bin</code></td><td>9.0.0-2build1</td><td>Miscellaneous Mesa utilities -- native applications</td><td align="right">878 KiB</td></tr>
<tr><td align="right">378</td><td><code>qml6-module-org-kde-kirigamiaddons-components</code></td><td>1.11.0-2ubuntu2</td><td>components module for QML</td><td align="right">878 KiB</td></tr>
<tr><td align="right">379</td><td><code>libcwidget4</code></td><td>0.5.18-6build2</td><td>high-level terminal interface library for C++ (runtime files)</td><td align="right">877 KiB</td></tr>
<tr><td align="right">380</td><td><code>liballegro5-dev</code></td><td>2:5.2.11.3+dfsg-1</td><td>development files for the Allegro 5 library</td><td align="right">874 KiB</td></tr>
<tr><td align="right">381</td><td><code>libbfio1</code></td><td>20170123-6build1</td><td>Library to provide basic input/output abstraction</td><td align="right">873 KiB</td></tr>
<tr><td align="right">382</td><td><code>qml6-module-qtquick-particles</code></td><td>6.10.2+dfsg-3</td><td>Qt 6 Quick Particle QML module</td><td align="right">872 KiB</td></tr>
<tr><td align="right">383</td><td><code>libtcod1t64</code></td><td>1.24.0+dfsg-2</td><td>graphics and utility library for roguelike developers</td><td align="right">867 KiB</td></tr>
<tr><td align="right">384</td><td><code>libsrt1.5-gnutls</code></td><td>1.5.4-3</td><td>Secure Reliable Transport UDP streaming library (GnuTLS flavour)</td><td align="right">865 KiB</td></tr>
<tr><td align="right">385</td><td><code>vlc-plugin-video-output</code></td><td>3.0.23-1</td><td>multimedia player and streamer (video output plugins)</td><td align="right">852 KiB</td></tr>
<tr><td align="right">386</td><td><code>imagemagick-7.q16</code></td><td>8:7.1.2.18+dfsg1-1</td><td>image manipulation programs -- quantum depth Q16</td><td align="right">847 KiB</td></tr>
<tr><td align="right">387</td><td><code>libkf6attica6</code></td><td>6.24.0-0ubuntu1</td><td>Qt library that implements the Open Collaboration Services API</td><td align="right">842 KiB</td></tr>
<tr><td align="right">388</td><td><code>libkf6contacts6</code></td><td>6.24.0-0ubuntu1</td><td>address book API for KDE frameworks</td><td align="right">836 KiB</td></tr>
<tr><td align="right">389</td><td><code>python3-capstone</code></td><td>5.0.7-2</td><td>lightweight multi-architecture disassembly framework - Python bindings</td><td align="right">834 KiB</td></tr>
<tr><td align="right">390</td><td><code>libosdgpu3.6.0</code></td><td>3.6.0-3build1</td><td>high performance subdivision surface (subdiv) library - GPU runtime</td><td align="right">831 KiB</td></tr>
<tr><td align="right">391</td><td><code>plymouth-theme-kubuntu-logo</code></td><td>1:26.04.13</td><td>graphical boot animation and logger - kubuntu-logo theme</td><td align="right">831 KiB</td></tr>
<tr><td align="right">392</td><td><code>qml6-module-qtquick3d</code></td><td>6.10.2-1</td><td>Qt 6 Quick 3D QML module</td><td align="right">822 KiB</td></tr>
<tr><td align="right">393</td><td><code>plasma-browser-integration</code></td><td>6.6.4-0ubuntu1</td><td>Chromium, Google Chrome, Firefox integration for Plasma</td><td align="right">820 KiB</td></tr>
<tr><td align="right">394</td><td><code>libkf5coreaddons5</code></td><td>5.116.0-1ubuntu1</td><td>KDE Frameworks 5 addons to QtCore</td><td align="right">819 KiB</td></tr>
<tr><td align="right">395</td><td><code>libkf5config-data</code></td><td>5.116.0-2</td><td>configuration settings framework for Qt</td><td align="right">816 KiB</td></tr>
<tr><td align="right">396</td><td><code>kwin-style-breeze</code></td><td>4:6.6.4-0ubuntu1</td><td>KWin Breeze Style</td><td align="right">813 KiB</td></tr>
<tr><td align="right">397</td><td><code>libkf6modemmanagerqt6</code></td><td>6.23.0-0ubuntu1</td><td>Qt wrapper library for ModemManager</td><td align="right">812 KiB</td></tr>
<tr><td align="right">398</td><td><code>libkf6purpose-bin</code></td><td>6.24.0-0ubuntu1</td><td>abstraction to provide and leverage actions of a specific kind, runtime</td><td align="right">812 KiB</td></tr>
<tr><td align="right">399</td><td><code>kde-config-screenlocker</code></td><td>6.6.4-0ubuntu2</td><td>KCM Module for kscreenlocker</td><td align="right">807 KiB</td></tr>
<tr><td align="right">400</td><td><code>plasma-disks</code></td><td>6.6.4-0ubuntu1</td><td>Monitor S.M.A.R.T. capable devices for imminent failure in Plasma</td><td align="right">804 KiB</td></tr>
<tr><td align="right">401</td><td><code>libpowerdevilcore2</code></td><td>4:6.6.4-0ubuntu1</td><td>shared library for the KDE power management features in Plasma</td><td align="right">803 KiB</td></tr>
<tr><td align="right">402</td><td><code>libsox-ng3</code></td><td>14.7.0.9+ds1-1</td><td>SoX library of audio effects and processing</td><td align="right">802 KiB</td></tr>
<tr><td align="right">403</td><td><code>calamares-data</code></td><td>3.3.14-0ubuntu25</td><td>data for Calamares</td><td align="right">798 KiB</td></tr>
<tr><td align="right">404</td><td><code>libqt6printsupport6</code></td><td>6.10.2+dfsg-7</td><td>Qt 6 print support module</td><td align="right">798 KiB</td></tr>
<tr><td align="right">405</td><td><code>libkf6syntaxhighlighting-data</code></td><td>6.24.0-0ubuntu1</td><td>Syntax highlighting Engine - translations</td><td align="right">793 KiB</td></tr>
<tr><td align="right">406</td><td><code>libinstpatch-1.0-2</code></td><td>1.1.7-1.1</td><td>MIDI instrument editing library</td><td align="right">790 KiB</td></tr>
<tr><td align="right">407</td><td><code>libhdf4-0</code></td><td>4.3.1-2</td><td>Hierarchical Data Format library</td><td align="right">789 KiB</td></tr>
<tr><td align="right">408</td><td><code>kde-config-sddm</code></td><td>4:6.6.4-0ubuntu1</td><td>KCM module for SDDM</td><td align="right">786 KiB</td></tr>
<tr><td align="right">409</td><td><code>libphonon4qt6-4t64</code></td><td>4:4.12.0-7</td><td>multimedia framework from KDE using Qt 6 - core library</td><td align="right">782 KiB</td></tr>
<tr><td align="right">410</td><td><code>libarmadillo15</code></td><td>1:15.2.1+dfsg-2</td><td>streamlined C++ linear algebra library</td><td align="right">778 KiB</td></tr>
<tr><td align="right">411</td><td><code>libqt5dbus5t64</code></td><td>5.15.18+dfsg-1ubuntu1</td><td>Qt 5 D-Bus module</td><td align="right">765 KiB</td></tr>
<tr><td align="right">412</td><td><code>libqt5printsupport5t64</code></td><td>5.15.18+dfsg-1ubuntu1</td><td>Qt 5 print support module</td><td align="right">761 KiB</td></tr>
<tr><td align="right">413</td><td><code>sddm-theme-breeze</code></td><td>4:6.6.4-0ubuntu1</td><td>Breeze SDDM theme</td><td align="right">760 KiB</td></tr>
<tr><td align="right">414</td><td><code>libkf5solid5</code></td><td>5.116.0-1ubuntu2</td><td>Qt library to query and control hardware</td><td align="right">749 KiB</td></tr>
<tr><td align="right">415</td><td><code>kubuntu-notification-helper</code></td><td>26.04ubuntu6</td><td>Kubuntu system notification helper</td><td align="right">747 KiB</td></tr>
<tr><td align="right">416</td><td><code>plasma5-integration</code></td><td>6.6.4-0ubuntu1</td><td>Qt Platform Theme integration plugins for KDE Plasma 5</td><td align="right">744 KiB</td></tr>
<tr><td align="right">417</td><td><code>plasma-thunderbolt</code></td><td>6.6.4-0ubuntu1</td><td>Plasma addons for managing Thunderbolt devices</td><td align="right">738 KiB</td></tr>
<tr><td align="right">418</td><td><code>libkdegames6-i18n</code></td><td>4:25.12.3-0ubuntu1</td><td>Localiztion files for the KDE games library</td><td align="right">734 KiB</td></tr>
<tr><td align="right">419</td><td><code>libqt6svg6</code></td><td>6.10.2-2</td><td>Qt 6 SVG library</td><td align="right">733 KiB</td></tr>
<tr><td align="right">420</td><td><code>libglut-dev</code></td><td>3.4.0-6</td><td>OpenGL Utility Toolkit development files</td><td align="right">732 KiB</td></tr>
<tr><td align="right">421</td><td><code>phonon4settings</code></td><td>4:4.12.0-7</td><td>multimedia framework from KDE using - settings application</td><td align="right">727 KiB</td></tr>
<tr><td align="right">422</td><td><code>libplasmaquick7</code></td><td>6.6.4-0ubuntu1</td><td>Plasma Runtime components</td><td align="right">724 KiB</td></tr>
<tr><td align="right">423</td><td><code>libkf6codecs-data</code></td><td>6.24.0-0ubuntu1</td><td>collection of methods to manipulate strings</td><td align="right">722 KiB</td></tr>
<tr><td align="right">424</td><td><code>khelpcenter</code></td><td>4:25.12.3-0ubuntu1</td><td>search and read system documentation</td><td align="right">721 KiB</td></tr>
<tr><td align="right">425</td><td><code>libkf6filemetadata-data</code></td><td>6.24.0-0ubuntu1</td><td>library for extracting file metadata</td><td align="right">720 KiB</td></tr>
<tr><td align="right">426</td><td><code>libopencv-videoio410</code></td><td>4.10.0+dfsg-7ubuntu5</td><td>computer vision Video I/O library</td><td align="right">715 KiB</td></tr>
<tr><td align="right">427</td><td><code>libzvbi0t64</code></td><td>0.2.44-1ubuntu2</td><td>Vertical Blanking Interval decoder (VBI) - runtime files</td><td align="right">714 KiB</td></tr>
<tr><td align="right">428</td><td><code>libkmldom1t64</code></td><td>1.3.0-13</td><td>Library to manipulate KML 2.2 OGC standard files - libkmldom</td><td align="right">712 KiB</td></tr>
<tr><td align="right">429</td><td><code>libkf6i18n6</code></td><td>6.24.0-0ubuntu1</td><td>Advanced internationalization framework</td><td align="right">711 KiB</td></tr>
<tr><td align="right">430</td><td><code>qml6-module-qtquick</code></td><td>6.10.2+dfsg-3</td><td>Qt 6 Quick QML module</td><td align="right">711 KiB</td></tr>
<tr><td align="right">431</td><td><code>bup</code></td><td>0.33.9-1.2build1</td><td>highly efficient file backup system based on git</td><td align="right">710 KiB</td></tr>
<tr><td align="right">432</td><td><code>kwin-decoration-oxygen</code></td><td>4:6.6.4-0ubuntu1</td><td>KWin decoration for the Oxygen desktop theme</td><td align="right">710 KiB</td></tr>
<tr><td align="right">433</td><td><code>kwin-style-aurorae</code></td><td>6.6.4-0ubuntu1</td><td>KWin Aurorae decoration style engine</td><td align="right">710 KiB</td></tr>
<tr><td align="right">434</td><td><code>libzmq5</code></td><td>4.3.5-1build3</td><td>lightweight messaging kernel (shared library)</td><td align="right">702 KiB</td></tr>
<tr><td align="right">435</td><td><code>kgamma</code></td><td>6.6.4-0ubuntu1</td><td>monitor calibration panel for KDE</td><td align="right">699 KiB</td></tr>
<tr><td align="right">436</td><td><code>python3-contourpy</code></td><td>1.3.3-1build1</td><td>Python library for calculating contours of 2D quadrilateral grids</td><td align="right">696 KiB</td></tr>
<tr><td align="right">437</td><td><code>libkf6windowsystem6</code></td><td>6.24.0-0ubuntu1</td><td>Convenience access to certain properties and features of the window manager</td><td align="right">692 KiB</td></tr>
<tr><td align="right">438</td><td><code>plasma-runners-addons</code></td><td>4:6.6.4-0ubuntu1</td><td>additional runners for Plasma 6 and Krunner</td><td align="right">686 KiB</td></tr>
<tr><td align="right">439</td><td><code>libkscreen-bin</code></td><td>4:6.6.4-0ubuntu1</td><td>library for screen management - helpers</td><td align="right">683 KiB</td></tr>
<tr><td align="right">440</td><td><code>openjdk-21-jre</code></td><td>21.0.11+10-1~26.04.2</td><td>OpenJDK Java runtime, using Hotspot JIT</td><td align="right">682 KiB</td></tr>
<tr><td align="right">441</td><td><code>catdoc</code></td><td>1:0.95-6build1</td><td>text extractor for MS-Office files</td><td align="right">681 KiB</td></tr>
<tr><td align="right">442</td><td><code>kde-style-breeze</code></td><td>4:6.6.4-0ubuntu1</td><td>Widget style for Qt and KDE Software</td><td align="right">677 KiB</td></tr>
<tr><td align="right">443</td><td><code>libkf5codecs-data</code></td><td>5.116.0-2</td><td>collection of methods to manipulate strings</td><td align="right">676 KiB</td></tr>
<tr><td align="right">444</td><td><code>libkf6filemetadata-bin</code></td><td>6.24.0-0ubuntu1</td><td>library for extracting file metadata</td><td align="right">676 KiB</td></tr>
<tr><td align="right">445</td><td><code>libkf6syndication6</code></td><td>6.24.0-0ubuntu1</td><td>parser library for RSS and Atom feeds</td><td align="right">676 KiB</td></tr>
<tr><td align="right">446</td><td><code>libmatroska7</code></td><td>1.7.1-2</td><td>extensible open standard audio/video container format (shared library)</td><td align="right">675 KiB</td></tr>
<tr><td align="right">447</td><td><code>python3-websockets</code></td><td>15.0.1-1build2</td><td>implementation of the WebSocket Protocol (RFC 6455)</td><td align="right">673 KiB</td></tr>
<tr><td align="right">448</td><td><code>qml-module-qtquick-templates2</code></td><td>5.15.18+dfsg-1</td><td>Qt 5 Qt Quick Templates 2 QML module</td><td align="right">671 KiB</td></tr>
<tr><td align="right">449</td><td><code>libxvidcore4</code></td><td>2:1.3.7-3</td><td>Open source MPEG-4 video codec (library)</td><td align="right">669 KiB</td></tr>
<tr><td align="right">450</td><td><code>python3-mutagen</code></td><td>1.47.0-1build1</td><td>audio metadata editing library (Python 3)</td><td align="right">667 KiB</td></tr>
<tr><td align="right">451</td><td><code>libpoppler-qt6-3t64</code></td><td>26.01.0-2build2</td><td>PDF rendering library (Qt 6 based shared library)</td><td align="right">663 KiB</td></tr>
<tr><td align="right">452</td><td><code>dasm</code></td><td>2.20.15~20201109+really2.20.14.1-3build1</td><td>Macro assembler with support for several 8-bit microprocessors</td><td align="right">661 KiB</td></tr>
<tr><td align="right">453</td><td><code>python3-binwalk</code></td><td>2.4.3+dfsg1-2build1</td><td>Python3 library for analyzing binary blobs and executable code</td><td align="right">659 KiB</td></tr>
<tr><td align="right">454</td><td><code>libfluidsynth3</code></td><td>2.4.8+dfsg-1</td><td>Real-time MIDI software synthesizer (runtime library)</td><td align="right">650 KiB</td></tr>
<tr><td align="right">455</td><td><code>libqt5qmlmodels5</code></td><td>5.15.18+dfsg-2</td><td>Qt 5 QML Models library</td><td align="right">645 KiB</td></tr>
<tr><td align="right">456</td><td><code>libklipper6</code></td><td>4:6.6.4-0ubuntu2</td><td>Plasma Workspaces Klipper library</td><td align="right">644 KiB</td></tr>
<tr><td align="right">457</td><td><code>libqt6virtualkeyboard6</code></td><td>6.10.2+dfsg-1</td><td>Qt 6 Virtual Keyboard library</td><td align="right">639 KiB</td></tr>
<tr><td align="right">458</td><td><code>designer-qt6</code></td><td>6.10.2-1</td><td>Qt 6 Designer</td><td align="right">638 KiB</td></tr>
<tr><td align="right">459</td><td><code>qml6-module-org-kde-kirigamiaddons-labs-components</code></td><td>1.11.0-2ubuntu2</td><td>labs/components module for QML</td><td align="right">631 KiB</td></tr>
<tr><td align="right">460</td><td><code>libmbedcrypto16</code></td><td>3.6.5-0.1ubuntu2</td><td>lightweight crypto and SSL/TLS library - crypto library</td><td align="right">629 KiB</td></tr>
<tr><td align="right">461</td><td><code>libqt6help6</code></td><td>6.10.2-1</td><td>Qt 6 Help library</td><td align="right">628 KiB</td></tr>
<tr><td align="right">462</td><td><code>libqt6test6</code></td><td>6.10.2+dfsg-7</td><td>Qt 6 test module</td><td align="right">625 KiB</td></tr>
<tr><td align="right">463</td><td><code>libfaad2</code></td><td>2.11.2-1build1</td><td>freeware Advanced Audio Decoder - runtime files</td><td align="right">616 KiB</td></tr>
<tr><td align="right">464</td><td><code>libqt6positioning6</code></td><td>6.10.2-1</td><td>Qt 6 Positioning library</td><td align="right">614 KiB</td></tr>
<tr><td align="right">465</td><td><code>libkf6kiogui6</code></td><td>6.24.0-0ubuntu1</td><td>resource and network access abstraction (KIO gui library)</td><td align="right">610 KiB</td></tr>
<tr><td align="right">466</td><td><code>kde-style-breeze-qt5</code></td><td>4:6.6.4-0ubuntu1</td><td>Widget style for Qt and KDE Software running Qt5</td><td align="right">607 KiB</td></tr>
<tr><td align="right">467</td><td><code>libkf5service5</code></td><td>5.116.0-1ubuntu1</td><td>Advanced plugin and service introspection</td><td align="right">606 KiB</td></tr>
<tr><td align="right">468</td><td><code>libkf6archive-data</code></td><td>6.24.0-0ubuntu1</td><td>data files for karchive</td><td align="right">602 KiB</td></tr>
<tr><td align="right">469</td><td><code>libkf6configcore6</code></td><td>6.24.0-0ubuntu1</td><td>configuration settings framework for Qt</td><td align="right">599 KiB</td></tr>
<tr><td align="right">470</td><td><code>libcalamaresui3.3</code></td><td>3.3.14-0ubuntu25</td><td>UI shared object files for Calamares</td><td align="right">598 KiB</td></tr>
<tr><td align="right">471</td><td><code>qml6-module-org-kde-kdeconnect</code></td><td>25.12.3-0ubuntu1</td><td>QML module wrapping KDE Connect functionality</td><td align="right">593 KiB</td></tr>
<tr><td align="right">472</td><td><code>libkf5service-data</code></td><td>5.116.0-1ubuntu1</td><td>Advanced plugin and service introspection</td><td align="right">591 KiB</td></tr>
<tr><td align="right">473</td><td><code>libwvstreams4.6t64-base</code></td><td>4.6.1-19</td><td>C++ network libraries for rapid application development</td><td align="right">590 KiB</td></tr>
<tr><td align="right">474</td><td><code>libqt6uitools6</code></td><td>6.10.2-1</td><td>Qt 6 UI tools library</td><td align="right">589 KiB</td></tr>
<tr><td align="right">475</td><td><code>qml-module-qtgraphicaleffects</code></td><td>5.15.18-1</td><td>Qt 5 Graphical Effects module</td><td align="right">588 KiB</td></tr>
<tr><td align="right">476</td><td><code>libzimg2</code></td><td>3.0.6+ds1-1</td><td>scaling, colorspace, depth conversion library (shared library)</td><td align="right">587 KiB</td></tr>
<tr><td align="right">477</td><td><code>libkf6kcmutils-data</code></td><td>6.24.0-0ubuntu1</td><td>Extra APIs to write KConfig modules</td><td align="right">586 KiB</td></tr>
<tr><td align="right">478</td><td><code>milou</code></td><td>4:6.6.4-0ubuntu1</td><td>Dedicated search plasmoid</td><td align="right">586 KiB</td></tr>
<tr><td align="right">479</td><td><code>libkf6baloowidgets6</code></td><td>4:25.12.3-0ubuntu1</td><td>Wigets for use with Baloo</td><td align="right">584 KiB</td></tr>
<tr><td align="right">480</td><td><code>libkf6holidays6</code></td><td>6.24.0-0ubuntu1</td><td>holidays calculation library</td><td align="right">582 KiB</td></tr>
<tr><td align="right">481</td><td><code>libsonivox3</code></td><td>3.6.16-1</td><td>Sonivox Embedded Audio Synthesis Library</td><td align="right">579 KiB</td></tr>
<tr><td align="right">482</td><td><code>cbios</code></td><td>0.29a-1.1build1</td><td>open source MSX BIOS roms</td><td align="right">576 KiB</td></tr>
<tr><td align="right">483</td><td><code>libopenconnect5</code></td><td>9.12-3.3</td><td>open client for various network vendors SSL VPNs - shared library</td><td align="right">570 KiB</td></tr>
<tr><td align="right">484</td><td><code>libmng2</code></td><td>2.0.3+dfsg-5</td><td>Multiple-image Network Graphics library</td><td align="right">568 KiB</td></tr>
<tr><td align="right">485</td><td><code>libkf5bookmarks-data</code></td><td>5.116.0-1ubuntu1</td><td>Qt library with support for bookmarks and the XBEL format.</td><td align="right">567 KiB</td></tr>
<tr><td align="right">486</td><td><code>libkf5package-data</code></td><td>5.116.0-1ubuntu1</td><td>non-binary asset management framework</td><td align="right">564 KiB</td></tr>
<tr><td align="right">487</td><td><code>libqt5opengl5t64</code></td><td>5.15.18+dfsg-1ubuntu1</td><td>Qt 5 OpenGL module</td><td align="right">563 KiB</td></tr>
<tr><td align="right">488</td><td><code>proj-bin</code></td><td>9.7.1-1</td><td>Cartographic projection library (tools)</td><td align="right">559 KiB</td></tr>
<tr><td align="right">489</td><td><code>libkf6holidays-data</code></td><td>6.24.0-0ubuntu1</td><td>holidays calculation library</td><td align="right">558 KiB</td></tr>
<tr><td align="right">490</td><td><code>plocate</code></td><td>1.1.23-1ubuntu3</td><td>much faster locate</td><td align="right">553 KiB</td></tr>
<tr><td align="right">491</td><td><code>libqt5help5</code></td><td>5.15.18-1</td><td>Qt 5 help module</td><td align="right">552 KiB</td></tr>
<tr><td align="right">492</td><td><code>qml6-module-org-kde-kcmutils</code></td><td>6.24.0-0ubuntu1</td><td>Extra APIs to write KConfig modules</td><td align="right">544 KiB</td></tr>
<tr><td align="right">493</td><td><code>qml6-module-org-kde-purpose</code></td><td>6.24.0-0ubuntu1</td><td>abstraction to provide and leverage actions of a specific kind, qml bindings</td><td align="right">544 KiB</td></tr>
<tr><td align="right">494</td><td><code>libafflib0t64</code></td><td>3.7.22-1build1</td><td>Advanced Forensics Format Library</td><td align="right">543 KiB</td></tr>
<tr><td align="right">495</td><td><code>libkscreenlocker6</code></td><td>6.6.4-0ubuntu2</td><td>Secure lock screen architecture</td><td align="right">543 KiB</td></tr>
<tr><td align="right">496</td><td><code>libqhull-r8.0</code></td><td>2020.2-8</td><td>calculate convex hulls and related structures (reentrant shared library)</td><td align="right">540 KiB</td></tr>
<tr><td align="right">497</td><td><code>libkirigamilayouts6</code></td><td>6.24.0-0ubuntu1</td><td>set of QtQuick components targeted for mobile use</td><td align="right">538 KiB</td></tr>
<tr><td align="right">498</td><td><code>libkf5windowsystem5</code></td><td>5.116.0-1ubuntu1</td><td>Convenience access to certain properties and features of the window manager</td><td align="right">536 KiB</td></tr>
<tr><td align="right">499</td><td><code>libprocesscore10</code></td><td>4:6.6.4-0ubuntu1</td><td>library for system monitoring - processcore shared library</td><td align="right">536 KiB</td></tr>
<tr><td align="right">500</td><td><code>libkf6contacts-data</code></td><td>6.24.0-0ubuntu1</td><td>data files for kcontacts</td><td align="right">533 KiB</td></tr>
<tr><td align="right">501</td><td><code>libappstreamqt3</code></td><td>1.1.2-1</td><td>Qt6 library to access AppStream services</td><td align="right">532 KiB</td></tr>
<tr><td align="right">502</td><td><code>libkf6package-data</code></td><td>6.24.0-0ubuntu1</td><td>non-binary asset management framework</td><td align="right">528 KiB</td></tr>
<tr><td align="right">503</td><td><code>libstb0t64</code></td><td>0.0~git20250907.fede005+ds-1</td><td>single-file image and audio processing libraries for C/C++</td><td align="right">528 KiB</td></tr>
<tr><td align="right">504</td><td><code>qt6-wayland</code></td><td>6.10.2-4</td><td>Qt 6 Wayland platform plugin</td><td align="right">527 KiB</td></tr>
<tr><td align="right">505</td><td><code>arj</code></td><td>3.10.22-29</td><td>archiver for .arj files</td><td align="right">526 KiB</td></tr>
<tr><td align="right">506</td><td><code>libkdegames6-6</code></td><td>4:25.12.3-0ubuntu1</td><td>shared library for KDE games</td><td align="right">522 KiB</td></tr>
<tr><td align="right">507</td><td><code>libkf6pulseaudioqt5</code></td><td>1.7.0-1build1</td><td>Pulseaudio bindings library for Qt 6</td><td align="right">522 KiB</td></tr>
<tr><td align="right">508</td><td><code>python3-asn1crypto</code></td><td>1.5.1-3build1</td><td>Fast ASN.1 parser and serializer (Python 3)</td><td align="right">518 KiB</td></tr>
<tr><td align="right">509</td><td><code>libkf5declarative-data</code></td><td>5.116.0-1ubuntu1</td><td>provides integration of QML and KDE frameworks</td><td align="right">517 KiB</td></tr>
<tr><td align="right">510</td><td><code>libphysfs-dev</code></td><td>3.2.0-1</td><td>filesystem abstraction library for game programmers (development headers)</td><td align="right">516 KiB</td></tr>
<tr><td align="right">511</td><td><code>libkf6jobwidgets-data</code></td><td>6.24.0-0ubuntu1</td><td>widgets for tracking KJob instances - data files</td><td align="right">515 KiB</td></tr>
<tr><td align="right">512</td><td><code>qrca</code></td><td>25.12.3-0ubuntu1</td><td>QR code scanner and generator</td><td align="right">515 KiB</td></tr>
<tr><td align="right">513</td><td><code>kubuntu-wallpapers</code></td><td>26.04.3</td><td>A collection of Kubuntu wallpapers for Resolute Raccoon.</td><td align="right">514 KiB</td></tr>
<tr><td align="right">514</td><td><code>plasma-calendar-addons</code></td><td>4:6.6.4-0ubuntu1</td><td>additional calendar plugins for Plasma 6</td><td align="right">514 KiB</td></tr>
<tr><td align="right">515</td><td><code>libqt5test5t64</code></td><td>5.15.18+dfsg-1ubuntu1</td><td>Qt 5 test module</td><td align="right">511 KiB</td></tr>
<tr><td align="right">516</td><td><code>libqt6wlshellintegration6</code></td><td>6.10.2+dfsg-7</td><td>Qt 6 Wayland ShellIntegration library</td><td align="right">507 KiB</td></tr>
<tr><td align="right">517</td><td><code>libqt6sql6</code></td><td>6.10.2+dfsg-7</td><td>Qt 6 SQL module</td><td align="right">506 KiB</td></tr>
<tr><td align="right">518</td><td><code>libkf6bookmarks-data</code></td><td>6.24.0-0ubuntu1</td><td>Qt library with support for bookmarks and the XBEL format</td><td align="right">505 KiB</td></tr>
<tr><td align="right">519</td><td><code>libkf6declarative-data</code></td><td>6.24.0-0ubuntu1</td><td>provides integration of QML and KDE frameworks</td><td align="right">505 KiB</td></tr>
<tr><td align="right">520</td><td><code>libkmpris6</code></td><td>4:6.6.4-0ubuntu2</td><td>Plasma Workspace MPRIS library</td><td align="right">503 KiB</td></tr>
<tr><td align="right">521</td><td><code>qml6-module-qtmultimedia</code></td><td>6.10.2-2</td><td>Qt 6 Multimedia QML module</td><td align="right">501 KiB</td></tr>
<tr><td align="right">522</td><td><code>libqt5svg5</code></td><td>5.15.18-1</td><td>Qt 5 SVG module</td><td align="right">497 KiB</td></tr>
<tr><td align="right">523</td><td><code>libkworkspace6-6</code></td><td>4:6.6.4-0ubuntu2</td><td>Plasma Workspace library</td><td align="right">496 KiB</td></tr>
<tr><td align="right">524</td><td><code>libksysguardsensorfaces2</code></td><td>4:6.6.4-0ubuntu1</td><td>library for system monitoring - ksysguardsensorfaces shared library</td><td align="right">494 KiB</td></tr>
<tr><td align="right">525</td><td><code>libkf5jobwidgets-data</code></td><td>5.116.0-1ubuntu1</td><td>Widgets for tracking KJob instances</td><td align="right">492 KiB</td></tr>
<tr><td align="right">526</td><td><code>libsfml-audio3.0</code></td><td>3.0.2+dfsg-2</td><td>Simple and Fast Multimedia Library - Audio part</td><td align="right">491 KiB</td></tr>
<tr><td align="right">527</td><td><code>libsocket++1</code></td><td>1.12.13+git20131030.5d039ba-2</td><td>lightweight convenience library to handle low level BSD sockets in C++ - libs</td><td align="right">490 KiB</td></tr>
<tr><td align="right">528</td><td><code>qt5-gtk-platformtheme</code></td><td>5.15.18+dfsg-1ubuntu1</td><td>Qt 5 GTK+ 3 platform theme</td><td align="right">490 KiB</td></tr>
<tr><td align="right">529</td><td><code>libqt6positioningquick6</code></td><td>6.10.2-1</td><td>Qt 6 Positioning library - Qt Quick library</td><td align="right">489 KiB</td></tr>
<tr><td align="right">530</td><td><code>python3-tqdm</code></td><td>4.67.3-1build1</td><td>fast, extensible progress bar for Python 3 and CLI tool</td><td align="right">489 KiB</td></tr>
<tr><td align="right">531</td><td><code>libsuperlu7</code></td><td>7.0.1+dfsg1-2build1</td><td>Direct solution of large, sparse systems of linear equations</td><td align="right">487 KiB</td></tr>
<tr><td align="right">532</td><td><code>kde-config-plymouth</code></td><td>6.6.4-0ubuntu1</td><td>KCM for Plymouth</td><td align="right">486 KiB</td></tr>
<tr><td align="right">533</td><td><code>librttopo1</code></td><td>1.1.0-4build1</td><td>Tuscany Region topology library</td><td align="right">486 KiB</td></tr>
<tr><td align="right">534</td><td><code>libspdlog1.15</code></td><td>1:1.15.3+ds-1build1</td><td>Very fast C++ logging library</td><td align="right">483 KiB</td></tr>
<tr><td align="right">535</td><td><code>qml6-module-qtquick-layouts</code></td><td>6.10.2+dfsg-3</td><td>Qt 6 Quick Layouts QML module</td><td align="right">483 KiB</td></tr>
<tr><td align="right">536</td><td><code>kaccounts-providers</code></td><td>4:25.12.3-0ubuntu1</td><td>KDE providers for accounts sign-on</td><td align="right">482 KiB</td></tr>
<tr><td align="right">537</td><td><code>libmuparser2v5</code></td><td>2.3.4-2</td><td>fast mathematical expressions parser library (runtime)</td><td align="right">482 KiB</td></tr>
<tr><td align="right">538</td><td><code>libkirigamitemplates6</code></td><td>6.24.0-0ubuntu1</td><td>set of QtQuick components targeted for mobile use</td><td align="right">481 KiB</td></tr>
<tr><td align="right">539</td><td><code>libkf5configwidgets5</code></td><td>5.116.0-2</td><td>Extra widgets for easier configuration support.</td><td align="right">480 KiB</td></tr>
<tr><td align="right">540</td><td><code>libkf5sonnetcore5</code></td><td>5.116.0-1ubuntu2</td><td>spell checking library for Qt, core lib</td><td align="right">479 KiB</td></tr>
<tr><td align="right">541</td><td><code>libkf6service6</code></td><td>6.24.0-0ubuntu1</td><td>Advanced plugin and service introspection</td><td align="right">477 KiB</td></tr>
<tr><td align="right">542</td><td><code>gnustep-base-common</code></td><td>1.31.1-4ubuntu2</td><td>GNUstep Base library - common files</td><td align="right">475 KiB</td></tr>
<tr><td align="right">543</td><td><code>libqt6quick3dutils6</code></td><td>6.10.2-1</td><td>Qt 6 Quick 3D Utils library</td><td align="right">469 KiB</td></tr>
<tr><td align="right">544</td><td><code>libsixel1</code></td><td>1.10.5-1build1</td><td>DEC SIXEL graphics codec implementation (runtime)</td><td align="right">465 KiB</td></tr>
<tr><td align="right">545</td><td><code>kio-admin</code></td><td>25.12.3-0ubuntu1</td><td>manage files as administrator using the admin:// KIO protocol</td><td align="right">464 KiB</td></tr>
<tr><td align="right">546</td><td><code>libqt5xml5t64</code></td><td>5.15.18+dfsg-1ubuntu1</td><td>Qt 5 XML module</td><td align="right">464 KiB</td></tr>
<tr><td align="right">547</td><td><code>imagemagick-7-common</code></td><td>8:7.1.2.18+dfsg1-1</td><td>image manipulation programs -- infrastructure</td><td align="right">461 KiB</td></tr>
<tr><td align="right">548</td><td><code>kunifiedpush</code></td><td>25.12.3-0ubuntu1</td><td>UnifiedPush distributor daemon</td><td align="right">460 KiB</td></tr>
<tr><td align="right">549</td><td><code>libkdecorations3-6</code></td><td>4:6.6.4-0ubuntu1</td><td>library to create window decorations</td><td align="right">460 KiB</td></tr>
<tr><td align="right">550</td><td><code>libkf5configcore5</code></td><td>5.116.0-2</td><td>configuration settings framework for Qt</td><td align="right">460 KiB</td></tr>
<tr><td align="right">551</td><td><code>qml-module-org-kde-qqc2desktopstyle</code></td><td>5.116.1-2</td><td>Qt Quick Controls 2: Desktop Style</td><td align="right">458 KiB</td></tr>
<tr><td align="right">552</td><td><code>libkf6guiaddons6</code></td><td>6.24.0-0ubuntu1.1</td><td>additional addons for QtGui</td><td align="right">455 KiB</td></tr>
<tr><td align="right">553</td><td><code>libopencv-imgcodecs410</code></td><td>4.10.0+dfsg-7ubuntu5</td><td>computer vision Image Codecs library</td><td align="right">455 KiB</td></tr>
<tr><td align="right">554</td><td><code>libplasma7</code></td><td>6.6.4-0ubuntu1</td><td>Plasma Runtime components</td><td align="right">451 KiB</td></tr>
<tr><td align="right">555</td><td><code>libqt5sql5t64</code></td><td>5.15.18+dfsg-1ubuntu1</td><td>Qt 5 SQL module</td><td align="right">451 KiB</td></tr>
<tr><td align="right">556</td><td><code>plasma-integration</code></td><td>6.6.4-0ubuntu1</td><td>Qt Platform Theme integration plugins for KDE Plasma 6</td><td align="right">448 KiB</td></tr>
<tr><td align="right">557</td><td><code>polkit-kde-agent-1</code></td><td>4:6.6.4-0ubuntu1</td><td>KDE dialogs for PolicyKit</td><td align="right">447 KiB</td></tr>
<tr><td align="right">558</td><td><code>liblua5.2-0</code></td><td>5.2.4-4</td><td>Shared library for the Lua interpreter version 5.2</td><td align="right">443 KiB</td></tr>
<tr><td align="right">559</td><td><code>libkf6screen8</code></td><td>4:6.6.4-0ubuntu1</td><td>library for screen management - shared library</td><td align="right">438 KiB</td></tr>
<tr><td align="right">560</td><td><code>libqt6quickshapes6</code></td><td>6.10.2+dfsg-3</td><td>Qt 6 Quick Shapes library</td><td align="right">438 KiB</td></tr>
<tr><td align="right">561</td><td><code>python3.14-tk</code></td><td>3.14.4-1</td><td>Tkinter - Writing Tk applications with Python (v3.14)</td><td align="right">438 KiB</td></tr>
<tr><td align="right">562</td><td><code>kde-inotify-survey</code></td><td>25.12.3-0ubuntu1</td><td>Notifies of excessive use of inotify watches</td><td align="right">435 KiB</td></tr>
<tr><td align="right">563</td><td><code>libjxr0t64</code></td><td>1.2~git20170615.f752187-5.3build1</td><td>JPEG-XR lib - libraries</td><td align="right">433 KiB</td></tr>
<tr><td align="right">564</td><td><code>libkf5i18n5</code></td><td>5.116.0-1ubuntu4</td><td>Advanced internationalization framework.</td><td align="right">431 KiB</td></tr>
<tr><td align="right">565</td><td><code>libkf6parts-data</code></td><td>6.24.0-0ubuntu1</td><td>Document centric plugin system</td><td align="right">429 KiB</td></tr>
<tr><td align="right">566</td><td><code>python3-mechanize</code></td><td>1:0.4.10+ds-7</td><td>stateful programmatic web browsing</td><td align="right">429 KiB</td></tr>
<tr><td align="right">567</td><td><code>qml6-module-org-kde-kirigamiaddons-datetime</code></td><td>1.11.0-2ubuntu2</td><td>datetime module for QML</td><td align="right">426 KiB</td></tr>
<tr><td align="right">568</td><td><code>libkf6configwidgets6</code></td><td>6.24.0-0ubuntu1</td><td>Extra widgets for easier configuration support</td><td align="right">425 KiB</td></tr>
<tr><td align="right">569</td><td><code>liblua5.1-0</code></td><td>5.1.5-12</td><td>Shared library for the Lua interpreter version 5.1</td><td align="right">423 KiB</td></tr>
<tr><td align="right">570</td><td><code>libsfml-graphics3.0</code></td><td>3.0.2+dfsg-2</td><td>Simple and Fast Multimedia Library - Graphics part</td><td align="right">423 KiB</td></tr>
<tr><td align="right">571</td><td><code>qml6-module-qtquick-effects</code></td><td>6.10.2+dfsg-3</td><td>Qt 6 Quick Effects QML module</td><td align="right">423 KiB</td></tr>
<tr><td align="right">572</td><td><code>libkexiv2qt6-0</code></td><td>25.12.3-0ubuntu1</td><td>Qt like interface for the libexiv2 library</td><td align="right">422 KiB</td></tr>
<tr><td align="right">573</td><td><code>librubberband3</code></td><td>4.0.0+dfsg-2ubuntu1</td><td>audio time-stretching and pitch-shifting library</td><td align="right">419 KiB</td></tr>
<tr><td align="right">574</td><td><code>libhwloc15</code></td><td>2.13.0-2</td><td>Hierarchical view of the machine - shared libs</td><td align="right">418 KiB</td></tr>
<tr><td align="right">575</td><td><code>libkf5archive-data</code></td><td>5.116.0-1ubuntu1</td><td>data files for karchive</td><td align="right">418 KiB</td></tr>
<tr><td align="right">576</td><td><code>libksanecore6-1</code></td><td>25.12.3-0ubuntu1</td><td>library providing logic to interface scanners</td><td align="right">414 KiB</td></tr>
<tr><td align="right">577</td><td><code>libqt6core5compat6</code></td><td>6.10.2-1</td><td>Qt 6 Qt5Compat library</td><td align="right">411 KiB</td></tr>
<tr><td align="right">578</td><td><code>libkf6itemmodels6</code></td><td>6.24.0-0ubuntu1</td><td>additional item/view models for Qt Itemview</td><td align="right">410 KiB</td></tr>
<tr><td align="right">579</td><td><code>libopenxr-loader1</code></td><td>1.1.47~ds-2build1</td><td>OpenXR loader library</td><td align="right">410 KiB</td></tr>
<tr><td align="right">580</td><td><code>libqt6labsplatform6</code></td><td>6.10.2+dfsg-3</td><td>Qt 6 Labs Platform library</td><td align="right">410 KiB</td></tr>
<tr><td align="right">581</td><td><code>libkirigamiplatform6</code></td><td>6.24.0-0ubuntu1</td><td>set of QtQuick components targeted for mobile use</td><td align="right">408 KiB</td></tr>
<tr><td align="right">582</td><td><code>libvpl2</code></td><td>1:2.16.0-1</td><td>Intel Video Processing Library -- shared library</td><td align="right">408 KiB</td></tr>
<tr><td align="right">583</td><td><code>libgvpr2</code></td><td>14.1.2-1ubuntu1</td><td>rich set of graph drawing tools - gvpr library</td><td align="right">407 KiB</td></tr>
<tr><td align="right">584</td><td><code>libpackagekitqt6-2</code></td><td>1.1.4-1</td><td>Library for accessing PackageKit using Qt6</td><td align="right">406 KiB</td></tr>
<tr><td align="right">585</td><td><code>libgeos-c1t64</code></td><td>3.14.1-2</td><td>Geometry engine for Geographic Information Systems - C Library</td><td align="right">403 KiB</td></tr>
<tr><td align="right">586</td><td><code>libmusicbrainz5cc2v5</code></td><td>5.1.0+git20150707-12</td><td>Library to access the MusicBrainz.org database</td><td align="right">401 KiB</td></tr>
<tr><td align="right">587</td><td><code>libzip-dev</code></td><td>1.11.4-2</td><td>library for reading, creating, and modifying zip archives (development)</td><td align="right">401 KiB</td></tr>
<tr><td align="right">588</td><td><code>libnorm1t64</code></td><td>1.5.9+dfsg-4</td><td>NACK-Oriented Reliable Multicast (NORM) library</td><td align="right">400 KiB</td></tr>
<tr><td align="right">589</td><td><code>libuniconf4.6t64</code></td><td>4.6.1-19</td><td>C++ network libraries for rapid application development</td><td align="right">400 KiB</td></tr>
<tr><td align="right">590</td><td><code>libquickcharts1</code></td><td>6.24.0-0ubuntu1</td><td>Quick Charts</td><td align="right">397 KiB</td></tr>
<tr><td align="right">591</td><td><code>libde265-0</code></td><td>1.0.16-1build1</td><td>Open H.265 video codec implementation</td><td align="right">396 KiB</td></tr>
<tr><td align="right">592</td><td><code>libkf6service-data</code></td><td>6.24.0-0ubuntu1</td><td>Advanced plugin and service introspection</td><td align="right">396 KiB</td></tr>
<tr><td align="right">593</td><td><code>libkf6auth-data</code></td><td>6.24.0-0ubuntu1</td><td>Abstraction to system policy and authentication features</td><td align="right">393 KiB</td></tr>
<tr><td align="right">594</td><td><code>qml6-module-qtwebengine-controlsdelegates</code></td><td>6.10.2+dfsg-1</td><td>Qt 6 WebEngine Controls Delegates QML module</td><td align="right">392 KiB</td></tr>
<tr><td align="right">595</td><td><code>libklookandfeel6</code></td><td>4:6.6.4-0ubuntu2</td><td>Plasma Workspaces KLookAndFeel library</td><td align="right">390 KiB</td></tr>
<tr><td align="right">596</td><td><code>lm-sensors</code></td><td>1:3.6.2-2build1</td><td>utilities to read temperature/voltage/fan sensors</td><td align="right">390 KiB</td></tr>
<tr><td align="right">597</td><td><code>libkf6unitconversion6</code></td><td>6.24.0-0ubuntu1</td><td>Support for unit conversion</td><td align="right">389 KiB</td></tr>
<tr><td align="right">598</td><td><code>libkirigamiprimitives6</code></td><td>6.24.0-0ubuntu1</td><td>set of QtQuick components targeted for mobile use</td><td align="right">389 KiB</td></tr>
<tr><td align="right">599</td><td><code>libkf5auth-data</code></td><td>5.116.0-1ubuntu1</td><td>Abstraction to system policy and authentication features</td><td align="right">388 KiB</td></tr>
<tr><td align="right">600</td><td><code>libglu1-mesa</code></td><td>9.0.2-1.1build2</td><td>Mesa OpenGL utility library (GLU)</td><td align="right">382 KiB</td></tr>
<tr><td align="right">601</td><td><code>libkf5globalaccel-data</code></td><td>5.116.0-1ubuntu1</td><td>Configurable global shortcut support.</td><td align="right">379 KiB</td></tr>
<tr><td align="right">602</td><td><code>libkf5notifications5</code></td><td>5.116.0-2</td><td>Framework for desktop notifications</td><td align="right">378 KiB</td></tr>
<tr><td align="right">603</td><td><code>libolm3</code></td><td>3.2.16+dfsg-5</td><td>implementation of the Double Ratchet cryptographic ratchet</td><td align="right">376 KiB</td></tr>
<tr><td align="right">604</td><td><code>libkcompactdisc6-5</code></td><td>4:25.12.3-0ubuntu1</td><td>CD drive library for KDE Platform (runtime)</td><td align="right">375 KiB</td></tr>
<tr><td align="right">605</td><td><code>libkf6runner6</code></td><td>6.24.0-0ubuntu1</td><td>Used to write plugins loaded at runtime called &quot;Runners&quot;</td><td align="right">375 KiB</td></tr>
<tr><td align="right">606</td><td><code>libutf8proc3</code></td><td>2.10.0-2</td><td>C library for processing UTF-8 Unicode data (shared library)</td><td align="right">374 KiB</td></tr>
<tr><td align="right">607</td><td><code>kde-config-gtk-style</code></td><td>4:6.6.4-0ubuntu1</td><td>KDE configuration module for GTK+ 2.x and GTK+ 3.x styles selection</td><td align="right">373 KiB</td></tr>
<tr><td align="right">608</td><td><code>qml6-module-org-kde-kirigamiaddons-settings</code></td><td>1.11.0-2ubuntu2</td><td>settings module for QML</td><td align="right">373 KiB</td></tr>
<tr><td align="right">609</td><td><code>libksysguardsensors2</code></td><td>4:6.6.4-0ubuntu1</td><td>library for system monitoring - ksysguardsensors shared library</td><td align="right">371 KiB</td></tr>
<tr><td align="right">610</td><td><code>qt6-location-plugins</code></td><td>6.10.2-1</td><td>Qt 6 Location plugins</td><td align="right">369 KiB</td></tr>
<tr><td align="right">611</td><td><code>phonon4qt6-backend-vlc</code></td><td>0.12.0-3build5</td><td>Phonon4Qt6 VLC backend</td><td align="right">368 KiB</td></tr>
<tr><td align="right">612</td><td><code>bup-doc</code></td><td>0.33.9-1.2build1</td><td>highly efficient file backup system based on git (documentation)</td><td align="right">367 KiB</td></tr>
<tr><td align="right">613</td><td><code>libkf6notifyconfig-data</code></td><td>6.24.0-0ubuntu1</td><td>Configuration system for KNotify</td><td align="right">365 KiB</td></tr>
<tr><td align="right">614</td><td><code>libglut3.12</code></td><td>3.4.0-6</td><td>OpenGL Utility Toolkit</td><td align="right">364 KiB</td></tr>
<tr><td align="right">615</td><td><code>qml6-module-qtquick-templates</code></td><td>6.10.2+dfsg-3</td><td>Qt 6 Quick Templates QML module</td><td align="right">363 KiB</td></tr>
<tr><td align="right">616</td><td><code>libkglobalacceld0</code></td><td>6.6.4-0ubuntu1</td><td>daemon for global keyboard shortcuts on the Plasma desktop - shared library</td><td align="right">362 KiB</td></tr>
<tr><td align="right">617</td><td><code>libkmahjongg6</code></td><td>4:25.12.3-0ubuntu1</td><td>shared library for kmahjongg and kshisen</td><td align="right">362 KiB</td></tr>
<tr><td align="right">618</td><td><code>qt6-gtk-platformtheme</code></td><td>6.10.2+dfsg-7</td><td>Qt 6 GTK+ 3 platform theme</td><td align="right">361 KiB</td></tr>
<tr><td align="right">619</td><td><code>libmodplug1</code></td><td>1:0.8.9.0-3build2</td><td>shared libraries for mod music based on ModPlug</td><td align="right">360 KiB</td></tr>
<tr><td align="right">620</td><td><code>edid-decode</code></td><td>0.1~git20220315.cb74358c2896-1.1build1</td><td>decode the binary EDID information from monitors</td><td align="right">358 KiB</td></tr>
<tr><td align="right">621</td><td><code>libkf6statusnotifieritem6</code></td><td>6.24.0-0ubuntu1</td><td>Implementation of Status Notifier Items</td><td align="right">357 KiB</td></tr>
<tr><td align="right">622</td><td><code>qml6-module-org-kde-kirigamiaddons-delegates</code></td><td>1.11.0-2ubuntu2</td><td>delegates module for QML</td><td align="right">357 KiB</td></tr>
<tr><td align="right">623</td><td><code>python3-pooch</code></td><td>1.9.0-1</td><td>Tools to manage Python library&#x27;s data files</td><td align="right">355 KiB</td></tr>
<tr><td align="right">624</td><td><code>libkf5bookmarks5</code></td><td>5.116.0-1ubuntu1</td><td>Qt library with support for bookmarks and the XBEL format.</td><td align="right">353 KiB</td></tr>
<tr><td align="right">625</td><td><code>libmujs3</code></td><td>1.3.8-2</td><td>Lightweight JavaScript interpreter library</td><td align="right">353 KiB</td></tr>
<tr><td align="right">626</td><td><code>qt6-qpa-plugins</code></td><td>6.10.2+dfsg-7</td><td>Qt 6 QPA plugins</td><td align="right">353 KiB</td></tr>
<tr><td align="right">627</td><td><code>libkf6itemviews6</code></td><td>6.24.0-0ubuntu1</td><td>Qt library with additional widgets for ItemModels</td><td align="right">352 KiB</td></tr>
<tr><td align="right">628</td><td><code>libkf6iconthemes-data</code></td><td>6.24.0-0ubuntu2</td><td>Support for icon themes &amp; widgets</td><td align="right">351 KiB</td></tr>
<tr><td align="right">629</td><td><code>libkf6notifications-data</code></td><td>6.24.0-0ubuntu1</td><td>Framework for desktop notifications data</td><td align="right">351 KiB</td></tr>
<tr><td align="right">630</td><td><code>fonts-lyx</code></td><td>2.5.0-1</td><td>TrueType versions of some TeX fonts used by LyX</td><td align="right">348 KiB</td></tr>
<tr><td align="right">631</td><td><code>libkf6sonnetcore6</code></td><td>6.24.0-0ubuntu1</td><td>spell checking library for Qt, core lib</td><td align="right">347 KiB</td></tr>
<tr><td align="right">632</td><td><code>libnetpbm11t64</code></td><td>2:11.10.02-1build1</td><td>Graphics conversion tools shared libraries</td><td align="right">347 KiB</td></tr>
<tr><td align="right">633</td><td><code>libqt6webenginewidgets6</code></td><td>6.10.2+dfsg-1</td><td>Qt 6 WebEngine Widgets library</td><td align="right">347 KiB</td></tr>
<tr><td align="right">634</td><td><code>libcmark0.30.2</code></td><td>0.30.2-6build2</td><td>CommonMark parsing and rendering library</td><td align="right">346 KiB</td></tr>
<tr><td align="right">635</td><td><code>libkf5kiogui5</code></td><td>5.116.0-2</td><td>resource and network access abstraction (KIO gui library)</td><td align="right">344 KiB</td></tr>
<tr><td align="right">636</td><td><code>libgme0</code></td><td>0.6.4-1</td><td>Playback library for video game music files - shared library</td><td align="right">341 KiB</td></tr>
<tr><td align="right">637</td><td><code>pulseaudio-utils</code></td><td>1:17.0+dfsg1-2ubuntu4</td><td>Command line tools for the PulseAudio sound server</td><td align="right">339 KiB</td></tr>
<tr><td align="right">638</td><td><code>kde-style-breeze-data</code></td><td>4:6.6.4-0ubuntu1</td><td>Widget style for Qt and KDE Software - data files</td><td align="right">336 KiB</td></tr>
<tr><td align="right">639</td><td><code>libkf5iconthemes-data</code></td><td>5.116.0-1ubuntu4</td><td>Support for icon themes.</td><td align="right">335 KiB</td></tr>
<tr><td align="right">640</td><td><code>libkf6notifications6</code></td><td>6.24.0-0ubuntu1</td><td>Framework for desktop notifications library</td><td align="right">335 KiB</td></tr>
<tr><td align="right">641</td><td><code>qml6-module-qtwebengine</code></td><td>6.10.2+dfsg-1</td><td>Qt 6 WebEngine QML module</td><td align="right">335 KiB</td></tr>
<tr><td align="right">642</td><td><code>libkf6svg6</code></td><td>6.24.0-0ubuntu1</td><td>library for rendering SVG-based themes</td><td align="right">334 KiB</td></tr>
<tr><td align="right">643</td><td><code>libimath-3-1-29t64</code></td><td>3.1.12-1ubuntu5</td><td>Utility libraries from ASF used by OpenEXR - runtime</td><td align="right">333 KiB</td></tr>
<tr><td align="right">644</td><td><code>libkf6jobwidgets6</code></td><td>6.24.0-0ubuntu1</td><td>widgets for tracking KJob instances</td><td align="right">332 KiB</td></tr>
<tr><td align="right">645</td><td><code>libkf6people6</code></td><td>6.24.0-0ubuntu1</td><td>framework providing unified access to contacts aggregated by person</td><td align="right">332 KiB</td></tr>
<tr><td align="right">646</td><td><code>libkf6pty-data</code></td><td>6.24.0-0ubuntu1</td><td>Pty abstraction</td><td align="right">331 KiB</td></tr>
<tr><td align="right">647</td><td><code>libphonon-l10n</code></td><td>4:4.12.0-7</td><td>multimedia framework from KDE using Qt - localization files</td><td align="right">331 KiB</td></tr>
<tr><td align="right">648</td><td><code>libkf6archive6</code></td><td>6.24.0-0ubuntu1</td><td>Qt 6 addon providing access to numerous types of archives</td><td align="right">330 KiB</td></tr>
<tr><td align="right">649</td><td><code>libkf6itemviews-data</code></td><td>6.24.0-0ubuntu1</td><td>Qt library with additional widgets for ItemModels</td><td align="right">330 KiB</td></tr>
<tr><td align="right">650</td><td><code>libbatterycontrol6</code></td><td>4:6.6.4-0ubuntu2</td><td>Plasma Workspace battery control library</td><td align="right">328 KiB</td></tr>
<tr><td align="right">651</td><td><code>libkf6dnssd-data</code></td><td>6.24.0-0ubuntu1</td><td>Abstraction to system DNSSD features</td><td align="right">327 KiB</td></tr>
<tr><td align="right">652</td><td><code>libkf6windowsystem-data</code></td><td>6.24.0-0ubuntu1</td><td>Convenience access to certain properties and features of the window manager</td><td align="right">327 KiB</td></tr>
<tr><td align="right">653</td><td><code>libqt6xml6</code></td><td>6.10.2+dfsg-7</td><td>Qt 6 XML module</td><td align="right">327 KiB</td></tr>
<tr><td align="right">654</td><td><code>libkf6textwidgets6</code></td><td>6.24.0-0ubuntu1</td><td>Advanced text editing widgets</td><td align="right">326 KiB</td></tr>
<tr><td align="right">655</td><td><code>libkf5windowsystem-data</code></td><td>5.116.0-1ubuntu1</td><td>Convenience access to certain properties and features of the window manager</td><td align="right">325 KiB</td></tr>
<tr><td align="right">656</td><td><code>libkf5iconthemes5</code></td><td>5.116.0-1ubuntu4</td><td>Support for icon themes.</td><td align="right">324 KiB</td></tr>
<tr><td align="right">657</td><td><code>libkimageannotator-common</code></td><td>0.7.2-2</td><td>Image Annotating Library (common data files)</td><td align="right">324 KiB</td></tr>
<tr><td align="right">658</td><td><code>qml6-module-org-kde-ksysguard</code></td><td>4:6.6.4-0ubuntu1</td><td>KSysGuard QtDeclarative QML Support</td><td align="right">324 KiB</td></tr>
<tr><td align="right">659</td><td><code>libkf6completion-data</code></td><td>6.24.0-0ubuntu1</td><td>Widgets with advanced auto-completion features</td><td align="right">323 KiB</td></tr>
<tr><td align="right">660</td><td><code>libbluray3</code></td><td>1:1.4.1-1</td><td>Blu-ray disc playback support library (shared library)</td><td align="right">322 KiB</td></tr>
<tr><td align="right">661</td><td><code>libcharls2</code></td><td>2.4.2-2build3</td><td>Implementation of the JPEG-LS standard</td><td align="right">322 KiB</td></tr>
<tr><td align="right">662</td><td><code>libkf6globalaccel-data</code></td><td>6.24.0-0ubuntu1</td><td>Configurable global shortcut support</td><td align="right">322 KiB</td></tr>
<tr><td align="right">663</td><td><code>libqaccessibilityclient-qt6-0</code></td><td>0.6.0-4ubuntu1</td><td>helper to make writing accessibility tools easier</td><td align="right">322 KiB</td></tr>
<tr><td align="right">664</td><td><code>plasma-wallpapers-addons</code></td><td>4:6.6.4-0ubuntu1</td><td>additional wallpaper plugins for Plasma 6</td><td align="right">321 KiB</td></tr>
<tr><td align="right">665</td><td><code>libkdsoap-qt6-2</code></td><td>2.2.0+dfsg-4ubuntu1</td><td>Qt-based client-side and server-side SOAP component - client-side library</td><td align="right">320 KiB</td></tr>
<tr><td align="right">666</td><td><code>libkf5notifications-data</code></td><td>5.116.0-2</td><td>Framework for desktop notifications</td><td align="right">320 KiB</td></tr>
<tr><td align="right">667</td><td><code>libkf6completion6</code></td><td>6.24.0-0ubuntu1</td><td>Widgets with advanced auto-completion features</td><td align="right">320 KiB</td></tr>
<tr><td align="right">668</td><td><code>plasma-discover-backend-snap</code></td><td>6.6.4-0ubuntu1</td><td>Discover software management suite - Snap backend</td><td align="right">320 KiB</td></tr>
<tr><td align="right">669</td><td><code>libsignon-qt6-1</code></td><td>8.61+git20231015.c8ad982-8</td><td>Single Sign On framework - Qt6 shared libraries</td><td align="right">319 KiB</td></tr>
<tr><td align="right">670</td><td><code>libkf5completion-data</code></td><td>5.116.0-1ubuntu1</td><td>Widgets with advanced auto-completion features.</td><td align="right">318 KiB</td></tr>
<tr><td align="right">671</td><td><code>libarpack2t64</code></td><td>3.9.1-6build1</td><td>Fortran77 subroutines to solve large scale eigenvalue problems</td><td align="right">317 KiB</td></tr>
<tr><td align="right">672</td><td><code>libkf5itemviews-data</code></td><td>5.116.0-1ubuntu1</td><td>Qt library with additional widgets for ItemModels</td><td align="right">317 KiB</td></tr>
<tr><td align="right">673</td><td><code>signon-ui-qt</code></td><td>0.17+git20231016.eef943f-3build1</td><td>single Sign-on UI based on Qt</td><td align="right">317 KiB</td></tr>
<tr><td align="right">674</td><td><code>libvlc5</code></td><td>3.0.23-1</td><td>multimedia player and streamer library</td><td align="right">316 KiB</td></tr>
<tr><td align="right">675</td><td><code>libfyba0t64</code></td><td>4.1.1-11build2</td><td>FYBA library to read and write Norwegian geodata standard format SOSI</td><td align="right">315 KiB</td></tr>
<tr><td align="right">676</td><td><code>libkfontinst6</code></td><td>4:6.6.4-0ubuntu2</td><td>Plasma Workspace fonts installation library</td><td align="right">315 KiB</td></tr>
<tr><td align="right">677</td><td><code>frotz</code></td><td>2.55+dfsg-3</td><td>interpreter of Z-code story-files</td><td align="right">314 KiB</td></tr>
<tr><td align="right">678</td><td><code>libkf6userfeedbackcore6</code></td><td>6.24.0-0ubuntu1</td><td>user feedback for applications - core library</td><td align="right">313 KiB</td></tr>
<tr><td align="right">679</td><td><code>libkf6parts6</code></td><td>6.24.0-0ubuntu1</td><td>Document centric plugin system</td><td align="right">312 KiB</td></tr>
<tr><td align="right">680</td><td><code>qml6-module-org-kde-kirigamiaddons-statefulapp</code></td><td>1.11.0-2ubuntu2</td><td>statefulapp module for QML</td><td align="right">312 KiB</td></tr>
<tr><td align="right">681</td><td><code>qt6-documentation-tools</code></td><td>6.10.2-1</td><td>Qt 6 documentation tools</td><td align="right">311 KiB</td></tr>
<tr><td align="right">682</td><td><code>libkirigami-data</code></td><td>6.24.0-0ubuntu1</td><td>set of QtQuick components targeted for mobile use</td><td align="right">308 KiB</td></tr>
<tr><td align="right">683</td><td><code>libpgm-5.3-0t64</code></td><td>5.3.128~dfsg-2.1build2</td><td>OpenPGM shared library</td><td align="right">307 KiB</td></tr>
<tr><td align="right">684</td><td><code>libts0t64</code></td><td>1.22-1.1build2</td><td>touch screen library</td><td align="right">307 KiB</td></tr>
<tr><td align="right">685</td><td><code>libkf5textwidgets5</code></td><td>5.116.0-1ubuntu1</td><td>Advanced text editing widgets.</td><td align="right">306 KiB</td></tr>
<tr><td align="right">686</td><td><code>libzopfli1</code></td><td>1.0.3-3build1</td><td>zlib (gzip, deflate) compatible compressor - shared library</td><td align="right">306 KiB</td></tr>
<tr><td align="right">687</td><td><code>libkf5completion5</code></td><td>5.116.0-1ubuntu1</td><td>Widgets with advanced auto-completion features.</td><td align="right">305 KiB</td></tr>
<tr><td align="right">688</td><td><code>libsfml-window3.0</code></td><td>3.0.2+dfsg-2</td><td>Simple and Fast Multimedia Library - Window part</td><td align="right">302 KiB</td></tr>
<tr><td align="right">689</td><td><code>qml6-module-qt-labs-qmlmodels</code></td><td>6.10.2+dfsg-3</td><td>Qt 6 QML Models QML module</td><td align="right">300 KiB</td></tr>
<tr><td align="right">690</td><td><code>libkf5archive5</code></td><td>5.116.0-1ubuntu1</td><td>Qt 5 addon providing access to numerous types of archives</td><td align="right">299 KiB</td></tr>
<tr><td align="right">691</td><td><code>libkf5guiaddons5</code></td><td>5.116.0-2ubuntu1</td><td>additional addons for QtGui</td><td align="right">299 KiB</td></tr>
<tr><td align="right">692</td><td><code>plasma-discover-notifier</code></td><td>6.6.4-0ubuntu1</td><td>Discover software management suite - Update Notifier</td><td align="right">299 KiB</td></tr>
<tr><td align="right">693</td><td><code>debian-archive-keyring</code></td><td>2025.1ubuntu1</td><td>OpenPGP archive certificates of the Debian archive</td><td align="right">298 KiB</td></tr>
<tr><td align="right">694</td><td><code>libtbb12</code></td><td>2022.3.0-2</td><td>parallelism library for C++ - runtime files</td><td align="right">298 KiB</td></tr>
<tr><td align="right">695</td><td><code>libmagickcore-7.q16-10-extra</code></td><td>8:7.1.2.18+dfsg1-1</td><td>low-level image manipulation library - extra codecs (Q16)</td><td align="right">297 KiB</td></tr>
<tr><td align="right">696</td><td><code>qml6-module-org-kde-bluezqt</code></td><td>6.24.0-0ubuntu1</td><td>QML wrapper for bluez</td><td align="right">296 KiB</td></tr>
<tr><td align="right">697</td><td><code>xa65</code></td><td>2.4.1-0.1build1</td><td>cross-assembler and utility suite for 65xx/65816 processors</td><td align="right">296 KiB</td></tr>
<tr><td align="right">698</td><td><code>libksysguard-bin</code></td><td>4:6.6.4-0ubuntu1</td><td>library for system monitoring - utilities</td><td align="right">294 KiB</td></tr>
<tr><td align="right">699</td><td><code>plasma-dataengines-addons</code></td><td>4:6.6.4-0ubuntu1</td><td>additional data engines for Plasma</td><td align="right">294 KiB</td></tr>
<tr><td align="right">700</td><td><code>qml-module-org-kde-kquickcontrolsaddons</code></td><td>5.116.0-1ubuntu1</td><td>provides integration of QML and KDE Frameworks - kquickcontrolsaddons</td><td align="right">294 KiB</td></tr>
<tr><td align="right">701</td><td><code>libplasmaactivitiesstats1</code></td><td>6.6.4-0ubuntu1</td><td>usage data collected by the activities system</td><td align="right">293 KiB</td></tr>
<tr><td align="right">702</td><td><code>qml6-module-org-kde-kirigamiaddons-tableview</code></td><td>1.11.0-2ubuntu2</td><td>tableview module for QML</td><td align="right">292 KiB</td></tr>
<tr><td align="right">703</td><td><code>qml6-module-sso-onlineaccounts</code></td><td>0.7+git20231028.05e79eb-7</td><td>Expose the Online Accounts API to Qt6 QML applications</td><td align="right">291 KiB</td></tr>
<tr><td align="right">704</td><td><code>libappimage1.0abi1t64</code></td><td>1.0.4-5-7build1</td><td>Core library for appimage</td><td align="right">290 KiB</td></tr>
<tr><td align="right">705</td><td><code>libkf6configgui6</code></td><td>6.24.0-0ubuntu1</td><td>configuration settings framework for Qt</td><td align="right">288 KiB</td></tr>
<tr><td align="right">706</td><td><code>plasma-distro-release-notifier</code></td><td>20241226-0ubuntu8</td><td>Notifies KDE Plasma users of a new version of Ubuntu</td><td align="right">285 KiB</td></tr>
<tr><td align="right">707</td><td><code>libkf6authcore6</code></td><td>6.24.0-0ubuntu1</td><td>Abstraction to system policy and authentication features</td><td align="right">283 KiB</td></tr>
<tr><td align="right">708</td><td><code>libpugixml1v5</code></td><td>1.14-2build1</td><td>Light-weight C++ XML processing library</td><td align="right">283 KiB</td></tr>
<tr><td align="right">709</td><td><code>libsidplay2</code></td><td>2.1.1-16build1</td><td>SID (MOS 6581) emulation library</td><td align="right">281 KiB</td></tr>
<tr><td align="right">710</td><td><code>libass9</code></td><td>1:0.17.4-2</td><td>library for SSA/ASS subtitles rendering</td><td align="right">280 KiB</td></tr>
<tr><td align="right">711</td><td><code>qml-module-qtquick2</code></td><td>5.15.18+dfsg-2</td><td>Qt 5 Qt Quick 2 QML module</td><td align="right">280 KiB</td></tr>
<tr><td align="right">712</td><td><code>libqt6webchannel6</code></td><td>6.10.2-1</td><td>Qt 6 WebChannel library</td><td align="right">279 KiB</td></tr>
<tr><td align="right">713</td><td><code>libquickchartscontrols1</code></td><td>6.24.0-0ubuntu1</td><td>Quick Charts</td><td align="right">279 KiB</td></tr>
<tr><td align="right">714</td><td><code>libupnp17t64</code></td><td>1:1.14.25-1ubuntu1</td><td>Portable SDK for UPnP Devices (shared library)</td><td align="right">278 KiB</td></tr>
<tr><td align="right">715</td><td><code>ksshaskpass</code></td><td>4:6.6.4-0ubuntu1</td><td>interactively prompt users for a passphrase for ssh-add</td><td align="right">277 KiB</td></tr>
<tr><td align="right">716</td><td><code>libyyjson0</code></td><td>0.12.0+ds-1</td><td>high performance JSON library written in ANSI C</td><td align="right">274 KiB</td></tr>
<tr><td align="right">717</td><td><code>libkf5itemviews5</code></td><td>5.116.0-1ubuntu1</td><td>Qt library with additional widgets for ItemModels</td><td align="right">273 KiB</td></tr>
<tr><td align="right">718</td><td><code>libavif16</code></td><td>1.3.0-1ubuntu4</td><td>Library for handling .avif files</td><td align="right">269 KiB</td></tr>
<tr><td align="right">719</td><td><code>libplasmaactivities7</code></td><td>6.6.4-0ubuntu1</td><td>core components for the KDE&#x27;s Activities system - shared library</td><td align="right">267 KiB</td></tr>
<tr><td align="right">720</td><td><code>sonnet6-plugins</code></td><td>6.24.0-0ubuntu1</td><td>spell checking library for Qt, plugins</td><td align="right">267 KiB</td></tr>
<tr><td align="right">721</td><td><code>libkunifiedpush-data</code></td><td>25.12.3-0ubuntu1</td><td>UnifiedPush client library - data files</td><td align="right">266 KiB</td></tr>
<tr><td align="right">722</td><td><code>libpolkit-qt6-1-1</code></td><td>0.200.0-4ubuntu1</td><td>PolicyKit-qt6-1 library</td><td align="right">266 KiB</td></tr>
<tr><td align="right">723</td><td><code>pyqt6-dev-tools</code></td><td>6.10.2-2build5</td><td>development tools for PyQt6</td><td align="right">265 KiB</td></tr>
<tr><td align="right">724</td><td><code>libkf5codecs5</code></td><td>5.116.0-2</td><td>collection of methods to manipulate strings</td><td align="right">264 KiB</td></tr>
<tr><td align="right">725</td><td><code>libkpipewirerecord6</code></td><td>6.6.4-0ubuntu1</td><td>KDE&#x27;s Pipewire libraries - libkpipewirerecord6</td><td align="right">264 KiB</td></tr>
<tr><td align="right">726</td><td><code>libqt5quickcontrols2-5</code></td><td>5.15.18+dfsg-1</td><td>Qt 5 Quick Controls 2 library</td><td align="right">264 KiB</td></tr>
<tr><td align="right">727</td><td><code>gdal-plugins</code></td><td>3.12.2+dfsg-1build2</td><td>Geospatial Data Abstraction Library - Plugins</td><td align="right">263 KiB</td></tr>
<tr><td align="right">728</td><td><code>libkf6baloo6</code></td><td>6.24.0-0ubuntu1</td><td>framework for file search and metadata management - shared library</td><td align="right">263 KiB</td></tr>
<tr><td align="right">729</td><td><code>libkf6texteditor-bin</code></td><td>6.24.0-0ubuntu1</td><td>provide advanced plain text editing services (binaries)</td><td align="right">262 KiB</td></tr>
<tr><td align="right">730</td><td><code>libkfontinstui6</code></td><td>4:6.6.4-0ubuntu2</td><td>Plasma Workspace UI components for fonts installation</td><td align="right">259 KiB</td></tr>
<tr><td align="right">731</td><td><code>libkirigamiaddonsstatefulapp6</code></td><td>1.11.0-2ubuntu2</td><td>statefulapp library</td><td align="right">257 KiB</td></tr>
<tr><td align="right">732</td><td><code>libkf6dnssd6</code></td><td>6.24.0-0ubuntu1</td><td>Abstraction to system DNSSD features</td><td align="right">256 KiB</td></tr>
<tr><td align="right">733</td><td><code>libkf6idletime6</code></td><td>6.24.0-0ubuntu1</td><td>library to provide information about idle time</td><td align="right">256 KiB</td></tr>
<tr><td align="right">734</td><td><code>libopenal-data</code></td><td>1:1.25.1-2</td><td>Software implementation of the OpenAL audio API (data files)</td><td align="right">256 KiB</td></tr>
<tr><td align="right">735</td><td><code>libqt6positioning6-plugins</code></td><td>6.10.2-1</td><td>Qt 6 Positioning module - position plugins</td><td align="right">256 KiB</td></tr>
<tr><td align="right">736</td><td><code>isympy-common</code></td><td>1.14.0-2</td><td>Python shell for SymPy</td><td align="right">255 KiB</td></tr>
<tr><td align="right">737</td><td><code>libkf6iconthemes6</code></td><td>6.24.0-0ubuntu2</td><td>Support for icon themes</td><td align="right">255 KiB</td></tr>
<tr><td align="right">738</td><td><code>libswresample6</code></td><td>7:8.0.1-3ubuntu2</td><td>FFmpeg library for audio resampling, rematrixing etc. - runtime files</td><td align="right">255 KiB</td></tr>
<tr><td align="right">739</td><td><code>kwin-addons</code></td><td>4:6.6.4-0ubuntu1</td><td>additional desktop and window switchers for KWin</td><td align="right">254 KiB</td></tr>
<tr><td align="right">740</td><td><code>libgeotiff5</code></td><td>1.7.4-1build1</td><td>GeoTIFF (geografic enabled TIFF) library -- run-time files</td><td align="right">254 KiB</td></tr>
<tr><td align="right">741</td><td><code>systemd-coredump</code></td><td>259.5-0ubuntu3</td><td>tools for storing and retrieving coredumps</td><td align="right">254 KiB</td></tr>
<tr><td align="right">742</td><td><code>libdc1394-25</code></td><td>2.2.6-6</td><td>high level programming interface for IEEE 1394 digital cameras</td><td align="right">253 KiB</td></tr>
<tr><td align="right">743</td><td><code>qt6-image-formats-plugins</code></td><td>6.10.2-1</td><td>Qt 6 Image Formats plugins</td><td align="right">253 KiB</td></tr>
<tr><td align="right">744</td><td><code>libkf5kirigami2-5</code></td><td>5.116.0-1ubuntu4</td><td>set of QtQuick components targeted for mobile use</td><td align="right">252 KiB</td></tr>
<tr><td align="right">745</td><td><code>libkf6codecs6</code></td><td>6.24.0-0ubuntu1</td><td>collection of methods to manipulate strings</td><td align="right">249 KiB</td></tr>
<tr><td align="right">746</td><td><code>libkf6globalaccel6</code></td><td>6.24.0-0ubuntu1</td><td>Configurable global shortcut support</td><td align="right">249 KiB</td></tr>
<tr><td align="right">747</td><td><code>libpolkit-qt5-1-1</code></td><td>0.200.0-4ubuntu1</td><td>PolicyKit-qt5-1 library</td><td align="right">248 KiB</td></tr>
<tr><td align="right">748</td><td><code>libqt6sql6-sqlite</code></td><td>6.10.2+dfsg-7</td><td>Qt 6 SQLite 3 database driver</td><td align="right">248 KiB</td></tr>
<tr><td align="right">749</td><td><code>libkf6dbusaddons-data</code></td><td>6.24.0-0ubuntu1</td><td>class library for qtdbus</td><td align="right">247 KiB</td></tr>
<tr><td align="right">750</td><td><code>pinentry-qt</code></td><td>1.3.2-3ubuntu1</td><td>Qt-based PIN or pass-phrase entry dialog for GnuPG</td><td align="right">247 KiB</td></tr>
<tr><td align="right">751</td><td><code>libkf6i18nqml6</code></td><td>6.24.0-0ubuntu1</td><td>Advanced internationalization framework (locale data)</td><td align="right">246 KiB</td></tr>
<tr><td align="right">752</td><td><code>libkmlengine1t64</code></td><td>1.3.0-13</td><td>Library to manipulate KML 2.2 OGC standard files - libkmlengine</td><td align="right">245 KiB</td></tr>
<tr><td align="right">753</td><td><code>qml6-module-qtcore</code></td><td>6.10.2+dfsg-3</td><td>Qt 6 QML Core QML module</td><td align="right">244 KiB</td></tr>
<tr><td align="right">754</td><td><code>libavdevice62</code></td><td>7:8.0.1-3ubuntu2</td><td>FFmpeg library for handling input and output devices - runtime files</td><td align="right">243 KiB</td></tr>
<tr><td align="right">755</td><td><code>libqt6websockets6</code></td><td>6.10.2-1</td><td>Qt 6 WebSockets library</td><td align="right">243 KiB</td></tr>
<tr><td align="right">756</td><td><code>plymouth-theme-breeze</code></td><td>6.6.4-0ubuntu1</td><td>Breeze theme for Plymouth</td><td align="right">243 KiB</td></tr>
<tr><td align="right">757</td><td><code>libkf6balooengine6</code></td><td>6.24.0-0ubuntu1</td><td>framework for file search and metadata management - internal library</td><td align="right">241 KiB</td></tr>
<tr><td align="right">758</td><td><code>libportaudio2</code></td><td>19.7.0+git20260206.e1b70d33-0ubuntu1</td><td>Portable audio I/O - shared library</td><td align="right">241 KiB</td></tr>
<tr><td align="right">759</td><td><code>par2</code></td><td>1.1.1-1</td><td>PAR 2.0 compatible file verification and repair tool</td><td align="right">241 KiB</td></tr>
<tr><td align="right">760</td><td><code>libqt5sql5-sqlite</code></td><td>5.15.18+dfsg-1ubuntu1</td><td>Qt 5 SQLite 3 database driver</td><td align="right">239 KiB</td></tr>
<tr><td align="right">761</td><td><code>libkirigami6</code></td><td>6.24.0-0ubuntu1</td><td>set of QtQuick components targeted for mobile use</td><td align="right">237 KiB</td></tr>
<tr><td align="right">762</td><td><code>libkirigamidialogs6</code></td><td>6.24.0-0ubuntu1</td><td>set of QtQuick components targeted for mobile use</td><td align="right">237 KiB</td></tr>
<tr><td align="right">763</td><td><code>signon-plugin-oauth2</code></td><td>0.25+git20231015.fab69886-3build1</td><td>OAuth 1.0/2.0 plugin for the Accounts SignOn daemon</td><td align="right">237 KiB</td></tr>
<tr><td align="right">764</td><td><code>libqt6qmlmeta6</code></td><td>6.10.2+dfsg-3</td><td>Qt 6 QML Meta library</td><td align="right">235 KiB</td></tr>
<tr><td align="right">765</td><td><code>libkf5authcore5</code></td><td>5.116.0-1ubuntu1</td><td>Abstraction to system policy and authentication features</td><td align="right">232 KiB</td></tr>
<tr><td align="right">766</td><td><code>sonnet-plugins</code></td><td>5.116.0-1ubuntu2</td><td>spell checking library for Qt, plugins</td><td align="right">232 KiB</td></tr>
<tr><td align="right">767</td><td><code>kio-fuse</code></td><td>5.1.1-1</td><td>FUSE Interface for KIO</td><td align="right">231 KiB</td></tr>
<tr><td align="right">768</td><td><code>libkaccounts6-2</code></td><td>4:25.12.3-0ubuntu1</td><td>integration library for Accounts-SSO and SignOn-SSO</td><td align="right">231 KiB</td></tr>
<tr><td align="right">769</td><td><code>libkf6filemetadata3</code></td><td>6.24.0-0ubuntu1</td><td>library for extracting file metadata</td><td align="right">231 KiB</td></tr>
<tr><td align="right">770</td><td><code>libkf6threadweaver6</code></td><td>6.24.0-0ubuntu1</td><td>ThreadWeaver library to help multithreaded programming in Qt</td><td align="right">228 KiB</td></tr>
<tr><td align="right">771</td><td><code>libksysguardsystemstats2</code></td><td>4:6.6.4-0ubuntu1</td><td>library for system monitoring - ksysguardsystemstats shared library</td><td align="right">226 KiB</td></tr>
<tr><td align="right">772</td><td><code>qml-module-qtquick-layouts</code></td><td>5.15.18+dfsg-2</td><td>Qt 5 Quick Layouts QML module</td><td align="right">226 KiB</td></tr>
<tr><td align="right">773</td><td><code>libopenal-dev</code></td><td>1:1.25.1-2</td><td>Software implementation of the OpenAL audio API (development files)</td><td align="right">225 KiB</td></tr>
<tr><td align="right">774</td><td><code>libqt6keychain1</code></td><td>0.15.0-2</td><td>Qt API to store passwords (Qt 6 version)</td><td align="right">224 KiB</td></tr>
<tr><td align="right">775</td><td><code>libkf5dbusaddons-data</code></td><td>5.116.0-2</td><td>class library for qtdbus</td><td align="right">223 KiB</td></tr>
<tr><td align="right">776</td><td><code>libkf5jobwidgets5</code></td><td>5.116.0-1ubuntu1</td><td>Widgets for tracking KJob instances</td><td align="right">223 KiB</td></tr>
<tr><td align="right">777</td><td><code>qml6-module-org-kde-plasma-plasma5support</code></td><td>4:6.6.4-0ubuntu1</td><td>support components for porting from KF5/Qt5 to KF6/Qt6 - QML module</td><td align="right">223 KiB</td></tr>
<tr><td align="right">778</td><td><code>frameworkintegration6</code></td><td>6.24.0-0ubuntu1</td><td>integration of Qt application with KDE workspaces</td><td align="right">220 KiB</td></tr>
<tr><td align="right">779</td><td><code>libkf6sonnetui6</code></td><td>6.24.0-0ubuntu1</td><td>spell checking library for Qt, ui lib</td><td align="right">219 KiB</td></tr>
<tr><td align="right">780</td><td><code>libkirigamidelegates6</code></td><td>6.24.0-0ubuntu1</td><td>set of QtQuick components targeted for mobile use</td><td align="right">218 KiB</td></tr>
<tr><td align="right">781</td><td><code>libresid-builder0c2a</code></td><td>2.1.1-16build1</td><td>SID chip emulation class based on resid</td><td align="right">218 KiB</td></tr>
<tr><td align="right">782</td><td><code>libsox-fmt-base</code></td><td>14.7.0.9+ds1-1</td><td>Minimal set of SoX format libraries</td><td align="right">218 KiB</td></tr>
<tr><td align="right">783</td><td><code>libsoxr0</code></td><td>0.1.3-4.1</td><td>High quality 1D sample-rate conversion library</td><td align="right">217 KiB</td></tr>
<tr><td align="right">784</td><td><code>qml6-module-qtqml-xmllistmodel</code></td><td>6.10.2+dfsg-3</td><td>Qt 6 QtQml XmlListModel QML module</td><td align="right">215 KiB</td></tr>
<tr><td align="right">785</td><td><code>zsync</code></td><td>0.6.2-9</td><td>client-side implementation of the rsync algorithm</td><td align="right">215 KiB</td></tr>
<tr><td align="right">786</td><td><code>libobjc4</code></td><td>16-20260322-1ubuntu1</td><td>Runtime library for GNU Objective-C applications</td><td align="right">214 KiB</td></tr>
<tr><td align="right">787</td><td><code>libqt6openglwidgets6</code></td><td>6.10.2+dfsg-7</td><td>Qt 6 OpenGL widgets module</td><td align="right">214 KiB</td></tr>
<tr><td align="right">788</td><td><code>qml6-module-qt-labs-folderlistmodel</code></td><td>6.10.2+dfsg-3</td><td>Qt 6 FolderListModel QML module</td><td align="right">212 KiB</td></tr>
<tr><td align="right">789</td><td><code>libdebconf-kde1</code></td><td>1.2.0-2ubuntu1</td><td>Debconf KDE GUI library</td><td align="right">210 KiB</td></tr>
<tr><td align="right">790</td><td><code>libkf5sonnetui5</code></td><td>5.116.0-1ubuntu2</td><td>spell checking library for Qt, ui lib</td><td align="right">207 KiB</td></tr>
<tr><td align="right">791</td><td><code>libkf6su-data</code></td><td>6.24.0-0ubuntu1</td><td>translation files for kdesu</td><td align="right">204 KiB</td></tr>
<tr><td align="right">792</td><td><code>gnustep-common</code></td><td>2.9.3-7</td><td>Common files for the core GNUstep environment</td><td align="right">203 KiB</td></tr>
<tr><td align="right">793</td><td><code>libkf6kcmutils6</code></td><td>6.24.0-0ubuntu1</td><td>Extra APIs to write KConfig modules</td><td align="right">203 KiB</td></tr>
<tr><td align="right">794</td><td><code>wvdial</code></td><td>1.61-8build1</td><td>intelligent Point-to-Point Protocol dialer</td><td align="right">203 KiB</td></tr>
<tr><td align="right">795</td><td><code>libkpipewire6</code></td><td>6.6.4-0ubuntu1</td><td>KDE&#x27;s Pipewire libraries - libkpipewire6 library</td><td align="right">202 KiB</td></tr>
<tr><td align="right">796</td><td><code>joystick</code></td><td>1:1.8.1-2build2</td><td>set of testing and calibration tools for joysticks</td><td align="right">201 KiB</td></tr>
<tr><td align="right">797</td><td><code>libkf6package6</code></td><td>6.24.0-0ubuntu1</td><td>non-binary asset management framework</td><td align="right">201 KiB</td></tr>
<tr><td align="right">798</td><td><code>sox</code></td><td>14.7.0.9+ds1-1</td><td>Swiss army knife of sound processing</td><td align="right">201 KiB</td></tr>
<tr><td align="right">799</td><td><code>libneon27t64-gnutls</code></td><td>0.36.0-1</td><td>HTTP and WebDAV client library (GnuTLS enabled)</td><td align="right">200 KiB</td></tr>
<tr><td align="right">800</td><td><code>libkf6people-data</code></td><td>6.24.0-0ubuntu1</td><td>data files for kpeople</td><td align="right">198 KiB</td></tr>
<tr><td align="right">801</td><td><code>libfmt10</code></td><td>10.1.1+ds1-4build1</td><td>fast type-safe C++ formatting library -- library</td><td align="right">193 KiB</td></tr>
<tr><td align="right">802</td><td><code>qml6-module-org-kde-ksvg</code></td><td>6.24.0-0ubuntu1</td><td>library for rendering SVG-based themes - QML module</td><td align="right">193 KiB</td></tr>
<tr><td align="right">803</td><td><code>libhdf5-hl-310</code></td><td>1.14.6+repack-2</td><td>HDF5 High Level runtime files - serial version</td><td align="right">191 KiB</td></tr>
<tr><td align="right">804</td><td><code>libkf5configgui5</code></td><td>5.116.0-2</td><td>configuration settings framework for Qt</td><td align="right">191 KiB</td></tr>
<tr><td align="right">805</td><td><code>librist4</code></td><td>0.2.11+dfsg-1build1</td><td>Reliable Internet Stream Transport -- shared library</td><td align="right">191 KiB</td></tr>
<tr><td align="right">806</td><td><code>qml6-module-org-kde-kirigamiaddons-treeview</code></td><td>1.11.0-2ubuntu2</td><td>treeview module for QML</td><td align="right">191 KiB</td></tr>
<tr><td align="right">807</td><td><code>kwayland-integration</code></td><td>4:6.6.4-0ubuntu1</td><td>kwayland runtime integration plugins</td><td align="right">190 KiB</td></tr>
<tr><td align="right">808</td><td><code>libqt6quickwidgets6</code></td><td>6.10.2+dfsg-3</td><td>Qt 6 Quick Widgets library</td><td align="right">190 KiB</td></tr>
<tr><td align="right">809</td><td><code>libdbusmenu-qt5-2</code></td><td>0.9.3+16.04.20160218-5</td><td>Qt implementation of the DBusMenu protocol</td><td align="right">189 KiB</td></tr>
<tr><td align="right">810</td><td><code>libphysfs1</code></td><td>3.2.0-1</td><td>filesystem abstraction library for game programmers</td><td align="right">189 KiB</td></tr>
<tr><td align="right">811</td><td><code>libqt5concurrent5t64</code></td><td>5.15.18+dfsg-1ubuntu1</td><td>Qt 5 concurrent module</td><td align="right">189 KiB</td></tr>
<tr><td align="right">812</td><td><code>libkf5globalaccelprivate5</code></td><td>5.116.0-1ubuntu1</td><td>Configurable global shortcut support - private runtime library</td><td align="right">188 KiB</td></tr>
<tr><td align="right">813</td><td><code>plasma-session-wayland</code></td><td>4:6.6.4-0ubuntu2</td><td>Plasma Workspace for KF6</td><td align="right">188 KiB</td></tr>
<tr><td align="right">814</td><td><code>qml6-module-org-kde-activities</code></td><td>6.6.4-0ubuntu1</td><td>core components for the KDE&#x27;s Activities system - QML module</td><td align="right">188 KiB</td></tr>
<tr><td align="right">815</td><td><code>qml6-module-org-kde-baloo</code></td><td>6.24.0-0ubuntu1</td><td>framework for file search and metadata management - QML module</td><td align="right">188 KiB</td></tr>
<tr><td align="right">816</td><td><code>debconf-kde-data</code></td><td>1.2.0-2ubuntu1</td><td>Debconf KDE data files</td><td align="right">187 KiB</td></tr>
<tr><td align="right">817</td><td><code>kdegraphics-thumbnailers</code></td><td>4:25.12.3-0ubuntu1</td><td>graphics file format thumbnailers for KDE SC</td><td align="right">187 KiB</td></tr>
<tr><td align="right">818</td><td><code>liballegro-audio5.2t64</code></td><td>2:5.2.11.3+dfsg-1</td><td>audio addon for the Allegro 5 library</td><td align="right">187 KiB</td></tr>
<tr><td align="right">819</td><td><code>libopencore-amrnb0</code></td><td>0.1.6-1build2</td><td>Adaptive Multi Rate speech codec - shared library</td><td align="right">187 KiB</td></tr>
<tr><td align="right">820</td><td><code>libkf6wallet6</code></td><td>6.24.0-0ubuntu1</td><td>safe desktop-wide storage for passwords - shared library</td><td align="right">186 KiB</td></tr>
<tr><td align="right">821</td><td><code>kded6</code></td><td>6.24.0-0ubuntu1</td><td>Extensible daemon for providing session services</td><td align="right">185 KiB</td></tr>
<tr><td align="right">822</td><td><code>python3-ufolib2</code></td><td>0.18.1+dfsg1-1</td><td>Unified Font Object (UFO) fonts library</td><td align="right">185 KiB</td></tr>
<tr><td align="right">823</td><td><code>python3-pyqt5.sip</code></td><td>12.17.2-1build1</td><td>runtime module for Python extensions using SIP</td><td align="right">184 KiB</td></tr>
<tr><td align="right">824</td><td><code>libqcoro6core0t64</code></td><td>0.13.0-1ubuntu2</td><td>C++20 coroutines for Qt6 - Core</td><td align="right">182 KiB</td></tr>
<tr><td align="right">825</td><td><code>kded5</code></td><td>5.116.0-1ubuntu1</td><td>Extensible daemon for providing session services</td><td align="right">181 KiB</td></tr>
<tr><td align="right">826</td><td><code>libsoundtouch1</code></td><td>2.4.0+ds-1build1</td><td>Sound stretching library</td><td align="right">181 KiB</td></tr>
<tr><td align="right">827</td><td><code>qml6-module-org-kde-kquickcontrols</code></td><td>6.24.0-0ubuntu1</td><td>provides integration of QML and KDE frameworks - kquickcontrols</td><td align="right">178 KiB</td></tr>
<tr><td align="right">828</td><td><code>clinfo</code></td><td>3.0.25.02.14-1build1</td><td>Query OpenCL system information</td><td align="right">177 KiB</td></tr>
<tr><td align="right">829</td><td><code>libkf6qqc2desktopstyle-data</code></td><td>6.24.0-0ubuntu1</td><td>Qt Quick Controls 2: Desktop Style - data files</td><td align="right">177 KiB</td></tr>
<tr><td align="right">830</td><td><code>libqt6concurrent6</code></td><td>6.10.2+dfsg-7</td><td>Qt 6 concurrent module</td><td align="right">177 KiB</td></tr>
<tr><td align="right">831</td><td><code>qml6-module-org-kde-kquickcontrolsaddons</code></td><td>6.24.0-0ubuntu1</td><td>provides integration of QML and KDE Frameworks - kquickcontrolsaddons</td><td align="right">177 KiB</td></tr>
<tr><td align="right">832</td><td><code>kpackagetool5</code></td><td>5.116.0-1ubuntu1</td><td>command line kpackage tool</td><td align="right">176 KiB</td></tr>
<tr><td align="right">833</td><td><code>phonon-backend-vlc-common</code></td><td>0.12.0-3build5</td><td>Phonon VLC backends - shared files</td><td align="right">176 KiB</td></tr>
<tr><td align="right">834</td><td><code>libtbbmalloc2</code></td><td>2022.3.0-2</td><td>parallelism helper library for C++ - runtime files</td><td align="right">175 KiB</td></tr>
<tr><td align="right">835</td><td><code>libxdgutilsdesktopentry1.0.1</code></td><td>1.0.1-3.2</td><td>Implementation Desktop Entry in Free Desktop Standards for C++</td><td align="right">175 KiB</td></tr>
<tr><td align="right">836</td><td><code>qml6-module-org-kde-kquickimageeditor</code></td><td>0.6.0-2build1</td><td>Image editing components</td><td align="right">175 KiB</td></tr>
<tr><td align="right">837</td><td><code>graphviz</code></td><td>14.1.2-1ubuntu1</td><td>rich set of graph drawing tools</td><td align="right">174 KiB</td></tr>
<tr><td align="right">838</td><td><code>libfreecell-solver0</code></td><td>5.0.0-4</td><td>Library for solving Freecell games</td><td align="right">174 KiB</td></tr>
<tr><td align="right">839</td><td><code>libplasmaweatherdata6</code></td><td>4:6.6.4-0ubuntu1</td><td>weather data manipulation library for Plasma widgets</td><td align="right">174 KiB</td></tr>
<tr><td align="right">840</td><td><code>libaccounts-qt6-1</code></td><td>1.17-4ubuntu2</td><td>Accounts database access Qt version - Qt6 shared library</td><td align="right">173 KiB</td></tr>
<tr><td align="right">841</td><td><code>libmpeg2-4</code></td><td>0.5.1-10</td><td>MPEG1 and MPEG2 video decoder library</td><td align="right">173 KiB</td></tr>
<tr><td align="right">842</td><td><code>python3-kiwisolver</code></td><td>1.4.10~rc0-1build1</td><td>fast implementation of the Cassowary constraint solver - Python 3.X</td><td align="right">173 KiB</td></tr>
<tr><td align="right">843</td><td><code>libdvdread8t64</code></td><td>7.0.1-1</td><td>library for reading DVDs</td><td align="right">172 KiB</td></tr>
<tr><td align="right">844</td><td><code>libkf5package5</code></td><td>5.116.0-1ubuntu1</td><td>non-binary asset management framework</td><td align="right">171 KiB</td></tr>
<tr><td align="right">845</td><td><code>qml6-module-org-kde-kitemmodels</code></td><td>6.24.0-0ubuntu1</td><td>additional item/view models for Qt Itemview (QML Bindings)</td><td align="right">171 KiB</td></tr>
<tr><td align="right">846</td><td><code>qml6-module-org-kde-pipewire</code></td><td>6.6.4-0ubuntu1</td><td>KDE&#x27;s Pipewire libraries - QML module</td><td align="right">169 KiB</td></tr>
<tr><td align="right">847</td><td><code>libkpipewire-data</code></td><td>6.6.4-0ubuntu1</td><td>KDE&#x27;s Pipewire libraries - data files</td><td align="right">168 KiB</td></tr>
<tr><td align="right">848</td><td><code>libvlc-bin</code></td><td>3.0.23-1</td><td>tools for VLC&#x27;s base library</td><td align="right">168 KiB</td></tr>
<tr><td align="right">849</td><td><code>libkf6purpose6</code></td><td>6.24.0-0ubuntu1</td><td>library for abstractions to get the developer&#x27;s purposes fulfilled</td><td align="right">167 KiB</td></tr>
<tr><td align="right">850</td><td><code>python3-pyqt6.sip</code></td><td>13.11.0-1build1</td><td>runtime module for Python extensions using SIP</td><td align="right">167 KiB</td></tr>
<tr><td align="right">851</td><td><code>qml6-module-qtlocation</code></td><td>6.10.2-1</td><td>Qt 6 Location QML module</td><td align="right">167 KiB</td></tr>
<tr><td align="right">852</td><td><code>libkdsoapwsdiscoveryclient0</code></td><td>0.4.0-2ubuntu2</td><td>Qt-based WS-Discovery client library based on KDSoap</td><td align="right">166 KiB</td></tr>
<tr><td align="right">853</td><td><code>libkf6dbusaddons6</code></td><td>6.24.0-0ubuntu1</td><td>class library for qtdbus</td><td align="right">166 KiB</td></tr>
<tr><td align="right">854</td><td><code>libkscreen-data</code></td><td>4:6.6.4-0ubuntu1</td><td>library for screen management - data files</td><td align="right">165 KiB</td></tr>
<tr><td align="right">855</td><td><code>libkf5config-bin</code></td><td>5.116.0-2</td><td>configuration settings framework for Qt</td><td align="right">163 KiB</td></tr>
<tr><td align="right">856</td><td><code>libkf5globalaccel5</code></td><td>5.116.0-1ubuntu1</td><td>Configurable global shortcut support.</td><td align="right">162 KiB</td></tr>
<tr><td align="right">857</td><td><code>libzvbi-common</code></td><td>0.2.44-1ubuntu2</td><td>Vertical Blanking Interval decoder (VBI) - common files</td><td align="right">162 KiB</td></tr>
<tr><td align="right">858</td><td><code>software-properties-qt</code></td><td>0.120</td><td>manage the repositories that you install software from (Qt)</td><td align="right">162 KiB</td></tr>
<tr><td align="right">859</td><td><code>libdvbpsi10</code></td><td>1.3.3-1build2</td><td>library for MPEG TS and DVB PSI tables decoding and generating</td><td align="right">160 KiB</td></tr>
<tr><td align="right">860</td><td><code>libmad0</code></td><td>0.16.4-2ubuntu1</td><td>MPEG audio decoder library</td><td align="right">160 KiB</td></tr>
<tr><td align="right">861</td><td><code>libebml5</code></td><td>1.4.5-2</td><td>access library for the EBML format (shared library)</td><td align="right">158 KiB</td></tr>
<tr><td align="right">862</td><td><code>libkf6walletbackend6</code></td><td>6.24.0-0ubuntu1</td><td>safe desktop-wide storage for passwords - backend library</td><td align="right">158 KiB</td></tr>
<tr><td align="right">863</td><td><code>libzip5</code></td><td>1.11.4-2</td><td>library for reading, creating, and modifying zip archives (runtime)</td><td align="right">158 KiB</td></tr>
<tr><td align="right">864</td><td><code>usb-creator-kde</code></td><td>0.4.1build1</td><td>create a startup disk using a CD or disc image (for KDE)</td><td align="right">158 KiB</td></tr>
<tr><td align="right">865</td><td><code>libfreexl1</code></td><td>2.0.0-1build3</td><td>library for direct reading of Microsoft Excel spreadsheets</td><td align="right">157 KiB</td></tr>
<tr><td align="right">866</td><td><code>libqt6serialport6</code></td><td>6.10.2-1</td><td>Qt 6 serial port support library</td><td align="right">157 KiB</td></tr>
<tr><td align="right">867</td><td><code>libqcoro6network0t64</code></td><td>0.13.0-1ubuntu2</td><td>C++20 coroutines for Qt6 - Network</td><td align="right">156 KiB</td></tr>
<tr><td align="right">868</td><td><code>libaacs0</code></td><td>0.11.1-4build1</td><td>free-and-libre implementation of AACS</td><td align="right">155 KiB</td></tr>
<tr><td align="right">869</td><td><code>libkf5quickaddons5</code></td><td>5.116.0-1ubuntu1</td><td>provides integration of QML and KDE frameworks -- quickaddons</td><td align="right">155 KiB</td></tr>
<tr><td align="right">870</td><td><code>libkf6bookmarks6</code></td><td>6.24.0-0ubuntu1</td><td>Qt library with support for bookmarks and the XBEL format</td><td align="right">155 KiB</td></tr>
<tr><td align="right">871</td><td><code>libkmlbase1t64</code></td><td>1.3.0-13</td><td>Library to manipulate KML 2.2 OGC standard files - libkmlbase</td><td align="right">155 KiB</td></tr>
<tr><td align="right">872</td><td><code>liboxygenstyle6-6</code></td><td>4:6.6.4-0ubuntu1</td><td>style library for the Oxygen desktop theme (Qt 6)</td><td align="right">155 KiB</td></tr>
<tr><td align="right">873</td><td><code>libqt6texttospeech6</code></td><td>6.10.2-1</td><td>Qt 6 Speech library</td><td align="right">154 KiB</td></tr>
<tr><td align="right">874</td><td><code>libtommath1</code></td><td>1.3.0-1build1</td><td>multiple-precision integer library [runtime]</td><td align="right">153 KiB</td></tr>
<tr><td align="right">875</td><td><code>kpackagetool6</code></td><td>6.24.0-0ubuntu1</td><td>command line kpackage tool</td><td align="right">152 KiB</td></tr>
<tr><td align="right">876</td><td><code>libkf5wallet5</code></td><td>5.116.0-1ubuntu1</td><td>Secure and unified container for user passwords.</td><td align="right">152 KiB</td></tr>
<tr><td align="right">877</td><td><code>qml6-module-org-kde-draganddrop</code></td><td>6.24.0-0ubuntu1</td><td>provides integration of QML and KDE frameworks - draganddrop</td><td align="right">151 KiB</td></tr>
<tr><td align="right">878</td><td><code>qml6-module-org-kde-people</code></td><td>6.24.0-0ubuntu1</td><td>framework providing unified access to contacts aggregated by person</td><td align="right">151 KiB</td></tr>
<tr><td align="right">879</td><td><code>qml6-module-qtqml-models</code></td><td>6.10.2+dfsg-3</td><td>Qt 6 QtQml Models QML module</td><td align="right">151 KiB</td></tr>
<tr><td align="right">880</td><td><code>libunibreak6</code></td><td>6.1-3build1</td><td>line breaking library for Unicode (shared library)</td><td align="right">150 KiB</td></tr>
<tr><td align="right">881</td><td><code>libkf6colorscheme-data</code></td><td>6.24.0-0ubuntu1</td><td>Classes to read and interact with KColorScheme</td><td align="right">148 KiB</td></tr>
<tr><td align="right">882</td><td><code>libaccounts-glib0</code></td><td>1.27-3</td><td>Accounts database access - shared library</td><td align="right">144 KiB</td></tr>
<tr><td align="right">883</td><td><code>libcgraph8</code></td><td>14.1.2-1ubuntu1</td><td>rich set of graph drawing tools - cgraph library</td><td align="right">144 KiB</td></tr>
<tr><td align="right">884</td><td><code>libchdr0</code></td><td>0.0~git20250608.8bba774+dfsg-2</td><td>standalone library for reading MAME CHDv1-v5 formats</td><td align="right">144 KiB</td></tr>
<tr><td align="right">885</td><td><code>libkf6kcmutilscore6</code></td><td>6.24.0-0ubuntu1</td><td>Extra APIs to write KConfig modules</td><td align="right">143 KiB</td></tr>
<tr><td align="right">886</td><td><code>libsfml-network3.0</code></td><td>3.0.2+dfsg-2</td><td>Simple and Fast Multimedia Library - Network part</td><td align="right">143 KiB</td></tr>
<tr><td align="right">887</td><td><code>qml6-module-org-kde-quickcharts</code></td><td>6.24.0-0ubuntu1</td><td>Quick Charts - QML module</td><td align="right">143 KiB</td></tr>
<tr><td align="right">888</td><td><code>qml6-module-org-kde-userfeedback</code></td><td>6.24.0-0ubuntu1</td><td>user feedback for applications - QML module</td><td align="right">143 KiB</td></tr>
<tr><td align="right">889</td><td><code>libkf5dbusaddons5</code></td><td>5.116.0-2</td><td>class library for qtdbus</td><td align="right">142 KiB</td></tr>
<tr><td align="right">890</td><td><code>qml6-module-org-kde-syntaxhighlighting</code></td><td>6.24.0-0ubuntu1</td><td>Syntax highlighting Engine (QML module)</td><td align="right">142 KiB</td></tr>
<tr><td align="right">891</td><td><code>libkf6bookmarkswidgets6</code></td><td>6.24.0-0ubuntu1</td><td>Qt library with support for bookmarks and the XBEL format (widgets)</td><td align="right">141 KiB</td></tr>
<tr><td align="right">892</td><td><code>libvorbisidec1</code></td><td>1.2.1+git20180316-8build1</td><td>Integer-only Ogg Vorbis decoder, AKA &quot;tremor&quot;</td><td align="right">140 KiB</td></tr>
<tr><td align="right">893</td><td><code>libb2-1</code></td><td>0.98.1-1.1build2</td><td>BLAKE2 family of hash functions</td><td align="right">139 KiB</td></tr>
<tr><td align="right">894</td><td><code>libkunifiedpush1</code></td><td>25.12.3-0ubuntu1</td><td>UnifiedPush client library</td><td align="right">139 KiB</td></tr>
<tr><td align="right">895</td><td><code>plasma-discover-backend-fwupd</code></td><td>6.6.4-0ubuntu1</td><td>Discover software management suite - fwupd backend</td><td align="right">139 KiB</td></tr>
<tr><td align="right">896</td><td><code>qml6-module-qt-labs-animation</code></td><td>6.10.2+dfsg-3</td><td>Qt 6 Animation QML module</td><td align="right">139 KiB</td></tr>
<tr><td align="right">897</td><td><code>python3-lz4</code></td><td>4.4.5+dfsg-1build1</td><td>Python interface to the lz4 compression library (Python 3)</td><td align="right">137 KiB</td></tr>
<tr><td align="right">898</td><td><code>qml6-module-org-kde-coreaddons</code></td><td>6.24.0-0ubuntu1</td><td>KDE Frameworks 6 addons to QtCore (QML module)</td><td align="right">137 KiB</td></tr>
<tr><td align="right">899</td><td><code>libkf6iconwidgets6</code></td><td>6.24.0-0ubuntu2</td><td>Support for icon widgets</td><td align="right">136 KiB</td></tr>
<tr><td align="right">900</td><td><code>libkf6statusnotifieritem-data</code></td><td>6.24.0-0ubuntu1</td><td>Implementation of Status Notifier Items - translations</td><td align="right">136 KiB</td></tr>
<tr><td align="right">901</td><td><code>liblilv-0-0</code></td><td>0.26.2-1</td><td>library for simple use of LV2 plugins</td><td align="right">136 KiB</td></tr>
<tr><td align="right">902</td><td><code>qml-module-qtquick-window2</code></td><td>5.15.18+dfsg-2</td><td>Qt 5 window 2 QML module</td><td align="right">136 KiB</td></tr>
<tr><td align="right">903</td><td><code>libkf6config-bin</code></td><td>6.24.0-0ubuntu1</td><td>configuration settings framework for Qt</td><td align="right">135 KiB</td></tr>
<tr><td align="right">904</td><td><code>qml6-module-qtwebview</code></td><td>6.10.2-1</td><td>Qt 6 WebView QML module</td><td align="right">135 KiB</td></tr>
<tr><td align="right">905</td><td><code>libqt6qmlworkerscript6</code></td><td>6.10.2+dfsg-3</td><td>Qt 6 QML WorkerScript library</td><td align="right">133 KiB</td></tr>
<tr><td align="right">906</td><td><code>qml6-module-org-kde-prison</code></td><td>6.24.0-0ubuntu1</td><td>barcode API for Qt</td><td align="right">133 KiB</td></tr>
<tr><td align="right">907</td><td><code>libdmtx0t64</code></td><td>0.7.8-1</td><td>Data Matrix barcodes (runtime library)</td><td align="right">128 KiB</td></tr>
<tr><td align="right">908</td><td><code>libqt5qmlworkerscript5</code></td><td>5.15.18+dfsg-2</td><td>Qt 5 QML Worker Script library</td><td align="right">128 KiB</td></tr>
<tr><td align="right">909</td><td><code>libsquashfuse0</code></td><td>0.5.2-0.3</td><td>FUSE filesystem to mount squashfs archives library</td><td align="right">128 KiB</td></tr>
<tr><td align="right">910</td><td><code>lame</code></td><td>3.101~svn6525+dfsg-2</td><td>MP3 encoding library (frontend)</td><td align="right">127 KiB</td></tr>
<tr><td align="right">911</td><td><code>python3-colorama</code></td><td>0.4.6-4build1</td><td>Cross-platform colored terminal text in Python - Python 3.x</td><td align="right">127 KiB</td></tr>
<tr><td align="right">912</td><td><code>knighttime</code></td><td>6.6.4-0ubuntu1</td><td>helpers for scheduling the dark-light cycle for the Plasma desktop</td><td align="right">126 KiB</td></tr>
<tr><td align="right">913</td><td><code>libbdplus0</code></td><td>0.2.0-4build1</td><td>implementation of BD+ for reading Blu-ray Discs</td><td align="right">126 KiB</td></tr>
<tr><td align="right">914</td><td><code>libkf6prison6</code></td><td>6.24.0-0ubuntu1</td><td>barcode API for Qt</td><td align="right">126 KiB</td></tr>
<tr><td align="right">915</td><td><code>libkquickcontrolsprivate0</code></td><td>6.24.0-0ubuntu1</td><td>provides integration of QML and KDE frameworks -- calendarevents</td><td align="right">126 KiB</td></tr>
<tr><td align="right">916</td><td><code>libsndio-dev</code></td><td>1.10.0-0.2</td><td>Small audio and MIDI framework from OpenBSD, development files</td><td align="right">126 KiB</td></tr>
<tr><td align="right">917</td><td><code>libminizip-ng4t64</code></td><td>4.0.10+dfsg-2</td><td>minizip-ng library (binaries)</td><td align="right">124 KiB</td></tr>
<tr><td align="right">918</td><td><code>live-boot</code></td><td>1:20250815</td><td>Live System Boot Components</td><td align="right">124 KiB</td></tr>
<tr><td align="right">919</td><td><code>libkf6colorscheme6</code></td><td>6.24.0-0ubuntu1</td><td>Classes to read and interact with KColorScheme</td><td align="right">122 KiB</td></tr>
<tr><td align="right">920</td><td><code>libopusfile0</code></td><td>0.12-4build4</td><td>High-level API for basic manipulation of Ogg Opus audio streams</td><td align="right">122 KiB</td></tr>
<tr><td align="right">921</td><td><code>qml6-module-qtpositioning</code></td><td>6.10.2-1</td><td>Qt 6 Positioning QML module</td><td align="right">122 KiB</td></tr>
<tr><td align="right">922</td><td><code>qt6-svg-plugins</code></td><td>6.10.2-2</td><td>Qt 6 SVG library plugins</td><td align="right">122 KiB</td></tr>
<tr><td align="right">923</td><td><code>sshfs</code></td><td>3.7.3-1.1build5</td><td>filesystem client based on SSH File Transfer Protocol</td><td align="right">122 KiB</td></tr>
<tr><td align="right">924</td><td><code>libserd-0-0</code></td><td>0.32.6-1</td><td>lightweight RDF syntax library</td><td align="right">120 KiB</td></tr>
<tr><td align="right">925</td><td><code>libsignon-extension1</code></td><td>8.61+git20231015.c8ad982-8</td><td>Single Sign On framework - extension shared library</td><td align="right">120 KiB</td></tr>
<tr><td align="right">926</td><td><code>ffmpegthumbs</code></td><td>4:25.12.3-0ubuntu1</td><td>video thumbnail generator using ffmpeg</td><td align="right">119 KiB</td></tr>
<tr><td align="right">927</td><td><code>libbaloowidgets-bin</code></td><td>4:25.12.3-0ubuntu1</td><td>Wigets for use with Baloo - binaries</td><td align="right">119 KiB</td></tr>
<tr><td align="right">928</td><td><code>libhwloc-plugins</code></td><td>2.13.0-2</td><td>Hierarchical view of the machine - plugins</td><td align="right">119 KiB</td></tr>
<tr><td align="right">929</td><td><code>libid3tag0</code></td><td>0.16.3-4</td><td>ID3 tag reading library from the MAD project</td><td align="right">119 KiB</td></tr>
<tr><td align="right">930</td><td><code>libtrio2</code></td><td>1.16+dfsg1-6</td><td>portable and extendable printf and string functions</td><td align="right">119 KiB</td></tr>
<tr><td align="right">931</td><td><code>libkate1</code></td><td>0.4.1-12</td><td>Codec for karaoke and text encapsulation</td><td align="right">118 KiB</td></tr>
<tr><td align="right">932</td><td><code>libkdcrawqt6-5</code></td><td>25.12.3-0ubuntu1</td><td>RAW picture decoding library qt6</td><td align="right">118 KiB</td></tr>
<tr><td align="right">933</td><td><code>python3-pil.imagetk</code></td><td>12.1.1-2ubuntu1.1</td><td>Python Imaging Library - ImageTk Module (Python3)</td><td align="right">118 KiB</td></tr>
<tr><td align="right">934</td><td><code>qml6-module-qt-labs-platform</code></td><td>6.10.2+dfsg-3</td><td>Qt 6 Platform QML module</td><td align="right">118 KiB</td></tr>
<tr><td align="right">935</td><td><code>qml6-module-qtquick-tooling</code></td><td>6.10.2+dfsg-3</td><td>Qt 6 Quick Tooling QML module</td><td align="right">118 KiB</td></tr>
<tr><td align="right">936</td><td><code>libkf6kcmutilsquick6</code></td><td>6.24.0-0ubuntu1</td><td>Extra APIs to write KConfig modules</td><td align="right">117 KiB</td></tr>
<tr><td align="right">937</td><td><code>live-config</code></td><td>11.0.5build1</td><td>Live System Configuration Components</td><td align="right">117 KiB</td></tr>
<tr><td align="right">938</td><td><code>libkf5service-bin</code></td><td>5.116.0-1ubuntu1</td><td>Advanced plugin and service introspection</td><td align="right">116 KiB</td></tr>
<tr><td align="right">939</td><td><code>libkf6userfeedbackwidgets6</code></td><td>6.24.0-0ubuntu1</td><td>user feedback for applications - widgets library</td><td align="right">116 KiB</td></tr>
<tr><td align="right">940</td><td><code>libdvdnav4</code></td><td>7.0.0-2</td><td>DVD navigation library</td><td align="right">114 KiB</td></tr>
<tr><td align="right">941</td><td><code>libkf6notifyconfig6</code></td><td>6.24.0-0ubuntu1</td><td>Configuration system for KNotify</td><td align="right">114 KiB</td></tr>
<tr><td align="right">942</td><td><code>liblayershellqtinterface6</code></td><td>6.6.4-0ubuntu1</td><td>use the Wayland wl-layer-shell protocol - run-time library</td><td align="right">114 KiB</td></tr>
<tr><td align="right">943</td><td><code>libplasma-geolocation-interface6</code></td><td>4:6.6.4-0ubuntu1</td><td>Plasma geolocation library</td><td align="right">114 KiB</td></tr>
<tr><td align="right">944</td><td><code>liburiparser1</code></td><td>0.9.8+dfsg-2build1</td><td>URI parsing library compliant with RFC 3986</td><td align="right">114 KiB</td></tr>
<tr><td align="right">945</td><td><code>qml6-module-org-kde-notifications</code></td><td>6.24.0-0ubuntu1</td><td>Framework for desktop notifications qml</td><td align="right">113 KiB</td></tr>
<tr><td align="right">946</td><td><code>libmd4c0</code></td><td>0.5.2-2build1</td><td>Markdown for C</td><td align="right">112 KiB</td></tr>
<tr><td align="right">947</td><td><code>qml-module-qtqml-models2</code></td><td>5.15.18+dfsg-2</td><td>Qt 5 Models2 QML module</td><td align="right">112 KiB</td></tr>
<tr><td align="right">948</td><td><code>libkubuntu1</code></td><td>26.04ubuntu1</td><td>library for Kubuntu platform integration</td><td align="right">111 KiB</td></tr>
<tr><td align="right">949</td><td><code>libsignon-plugins-common1</code></td><td>8.61+git20231015.c8ad982-8</td><td>Single Sign On framework - plugins common shared library</td><td align="right">111 KiB</td></tr>
<tr><td align="right">950</td><td><code>libspatialaudio0t64</code></td><td>0.3.0+git20180730+dfsg1-3</td><td>library for ambisonic encoding and decoding (runtime files)</td><td align="right">111 KiB</td></tr>
<tr><td align="right">951</td><td><code>libkf5declarative5</code></td><td>5.116.0-1ubuntu1</td><td>provides integration of QML and KDE frameworks</td><td align="right">110 KiB</td></tr>
<tr><td align="right">952</td><td><code>libsdl2-ttf-2.0-0</code></td><td>2.24.0+dfsg-3</td><td>TrueType Font library for Simple DirectMedia Layer 2, libraries</td><td align="right">109 KiB</td></tr>
<tr><td align="right">953</td><td><code>libminizip1t64</code></td><td>1:1.3.dfsg+really1.3.1-1ubuntu3</td><td>compression library - minizip library</td><td align="right">108 KiB</td></tr>
<tr><td align="right">954</td><td><code>live-tools</code></td><td>1:20240525build1</td><td>Live System Extra Components</td><td align="right">108 KiB</td></tr>
<tr><td align="right">955</td><td><code>libopencore-amrwb0</code></td><td>0.1.6-1build2</td><td>Adaptive Multi-Rate - Wideband speech codec - shared library</td><td align="right">107 KiB</td></tr>
<tr><td align="right">956</td><td><code>liboxygenstyleconfig6-6</code></td><td>4:6.6.4-0ubuntu1</td><td>style library configuration for the Oxygen desktop theme (Qt 6)</td><td align="right">107 KiB</td></tr>
<tr><td align="right">957</td><td><code>liballegro-image5.2t64</code></td><td>2:5.2.11.3+dfsg-1</td><td>image addon for the Allegro 5 library</td><td align="right">106 KiB</td></tr>
<tr><td align="right">958</td><td><code>libdouble-conversion3</code></td><td>3.4.0-1</td><td>routines to convert IEEE floats to and from strings</td><td align="right">106 KiB</td></tr>
<tr><td align="right">959</td><td><code>libkf6kcmutils-bin</code></td><td>6.24.0-0ubuntu1</td><td>runtime files for kcmutils</td><td align="right">106 KiB</td></tr>
<tr><td align="right">960</td><td><code>libkf6newstuffwidgets6</code></td><td>6.24.0-0ubuntu1</td><td>Support for downloading application assets from the network</td><td align="right">105 KiB</td></tr>
<tr><td align="right">961</td><td><code>libtinyxml2.6.2v5</code></td><td>2.6.2-7build1</td><td>C++ XML parsing library</td><td align="right">105 KiB</td></tr>
<tr><td align="right">962</td><td><code>libdisplay-info-bin</code></td><td>0.3.0-1</td><td>EDID and DisplayID library (utils)</td><td align="right">104 KiB</td></tr>
<tr><td align="right">963</td><td><code>libpulsedsp</code></td><td>1:17.0+dfsg1-2ubuntu4</td><td>PulseAudio OSS pre-load library</td><td align="right">104 KiB</td></tr>
<tr><td align="right">964</td><td><code>qml6-module-org-kde-sonnet</code></td><td>6.24.0-0ubuntu1</td><td>spell checking library for Qt, plugins</td><td align="right">104 KiB</td></tr>
<tr><td align="right">965</td><td><code>liballegro-dialog5.2t64</code></td><td>2:5.2.11.3+dfsg-1</td><td>dialog addon for the Allegro 5 library</td><td align="right">103 KiB</td></tr>
<tr><td align="right">966</td><td><code>breeze</code></td><td>4:6.6.4-0ubuntu1</td><td>Default Plasma theme (Metapackage)</td><td align="right">102 KiB</td></tr>
<tr><td align="right">967</td><td><code>evemu-tools</code></td><td>2.7.0-4build2</td><td>Linux Input Event Device Emulation Library - test tools</td><td align="right">102 KiB</td></tr>
<tr><td align="right">968</td><td><code>libkf6su6</code></td><td>6.24.0-0ubuntu1</td><td>Integration with su for privilege escalation</td><td align="right">102 KiB</td></tr>
<tr><td align="right">969</td><td><code>libknighttime0</code></td><td>6.6.4-0ubuntu1</td><td>dark-light cycle scheduling helpers for Plasma - shared library</td><td align="right">102 KiB</td></tr>
<tr><td align="right">970</td><td><code>qdbus-qt6</code></td><td>6.10.2-1</td><td>Qt 6 D-Bus application</td><td align="right">102 KiB</td></tr>
<tr><td align="right">971</td><td><code>z80asm</code></td><td>1.8-2build1</td><td>assembler for the Zilog Z80 microprocessor</td><td align="right">99 KiB</td></tr>
<tr><td align="right">972</td><td><code>z80dasm</code></td><td>1.1.6-1build1</td><td>disassembler for the Zilog Z80 microprocessor</td><td align="right">99 KiB</td></tr>
<tr><td align="right">973</td><td><code>libblosc1</code></td><td>1.21.5+ds-2</td><td>high performance meta-compressor optimized for binary data</td><td align="right">98 KiB</td></tr>
<tr><td align="right">974</td><td><code>libkf6userfeedback-doc</code></td><td>6.24.0-0ubuntu1</td><td>user feedback for applications - documentation</td><td align="right">98 KiB</td></tr>
<tr><td align="right">975</td><td><code>libmpvqt2</code></td><td>1.1.1-2</td><td>Libmpv wrapper for QtQuick2 and QML</td><td align="right">96 KiB</td></tr>
<tr><td align="right">976</td><td><code>libcddb2</code></td><td>1.3.2-7.1fakesync1build1</td><td>library to access CDDB data - runtime files</td><td align="right">95 KiB</td></tr>
<tr><td align="right">977</td><td><code>libepub0</code></td><td>0.2.2-8</td><td>library to work with the EPUB file format - runtime library</td><td align="right">95 KiB</td></tr>
<tr><td align="right">978</td><td><code>libkcolorpicker-qt6-0</code></td><td>0.3.1-3</td><td>QToolButton-like widget with color selection popup menu (Qt6 lib)</td><td align="right">95 KiB</td></tr>
<tr><td align="right">979</td><td><code>libkf6su-bin</code></td><td>6.24.0-0ubuntu1</td><td>runtime files for kdesu</td><td align="right">95 KiB</td></tr>
<tr><td align="right">980</td><td><code>libkf6configqml6</code></td><td>6.24.0-0ubuntu1</td><td>configuration settings framework for Qt</td><td align="right">94 KiB</td></tr>
<tr><td align="right">981</td><td><code>libkirigamilayoutsprivate6</code></td><td>6.24.0-0ubuntu1</td><td>set of QtQuick components targeted for mobile use</td><td align="right">93 KiB</td></tr>
<tr><td align="right">982</td><td><code>libpystring0</code></td><td>1.1.4-2</td><td>collection of C++ functions for Python&#x27;s string class methods - runtime</td><td align="right">93 KiB</td></tr>
<tr><td align="right">983</td><td><code>qml6-module-qtquick-shapes</code></td><td>6.10.2+dfsg-3</td><td>Qt 6 Quick Shapes QML module</td><td align="right">93 KiB</td></tr>
<tr><td align="right">984</td><td><code>libsndio7.0</code></td><td>1.10.0-0.2</td><td>Small audio and MIDI framework from OpenBSD, runtime libraries</td><td align="right">92 KiB</td></tr>
<tr><td align="right">985</td><td><code>libvidstab1.1</code></td><td>1.1.0-2.1</td><td>video stabilization library (shared library)</td><td align="right">92 KiB</td></tr>
<tr><td align="right">986</td><td><code>qml-module-org-kde-sonnet</code></td><td>5.116.0-1ubuntu2</td><td>spell checking library for Qt, plugins</td><td align="right">92 KiB</td></tr>
<tr><td align="right">987</td><td><code>libaribb24-0t64</code></td><td>1.0.3-3</td><td>library for ARIB STD-B24 decoding (runtime files)</td><td align="right">90 KiB</td></tr>
<tr><td align="right">988</td><td><code>libgvplugin-pango8</code></td><td>14.1.2-1ubuntu1</td><td>rich set of graph drawing tools - pango plugin</td><td align="right">90 KiB</td></tr>
<tr><td align="right">989</td><td><code>libkf6screendpms8</code></td><td>4:6.6.4-0ubuntu1</td><td>library for screen management - dpms support library</td><td align="right">90 KiB</td></tr>
<tr><td align="right">990</td><td><code>libopenmpt-modplug1</code></td><td>0.8.9.0-openmpt1-2build3</td><td>module music library based on OpenMPT -- modplug compat library</td><td align="right">90 KiB</td></tr>
<tr><td align="right">991</td><td><code>libqt6webview6</code></td><td>6.10.2-1</td><td>Qt 6 WebView library</td><td align="right">89 KiB</td></tr>
<tr><td align="right">992</td><td><code>wayland-utils</code></td><td>1.3.0-1</td><td>Wayland utilities</td><td align="right">89 KiB</td></tr>
<tr><td align="right">993</td><td><code>libappmenu-gtk3-parser0</code></td><td>25.04-1build1</td><td>GtkMenuShell to GMenuModel parser (GTK+3.0)</td><td align="right">88 KiB</td></tr>
<tr><td align="right">994</td><td><code>libchromaprint1</code></td><td>1.6.0-2build1</td><td>audio fingerprint library</td><td align="right">88 KiB</td></tr>
<tr><td align="right">995</td><td><code>libgvplugin-gd8</code></td><td>14.1.2-1ubuntu1</td><td>rich set of graph drawing tools - gd plugin</td><td align="right">88 KiB</td></tr>
<tr><td align="right">996</td><td><code>libgsm1</code></td><td>1.0.23-2</td><td>Shared libraries for GSM speech compressor</td><td align="right">87 KiB</td></tr>
<tr><td align="right">997</td><td><code>libkf5kiontlm5</code></td><td>5.116.0-2</td><td>resource and network access abstraction (KIO NTLM library)</td><td align="right">87 KiB</td></tr>
<tr><td align="right">998</td><td><code>liballegro-acodec5.2t64</code></td><td>2:5.2.11.3+dfsg-1</td><td>audio codec addon for the Allegro 5 library</td><td align="right">86 KiB</td></tr>
<tr><td align="right">999</td><td><code>libmpcdec6</code></td><td>2:0.1~r495-3build1</td><td>MusePack decoder - library</td><td align="right">86 KiB</td></tr>
<tr><td align="right">1000</td><td><code>qml6-module-org-kde-graphicaleffects</code></td><td>6.24.0-0ubuntu1</td><td>provides integration of QML and KDE Frameworks - graphicaleffects</td><td align="right">86 KiB</td></tr>
<tr><td align="right">1001</td><td><code>qml6-module-org-kde-guiaddons</code></td><td>6.24.0-0ubuntu1.1</td><td>additional addons for QtGui</td><td align="right">86 KiB</td></tr>
<tr><td align="right">1002</td><td><code>liba52-0.7.4</code></td><td>0.7.4-22</td><td>library for decoding ATSC A/52 streams</td><td align="right">85 KiB</td></tr>
<tr><td align="right">1003</td><td><code>liballegro-video5.2t64</code></td><td>2:5.2.11.3+dfsg-1</td><td>video addon for the Allegro 5 library</td><td align="right">85 KiB</td></tr>
<tr><td align="right">1004</td><td><code>libkf6prisonscanner6</code></td><td>6.24.0-0ubuntu1</td><td>barcode API for Qt - barcode scanner</td><td align="right">85 KiB</td></tr>
<tr><td align="right">1005</td><td><code>libkf6service-bin</code></td><td>6.24.0-0ubuntu1</td><td>Advanced plugin and service introspection</td><td align="right">85 KiB</td></tr>
<tr><td align="right">1006</td><td><code>libspectre1</code></td><td>0.2.12-2</td><td>Library for rendering PostScript documents</td><td align="right">85 KiB</td></tr>
<tr><td align="right">1007</td><td><code>qml6-module-org-kde-kholidays</code></td><td>6.24.0-0ubuntu1</td><td>holidays calculation library - qml files</td><td align="right">85 KiB</td></tr>
<tr><td align="right">1008</td><td><code>kde-config-updates</code></td><td>6.6.4-0ubuntu1</td><td>Unattended updates configuration</td><td align="right">84 KiB</td></tr>
<tr><td align="right">1009</td><td><code>libcjson1</code></td><td>1.7.19-2</td><td>Ultralightweight JSON parser in ANSI C</td><td align="right">83 KiB</td></tr>
<tr><td align="right">1010</td><td><code>libsfml-system3.0</code></td><td>3.0.2+dfsg-2</td><td>Simple and Fast Multimedia Library - System part</td><td align="right">82 KiB</td></tr>
<tr><td align="right">1011</td><td><code>libqt6svgwidgets6</code></td><td>6.10.2-2</td><td>Qt 6 SVG Widgets library</td><td align="right">80 KiB</td></tr>
<tr><td align="right">1012</td><td><code>libkf6crash6</code></td><td>6.24.0-0ubuntu1</td><td>Support for application crash analysis and bug report from apps</td><td align="right">79 KiB</td></tr>
<tr><td align="right">1013</td><td><code>libqt6multimediawidgets6</code></td><td>6.10.2-2</td><td>Qt 6 Multimedia Widgets library</td><td align="right">79 KiB</td></tr>
<tr><td align="right">1014</td><td><code>qml6-module-qtqml</code></td><td>6.10.2+dfsg-3</td><td>Qt 6 QtQml QML module</td><td align="right">79 KiB</td></tr>
<tr><td align="right">1015</td><td><code>imagemagick</code></td><td>8:7.1.2.18+dfsg1-1</td><td>image manipulation programs -- binaries</td><td align="right">78 KiB</td></tr>
<tr><td align="right">1016</td><td><code>libixml11t64</code></td><td>1:1.14.25-1ubuntu1</td><td>Portable SDK for UPnP Devices (ixml shared library)</td><td align="right">78 KiB</td></tr>
<tr><td align="right">1017</td><td><code>libkf5globalaccel-bin</code></td><td>5.116.0-1ubuntu1</td><td>Configurable global shortcut support.</td><td align="right">78 KiB</td></tr>
<tr><td align="right">1018</td><td><code>libkf6pty6</code></td><td>6.24.0-0ubuntu1</td><td>Pty abstraction</td><td align="right">78 KiB</td></tr>
<tr><td align="right">1019</td><td><code>liblqr-1-0</code></td><td>0.4.2-2.2</td><td>converts plain array images into multi-size representation</td><td align="right">78 KiB</td></tr>
<tr><td align="right">1020</td><td><code>live-config-doc</code></td><td>11.0.5build1</td><td>Live System Configuration Components (documentation)</td><td align="right">78 KiB</td></tr>
<tr><td align="right">1021</td><td><code>xsettingsd</code></td><td>1.0.2-1build2</td><td>Provides settings to X11 applications</td><td align="right">78 KiB</td></tr>
<tr><td align="right">1022</td><td><code>libkf6peoplebackend6</code></td><td>6.24.0-0ubuntu1</td><td>framework providing unified access to contacts aggregated by person</td><td align="right">77 KiB</td></tr>
<tr><td align="right">1023</td><td><code>libkirigamiapp6</code></td><td>1.11.0-2ubuntu2</td><td>app library</td><td align="right">77 KiB</td></tr>
<tr><td align="right">1024</td><td><code>libplasmaweatherion6</code></td><td>4:6.6.4-0ubuntu1</td><td>weather location and forcast library for Plasma widgets</td><td align="right">77 KiB</td></tr>
<tr><td align="right">1025</td><td><code>libstoken1t64</code></td><td>0.93-1</td><td>Software Token for cryptographic authentication - shared library</td><td align="right">76 KiB</td></tr>
<tr><td align="right">1026</td><td><code>libkirigamiprivate6</code></td><td>6.24.0-0ubuntu1</td><td>set of QtQuick components targeted for mobile use</td><td align="right">75 KiB</td></tr>
<tr><td align="right">1027</td><td><code>libkdsingleapplication-qt6-1.0</code></td><td>1.2.0-1</td><td>KDAB&#x27;s helper class for single-instance policy applications (lib)</td><td align="right">74 KiB</td></tr>
<tr><td align="right">1028</td><td><code>libkf5xmlgui-bin</code></td><td>5.116.0-1ubuntu4</td><td>User configurable main windows.</td><td align="right">74 KiB</td></tr>
<tr><td align="right">1029</td><td><code>libksysguardformatter2</code></td><td>4:6.6.4-0ubuntu1</td><td>library for system monitoring - ksysguardformatter shared library</td><td align="right">74 KiB</td></tr>
<tr><td align="right">1030</td><td><code>qml6-module-qtqml-workerscript</code></td><td>6.10.2+dfsg-3</td><td>Qt 6 QtQml Workerscript QML module</td><td align="right">74 KiB</td></tr>
<tr><td align="right">1031</td><td><code>apport-kde</code></td><td>2.34.0-0ubuntu2</td><td>KDE frontend for the apport crash report system</td><td align="right">73 KiB</td></tr>
<tr><td align="right">1032</td><td><code>evtest</code></td><td>1:1.36-1</td><td>utility to monitor Linux input device events</td><td align="right">73 KiB</td></tr>
<tr><td align="right">1033</td><td><code>libqrencode4</code></td><td>4.1.1-2build1</td><td>QR Code encoding library</td><td align="right">73 KiB</td></tr>
<tr><td align="right">1034</td><td><code>libqt5texttospeech5</code></td><td>5.15.18-1</td><td>Speech library for Qt - libraries</td><td align="right">73 KiB</td></tr>
<tr><td align="right">1035</td><td><code>qml6-module-qtquick-window</code></td><td>6.10.2+dfsg-3</td><td>Qt 6 Quick Window QML module</td><td align="right">73 KiB</td></tr>
<tr><td align="right">1036</td><td><code>libheif-plugin-x265</code></td><td>1.21.2-3</td><td>HEIF and AVIF file format decoder and encoder - x265 plugin</td><td align="right">72 KiB</td></tr>
<tr><td align="right">1037</td><td><code>libjxr-tools</code></td><td>1.2~git20170615.f752187-5.3build1</td><td>JPEG-XR lib - command line apps</td><td align="right">72 KiB</td></tr>
<tr><td align="right">1038</td><td><code>libkf5crash5</code></td><td>5.116.0-1ubuntu1</td><td>Support for application crash analysis and bug report from apps</td><td align="right">71 KiB</td></tr>
<tr><td align="right">1039</td><td><code>libpathplan4</code></td><td>14.1.2-1ubuntu1</td><td>rich set of graph drawing tools - pathplan library</td><td align="right">71 KiB</td></tr>
<tr><td align="right">1040</td><td><code>libqt6webchannelquick6</code></td><td>6.10.2-1</td><td>Qt 6 WebChannel Quick library</td><td align="right">71 KiB</td></tr>
<tr><td align="right">1041</td><td><code>libzix-0-0</code></td><td>0.8.0-1</td><td>library of portability wrappers and data structures (shared library)</td><td align="right">71 KiB</td></tr>
<tr><td align="right">1042</td><td><code>kwrited</code></td><td>4:6.6.4-0ubuntu1</td><td>Read and write console output to X</td><td align="right">70 KiB</td></tr>
<tr><td align="right">1043</td><td><code>libkf6purposewidgets6</code></td><td>6.24.0-0ubuntu1</td><td>library for abstractions to get the developer&#x27;s purposes fulfilled</td><td align="right">70 KiB</td></tr>
<tr><td align="right">1044</td><td><code>live-boot-doc</code></td><td>1:20250815</td><td>Live System Boot Components (documentation)</td><td align="right">70 KiB</td></tr>
<tr><td align="right">1045</td><td><code>liballegro-ttf5.2t64</code></td><td>2:5.2.11.3+dfsg-1</td><td>ttf addon for the Allegro 5 library</td><td align="right">69 KiB</td></tr>
<tr><td align="right">1046</td><td><code>qml6-module-org-kde-iconthemes</code></td><td>6.24.0-0ubuntu2</td><td>Support for icon themes (QML module)</td><td align="right">69 KiB</td></tr>
<tr><td align="right">1047</td><td><code>libkf5style5</code></td><td>5.116.0-3</td><td>KF5 cross-framework integration plugins - KStyle</td><td align="right">67 KiB</td></tr>
<tr><td align="right">1048</td><td><code>libkf6style6</code></td><td>6.24.0-0ubuntu1</td><td>integration of Qt application with KDE workspaces - KF6Style shared library</td><td align="right">66 KiB</td></tr>
<tr><td align="right">1049</td><td><code>libqcoro6dbus0t64</code></td><td>0.13.0-1ubuntu2</td><td>C++20 coroutines for Qt6 - DBus</td><td align="right">66 KiB</td></tr>
<tr><td align="right">1050</td><td><code>ziptool</code></td><td>1.11.4-2</td><td>modify zip archives</td><td align="right">66 KiB</td></tr>
<tr><td align="right">1051</td><td><code>libkf6iconthemes-bin</code></td><td>6.24.0-0ubuntu2</td><td>Support for icon themes &amp; widgets</td><td align="right">65 KiB</td></tr>
<tr><td align="right">1052</td><td><code>libxdot4</code></td><td>14.1.2-1ubuntu1</td><td>rich set of graph drawing tools - xdot library</td><td align="right">65 KiB</td></tr>
<tr><td align="right">1053</td><td><code>python3-dbus.mainloop.pyqt6</code></td><td>6.10.2-2build5</td><td>D-Bus Qt main loop support for Python</td><td align="right">65 KiB</td></tr>
<tr><td align="right">1054</td><td><code>plasma-activities-bin</code></td><td>6.6.4-0ubuntu1</td><td>core components for the KDE&#x27;s Activities system - CLI</td><td align="right">64 KiB</td></tr>
<tr><td align="right">1055</td><td><code>qml6-module-org-kde-kwindowsystem</code></td><td>6.24.0-0ubuntu1</td><td>QML module for kwindowsystem</td><td align="right">63 KiB</td></tr>
<tr><td align="right">1056</td><td><code>libaec0</code></td><td>1.1.5-1</td><td>Adaptive Entropy Coding library</td><td align="right">62 KiB</td></tr>
<tr><td align="right">1057</td><td><code>libsratom-0-0</code></td><td>0.6.20-1</td><td>library for serialising LV2 atoms to/from Turtle</td><td align="right">62 KiB</td></tr>
<tr><td align="right">1058</td><td><code>qml6-module-org-kde-config</code></td><td>6.24.0-0ubuntu1</td><td>QML module for kconfig</td><td align="right">62 KiB</td></tr>
<tr><td align="right">1059</td><td><code>qtspeech5-speechd-plugin</code></td><td>5.15.18-1</td><td>Speech library for Qt - speechd plugin</td><td align="right">60 KiB</td></tr>
<tr><td align="right">1060</td><td><code>vpnc-scripts</code></td><td>0.1~git20220510-1.1</td><td>Network configuration scripts for VPNC and OpenConnect</td><td align="right">60 KiB</td></tr>
<tr><td align="right">1061</td><td><code>appmenu-gtk3-module</code></td><td>25.04-1build1</td><td>GtkMenuShell D-Bus exporter (GTK+3.0)</td><td align="right">59 KiB</td></tr>
<tr><td align="right">1062</td><td><code>kpackagelauncherqml</code></td><td>5.116.0-1ubuntu1</td><td>commandline tool for launching kpackage QML application</td><td align="right">59 KiB</td></tr>
<tr><td align="right">1063</td><td><code>libcdt6</code></td><td>14.1.2-1ubuntu1</td><td>rich set of graph drawing tools - cdt library</td><td align="right">59 KiB</td></tr>
<tr><td align="right">1064</td><td><code>libkf6guiaddons-bin</code></td><td>6.24.0-0ubuntu1.1</td><td>additional addons for QtGui</td><td align="right">59 KiB</td></tr>
<tr><td align="right">1065</td><td><code>libshine3</code></td><td>3.1.1-3build1</td><td>Fixed-point MP3 encoding library - runtime files</td><td align="right">59 KiB</td></tr>
<tr><td align="right">1066</td><td><code>signon-kwallet-extension</code></td><td>4:25.12.3-0ubuntu1</td><td>KWallet extension for signond</td><td align="right">59 KiB</td></tr>
<tr><td align="right">1067</td><td><code>liballegro-physfs5.2t64</code></td><td>2:5.2.11.3+dfsg-1</td><td>physfs addon for the Allegro 5 library</td><td align="right">58 KiB</td></tr>
<tr><td align="right">1068</td><td><code>libkf6calendarevents6</code></td><td>6.24.0-0ubuntu1</td><td>provides integration of QML and KDE frameworks -- calendarevents</td><td align="right">58 KiB</td></tr>
<tr><td align="right">1069</td><td><code>libportmidi2</code></td><td>2:2.0.8-1</td><td>library for real-time MIDI input/output</td><td align="right">58 KiB</td></tr>
<tr><td align="right">1070</td><td><code>libva-wayland2</code></td><td>2.23.0-1ubuntu1</td><td>Video Acceleration (VA) API for Linux -- Wayland runtime</td><td align="right">57 KiB</td></tr>
<tr><td align="right">1071</td><td><code>libva-x11-2</code></td><td>2.23.0-1ubuntu1</td><td>Video Acceleration (VA) API for Linux -- X11 runtime</td><td align="right">57 KiB</td></tr>
<tr><td align="right">1072</td><td><code>liballegro-audio5-dev</code></td><td>2:5.2.11.3+dfsg-1</td><td>header files for the Allegro 5 audio addon</td><td align="right">56 KiB</td></tr>
<tr><td align="right">1073</td><td><code>libkf5doctools5</code></td><td>5.116.0-1ubuntu2</td><td>Tools to generate documentation in various formats from DocBook</td><td align="right">56 KiB</td></tr>
<tr><td align="right">1074</td><td><code>qml6-module-org-kde-layershell</code></td><td>6.6.4-0ubuntu1</td><td>use the Wayland wl-layer-shell protocol - QML module</td><td align="right">56 KiB</td></tr>
<tr><td align="right">1075</td><td><code>libkf5iconthemes-bin</code></td><td>5.116.0-1ubuntu4</td><td>Support for icon themes</td><td align="right">55 KiB</td></tr>
<tr><td align="right">1076</td><td><code>libsord-0-0</code></td><td>0.16.20-1</td><td>library for storing RDF data in memory</td><td align="right">55 KiB</td></tr>
<tr><td align="right">1077</td><td><code>libudfread3</code></td><td>1.2.0-2</td><td>UDF reader library</td><td align="right">55 KiB</td></tr>
<tr><td align="right">1078</td><td><code>libdolphinvcs6</code></td><td>4:25.12.3-0ubuntu1</td><td>library to show version control in Dolphin</td><td align="right">54 KiB</td></tr>
<tr><td align="right">1079</td><td><code>libkdecorations3private2</code></td><td>4:6.6.4-0ubuntu1</td><td>library to create window decorations - private library</td><td align="right">54 KiB</td></tr>
<tr><td align="right">1080</td><td><code>ncompress</code></td><td>5.0-4</td><td>original Lempel-Ziv compress/uncompress programs</td><td align="right">54 KiB</td></tr>
<tr><td align="right">1081</td><td><code>libsz2</code></td><td>1.1.5-1</td><td>Adaptive Entropy Coding library - SZIP</td><td align="right">53 KiB</td></tr>
<tr><td align="right">1082</td><td><code>zipcmp</code></td><td>1.11.4-2</td><td>compare contents of zip archives</td><td align="right">53 KiB</td></tr>
<tr><td align="right">1083</td><td><code>libkf6doctools6</code></td><td>6.24.0-0ubuntu1</td><td>Tools to generate documentation in various formats from DocBook</td><td align="right">52 KiB</td></tr>
<tr><td align="right">1084</td><td><code>libkf6texteditor-katepart</code></td><td>6.24.0-0ubuntu1</td><td>provide advanced plain text editing services</td><td align="right">52 KiB</td></tr>
<tr><td align="right">1085</td><td><code>libqmobipocket6-3</code></td><td>4:25.12.3-0ubuntu1</td><td>Qt6 library for reading Mobipocket documents</td><td align="right">52 KiB</td></tr>
<tr><td align="right">1086</td><td><code>libspnav0</code></td><td>1.2-1build1</td><td>Library to access 3D-input-devices (development files)</td><td align="right">52 KiB</td></tr>
<tr><td align="right">1087</td><td><code>qml6-module-org-kde-networkmanager</code></td><td>6.24.0-0ubuntu1</td><td>Qt wrapper for NetworkManager - QML module</td><td align="right">52 KiB</td></tr>
<tr><td align="right">1088</td><td><code>aha</code></td><td>0.5.1-3build2</td><td>ANSI color to HTML converter</td><td align="right">50 KiB</td></tr>
<tr><td align="right">1089</td><td><code>libevemu3t64</code></td><td>2.7.0-4build2</td><td>Linux Input Event Device Emulation Library</td><td align="right">50 KiB</td></tr>
<tr><td align="right">1090</td><td><code>ubuntu-release-upgrader-qt</code></td><td>1:26.04.18</td><td>manage release upgrades</td><td align="right">50 KiB</td></tr>
<tr><td align="right">1091</td><td><code>gnustep-multiarch</code></td><td>2.9.3-7</td><td>GNUstep Multi-Arch support</td><td align="right">49 KiB</td></tr>
<tr><td align="right">1092</td><td><code>libpotrace0</code></td><td>1.16-2build2</td><td>library for tracing bitmaps</td><td align="right">49 KiB</td></tr>
<tr><td align="right">1093</td><td><code>libsox-fmt-alsa</code></td><td>14.7.0.9+ds1-1</td><td>SoX alsa format I/O library</td><td align="right">49 KiB</td></tr>
<tr><td align="right">1094</td><td><code>libtbbbind-2-5</code></td><td>2022.3.0-2</td><td>parallelism library for C++ - runtime files</td><td align="right">49 KiB</td></tr>
<tr><td align="right">1095</td><td><code>python3-zopfli</code></td><td>0.4.1-1</td><td>cPython bindings for zopfli (Python 3)</td><td align="right">49 KiB</td></tr>
<tr><td align="right">1096</td><td><code>libheif-plugin-j2kdec</code></td><td>1.21.2-3</td><td>HEIF and AVIF file format decoder and encoder - j2kdec plugin</td><td align="right">48 KiB</td></tr>
<tr><td align="right">1097</td><td><code>libheif-plugin-libde265</code></td><td>1.21.2-3</td><td>HEIF and AVIF file format decoder and encoder - libde265 plugin</td><td align="right">48 KiB</td></tr>
<tr><td align="right">1098</td><td><code>kglobalacceld</code></td><td>6.6.4-0ubuntu1</td><td>daemon for global keyboard shortcuts on the Plasma desktop</td><td align="right">47 KiB</td></tr>
<tr><td align="right">1099</td><td><code>libebur128-1</code></td><td>1.2.6-2</td><td>implementation of the EBU R128 loudness standard</td><td align="right">47 KiB</td></tr>
<tr><td align="right">1100</td><td><code>libpam-kwallet5</code></td><td>4:6.6.4-0ubuntu1</td><td>KWallet integration with PAM</td><td align="right">46 KiB</td></tr>
<tr><td align="right">1101</td><td><code>libkf6dbusaddons-bin</code></td><td>6.24.0-0ubuntu1</td><td>class library for qtdbus</td><td align="right">45 KiB</td></tr>
<tr><td align="right">1102</td><td><code>libkpipewiredmabuf6</code></td><td>6.6.4-0ubuntu1</td><td>KDE&#x27;s Pipewire libraries - libkpipewiredmabuf6</td><td align="right">45 KiB</td></tr>
<tr><td align="right">1103</td><td><code>libqt5x11extras5</code></td><td>5.15.18-1</td><td>Qt 5 X11 extras</td><td align="right">45 KiB</td></tr>
<tr><td align="right">1104</td><td><code>libbs2b0</code></td><td>3.1.0+dfsg-8build1</td><td>Bauer stereophonic-to-binaural DSP library</td><td align="right">44 KiB</td></tr>
<tr><td align="right">1105</td><td><code>libsdl2-net-2.0-0</code></td><td>2.2.0+dfsg-4</td><td>Network library for Simple DirectMedia Layer 2, libraries</td><td align="right">44 KiB</td></tr>
<tr><td align="right">1106</td><td><code>qml6-module-qtwebchannel</code></td><td>6.10.2-1</td><td>Qt 6 WebChannel QML module</td><td align="right">44 KiB</td></tr>
<tr><td align="right">1107</td><td><code>libqt6bluetooth6-bin</code></td><td>6.10.2-1</td><td>Qt 6 Connectivity Bluetooth module helper binaries</td><td align="right">43 KiB</td></tr>
<tr><td align="right">1108</td><td><code>libxcb-render-util0</code></td><td>0.3.10-1build1</td><td>utility libraries for X C Binding -- render-util</td><td align="right">43 KiB</td></tr>
<tr><td align="right">1109</td><td><code>liballegro-dialog5-dev</code></td><td>2:5.2.11.3+dfsg-1</td><td>header files for the Allegro 5 dialog addon</td><td align="right">42 KiB</td></tr>
<tr><td align="right">1110</td><td><code>libxcb-icccm4</code></td><td>0.4.2-1build1</td><td>utility libraries for X C Binding -- icccm</td><td align="right">41 KiB</td></tr>
<tr><td align="right">1111</td><td><code>python3-cycler</code></td><td>0.12.1-2</td><td>composable kwarg iterator (Python 3)</td><td align="right">41 KiB</td></tr>
<tr><td align="right">1112</td><td><code>libkirigamipolyfill6</code></td><td>6.24.0-0ubuntu1</td><td>set of QtQuick components targeted for mobile use</td><td align="right">40 KiB</td></tr>
<tr><td align="right">1113</td><td><code>liballegro-video5-dev</code></td><td>2:5.2.11.3+dfsg-1</td><td>header files for the Allegro 5 video addon</td><td align="right">39 KiB</td></tr>
<tr><td align="right">1114</td><td><code>libproxy-tools</code></td><td>0.5.12-1</td><td>automatic proxy configuration management library (tools)</td><td align="right">39 KiB</td></tr>
<tr><td align="right">1115</td><td><code>libxcb-cursor0</code></td><td>0.1.6-1</td><td>utility libraries for X C Binding -- cursor</td><td align="right">39 KiB</td></tr>
<tr><td align="right">1116</td><td><code>debconf-kde-helper</code></td><td>1.2.0-2ubuntu1</td><td>Debconf KDE GUI frontend tool</td><td align="right">38 KiB</td></tr>
<tr><td align="right">1117</td><td><code>layer-shell-qt</code></td><td>6.6.4-0ubuntu1</td><td>use the Wayland wl-layer-shell protocol - plugin</td><td align="right">38 KiB</td></tr>
<tr style="font-style:italic;color:#666"><td align="right">1118</td><td><code>libopenblas0</code><span style="font-size:0.7em;font-weight:normal;background:#e0e0e0;color:#555;border-radius:3px;padding:1px 4px;margin-left:5px;font-style:normal;vertical-align:middle">meta</span></td><td>0.3.32+ds-5</td><td>Optimized BLAS (linear algebra) library (meta)</td><td align="right">38 KiB</td></tr>
<tr><td align="right">1119</td><td><code>liballegro-acodec5-dev</code></td><td>2:5.2.11.3+dfsg-1</td><td>header files for the Allegro 5 audio codec addon</td><td align="right">37 KiB</td></tr>
<tr><td align="right">1120</td><td><code>liballegro-image5-dev</code></td><td>2:5.2.11.3+dfsg-1</td><td>header files for the Allegro 5 image addon</td><td align="right">37 KiB</td></tr>
<tr><td align="right">1121</td><td><code>liballegro-physfs5-dev</code></td><td>2:5.2.11.3+dfsg-1</td><td>header files for the Allegro 5 physfs addon</td><td align="right">37 KiB</td></tr>
<tr><td align="right">1122</td><td><code>liballegro-ttf5-dev</code></td><td>2:5.2.11.3+dfsg-1</td><td>header files for the Allegro 5 ttf addon</td><td align="right">37 KiB</td></tr>
<tr><td align="right">1123</td><td><code>libkf5dbusaddons-bin</code></td><td>5.116.0-2</td><td>class library for qtdbus</td><td align="right">37 KiB</td></tr>
<tr><td align="right">1124</td><td><code>zipmerge</code></td><td>1.11.4-2</td><td>merge zip archives</td><td align="right">37 KiB</td></tr>
<tr><td align="right">1125</td><td><code>qt6-virtualkeyboard-plugin</code></td><td>6.10.2+dfsg-1</td><td>Qt 6 Virtual Keyboard</td><td align="right">36 KiB</td></tr>
<tr><td align="right">1126</td><td><code>fonts-dejavu</code></td><td>2.37-8build1</td><td>metapackage to pull in fonts-dejavu-core, -mono and -extra</td><td align="right">35 KiB</td></tr>
<tr><td align="right">1127</td><td><code>libxcb-image0</code></td><td>0.4.0-2build2</td><td>utility libraries for X C Binding -- image</td><td align="right">35 KiB</td></tr>
<tr><td align="right">1128</td><td><code>libxcb-keysyms1</code></td><td>0.4.1-1build1</td><td>utility libraries for X C Binding -- keysyms</td><td align="right">34 KiB</td></tr>
<tr><td align="right">1129</td><td><code>libxpresent1</code></td><td>1.0.1-1build1</td><td>X11 Present extension library</td><td align="right">34 KiB</td></tr>
<tr><td align="right">1130</td><td><code>kde-config-gtk-style-preview</code></td><td>4:6.6.4-0ubuntu1</td><td>KDE configuration module for GTK+ 2.x and GTK+ 3.x styles selection (extras)</td><td align="right">33 KiB</td></tr>
<tr><td align="right">1131</td><td><code>plymouth-theme-kubuntu-text</code></td><td>1:26.04.13</td><td>graphical boot animation and logger - kubuntu-text theme</td><td align="right">32 KiB</td></tr>
<tr><td align="right">1132</td><td><code>fonts-noto-hinted</code></td><td>20201225-2build1</td><td>obsolete metapackage to pull in a subset of Noto fonts</td><td align="right">31 KiB</td></tr>
<tr><td align="right">1133</td><td><code>fonts-noto-unhinted</code></td><td>20201225-2build1</td><td>&quot;No Tofu&quot; font families with large Unicode coverage (unhinted)</td><td align="right">31 KiB</td></tr>
<tr><td align="right">1134</td><td><code>libxdgutilsbasedir1.0.1</code></td><td>1.0.1-3.2</td><td>Implementation BaseDir in Free Desktop Standards for C++</td><td align="right">31 KiB</td></tr>
<tr><td align="right">1135</td><td><code>cramfsswap</code></td><td>1.4.5</td><td>swap endianness of a cram filesystem (cramfs)</td><td align="right">28 KiB</td></tr>
<tr><td align="right">1136</td><td><code>libpam-kwallet-common</code></td><td>4:6.6.4-0ubuntu1</td><td>KWallet integration with PAM (common files)</td><td align="right">28 KiB</td></tr>
<tr><td align="right">1137</td><td><code>libfakekey0</code></td><td>0.3+git20170516-4</td><td>library for converting characters to X key-presses [runtime]</td><td align="right">27 KiB</td></tr>
<tr><td align="right">1138</td><td><code>libkf6guiaddons-data</code></td><td>6.24.0-0ubuntu1.1</td><td>additional addons for QtGui</td><td align="right">27 KiB</td></tr>
<tr><td align="right">1139</td><td><code>python3-tk</code></td><td>3.14.3-0ubuntu2</td><td>Tkinter - Writing Tk applications with Python 3.x</td><td align="right">27 KiB</td></tr>
<tr><td align="right">1140</td><td><code>phonon4qt6</code></td><td>4:4.12.0-7</td><td>multimedia framework from KDE using Qt 6 - metapackage</td><td align="right">26 KiB</td></tr>
<tr><td align="right">1141</td><td><code>live-boot-initramfs-tools</code></td><td>1:20250815</td><td>Live System Boot Components (initramfs-tools backend)</td><td align="right">25 KiB</td></tr>
<tr><td align="right">1142</td><td><code>live-config-systemd</code></td><td>11.0.5build1</td><td>Live System Configuration Components (systemd backend)</td><td align="right">24 KiB</td></tr>
<tr><td align="right">1143</td><td><code>va-driver-all</code></td><td>2.23.0-1ubuntu1</td><td>Video Acceleration (VA) API -- driver metapackage</td><td align="right">24 KiB</td></tr>
<tr><td align="right">1144</td><td><code>ppa-purge</code></td><td>0.2.8+bzr63-0ubuntu4</td><td>disables a PPA and reverts to official packages</td><td align="right">23 KiB</td></tr>
<tr><td align="right">1145</td><td><code>appmenu-gtk-module-common</code></td><td>25.04-1build1</td><td>Common files for GtkMenuShell D-Bus exporter</td><td align="right">22 KiB</td></tr>
<tr><td align="right">1146</td><td><code>kwayland5-data</code></td><td>4:5.116.0-0ubuntu7</td><td>Qt library wrapper for Wayland libraries - data files</td><td align="right">22 KiB</td></tr>
<tr><td align="right">1147</td><td><code>isympy3</code></td><td>1.14.0-2</td><td>Python3 shell for SymPy</td><td align="right">19 KiB</td></tr>
<tr><td align="right">1148</td><td><code>libkf5guiaddons-bin</code></td><td>5.116.0-2ubuntu1</td><td>additional addons for QtGui (runtime)</td><td align="right">18 KiB</td></tr>
<tr><td align="right">1149</td><td><code>libkf5guiaddons-data</code></td><td>5.116.0-2ubuntu1</td><td>additional addons for QtGui (documentation)</td><td align="right">18 KiB</td></tr>
<tr><td align="right">1150</td><td><code>sse3-support</code></td><td>27ubuntu2</td><td>CPU feature checking - require SSE3</td><td align="right">18 KiB</td></tr>
<tr><td align="right">1151</td><td><code>kwayland6-data</code></td><td>4:6.6.4-0ubuntu1</td><td>Qt library wrapper for Wayland libraries - data files</td><td align="right">17 KiB</td></tr>
<tr><td align="right">1152</td><td><code>libkf5wallet-bin</code></td><td>5.116.0-1ubuntu1</td><td>transitional dummy package</td><td align="right">17 KiB</td></tr>
<tr><td align="right">1153</td><td><code>libplasmaactivities-data</code></td><td>6.6.4-0ubuntu1</td><td>core components for the KDE&#x27;s Activities system - shared library</td><td align="right">17 KiB</td></tr>
<tr><td align="right">1154</td><td><code>binwalk</code></td><td>2.4.3+dfsg1-2build1</td><td>tool library for analyzing binary blobs and executable code</td><td align="right">16 KiB</td></tr>
<tr><td align="right">1155</td><td><code>libkf6bluezqt-data</code></td><td>6.24.0-0ubuntu1</td><td>data files for bluez-qt</td><td align="right">15 KiB</td></tr>
<tr><td align="right">1156</td><td><code>signon-ui-service</code></td><td>0.17+git20231016.eef943f-3build1</td><td>D-Bus service file for signon-ui</td><td align="right">15 KiB</td></tr>
<tr style="font-style:italic;color:#666"><td align="right">1157</td><td><code>kubuntu-desktop</code><span style="font-size:0.7em;font-weight:normal;background:#e0e0e0;color:#555;border-radius:3px;padding:1px 4px;margin-left:5px;font-style:normal;vertical-align:middle">meta</span></td><td>1.496</td><td>Kubuntu Plasma Desktop/Netbook system</td><td align="right">13 KiB</td></tr>
<tr><td align="right">1158</td><td><code>language-pack-kde-en</code></td><td>1:24.04.0ubuntu1</td><td>KDE translation meta package for language en_GB</td><td align="right">8 KiB</td></tr>
<tr><td></td><td><strong>Ubuntu universe total (1,158)</strong></td><td></td><td></td><td align="right"><strong>4.17 GiB</strong></td></tr>
</tbody></table>

</details>

<details>
<summary><strong>Ubuntu main</strong> — 1,407 packages · 3 meta · 3.46 GiB</summary>

<table>
<thead><tr><th align="right">#</th><th align="left">Package</th><th align="left">Version</th><th align="left">Description</th><th align="right">Installed Size</th></tr></thead>
<tbody>
<tr><td align="right">1</td><td><code>linux-modules-7.0.0-22-generic</code></td><td>7.0.0-22.22</td><td>Linux kernel modules for version 7.0.0</td><td align="right">159.6 MiB</td></tr>
<tr><td align="right">2</td><td><code>linux-firmware-qualcomm-misc</code></td><td>20260319.git217ca6e4-0ubuntu2</td><td>Firmware for miscellaneous Qualcomm devices</td><td align="right">159.5 MiB</td></tr>
<tr><td align="right">3</td><td><code>ibus-data</code></td><td>1.5.34~rc2-1</td><td>Intelligent Input Bus - data files</td><td align="right">144.2 MiB</td></tr>
<tr><td align="right">4</td><td><code>snapd</code></td><td>2.75.2+ubuntu26.04.2</td><td>Daemon and tooling that enable snap packages</td><td align="right">140.6 MiB</td></tr>
<tr><td align="right">5</td><td><code>libllvm21</code></td><td>1:21.1.8-6ubuntu1</td><td>Modular compiler and toolchain technologies, runtime library</td><td align="right">132.2 MiB</td></tr>
<tr><td align="right">6</td><td><code>mesa-vulkan-drivers</code></td><td>26.0.3-1ubuntu1</td><td>Mesa Vulkan graphics drivers</td><td align="right">107.3 MiB</td></tr>
<tr><td align="right">7</td><td><code>linux-firmware-nvidia-graphics</code></td><td>20260319.git217ca6e4-0ubuntu1</td><td>Firmware for Nvidia graphics</td><td align="right">103.7 MiB</td></tr>
<tr><td align="right">8</td><td><code>linux-firmware-intel-wireless</code></td><td>20260319.git217ca6e4-0ubuntu2</td><td>Firmware for Intel Wi-Fi and Bluetooth adapters</td><td align="right">97.3 MiB</td></tr>
<tr><td align="right">9</td><td><code>fonts-noto-cjk</code></td><td>1:20240730+repack1-1build1</td><td>&quot;No Tofu&quot; font families with large Unicode coverage (CJK regular and bold)</td><td align="right">89.1 MiB</td></tr>
<tr><td align="right">10</td><td><code>gcc-15-x86-64-linux-gnu</code></td><td>15.2.0-16ubuntu1</td><td>GNU C compiler for the x86_64-linux-gnu architecture</td><td align="right">74.0 MiB</td></tr>
<tr><td align="right">11</td><td><code>linux-firmware-mellanox-spectrum</code></td><td>20260319.git217ca6e4-0ubuntu1</td><td>Firmware for Mellanox Spectrum switches</td><td align="right">59.0 MiB</td></tr>
<tr><td align="right">12</td><td><code>libclang-cpp21</code></td><td>1:21.1.8-6ubuntu1</td><td>C++ interface to the Clang library</td><td align="right">57.6 MiB</td></tr>
<tr><td align="right">13</td><td><code>libicu-dev</code></td><td>78.2-2ubuntu1</td><td>Development files for International Components for Unicode</td><td align="right">49.2 MiB</td></tr>
<tr><td align="right">14</td><td><code>linux-firmware-qualcomm-wireless</code></td><td>20260319.git217ca6e4-0ubuntu1</td><td>Firmware for Qualcomm and Atheros Wi-Fi and Bluetooth adapters</td><td align="right">46.5 MiB</td></tr>
<tr><td align="right">15</td><td><code>mesa-libgallium</code></td><td>26.0.3-1ubuntu1</td><td>shared infrastructure for Mesa drivers</td><td align="right">43.6 MiB</td></tr>
<tr><td align="right">16</td><td><code>firmware-sof-signed</code></td><td>2025.12.2-1</td><td>Intel SOF firmware - signed</td><td align="right">43.2 MiB</td></tr>
<tr><td align="right">17</td><td><code>fonts-noto-core</code></td><td>20201225-2build1</td><td>&quot;No Tofu&quot; font families with large Unicode coverage (core)</td><td align="right">41.6 MiB</td></tr>
<tr><td align="right">18</td><td><code>g++-15-x86-64-linux-gnu</code></td><td>15.2.0-16ubuntu1</td><td>GNU C++ compiler for the x86_64-linux-gnu architecture</td><td align="right">39.9 MiB</td></tr>
<tr><td align="right">19</td><td><code>libicu78</code></td><td>78.2-2ubuntu1</td><td>International Components for Unicode</td><td align="right">37.8 MiB</td></tr>
<tr><td align="right">20</td><td><code>vim-runtime</code></td><td>2:9.1.2141-1ubuntu4.2</td><td>Vi IMproved - Runtime files</td><td align="right">37.2 MiB</td></tr>
<tr><td align="right">21</td><td><code>cpp-15-x86-64-linux-gnu</code></td><td>15.2.0-16ubuntu1</td><td>GNU C preprocessor for x86_64-linux-gnu</td><td align="right">37.0 MiB</td></tr>
<tr><td align="right">22</td><td><code>linux-firmware-marvell-prestera</code></td><td>20260319.git217ca6e4-0ubuntu1</td><td>Firmware for Marvell Prestera ASIC devices</td><td align="right">31.4 MiB</td></tr>
<tr><td align="right">23</td><td><code>libclang1-21</code></td><td>1:21.1.8-6ubuntu1</td><td>C interface to the Clang library</td><td align="right">31.2 MiB</td></tr>
<tr><td align="right">24</td><td><code>linux-firmware-misc</code></td><td>20260319.git217ca6e4-0ubuntu2</td><td>Firmware for miscellaneous devices and adapters</td><td align="right">30.4 MiB</td></tr>
<tr><td align="right">25</td><td><code>speech-dispatcher</code></td><td>0.12.1-2ubuntu1</td><td>Common interface to speech synthesizers</td><td align="right">28.9 MiB</td></tr>
<tr><td align="right">26</td><td><code>libperl5.40</code></td><td>5.40.1-7build1</td><td>shared Perl library</td><td align="right">28.6 MiB</td></tr>
<tr><td align="right">27</td><td><code>linux-firmware-amd-graphics</code></td><td>20260319.git217ca6e4-0ubuntu3</td><td>Firmware for AMD/ATI graphics</td><td align="right">28.1 MiB</td></tr>
<tr><td align="right">28</td><td><code>linux-firmware-mediatek</code></td><td>20260319.git217ca6e4-0ubuntu1</td><td>Firmware for Mediatek Wi-Fi, Bluetooth, and Ethernet adapters and SoCs</td><td align="right">26.5 MiB</td></tr>
<tr><td align="right">29</td><td><code>python3-numpy</code></td><td>1:2.3.5+ds-3ubuntu1</td><td>Python library for numerical computations (Python 3)</td><td align="right">26.2 MiB</td></tr>
<tr><td align="right">30</td><td><code>libgs10</code></td><td>10.06.0~dfsg-3ubuntu1</td><td>interpreter for the PostScript language and for PDF - Library</td><td align="right">25.7 MiB</td></tr>
<tr><td align="right">31</td><td><code>samba-libs</code></td><td>2:4.23.6+dfsg-1ubuntu2.1</td><td>Samba core libraries</td><td align="right">25.3 MiB</td></tr>
<tr><td align="right">32</td><td><code>libstdc++-15-dev</code></td><td>15.2.0-16ubuntu1</td><td>GNU Standard C++ Library v3 (development files)</td><td align="right">24.9 MiB</td></tr>
<tr><td align="right">33</td><td><code>git</code></td><td>1:2.53.0-1ubuntu1</td><td>fast, scalable, distributed revision control system</td><td align="right">24.7 MiB</td></tr>
<tr><td align="right">34</td><td><code>espeak-ng-data</code></td><td>1.52.0+dfsg-5build1</td><td>Multi-lingual software speech synthesizer: speech data files</td><td align="right">23.5 MiB</td></tr>
<tr><td align="right">35</td><td><code>iso-codes</code></td><td>4.20.1-1</td><td>ISO language, territory, currency, script codes and their translations</td><td align="right">23.5 MiB</td></tr>
<tr><td align="right">36</td><td><code>linux-firmware-intel-graphics</code></td><td>20260319.git217ca6e4-0ubuntu2</td><td>Firmware for Intel graphics and IPU and VSC processors</td><td align="right">20.9 MiB</td></tr>
<tr><td align="right">37</td><td><code>intel-microcode</code></td><td>3.20260210.1ubuntu2</td><td>Processor microcode firmware for Intel CPUs</td><td align="right">20.5 MiB</td></tr>
<tr><td align="right">38</td><td><code>python3-samba</code></td><td>2:4.23.6+dfsg-1ubuntu2.1</td><td>Python 3 bindings for Samba</td><td align="right">19.5 MiB</td></tr>
<tr><td align="right">39</td><td><code>perl-modules-5.40</code></td><td>5.40.1-7build1</td><td>Core Perl modules</td><td align="right">19.5 MiB</td></tr>
<tr><td align="right">40</td><td><code>fwupd</code></td><td>2.1.1-1ubuntu3</td><td>Firmware update daemon</td><td align="right">18.1 MiB</td></tr>
<tr><td align="right">41</td><td><code>libgcc-15-dev</code></td><td>15.2.0-16ubuntu1</td><td>GCC support library (development files)</td><td align="right">17.8 MiB</td></tr>
<tr><td align="right">42</td><td><code>linux-image-7.0.0-22-generic</code></td><td>7.0.0-22.22</td><td>Signed kernel image generic</td><td align="right">16.5 MiB</td></tr>
<tr><td align="right">43</td><td><code>grub2-common</code></td><td>2.14-2ubuntu2</td><td>GRand Unified Bootloader (common files for version 2)</td><td align="right">16.1 MiB</td></tr>
<tr><td align="right">44</td><td><code>locales</code></td><td>2.43-2ubuntu2</td><td>GNU C Library: National Language (locale) data [support]</td><td align="right">15.4 MiB</td></tr>
<tr><td align="right">45</td><td><code>rust-coreutils</code></td><td>0.8.0-0ubuntu3</td><td>Universal coreutils utils, written in Rust</td><td align="right">15.3 MiB</td></tr>
<tr><td align="right">46</td><td><code>fonts-urw-base35</code></td><td>20200910-8build1</td><td>font set metric-compatible with the 35 PostScript Level 2 Base Fonts</td><td align="right">15.2 MiB</td></tr>
<tr><td align="right">47</td><td><code>libgtk-3-dev</code></td><td>3.24.52-0ubuntu1</td><td>development files for the GTK library</td><td align="right">13.8 MiB</td></tr>
<tr><td align="right">48</td><td><code>libc6-dev</code></td><td>2.43-2ubuntu2</td><td>GNU C Library: Development Libraries and Header Files</td><td align="right">13.5 MiB</td></tr>
<tr><td align="right">49</td><td><code>adwaita-icon-theme</code></td><td>50.0-1</td><td>default icon theme of GNOME</td><td align="right">13.2 MiB</td></tr>
<tr><td align="right">50</td><td><code>liblouis-data</code></td><td>3.36.0-1</td><td>Braille translation library - data</td><td align="right">13.0 MiB</td></tr>
<tr><td align="right">51</td><td><code>glycin-loaders</code></td><td>2.1.1+ds-0ubuntu1</td><td>sandboxed image loaders for GNOME</td><td align="right">13.0 MiB</td></tr>
<tr><td align="right">52</td><td><code>poppler-data</code></td><td>0.4.12-1build1</td><td>encoding data for the poppler PDF rendering library</td><td align="right">12.8 MiB</td></tr>
<tr><td align="right">53</td><td><code>libsane1</code></td><td>1.4.0-1ubuntu1</td><td>API library for scanners</td><td align="right">12.6 MiB</td></tr>
<tr><td align="right">54</td><td><code>memtest86+</code></td><td>8.00-3</td><td>stand-alone memory tester for x86 and x86-64</td><td align="right">12.2 MiB</td></tr>
<tr><td align="right">55</td><td><code>libgtk-4-bin</code></td><td>4.22.2+ds-1ubuntu1</td><td>programs for the GTK graphical user interface library</td><td align="right">12.1 MiB</td></tr>
<tr><td align="right">56</td><td><code>libgtk-4-1</code></td><td>4.22.2+ds-1ubuntu1</td><td>GTK graphical user interface library</td><td align="right">11.8 MiB</td></tr>
<tr><td align="right">57</td><td><code>gdb</code></td><td>17.1-2ubuntu1</td><td>GNU Debugger</td><td align="right">11.5 MiB</td></tr>
<tr><td align="right">58</td><td><code>gir1.2-glib-2.0-dev</code></td><td>2.88.0-1</td><td>GIR XML for GLib, GObject, Gio and GModule</td><td align="right">11.3 MiB</td></tr>
<tr><td align="right">59</td><td><code>libasan8</code></td><td>16-20260322-1ubuntu1</td><td>AddressSanitizer -- a fast memory error detector</td><td align="right">11.2 MiB</td></tr>
<tr><td align="right">60</td><td><code>libdate-manip-perl</code></td><td>6.98-1</td><td>module for manipulating dates</td><td align="right">11.1 MiB</td></tr>
<tr><td align="right">61</td><td><code>systemd</code></td><td>259.5-0ubuntu3</td><td>system and service manager</td><td align="right">10.7 MiB</td></tr>
<tr><td align="right">62</td><td><code>linux-firmware-broadcom-wireless</code></td><td>20260319.git217ca6e4-0ubuntu1</td><td>Firmware for Broadcom and Cypress Wi-Fi and Bluetooth adapters</td><td align="right">10.7 MiB</td></tr>
<tr><td align="right">63</td><td><code>libgio-2.0-dev</code></td><td>2.88.0-1</td><td>Development files for the GLib, GObject and GIO libraries</td><td align="right">10.6 MiB</td></tr>
<tr><td align="right">64</td><td><code>fonts-noto-color-emoji</code></td><td>2.051-1build1</td><td>color emoji font from Google</td><td align="right">10.5 MiB</td></tr>
<tr><td align="right">65</td><td><code>linux-firmware-qualcomm-graphics</code></td><td>20260319.git217ca6e4-0ubuntu1</td><td>Firmware for Qualcomm graphics and video processors</td><td align="right">10.5 MiB</td></tr>
<tr><td align="right">66</td><td><code>hplip-data</code></td><td>3.24.4+dfsg0-0ubuntu8</td><td>HP Linux Printing and Imaging - data files</td><td align="right">10.1 MiB</td></tr>
<tr><td align="right">67</td><td><code>udev</code></td><td>259.5-0ubuntu3</td><td>/dev/ and hotplug management daemon</td><td align="right">10.1 MiB</td></tr>
<tr><td align="right">68</td><td><code>libmagic-mgc</code></td><td>1:5.46-5build2</td><td>File type determination library using &quot;magic&quot; numbers (compiled magic file)</td><td align="right">9.9 MiB</td></tr>
<tr><td align="right">69</td><td><code>libpython3.14-stdlib</code></td><td>3.14.4-1</td><td>Interactive high-level object-oriented language (standard library, version 3.14)</td><td align="right">9.9 MiB</td></tr>
<tr><td align="right">70</td><td><code>linux-firmware-intel-misc</code></td><td>20260319.git217ca6e4-0ubuntu1</td><td>Firmware for miscellaneous Intel devices and adapters</td><td align="right">9.3 MiB</td></tr>
<tr><td align="right">71</td><td><code>libtsan2</code></td><td>16-20260322-1ubuntu1</td><td>ThreadSanitizer -- a Valgrind-based detector of data races (runtime)</td><td align="right">9.1 MiB</td></tr>
<tr><td align="right">72</td><td><code>libgtk-3-0t64</code></td><td>3.24.52-0ubuntu1</td><td>GTK graphical user interface library</td><td align="right">8.9 MiB</td></tr>
<tr><td align="right">73</td><td><code>libc6-dbg</code></td><td>2.43-2ubuntu2</td><td>GNU C Library: detached debugging symbols</td><td align="right">8.7 MiB</td></tr>
<tr><td align="right">74</td><td><code>openprinting-ppds</code></td><td>20250819-1build1</td><td>OpenPrinting printer support - PostScript PPD files</td><td align="right">8.0 MiB</td></tr>
<tr><td align="right">75</td><td><code>libpython3.14</code></td><td>3.14.4-1</td><td>Shared Python runtime library (version 3.14)</td><td align="right">7.9 MiB</td></tr>
<tr><td align="right">76</td><td><code>libc-gconv-modules-extra</code></td><td>2.43-2ubuntu2</td><td>GNU C Library: Non-essential gconv modules</td><td align="right">7.9 MiB</td></tr>
<tr><td align="right">77</td><td><code>libssl3t64</code></td><td>3.5.5-1ubuntu3</td><td>Secure Sockets Layer toolkit - shared libraries</td><td align="right">7.8 MiB</td></tr>
<tr><td align="right">78</td><td><code>perl-base</code></td><td>5.40.1-7build1</td><td>minimal Perl system</td><td align="right">7.8 MiB</td></tr>
<tr><td align="right">79</td><td><code>linux-libc-dev</code></td><td>7.0.0-22.22</td><td>Linux Kernel Headers for development</td><td align="right">7.6 MiB</td></tr>
<tr><td align="right">80</td><td><code>network-manager</code></td><td>1.54.3-2ubuntu3</td><td>network management framework (daemon and userspace tools)</td><td align="right">7.4 MiB</td></tr>
<tr><td align="right">81</td><td><code>gstreamer1.0-plugins-good</code></td><td>1.28.2-2</td><td>GStreamer plugins from the &quot;good&quot; set</td><td align="right">7.4 MiB</td></tr>
<tr><td align="right">82</td><td><code>python3.14-minimal</code></td><td>3.14.4-1</td><td>Minimal subset of the Python language (version 3.14)</td><td align="right">7.2 MiB</td></tr>
<tr><td align="right">83</td><td><code>libsystemd-shared</code></td><td>259.5-0ubuntu3</td><td>systemd shared private library</td><td align="right">7.2 MiB</td></tr>
<tr><td align="right">84</td><td><code>fonts-droid-fallback</code></td><td>1:8.1.0r7-1~1.gbp36536bbuild1</td><td>handheld device font with extensive style and language support (fallback)</td><td align="right">7.2 MiB</td></tr>
<tr><td align="right">85</td><td><code>liblapack3</code></td><td>3.12.1-7ubuntu1</td><td>Library of linear algebra routines 3 - shared version</td><td align="right">7.1 MiB</td></tr>
<tr><td align="right">86</td><td><code>console-setup-linux</code></td><td>1.237ubuntu3</td><td>Linux specific part of console-setup</td><td align="right">7.1 MiB</td></tr>
<tr><td align="right">87</td><td><code>linux-firmware-marvell-wireless</code></td><td>20260319.git217ca6e4-0ubuntu1</td><td>Firmware for Marvell and NXP Wi-Fi adapters</td><td align="right">7.1 MiB</td></tr>
<tr><td align="right">88</td><td><code>xfonts-base</code></td><td>1:1.0.5+nmu1build1</td><td>standard fonts for X</td><td align="right">7.0 MiB</td></tr>
<tr><td align="right">89</td><td><code>linux-firmware-realtek</code></td><td>20260319.git217ca6e4-0ubuntu1</td><td>Firmware for Realtek Wi-Fi, Bluetooth, Ethernet and audio adapters</td><td align="right">6.9 MiB</td></tr>
<tr><td align="right">90</td><td><code>linux-firmware-qlogic</code></td><td>20260319.git217ca6e4-0ubuntu1</td><td>Firmware for QLogic SCSI, FC, and IB host bus and Ethernet adapters</td><td align="right">6.8 MiB</td></tr>
<tr><td align="right">91</td><td><code>gnu-coreutils</code></td><td>9.7-3ubuntu2</td><td>GNU core utilities</td><td align="right">6.8 MiB</td></tr>
<tr><td align="right">92</td><td><code>ipp-usb</code></td><td>0.9.31-1</td><td>Daemon for IPP over USB printer support</td><td align="right">6.7 MiB</td></tr>
<tr><td align="right">93</td><td><code>fonts-dejavu-extra</code></td><td>2.37-8build1</td><td>Vera font family derivate with additional characters (extra variants)</td><td align="right">6.6 MiB</td></tr>
<tr><td align="right">94</td><td><code>libmysqlclient24</code></td><td>8.4.8-0ubuntu1</td><td>MySQL database client library</td><td align="right">6.6 MiB</td></tr>
<tr><td align="right">95</td><td><code>libgtk-4-common</code></td><td>4.22.2+ds-1ubuntu1</td><td>common files for the GTK graphical user interface library</td><td align="right">6.5 MiB</td></tr>
<tr><td align="right">96</td><td><code>dpkg</code></td><td>1.23.7ubuntu1</td><td>Debian package management system</td><td align="right">6.1 MiB</td></tr>
<tr><td align="right">97</td><td><code>orca</code></td><td>50.1.2-1ubuntu1</td><td>Scriptable screen reader</td><td align="right">6.0 MiB</td></tr>
<tr><td align="right">98</td><td><code>binutils-x86-64-linux-gnu</code></td><td>2.46-3ubuntu2</td><td>GNU binary utilities, for x86-64-linux-gnu target</td><td align="right">6.0 MiB</td></tr>
<tr><td align="right">99</td><td><code>libgutenprint-common</code></td><td>5.3.4.20220624T01008808d602-4ubuntu2</td><td>support files for the Gutenprint printer driver library</td><td align="right">5.9 MiB</td></tr>
<tr><td align="right">100</td><td><code>libc6</code></td><td>2.43-2ubuntu2</td><td>GNU C Library: Shared libraries</td><td align="right">5.7 MiB</td></tr>
<tr><td align="right">101</td><td><code>libhwy1t64</code></td><td>1.3.0-2</td><td>Efficient and performance-portable SIMD wrapper (runtime files)</td><td align="right">5.6 MiB</td></tr>
<tr><td align="right">102</td><td><code>libsystemd-dev</code></td><td>259.5-0ubuntu3</td><td>systemd utility library - development files</td><td align="right">5.6 MiB</td></tr>
<tr><td align="right">103</td><td><code>librsvg2-2</code></td><td>2.61.3+dfsg-3</td><td>SAX-based renderer library for SVG files (runtime)</td><td align="right">5.6 MiB</td></tr>
<tr><td align="right">104</td><td><code>python3-lxml</code></td><td>6.0.2-1build1</td><td>pythonic binding for the libxml2 and libxslt libraries</td><td align="right">5.5 MiB</td></tr>
<tr><td align="right">105</td><td><code>libpython3.14-minimal</code></td><td>3.14.4-1</td><td>Minimal subset of the Python language (version 3.14)</td><td align="right">5.5 MiB</td></tr>
<tr><td align="right">106</td><td><code>linux-firmware-netronome</code></td><td>20260319.git217ca6e4-0ubuntu1</td><td>Firmware for Netronome Ethernet adapters</td><td align="right">5.5 MiB</td></tr>
<tr><td align="right">107</td><td><code>libaom3</code></td><td>3.13.1-2</td><td>AV1 Video Codec Library</td><td align="right">5.3 MiB</td></tr>
<tr><td align="right">108</td><td><code>samba-common-bin</code></td><td>2:4.23.6+dfsg-1ubuntu2.1</td><td>Samba common files used by both the server and the client</td><td align="right">5.1 MiB</td></tr>
<tr><td align="right">109</td><td><code>libhwasan0</code></td><td>16-20260322-1ubuntu1</td><td>AddressSanitizer -- a fast memory error detector</td><td align="right">5.0 MiB</td></tr>
<tr><td align="right">110</td><td><code>modemmanager</code></td><td>1.25.95-1ubuntu1</td><td>D-Bus service for managing modems</td><td align="right">5.0 MiB</td></tr>
<tr><td align="right">111</td><td><code>btrfs-progs</code></td><td>6.17.1-1build1</td><td>Checksumming Copy on Write Filesystem utilities</td><td align="right">5.0 MiB</td></tr>
<tr><td align="right">112</td><td><code>fonts-noto-ui-core</code></td><td>20201225-2build1</td><td>&quot;No Tofu&quot; font families with large Unicode coverage (UI core)</td><td align="right">4.9 MiB</td></tr>
<tr><td align="right">113</td><td><code>libgtkmm-3.0-1t64</code></td><td>3.24.10-2</td><td>C++ wrappers for GTK+ (shared libraries)</td><td align="right">4.8 MiB</td></tr>
<tr><td align="right">114</td><td><code>libopus-dev</code></td><td>1.6.1-1</td><td>Opus codec library development files</td><td align="right">4.8 MiB</td></tr>
<tr><td align="right">115</td><td><code>bluez</code></td><td>5.85-4</td><td>Bluetooth tools and daemons</td><td align="right">4.8 MiB</td></tr>
<tr><td align="right">116</td><td><code>libsane-common</code></td><td>1.4.0-1ubuntu1</td><td>API library for scanners -- documentation and support files</td><td align="right">4.8 MiB</td></tr>
<tr><td align="right">117</td><td><code>libqmi-glib5</code></td><td>1.38.0-1</td><td>Support library to use the Qualcomm MSM Interface (QMI) protocol</td><td align="right">4.7 MiB</td></tr>
<tr><td align="right">118</td><td><code>python3-pygments</code></td><td>2.19.2+dfsg-1</td><td>syntax highlighting package written in Python 3</td><td align="right">4.5 MiB</td></tr>
<tr><td align="right">119</td><td><code>vim</code></td><td>2:9.1.2141-1ubuntu4.2</td><td>Vi IMproved - enhanced vi editor</td><td align="right">4.5 MiB</td></tr>
<tr><td align="right">120</td><td><code>apt</code></td><td>3.2.0</td><td>commandline package manager</td><td align="right">4.3 MiB</td></tr>
<tr><td align="right">121</td><td><code>libgtk-3-common</code></td><td>3.24.52-0ubuntu1</td><td>common files for the GTK graphical user interface library</td><td align="right">4.2 MiB</td></tr>
<tr><td align="right">122</td><td><code>passwd</code></td><td>1:4.17.4-2ubuntu3</td><td>change and administer password and group data</td><td align="right">4.2 MiB</td></tr>
<tr><td align="right">123</td><td><code>python3-cryptography</code></td><td>46.0.5-1ubuntu2</td><td>Python library exposing cryptographic recipes and primitives</td><td align="right">4.2 MiB</td></tr>
<tr><td align="right">124</td><td><code>libglib2.0-0t64</code></td><td>2.88.0-1</td><td>GLib library of C routines</td><td align="right">4.2 MiB</td></tr>
<tr><td align="right">125</td><td><code>fonts-ubuntu</code></td><td>0.869+git20240321-0ubuntu2</td><td>sans-serif font set from Ubuntu</td><td align="right">4.2 MiB</td></tr>
<tr><td align="right">126</td><td><code>cups-common</code></td><td>2.4.16-1ubuntu1</td><td>Common UNIX Printing System(tm) - common files</td><td align="right">4.2 MiB</td></tr>
<tr><td align="right">127</td><td><code>fonts-liberation</code></td><td>1:2.1.5-3build1</td><td>fonts with the same metrics as Times, Arial and Courier</td><td align="right">4.2 MiB</td></tr>
<tr><td align="right">128</td><td><code>libnss3</code></td><td>2:3.120-1ubuntu2</td><td>Network Security Service libraries</td><td align="right">4.2 MiB</td></tr>
<tr><td align="right">129</td><td><code>libpipewire-0.3-modules</code></td><td>1.6.2-1ubuntu1</td><td>libraries for the PipeWire multimedia server - modules</td><td align="right">4.1 MiB</td></tr>
<tr><td align="right">130</td><td><code>libopus0</code></td><td>1.6.1-1</td><td>Opus codec runtime library</td><td align="right">4.1 MiB</td></tr>
<tr><td align="right">131</td><td><code>manpages-dev</code></td><td>6.17-1</td><td>Manual pages about using GNU/Linux for development</td><td align="right">4.0 MiB</td></tr>
<tr><td align="right">132</td><td><code>xkb-data</code></td><td>2.46-2</td><td>X Keyboard Extension (XKB) configuration data</td><td align="right">4.0 MiB</td></tr>
<tr><td align="right">133</td><td><code>libtcl8.6</code></td><td>8.6.17+dfsg-1build1</td><td>Tcl (the Tool Command Language) v8.6 - run-time library files</td><td align="right">4.0 MiB</td></tr>
<tr><td align="right">134</td><td><code>libjxl0.11</code></td><td>0.11.1-6ubuntu4</td><td>JPEG XL Image Coding System - &quot;JXL&quot; (shared libraries)</td><td align="right">4.0 MiB</td></tr>
<tr><td align="right">135</td><td><code>libglycin-2-0</code></td><td>2.1.1+ds-0ubuntu1</td><td>sandboxed image loaders for GNOME</td><td align="right">4.0 MiB</td></tr>
<tr><td align="right">136</td><td><code>libgprofng0</code></td><td>2.46-3ubuntu2</td><td>GNU Next Generation profiler (runtime library)</td><td align="right">4.0 MiB</td></tr>
<tr><td align="right">137</td><td><code>liblsan0</code></td><td>16-20260322-1ubuntu1</td><td>LeakSanitizer -- a memory leak detector (runtime)</td><td align="right">3.9 MiB</td></tr>
<tr><td align="right">138</td><td><code>wpasupplicant</code></td><td>2:2.11-0ubuntu5</td><td>client support for WPA and WPA2 (IEEE 802.11i)</td><td align="right">3.9 MiB</td></tr>
<tr><td align="right">139</td><td><code>libpoppler156</code></td><td>26.01.0-2build2</td><td>PDF rendering library</td><td align="right">3.9 MiB</td></tr>
<tr><td align="right">140</td><td><code>libspa-0.2-modules</code></td><td>1.6.2-1ubuntu1</td><td>libraries for the PipeWire multimedia server Simple Plugin API - modules</td><td align="right">3.9 MiB</td></tr>
<tr><td align="right">141</td><td><code>python3-setuptools</code></td><td>78.1.1-0.1build1</td><td>Python3 Distutils Enhancements</td><td align="right">3.8 MiB</td></tr>
<tr><td align="right">142</td><td><code>language-pack-en-base</code></td><td>1:26.04+20260417</td><td>translations for language English</td><td align="right">3.8 MiB</td></tr>
<tr><td align="right">143</td><td><code>libvpx12</code></td><td>1.16.0-3</td><td>VP8 and VP9 video codec (shared library)</td><td align="right">3.7 MiB</td></tr>
<tr><td align="right">144</td><td><code>grub-pc-bin</code></td><td>2.14-2ubuntu2</td><td>GRand Unified Bootloader, version 2 (PC/BIOS modules)</td><td align="right">3.7 MiB</td></tr>
<tr><td align="right">145</td><td><code>groff-base</code></td><td>1.23.0-10</td><td>GNU troff text-formatting system (base system components)</td><td align="right">3.6 MiB</td></tr>
<tr><td align="right">146</td><td><code>libsnmp40t64</code></td><td>5.9.4+dfsg-2ubuntu3</td><td>SNMP (Simple Network Management Protocol) library</td><td align="right">3.6 MiB</td></tr>
<tr><td align="right">147</td><td><code>libapt-pkg7.0</code></td><td>3.2.0</td><td>package management runtime library</td><td align="right">3.6 MiB</td></tr>
<tr><td align="right">148</td><td><code>openssh-client</code></td><td>1:10.2p1-2ubuntu3.2</td><td>secure shell (SSH) client, for secure access to remote machines</td><td align="right">3.6 MiB</td></tr>
<tr><td align="right">149</td><td><code>libexiv2-28</code></td><td>0.28.8+dfsg-1</td><td>EXIF/IPTC/XMP metadata manipulation library</td><td align="right">3.6 MiB</td></tr>
<tr><td align="right">150</td><td><code>libgfortran5</code></td><td>16-20260322-1ubuntu1</td><td>Runtime library for GNU Fortran applications</td><td align="right">3.6 MiB</td></tr>
<tr><td align="right">151</td><td><code>libgstreamer1.0-0</code></td><td>1.28.2-1</td><td>Core GStreamer libraries and elements</td><td align="right">3.5 MiB</td></tr>
<tr><td align="right">152</td><td><code>libubsan1</code></td><td>16-20260322-1ubuntu1</td><td>UBSan -- undefined behaviour sanitizer (runtime)</td><td align="right">3.4 MiB</td></tr>
<tr><td align="right">153</td><td><code>sudo</code></td><td>1.9.17p2-1ubuntu3</td><td>Provide limited super user privileges to specific users</td><td align="right">3.4 MiB</td></tr>
<tr><td align="right">154</td><td><code>iproute2</code></td><td>6.19.0-1ubuntu1</td><td>networking and traffic control tools</td><td align="right">3.2 MiB</td></tr>
<tr><td align="right">155</td><td><code>libstdc++6</code></td><td>16-20260322-1ubuntu1</td><td>GNU Standard C++ Library v3</td><td align="right">3.2 MiB</td></tr>
<tr><td align="right">156</td><td><code>elfutils</code></td><td>0.194-4</td><td>collection of utilities to handle ELF objects</td><td align="right">3.1 MiB</td></tr>
<tr><td align="right">157</td><td><code>libprotobuf32t64</code></td><td>3.21.12-15ubuntu1</td><td>protocol buffers C++ library</td><td align="right">3.0 MiB</td></tr>
<tr><td align="right">158</td><td><code>man-db</code></td><td>2.13.1-1build1</td><td>tools for reading manual pages</td><td align="right">3.0 MiB</td></tr>
<tr><td align="right">159</td><td><code>libgstreamer-plugins-base1.0-0</code></td><td>1.28.2-1</td><td>GStreamer libraries from the &quot;base&quot; set</td><td align="right">2.9 MiB</td></tr>
<tr><td align="right">160</td><td><code>liblcms2-dev</code></td><td>2.17-1ubuntu0.2</td><td>Little CMS 2 color management library development headers</td><td align="right">2.9 MiB</td></tr>
<tr><td align="right">161</td><td><code>util-linux</code></td><td>2.41.3-3ubuntu2</td><td>miscellaneous system utilities</td><td align="right">2.8 MiB</td></tr>
<tr><td align="right">162</td><td><code>shared-mime-info</code></td><td>2.4-5build3</td><td>FreeDesktop.org shared MIME database and spec</td><td align="right">2.8 MiB</td></tr>
<tr><td align="right">163</td><td><code>libibus-1.0-dev</code></td><td>1.5.34~rc2-1</td><td>Intelligent Input Bus - development file</td><td align="right">2.8 MiB</td></tr>
<tr><td align="right">164</td><td><code>openssh-server</code></td><td>1:10.2p1-2ubuntu3.2</td><td>secure shell (SSH) server, for secure access from remote machines</td><td align="right">2.8 MiB</td></tr>
<tr><td align="right">165</td><td><code>libpcre2-dev</code></td><td>10.46-1build1</td><td>New Perl Compatible Regular Expression Library - development files</td><td align="right">2.8 MiB</td></tr>
<tr><td align="right">166</td><td><code>libabsl20260107</code></td><td>20260107.0-4</td><td>extensions to the C++ standard library</td><td align="right">2.7 MiB</td></tr>
<tr><td align="right">167</td><td><code>libgutenprint9</code></td><td>5.3.4.20220624T01008808d602-4ubuntu2</td><td>runtime for the Gutenprint printer driver library</td><td align="right">2.7 MiB</td></tr>
<tr><td align="right">168</td><td><code>alsa-utils</code></td><td>1.2.15.2-1ubuntu1</td><td>Utilities for configuring and using ALSA</td><td align="right">2.7 MiB</td></tr>
<tr><td align="right">169</td><td><code>libgphoto2-6t64</code></td><td>2.5.33-1ubuntu1</td><td>gphoto2 digital camera library</td><td align="right">2.6 MiB</td></tr>
<tr><td align="right">170</td><td><code>libglibmm-2.4-1t64</code></td><td>2.66.8-3</td><td>C++ wrapper for the GLib toolkit (shared libraries)</td><td align="right">2.6 MiB</td></tr>
<tr><td align="right">171</td><td><code>libqpdf30</code></td><td>12.3.2-1</td><td>runtime library for PDF transformation/inspection software</td><td align="right">2.6 MiB</td></tr>
<tr><td align="right">172</td><td><code>colord-data</code></td><td>1.4.8-3</td><td>system service to manage device colour profiles -- data files</td><td align="right">2.5 MiB</td></tr>
<tr><td align="right">173</td><td><code>gstreamer1.0-plugins-base</code></td><td>1.28.2-1</td><td>GStreamer plugins from the &quot;base&quot; set</td><td align="right">2.5 MiB</td></tr>
<tr><td align="right">174</td><td><code>openssl</code></td><td>3.5.5-1ubuntu3</td><td>Secure Sockets Layer toolkit - cryptographic utility</td><td align="right">2.5 MiB</td></tr>
<tr><td align="right">175</td><td><code>printer-driver-foo2zjs-common</code></td><td>20200505dfsg0-3ubuntu1</td><td>printer driver for ZjStream-based printers - common files</td><td align="right">2.4 MiB</td></tr>
<tr><td align="right">176</td><td><code>libx11-dev</code></td><td>2:1.8.13-1</td><td>X11 client-side library (development headers)</td><td align="right">2.4 MiB</td></tr>
<tr><td align="right">177</td><td><code>sudo-rs</code></td><td>0.2.13-0ubuntu1</td><td>Rust-based sudo and su implementations</td><td align="right">2.4 MiB</td></tr>
<tr><td align="right">178</td><td><code>xwayland</code></td><td>2:24.1.10-1</td><td>X server for running X clients under Wayland</td><td align="right">2.4 MiB</td></tr>
<tr><td align="right">179</td><td><code>libgnutls30t64</code></td><td>3.8.12-2ubuntu1.1</td><td>GNU TLS library - main runtime library</td><td align="right">2.4 MiB</td></tr>
<tr><td align="right">180</td><td><code>iptables</code></td><td>1.8.11-2ubuntu3</td><td>administration tools for packet filtering and NAT</td><td align="right">2.4 MiB</td></tr>
<tr><td align="right">181</td><td><code>apparmor</code></td><td>5.0.0~beta1-0ubuntu7</td><td>user-space parser utility for AppArmor</td><td align="right">2.4 MiB</td></tr>
<tr><td align="right">182</td><td><code>libbinutils</code></td><td>2.46-3ubuntu2</td><td>GNU binary utilities (private shared library)</td><td align="right">2.4 MiB</td></tr>
<tr><td align="right">183</td><td><code>libtk8.6</code></td><td>8.6.17-1build1</td><td>Tk toolkit for Tcl and X11 v8.6 - run-time files</td><td align="right">2.3 MiB</td></tr>
<tr><td align="right">184</td><td><code>cups-server-common</code></td><td>2.4.16-1ubuntu1</td><td>Common UNIX Printing System(tm) - server common files</td><td align="right">2.3 MiB</td></tr>
<tr><td align="right">185</td><td><code>libslang2</code></td><td>2.3.3-5build1</td><td>S-Lang programming library - runtime version</td><td align="right">2.3 MiB</td></tr>
<tr><td align="right">186</td><td><code>linux-main-modules-zfs-7.0.0-22-generic</code></td><td>7.0.0-22.22</td><td>Signed zfs module for 7.0.0-22</td><td align="right">2.3 MiB</td></tr>
<tr><td align="right">187</td><td><code>git-man</code></td><td>1:2.53.0-1ubuntu1</td><td>fast, scalable, distributed revision control system (manual pages)</td><td align="right">2.3 MiB</td></tr>
<tr><td align="right">188</td><td><code>libfftw3-single3</code></td><td>3.3.10-2fakesync1build3</td><td>Library for computing Fast Fourier Transforms - Single precision</td><td align="right">2.3 MiB</td></tr>
<tr><td align="right">189</td><td><code>libboost-program-options1.90.0</code></td><td>1.90.0-6ubuntu1</td><td>program options library for C++</td><td align="right">2.3 MiB</td></tr>
<tr><td align="right">190</td><td><code>libraw23t64</code></td><td>0.21.5b-1ubuntu1</td><td>raw image decoder library</td><td align="right">2.3 MiB</td></tr>
<tr><td align="right">191</td><td><code>libboost-python1.90.0</code></td><td>1.90.0-6ubuntu1</td><td>Boost.Python Library</td><td align="right">2.3 MiB</td></tr>
<tr><td align="right">192</td><td><code>fonts-dejavu-core</code></td><td>2.37-8build1</td><td>Vera font family derivate with additional characters</td><td align="right">2.2 MiB</td></tr>
<tr><td align="right">193</td><td><code>libfftw3-double3</code></td><td>3.3.10-2fakesync1build3</td><td>Library for computing Fast Fourier Transforms - Double precision</td><td align="right">2.2 MiB</td></tr>
<tr><td align="right">194</td><td><code>vim-common</code></td><td>2:9.1.2141-1ubuntu4.2</td><td>Vi IMproved - Common files</td><td align="right">2.2 MiB</td></tr>
<tr><td align="right">195</td><td><code>libgs10-common</code></td><td>10.06.0~dfsg-3ubuntu1</td><td>interpreter for the PostScript language and for PDF - common files</td><td align="right">2.2 MiB</td></tr>
<tr><td align="right">196</td><td><code>libc-bin</code></td><td>2.43-2ubuntu2</td><td>GNU C Library: Binaries</td><td align="right">2.2 MiB</td></tr>
<tr><td align="right">197</td><td><code>smartmontools</code></td><td>7.5-2</td><td>control and monitor storage systems using S.M.A.R.T.</td><td align="right">2.2 MiB</td></tr>
<tr><td align="right">198</td><td><code>libboost-filesystem1.90.0</code></td><td>1.90.0-6ubuntu1</td><td>filesystem operations (portable paths, iteration over directories, etc) in C++</td><td align="right">2.2 MiB</td></tr>
<tr><td align="right">199</td><td><code>libboost-thread1.90.0</code></td><td>1.90.0-6ubuntu1</td><td>portable C++ multi-threading</td><td align="right">2.1 MiB</td></tr>
<tr><td align="right">200</td><td><code>libfreerdp3-3</code></td><td>3.24.2+dfsg-1ubuntu1</td><td>Free Remote Desktop Protocol library (core library)</td><td align="right">2.1 MiB</td></tr>
<tr><td align="right">201</td><td><code>pipewire-bin</code></td><td>1.6.2-1ubuntu1</td><td>PipeWire multimedia server - programs</td><td align="right">2.1 MiB</td></tr>
<tr><td align="right">202</td><td><code>libboost-iostreams1.90.0</code></td><td>1.90.0-6ubuntu1</td><td>Boost.Iostreams Library</td><td align="right">2.1 MiB</td></tr>
<tr><td align="right">203</td><td><code>libaspell15</code></td><td>0.60.8.2-3</td><td>GNU Aspell spell-checker runtime library</td><td align="right">2.1 MiB</td></tr>
<tr><td align="right">204</td><td><code>docbook-xml</code></td><td>4.5-13build1</td><td>standard XML documentation system for software and systems</td><td align="right">2.1 MiB</td></tr>
<tr><td align="right">205</td><td><code>libfreetype-dev</code></td><td>2.14.2+dfsg-1</td><td>Freetype 2 font engine, development files</td><td align="right">2.1 MiB</td></tr>
<tr><td align="right">206</td><td><code>printer-driver-hpcups</code></td><td>3.24.4+dfsg0-0ubuntu8</td><td>HP Linux Printing and Imaging - CUPS Raster driver (hpcups)</td><td align="right">2.0 MiB</td></tr>
<tr><td align="right">207</td><td><code>smbclient</code></td><td>2:4.23.6+dfsg-1ubuntu2.1</td><td>command-line SMB/CIFS clients for Unix</td><td align="right">2.0 MiB</td></tr>
<tr><td align="right">208</td><td><code>libunistring5</code></td><td>1.3-2build1</td><td>Unicode string library for C</td><td align="right">2.0 MiB</td></tr>
<tr><td align="right">209</td><td><code>mtd-utils</code></td><td>1:2.3.0-1ubuntu2</td><td>Memory Technology Device Utilities</td><td align="right">2.0 MiB</td></tr>
<tr><td align="right">210</td><td><code>libisl23</code></td><td>0.27-1build1</td><td>manipulating sets and relations of integer points bounded by linear constraints</td><td align="right">2.0 MiB</td></tr>
<tr><td align="right">211</td><td><code>libsdl2-classic</code></td><td>2.32.10+dfsg-6</td><td>Simple DirectMedia Layer - &quot;classic&quot; implementation as non-default</td><td align="right">2.0 MiB</td></tr>
<tr><td align="right">212</td><td><code>bash</code></td><td>5.3-2ubuntu1</td><td>GNU Bourne Again SHell</td><td align="right">2.0 MiB</td></tr>
<tr><td align="right">213</td><td><code>vim-tiny</code></td><td>2:9.1.2141-1ubuntu4.2</td><td>Vi IMproved - enhanced vi editor - compact version</td><td align="right">1.9 MiB</td></tr>
<tr><td align="right">214</td><td><code>python3-pil</code></td><td>12.1.1-2ubuntu1.1</td><td>Python Imaging Library (Python3)</td><td align="right">1.9 MiB</td></tr>
<tr><td align="right">215</td><td><code>bash-completion</code></td><td>1:2.16.0-8build1</td><td>programmable completion for the bash shell</td><td align="right">1.9 MiB</td></tr>
<tr><td align="right">216</td><td><code>libp11-kit0</code></td><td>0.26.2-2</td><td>library for loading and coordinating access to PKCS#11 modules - runtime</td><td align="right">1.9 MiB</td></tr>
<tr><td align="right">217</td><td><code>printer-driver-pnm2ppa</code></td><td>1.13+nondbs-0ubuntu11</td><td>printer driver for HP-GDI printers</td><td align="right">1.9 MiB</td></tr>
<tr><td align="right">218</td><td><code>zstd</code></td><td>1.5.7+dfsg-3</td><td>fast lossless compression algorithm -- CLI tool</td><td align="right">1.9 MiB</td></tr>
<tr><td align="right">219</td><td><code>libwacom-common</code></td><td>2.18.0-1</td><td>Wacom model feature query library (common files)</td><td align="right">1.9 MiB</td></tr>
<tr><td align="right">220</td><td><code>libwebp-dev</code></td><td>1.5.0-0.1build1</td><td>Lossy compression of digital photographic images (development files)</td><td align="right">1.8 MiB</td></tr>
<tr><td align="right">221</td><td><code>remmina-common</code></td><td>1.4.40+dfsg-2ubuntu1</td><td>Common files for Remmina</td><td align="right">1.8 MiB</td></tr>
<tr><td align="right">222</td><td><code>libharfbuzz-dev</code></td><td>12.3.2-2</td><td>Development files for OpenType text shaping engine</td><td align="right">1.8 MiB</td></tr>
<tr><td align="right">223</td><td><code>rsyslog</code></td><td>8.2512.0-1ubuntu4</td><td>reliable system and kernel logging daemon</td><td align="right">1.8 MiB</td></tr>
<tr><td align="right">224</td><td><code>libgcrypt20</code></td><td>1.12.0-2ubuntu0.1</td><td>LGPL Crypto library - runtime library</td><td align="right">1.8 MiB</td></tr>
<tr><td align="right">225</td><td><code>printer-driver-postscript-hp</code></td><td>3.24.4+dfsg0-0ubuntu8</td><td>HP Printers PostScript Descriptions</td><td align="right">1.8 MiB</td></tr>
<tr><td align="right">226</td><td><code>openvpn</code></td><td>2.7.0-1ubuntu1.1</td><td>virtual private network daemon</td><td align="right">1.8 MiB</td></tr>
<tr><td align="right">227</td><td><code>libdb5.3t64</code></td><td>5.3.28+dfsg2-10ubuntu1</td><td>Berkeley v5.3 Database Libraries [runtime]</td><td align="right">1.8 MiB</td></tr>
<tr><td align="right">228</td><td><code>procps</code></td><td>2:4.0.4-9ubuntu1</td><td>/proc file system utilities</td><td align="right">1.8 MiB</td></tr>
<tr><td align="right">229</td><td><code>language-selector-common</code></td><td>0.228build1</td><td>Language selector for Ubuntu</td><td align="right">1.8 MiB</td></tr>
<tr><td align="right">230</td><td><code>libsepol-dev</code></td><td>3.9-2</td><td>SELinux binary policy manipulation library and development files</td><td align="right">1.8 MiB</td></tr>
<tr><td align="right">231</td><td><code>libdpkg-perl</code></td><td>1.23.7ubuntu1</td><td>Dpkg perl modules</td><td align="right">1.7 MiB</td></tr>
<tr><td align="right">232</td><td><code>libsqlite3-0</code></td><td>3.46.1-9</td><td>SQLite 3 shared library</td><td align="right">1.7 MiB</td></tr>
<tr><td align="right">233</td><td><code>packagekit</code></td><td>1.3.4-3ubuntu1</td><td>Provides a package management service</td><td align="right">1.7 MiB</td></tr>
<tr><td align="right">234</td><td><code>python3-launchpadlib</code></td><td>2.1.0-1build1</td><td>Launchpad web services client library (Python 3)</td><td align="right">1.7 MiB</td></tr>
<tr><td align="right">235</td><td><code>libdjvulibre21</code></td><td>3.5.29-1</td><td>Runtime support for the DjVu image format</td><td align="right">1.7 MiB</td></tr>
<tr><td align="right">236</td><td><code>python3-tornado</code></td><td>6.5.4-0.1ubuntu0.1</td><td>scalable, non-blocking web server and tools - Python 3 package</td><td align="right">1.7 MiB</td></tr>
<tr><td align="right">237</td><td><code>socat</code></td><td>1.8.1.1-1</td><td>multipurpose relay for bidirectional data transfer</td><td align="right">1.7 MiB</td></tr>
<tr><td align="right">238</td><td><code>x11proto-dev</code></td><td>2025.1-1</td><td>X11 extension protocols and auxiliary headers</td><td align="right">1.7 MiB</td></tr>
<tr><td align="right">239</td><td><code>libvorbis-dev</code></td><td>1.3.7-3build2</td><td>development files for Vorbis General Audio Compression Codec</td><td align="right">1.7 MiB</td></tr>
<tr><td align="right">240</td><td><code>manpages</code></td><td>6.17-1</td><td>Manual pages about using a GNU/Linux system</td><td align="right">1.6 MiB</td></tr>
<tr><td align="right">241</td><td><code>libheif1</code></td><td>1.21.2-3</td><td>HEIF and AVIF file format decoder and encoder - shared library</td><td align="right">1.6 MiB</td></tr>
<tr><td align="right">242</td><td><code>libepoxy-dev</code></td><td>1.5.10-2build1</td><td>OpenGL function pointer management library- development</td><td align="right">1.6 MiB</td></tr>
<tr><td align="right">243</td><td><code>libsane-hpaio</code></td><td>3.24.4+dfsg0-0ubuntu8</td><td>HP SANE backend for multi-function peripherals</td><td align="right">1.6 MiB</td></tr>
<tr><td align="right">244</td><td><code>libpango1.0-dev</code></td><td>1.57.0-1</td><td>Development files for the Pango</td><td align="right">1.6 MiB</td></tr>
<tr><td align="right">245</td><td><code>libspa-0.2-bluetooth</code></td><td>1.6.2-1ubuntu1</td><td>libraries for the PipeWire multimedia server - bluetooth plugins</td><td align="right">1.5 MiB</td></tr>
<tr><td align="right">246</td><td><code>update-notifier-common</code></td><td>3.207</td><td>Files shared between update-notifier and other packages</td><td align="right">1.5 MiB</td></tr>
<tr><td align="right">247</td><td><code>libnm0</code></td><td>1.54.3-2ubuntu3</td><td>GObject-based client library for NetworkManager</td><td align="right">1.5 MiB</td></tr>
<tr><td align="right">248</td><td><code>hunspell-en-us</code></td><td>1:2020.12.07-4build1</td><td>English_american dictionary for hunspell</td><td align="right">1.5 MiB</td></tr>
<tr><td align="right">249</td><td><code>libcamel-1.2-64t64</code></td><td>3.56.2-8</td><td>Evolution MIME message handling library</td><td align="right">1.5 MiB</td></tr>
<tr><td align="right">250</td><td><code>libxml2-16</code></td><td>2.15.2+dfsg-0.1</td><td>GNOME XML library</td><td align="right">1.5 MiB</td></tr>
<tr><td align="right">251</td><td><code>pci.ids</code></td><td>0.0~2026.02.12-1</td><td>PCI ID Repository</td><td align="right">1.5 MiB</td></tr>
<tr><td align="right">252</td><td><code>xdg-desktop-portal</code></td><td>1.21.1+ds-1ubuntu3</td><td>desktop integration portal for Flatpak and Snap</td><td align="right">1.5 MiB</td></tr>
<tr><td align="right">253</td><td><code>e2fsprogs</code></td><td>1.47.2-3ubuntu4</td><td>ext2/ext3/ext4 file system utilities</td><td align="right">1.5 MiB</td></tr>
<tr><td align="right">254</td><td><code>alsa-ucm-conf</code></td><td>1.2.15.3-1ubuntu1</td><td>ALSA Use Case Manager configuration files</td><td align="right">1.5 MiB</td></tr>
<tr><td align="right">255</td><td><code>libflac-dev</code></td><td>1.5.0+ds-5</td><td>Free Lossless Audio Codec - C/C++ development libraries</td><td align="right">1.5 MiB</td></tr>
<tr><td align="right">256</td><td><code>libgstreamer-plugins-extra1.0-0</code></td><td>1.28.2-1ubuntu1</td><td>GStreamer libraries from the &quot;extra&quot; set</td><td align="right">1.5 MiB</td></tr>
<tr><td align="right">257</td><td><code>libsamplerate0</code></td><td>0.2.2-4build2</td><td>Audio sample rate conversion library</td><td align="right">1.5 MiB</td></tr>
<tr><td align="right">258</td><td><code>libtag2</code></td><td>2.2.1-3</td><td>audio meta-data library</td><td align="right">1.4 MiB</td></tr>
<tr><td align="right">259</td><td><code>printer-driver-m2300w</code></td><td>0.51-15build3</td><td>printer driver for Minolta magicolor 2300W/2400W color laser printers</td><td align="right">1.4 MiB</td></tr>
<tr><td align="right">260</td><td><code>gpg</code></td><td>2.4.8-4ubuntu3</td><td>GNU Privacy Guard -- minimalist public key operations</td><td align="right">1.4 MiB</td></tr>
<tr><td align="right">261</td><td><code>libdrm-dev</code></td><td>2.4.131-1</td><td>Userspace interface to kernel DRM services -- development files</td><td align="right">1.4 MiB</td></tr>
<tr><td align="right">262</td><td><code>libwinpr3-3</code></td><td>3.24.2+dfsg-1ubuntu1</td><td>Windows Portable Runtime library</td><td align="right">1.4 MiB</td></tr>
<tr><td align="right">263</td><td><code>libcairo2</code></td><td>1.18.4-3</td><td>Cairo 2D vector graphics library</td><td align="right">1.4 MiB</td></tr>
<tr><td align="right">264</td><td><code>libgit2-1.9</code></td><td>1.9.1+ds-1ubuntu1</td><td>low-level Git library</td><td align="right">1.4 MiB</td></tr>
<tr><td align="right">265</td><td><code>libx11-6</code></td><td>2:1.8.13-1</td><td>X11 client-side library</td><td align="right">1.3 MiB</td></tr>
<tr><td align="right">266</td><td><code>ibverbs-providers</code></td><td>61.0-2ubuntu3</td><td>User space provider drivers for libibverbs</td><td align="right">1.3 MiB</td></tr>
<tr><td align="right">267</td><td><code>libxt-dev</code></td><td>1:1.2.1-1.3build1</td><td>X11 toolkit intrinsics library (development headers)</td><td align="right">1.3 MiB</td></tr>
<tr><td align="right">268</td><td><code>libgl-dev</code></td><td>1.7.0-3</td><td>Vendor neutral GL dispatch library -- GL development files</td><td align="right">1.3 MiB</td></tr>
<tr><td align="right">269</td><td><code>tzdata</code></td><td>2026a-3ubuntu1</td><td>time zone and daylight-saving time data</td><td align="right">1.3 MiB</td></tr>
<tr><td align="right">270</td><td><code>libatk1.0-dev</code></td><td>2.60.0-1</td><td>Development files for the ATK accessibility toolkit</td><td align="right">1.3 MiB</td></tr>
<tr><td align="right">271</td><td><code>ubuntu-pro-client</code></td><td>37.2ubuntu</td><td>Management tools for Ubuntu Pro</td><td align="right">1.3 MiB</td></tr>
<tr><td align="right">272</td><td><code>sgml-data</code></td><td>2.0.11+nmu1build1</td><td>common SGML and XML data</td><td align="right">1.3 MiB</td></tr>
<tr><td align="right">273</td><td><code>libx11-data</code></td><td>2:1.8.13-1</td><td>X11 client-side library</td><td align="right">1.3 MiB</td></tr>
<tr><td align="right">274</td><td><code>libnet-ssleay-perl</code></td><td>1.94-3</td><td>Perl module for Secure Sockets Layer (SSL)</td><td align="right">1.3 MiB</td></tr>
<tr><td align="right">275</td><td><code>libroc0.4</code></td><td>0.4.0+dfsg-5ubuntu3</td><td>real-time audio streaming over the network (shared library)</td><td align="right">1.3 MiB</td></tr>
<tr><td align="right">276</td><td><code>zlib1g-dev</code></td><td>1:1.3.dfsg+really1.3.1-1ubuntu3</td><td>compression library - development</td><td align="right">1.3 MiB</td></tr>
<tr><td align="right">277</td><td><code>libmm-glib0</code></td><td>1.25.95-1ubuntu1</td><td>D-Bus service for managing modems - shared libraries</td><td align="right">1.3 MiB</td></tr>
<tr><td align="right">278</td><td><code>udisks2</code></td><td>2.10.91-1ubuntu2</td><td>D-Bus service to access and manipulate storage devices</td><td align="right">1.3 MiB</td></tr>
<tr><td align="right">279</td><td><code>ntfs-3g</code></td><td>1:2022.10.3-5ubuntu1</td><td>read/write NTFS driver for FUSE</td><td align="right">1.3 MiB</td></tr>
<tr><td align="right">280</td><td><code>libepoxy0</code></td><td>1.5.10-2build1</td><td>OpenGL function pointer management library</td><td align="right">1.3 MiB</td></tr>
<tr><td align="right">281</td><td><code>util-linux-extra</code></td><td>2.41.3-3ubuntu2</td><td>interactive login tools</td><td align="right">1.3 MiB</td></tr>
<tr><td align="right">282</td><td><code>genisoimage</code></td><td>9:1.1.11-5</td><td>Creates ISO-9660 CD-ROM filesystem images</td><td align="right">1.3 MiB</td></tr>
<tr><td align="right">283</td><td><code>kbd</code></td><td>2.7.1-2ubuntu2</td><td>Linux console font and keytable utilities</td><td align="right">1.3 MiB</td></tr>
<tr><td align="right">284</td><td><code>fonts-dejavu-mono</code></td><td>2.37-8build1</td><td>Vera font family derivate with additional characters</td><td align="right">1.3 MiB</td></tr>
<tr><td align="right">285</td><td><code>libwireplumber-0.5-0</code></td><td>0.5.13-1ubuntu1</td><td>Shared libraries for WirePlumber</td><td align="right">1.2 MiB</td></tr>
<tr><td align="right">286</td><td><code>libasound2t64</code></td><td>1.2.15.3-1ubuntu1</td><td>shared library for ALSA applications</td><td align="right">1.2 MiB</td></tr>
<tr><td align="right">287</td><td><code>libharfbuzz0b</code></td><td>12.3.2-2</td><td>OpenType text shaping engine (shared library)</td><td align="right">1.2 MiB</td></tr>
<tr><td align="right">288</td><td><code>tzdata-legacy</code></td><td>2026a-3ubuntu1</td><td>time zone data for TAI minus ten seconds</td><td align="right">1.2 MiB</td></tr>
<tr><td align="right">289</td><td><code>libmpfr6</code></td><td>4.2.2-3</td><td>multiple precision floating-point computation</td><td align="right">1.2 MiB</td></tr>
<tr><td align="right">290</td><td><code>libsystemd0</code></td><td>259.5-0ubuntu3</td><td>systemd utility library</td><td align="right">1.2 MiB</td></tr>
<tr><td align="right">291</td><td><code>libphonenumber8</code></td><td>8.13.51+ds-5</td><td>parsing/formatting/validating phone numbers</td><td align="right">1.2 MiB</td></tr>
<tr><td align="right">292</td><td><code>fonts-noto-mono</code></td><td>20201225-2build1</td><td>&quot;No Tofu&quot; monospaced font family with large Unicode coverage</td><td align="right">1.1 MiB</td></tr>
<tr><td align="right">293</td><td><code>binutils</code></td><td>2.46-3ubuntu2</td><td>GNU assembler, linker and binary utilities</td><td align="right">1.1 MiB</td></tr>
<tr><td align="right">294</td><td><code>libharfbuzz-subset0</code></td><td>12.3.2-2</td><td>OpenType text shaping engine (subset library)</td><td align="right">1.1 MiB</td></tr>
<tr><td align="right">295</td><td><code>libjack-jackd2-0</code></td><td>1.9.22~dfsg-5build1</td><td>JACK Audio Connection Kit (libraries)</td><td align="right">1.1 MiB</td></tr>
<tr><td align="right">296</td><td><code>printer-driver-gutenprint</code></td><td>5.3.4.20220624T01008808d602-4ubuntu2</td><td>printer drivers for CUPS</td><td align="right">1.1 MiB</td></tr>
<tr><td align="right">297</td><td><code>python3-chardet</code></td><td>5.2.0+dfsg-2build1</td><td>Universal Character Encoding Detector (Python3)</td><td align="right">1.1 MiB</td></tr>
<tr><td align="right">298</td><td><code>liborc-0.4-0t64</code></td><td>1:0.4.42-2</td><td>Library of Optimized Inner Loops Runtime Compiler</td><td align="right">1.1 MiB</td></tr>
<tr><td align="right">299</td><td><code>libunbound8</code></td><td>1.24.2-1ubuntu2.1</td><td>library implementing DNS resolution and validation</td><td align="right">1.1 MiB</td></tr>
<tr><td align="right">300</td><td><code>libjpeg-turbo8-dev</code></td><td>2.1.5-4ubuntu4</td><td>Development files for the libjpeg-turbo JPEG library</td><td align="right">1.1 MiB</td></tr>
<tr><td align="right">301</td><td><code>python3-psutil</code></td><td>7.1.0-1ubuntu1</td><td>module providing convenience functions for managing processes (Python3)</td><td align="right">1.1 MiB</td></tr>
<tr><td align="right">302</td><td><code>wayland-protocols</code></td><td>1.47-1</td><td>wayland compositor protocols</td><td align="right">1.1 MiB</td></tr>
<tr><td align="right">303</td><td><code>libflashrom1</code></td><td>1.6.0-2ubuntu1</td><td>Identify, read, write, erase, and verify BIOS/ROM/flash chips - library</td><td align="right">1.1 MiB</td></tr>
<tr><td align="right">304</td><td><code>libnftables1</code></td><td>1.1.6-1</td><td>Netfilter nftables high level userspace API library</td><td align="right">1.1 MiB</td></tr>
<tr><td align="right">305</td><td><code>ethtool</code></td><td>1:6.19-1</td><td>display or change Ethernet device settings</td><td align="right">1.1 MiB</td></tr>
<tr><td align="right">306</td><td><code>libisoburn1t64</code></td><td>1:1.5.6-1.1ubuntu4</td><td>library to handle creation and inspection of ISO-9660 file systems</td><td align="right">1.0 MiB</td></tr>
<tr><td align="right">307</td><td><code>libpam-modules</code></td><td>1.7.0-5ubuntu3</td><td>Pluggable Authentication Modules for PAM</td><td align="right">1.0 MiB</td></tr>
<tr><td align="right">308</td><td><code>python3-rich</code></td><td>13.9.4-1.2</td><td>render rich text, tables, progress bars, syntax highlighting, markdown and more</td><td align="right">1.0 MiB</td></tr>
<tr><td align="right">309</td><td><code>libkrb5-3</code></td><td>1.22.1-2ubuntu4</td><td>MIT Kerberos runtime libraries</td><td align="right">1.0 MiB</td></tr>
<tr><td align="right">310</td><td><code>cups</code></td><td>2.4.16-1ubuntu1</td><td>Common UNIX Printing System(tm) - PPD/driver support, web interface</td><td align="right">1.0 MiB</td></tr>
<tr><td align="right">311</td><td><code>ppp</code></td><td>2.5.2-1+1.2</td><td>Point-to-Point Protocol (PPP) - daemon</td><td align="right">1.0 MiB</td></tr>
<tr><td align="right">312</td><td><code>libedataserver-1.2-27t64</code></td><td>3.56.2-8</td><td>Utility library for evolution data servers</td><td align="right">1.0 MiB</td></tr>
<tr><td align="right">313</td><td><code>libtheora-dev</code></td><td>1.2.0+dfsg-6</td><td>Theora Video Compression Codec (development files)</td><td align="right">1.0 MiB</td></tr>
<tr><td align="right">314</td><td><code>libwebrtc-audio-processing-1-3</code></td><td>1.3-3build2</td><td>WebRTC Audio Processing library</td><td align="right">1.0 MiB</td></tr>
<tr><td align="right">315</td><td><code>libbrotli-dev</code></td><td>1.2.0-3build1</td><td>library implementing brotli encoder and decoder (development files)</td><td align="right">1.0 MiB</td></tr>
<tr><td align="right">316</td><td><code>colord</code></td><td>1.4.8-3</td><td>system service to manage device colour profiles -- system daemon</td><td align="right">1019 KiB</td></tr>
<tr><td align="right">317</td><td><code>libcurl4t64</code></td><td>8.18.0-1ubuntu2.1</td><td>easy-to-use client-side URL transfer library (OpenSSL flavour)</td><td align="right">1018 KiB</td></tr>
<tr><td align="right">318</td><td><code>screen</code></td><td>4.9.1-3ubuntu2</td><td>terminal multiplexer with VT100/ANSI terminal emulation</td><td align="right">1004 KiB</td></tr>
<tr><td align="right">319</td><td><code>remmina</code></td><td>1.4.40+dfsg-2ubuntu1</td><td>GTK+ Remote Desktop Client</td><td align="right">1000 KiB</td></tr>
<tr><td align="right">320</td><td><code>cups-ipp-utils</code></td><td>2.4.16-1ubuntu1</td><td>Common UNIX Printing System(tm) - IPP developer/admin utilities</td><td align="right">999 KiB</td></tr>
<tr><td align="right">321</td><td><code>wamerican</code></td><td>2020.12.07-4build1</td><td>American English dictionary words for /usr/share/dict</td><td align="right">998 KiB</td></tr>
<tr><td align="right">322</td><td><code>libcurl3t64-gnutls</code></td><td>8.18.0-1ubuntu2.1</td><td>easy-to-use client-side URL transfer library (GnuTLS flavour)</td><td align="right">993 KiB</td></tr>
<tr><td align="right">323</td><td><code>gir1.2-gtk-3.0</code></td><td>3.24.52-0ubuntu1</td><td>GTK graphical user interface library -- gir bindings</td><td align="right">984 KiB</td></tr>
<tr><td align="right">324</td><td><code>libblkid-dev</code></td><td>2.41.3-3ubuntu2</td><td>block device ID library - headers</td><td align="right">976 KiB</td></tr>
<tr><td align="right">325</td><td><code>libvte-2.91-0</code></td><td>0.84.0-2</td><td>Terminal emulator widget for GTK 3 - runtime files</td><td align="right">976 KiB</td></tr>
<tr><td align="right">326</td><td><code>xz-utils</code></td><td>5.8.3-1</td><td>XZ-format compression utilities</td><td align="right">976 KiB</td></tr>
<tr><td align="right">327</td><td><code>libpng-dev</code></td><td>1.6.57-1</td><td>PNG library - development (version 1.6)</td><td align="right">960 KiB</td></tr>
<tr><td align="right">328</td><td><code>libdbus-1-dev</code></td><td>1.16.2-2ubuntu4</td><td>simple interprocess messaging system (development headers)</td><td align="right">954 KiB</td></tr>
<tr><td align="right">329</td><td><code>libpipewire-0.3-0t64</code></td><td>1.6.2-1ubuntu1</td><td>libraries for the PipeWire multimedia server</td><td align="right">951 KiB</td></tr>
<tr><td align="right">330</td><td><code>libarchive13t64</code></td><td>3.8.5-1ubuntu2.1</td><td>Multi-format archive and compression library (shared library)</td><td align="right">947 KiB</td></tr>
<tr><td align="right">331</td><td><code>cups-daemon</code></td><td>2.4.16-1ubuntu1</td><td>Common UNIX Printing System(tm) - daemon</td><td align="right">921 KiB</td></tr>
<tr><td align="right">332</td><td><code>libpulse0</code></td><td>1:17.0+dfsg1-2ubuntu4</td><td>PulseAudio client libraries</td><td align="right">921 KiB</td></tr>
<tr><td align="right">333</td><td><code>dnsmasq-base</code></td><td>2.92-1ubuntu0.3</td><td>Small caching DNS proxy and DHCP/TFTP server - executable</td><td align="right">920 KiB</td></tr>
<tr><td align="right">334</td><td><code>libstemmer0d</code></td><td>3.0.1-1</td><td>Snowball stemming algorithms for use in Information Retrieval</td><td align="right">914 KiB</td></tr>
<tr><td align="right">335</td><td><code>python3.14</code></td><td>3.14.4-1</td><td>Interactive high-level object-oriented language (version 3.14)</td><td align="right">912 KiB</td></tr>
<tr><td align="right">336</td><td><code>libudisks2-0</code></td><td>2.10.91-1ubuntu2</td><td>GObject based library to access udisks2</td><td align="right">910 KiB</td></tr>
<tr><td align="right">337</td><td><code>libedata-book-1.2-27t64</code></td><td>3.56.2-8</td><td>Backend library for evolution address books</td><td align="right">906 KiB</td></tr>
<tr><td align="right">338</td><td><code>libfreetype6</code></td><td>2.14.2+dfsg-1</td><td>Freetype 2 font engine, shared library files</td><td align="right">906 KiB</td></tr>
<tr><td align="right">339</td><td><code>plymouth</code></td><td>24.004.60+git20250831.4a3c171d-0ubuntu8</td><td>boot animation, logger and I/O multiplexer</td><td align="right">904 KiB</td></tr>
<tr><td align="right">340</td><td><code>libmbim-glib4</code></td><td>1.32.0-2ubuntu1</td><td>Support library to use the MBIM protocol</td><td align="right">903 KiB</td></tr>
<tr><td align="right">341</td><td><code>libfprint-2-2</code></td><td>1:1.95.1+tod1-0ubuntu1</td><td>async fingerprint library of fprint project, shared libraries</td><td align="right">899 KiB</td></tr>
<tr><td align="right">342</td><td><code>libatspi2.0-dev</code></td><td>2.60.0-1</td><td>Development files for the assistive technology service provider</td><td align="right">890 KiB</td></tr>
<tr><td align="right">343</td><td><code>libprotobuf-lite32t64</code></td><td>3.21.12-15ubuntu1</td><td>protocol buffers C++ library (lite version)</td><td align="right">890 KiB</td></tr>
<tr><td align="right">344</td><td><code>libglib-perl</code></td><td>3:1.329.4-1</td><td>interface to the GLib and GObject libraries</td><td align="right">888 KiB</td></tr>
<tr><td align="right">345</td><td><code>libbrotli1</code></td><td>1.2.0-3build1</td><td>library implementing brotli encoder and decoder (shared libraries)</td><td align="right">886 KiB</td></tr>
<tr><td align="right">346</td><td><code>amd64-microcode</code></td><td>3.20251202.1ubuntu2</td><td>Platform firmware and microcode for AMD CPUs and SoCs</td><td align="right">885 KiB</td></tr>
<tr><td align="right">347</td><td><code>libexpat1-dev</code></td><td>2.7.4-1</td><td>XML parsing C library - development kit</td><td align="right">882 KiB</td></tr>
<tr><td align="right">348</td><td><code>libcaca0</code></td><td>0.99.beta20-6ubuntu2.1</td><td>colour ASCII art library</td><td align="right">876 KiB</td></tr>
<tr><td align="right">349</td><td><code>system-config-printer-common</code></td><td>1.5.18-4ubuntu2</td><td>backend and the translation files for system-config-printer</td><td align="right">872 KiB</td></tr>
<tr><td align="right">350</td><td><code>systemd-resolved</code></td><td>259.5-0ubuntu3</td><td>systemd DNS resolver</td><td align="right">867 KiB</td></tr>
<tr><td align="right">351</td><td><code>libqmi-utils</code></td><td>1.38.0-1</td><td>Utilities to use the QMI protocol from the command line</td><td align="right">866 KiB</td></tr>
<tr><td align="right">352</td><td><code>poppler-utils</code></td><td>26.01.0-2build2</td><td>PDF utilities (based on Poppler)</td><td align="right">860 KiB</td></tr>
<tr><td align="right">353</td><td><code>libjemalloc2</code></td><td>5.3.0-4</td><td>general-purpose scalable concurrent malloc(3) implementation</td><td align="right">859 KiB</td></tr>
<tr><td align="right">354</td><td><code>ufw</code></td><td>0.36.2-9build1</td><td>program for managing a Netfilter firewall</td><td align="right">850 KiB</td></tr>
<tr><td align="right">355</td><td><code>libcupsfilters2t64</code></td><td>2.1.1-0ubuntu5</td><td>OpenPrinting libcupsfilters - Shared library</td><td align="right">844 KiB</td></tr>
<tr><td align="right">356</td><td><code>debconf-i18n</code></td><td>1.5.92</td><td>full internationalization support for debconf</td><td align="right">842 KiB</td></tr>
<tr><td align="right">357</td><td><code>cups-filters</code></td><td>2.0.1-0ubuntu4</td><td>OpenPrinting CUPS Filters - Main Package</td><td align="right">839 KiB</td></tr>
<tr><td align="right">358</td><td><code>perl</code></td><td>5.40.1-7build1</td><td>Larry Wall&#x27;s Practical Extraction and Report Language</td><td align="right">835 KiB</td></tr>
<tr><td align="right">359</td><td><code>libdw1t64</code></td><td>0.194-4</td><td>library that provides access to the DWARF debug information</td><td align="right">831 KiB</td></tr>
<tr><td align="right">360</td><td><code>rsync</code></td><td>3.4.1+ds1-7ubuntu0.2</td><td>fast, versatile, remote (and local) file-copying tool</td><td align="right">825 KiB</td></tr>
<tr><td align="right">361</td><td><code>libsepol2</code></td><td>3.9-2</td><td>SELinux library for manipulating binary security policies</td><td align="right">823 KiB</td></tr>
<tr><td align="right">362</td><td><code>keyboard-configuration</code></td><td>1.237ubuntu3</td><td>system-wide keyboard preferences</td><td align="right">819 KiB</td></tr>
<tr><td align="right">363</td><td><code>libzstd1</code></td><td>1.5.7+dfsg-3</td><td>fast lossless compression algorithm</td><td align="right">802 KiB</td></tr>
<tr><td align="right">364</td><td><code>libfyaml0</code></td><td>0.9.4-1</td><td>Fully feature complete YAML parser and emitter (library)</td><td align="right">797 KiB</td></tr>
<tr><td align="right">365</td><td><code>plymouth-theme-spinner</code></td><td>24.004.60+git20250831.4a3c171d-0ubuntu8</td><td>boot animation, logger and I/O multiplexer - spinner theme</td><td align="right">794 KiB</td></tr>
<tr><td align="right">366</td><td><code>libsource-highlight4t64</code></td><td>3.1.9-4.3build2</td><td>source highlighting library</td><td align="right">793 KiB</td></tr>
<tr><td align="right">367</td><td><code>libhunspell-1.7-0</code></td><td>1.7.2+really1.7.2-11</td><td>spell checker and morphological analyzer (shared library)</td><td align="right">786 KiB</td></tr>
<tr><td align="right">368</td><td><code>hicolor-icon-theme</code></td><td>0.18-2build1</td><td>default fallback theme for FreeDesktop.org icon themes</td><td align="right">780 KiB</td></tr>
<tr><td align="right">369</td><td><code>wireplumber</code></td><td>0.5.13-1ubuntu1</td><td>modular session / policy manager for PipeWire</td><td align="right">768 KiB</td></tr>
<tr><td align="right">370</td><td><code>localechooser-data</code></td><td>2.89ubuntu7</td><td>Lists of locales supported by the installer</td><td align="right">766 KiB</td></tr>
<tr><td align="right">371</td><td><code>libselinux-dev</code></td><td>3.9-4build1</td><td>SELinux development headers</td><td align="right">763 KiB</td></tr>
<tr><td align="right">372</td><td><code>libappstream5</code></td><td>1.1.2-1</td><td>Library to access AppStream services</td><td align="right">759 KiB</td></tr>
<tr><td align="right">373</td><td><code>libpixman-1-0</code></td><td>0.46.4-1</td><td>pixel-manipulation library for X and cairo</td><td align="right">757 KiB</td></tr>
<tr><td align="right">374</td><td><code>python3-gi</code></td><td>3.56.2-1</td><td>Python 3 bindings for gobject-introspection libraries</td><td align="right">757 KiB</td></tr>
<tr><td align="right">375</td><td><code>libfreerdp-client3-3</code></td><td>3.24.2+dfsg-1ubuntu1</td><td>Free Remote Desktop Protocol library (client library)</td><td align="right">752 KiB</td></tr>
<tr><td align="right">376</td><td><code>gir1.2-glib-2.0</code></td><td>2.88.0-1</td><td>Introspection data for GLib, GObject, Gio and GModule</td><td align="right">746 KiB</td></tr>
<tr><td align="right">377</td><td><code>libblas3</code></td><td>3.12.1-7ubuntu1</td><td>Basic Linear Algebra Reference implementations, shared library</td><td align="right">746 KiB</td></tr>
<tr><td align="right">378</td><td><code>libasound2-dev</code></td><td>1.2.15.3-1ubuntu1</td><td>shared library for ALSA applications -- development files</td><td align="right">742 KiB</td></tr>
<tr><td align="right">379</td><td><code>printer-driver-foo2zjs</code></td><td>20200505dfsg0-3ubuntu1</td><td>printer driver for ZjStream-based printers</td><td align="right">738 KiB</td></tr>
<tr><td align="right">380</td><td><code>wget</code></td><td>1.25.0-2ubuntu4</td><td>retrieves files from the web</td><td align="right">732 KiB</td></tr>
<tr><td align="right">381</td><td><code>libpam-systemd</code></td><td>259.5-0ubuntu3</td><td>system and service manager - PAM module</td><td align="right">729 KiB</td></tr>
<tr><td align="right">382</td><td><code>libglvnd0</code></td><td>1.7.0-3</td><td>Vendor neutral GL dispatch library</td><td align="right">728 KiB</td></tr>
<tr><td align="right">383</td><td><code>bluez-obexd</code></td><td>5.85-4</td><td>bluez obex daemon</td><td align="right">726 KiB</td></tr>
<tr><td align="right">384</td><td><code>usb.ids</code></td><td>2025.12.13-1</td><td>USB ID Repository</td><td align="right">726 KiB</td></tr>
<tr><td align="right">385</td><td><code>dirmngr</code></td><td>2.4.8-4ubuntu3</td><td>GNU privacy guard - network certificate management service</td><td align="right">724 KiB</td></tr>
<tr><td align="right">386</td><td><code>tar</code></td><td>1.35+dfsg-4</td><td>GNU version of the tar archiving utility</td><td align="right">724 KiB</td></tr>
<tr><td align="right">387</td><td><code>python3-numpy-dev</code></td><td>1:2.3.5+ds-3ubuntu1</td><td>Python library for numerical computations (Python 3 headers)</td><td align="right">723 KiB</td></tr>
<tr><td align="right">388</td><td><code>libsoup-3.0-0</code></td><td>3.6.6-1</td><td>HTTP library implementation in C -- Shared library</td><td align="right">722 KiB</td></tr>
<tr><td align="right">389</td><td><code>sound-icons</code></td><td>0.1-8build1</td><td>Sounds for speech enabled applications</td><td align="right">722 KiB</td></tr>
<tr><td align="right">390</td><td><code>dictionaries-common</code></td><td>1.31.4</td><td>spelling dictionaries - common utilities</td><td align="right">717 KiB</td></tr>
<tr><td align="right">391</td><td><code>3cpio</code></td><td>0.14.0-1ubuntu1</td><td>Manage initrd cpio archives</td><td align="right">713 KiB</td></tr>
<tr><td align="right">392</td><td><code>libcups2t64</code></td><td>2.4.16-1ubuntu1</td><td>Common UNIX Printing System(tm) - Core library</td><td align="right">712 KiB</td></tr>
<tr><td align="right">393</td><td><code>libgstreamer-gl1.0-0</code></td><td>1.28.2-1</td><td>GStreamer GL libraries</td><td align="right">712 KiB</td></tr>
<tr><td align="right">394</td><td><code>icu-devtools</code></td><td>78.2-2ubuntu1</td><td>Development utilities for International Components for Unicode</td><td align="right">711 KiB</td></tr>
<tr><td align="right">395</td><td><code>libpcre2-8-0</code></td><td>10.46-1build1</td><td>New Perl Compatible Regular Expression Library- 8 bit runtime files</td><td align="right">708 KiB</td></tr>
<tr><td align="right">396</td><td><code>libgles-dev</code></td><td>1.7.0-3</td><td>Vendor neutral GL dispatch library -- GLES development files</td><td align="right">701 KiB</td></tr>
<tr><td align="right">397</td><td><code>libvorbisenc2</code></td><td>1.3.7-3build2</td><td>encoder library for Vorbis General Audio Compression Codec</td><td align="right">698 KiB</td></tr>
<tr><td align="right">398</td><td><code>libxi-dev</code></td><td>2:1.8.2-2</td><td>X11 Input extension library (development headers)</td><td align="right">696 KiB</td></tr>
<tr><td align="right">399</td><td><code>python3-pkg-resources</code></td><td>78.1.1-0.1build1</td><td>Package Discovery and Resource Access using pkg_resources</td><td align="right">696 KiB</td></tr>
<tr><td align="right">400</td><td><code>chrony</code></td><td>4.8-2ubuntu1</td><td>Versatile implementation of the Network Time Protocol</td><td align="right">692 KiB</td></tr>
<tr><td align="right">401</td><td><code>python3-apt</code></td><td>3.1.0ubuntu1</td><td>Python 3 interface to libapt-pkg</td><td align="right">686 KiB</td></tr>
<tr><td align="right">402</td><td><code>libgio-2.0-dev-bin</code></td><td>2.88.0-1</td><td>Development utilities for GLib, GObject and GIO libraries</td><td align="right">685 KiB</td></tr>
<tr><td align="right">403</td><td><code>libnl-route-3-200</code></td><td>3.12.0-2</td><td>library for dealing with netlink sockets - route interface</td><td align="right">684 KiB</td></tr>
<tr><td align="right">404</td><td><code>libxcb1-dev</code></td><td>1.17.0-2ubuntu1</td><td>X C Binding, development files</td><td align="right">681 KiB</td></tr>
<tr><td align="right">405</td><td><code>fprintd</code></td><td>1.94.5-4</td><td>D-Bus daemon for fingerprint reader access</td><td align="right">676 KiB</td></tr>
<tr><td align="right">406</td><td><code>libxml-parser-perl</code></td><td>2.47-1ubuntu1</td><td>Perl module for parsing XML files</td><td align="right">673 KiB</td></tr>
<tr><td align="right">407</td><td><code>libio-socket-ssl-perl</code></td><td>2.098-1</td><td>Perl module implementing object oriented interface to SSL sockets</td><td align="right">668 KiB</td></tr>
<tr><td align="right">408</td><td><code>libjpeg-turbo8</code></td><td>2.1.5-4ubuntu4</td><td>libjpeg-turbo JPEG runtime library</td><td align="right">664 KiB</td></tr>
<tr><td align="right">409</td><td><code>ncurses-bin</code></td><td>6.6+20251231-1</td><td>terminal-related programs and man pages</td><td align="right">663 KiB</td></tr>
<tr><td align="right">410</td><td><code>x11-utils</code></td><td>7.7+7build1</td><td>X11 utilities</td><td align="right">663 KiB</td></tr>
<tr><td align="right">411</td><td><code>xfonts-encodings</code></td><td>1:1.0.5-0ubuntu3</td><td>Encodings for X.Org fonts</td><td align="right">662 KiB</td></tr>
<tr><td align="right">412</td><td><code>libv4l-0t64</code></td><td>1.32.0-2ubuntu1</td><td>Collection of video4linux support libraries</td><td align="right">661 KiB</td></tr>
<tr><td align="right">413</td><td><code>libcryptsetup12</code></td><td>2:2.8.4-1ubuntu4</td><td>disk encryption support - shared library</td><td align="right">660 KiB</td></tr>
<tr><td align="right">414</td><td><code>gpg-agent</code></td><td>2.4.8-4ubuntu3</td><td>GNU privacy guard - cryptographic agent</td><td align="right">656 KiB</td></tr>
<tr><td align="right">415</td><td><code>liblog4cplus-2.0.5t64</code></td><td>2.0.8-1.1ubuntu4</td><td>C++ logging API modeled after the Java log4j API - shared library</td><td align="right">656 KiB</td></tr>
<tr><td align="right">416</td><td><code>libsnmp-base</code></td><td>5.9.4+dfsg-2ubuntu3</td><td>SNMP configuration script, MIBs and documentation</td><td align="right">655 KiB</td></tr>
<tr><td align="right">417</td><td><code>libppd2</code></td><td>2:2.1.1-0ubuntu2</td><td>OpenPrinting libppd - Shared library</td><td align="right">654 KiB</td></tr>
<tr><td align="right">418</td><td><code>cryptsetup-bin</code></td><td>2:2.8.4-1ubuntu4</td><td>disk encryption support - command line tools</td><td align="right">652 KiB</td></tr>
<tr><td align="right">419</td><td><code>libonig5</code></td><td>6.9.10-1build1</td><td>regular expressions library</td><td align="right">652 KiB</td></tr>
<tr><td align="right">420</td><td><code>libuv1-dev</code></td><td>1.51.0-2ubuntu1</td><td>asynchronous event notification library - development files</td><td align="right">652 KiB</td></tr>
<tr><td align="right">421</td><td><code>libpcre2-16-0</code></td><td>10.46-1build1</td><td>New Perl Compatible Regular Expression Library - 16 bit runtime files</td><td align="right">651 KiB</td></tr>
<tr><td align="right">422</td><td><code>libgl1</code></td><td>1.7.0-3</td><td>Vendor neutral GL dispatch library -- legacy GL support</td><td align="right">648 KiB</td></tr>
<tr><td align="right">423</td><td><code>libyuv0</code></td><td>0.0.1922.20260106-1</td><td>Library for YUV scaling (shared library)</td><td align="right">645 KiB</td></tr>
<tr><td align="right">424</td><td><code>libcupsfilters2-common</code></td><td>2.1.1-0ubuntu5</td><td>OpenPrinting libcupsfilters - Auxiliary files</td><td align="right">643 KiB</td></tr>
<tr><td align="right">425</td><td><code>apport</code></td><td>2.34.0-0ubuntu2</td><td>automatically generate crash reports for debugging</td><td align="right">640 KiB</td></tr>
<tr><td align="right">426</td><td><code>libwayland-dev</code></td><td>1.24.0-2</td><td>wayland compositor infrastructure - development files</td><td align="right">631 KiB</td></tr>
<tr><td align="right">427</td><td><code>libimagequant0</code></td><td>4.4.1-1</td><td>palette quantization library</td><td align="right">630 KiB</td></tr>
<tr><td align="right">428</td><td><code>liblerc4</code></td><td>4.0.0+ds-5ubuntu2</td><td>Limited Error Raster Compression library</td><td align="right">628 KiB</td></tr>
<tr><td align="right">429</td><td><code>libpcre2-32-0</code></td><td>10.46-1build1</td><td>New Perl Compatible Regular Expression Library - 32 bit runtime files</td><td align="right">627 KiB</td></tr>
<tr><td align="right">430</td><td><code>libfontconfig-dev</code></td><td>2.17.1-3ubuntu1</td><td>generic font configuration library - development</td><td align="right">622 KiB</td></tr>
<tr><td align="right">431</td><td><code>sane-utils</code></td><td>1.4.0-1ubuntu1</td><td>API library for scanners -- utilities</td><td align="right">621 KiB</td></tr>
<tr><td align="right">432</td><td><code>libtiff6</code></td><td>4.7.0-3ubuntu4</td><td>Tag Image File Format (TIFF) library</td><td align="right">618 KiB</td></tr>
<tr><td align="right">433</td><td><code>libldb2</code></td><td>2:2.11.0+samba4.23.6+dfsg-1ubuntu2.1</td><td>LDAP-like embedded database - shared library</td><td align="right">609 KiB</td></tr>
<tr><td align="right">434</td><td><code>python3-debian</code></td><td>1.0.1ubuntu2</td><td>Python 3 modules to work with Debian-related data formats</td><td align="right">608 KiB</td></tr>
<tr><td align="right">435</td><td><code>gcc-15</code></td><td>15.2.0-16ubuntu1</td><td>GNU C compiler</td><td align="right">607 KiB</td></tr>
<tr><td align="right">436</td><td><code>libcolord2</code></td><td>1.4.8-3</td><td>system service to manage device colour profiles -- runtime</td><td align="right">607 KiB</td></tr>
<tr><td align="right">437</td><td><code>libvulkan1</code></td><td>1.4.341.0-1</td><td>Vulkan loader library</td><td align="right">607 KiB</td></tr>
<tr><td align="right">438</td><td><code>psmisc</code></td><td>23.7-2ubuntu2</td><td>utilities that use the proc file system</td><td align="right">604 KiB</td></tr>
<tr><td align="right">439</td><td><code>libsndfile1</code></td><td>1.2.2-4</td><td>Library for reading/writing audio files</td><td align="right">602 KiB</td></tr>
<tr><td align="right">440</td><td><code>publicsuffix</code></td><td>20260129.1928-1</td><td>accurate, machine-readable list of domain name suffixes</td><td align="right">602 KiB</td></tr>
<tr><td align="right">441</td><td><code>libwebp7</code></td><td>1.5.0-0.1build1</td><td>Lossy compression of digital photographic images (shared library)</td><td align="right">600 KiB</td></tr>
<tr><td align="right">442</td><td><code>squashfs-tools</code></td><td>1:4.7.5-1</td><td>Tool to create and append to squashfs filesystems</td><td align="right">599 KiB</td></tr>
<tr><td align="right">443</td><td><code>bzip2-doc</code></td><td>1.0.8-6build2</td><td>high-quality block-sorting file compressor - documentation</td><td align="right">597 KiB</td></tr>
<tr><td align="right">444</td><td><code>libnet-dbus-perl</code></td><td>1.2.0-2build4</td><td>Perl extension for the DBus bindings</td><td align="right">596 KiB</td></tr>
<tr><td align="right">445</td><td><code>libthai-data</code></td><td>0.1.30-1</td><td>Data files for Thai language support library</td><td align="right">594 KiB</td></tr>
<tr><td align="right">446</td><td><code>thermald</code></td><td>2.5.11-0ubuntu1</td><td>Thermal monitoring and controlling daemon</td><td align="right">591 KiB</td></tr>
<tr><td align="right">447</td><td><code>libibus-1.0-5</code></td><td>1.5.34~rc2-1</td><td>Intelligent Input Bus - shared library</td><td align="right">588 KiB</td></tr>
<tr><td align="right">448</td><td><code>libtss2-esys-3.0.2-0t64</code></td><td>4.1.3-6</td><td>TPM2 Software stack library - TSS and TCTI libraries</td><td align="right">588 KiB</td></tr>
<tr><td align="right">449</td><td><code>libpoppler-glib8t64</code></td><td>26.01.0-2build2</td><td>PDF rendering library (GLib-based shared library)</td><td align="right">584 KiB</td></tr>
<tr><td align="right">450</td><td><code>cracklib-runtime</code></td><td>2.9.6-5.2build3</td><td>runtime support for password checker library cracklib2</td><td align="right">583 KiB</td></tr>
<tr><td align="right">451</td><td><code>libxmlsec1-1</code></td><td>1.3.9-1</td><td>XML security library</td><td align="right">583 KiB</td></tr>
<tr><td align="right">452</td><td><code>python3-oauthlib</code></td><td>3.3.1-1build1</td><td>generic, spec-compliant implementation of OAuth for Python3</td><td align="right">581 KiB</td></tr>
<tr><td align="right">453</td><td><code>gpgsm</code></td><td>2.4.8-4ubuntu3</td><td>GNU privacy guard - S/MIME version</td><td align="right">573 KiB</td></tr>
<tr><td align="right">454</td><td><code>findutils</code></td><td>4.10.0-3build2</td><td>utilities for finding files--find, xargs</td><td align="right">572 KiB</td></tr>
<tr><td align="right">455</td><td><code>liblua5.4-0</code></td><td>5.4.8-1build1</td><td>Shared library for the Lua interpreter version 5.4</td><td align="right">570 KiB</td></tr>
<tr><td align="right">456</td><td><code>libtinfo6</code></td><td>6.6+20251231-1</td><td>shared low-level terminfo library for terminal handling</td><td align="right">569 KiB</td></tr>
<tr><td align="right">457</td><td><code>libldap2</code></td><td>2.6.10+dfsg-1ubuntu5</td><td>OpenLDAP libraries</td><td align="right">568 KiB</td></tr>
<tr><td align="right">458</td><td><code>x11-xserver-utils</code></td><td>7.7+11build1</td><td>X server utilities</td><td align="right">567 KiB</td></tr>
<tr><td align="right">459</td><td><code>linux-firmware-amd-misc</code></td><td>20260319.git217ca6e4-0ubuntu1</td><td>Firmware for AMD NPU accelerators</td><td align="right">561 KiB</td></tr>
<tr><td align="right">460</td><td><code>python3-html5lib</code></td><td>1.2-3</td><td>HTML parser/tokenizer based on the WHATWG HTML5 specification</td><td align="right">561 KiB</td></tr>
<tr><td align="right">461</td><td><code>python3-yaml</code></td><td>6.0.3-1build1</td><td>YAML parser and emitter for Python3</td><td align="right">561 KiB</td></tr>
<tr><td align="right">462</td><td><code>libpango-1.0-0</code></td><td>1.57.0-1</td><td>Layout and rendering of internationalized text</td><td align="right">556 KiB</td></tr>
<tr><td align="right">463</td><td><code>libgmp10</code></td><td>2:6.3.0+dfsg-5ubuntu2</td><td>Multiprecision arithmetic library</td><td align="right">552 KiB</td></tr>
<tr><td align="right">464</td><td><code>libext2fs2t64</code></td><td>1.47.2-3ubuntu4</td><td>ext2/ext3/ext4 file system libraries</td><td align="right">551 KiB</td></tr>
<tr><td align="right">465</td><td><code>zip</code></td><td>3.0-15ubuntu3</td><td>Archiver for .zip files</td><td align="right">549 KiB</td></tr>
<tr><td align="right">466</td><td><code>libssh-4</code></td><td>0.11.3-1ubuntu1</td><td>tiny C SSH library (OpenSSL flavor)</td><td align="right">548 KiB</td></tr>
<tr><td align="right">467</td><td><code>libxml-twig-perl</code></td><td>1:3.54-1build1</td><td>Perl module for processing huge XML documents in tree mode</td><td align="right">548 KiB</td></tr>
<tr><td align="right">468</td><td><code>python3-bcrypt</code></td><td>5.0.0-3build1</td><td>password hashing library for Python 3</td><td align="right">541 KiB</td></tr>
<tr><td align="right">469</td><td><code>polkitd</code></td><td>127-2ubuntu1</td><td>framework for managing administrative policies and privileges</td><td align="right">540 KiB</td></tr>
<tr><td align="right">470</td><td><code>dhcpcd-base</code></td><td>1:10.3.0-7</td><td>DHCPv4 and DHCPv6 dual-stack client (binaries and exit hooks)</td><td align="right">537 KiB</td></tr>
<tr><td align="right">471</td><td><code>sound-theme-freedesktop</code></td><td>0.8-7build1</td><td>freedesktop.org sound theme</td><td align="right">534 KiB</td></tr>
<tr><td align="right">472</td><td><code>accountsservice</code></td><td>23.13.9-8ubuntu5.1</td><td>query and manipulate user account information</td><td align="right">532 KiB</td></tr>
<tr><td align="right">473</td><td><code>binutils-common</code></td><td>2.46-3ubuntu2</td><td>Common files for the GNU assembler, linker and binary utilities</td><td align="right">532 KiB</td></tr>
<tr><td align="right">474</td><td><code>libsimdutf31</code></td><td>8.0.0-1</td><td>Fast Unicode validation and transcoding</td><td align="right">529 KiB</td></tr>
<tr><td align="right">475</td><td><code>libpam-runtime</code></td><td>1.7.0-5ubuntu3</td><td>Runtime support for the PAM library</td><td align="right">528 KiB</td></tr>
<tr><td align="right">476</td><td><code>mobile-broadband-provider-info</code></td><td>20251101-1build1</td><td>database of mobile broadband service providers</td><td align="right">527 KiB</td></tr>
<tr><td align="right">477</td><td><code>libfwupd3</code></td><td>2.1.1-1ubuntu3</td><td>Firmware update daemon library</td><td align="right">524 KiB</td></tr>
<tr><td align="right">478</td><td><code>libbabeltrace1</code></td><td>1.5.11-5build1</td><td>Babeltrace conversion libraries</td><td align="right">523 KiB</td></tr>
<tr><td align="right">479</td><td><code>libisofs6t64</code></td><td>1.5.6.pl01-1.1ubuntu3</td><td>library to create ISO 9660 images</td><td align="right">521 KiB</td></tr>
<tr><td align="right">480</td><td><code>libespeak-ng1</code></td><td>1.52.0+dfsg-5build1</td><td>Multi-lingual software speech synthesizer: shared library</td><td align="right">518 KiB</td></tr>
<tr><td align="right">481</td><td><code>libnss-systemd</code></td><td>259.5-0ubuntu3</td><td>nss module providing dynamic user and group name resolution</td><td align="right">515 KiB</td></tr>
<tr><td align="right">482</td><td><code>python3-typing-extensions</code></td><td>4.15.0-2</td><td>Backported and Experimental Type Hints for Python</td><td align="right">511 KiB</td></tr>
<tr><td align="right">483</td><td><code>curl</code></td><td>8.18.0-1ubuntu2.1</td><td>command line tool for transferring data with URL syntax</td><td align="right">509 KiB</td></tr>
<tr><td align="right">484</td><td><code>evolution-data-server-common</code></td><td>3.56.2-8</td><td>architecture independent files for Evolution Data Server</td><td align="right">508 KiB</td></tr>
<tr><td align="right">485</td><td><code>debconf</code></td><td>1.5.92</td><td>Debian configuration management system</td><td align="right">507 KiB</td></tr>
<tr><td align="right">486</td><td><code>python3-cairo</code></td><td>1.27.0-2build2</td><td>Python3 bindings for the Cairo vector graphics library</td><td align="right">505 KiB</td></tr>
<tr><td align="right">487</td><td><code>systemd-cryptsetup</code></td><td>259.5-0ubuntu3</td><td>Provides cryptsetup, integritysetup and veritysetup utilities</td><td align="right">503 KiB</td></tr>
<tr><td align="right">488</td><td><code>glycin-thumbnailers</code></td><td>2.1.1+ds-0ubuntu1</td><td>sandboxed image loaders for GNOME</td><td align="right">502 KiB</td></tr>
<tr><td align="right">489</td><td><code>gnome-themes-extra-data</code></td><td>3.28-5</td><td>Adwaita GTK 2 theme and Adwaita-dark GTK 3 theme — common files</td><td align="right">501 KiB</td></tr>
<tr><td align="right">490</td><td><code>grub-pc</code></td><td>2.14-2ubuntu2</td><td>GRand Unified Bootloader, version 2 (PC/BIOS version)</td><td align="right">500 KiB</td></tr>
<tr><td align="right">491</td><td><code>libmtp9t64</code></td><td>1.1.22-1ubuntu1</td><td>Media Transfer Protocol (MTP) library</td><td align="right">500 KiB</td></tr>
<tr><td align="right">492</td><td><code>upower</code></td><td>1.91.1-1</td><td>abstraction for power management</td><td align="right">500 KiB</td></tr>
<tr><td align="right">493</td><td><code>libxaw7</code></td><td>2:1.0.16-1build1</td><td>X11 Athena Widget library</td><td align="right">499 KiB</td></tr>
<tr><td align="right">494</td><td><code>klibc-utils</code></td><td>2.0.14-1ubuntu2</td><td>small utilities built with klibc for early boot</td><td align="right">497 KiB</td></tr>
<tr><td align="right">495</td><td><code>libopenjp2-7</code></td><td>2.5.4-1ubuntu0.1</td><td>JPEG 2000 image compression/decompression library</td><td align="right">497 KiB</td></tr>
<tr><td align="right">496</td><td><code>fonts-liberation-sans-narrow</code></td><td>1:1.07.6-4build1</td><td>Sans-serif Narrow fonts to replace commonly used Arial Narrow</td><td align="right">494 KiB</td></tr>
<tr><td align="right">497</td><td><code>libreadline8t64</code></td><td>8.3-4</td><td>GNU readline and history libraries, run-time libraries</td><td align="right">494 KiB</td></tr>
<tr><td align="right">498</td><td><code>python3-apport</code></td><td>2.34.0-0ubuntu2</td><td>Python 3 library for Apport crash report handling</td><td align="right">491 KiB</td></tr>
<tr><td align="right">499</td><td><code>x11-xkb-utils</code></td><td>7.7+9build1</td><td>X11 XKB utilities</td><td align="right">488 KiB</td></tr>
<tr><td align="right">500</td><td><code>libseccomp-dev</code></td><td>2.6.0-2ubuntu5</td><td>high level interface to Linux seccomp filter (development files)</td><td align="right">487 KiB</td></tr>
<tr><td align="right">501</td><td><code>python3-distupgrade</code></td><td>1:26.04.18</td><td>manage release upgrades</td><td align="right">487 KiB</td></tr>
<tr><td align="right">502</td><td><code>python3-pyparsing</code></td><td>3.3.2-2</td><td>alternative to creating and executing simple grammars - Python 3.x</td><td align="right">485 KiB</td></tr>
<tr><td align="right">503</td><td><code>libdevmapper1.02.1</code></td><td>2:1.02.205-2ubuntu3</td><td>Linux Kernel Device Mapper userspace library</td><td align="right">484 KiB</td></tr>
<tr><td align="right">504</td><td><code>libpulse-dev</code></td><td>1:17.0+dfsg1-2ubuntu4</td><td>PulseAudio client development headers and libraries</td><td align="right">484 KiB</td></tr>
<tr><td align="right">505</td><td><code>python3-urllib3</code></td><td>2.6.3-1ubuntu1</td><td>HTTP library with thread-safe connection pooling for Python3</td><td align="right">484 KiB</td></tr>
<tr><td align="right">506</td><td><code>libplymouth5</code></td><td>24.004.60+git20250831.4a3c171d-0ubuntu8</td><td>graphical boot animation and logger - shared libraries</td><td align="right">482 KiB</td></tr>
<tr style="font-style:italic;color:#666"><td align="right">507</td><td><code>gnupg</code><span style="font-size:0.7em;font-weight:normal;background:#e0e0e0;color:#555;border-radius:3px;padding:1px 4px;margin-left:5px;font-style:normal;vertical-align:middle">meta</span></td><td>2.4.8-4ubuntu3</td><td>GNU privacy guard - a free PGP replacement</td><td align="right">479 KiB</td></tr>
<tr><td align="right">508</td><td><code>libogg-dev</code></td><td>1.3.6-2</td><td>Ogg bitstream library development files</td><td align="right">479 KiB</td></tr>
<tr><td align="right">509</td><td><code>libhtml-tree-perl</code></td><td>5.07-3</td><td>Perl module to represent and create HTML syntax trees</td><td align="right">478 KiB</td></tr>
<tr><td align="right">510</td><td><code>libgdk-pixbuf-2.0-dev</code></td><td>2.44.5+dfsg-4ubuntu1</td><td>GDK Pixbuf library (development files)</td><td align="right">477 KiB</td></tr>
<tr><td align="right">511</td><td><code>cups-ppdc</code></td><td>2.4.16-1ubuntu1</td><td>Common UNIX Printing System(tm) - PPD manipulation utilities</td><td align="right">474 KiB</td></tr>
<tr><td align="right">512</td><td><code>liblzma5</code></td><td>5.8.3-1</td><td>XZ-format compression library</td><td align="right">473 KiB</td></tr>
<tr><td align="right">513</td><td><code>fontconfig</code></td><td>2.17.1-3ubuntu1</td><td>generic font configuration library - support binaries</td><td align="right">468 KiB</td></tr>
<tr><td align="right">514</td><td><code>libpq5</code></td><td>18.4-0ubuntu0.26.04.1</td><td>PostgreSQL C client library</td><td align="right">468 KiB</td></tr>
<tr><td align="right">515</td><td><code>libegl-mesa0</code></td><td>26.0.3-1ubuntu1</td><td>free implementation of the EGL API -- Mesa vendor library</td><td align="right">466 KiB</td></tr>
<tr><td align="right">516</td><td><code>cryptsetup</code></td><td>2:2.8.4-1ubuntu4</td><td>disk encryption support - startup scripts</td><td align="right">464 KiB</td></tr>
<tr><td align="right">517</td><td><code>libodbc2</code></td><td>2.3.14-1</td><td>ODBC Driver Manager library for Unix</td><td align="right">462 KiB</td></tr>
<tr><td align="right">518</td><td><code>libbpf1</code></td><td>1:1.6.3-1ubuntu1</td><td>eBPF helper library (shared library)</td><td align="right">459 KiB</td></tr>
<tr><td align="right">519</td><td><code>libxt6t64</code></td><td>1:1.2.1-1.3build1</td><td>X11 toolkit intrinsics library</td><td align="right">459 KiB</td></tr>
<tr><td align="right">520</td><td><code>libx11-protocol-perl</code></td><td>0.56-9</td><td>Perl module for the X Window System Protocol, version 11</td><td align="right">457 KiB</td></tr>
<tr><td align="right">521</td><td><code>bolt</code></td><td>0.9.10-1</td><td>system daemon to manage thunderbolt 3 devices</td><td align="right">454 KiB</td></tr>
<tr><td align="right">522</td><td><code>python3-bs4</code></td><td>4.14.3-2build1</td><td>error-tolerant HTML parser for Python 3</td><td align="right">454 KiB</td></tr>
<tr><td align="right">523</td><td><code>libssh2-1t64</code></td><td>1.11.1-1ubuntu0.26.04.1</td><td>SSH2 client-side library</td><td align="right">453 KiB</td></tr>
<tr><td align="right">524</td><td><code>sane-airscan</code></td><td>0.99.36-2ubuntu1</td><td>SANE backend for AirScan (eSCL) and WSD document scanner</td><td align="right">452 KiB</td></tr>
<tr><td align="right">525</td><td><code>diffutils</code></td><td>1:3.12-1</td><td>File comparison utilities</td><td align="right">448 KiB</td></tr>
<tr><td align="right">526</td><td><code>unattended-upgrades</code></td><td>2.12ubuntu9</td><td>automatic installation of security upgrades</td><td align="right">448 KiB</td></tr>
<tr><td align="right">527</td><td><code>foomatic-db-compressed-ppds</code></td><td>20250819-1build1</td><td>OpenPrinting printer support - Compressed PPDs derived from the database</td><td align="right">446 KiB</td></tr>
<tr><td align="right">528</td><td><code>liblcms2-2</code></td><td>2.17-1ubuntu0.2</td><td>Little CMS 2 color management library</td><td align="right">445 KiB</td></tr>
<tr><td align="right">529</td><td><code>cups-client</code></td><td>2.4.16-1ubuntu1</td><td>Common UNIX Printing System(tm) - client programs (SysV)</td><td align="right">442 KiB</td></tr>
<tr><td align="right">530</td><td><code>alsa-base</code></td><td>1.0.25+dfsg-0ubuntu9</td><td>ALSA driver configuration files</td><td align="right">441 KiB</td></tr>
<tr><td align="right">531</td><td><code>libgd3</code></td><td>2.3.3-13ubuntu2</td><td>GD Graphics Library</td><td align="right">441 KiB</td></tr>
<tr><td align="right">532</td><td><code>libgssapi-krb5-2</code></td><td>1.22.1-2ubuntu4</td><td>MIT Kerberos runtime libraries - krb5 GSS-API Mechanism</td><td align="right">441 KiB</td></tr>
<tr><td align="right">533</td><td><code>base-files</code></td><td>14ubuntu6.1</td><td>Debian base system miscellaneous files</td><td align="right">440 KiB</td></tr>
<tr><td align="right">534</td><td><code>htop</code></td><td>3.4.1-5build2</td><td>interactive processes viewer</td><td align="right">440 KiB</td></tr>
<tr><td align="right">535</td><td><code>libinput10</code></td><td>1.31.1-1</td><td>input device management and event handling library - shared library</td><td align="right">440 KiB</td></tr>
<tr><td align="right">536</td><td><code>libflac14</code></td><td>1.5.0+ds-5</td><td>Free Lossless Audio Codec - runtime C library</td><td align="right">438 KiB</td></tr>
<tr><td align="right">537</td><td><code>libnettle8t64</code></td><td>3.10.2-1</td><td>low level cryptographic library (symmetric and one-way cryptos)</td><td align="right">438 KiB</td></tr>
<tr><td align="right">538</td><td><code>adduser</code></td><td>3.153ubuntu1</td><td>add and remove users and groups</td><td align="right">437 KiB</td></tr>
<tr><td align="right">539</td><td><code>libpackagekit-glib2-18</code></td><td>1.3.4-3ubuntu1</td><td>Library for accessing PackageKit using GLib</td><td align="right">437 KiB</td></tr>
<tr><td align="right">540</td><td><code>libsnapd-glib-2-1</code></td><td>1.72-0ubuntu3</td><td>GLib snapd library</td><td align="right">437 KiB</td></tr>
<tr><td align="right">541</td><td><code>libxmlsec1-openssl1</code></td><td>1.3.9-1</td><td>Openssl engine for the XML security library</td><td align="right">437 KiB</td></tr>
<tr><td align="right">542</td><td><code>hplip</code></td><td>3.24.4+dfsg0-0ubuntu8</td><td>HP Linux Printing and Imaging System (HPLIP)</td><td align="right">433 KiB</td></tr>
<tr><td align="right">543</td><td><code>aspell-en</code></td><td>2020.12.07-0-1.1build1</td><td>English dictionary for GNU Aspell</td><td align="right">429 KiB</td></tr>
<tr><td align="right">544</td><td><code>libfdisk1</code></td><td>2.41.3-3ubuntu2</td><td>fdisk partitioning library</td><td align="right">428 KiB</td></tr>
<tr><td align="right">545</td><td><code>xfonts-utils</code></td><td>1:7.7+7build1</td><td>X Window System font utility programs</td><td align="right">428 KiB</td></tr>
<tr><td align="right">546</td><td><code>libncursesw6</code></td><td>6.6+20251231-1</td><td>shared libraries for terminal handling (wide character support)</td><td align="right">426 KiB</td></tr>
<tr><td align="right">547</td><td><code>libgpgmepp7</code></td><td>2.0.0-2</td><td>C++ wrapper library for GPGME</td><td align="right">425 KiB</td></tr>
<tr><td align="right">548</td><td><code>libglx-mesa0</code></td><td>26.0.3-1ubuntu1</td><td>free implementation of the OpenGL API -- GLX vendor library</td><td align="right">423 KiB</td></tr>
<tr><td align="right">549</td><td><code>alsa-topology-conf</code></td><td>1.2.5.1-3build1</td><td>ALSA topology configuration files</td><td align="right">420 KiB</td></tr>
<tr><td align="right">550</td><td><code>libdbus-1-3</code></td><td>1.16.2-2ubuntu4</td><td>simple interprocess messaging system (library)</td><td align="right">420 KiB</td></tr>
<tr><td align="right">551</td><td><code>openssl-provider-legacy</code></td><td>3.5.5-1ubuntu3</td><td>Secure Sockets Layer toolkit - cryptographic utility</td><td align="right">420 KiB</td></tr>
<tr><td align="right">552</td><td><code>unzip</code></td><td>6.0-29ubuntu1</td><td>De-archiver for .zip files</td><td align="right">416 KiB</td></tr>
<tr><td align="right">553</td><td><code>gstreamer1.0-gl</code></td><td>1.28.2-1</td><td>GStreamer plugins for GL</td><td align="right">415 KiB</td></tr>
<tr><td align="right">554</td><td><code>libsodium23</code></td><td>1.0.18-2</td><td>Network communication, cryptography and signaturing library</td><td align="right">413 KiB</td></tr>
<tr><td align="right">555</td><td><code>python3-dbus</code></td><td>1.4.0-1build2</td><td>simple interprocess messaging system (Python 3 interface)</td><td align="right">413 KiB</td></tr>
<tr><td align="right">556</td><td><code>libvisual-0.4-0</code></td><td>0.4.2-4</td><td>audio visualization framework</td><td align="right">412 KiB</td></tr>
<tr><td align="right">557</td><td><code>libexpat1</code></td><td>2.7.4-1</td><td>XML parsing C library - runtime library</td><td align="right">411 KiB</td></tr>
<tr><td align="right">558</td><td><code>libsecret-1-0</code></td><td>0.21.7-2build1</td><td>Secret store</td><td align="right">410 KiB</td></tr>
<tr><td align="right">559</td><td><code>libgc1</code></td><td>1:8.2.12-1</td><td>conservative garbage collector for C and C++</td><td align="right">409 KiB</td></tr>
<tr><td align="right">560</td><td><code>xdg-desktop-portal-gtk</code></td><td>1.15.3-2ubuntu1</td><td>GTK+/GNOME portal backend for xdg-desktop-portal</td><td align="right">408 KiB</td></tr>
<tr><td align="right">561</td><td><code>libpcap0.8t64</code></td><td>1.10.6-1ubuntu1</td><td>system interface for user-level packet capture</td><td align="right">407 KiB</td></tr>
<tr><td align="right">562</td><td><code>libgomp1</code></td><td>16-20260322-1ubuntu1</td><td>GCC OpenMP (GOMP) support library</td><td align="right">406 KiB</td></tr>
<tr><td align="right">563</td><td><code>libxslt1.1</code></td><td>1.1.45-0.1</td><td>XSLT 1.0 processing library - runtime library</td><td align="right">406 KiB</td></tr>
<tr><td align="right">564</td><td><code>libmpg123-0t64</code></td><td>1.33.3-2</td><td>MPEG layer 1/2/3 audio decoder (shared library)</td><td align="right">404 KiB</td></tr>
<tr><td align="right">565</td><td><code>console-setup</code></td><td>1.237ubuntu3</td><td>console font and keymap setup program</td><td align="right">402 KiB</td></tr>
<tr><td align="right">566</td><td><code>fdisk</code></td><td>2.41.3-3ubuntu2</td><td>collection of partitioning utilities</td><td align="right">401 KiB</td></tr>
<tr><td align="right">567</td><td><code>libmount1</code></td><td>2.41.3-3ubuntu2</td><td>device mounting library</td><td align="right">401 KiB</td></tr>
<tr><td align="right">568</td><td><code>geoclue-2.0</code></td><td>2.7.2-2ubuntu3</td><td>geoinformation service</td><td align="right">397 KiB</td></tr>
<tr><td align="right">569</td><td><code>libxkbcommon0</code></td><td>1.13.1-1</td><td>library interface to the XKB compiler - shared library</td><td align="right">397 KiB</td></tr>
<tr><td align="right">570</td><td><code>user-setup</code></td><td>1.107ubuntu2</td><td>Set up initial user and password</td><td align="right">397 KiB</td></tr>
<tr><td align="right">571</td><td><code>gir1.2-gstreamer-1.0</code></td><td>1.28.2-1</td><td>GObject introspection data for the GStreamer library</td><td align="right">396 KiB</td></tr>
<tr><td align="right">572</td><td><code>ncurses-base</code></td><td>6.6+20251231-1</td><td>basic terminal type definitions</td><td align="right">393 KiB</td></tr>
<tr><td align="right">573</td><td><code>libebackend-1.2-11t64</code></td><td>3.56.2-8</td><td>Utility library for evolution data servers</td><td align="right">390 KiB</td></tr>
<tr><td align="right">574</td><td><code>libxkbcommon-dev</code></td><td>1.13.1-1</td><td>library interface to the XKB compiler - development files</td><td align="right">390 KiB</td></tr>
<tr><td align="right">575</td><td><code>libcairo-perl</code></td><td>1.109-5build1</td><td>Perl interface to the Cairo graphics library</td><td align="right">387 KiB</td></tr>
<tr><td align="right">576</td><td><code>media-player-info</code></td><td>26-1build1</td><td>Media player identification files</td><td align="right">387 KiB</td></tr>
<tr><td align="right">577</td><td><code>libnfs14</code></td><td>5.0.2-1ubuntu1</td><td>NFS client library (shared library)</td><td align="right">386 KiB</td></tr>
<tr><td align="right">578</td><td><code>librsvg2-common</code></td><td>2.61.3+dfsg-3</td><td>SAX-based renderer library for SVG files (extra runtime)</td><td align="right">386 KiB</td></tr>
<tr><td align="right">579</td><td><code>libexif12</code></td><td>0.6.25-2</td><td>library to parse EXIF files</td><td align="right">384 KiB</td></tr>
<tr><td align="right">580</td><td><code>libevent-2.1-7t64</code></td><td>2.1.12-stable-10build2</td><td>Asynchronous event notification library</td><td align="right">383 KiB</td></tr>
<tr><td align="right">581</td><td><code>libjs-underscore</code></td><td>1.13.8~dfsg+~1.13.0-1</td><td>JavaScript&#x27;s functional programming helper library</td><td align="right">383 KiB</td></tr>
<tr><td align="right">582</td><td><code>libnetplan1</code></td><td>1.2-1ubuntu5</td><td>Declarative network configuration runtime library</td><td align="right">383 KiB</td></tr>
<tr><td align="right">583</td><td><code>libmagic-dev</code></td><td>1:5.46-5build2</td><td>Recognize the type of data in a file using &quot;magic&quot; numbers - development</td><td align="right">380 KiB</td></tr>
<tr><td align="right">584</td><td><code>libparted2t64</code></td><td>3.6-6</td><td>disk partition manipulator - shared library</td><td align="right">380 KiB</td></tr>
<tr><td align="right">585</td><td><code>ca-certificates</code></td><td>20260223</td><td>Common CA certificates</td><td align="right">378 KiB</td></tr>
<tr><td align="right">586</td><td><code>libjq1</code></td><td>1.8.1-4ubuntu2</td><td>lightweight and flexible command-line JSON processor - shared library</td><td align="right">377 KiB</td></tr>
<tr><td align="right">587</td><td><code>python3-brlapi</code></td><td>6.7-1ubuntu6</td><td>Braille display access via BRLTTY - Python3 bindings</td><td align="right">377 KiB</td></tr>
<tr><td align="right">588</td><td><code>flac</code></td><td>1.5.0+ds-5</td><td>Free Lossless Audio Codec - command line tools</td><td align="right">375 KiB</td></tr>
<tr><td align="right">589</td><td><code>libebook-1.2-21t64</code></td><td>3.56.2-8</td><td>Client library for evolution address books</td><td align="right">375 KiB</td></tr>
<tr><td align="right">590</td><td><code>libwww-perl</code></td><td>6.81-1build1</td><td>simple and consistent interface to the world-wide web</td><td align="right">375 KiB</td></tr>
<tr><td align="right">591</td><td><code>python3-click</code></td><td>8.2.0+0.really.8.1.8-1build1</td><td>Command-Line Interface Creation Kit - Python 3.x</td><td align="right">374 KiB</td></tr>
<tr><td align="right">592</td><td><code>less</code></td><td>668-1build1</td><td>pager program similar to more</td><td align="right">373 KiB</td></tr>
<tr><td align="right">593</td><td><code>python3-idna</code></td><td>3.11-1</td><td>Python IDNA2008 (RFC 5891) handling (Python 3)</td><td align="right">372 KiB</td></tr>
<tr><td align="right">594</td><td><code>libgpgme45</code></td><td>2.0.1-2build1</td><td>GPGME - GnuPG Made Easy (library)</td><td align="right">365 KiB</td></tr>
<tr><td align="right">595</td><td><code>grep</code></td><td>3.12-1</td><td>GNU grep, egrep and fgrep</td><td align="right">364 KiB</td></tr>
<tr><td align="right">596</td><td><code>libglib2.0-bin</code></td><td>2.88.0-1</td><td>Programs for the GLib library</td><td align="right">364 KiB</td></tr>
<tr><td align="right">597</td><td><code>libhpmud0</code></td><td>3.24.4+dfsg0-0ubuntu8</td><td>HP Multi-Point Transport Driver (hpmud) run-time libraries</td><td align="right">364 KiB</td></tr>
<tr><td align="right">598</td><td><code>libntfs-3g89t64</code></td><td>1:2022.10.3-5ubuntu1</td><td>read/write NTFS driver for FUSE (runtime library)</td><td align="right">363 KiB</td></tr>
<tr><td align="right">599</td><td><code>libffi-dev</code></td><td>3.5.2-4</td><td>Foreign Function Interface library (development files)</td><td align="right">361 KiB</td></tr>
<tr><td align="right">600</td><td><code>libfontconfig1</code></td><td>2.17.1-3ubuntu1</td><td>generic font configuration library - runtime</td><td align="right">358 KiB</td></tr>
<tr><td align="right">601</td><td><code>mount</code></td><td>2.41.3-3ubuntu2</td><td>tools for mounting and manipulating filesystems</td><td align="right">356 KiB</td></tr>
<tr><td align="right">602</td><td><code>gir1.2-ibus-1.0</code></td><td>1.5.34~rc2-1</td><td>Intelligent Input Bus - introspection data</td><td align="right">353 KiB</td></tr>
<tr><td align="right">603</td><td><code>libudev1</code></td><td>259.5-0ubuntu3</td><td>libudev shared library</td><td align="right">353 KiB</td></tr>
<tr><td align="right">604</td><td><code>appstream</code></td><td>1.1.2-1</td><td>Software component metadata management</td><td align="right">352 KiB</td></tr>
<tr><td align="right">605</td><td><code>cifs-utils</code></td><td>2:7.4-1build1</td><td>Common Internet File System utilities</td><td align="right">352 KiB</td></tr>
<tr><td align="right">606</td><td><code>busybox-initramfs</code></td><td>1:1.37.0-7ubuntu1</td><td>Standalone shell setup for initramfs</td><td align="right">351 KiB</td></tr>
<tr><td align="right">607</td><td><code>libhogweed6t64</code></td><td>3.10.2-1</td><td>low level cryptographic library (public-key cryptos)</td><td align="right">350 KiB</td></tr>
<tr><td align="right">608</td><td><code>libburn4t64</code></td><td>1.5.6-1.2build1</td><td>library to provide CD/DVD/BD writing functions</td><td align="right">347 KiB</td></tr>
<tr><td align="right">609</td><td><code>libpng16-16t64</code></td><td>1.6.57-1</td><td>PNG library - runtime (version 1.6)</td><td align="right">346 KiB</td></tr>
<tr><td align="right">610</td><td><code>im-config</code></td><td>0.62</td><td>Input method configuration framework</td><td align="right">344 KiB</td></tr>
<tr><td align="right">611</td><td><code>sed</code></td><td>4.9-2ubuntu1</td><td>GNU stream editor for filtering/transforming text</td><td align="right">344 KiB</td></tr>
<tr><td align="right">612</td><td><code>dbus-daemon</code></td><td>1.16.2-2ubuntu4</td><td>simple interprocess messaging system (reference message bus)</td><td align="right">343 KiB</td></tr>
<tr><td align="right">613</td><td><code>libyaml-cpp0.8</code></td><td>0.8.0+dfsg-9</td><td>YAML parser and emitter for C++</td><td align="right">341 KiB</td></tr>
<tr><td align="right">614</td><td><code>aspell</code></td><td>0.60.8.2-3</td><td>GNU Aspell spell-checker</td><td align="right">340 KiB</td></tr>
<tr><td align="right">615</td><td><code>libatkmm-1.6-1v5</code></td><td>2.28.4-2</td><td>C++ wrappers for ATK accessibility toolkit (shared libraries)</td><td align="right">340 KiB</td></tr>
<tr><td align="right">616</td><td><code>libdjvulibre-text</code></td><td>3.5.29-1</td><td>Linguistic support files for libdjvulibre</td><td align="right">340 KiB</td></tr>
<tr><td align="right">617</td><td><code>libwebpdecoder3</code></td><td>1.5.0-0.1build1</td><td>Library for the WebP graphics format (decode only)</td><td align="right">340 KiB</td></tr>
<tr><td align="right">618</td><td><code>netplan.io</code></td><td>1.2-1ubuntu5</td><td>Declarative network configuration for various backends at runtime</td><td align="right">340 KiB</td></tr>
<tr><td align="right">619</td><td><code>xdg-utils</code></td><td>1.2.1-2ubuntu2</td><td>desktop integration utilities from freedesktop.org</td><td align="right">340 KiB</td></tr>
<tr><td align="right">620</td><td><code>libncurses6</code></td><td>6.6+20251231-1</td><td>shared libraries for terminal handling</td><td align="right">337 KiB</td></tr>
<tr><td align="right">621</td><td><code>girepository-tools</code></td><td>2.88.0-1</td><td>Tools for working with GObject-Introspection repositories</td><td align="right">336 KiB</td></tr>
<tr><td align="right">622</td><td><code>gpgv</code></td><td>2.4.8-4ubuntu3</td><td>GNU privacy guard - signature verification tool</td><td align="right">336 KiB</td></tr>
<tr><td align="right">623</td><td><code>libxxhash-dev</code></td><td>0.8.3-2build1</td><td>header files and a static library for libxxhash</td><td align="right">335 KiB</td></tr>
<tr><td align="right">624</td><td><code>xorriso</code></td><td>1:1.5.6-1.1ubuntu4</td><td>command line ISO-9660 and Rock Ridge manipulation tool</td><td align="right">333 KiB</td></tr>
<tr><td align="right">625</td><td><code>cups-browsed</code></td><td>2.1.1-0ubuntu3</td><td>OpenPrinting cups-browsed</td><td align="right">332 KiB</td></tr>
<tr><td align="right">626</td><td><code>iw</code></td><td>6.17-1</td><td>tool for configuring Linux wireless devices</td><td align="right">332 KiB</td></tr>
<tr><td align="right">627</td><td><code>python3-wadllib</code></td><td>2.0.0-3</td><td>Python 3 library for navigating WADL files</td><td align="right">330 KiB</td></tr>
<tr><td align="right">628</td><td><code>gsettings-desktop-schemas</code></td><td>50.0-1ubuntu2</td><td>GSettings desktop-wide schemas</td><td align="right">328 KiB</td></tr>
<tr><td align="right">629</td><td><code>libv4lconvert0t64</code></td><td>1.32.0-2ubuntu1</td><td>Video4linux frame format conversion library</td><td align="right">327 KiB</td></tr>
<tr><td align="right">630</td><td><code>liblz4-dev</code></td><td>1.10.0-8</td><td>Fast LZ compression algorithm library - development files</td><td align="right">326 KiB</td></tr>
<tr><td align="right">631</td><td><code>gstreamer1.0-x</code></td><td>1.28.2-1</td><td>GStreamer plugins for X11 and Pango</td><td align="right">324 KiB</td></tr>
<tr><td align="right">632</td><td><code>libduktape207</code></td><td>2.7.0+tests-0ubuntu4</td><td>embeddable Javascript engine, library</td><td align="right">324 KiB</td></tr>
<tr><td align="right">633</td><td><code>libctf-nobfd0</code></td><td>2.46-3ubuntu2</td><td>Compact C Type Format library (runtime, no BFD dependency)</td><td align="right">323 KiB</td></tr>
<tr><td align="right">634</td><td><code>python3-dateutil</code></td><td>2.9.0-4build1</td><td>powerful extensions to the standard Python 3 datetime module</td><td align="right">323 KiB</td></tr>
<tr><td align="right">635</td><td><code>libgs-common</code></td><td>10.06.0~dfsg-3ubuntu1</td><td>interpreter for the PostScript language and for PDF - ICC profiles</td><td align="right">320 KiB</td></tr>
<tr><td align="right">636</td><td><code>libimobiledevice-1.0-6</code></td><td>1.4.0-1build1</td><td>Library for communicating with iPhone and other Apple devices</td><td align="right">320 KiB</td></tr>
<tr><td align="right">637</td><td><code>netplan-generator</code></td><td>1.2-1ubuntu5</td><td>Declarative network configuration for various backends at boot</td><td align="right">317 KiB</td></tr>
<tr><td align="right">638</td><td><code>cups-filters-core-drivers</code></td><td>2.0.1-0ubuntu4</td><td>OpenPrinting CUPS Filters - Driverless printing</td><td align="right">316 KiB</td></tr>
<tr><td align="right">639</td><td><code>libnspr4</code></td><td>2:4.38.2-1ubuntu1</td><td>NetScape Portable Runtime Library</td><td align="right">314 KiB</td></tr>
<tr><td align="right">640</td><td><code>libfuse3-4</code></td><td>3.18.2-1</td><td>Filesystem in Userspace (library) (3.x version)</td><td align="right">313 KiB</td></tr>
<tr><td align="right">641</td><td><code>libksba8</code></td><td>1.6.7-2build1</td><td>X.509 and CMS support library</td><td align="right">311 KiB</td></tr>
<tr><td align="right">642</td><td><code>libwnck-3-0</code></td><td>43.3-1build1</td><td>Window Navigator Construction Kit - runtime files</td><td align="right">311 KiB</td></tr>
<tr><td align="right">643</td><td><code>libgphoto2-port12t64</code></td><td>2.5.33-1ubuntu1</td><td>gphoto2 digital camera port library</td><td align="right">308 KiB</td></tr>
<tr><td align="right">644</td><td><code>python3-more-itertools</code></td><td>10.8.0-1build1</td><td>library with routines for operating on iterables, beyond itertools (Python 3)</td><td align="right">308 KiB</td></tr>
<tr><td align="right">645</td><td><code>xbrlapi</code></td><td>6.7-1ubuntu6</td><td>Access software for a blind person using a braille display - xbrlapi</td><td align="right">307 KiB</td></tr>
<tr><td align="right">646</td><td><code>libxext-dev</code></td><td>2:1.3.4-1build3</td><td>X11 miscellaneous extensions library (development headers)</td><td align="right">306 KiB</td></tr>
<tr><td align="right">647</td><td><code>usbutils</code></td><td>1:019-1</td><td>Linux USB utilities</td><td align="right">306 KiB</td></tr>
<tr><td align="right">648</td><td><code>libsemanage2</code></td><td>3.9-1build1</td><td>SELinux policy management library</td><td align="right">305 KiB</td></tr>
<tr><td align="right">649</td><td><code>libsource-highlight-common</code></td><td>3.1.9-4.3build2</td><td>architecture-independent files for source highlighting library</td><td align="right">305 KiB</td></tr>
<tr><td align="right">650</td><td><code>libgtk-3-bin</code></td><td>3.24.52-0ubuntu1</td><td>programs for the GTK graphical user interface library</td><td align="right">304 KiB</td></tr>
<tr><td align="right">651</td><td><code>libquadmath0</code></td><td>16-20260322-1ubuntu1</td><td>GCC Quad-Precision Math Library</td><td align="right">304 KiB</td></tr>
<tr><td align="right">652</td><td><code>network-manager-openvpn</code></td><td>1.12.5-1</td><td>network management framework (OpenVPN plugin core)</td><td align="right">304 KiB</td></tr>
<tr><td align="right">653</td><td><code>cups-pk-helper</code></td><td>0.2.6-2.1build1</td><td>PolicyKit helper to configure cups with fine-grained privileges</td><td align="right">300 KiB</td></tr>
<tr><td align="right">654</td><td><code>libblkid1</code></td><td>2.41.3-3ubuntu2</td><td>block device ID library</td><td align="right">298 KiB</td></tr>
<tr><td align="right">655</td><td><code>libsysprof-capture-4-dev</code></td><td>50.0-1</td><td>capture library for system-wide Linux profiler</td><td align="right">296 KiB</td></tr>
<tr><td align="right">656</td><td><code>libtss2-mu-4.0.1-0t64</code></td><td>4.1.3-6</td><td>TPM2 Software stack library - TSS and TCTI libraries</td><td align="right">296 KiB</td></tr>
<tr><td align="right">657</td><td><code>libsasl2-modules</code></td><td>2.1.28+dfsg1-9ubuntu3</td><td>Cyrus SASL - pluggable authentication modules</td><td align="right">294 KiB</td></tr>
<tr><td align="right">658</td><td><code>libtheora1</code></td><td>1.2.0+dfsg-6</td><td>Theora Video Compression Codec (shared library - libtheora)</td><td align="right">289 KiB</td></tr>
<tr><td align="right">659</td><td><code>libfribidi-dev</code></td><td>1.0.16-5</td><td>Development files for FreeBidi library</td><td align="right">288 KiB</td></tr>
<tr><td align="right">660</td><td><code>liburi-perl</code></td><td>5.34-2build1</td><td>module to manipulate and access URI strings</td><td align="right">288 KiB</td></tr>
<tr><td align="right">661</td><td><code>python3-packaging</code></td><td>26.0-1</td><td>core utilities for python3 packages</td><td align="right">288 KiB</td></tr>
<tr><td align="right">662</td><td><code>libgl1-mesa-dri</code></td><td>26.0.3-1ubuntu1</td><td>free implementation of the OpenGL API -- DRI modules</td><td align="right">286 KiB</td></tr>
<tr><td align="right">663</td><td><code>mawk</code></td><td>1.3.4.20260129-1</td><td>Pattern scanning and text processing language</td><td align="right">286 KiB</td></tr>
<tr><td align="right">664</td><td><code>bsdextrautils</code></td><td>2.41.3-3ubuntu2</td><td>extra utilities from 4.4BSD-Lite</td><td align="right">284 KiB</td></tr>
<tr><td align="right">665</td><td><code>libatk-bridge2.0-0t64</code></td><td>2.60.0-1</td><td>AT-SPI 2 toolkit bridge - shared library</td><td align="right">283 KiB</td></tr>
<tr><td align="right">666</td><td><code>exfatprogs</code></td><td>1.3.2-1</td><td>exFAT file system utilities</td><td align="right">281 KiB</td></tr>
<tr><td align="right">667</td><td><code>libatspi2.0-0t64</code></td><td>2.60.0-1</td><td>Assistive Technology Service Provider Interface - shared library</td><td align="right">281 KiB</td></tr>
<tr><td align="right">668</td><td><code>python3-dasbus</code></td><td>1.7-2build1</td><td>Pythonic D-Bus library</td><td align="right">281 KiB</td></tr>
<tr><td align="right">669</td><td><code>python3-markdown-it</code></td><td>3.0.0-3build1</td><td>Python port of markdown-it and some its associated plugins</td><td align="right">280 KiB</td></tr>
<tr><td align="right">670</td><td><code>ubuntu-drivers-common</code></td><td>1:0.10.9</td><td>Detect and install additional Ubuntu driver packages</td><td align="right">280 KiB</td></tr>
<tr><td align="right">671</td><td><code>xtrans-dev</code></td><td>1.6.0-1build1</td><td>X transport library (development files)</td><td align="right">279 KiB</td></tr>
<tr><td align="right">672</td><td><code>libnvme1t64</code></td><td>1.16.1-4</td><td>NVMe management library (library)</td><td align="right">278 KiB</td></tr>
<tr><td align="right">673</td><td><code>x11-common</code></td><td>1:7.7+26ubuntu1</td><td>X Window System (X.Org) infrastructure</td><td align="right">278 KiB</td></tr>
<tr><td align="right">674</td><td><code>base-passwd</code></td><td>3.6.8</td><td>Debian base system master password and group files</td><td align="right">277 KiB</td></tr>
<tr><td align="right">675</td><td><code>libblockdev3</code></td><td>3.4.0-1</td><td>Library for manipulating block devices</td><td align="right">277 KiB</td></tr>
<tr><td align="right">676</td><td><code>gettext-base</code></td><td>0.23.2-1</td><td>GNU Internationalization utilities for the base system</td><td align="right">276 KiB</td></tr>
<tr><td align="right">677</td><td><code>libavahi-core7</code></td><td>0.8-18ubuntu1.1</td><td>Avahi&#x27;s embeddable mDNS/DNS-SD library</td><td align="right">276 KiB</td></tr>
<tr><td align="right">678</td><td><code>libxft-dev</code></td><td>2.3.6-1build2</td><td>FreeType-based font drawing library for X (development files)</td><td align="right">276 KiB</td></tr>
<tr><td align="right">679</td><td><code>libuv1t64</code></td><td>1.51.0-2ubuntu1</td><td>asynchronous event notification library - runtime library</td><td align="right">275 KiB</td></tr>
<tr><td align="right">680</td><td><code>libmp3lame0</code></td><td>3.101~svn6525+dfsg-2</td><td>MP3 encoding library</td><td align="right">271 KiB</td></tr>
<tr><td align="right">681</td><td><code>libxpm-dev</code></td><td>1:3.5.17-1build3</td><td>X11 pixmap library (development headers)</td><td align="right">271 KiB</td></tr>
<tr><td align="right">682</td><td><code>libgdk-pixbuf-2.0-0</code></td><td>2.44.5+dfsg-4ubuntu1</td><td>GDK Pixbuf library</td><td align="right">270 KiB</td></tr>
<tr><td align="right">683</td><td><code>gpgconf</code></td><td>2.4.8-4ubuntu3</td><td>GNU privacy guard - core configuration utilities</td><td align="right">268 KiB</td></tr>
<tr><td align="right">684</td><td><code>libmbim-utils</code></td><td>1.32.0-2ubuntu1</td><td>Utilities to use the MBIM protocol from the command line</td><td align="right">265 KiB</td></tr>
<tr><td align="right">685</td><td><code>kmod</code></td><td>34.2-2ubuntu2</td><td>tools for managing Linux kernel modules</td><td align="right">264 KiB</td></tr>
<tr><td align="right">686</td><td><code>gir1.2-freedesktop-dev</code></td><td>1.86.0-6build1</td><td>GIR XML for some FreeDesktop components</td><td align="right">263 KiB</td></tr>
<tr><td align="right">687</td><td><code>python3-attr</code></td><td>25.4.0-1build1</td><td>Attributes without boilerplate (Python 3)</td><td align="right">263 KiB</td></tr>
<tr><td align="right">688</td><td><code>avahi-daemon</code></td><td>0.8-18ubuntu1.1</td><td>Avahi mDNS/DNS-SD daemon</td><td align="right">262 KiB</td></tr>
<tr><td align="right">689</td><td><code>libdisplay-info3</code></td><td>0.3.0-1</td><td>EDID and DisplayID library (shared library)</td><td align="right">260 KiB</td></tr>
<tr><td align="right">690</td><td><code>libedit2</code></td><td>3.1-20251016-1</td><td>BSD editline and history libraries</td><td align="right">258 KiB</td></tr>
<tr><td align="right">691</td><td><code>libgirepository-2.0-0</code></td><td>2.88.0-1</td><td>GLib runtime library for handling GObject introspection data</td><td align="right">258 KiB</td></tr>
<tr><td align="right">692</td><td><code>libvoikko1</code></td><td>4.3.3-1</td><td>Library of free natural language processing tools</td><td align="right">258 KiB</td></tr>
<tr><td align="right">693</td><td><code>libk5crypto3</code></td><td>1.22.1-2ubuntu4</td><td>MIT Kerberos runtime libraries - Crypto Library</td><td align="right">257 KiB</td></tr>
<tr><td align="right">694</td><td><code>pciutils</code></td><td>1:3.14.0-1build2</td><td>PCI utilities</td><td align="right">256 KiB</td></tr>
<tr><td align="right">695</td><td><code>ubuntu-release-upgrader-core</code></td><td>1:26.04.18</td><td>manage release upgrades</td><td align="right">256 KiB</td></tr>
<tr><td align="right">696</td><td><code>libva2</code></td><td>2.23.0-1ubuntu1</td><td>Video Acceleration (VA) API for Linux -- runtime</td><td align="right">255 KiB</td></tr>
<tr><td align="right">697</td><td><code>dmsetup</code></td><td>2:1.02.205-2ubuntu3</td><td>Linux Kernel Device Mapper userspace library</td><td align="right">254 KiB</td></tr>
<tr><td align="right">698</td><td><code>libfprint-2-tod1</code></td><td>1:1.95.1+tod1-0ubuntu1</td><td>async fingerprint library of fprint project, drivers shared libraries</td><td align="right">254 KiB</td></tr>
<tr><td align="right">699</td><td><code>libpam-modules-bin</code></td><td>1.7.0-5ubuntu3</td><td>Pluggable Authentication Modules for PAM - helper binaries</td><td align="right">251 KiB</td></tr>
<tr><td align="right">700</td><td><code>python3-cups</code></td><td>2.0.4-3build1</td><td>Python3 bindings for CUPS</td><td align="right">251 KiB</td></tr>
<tr><td align="right">701</td><td><code>bsdutils</code></td><td>1:2.41.3-3ubuntu2</td><td>basic utilities from 4.4BSD-Lite</td><td align="right">250 KiB</td></tr>
<tr><td align="right">702</td><td><code>libaccountsservice0</code></td><td>23.13.9-8ubuntu5.1</td><td>query and manipulate user account information - shared libraries</td><td align="right">249 KiB</td></tr>
<tr><td align="right">703</td><td><code>liblouisutdml-data</code></td><td>2.12.0-8build1</td><td>Braille UTDML translation library - data</td><td align="right">249 KiB</td></tr>
<tr><td align="right">704</td><td><code>rpcsvc-proto</code></td><td>1.4.3-1build1</td><td>RPC protocol compiler and definitions</td><td align="right">249 KiB</td></tr>
<tr><td align="right">705</td><td><code>libnftnl11</code></td><td>1.3.1-1</td><td>Netfilter nftables userspace API library</td><td align="right">248 KiB</td></tr>
<tr><td align="right">706</td><td><code>desktop-file-utils</code></td><td>0.28-1build1</td><td>Utilities for .desktop files</td><td align="right">247 KiB</td></tr>
<tr><td align="right">707</td><td><code>libfido2-1</code></td><td>1.16.0-2build1</td><td>library for generating and verifying FIDO 2.0 objects</td><td align="right">246 KiB</td></tr>
<tr><td align="right">708</td><td><code>libjsoncpp26</code></td><td>1.9.6-5</td><td>library for reading and writing JSON for C++</td><td align="right">245 KiB</td></tr>
<tr><td align="right">709</td><td><code>dosfstools</code></td><td>4.2-1.2build1</td><td>utilities for making and checking MS-DOS FAT filesystems</td><td align="right">244 KiB</td></tr>
<tr><td align="right">710</td><td><code>libcrypt1</code></td><td>1:4.5.1-1</td><td>libcrypt shared library</td><td align="right">244 KiB</td></tr>
<tr><td align="right">711</td><td><code>libctf0</code></td><td>2.46-3ubuntu2</td><td>Compact C Type Format library (runtime, BFD dependency)</td><td align="right">244 KiB</td></tr>
<tr><td align="right">712</td><td><code>fontconfig-config</code></td><td>2.17.1-3ubuntu1</td><td>generic font configuration library - configuration</td><td align="right">243 KiB</td></tr>
<tr><td align="right">713</td><td><code>python3-cffi-backend</code></td><td>2.0.0-3build1</td><td>Foreign Function Interface for Python 3 calling C code - runtime</td><td align="right">242 KiB</td></tr>
<tr><td align="right">714</td><td><code>python3-requests</code></td><td>2.32.5+dfsg-1ubuntu1</td><td>elegant and simple HTTP library for Python3, built for human beings</td><td align="right">242 KiB</td></tr>
<tr><td align="right">715</td><td><code>libproc2-0</code></td><td>2:4.0.4-9ubuntu1</td><td>library for accessing process information from /proc</td><td align="right">241 KiB</td></tr>
<tr><td align="right">716</td><td><code>libsmbclient0</code></td><td>2:4.23.6+dfsg-1ubuntu2.1</td><td>shared library for communication with SMB/CIFS servers</td><td align="right">239 KiB</td></tr>
<tr><td align="right">717</td><td><code>coreutils-from-uutils</code></td><td>0.0.0~ubuntu25</td><td>coreutils from the uutils project</td><td align="right">237 KiB</td></tr>
<tr><td align="right">718</td><td><code>cron</code></td><td>3.0pl1-200ubuntu1</td><td>process scheduling daemon</td><td align="right">235 KiB</td></tr>
<tr><td align="right">719</td><td><code>libgirepository-1.0-1</code></td><td>1.86.0-6build1</td><td>Library for handling GObject introspection data (runtime library)</td><td align="right">235 KiB</td></tr>
<tr><td align="right">720</td><td><code>libglib-object-introspection-perl</code></td><td>0.052-1</td><td>Perl bindings for gobject-introspection libraries</td><td align="right">235 KiB</td></tr>
<tr><td align="right">721</td><td><code>gir1.2-freedesktop</code></td><td>1.86.0-6build1</td><td>Introspection data for some FreeDesktop components</td><td align="right">234 KiB</td></tr>
<tr><td align="right">722</td><td><code>libbluetooth3</code></td><td>5.85-4</td><td>Library to use the BlueZ Linux Bluetooth stack</td><td align="right">234 KiB</td></tr>
<tr><td align="right">723</td><td><code>libcairo2-dev</code></td><td>1.18.4-3</td><td>Development files for the Cairo 2D graphics library</td><td align="right">234 KiB</td></tr>
<tr><td align="right">724</td><td><code>libidn12</code></td><td>1.43-2build1</td><td>GNU Libidn library, implementation of IETF IDN specifications</td><td align="right">233 KiB</td></tr>
<tr><td align="right">725</td><td><code>debianutils</code></td><td>5.23.2build1</td><td>Miscellaneous utilities specific to Debian</td><td align="right">229 KiB</td></tr>
<tr><td align="right">726</td><td><code>dmidecode</code></td><td>3.6-2build1</td><td>SMBIOS/DMI table decoder</td><td align="right">228 KiB</td></tr>
<tr><td align="right">727</td><td><code>libidn2-0</code></td><td>2.3.8-4build1</td><td>Internationalized domain names (IDNA2008/TR46) library</td><td align="right">228 KiB</td></tr>
<tr><td align="right">728</td><td><code>libselinux1</code></td><td>3.9-4build1</td><td>SELinux runtime shared libraries</td><td align="right">228 KiB</td></tr>
<tr><td align="right">729</td><td><code>libtheoraenc2</code></td><td>1.2.0+dfsg-6</td><td>Theora Video Compression Codec (shared library - libtheoraenc)</td><td align="right">228 KiB</td></tr>
<tr><td align="right">730</td><td><code>libice-dev</code></td><td>2:1.1.1-1build1</td><td>X11 Inter-Client Exchange library (development headers)</td><td align="right">226 KiB</td></tr>
<tr><td align="right">731</td><td><code>libpangomm-1.4-1v5</code></td><td>2.46.4-2</td><td>C++ Wrapper for pango (shared libraries)</td><td align="right">226 KiB</td></tr>
<tr><td align="right">732</td><td><code>libmagic1t64</code></td><td>1:5.46-5build2</td><td>Recognize the type of data in a file using &quot;magic&quot; numbers - library</td><td align="right">225 KiB</td></tr>
<tr><td align="right">733</td><td><code>libebook-contacts-1.2-4t64</code></td><td>3.56.2-8</td><td>Client library for evolution contacts books</td><td align="right">224 KiB</td></tr>
<tr><td align="right">734</td><td><code>python3-systemd</code></td><td>235-1build9</td><td>Python 3 bindings for systemd</td><td align="right">223 KiB</td></tr>
<tr><td align="right">735</td><td><code>sbsigntool</code></td><td>0.9.4-3.1ubuntu9</td><td>Tools to manipulate signatures on UEFI binaries and drivers</td><td align="right">223 KiB</td></tr>
<tr><td align="right">736</td><td><code>liblouisutdml9t64</code></td><td>2.12.0-8build1</td><td>Braille UTDML translation library - shared libs</td><td align="right">221 KiB</td></tr>
<tr><td align="right">737</td><td><code>libwavpack1</code></td><td>5.9.0-1</td><td>audio codec (lossy and lossless) - library</td><td align="right">220 KiB</td></tr>
<tr><td align="right">738</td><td><code>libgpg-error0</code></td><td>1.58-2</td><td>GnuPG development runtime library</td><td align="right">219 KiB</td></tr>
<tr><td align="right">739</td><td><code>libopengl0</code></td><td>1.7.0-3</td><td>Vendor neutral GL dispatch library -- OpenGL support</td><td align="right">219 KiB</td></tr>
<tr><td align="right">740</td><td><code>libtirpc3t64</code></td><td>1.3.7-0.1</td><td>transport-independent RPC library</td><td align="right">219 KiB</td></tr>
<tr><td align="right">741</td><td><code>libefivar1t64</code></td><td>39-2</td><td>Library to manage UEFI variables</td><td align="right">218 KiB</td></tr>
<tr><td align="right">742</td><td><code>spice-vdagent</code></td><td>0.23.0-1</td><td>Spice agent for Linux</td><td align="right">218 KiB</td></tr>
<tr><td align="right">743</td><td><code>libmailtools-perl</code></td><td>2.22-1</td><td>modules to manipulate email in perl programs</td><td align="right">217 KiB</td></tr>
<tr><td align="right">744</td><td><code>libsmartcols1</code></td><td>2.41.3-3ubuntu2</td><td>smart column output alignment library</td><td align="right">217 KiB</td></tr>
<tr><td align="right">745</td><td><code>network-manager-pptp</code></td><td>1.2.12-6</td><td>network management framework (PPTP plugin core)</td><td align="right">216 KiB</td></tr>
<tr><td align="right">746</td><td><code>libhtml-parser-perl</code></td><td>3.83-1build1</td><td>collection of modules that parse HTML text documents</td><td align="right">214 KiB</td></tr>
<tr><td align="right">747</td><td><code>gzip</code></td><td>1.14-1~exp2ubuntu1</td><td>GNU compression utilities</td><td align="right">213 KiB</td></tr>
<tr><td align="right">748</td><td><code>libxfont2</code></td><td>1:2.0.6-2</td><td>X11 font rasterisation library</td><td align="right">212 KiB</td></tr>
<tr><td align="right">749</td><td><code>python3-pexpect</code></td><td>4.9-4</td><td>Python 3 module for automating interactive applications</td><td align="right">212 KiB</td></tr>
<tr><td align="right">750</td><td><code>bc</code></td><td>1.07.1-4build1</td><td>GNU bc arbitrary precision calculator language</td><td align="right">211 KiB</td></tr>
<tr><td align="right">751</td><td><code>libjson-glib-1.0-0</code></td><td>1.10.8+ds-2</td><td>GLib JSON manipulation library</td><td align="right">211 KiB</td></tr>
<tr><td align="right">752</td><td><code>libpam0g</code></td><td>1.7.0-5ubuntu3</td><td>Pluggable Authentication Modules library</td><td align="right">211 KiB</td></tr>
<tr><td align="right">753</td><td><code>libxml2-utils</code></td><td>2.15.2+dfsg-0.1</td><td>GNOME XML library - utilities</td><td align="right">211 KiB</td></tr>
<tr><td align="right">754</td><td><code>liblouis20</code></td><td>3.36.0-1</td><td>Braille translation library - shared libs</td><td align="right">210 KiB</td></tr>
<tr><td align="right">755</td><td><code>libvncclient1</code></td><td>0.9.15+dfsg-3</td><td>API to write one&#x27;s own VNC server - client library</td><td align="right">210 KiB</td></tr>
<tr><td align="right">756</td><td><code>gstreamer1.0-pipewire</code></td><td>1.6.2-1ubuntu1</td><td>GStreamer 1.0 plugin for the PipeWire multimedia server</td><td align="right">209 KiB</td></tr>
<tr><td align="right">757</td><td><code>libatk1.0-0t64</code></td><td>2.60.0-1</td><td>ATK accessibility toolkit</td><td align="right">209 KiB</td></tr>
<tr><td align="right">758</td><td><code>libelf1t64</code></td><td>0.194-4</td><td>library to read and write ELF files</td><td align="right">208 KiB</td></tr>
<tr><td align="right">759</td><td><code>libvorbis0a</code></td><td>1.3.7-3build2</td><td>decoder library for Vorbis General Audio Compression Codec</td><td align="right">208 KiB</td></tr>
<tr><td align="right">760</td><td><code>casper</code></td><td>26.04.2</td><td>Run a &quot;live&quot; preinstalled system from read-only media</td><td align="right">207 KiB</td></tr>
<tr><td align="right">761</td><td><code>libuchardet0</code></td><td>0.0.8-2</td><td>universal charset detection library - shared library</td><td align="right">207 KiB</td></tr>
<tr><td align="right">762</td><td><code>libgbm1</code></td><td>26.0.3-1ubuntu1</td><td>generic buffer management API -- runtime</td><td align="right">206 KiB</td></tr>
<tr><td align="right">763</td><td><code>libibverbs1</code></td><td>61.0-2ubuntu3</td><td>Library for direct userspace use of RDMA (InfiniBand/iWARP)</td><td align="right">206 KiB</td></tr>
<tr><td align="right">764</td><td><code>libxcb1</code></td><td>1.17.0-2ubuntu1</td><td>X C Binding</td><td align="right">205 KiB</td></tr>
<tr><td align="right">765</td><td><code>initramfs-tools-core</code></td><td>0.151ubuntu1</td><td>generic modular initramfs generator (core tools)</td><td align="right">204 KiB</td></tr>
<tr><td align="right">766</td><td><code>usb-creator-common</code></td><td>0.4.1build1</td><td>create a startup disk using a CD or disc image (common files)</td><td align="right">204 KiB</td></tr>
<tr><td align="right">767</td><td><code>gir1.2-udisks-2.0</code></td><td>2.10.91-1ubuntu2</td><td>GObject based library to access udisks2 - introspection data</td><td align="right">201 KiB</td></tr>
<tr><td align="right">768</td><td><code>glib-networking</code></td><td>2.80.1-1build2</td><td>network-related giomodules for GLib</td><td align="right">201 KiB</td></tr>
<tr><td align="right">769</td><td><code>libgcc-s1</code></td><td>16-20260322-1ubuntu1</td><td>GCC support library</td><td align="right">201 KiB</td></tr>
<tr><td align="right">770</td><td><code>dash</code></td><td>0.5.12-12ubuntu3</td><td>POSIX-compliant shell</td><td align="right">200 KiB</td></tr>
<tr><td align="right">771</td><td><code>libcairo-script-interpreter2</code></td><td>1.18.4-3</td><td>Cairo 2D vector graphics library (script interpreter)</td><td align="right">200 KiB</td></tr>
<tr><td align="right">772</td><td><code>libseccomp2</code></td><td>2.6.0-2ubuntu5</td><td>high level interface to Linux seccomp filter</td><td align="right">200 KiB</td></tr>
<tr><td align="right">773</td><td><code>patch</code></td><td>2.8-2build1</td><td>Apply a diff file to an original</td><td align="right">198 KiB</td></tr>
<tr><td align="right">774</td><td><code>libasound2-data</code></td><td>1.2.15.3-1ubuntu1</td><td>Configuration files and profiles for ALSA drivers</td><td align="right">197 KiB</td></tr>
<tr><td align="right">775</td><td><code>libnghttp2-14</code></td><td>1.68.0-2ubuntu0.1</td><td>library implementing HTTP/2 protocol (shared library)</td><td align="right">196 KiB</td></tr>
<tr><td align="right">776</td><td><code>login</code></td><td>1:4.16.0-2+really2.41.3-3ubuntu2</td><td>system login tools</td><td align="right">196 KiB</td></tr>
<tr><td align="right">777</td><td><code>pipewire-alsa</code></td><td>1.6.2-1ubuntu1</td><td>PipeWire ALSA plugin, for ALSA applications to output via PipeWire</td><td align="right">196 KiB</td></tr>
<tr><td align="right">778</td><td><code>libaudit1</code></td><td>1:4.1.2-1build1</td><td>Dynamic library for security auditing</td><td align="right">195 KiB</td></tr>
<tr><td align="right">779</td><td><code>libhttp-message-perl</code></td><td>7.01-1ubuntu1</td><td>perl interface to HTTP style messages</td><td align="right">195 KiB</td></tr>
<tr><td align="right">780</td><td><code>dbus-bin</code></td><td>1.16.2-2ubuntu4</td><td>simple interprocess messaging system (command line utilities)</td><td align="right">193 KiB</td></tr>
<tr><td align="right">781</td><td><code>iputils-ping</code></td><td>3:20250605-1ubuntu1</td><td>Tools to test the reachability of network hosts</td><td align="right">193 KiB</td></tr>
<tr><td align="right">782</td><td><code>libcairo-gobject2</code></td><td>1.18.4-3</td><td>Cairo 2D vector graphics library (GObject library)</td><td align="right">193 KiB</td></tr>
<tr><td align="right">783</td><td><code>libflac++11</code></td><td>1.5.0+ds-5</td><td>Free Lossless Audio Codec - C++ runtime library</td><td align="right">193 KiB</td></tr>
<tr><td align="right">784</td><td><code>acl</code></td><td>2.3.2-2</td><td>access control list - utilities</td><td align="right">192 KiB</td></tr>
<tr><td align="right">785</td><td><code>libxmlb2</code></td><td>0.3.24-2</td><td>Binary XML library</td><td align="right">192 KiB</td></tr>
<tr><td align="right">786</td><td><code>software-properties-common</code></td><td>0.120</td><td>manage the repositories that you install software from (common)</td><td align="right">192 KiB</td></tr>
<tr><td align="right">787</td><td><code>libcairomm-1.0-1v5</code></td><td>1.14.5-3</td><td>C++ wrappers for Cairo (shared libraries)</td><td align="right">190 KiB</td></tr>
<tr><td align="right">788</td><td><code>nftables</code></td><td>1.1.6-1</td><td>Program to control packet filtering rules by Netfilter project</td><td align="right">190 KiB</td></tr>
<tr><td align="right">789</td><td><code>libayatana-ido3-0.4-0</code></td><td>0.10.4-1build1</td><td>Widgets and other objects used for Ayatana Indicators</td><td align="right">189 KiB</td></tr>
<tr><td align="right">790</td><td><code>ghostscript</code></td><td>10.06.0~dfsg-3ubuntu1</td><td>interpreter for the PostScript language and for PDF</td><td align="right">187 KiB</td></tr>
<tr><td align="right">791</td><td><code>gpg-wks-client</code></td><td>2.4.8-4ubuntu3</td><td>GNU privacy guard - Web Key Service client</td><td align="right">187 KiB</td></tr>
<tr><td align="right">792</td><td><code>python3-update-manager</code></td><td>1:26.04.5</td><td>Python 3.x module for update-manager</td><td align="right">187 KiB</td></tr>
<tr><td align="right">793</td><td><code>gir1.2-harfbuzz-0.0</code></td><td>12.3.2-2</td><td>OpenType text shaping engine (GObject introspection data)</td><td align="right">186 KiB</td></tr>
<tr><td align="right">794</td><td><code>power-profiles-daemon</code></td><td>0.30-2</td><td>D-Bus service for power profile handling</td><td align="right">186 KiB</td></tr>
<tr><td align="right">795</td><td><code>liblz4-1</code></td><td>1.10.0-8</td><td>Fast LZ compression algorithm library - runtime</td><td align="right">185 KiB</td></tr>
<tr><td align="right">796</td><td><code>libwmflite-0.2-7</code></td><td>0.2.14-1</td><td>Windows metafile conversion lite library</td><td align="right">184 KiB</td></tr>
<tr><td align="right">797</td><td><code>libxcb-render0-dev</code></td><td>1.17.0-2ubuntu1</td><td>X C Binding, render extension, development files</td><td align="right">184 KiB</td></tr>
<tr><td align="right">798</td><td><code>python3-ldb</code></td><td>2:2.11.0+samba4.23.6+dfsg-1ubuntu2.1</td><td>Python 3 bindings for LDB</td><td align="right">184 KiB</td></tr>
<tr><td align="right">799</td><td><code>python3-xdg</code></td><td>0.28-3</td><td>Python 3 library to access freedesktop.org standards</td><td align="right">184 KiB</td></tr>
<tr><td align="right">800</td><td><code>libcdio19t64</code></td><td>2.2.0-4build1</td><td>library to read and control CD-ROM</td><td align="right">181 KiB</td></tr>
<tr><td align="right">801</td><td><code>printer-driver-splix</code></td><td>2.0.1-2</td><td>Driver for Samsung and Xerox SPL2 and SPLc laser printers</td><td align="right">181 KiB</td></tr>
<tr><td align="right">802</td><td><code>liblcms2-utils</code></td><td>2.17-1ubuntu0.2</td><td>Little CMS 2 color management library (utilities)</td><td align="right">180 KiB</td></tr>
<tr><td align="right">803</td><td><code>libnewt0.52</code></td><td>0.52.25-1ubuntu3</td><td>Not Erik&#x27;s Windowing Toolkit - text mode windowing with slang</td><td align="right">180 KiB</td></tr>
<tr><td align="right">804</td><td><code>at-spi2-core</code></td><td>2.60.0-1</td><td>Assistive Technology Service Provider Interface (D-Bus core)</td><td align="right">178 KiB</td></tr>
<tr><td align="right">805</td><td><code>python3-lazr.restfulclient</code></td><td>0.14.6-3build1</td><td>client for lazr.restful-based web services (Python 3)</td><td align="right">178 KiB</td></tr>
<tr><td align="right">806</td><td><code>libhfstospell11</code></td><td>0.5.4-1build5</td><td>HFST spell checker runtime libraries</td><td align="right">177 KiB</td></tr>
<tr><td align="right">807</td><td><code>liblc3-1</code></td><td>1.1.3+dfsg-1build1</td><td>Low Complexity Communication Codec (shared library)</td><td align="right">177 KiB</td></tr>
<tr><td align="right">808</td><td><code>liblzo2-2</code></td><td>2.10-3build2</td><td>data compression library</td><td align="right">177 KiB</td></tr>
<tr><td align="right">809</td><td><code>libglycin-2-dev</code></td><td>2.1.1+ds-0ubuntu1</td><td>sandboxed image loaders for GNOME</td><td align="right">174 KiB</td></tr>
<tr><td align="right">810</td><td><code>libjbig2dec0</code></td><td>0.20-1build4</td><td>JBIG2 decoder library - shared libraries</td><td align="right">174 KiB</td></tr>
<tr><td align="right">811</td><td><code>libupower-glib3</code></td><td>1.91.1-1</td><td>abstraction for power management - shared library</td><td align="right">174 KiB</td></tr>
<tr><td align="right">812</td><td><code>libatk-wrapper-java-jni</code></td><td>0.44.0-1</td><td>ATK implementation for Java using JNI (JNI bindings)</td><td align="right">173 KiB</td></tr>
<tr><td align="right">813</td><td><code>libdrm-intel1</code></td><td>2.4.131-1</td><td>Userspace interface to intel-specific kernel DRM services -- runtime</td><td align="right">173 KiB</td></tr>
<tr><td align="right">814</td><td><code>libgraphite2-3</code></td><td>1.3.14-11ubuntu1</td><td>Font rendering engine for Complex Scripts -- library</td><td align="right">172 KiB</td></tr>
<tr><td align="right">815</td><td><code>libpangoft2-1.0-0</code></td><td>1.57.0-1</td><td>Layout and rendering of internationalized text</td><td align="right">172 KiB</td></tr>
<tr><td align="right">816</td><td><code>libvolume-key1</code></td><td>0.3.12-10build2</td><td>Library for manipulating storage encryption keys and passphrases</td><td align="right">172 KiB</td></tr>
<tr><td align="right">817</td><td><code>libxkbfile1</code></td><td>1:1.1.0-1build5</td><td>X11 keyboard file manipulation library</td><td align="right">172 KiB</td></tr>
<tr><td align="right">818</td><td><code>libnl-3-200</code></td><td>3.12.0-2</td><td>library for dealing with netlink sockets</td><td align="right">171 KiB</td></tr>
<tr><td align="right">819</td><td><code>zlib1g</code></td><td>1:1.3.dfsg+really1.3.1-1ubuntu3</td><td>compression library - runtime</td><td align="right">171 KiB</td></tr>
<tr><td align="right">820</td><td><code>keyutils</code></td><td>1.6.3-6ubuntu3</td><td>Linux Key Management Utilities</td><td align="right">170 KiB</td></tr>
<tr><td align="right">821</td><td><code>python3-soupsieve</code></td><td>2.8.3-1</td><td>modern CSS selector implementation for BeautifulSoup (Python 3)</td><td align="right">169 KiB</td></tr>
<tr><td align="right">822</td><td><code>xdg-user-dirs</code></td><td>0.19-1</td><td>tool to manage well known user directories</td><td align="right">168 KiB</td></tr>
<tr><td align="right">823</td><td><code>libslirp0</code></td><td>4.9.1-1ubuntu1</td><td>General purpose TCP-IP emulator library</td><td align="right">167 KiB</td></tr>
<tr><td align="right">824</td><td><code>libdv4t64</code></td><td>1.0.0-17.1build2</td><td>software library for DV format digital video (runtime lib)</td><td align="right">166 KiB</td></tr>
<tr><td align="right">825</td><td><code>python3-typeguard</code></td><td>4.4.4-2</td><td>Run-time type checker for Python</td><td align="right">165 KiB</td></tr>
<tr><td align="right">826</td><td><code>remmina-plugin-rdp</code></td><td>1.4.40+dfsg-2ubuntu1</td><td>RDP plugin for Remmina</td><td align="right">165 KiB</td></tr>
<tr><td align="right">827</td><td><code>xxd</code></td><td>2:9.1.2141-1ubuntu4.2</td><td>tool to make (or reverse) a hex dump</td><td align="right">164 KiB</td></tr>
<tr><td align="right">828</td><td><code>libharfbuzz-gobject0</code></td><td>12.3.2-2</td><td>OpenType text shaping engine ICU backend (GObject library)</td><td align="right">163 KiB</td></tr>
<tr><td align="right">829</td><td><code>libglx0</code></td><td>1.7.0-3</td><td>Vendor neutral GL dispatch library -- GLX support</td><td align="right">162 KiB</td></tr>
<tr><td align="right">830</td><td><code>libtss2-sys1t64</code></td><td>4.1.3-6</td><td>TPM2 Software stack library - TSS and TCTI libraries</td><td align="right">162 KiB</td></tr>
<tr><td align="right">831</td><td><code>uuid-runtime</code></td><td>2.41.3-3ubuntu2</td><td>runtime components for the Universally Unique ID library</td><td align="right">162 KiB</td></tr>
<tr><td align="right">832</td><td><code>libgeoclue-2-0</code></td><td>2.7.2-2ubuntu3</td><td>convenience library to interact with geoinformation service</td><td align="right">161 KiB</td></tr>
<tr><td align="right">833</td><td><code>libpolkit-gobject-1-0</code></td><td>127-2ubuntu1</td><td>polkit Authorization API</td><td align="right">161 KiB</td></tr>
<tr><td align="right">834</td><td><code>python3-cupshelpers</code></td><td>1.5.18-4ubuntu2</td><td>Python utility modules around the CUPS printing system</td><td align="right">161 KiB</td></tr>
<tr><td align="right">835</td><td><code>libvte-2.91-common</code></td><td>0.84.0-2</td><td>Terminal emulator widget for GTK 3 - common files</td><td align="right">160 KiB</td></tr>
<tr><td align="right">836</td><td><code>libxcursor-dev</code></td><td>1:1.2.3-1build1</td><td>X cursor management library (development files)</td><td align="right">160 KiB</td></tr>
<tr><td align="right">837</td><td><code>python3-olefile</code></td><td>0.47-1build1</td><td>Python module to read/write MS OLE2 files</td><td align="right">160 KiB</td></tr>
<tr><td align="right">838</td><td><code>libinput-bin</code></td><td>1.31.1-1</td><td>input device management and event handling library - udev quirks</td><td align="right">159 KiB</td></tr>
<tr><td align="right">839</td><td><code>libpkcs11-helper1t64</code></td><td>1.31.0-1</td><td>library that simplifies the interaction with PKCS#11</td><td align="right">159 KiB</td></tr>
<tr><td align="right">840</td><td><code>ocl-icd-libopencl1</code></td><td>2.3.4-1</td><td>Generic OpenCL ICD Loader</td><td align="right">157 KiB</td></tr>
<tr><td align="right">841</td><td><code>libcc1-0</code></td><td>16-20260322-1ubuntu1</td><td>GCC cc1 plugin for GDB</td><td align="right">156 KiB</td></tr>
<tr><td align="right">842</td><td><code>libtdb1</code></td><td>2:1.4.14+samba4.23.6+dfsg-1ubuntu2.1</td><td>Trivial Database - shared library</td><td align="right">156 KiB</td></tr>
<tr><td align="right">843</td><td><code>libxcb-xkb1</code></td><td>1.17.0-2ubuntu1</td><td>X C Binding, XKEYBOARD extension</td><td align="right">156 KiB</td></tr>
<tr><td align="right">844</td><td><code>libxcb-xinput0</code></td><td>1.17.0-2ubuntu1</td><td>X C Binding, xinput extension</td><td align="right">154 KiB</td></tr>
<tr><td align="right">845</td><td><code>linux-base</code></td><td>4.15ubuntu5</td><td>Linux image base package</td><td align="right">154 KiB</td></tr>
<tr><td align="right">846</td><td><code>libtevent0t64</code></td><td>2:0.17.1+samba4.23.6+dfsg-1ubuntu2.1</td><td>talloc-based event loop library - shared library</td><td align="right">153 KiB</td></tr>
<tr><td align="right">847</td><td><code>libaa1</code></td><td>1.4p5-51.1build1</td><td>ASCII art library</td><td align="right">152 KiB</td></tr>
<tr><td align="right">848</td><td><code>libsasl2-2</code></td><td>2.1.28+dfsg1-9ubuntu3</td><td>Cyrus SASL - authentication abstraction library</td><td align="right">152 KiB</td></tr>
<tr><td align="right">849</td><td><code>pastebinit</code></td><td>1.8.0-1</td><td>command-line pastebin client</td><td align="right">152 KiB</td></tr>
<tr><td align="right">850</td><td><code>libtwolame0</code></td><td>0.4.0-2build4</td><td>MPEG Audio Layer 2 encoding library</td><td align="right">151 KiB</td></tr>
<tr><td align="right">851</td><td><code>libudev-dev</code></td><td>259.5-0ubuntu3</td><td>libudev development files</td><td align="right">151 KiB</td></tr>
<tr><td align="right">852</td><td><code>libusb-1.0-0</code></td><td>2:1.0.29-2build1</td><td>userspace USB programming library</td><td align="right">151 KiB</td></tr>
<tr><td align="right">853</td><td><code>pango1.0-tools</code></td><td>1.57.0-1</td><td>Development utilities for Pango</td><td align="right">151 KiB</td></tr>
<tr><td align="right">854</td><td><code>python3-netplan</code></td><td>1.2-1ubuntu5</td><td>Declarative network configuration Python bindings</td><td align="right">150 KiB</td></tr>
<tr><td align="right">855</td><td><code>libmpc3</code></td><td>1.3.1-3</td><td>multiple precision complex floating-point library</td><td align="right">149 KiB</td></tr>
<tr><td align="right">856</td><td><code>libxcb-glx0</code></td><td>1.17.0-2ubuntu1</td><td>X C Binding, glx extension</td><td align="right">149 KiB</td></tr>
<tr><td align="right">857</td><td><code>libgraphene-1.0-0</code></td><td>1.10.8-5build1</td><td>library of graphic data types</td><td align="right">148 KiB</td></tr>
<tr><td align="right">858</td><td><code>libnetfilter-conntrack3</code></td><td>1.1.1-1</td><td>Netfilter netlink-conntrack library</td><td align="right">148 KiB</td></tr>
<tr><td align="right">859</td><td><code>libyaml-0-2</code></td><td>0.2.5-2build3</td><td>Fast YAML 1.1 parser and emitter library</td><td align="right">148 KiB</td></tr>
<tr><td align="right">860</td><td><code>python3-inflect</code></td><td>7.5.0-1build1</td><td>Generate plurals, singular nouns, ordinals, indefinite articles (Python 3)</td><td align="right">148 KiB</td></tr>
<tr><td align="right">861</td><td><code>rtkit</code></td><td>0.14-1</td><td>Realtime Policy and Watchdog Daemon</td><td align="right">148 KiB</td></tr>
<tr><td align="right">862</td><td><code>speech-dispatcher-audio-plugins</code></td><td>0.12.1-2ubuntu1</td><td>Speech Dispatcher: Audio output plugins</td><td align="right">148 KiB</td></tr>
<tr><td align="right">863</td><td><code>ucf</code></td><td>3.0052ubuntu1</td><td>Update Configuration File(s): preserve user changes to config files</td><td align="right">148 KiB</td></tr>
<tr><td align="right">864</td><td><code>libao4</code></td><td>1.2.2+20180113-1.2ubuntu2</td><td>Cross Platform Audio Output Library</td><td align="right">147 KiB</td></tr>
<tr><td align="right">865</td><td><code>libshout3</code></td><td>2.4.6-1build3</td><td>MP3/Ogg Vorbis broadcast streaming library</td><td align="right">147 KiB</td></tr>
<tr><td align="right">866</td><td><code>libbrlapi0.8</code></td><td>6.7-1ubuntu6</td><td>braille display access via BRLTTY - shared library</td><td align="right">146 KiB</td></tr>
<tr><td align="right">867</td><td><code>libkmod2</code></td><td>34.2-2ubuntu2</td><td>libkmod shared library</td><td align="right">146 KiB</td></tr>
<tr><td align="right">868</td><td><code>xserver-common</code></td><td>2:21.1.22-1ubuntu1</td><td>common files used by various X servers</td><td align="right">146 KiB</td></tr>
<tr><td align="right">869</td><td><code>libspeex1</code></td><td>1.2.1-3build1</td><td>Speex codec library (runtime library)</td><td align="right">145 KiB</td></tr>
<tr><td align="right">870</td><td><code>logrotate</code></td><td>3.22.0-1build1</td><td>Log rotation utility</td><td align="right">145 KiB</td></tr>
<tr><td align="right">871</td><td><code>libcrack2</code></td><td>2.9.6-5.2build3</td><td>pro-active password checker library</td><td align="right">144 KiB</td></tr>
<tr><td align="right">872</td><td><code>libfribidi0</code></td><td>1.0.16-5</td><td>Free Implementation of the Unicode BiDi algorithm</td><td align="right">144 KiB</td></tr>
<tr><td align="right">873</td><td><code>python-apt-common</code></td><td>3.1.0ubuntu1</td><td>Python interface to libapt-pkg (locales)</td><td align="right">144 KiB</td></tr>
<tr><td align="right">874</td><td><code>iio-sensor-proxy</code></td><td>3.8-1</td><td>IIO sensors to D-Bus proxy</td><td align="right">142 KiB</td></tr>
<tr><td align="right">875</td><td><code>libevdev2</code></td><td>1.13.6+dfsg-1</td><td>wrapper library for evdev devices</td><td align="right">142 KiB</td></tr>
<tr><td align="right">876</td><td><code>python3-speechd</code></td><td>0.12.1-2ubuntu1</td><td>Python interface to Speech Dispatcher</td><td align="right">142 KiB</td></tr>
<tr><td align="right">877</td><td><code>dbus</code></td><td>1.16.2-2ubuntu4</td><td>simple interprocess messaging system (system message bus)</td><td align="right">141 KiB</td></tr>
<tr><td align="right">878</td><td><code>libeis1</code></td><td>1.5.0-3</td><td>Emulated Input server library</td><td align="right">141 KiB</td></tr>
<tr><td align="right">879</td><td><code>libnss-mdns</code></td><td>0.15.1-5</td><td>NSS module for Multicast DNS name resolution</td><td align="right">141 KiB</td></tr>
<tr><td align="right">880</td><td><code>librtmp1</code></td><td>2.4+20151223.gitfa8646d.1-3</td><td>toolkit for RTMP streams (shared library)</td><td align="right">141 KiB</td></tr>
<tr><td align="right">881</td><td><code>pipewire</code></td><td>1.6.2-1ubuntu1</td><td>audio and video processing engine multimedia server</td><td align="right">141 KiB</td></tr>
<tr><td align="right">882</td><td><code>libsframe3</code></td><td>2.46-3ubuntu2</td><td>Library to handle the SFrame format (runtime library)</td><td align="right">140 KiB</td></tr>
<tr><td align="right">883</td><td><code>libipt2</code></td><td>2.1.2-3</td><td>Intel Processor Trace Decoder Library</td><td align="right">139 KiB</td></tr>
<tr><td align="right">884</td><td><code>tdb-tools</code></td><td>2:1.4.14+samba4.23.6+dfsg-1ubuntu2.1</td><td>Trivial Database - bundled binaries</td><td align="right">139 KiB</td></tr>
<tr><td align="right">885</td><td><code>gir1.2-pango-1.0</code></td><td>1.57.0-1</td><td>Layout and rendering of internationalized text - gir bindings</td><td align="right">138 KiB</td></tr>
<tr><td align="right">886</td><td><code>libmtp-common</code></td><td>1.1.22-1ubuntu1</td><td>Media Transfer Protocol (MTP) common files</td><td align="right">138 KiB</td></tr>
<tr><td align="right">887</td><td><code>libatopology2t64</code></td><td>1.2.15.3-1ubuntu1</td><td>shared library for handling ALSA topology definitions</td><td align="right">137 KiB</td></tr>
<tr><td align="right">888</td><td><code>libsamplerate0-dev</code></td><td>0.2.2-4build2</td><td>Development files for audio sample rate conversion</td><td align="right">137 KiB</td></tr>
<tr><td align="right">889</td><td><code>switcheroo-control</code></td><td>3.0-2</td><td>D-Bus service to check the availability of dual-GPU</td><td align="right">137 KiB</td></tr>
<tr><td align="right">890</td><td><code>libauthen-sasl-perl</code></td><td>2.2000-1</td><td>Authen::SASL - SASL Authentication framework</td><td align="right">136 KiB</td></tr>
<tr><td align="right">891</td><td><code>libdbusmenu-glib4</code></td><td>18.10.20180917~bzr492+repack1-4build1</td><td>library for passing menus over DBus</td><td align="right">136 KiB</td></tr>
<tr><td align="right">892</td><td><code>libei1</code></td><td>1.5.0-3</td><td>Emulated Input client library</td><td align="right">136 KiB</td></tr>
<tr><td align="right">893</td><td><code>libkrb5support0</code></td><td>1.22.1-2ubuntu4</td><td>MIT Kerberos runtime libraries - Support library</td><td align="right">136 KiB</td></tr>
<tr><td align="right">894</td><td><code>libpoppler-cpp3</code></td><td>26.01.0-2build2</td><td>PDF rendering library (CPP shared library)</td><td align="right">136 KiB</td></tr>
<tr><td align="right">895</td><td><code>libthai-dev</code></td><td>0.1.30-1</td><td>Development files for Thai language support library</td><td align="right">136 KiB</td></tr>
<tr><td align="right">896</td><td><code>cups-bsd</code></td><td>2.4.16-1ubuntu1</td><td>Common UNIX Printing System(tm) - BSD commands</td><td align="right">135 KiB</td></tr>
<tr><td align="right">897</td><td><code>libcdparanoia0</code></td><td>3.10.2+debian-14ubuntu2</td><td>audio extraction tool for sampling CDs (library)</td><td align="right">135 KiB</td></tr>
<tr><td align="right">898</td><td><code>luit</code></td><td>2.0.20250912-1</td><td>locale and ISO 2022 support for Unicode terminals</td><td align="right">135 KiB</td></tr>
<tr><td align="right">899</td><td><code>cups-core-drivers</code></td><td>2.4.16-1ubuntu1</td><td>Common UNIX Printing System(tm) - driverless printing</td><td align="right">134 KiB</td></tr>
<tr><td align="right">900</td><td><code>libcolorhug2</code></td><td>1.4.8-3</td><td>library to access the ColorHug colourimeter -- runtime</td><td align="right">134 KiB</td></tr>
<tr><td align="right">901</td><td><code>libwbclient0</code></td><td>2:4.23.6+dfsg-1ubuntu2.1</td><td>Samba winbind client library</td><td align="right">134 KiB</td></tr>
<tr><td align="right">902</td><td><code>bubblewrap</code></td><td>0.11.1-1ubuntu0.1</td><td>utility for unprivileged chroot and namespace manipulation</td><td align="right">133 KiB</td></tr>
<tr><td align="right">903</td><td><code>init-system-helpers</code></td><td>1.69</td><td>helper tools for all init systems</td><td align="right">133 KiB</td></tr>
<tr><td align="right">904</td><td><code>libcap2-bin</code></td><td>1:2.75-10ubuntu2</td><td>POSIX 1003.1e capabilities (utilities)</td><td align="right">133 KiB</td></tr>
<tr><td align="right">905</td><td><code>uuid-dev</code></td><td>2.41.3-3ubuntu2</td><td>Universally Unique ID library - headers and static libraries</td><td align="right">133 KiB</td></tr>
<tr><td align="right">906</td><td><code>dc</code></td><td>1.07.1-4build1</td><td>GNU dc arbitrary precision reverse-polish calculator</td><td align="right">130 KiB</td></tr>
<tr><td align="right">907</td><td><code>parted</code></td><td>3.6-6</td><td>disk partition manipulator</td><td align="right">130 KiB</td></tr>
<tr><td align="right">908</td><td><code>python3-httplib2</code></td><td>0.22.0-1build1</td><td>comprehensive HTTP client library written for Python3</td><td align="right">130 KiB</td></tr>
<tr><td align="right">909</td><td><code>python3-software-properties</code></td><td>0.120</td><td>manage the repositories that you install software from</td><td align="right">130 KiB</td></tr>
<tr><td align="right">910</td><td><code>usb-modeswitch</code></td><td>2.6.2-1ubuntu1</td><td>mode switching tool for controlling &quot;flip flop&quot; USB devices</td><td align="right">130 KiB</td></tr>
<tr><td align="right">911</td><td><code>libbsd0</code></td><td>0.12.2-2build2</td><td>utility functions from BSD systems - shared library</td><td align="right">129 KiB</td></tr>
<tr><td align="right">912</td><td><code>libgusb2a</code></td><td>0.4.9-7</td><td>GLib wrapper around libusb1</td><td align="right">129 KiB</td></tr>
<tr><td align="right">913</td><td><code>libblockdev-fs3</code></td><td>3.4.0-1</td><td>file system plugin for libblockdev</td><td align="right">128 KiB</td></tr>
<tr><td align="right">914</td><td><code>libplist-2.0-4</code></td><td>2.7.0+git20250820-1build1</td><td>Library for handling Apple binary and XML property lists</td><td align="right">128 KiB</td></tr>
<tr><td align="right">915</td><td><code>libfile-fcntllock-perl</code></td><td>0.22-4ubuntu6</td><td>Perl module for file locking with fcntl(2)</td><td align="right">127 KiB</td></tr>
<tr><td align="right">916</td><td><code>libpkgconf7</code></td><td>2.5.1-4</td><td>shared library for pkgconf</td><td align="right">127 KiB</td></tr>
<tr><td align="right">917</td><td><code>libtasn1-6</code></td><td>4.21.0-2</td><td>Manage ASN.1 structures (runtime)</td><td align="right">127 KiB</td></tr>
<tr><td align="right">918</td><td><code>libxml-xpathengine-perl</code></td><td>0.14-2</td><td>re-usable XPath engine for DOM-like trees</td><td align="right">127 KiB</td></tr>
<tr><td align="right">919</td><td><code>libxrandr-dev</code></td><td>2:1.5.4-1build1</td><td>X11 RandR extension library (development headers)</td><td align="right">126 KiB</td></tr>
<tr><td align="right">920</td><td><code>printer-driver-c2esp</code></td><td>27-11ubuntu8</td><td>printer driver for Kodak ESP AiO color inkjet Series</td><td align="right">126 KiB</td></tr>
<tr><td align="right">921</td><td><code>libtheoradec2</code></td><td>1.2.0+dfsg-6</td><td>Theora Video Compression Codec (shared library - libtheoradec)</td><td align="right">125 KiB</td></tr>
<tr><td align="right">922</td><td><code>libxmu6</code></td><td>2:1.1.3-4</td><td>X11 miscellaneous utility library</td><td align="right">125 KiB</td></tr>
<tr><td align="right">923</td><td><code>python3-platformdirs</code></td><td>4.9.4-1</td><td>determining appropriate platform-specific directories (Python 3)</td><td align="right">125 KiB</td></tr>
<tr><td align="right">924</td><td><code>libpopt0</code></td><td>1.19+dfsg-2build1</td><td>lib for parsing cmdline parameters</td><td align="right">124 KiB</td></tr>
<tr><td align="right">925</td><td><code>usbmuxd</code></td><td>1.1.1-7</td><td>USB multiplexor daemon for iPhone and iPod Touch devices</td><td align="right">124 KiB</td></tr>
<tr><td align="right">926</td><td><code>isa-support</code></td><td>27ubuntu2</td><td>CPU feature checking - common back-end</td><td align="right">123 KiB</td></tr>
<tr><td align="right">927</td><td><code>libtimedate-perl</code></td><td>2.3300-2</td><td>collection of modules to manipulate date/time information</td><td align="right">123 KiB</td></tr>
<tr><td align="right">928</td><td><code>jq</code></td><td>1.8.1-4ubuntu2</td><td>lightweight and flexible command-line JSON processor</td><td align="right">122 KiB</td></tr>
<tr><td align="right">929</td><td><code>libavahi-client3</code></td><td>0.8-18ubuntu1.1</td><td>Avahi client library</td><td align="right">122 KiB</td></tr>
<tr><td align="right">930</td><td><code>libefiboot1t64</code></td><td>39-2</td><td>Library to manage UEFI variables</td><td align="right">122 KiB</td></tr>
<tr><td align="right">931</td><td><code>firefox</code></td><td>1:1snap1-0ubuntu8</td><td>Installs Firefox snap and provides some system integration</td><td align="right">121 KiB</td></tr>
<tr><td align="right">932</td><td><code>libio-stringy-perl</code></td><td>2.113-2</td><td>modules for I/O on in-core objects (strings/arrays)</td><td align="right">120 KiB</td></tr>
<tr><td align="right">933</td><td><code>update-manager-core</code></td><td>1:26.04.5</td><td>manage release upgrades</td><td align="right">120 KiB</td></tr>
<tr><td align="right">934</td><td><code>libdrm2</code></td><td>2.4.131-1</td><td>Userspace interface to kernel DRM services -- runtime</td><td align="right">119 KiB</td></tr>
<tr><td align="right">935</td><td><code>libhtml-format-perl</code></td><td>2.16-2</td><td>module for transforming HTML into various formats</td><td align="right">119 KiB</td></tr>
<tr><td align="right">936</td><td><code>libitm1</code></td><td>16-20260322-1ubuntu1</td><td>GNU Transactional Memory Library</td><td align="right">119 KiB</td></tr>
<tr><td align="right">937</td><td><code>libjcat1</code></td><td>0.2.5-1build1</td><td>JSON catalog library</td><td align="right">119 KiB</td></tr>
<tr><td align="right">938</td><td><code>dbus-x11</code></td><td>1.16.2-2ubuntu4</td><td>simple interprocess messaging system (X11 deps)</td><td align="right">118 KiB</td></tr>
<tr><td align="right">939</td><td><code>libcanberra0</code></td><td>0.30-18ubuntu3</td><td>simple abstract interface for playing event sounds</td><td align="right">118 KiB</td></tr>
<tr><td align="right">940</td><td><code>libegl-dev</code></td><td>1.7.0-3</td><td>Vendor neutral GL dispatch library -- EGL development files</td><td align="right">118 KiB</td></tr>
<tr><td align="right">941</td><td><code>libxft2</code></td><td>2.3.6-1build2</td><td>FreeType-based font drawing library for X</td><td align="right">118 KiB</td></tr>
<tr><td align="right">942</td><td><code>libxrender-dev</code></td><td>1:0.9.12-1build1</td><td>X Rendering Extension client library (development files)</td><td align="right">118 KiB</td></tr>
<tr><td align="right">943</td><td><code>libapparmor1</code></td><td>5.0.0~beta1-0ubuntu7</td><td>changehat AppArmor library</td><td align="right">117 KiB</td></tr>
<tr><td align="right">944</td><td><code>libpci3</code></td><td>1:3.14.0-1build2</td><td>PCI utilities (shared library)</td><td align="right">117 KiB</td></tr>
<tr><td align="right">945</td><td><code>liburing2</code></td><td>2.14-1</td><td>Linux kernel io_uring access library - shared library</td><td align="right">116 KiB</td></tr>
<tr><td align="right">946</td><td><code>cryptsetup-initramfs</code></td><td>2:2.8.4-1ubuntu4</td><td>disk encryption support - initramfs integration</td><td align="right">115 KiB</td></tr>
<tr><td align="right">947</td><td><code>samba-common</code></td><td>2:4.23.6+dfsg-1ubuntu2.1</td><td>common files used by both the Samba server and client</td><td align="right">115 KiB</td></tr>
<tr><td align="right">948</td><td><code>libassuan9</code></td><td>3.0.2-2build1</td><td>IPC library for the GnuPG components</td><td align="right">114 KiB</td></tr>
<tr><td align="right">949</td><td><code>libice6</code></td><td>2:1.1.1-1build1</td><td>X11 Inter-Client Exchange library</td><td align="right">114 KiB</td></tr>
<tr><td align="right">950</td><td><code>xml-core</code></td><td>0.19build1</td><td>XML infrastructure and XML catalog file support</td><td align="right">114 KiB</td></tr>
<tr><td align="right">951</td><td><code>libc-dev-bin</code></td><td>2.43-2ubuntu2</td><td>GNU C Library: Development binaries</td><td align="right">113 KiB</td></tr>
<tr><td align="right">952</td><td><code>libpskc0t64</code></td><td>2.6.14-1</td><td>OATH Toolkit Libpskc library</td><td align="right">113 KiB</td></tr>
<tr><td align="right">953</td><td><code>liblmdb0</code></td><td>0.9.31-1build2</td><td>Lightning Memory-Mapped Database shared library</td><td align="right">112 KiB</td></tr>
<tr><td align="right">954</td><td><code>libtalloc2</code></td><td>2:2.4.3+samba4.23.6+dfsg-1ubuntu2.1</td><td>hierarchical pool based memory allocator</td><td align="right">112 KiB</td></tr>
<tr><td align="right">955</td><td><code>python3-xkit</code></td><td>0.5.0ubuntu8</td><td>library for the manipulation of xorg.conf files (Python 3)</td><td align="right">112 KiB</td></tr>
<tr><td align="right">956</td><td><code>sysvinit-utils</code></td><td>3.15-5ubuntu1</td><td>System-V-like utilities</td><td align="right">112 KiB</td></tr>
<tr><td align="right">957</td><td><code>bzip2</code></td><td>1.0.8-6build2</td><td>high-quality block-sorting file compressor - utilities</td><td align="right">111 KiB</td></tr>
<tr><td align="right">958</td><td><code>libsdl2-2.0-0</code></td><td>2.32.10+dfsg-6</td><td>Simple DirectMedia Layer</td><td align="right">111 KiB</td></tr>
<tr><td align="right">959</td><td><code>gir1.2-packagekitglib-1.0</code></td><td>1.3.4-3ubuntu1</td><td>GObject introspection data for the PackageKit GLib library</td><td align="right">110 KiB</td></tr>
<tr><td align="right">960</td><td><code>librabbitmq4</code></td><td>0.15.0-1build2</td><td>AMQP client library written in C</td><td align="right">110 KiB</td></tr>
<tr><td align="right">961</td><td><code>python3-minimal</code></td><td>3.14.3-0ubuntu2</td><td>minimal subset of the Python language (default python3 version)</td><td align="right">110 KiB</td></tr>
<tr><td align="right">962</td><td><code>libpangocairo-1.0-0</code></td><td>1.57.0-1</td><td>Layout and rendering of internationalized text</td><td align="right">109 KiB</td></tr>
<tr><td align="right">963</td><td><code>libwayland-server0</code></td><td>1.24.0-2</td><td>wayland compositor infrastructure - server library</td><td align="right">109 KiB</td></tr>
<tr><td align="right">964</td><td><code>gir1.2-girepository-3.0</code></td><td>2.88.0-1</td><td>Introspection data for GIRepository library, API version 3.0</td><td align="right">108 KiB</td></tr>
<tr><td align="right">965</td><td><code>gtk-update-icon-cache</code></td><td>4.22.2+ds-1ubuntu1</td><td>icon theme caching utility</td><td align="right">108 KiB</td></tr>
<tr><td align="right">966</td><td><code>libfile-mimeinfo-perl</code></td><td>0.36-2</td><td>Perl module to determine file types</td><td align="right">108 KiB</td></tr>
<tr><td align="right">967</td><td><code>libglib2.0-data</code></td><td>2.88.0-1</td><td>Common files for GLib library</td><td align="right">108 KiB</td></tr>
<tr><td align="right">968</td><td><code>libpsl5t64</code></td><td>0.21.2-1.1build2</td><td>Library for Public Suffix List (shared libraries)</td><td align="right">108 KiB</td></tr>
<tr><td align="right">969</td><td><code>dracut-install</code></td><td>110-11</td><td>dracut is an event driven initramfs infrastructure (dracut-install)</td><td align="right">107 KiB</td></tr>
<tr><td align="right">970</td><td><code>libdeflate0</code></td><td>1.23-2ubuntu1</td><td>fast, whole-buffer DEFLATE-based compression and decompression</td><td align="right">107 KiB</td></tr>
<tr><td align="right">971</td><td><code>libvdpau1</code></td><td>1.5-4</td><td>Video Decode and Presentation API for Unix (libraries)</td><td align="right">107 KiB</td></tr>
<tr><td align="right">972</td><td><code>libwrap0</code></td><td>7.6.q-36build2</td><td>Wietse Venema&#x27;s TCP wrappers library</td><td align="right">107 KiB</td></tr>
<tr><td align="right">973</td><td><code>gcc-15-base</code></td><td>15.2.0-16ubuntu1</td><td>GCC, the GNU Compiler Collection (base package)</td><td align="right">106 KiB</td></tr>
<tr><td align="right">974</td><td><code>libgnutls-dane0t64</code></td><td>3.8.12-2ubuntu1.1</td><td>GNU TLS library - DANE security support</td><td align="right">106 KiB</td></tr>
<tr><td align="right">975</td><td><code>pptp-linux</code></td><td>1.10.0-2build1</td><td>Point-to-Point Tunneling Protocol (PPTP) Client</td><td align="right">106 KiB</td></tr>
<tr><td align="right">976</td><td><code>gcc-16-base</code></td><td>16-20260322-1ubuntu1</td><td>GCC, the GNU Compiler Collection (base package)</td><td align="right">105 KiB</td></tr>
<tr><td align="right">977</td><td><code>libgles2</code></td><td>1.7.0-3</td><td>Vendor neutral GL dispatch library -- GLESv2 support</td><td align="right">105 KiB</td></tr>
<tr><td align="right">978</td><td><code>libgtk3-perl</code></td><td>0.038-3</td><td>Perl bindings for the GTK+ graphical user interface library</td><td align="right">105 KiB</td></tr>
<tr><td align="right">979</td><td><code>netcat-openbsd</code></td><td>1.234-1</td><td>TCP/IP swiss army knife</td><td align="right">105 KiB</td></tr>
<tr><td align="right">980</td><td><code>python3-jwt</code></td><td>2.10.1-4ubuntu1</td><td>Python 3 implementation of JSON Web Token</td><td align="right">105 KiB</td></tr>
<tr><td align="right">981</td><td><code>krb5-locales</code></td><td>1.22.1-2ubuntu4</td><td>internationalization support for MIT Kerberos</td><td align="right">104 KiB</td></tr>
<tr><td align="right">982</td><td><code>libayatana-indicator3-7</code></td><td>0.9.4-2</td><td>panel indicator applet - shared library (GTK-3+ variant)</td><td align="right">104 KiB</td></tr>
<tr><td align="right">983</td><td><code>libbz2-dev</code></td><td>1.0.8-6build2</td><td>high-quality block-sorting file compressor library - development</td><td align="right">104 KiB</td></tr>
<tr><td align="right">984</td><td><code>libxtables12</code></td><td>1.8.11-2ubuntu3</td><td>netfilter xtables library</td><td align="right">104 KiB</td></tr>
<tr><td align="right">985</td><td><code>printer-driver-min12xxw</code></td><td>0.0.9-11build4</td><td>printer driver for KonicaMinolta PagePro 1[234]xxW</td><td align="right">104 KiB</td></tr>
<tr><td align="right">986</td><td><code>libdconf1</code></td><td>0.49.0-4</td><td>simple configuration storage system - runtime library</td><td align="right">103 KiB</td></tr>
<tr><td align="right">987</td><td><code>libjson-c5</code></td><td>0.18+ds-3</td><td>JSON manipulation library - shared library</td><td align="right">103 KiB</td></tr>
<tr><td align="right">988</td><td><code>libklibc</code></td><td>2.0.14-1ubuntu2</td><td>minimal libc subset for use with initramfs</td><td align="right">103 KiB</td></tr>
<tr><td align="right">989</td><td><code>libxext6</code></td><td>2:1.3.4-1build3</td><td>X11 miscellaneous extension library</td><td align="right">103 KiB</td></tr>
<tr><td align="right">990</td><td><code>login.defs</code></td><td>1:4.17.4-2ubuntu3</td><td>system user management configuration</td><td align="right">103 KiB</td></tr>
<tr><td align="right">991</td><td><code>eject</code></td><td>2.41.3-3ubuntu2</td><td>ejects CDs and operates CD-Changers under Linux</td><td align="right">102 KiB</td></tr>
<tr><td align="right">992</td><td><code>libdbusmenu-gtk3-4</code></td><td>18.10.20180917~bzr492+repack1-4build1</td><td>library for passing menus over DBus - GTK-3+ version</td><td align="right">102 KiB</td></tr>
<tr><td align="right">993</td><td><code>libimobiledevice-glue-1.0-0</code></td><td>1.3.2-2build1</td><td>Common library used by the libimobiledevice project</td><td align="right">102 KiB</td></tr>
<tr><td align="right">994</td><td><code>libspeexdsp1</code></td><td>1.2.1-3build1</td><td>DSP library derived from speex</td><td align="right">102 KiB</td></tr>
<tr><td align="right">995</td><td><code>libproxy1v5</code></td><td>0.5.12-1</td><td>automatic proxy configuration management library (shared)</td><td align="right">101 KiB</td></tr>
<tr><td align="right">996</td><td><code>openssh-sftp-server</code></td><td>1:10.2p1-2ubuntu3.2</td><td>secure shell (SSH) sftp server module, for SFTP access from remote machines</td><td align="right">101 KiB</td></tr>
<tr><td align="right">997</td><td><code>gir1.2-atk-1.0</code></td><td>2.60.0-1</td><td>ATK accessibility toolkit (GObject introspection)</td><td align="right">100 KiB</td></tr>
<tr><td align="right">998</td><td><code>iucode-tool</code></td><td>2.3.1-3build2</td><td>Intel processor microcode tool</td><td align="right">100 KiB</td></tr>
<tr><td align="right">999</td><td><code>pinentry-curses</code></td><td>1.3.2-3ubuntu1</td><td>curses-based PIN or pass-phrase entry dialog for GnuPG</td><td align="right">100 KiB</td></tr>
<tr><td align="right">1000</td><td><code>anacron</code></td><td>2.3-45ubuntu1</td><td>cron-like program that doesn&#x27;t go by time</td><td align="right">99 KiB</td></tr>
<tr><td align="right">1001</td><td><code>libavahi-common3</code></td><td>0.8-18ubuntu1.1</td><td>Avahi common library</td><td align="right">99 KiB</td></tr>
<tr><td align="right">1002</td><td><code>libbz2-1.0</code></td><td>1.0.8-6build2</td><td>high-quality block-sorting file compressor library - runtime</td><td align="right">99 KiB</td></tr>
<tr><td align="right">1003</td><td><code>libegl1</code></td><td>1.7.0-3</td><td>Vendor neutral GL dispatch library -- EGL support</td><td align="right">99 KiB</td></tr>
<tr><td align="right">1004</td><td><code>libldap-common</code></td><td>2.6.10+dfsg-1ubuntu5</td><td>OpenLDAP common files for libraries</td><td align="right">99 KiB</td></tr>
<tr><td align="right">1005</td><td><code>libxi6</code></td><td>2:1.8.2-2</td><td>X11 Input extension library</td><td align="right">99 KiB</td></tr>
<tr><td align="right">1006</td><td><code>media-types</code></td><td>14.0.0build1</td><td>List of standard media types and their usual file extension</td><td align="right">99 KiB</td></tr>
<tr><td align="right">1007</td><td><code>python3-talloc</code></td><td>2:2.4.3+samba4.23.6+dfsg-1ubuntu2.1</td><td>hierarchical pool based memory allocator - Python3 bindings</td><td align="right">99 KiB</td></tr>
<tr><td align="right">1008</td><td><code>libmarkdown2</code></td><td>2.2.7-2.1build1</td><td>implementation of the Markdown markup language in C (library)</td><td align="right">98 KiB</td></tr>
<tr><td align="right">1009</td><td><code>dconf-service</code></td><td>0.49.0-4</td><td>simple configuration storage system - D-Bus service</td><td align="right">97 KiB</td></tr>
<tr><td align="right">1010</td><td><code>fuse3</code></td><td>3.18.2-1</td><td>Filesystem in Userspace (3.x version)</td><td align="right">97 KiB</td></tr>
<tr><td align="right">1011</td><td><code>libavahi-ui-gtk3-0</code></td><td>0.8-18ubuntu1.1</td><td>Avahi GTK+ User interface library for GTK3</td><td align="right">97 KiB</td></tr>
<tr><td align="right">1012</td><td><code>libgdbm6t64</code></td><td>1.26-1build1</td><td>GNU dbm database routines (runtime version)</td><td align="right">97 KiB</td></tr>
<tr><td align="right">1013</td><td><code>libthai0</code></td><td>0.1.30-1</td><td>Thai language support library</td><td align="right">97 KiB</td></tr>
<tr><td align="right">1014</td><td><code>libxcb-randr0</code></td><td>1.17.0-2ubuntu1</td><td>X C Binding, randr extension</td><td align="right">97 KiB</td></tr>
<tr><td align="right">1015</td><td><code>python3-tdb</code></td><td>2:1.4.14+samba4.23.6+dfsg-1ubuntu2.1</td><td>Python3 bindings for TDB</td><td align="right">97 KiB</td></tr>
<tr><td align="right">1016</td><td><code>libavahi-common-data</code></td><td>0.8-18ubuntu1.1</td><td>Avahi common data files</td><td align="right">96 KiB</td></tr>
<tr><td align="right">1017</td><td><code>libpciaccess-dev</code></td><td>0.18.1-1ubuntu4</td><td>Generic PCI access library for X - development files</td><td align="right">96 KiB</td></tr>
<tr><td align="right">1018</td><td><code>libpng-tools</code></td><td>1.6.57-1</td><td>PNG library - tools (version 1.6)</td><td align="right">96 KiB</td></tr>
<tr><td align="right">1019</td><td><code>libxpm4</code></td><td>1:3.5.17-1build3</td><td>X11 pixmap library</td><td align="right">96 KiB</td></tr>
<tr><td align="right">1020</td><td><code>gir1.2-atspi-2.0</code></td><td>2.60.0-1</td><td>Assistive Technology Service Provider (GObject introspection)</td><td align="right">95 KiB</td></tr>
<tr><td align="right">1021</td><td><code>libjansson4</code></td><td>2.14-2build4</td><td>C library for encoding, decoding and manipulating JSON data</td><td align="right">95 KiB</td></tr>
<tr><td align="right">1022</td><td><code>dbus-system-bus-common</code></td><td>1.16.2-2ubuntu4</td><td>simple interprocess messaging system (system bus configuration)</td><td align="right">94 KiB</td></tr>
<tr><td align="right">1023</td><td><code>libmount-dev</code></td><td>2.41.3-3ubuntu2</td><td>device mounting library - headers</td><td align="right">94 KiB</td></tr>
<tr><td align="right">1024</td><td><code>libcap2</code></td><td>1:2.75-10ubuntu2</td><td>POSIX 1003.1e capabilities (library)</td><td align="right">93 KiB</td></tr>
<tr><td align="right">1025</td><td><code>libsnappy1v5</code></td><td>1.2.2-2</td><td>fast compression/decompression library</td><td align="right">93 KiB</td></tr>
<tr><td align="right">1026</td><td><code>libhtml-form-perl</code></td><td>6.13-1build1</td><td>module that represents an HTML form element</td><td align="right">92 KiB</td></tr>
<tr><td align="right">1027</td><td><code>libsbc1</code></td><td>2.1-1build1</td><td>Sub Band CODEC library - runtime</td><td align="right">92 KiB</td></tr>
<tr><td align="right">1028</td><td><code>pcmciautils</code></td><td>018-19</td><td>PCMCIA utilities for Linux 2.6</td><td align="right">92 KiB</td></tr>
<tr><td align="right">1029</td><td><code>python3-cssselect</code></td><td>1.4.0-1</td><td>cssselect parses CSS3 Selectors and translates them to XPath 1.0</td><td align="right">92 KiB</td></tr>
<tr><td align="right">1030</td><td><code>printer-driver-brlaser</code></td><td>6.2.8-1</td><td>printer driver for (some) Brother laser printers</td><td align="right">91 KiB</td></tr>
<tr><td align="right">1031</td><td><code>libblockdev-crypto3</code></td><td>3.4.0-1</td><td>Crypto plugin for libblockdev</td><td align="right">90 KiB</td></tr>
<tr><td align="right">1032</td><td><code>libharfbuzz-cairo0</code></td><td>12.3.2-2</td><td>OpenType text shaping engine Cairo backend</td><td align="right">90 KiB</td></tr>
<tr><td align="right">1033</td><td><code>libuuid1</code></td><td>2.41.3-3ubuntu2</td><td>Universally Unique ID library</td><td align="right">90 KiB</td></tr>
<tr><td align="right">1034</td><td><code>libwayland-client0</code></td><td>1.24.0-2</td><td>wayland compositor infrastructure - client library</td><td align="right">90 KiB</td></tr>
<tr><td align="right">1035</td><td><code>libxxhash0</code></td><td>0.8.3-2build1</td><td>shared library for xxhash</td><td align="right">90 KiB</td></tr>
<tr><td align="right">1036</td><td><code>libodbcinst2</code></td><td>2.3.14-1</td><td>Support library for accessing ODBC configuration files</td><td align="right">89 KiB</td></tr>
<tr><td align="right">1037</td><td><code>libayatana-appindicator3-1</code></td><td>0.5.94-1build1</td><td>Ayatana Application Indicators (GTK-3+ version)</td><td align="right">88 KiB</td></tr>
<tr><td align="right">1038</td><td><code>libftdi1-2</code></td><td>1.6~rc1-1build1</td><td>C Library to control and program the FTDI USB controllers</td><td align="right">88 KiB</td></tr>
<tr><td align="right">1039</td><td><code>libglib2.0-dev</code></td><td>2.88.0-1</td><td>Development metapackage for the GLib family of libraries</td><td align="right">88 KiB</td></tr>
<tr><td align="right">1040</td><td><code>libpipeline1</code></td><td>1.5.8-2</td><td>Unix process pipeline manipulation library</td><td align="right">88 KiB</td></tr>
<tr><td align="right">1041</td><td><code>libsensors5</code></td><td>1:3.6.2-2build1</td><td>library to read temperature/voltage/fan sensors</td><td align="right">88 KiB</td></tr>
<tr><td align="right">1042</td><td><code>libxv-dev</code></td><td>2:1.0.13-1</td><td>X11 Video extension library (development headers)</td><td align="right">88 KiB</td></tr>
<tr><td align="right">1043</td><td><code>dbus-user-session</code></td><td>1.16.2-2ubuntu4</td><td>simple interprocess messaging system (systemd --user integration)</td><td align="right">87 KiB</td></tr>
<tr><td align="right">1044</td><td><code>libffi8</code></td><td>3.5.2-4</td><td>Foreign Function Interface library runtime</td><td align="right">87 KiB</td></tr>
<tr><td align="right">1045</td><td><code>libglx-dev</code></td><td>1.7.0-3</td><td>Vendor neutral GL dispatch library -- GLX development files</td><td align="right">87 KiB</td></tr>
<tr><td align="right">1046</td><td><code>libsm-dev</code></td><td>2:1.2.6-1build1</td><td>X11 Session Management library (development headers)</td><td align="right">87 KiB</td></tr>
<tr><td align="right">1047</td><td><code>rfkill</code></td><td>2.41.3-3ubuntu2</td><td>tool for enabling and disabling wireless devices</td><td align="right">87 KiB</td></tr>
<tr><td align="right">1048</td><td><code>whoopsie</code></td><td>0.2.82ubuntu</td><td>Ubuntu error tracker submission</td><td align="right">87 KiB</td></tr>
<tr><td align="right">1049</td><td><code>libcbor0.10</code></td><td>0.10.2-2ubuntu3</td><td>library for parsing and generating CBOR (RFC 7049)</td><td align="right">85 KiB</td></tr>
<tr><td align="right">1050</td><td><code>libjbig0</code></td><td>2.1-6.1ubuntu3</td><td>JBIGkit libraries</td><td align="right">85 KiB</td></tr>
<tr><td align="right">1051</td><td><code>os-prober</code></td><td>1.84ubuntu1</td><td>utility to detect other OSes on a set of drives</td><td align="right">85 KiB</td></tr>
<tr><td align="right">1052</td><td><code>pkgconf-bin</code></td><td>2.5.1-4</td><td>manage compile and link flags for libraries (binaries)</td><td align="right">85 KiB</td></tr>
<tr><td align="right">1053</td><td><code>bluez-cups</code></td><td>5.85-4</td><td>Bluetooth printer driver for CUPS</td><td align="right">84 KiB</td></tr>
<tr><td align="right">1054</td><td><code>libglib2.0-dev-bin</code></td><td>2.88.0-1</td><td>Development utilities for the GLib library</td><td align="right">84 KiB</td></tr>
<tr><td align="right">1055</td><td><code>libiec61883-0</code></td><td>1.2.0-8</td><td>partial implementation of IEC 61883 (shared lib)</td><td align="right">84 KiB</td></tr>
<tr><td align="right">1056</td><td><code>usb-modeswitch-data</code></td><td>20191128-7build1</td><td>mode switching data for usb-modeswitch</td><td align="right">84 KiB</td></tr>
<tr><td align="right">1057</td><td><code>dbus-session-bus-common</code></td><td>1.16.2-2ubuntu4</td><td>simple interprocess messaging system (session bus configuration)</td><td align="right">83 KiB</td></tr>
<tr><td align="right">1058</td><td><code>libdecor-0-plugin-1-gtk</code></td><td>0.2.5-1</td><td>libdecor decoration plugin using GTK</td><td align="right">83 KiB</td></tr>
<tr><td align="right">1059</td><td><code>libltdl7</code></td><td>2.5.4-9</td><td>System independent dlopen wrapper for GNU libtool</td><td align="right">83 KiB</td></tr>
<tr><td align="right">1060</td><td><code>libminiupnpc21</code></td><td>2.3.3-2build1</td><td>UPnP IGD client lightweight library</td><td align="right">83 KiB</td></tr>
<tr><td align="right">1061</td><td><code>libraw1394-11</code></td><td>2.1.2-2build4</td><td>library for direct access to IEEE 1394 bus (aka FireWire)</td><td align="right">83 KiB</td></tr>
<tr><td align="right">1062</td><td><code>libwacom9</code></td><td>2.18.0-1</td><td>Wacom model feature query library</td><td align="right">83 KiB</td></tr>
<tr><td align="right">1063</td><td><code>printer-driver-pxljr</code></td><td>1.4+repack0-6build3</td><td>printer driver for HP Color LaserJet 35xx/36xx</td><td align="right">83 KiB</td></tr>
<tr><td align="right">1064</td><td><code>ibus-gtk3</code></td><td>1.5.34~rc2-1</td><td>Intelligent Input Bus - GTK3 support</td><td align="right">82 KiB</td></tr>
<tr><td align="right">1065</td><td><code>ibus-gtk4</code></td><td>1.5.34~rc2-1</td><td>Intelligent Input Bus - GTK4 support</td><td align="right">82 KiB</td></tr>
<tr><td align="right">1066</td><td><code>mokutil</code></td><td>0.7.2-2</td><td>tools for manipulating machine owner keys</td><td align="right">82 KiB</td></tr>
<tr><td align="right">1067</td><td><code>fwupd-signed</code></td><td>1.55+1.7-1</td><td>Linux Firmware Updater EFI signed binary</td><td align="right">81 KiB</td></tr>
<tr><td align="right">1068</td><td><code>initramfs-tools-bin</code></td><td>0.151ubuntu1</td><td>generic modular initramfs generator (binary tools)</td><td align="right">81 KiB</td></tr>
<tr><td align="right">1069</td><td><code>libdatrie-dev</code></td><td>0.2.14-1</td><td>Development files for double-array trie library</td><td align="right">81 KiB</td></tr>
<tr><td align="right">1070</td><td><code>libxcb-render0</code></td><td>1.17.0-2ubuntu1</td><td>X C Binding, render extension</td><td align="right">81 KiB</td></tr>
<tr><td align="right">1071</td><td><code>readline-common</code></td><td>8.3-4</td><td>GNU readline and history libraries, common files</td><td align="right">81 KiB</td></tr>
<tr><td align="right">1072</td><td><code>update-inetd</code></td><td>4.54build1</td><td>inetd configuration file updater</td><td align="right">81 KiB</td></tr>
<tr><td align="right">1073</td><td><code>libatasmart4</code></td><td>0.19-6</td><td>ATA S.M.A.R.T. reading and parsing library</td><td align="right">80 KiB</td></tr>
<tr><td align="right">1074</td><td><code>libexiv2-data</code></td><td>0.28.8+dfsg-1</td><td>EXIF/IPTC/XMP metadata manipulation library - shared data</td><td align="right">80 KiB</td></tr>
<tr><td align="right">1075</td><td><code>libnotify4</code></td><td>0.8.8-1</td><td>sends desktop notifications to a notification daemon</td><td align="right">80 KiB</td></tr>
<tr><td align="right">1076</td><td><code>libpangoxft-1.0-0</code></td><td>1.57.0-1</td><td>Layout and rendering of internationalized text</td><td align="right">80 KiB</td></tr>
<tr><td align="right">1077</td><td><code>python3</code></td><td>3.14.3-0ubuntu2</td><td>interactive high-level object-oriented language (default python3 version)</td><td align="right">80 KiB</td></tr>
<tr><td align="right">1078</td><td><code>speech-dispatcher-espeak-ng</code></td><td>0.12.1-2ubuntu1</td><td>Speech Dispatcher: Espeak-ng output module</td><td align="right">80 KiB</td></tr>
<tr><td align="right">1079</td><td><code>systemd-sysv</code></td><td>259.5-0ubuntu3</td><td>system and service manager - SysV compatibility symlinks</td><td align="right">80 KiB</td></tr>
<tr><td align="right">1080</td><td><code>ubuntu-pro-client-l10n</code></td><td>37.2ubuntu</td><td>Translations for Ubuntu Pro Client</td><td align="right">80 KiB</td></tr>
<tr><td align="right">1081</td><td><code>libmd0</code></td><td>1.1.0-2build4</td><td>message digest functions from BSD systems - shared library</td><td align="right">79 KiB</td></tr>
<tr><td align="right">1082</td><td><code>python3-distro</code></td><td>1.9.0-1build1</td><td>Linux OS platform information API</td><td align="right">79 KiB</td></tr>
<tr><td align="right">1083</td><td><code>libdebuginfod1t64</code></td><td>0.194-4</td><td>library to interact with debuginfod (development files)</td><td align="right">78 KiB</td></tr>
<tr><td align="right">1084</td><td><code>libss2</code></td><td>1.47.2-3ubuntu4</td><td>command-line interface parsing library</td><td align="right">78 KiB</td></tr>
<tr><td align="right">1085</td><td><code>python3-linkify-it</code></td><td>2.0.3-1ubuntu3</td><td>links recognition library with FULL unicode support</td><td align="right">78 KiB</td></tr>
<tr><td align="right">1086</td><td><code>python3-pylibacl</code></td><td>0.7.2-1build2</td><td>module for manipulating POSIX.1e ACLs (Python3 version)</td><td align="right">78 KiB</td></tr>
<tr><td align="right">1087</td><td><code>libpcsclite1</code></td><td>2.4.1-1</td><td>Middleware to access a smart card using PC/SC (library)</td><td align="right">77 KiB</td></tr>
<tr><td align="right">1088</td><td><code>xorg-sgml-doctools</code></td><td>1:1.11-1.1build1</td><td>Common tools for building X.Org SGML documentation</td><td align="right">77 KiB</td></tr>
<tr><td align="right">1089</td><td><code>libgif7</code></td><td>5.2.2-1ubuntu3</td><td>library for GIF images (library)</td><td align="right">76 KiB</td></tr>
<tr><td align="right">1090</td><td><code>libgphoto2-l10n</code></td><td>2.5.33-1ubuntu1</td><td>gphoto2 digital camera library - localized messages</td><td align="right">76 KiB</td></tr>
<tr><td align="right">1091</td><td><code>libsharpyuv-dev</code></td><td>1.5.0-0.1build1</td><td>Library for sharp RGB to YUV conversion (development files)</td><td align="right">76 KiB</td></tr>
<tr><td align="right">1092</td><td><code>libwebpmux3</code></td><td>1.5.0-0.1build1</td><td>Lossy compression of digital photographic images (libwebpmux library)</td><td align="right">76 KiB</td></tr>
<tr><td align="right">1093</td><td><code>libpulse-mainloop-glib0</code></td><td>1:17.0+dfsg1-2ubuntu4</td><td>PulseAudio client libraries (glib support)</td><td align="right">75 KiB</td></tr>
<tr><td align="right">1094</td><td><code>libsasl2-modules-db</code></td><td>2.1.28+dfsg1-9ubuntu3</td><td>Cyrus SASL - pluggable authentication modules (DB)</td><td align="right">75 KiB</td></tr>
<tr><td align="right">1095</td><td><code>libxxf86vm-dev</code></td><td>1:1.1.4-2</td><td>X11 XFree86 video mode extension library (development headers)</td><td align="right">75 KiB</td></tr>
<tr><td align="right">1096</td><td><code>dconf-gsettings-backend</code></td><td>0.49.0-4</td><td>simple configuration storage system - GSettings back-end</td><td align="right">74 KiB</td></tr>
<tr><td align="right">1097</td><td><code>libacl1</code></td><td>2.3.2-2</td><td>access control list - shared library</td><td align="right">74 KiB</td></tr>
<tr><td align="right">1098</td><td><code>libdrm-radeon1</code></td><td>2.4.131-1</td><td>Userspace interface to radeon-specific kernel DRM services -- runtime</td><td align="right">74 KiB</td></tr>
<tr><td align="right">1099</td><td><code>liblirc-client0t64</code></td><td>0.10.2-0.10ubuntu1</td><td>infra-red remote control support - client library</td><td align="right">74 KiB</td></tr>
<tr><td align="right">1100</td><td><code>libpixman-1-dev</code></td><td>0.46.4-1</td><td>pixel-manipulation library for X and cairo (development files)</td><td align="right">74 KiB</td></tr>
<tr><td align="right">1101</td><td><code>python3-lazr.uri</code></td><td>1.0.6-7build1</td><td>library for parsing, manipulating, and generating URIs</td><td align="right">74 KiB</td></tr>
<tr><td align="right">1102</td><td><code>libgles1</code></td><td>1.7.0-3</td><td>Vendor neutral GL dispatch library -- GLESv1 support</td><td align="right">73 KiB</td></tr>
<tr><td align="right">1103</td><td><code>libx11-xcb1</code></td><td>2:1.8.13-1</td><td>Xlib/XCB interface library</td><td align="right">73 KiB</td></tr>
<tr><td align="right">1104</td><td><code>libxtst-dev</code></td><td>2:1.2.5-1build1</td><td>X11 Record extension library (development headers)</td><td align="right">73 KiB</td></tr>
<tr><td align="right">1105</td><td><code>pkgconf</code></td><td>2.5.1-4</td><td>manage compile and link flags for libraries</td><td align="right">73 KiB</td></tr>
<tr><td align="right">1106</td><td><code>pnp.ids</code></td><td>0.394-1build1</td><td>PNP ID Registry</td><td align="right">73 KiB</td></tr>
<tr><td align="right">1107</td><td><code>xauth</code></td><td>1:1.1.2-1.1build1</td><td>X authentication utility</td><td align="right">73 KiB</td></tr>
<tr><td align="right">1108</td><td><code>libblockdev-smart3</code></td><td>3.4.0-1</td><td>SMART plugin for libblockdev</td><td align="right">72 KiB</td></tr>
<tr><td align="right">1109</td><td><code>libdrm-amdgpu1</code></td><td>2.4.131-1</td><td>Userspace interface to amdgpu-specific kernel DRM services -- runtime</td><td align="right">72 KiB</td></tr>
<tr><td align="right">1110</td><td><code>libgbm-dev</code></td><td>26.0.3-1ubuntu1</td><td>generic buffer management API -- development files</td><td align="right">72 KiB</td></tr>
<tr><td align="right">1111</td><td><code>libgraphite2-dev</code></td><td>1.3.14-11ubuntu1</td><td>Development files for libgraphite2</td><td align="right">72 KiB</td></tr>
<tr><td align="right">1112</td><td><code>libheif-plugin-aomenc</code></td><td>1.21.2-3</td><td>HEIF and AVIF file format decoder and encoder - aomenc plugin</td><td align="right">72 KiB</td></tr>
<tr><td align="right">1113</td><td><code>liblastlog2-2</code></td><td>2.41.3-3ubuntu2</td><td>lastlog2 database shared library</td><td align="right">72 KiB</td></tr>
<tr><td align="right">1114</td><td><code>liblwp-mediatypes-perl</code></td><td>6.04-2</td><td>module to guess media type for a file or a URL</td><td align="right">72 KiB</td></tr>
<tr><td align="right">1115</td><td><code>remmina-plugin-vnc</code></td><td>1.4.40+dfsg-2ubuntu1</td><td>VNC plugin for Remmina</td><td align="right">72 KiB</td></tr>
<tr><td align="right">1116</td><td><code>distro-info</code></td><td>1.15</td><td>provides information about the distributions&#x27; releases</td><td align="right">71 KiB</td></tr>
<tr><td align="right">1117</td><td><code>libasm1t64</code></td><td>0.194-4</td><td>library with a programmable assembler interface</td><td align="right">71 KiB</td></tr>
<tr><td align="right">1118</td><td><code>libldacbt-enc2</code></td><td>2.0.2.3+git20200429+ed310a0-5build1</td><td>LDAC Bluetooth encoder library (shared library)</td><td align="right">71 KiB</td></tr>
<tr><td align="right">1119</td><td><code>libnuma1</code></td><td>2.0.19-1build1</td><td>Libraries for controlling NUMA policy</td><td align="right">71 KiB</td></tr>
<tr><td align="right">1120</td><td><code>libatk-wrapper-java</code></td><td>0.44.0-1</td><td>ATK implementation for Java using JNI</td><td align="right">70 KiB</td></tr>
<tr><td align="right">1121</td><td><code>libpolkit-agent-1-0</code></td><td>127-2ubuntu1</td><td>polkit Authentication Agent API</td><td align="right">70 KiB</td></tr>
<tr><td align="right">1122</td><td><code>system-config-printer-udev</code></td><td>1.5.18-4ubuntu2</td><td>Utilities to detect and configure printers automatically</td><td align="right">70 KiB</td></tr>
<tr><td align="right">1123</td><td><code>libblockdev-nvme3</code></td><td>3.4.0-1</td><td>NVMe plugin for libblockdev</td><td align="right">69 KiB</td></tr>
<tr><td align="right">1124</td><td><code>libfastjson4</code></td><td>1.2304.0-2build1</td><td>fast json library for C</td><td align="right">69 KiB</td></tr>
<tr><td align="right">1125</td><td><code>libgudev-1.0-0</code></td><td>1:238-7build1</td><td>GObject-based wrapper library for libudev</td><td align="right">69 KiB</td></tr>
<tr><td align="right">1126</td><td><code>libspeechd2</code></td><td>0.12.1-2ubuntu1</td><td>Speech Dispatcher: Shared libraries</td><td align="right">69 KiB</td></tr>
<tr><td align="right">1127</td><td><code>libfont-afm-perl</code></td><td>1.20-4</td><td>Perl interface to Adobe Font Metrics files</td><td align="right">68 KiB</td></tr>
<tr><td align="right">1128</td><td><code>libieee1284-3t64</code></td><td>0.2.11-14.1build2</td><td>cross-platform library for parallel port access</td><td align="right">68 KiB</td></tr>
<tr><td align="right">1129</td><td><code>libip4tc2</code></td><td>1.8.11-2ubuntu3</td><td>netfilter libip4tc library</td><td align="right">68 KiB</td></tr>
<tr><td align="right">1130</td><td><code>libip6tc2</code></td><td>1.8.11-2ubuntu3</td><td>netfilter libip6tc library</td><td align="right">68 KiB</td></tr>
<tr><td align="right">1131</td><td><code>libpaper2</code></td><td>2.2.5-0.3maysync1</td><td>Paper sizes catalogue</td><td align="right">68 KiB</td></tr>
<tr><td align="right">1132</td><td><code>libpwquality-common</code></td><td>1.4.5-5build1</td><td>library for password quality checking and generation (data files)</td><td align="right">68 KiB</td></tr>
<tr><td align="right">1133</td><td><code>liberror-perl</code></td><td>0.17030-1</td><td>Perl module for error/exception handling in an OO-ish way</td><td align="right">67 KiB</td></tr>
<tr><td align="right">1134</td><td><code>libodbccr2</code></td><td>2.3.14-1</td><td>ODBC Cursor library for Unix</td><td align="right">67 KiB</td></tr>
<tr><td align="right">1135</td><td><code>libxdmcp-dev</code></td><td>1:1.1.5-2</td><td>X11 authorisation library (development headers)</td><td align="right">67 KiB</td></tr>
<tr><td align="right">1136</td><td><code>networkd-dispatcher</code></td><td>2.2.4-1.1ubuntu1</td><td>Dispatcher service for systemd-networkd connection status changes</td><td align="right">67 KiB</td></tr>
<tr><td align="right">1137</td><td><code>ssl-cert</code></td><td>1.1.3ubuntu2</td><td>simple debconf wrapper for OpenSSL</td><td align="right">67 KiB</td></tr>
<tr><td align="right">1138</td><td><code>libcom-err2</code></td><td>1.47.2-3ubuntu4</td><td>common error description library</td><td align="right">66 KiB</td></tr>
<tr><td align="right">1139</td><td><code>libpaper-utils</code></td><td>2.2.5-0.3maysync1</td><td>Paper sizes catalogue (utilities)</td><td align="right">66 KiB</td></tr>
<tr><td align="right">1140</td><td><code>libxrandr2</code></td><td>2:1.5.4-1build1</td><td>X11 RandR extension library</td><td align="right">66 KiB</td></tr>
<tr><td align="right">1141</td><td><code>sensible-utils</code></td><td>0.0.26build1</td><td>Utilities for sensible alternative selection</td><td align="right">66 KiB</td></tr>
<tr><td align="right">1142</td><td><code>inputattach</code></td><td>1:1.8.1-2build2</td><td>utility to connect serial-attached peripherals to the input subsystem</td><td align="right">65 KiB</td></tr>
<tr><td align="right">1143</td><td><code>libblockdev-utils3</code></td><td>3.4.0-1</td><td>Utility functions for libblockdev</td><td align="right">65 KiB</td></tr>
<tr><td align="right">1144</td><td><code>sgml-base</code></td><td>1.31+nmu1build1</td><td>SGML infrastructure and SGML catalog file support</td><td align="right">65 KiB</td></tr>
<tr><td align="right">1145</td><td><code>libblockdev-part3</code></td><td>3.4.0-1</td><td>Partitioning plugin for libblockdev</td><td align="right">64 KiB</td></tr>
<tr><td align="right">1146</td><td><code>libdata-dump-perl</code></td><td>1.25-1</td><td>Perl module to help dump data structures</td><td align="right">64 KiB</td></tr>
<tr><td align="right">1147</td><td><code>libwayland-bin</code></td><td>1.24.0-2</td><td>wayland compositor infrastructure - binary utilities</td><td align="right">64 KiB</td></tr>
<tr><td align="right">1148</td><td><code>libglvnd-core-dev</code></td><td>1.7.0-3</td><td>Vendor neutral GL dispatch library -- core development files</td><td align="right">63 KiB</td></tr>
<tr><td align="right">1149</td><td><code>printer-driver-ptouch</code></td><td>1.7.1-1build1</td><td>printer driver Brother P-touch label printers</td><td align="right">63 KiB</td></tr>
<tr><td align="right">1150</td><td><code>gir1.2-wnck-3.0</code></td><td>43.3-1build1</td><td>GObject introspection data for the WNCK library</td><td align="right">62 KiB</td></tr>
<tr><td align="right">1151</td><td><code>libdrm-nouveau2</code></td><td>2.4.131-1</td><td>Userspace interface to nouveau-specific kernel DRM services -- runtime</td><td align="right">62 KiB</td></tr>
<tr><td align="right">1152</td><td><code>libstartup-notification0</code></td><td>0.12-8build1</td><td>library for program launch feedback (shared library)</td><td align="right">62 KiB</td></tr>
<tr><td align="right">1153</td><td><code>libxcursor1</code></td><td>1:1.2.3-1build1</td><td>X cursor management library</td><td align="right">62 KiB</td></tr>
<tr><td align="right">1154</td><td><code>emacsen-common</code></td><td>3.0.8build1</td><td>Common facilities for all emacsen</td><td align="right">61 KiB</td></tr>
<tr><td align="right">1155</td><td><code>libattr1</code></td><td>1:2.5.2-4</td><td>extended attribute handling - shared library</td><td align="right">61 KiB</td></tr>
<tr><td align="right">1156</td><td><code>libavc1394-0</code></td><td>0.5.4-5build4</td><td>control IEEE 1394 audio/video devices</td><td align="right">61 KiB</td></tr>
<tr><td align="right">1157</td><td><code>libpciaccess0</code></td><td>0.18.1-1ubuntu4</td><td>Generic PCI access library for X</td><td align="right">61 KiB</td></tr>
<tr><td align="right">1158</td><td><code>libqrtr-glib0</code></td><td>1.2.2-2ubuntu1</td><td>Support library to use the QRTR protocol</td><td align="right">61 KiB</td></tr>
<tr><td align="right">1159</td><td><code>python3-autocommand</code></td><td>2.2.2-4</td><td>library to generate argparse parsers from function signatures</td><td align="right">61 KiB</td></tr>
<tr><td align="right">1160</td><td><code>python3-pyxattr</code></td><td>0.8.1-1build6</td><td>module for manipulating filesystem extended attributes (Python3)</td><td align="right">61 KiB</td></tr>
<tr><td align="right">1161</td><td><code>apport-symptoms</code></td><td>0.25build1</td><td>symptom scripts for apport</td><td align="right">60 KiB</td></tr>
<tr><td align="right">1162</td><td><code>file</code></td><td>1:5.46-5build2</td><td>Recognize the type of data in a file using &quot;magic&quot; numbers</td><td align="right">60 KiB</td></tr>
<tr><td align="right">1163</td><td><code>libavahi-glib1</code></td><td>0.8-18ubuntu1.1</td><td>Avahi GLib integration library</td><td align="right">60 KiB</td></tr>
<tr><td align="right">1164</td><td><code>libegl1-mesa-dev</code></td><td>26.0.3-1ubuntu1</td><td>free implementation of the EGL API -- development files</td><td align="right">60 KiB</td></tr>
<tr><td align="right">1165</td><td><code>libgpg-error-l10n</code></td><td>1.58-2</td><td>library of error values and messages in GnuPG (localization files)</td><td align="right">60 KiB</td></tr>
<tr><td align="right">1166</td><td><code>libogg0</code></td><td>1.3.6-2</td><td>Ogg bitstream library</td><td align="right">60 KiB</td></tr>
<tr><td align="right">1167</td><td><code>libwnck-3-common</code></td><td>43.3-1build1</td><td>Window Navigator Construction Kit - common files</td><td align="right">60 KiB</td></tr>
<tr><td align="right">1168</td><td><code>logsave</code></td><td>1.47.2-3ubuntu4</td><td>save the output of a command in a log file</td><td align="right">60 KiB</td></tr>
<tr><td align="right">1169</td><td><code>libcap-ng0</code></td><td>0.8.5-4build5</td><td>alternate POSIX capabilities library</td><td align="right">59 KiB</td></tr>
<tr><td align="right">1170</td><td><code>libdecor-0-0</code></td><td>0.2.5-1</td><td>client-side window decoration library</td><td align="right">59 KiB</td></tr>
<tr><td align="right">1171</td><td><code>libipc-system-simple-perl</code></td><td>1.30-2</td><td>Perl module to run commands simply, with detailed diagnostics</td><td align="right">59 KiB</td></tr>
<tr><td align="right">1172</td><td><code>libnet-http-perl</code></td><td>6.24-1build1</td><td>module providing low-level HTTP connection client</td><td align="right">59 KiB</td></tr>
<tr><td align="right">1173</td><td><code>libxfixes-dev</code></td><td>1:6.0.0-2build2</td><td>X11 miscellaneous &#x27;fixes&#x27; extension library (development headers)</td><td align="right">59 KiB</td></tr>
<tr><td align="right">1174</td><td><code>python3-ptyprocess</code></td><td>0.7.0-6build1</td><td>Run a subprocess in a pseudo terminal from Python 3</td><td align="right">59 KiB</td></tr>
<tr><td align="right">1175</td><td><code>python3-setproctitle</code></td><td>1.3.7-2build1</td><td>Setproctitle implementation for Python 3</td><td align="right">59 KiB</td></tr>
<tr><td align="right">1176</td><td><code>libcairo-gobject-perl</code></td><td>1.005-4build4</td><td>integrate Cairo into the Glib type system in Perl</td><td align="right">58 KiB</td></tr>
<tr><td align="right">1177</td><td><code>libpam-fprintd</code></td><td>1.94.5-4</td><td>PAM module for fingerprint authentication through fprintd</td><td align="right">58 KiB</td></tr>
<tr><td align="right">1178</td><td><code>libvorbisfile3</code></td><td>1.3.7-3build2</td><td>high-level API for Vorbis General Audio Compression Codec</td><td align="right">58 KiB</td></tr>
<tr><td align="right">1179</td><td><code>libxrender1</code></td><td>1:0.9.12-1build1</td><td>X Rendering Extension client library</td><td align="right">58 KiB</td></tr>
<tr><td align="right">1180</td><td><code>initramfs-tools</code></td><td>0.151ubuntu1</td><td>generic modular initramfs generator (automation)</td><td align="right">57 KiB</td></tr>
<tr><td align="right">1181</td><td><code>libcrypt-urandom-perl</code></td><td>0.55-1</td><td>module that provides non blocking randomness</td><td align="right">57 KiB</td></tr>
<tr><td align="right">1182</td><td><code>libhttp-daemon-perl</code></td><td>6.16-1</td><td>simple http server class</td><td align="right">57 KiB</td></tr>
<tr><td align="right">1183</td><td><code>libsigc++-2.0-0v5</code></td><td>2.12.1-4build1</td><td>type-safe Signal Framework for C++ - runtime</td><td align="right">57 KiB</td></tr>
<tr><td align="right">1184</td><td><code>libsm6</code></td><td>2:1.2.6-1build1</td><td>X11 Session Management library</td><td align="right">57 KiB</td></tr>
<tr><td align="right">1185</td><td><code>libusbmuxd-2.0-7</code></td><td>2.1.1-1</td><td>Client library to handle usbmux connections with iOS devices (library)</td><td align="right">57 KiB</td></tr>
<tr><td align="right">1186</td><td><code>libxkbcommon-x11-0</code></td><td>1.13.1-1</td><td>library to create keymaps with the XKB X11 protocol</td><td align="right">57 KiB</td></tr>
<tr><td align="right">1187</td><td><code>python3-decorator</code></td><td>5.2.1-2</td><td>simplify usage of Python decorators by programmers</td><td align="right">57 KiB</td></tr>
<tr><td align="right">1188</td><td><code>libblockdev-mdraid3</code></td><td>3.4.0-1</td><td>MD RAID plugin for libblockdev</td><td align="right">56 KiB</td></tr>
<tr><td align="right">1189</td><td><code>libpam-cap</code></td><td>1:2.75-10ubuntu2</td><td>POSIX 1003.1e capabilities (PAM module)</td><td align="right">56 KiB</td></tr>
<tr><td align="right">1190</td><td><code>libtss2-tcti-mssim0t64</code></td><td>4.1.3-6</td><td>TPM2 Software stack library - TSS and TCTI libraries</td><td align="right">56 KiB</td></tr>
<tr><td align="right">1191</td><td><code>libtss2-tcti-swtpm0t64</code></td><td>4.1.3-6</td><td>TPM2 Software stack library - TSS and TCTI libraries</td><td align="right">56 KiB</td></tr>
<tr><td align="right">1192</td><td><code>libxcb-xfixes0</code></td><td>1.17.0-2ubuntu1</td><td>X C Binding, xfixes extension</td><td align="right">56 KiB</td></tr>
<tr><td align="right">1193</td><td><code>python3-netifaces</code></td><td>0.11.0-2build7</td><td>portable network interface information - Python 3.x</td><td align="right">56 KiB</td></tr>
<tr><td align="right">1194</td><td><code>python3-webencodings</code></td><td>0.5.1-5build1</td><td>Python implementation of the WHATWG Encoding standard</td><td align="right">56 KiB</td></tr>
<tr><td align="right">1195</td><td><code>libcupsimage2t64</code></td><td>2.4.16-1ubuntu1</td><td>Common UNIX Printing System(tm) - Raster image library</td><td align="right">55 KiB</td></tr>
<tr><td align="right">1196</td><td><code>libxcb-shm0-dev</code></td><td>1.17.0-2ubuntu1</td><td>X C Binding, shm extension, development files</td><td align="right">55 KiB</td></tr>
<tr><td align="right">1197</td><td><code>libxcb-xv0</code></td><td>1.17.0-2ubuntu1</td><td>X C Binding, xv extension</td><td align="right">55 KiB</td></tr>
<tr><td align="right">1198</td><td><code>libxkbregistry0</code></td><td>1.13.1-1</td><td>library to query available RMLVO</td><td align="right">55 KiB</td></tr>
<tr><td align="right">1199</td><td><code>pipewire-pulse</code></td><td>1.6.2-1ubuntu1</td><td>PipeWire PulseAudio daemon</td><td align="right">55 KiB</td></tr>
<tr><td align="right">1200</td><td><code>pkexec</code></td><td>127-2ubuntu1</td><td>run commands as another user with polkit authorization</td><td align="right">55 KiB</td></tr>
<tr><td align="right">1201</td><td><code>whiptail</code></td><td>0.52.25-1ubuntu3</td><td>Displays user-friendly dialog boxes from shell scripts</td><td align="right">55 KiB</td></tr>
<tr><td align="right">1202</td><td><code>cron-daemon-common</code></td><td>3.0pl1-200ubuntu1</td><td>process scheduling daemon&#x27;s configuration files</td><td align="right">54 KiB</td></tr>
<tr><td align="right">1203</td><td><code>libcdio-cdda2t64</code></td><td>10.2+2.0.2-2</td><td>library to read and control digital audio CDs</td><td align="right">54 KiB</td></tr>
<tr><td align="right">1204</td><td><code>libharfbuzz-icu0</code></td><td>12.3.2-2</td><td>OpenType text shaping engine ICU backend</td><td align="right">54 KiB</td></tr>
<tr><td align="right">1205</td><td><code>libxcb-sync1</code></td><td>1.17.0-2ubuntu1</td><td>X C Binding, sync extension</td><td align="right">54 KiB</td></tr>
<tr><td align="right">1206</td><td><code>libcdio-paranoia2t64</code></td><td>10.2+2.0.2-2</td><td>library to read digital audio CDs with error correction</td><td align="right">53 KiB</td></tr>
<tr><td align="right">1207</td><td><code>libdatrie1</code></td><td>0.2.14-1</td><td>Double-array trie library</td><td align="right">53 KiB</td></tr>
<tr><td align="right">1208</td><td><code>libfontenc1</code></td><td>1:1.1.8-1build2</td><td>X11 font encoding library</td><td align="right">53 KiB</td></tr>
<tr><td align="right">1209</td><td><code>liblocale-gettext-perl</code></td><td>1.07-8</td><td>module using libc functions for internationalization in Perl</td><td align="right">53 KiB</td></tr>
<tr><td align="right">1210</td><td><code>libxnvctrl0</code></td><td>510.47.03-0ubuntu7</td><td>NV-CONTROL X extension (runtime library)</td><td align="right">53 KiB</td></tr>
<tr><td align="right">1211</td><td><code>python3-louis</code></td><td>3.36.0-1</td><td>Python bindings for liblouis</td><td align="right">53 KiB</td></tr>
<tr><td align="right">1212</td><td><code>python3-problem-report</code></td><td>2.34.0-0ubuntu2</td><td>Python 3 library to handle problem reports</td><td align="right">53 KiB</td></tr>
<tr><td align="right">1213</td><td><code>at-spi2-common</code></td><td>2.60.0-1</td><td>Assistive Technology Service Provider Interface (common files)</td><td align="right">52 KiB</td></tr>
<tr><td align="right">1214</td><td><code>libgdk-pixbuf2.0-bin</code></td><td>2.44.5+dfsg-4ubuntu1</td><td>GDK Pixbuf library (thumbnailer)</td><td align="right">52 KiB</td></tr>
<tr><td align="right">1215</td><td><code>libinireader0</code></td><td>61-1ubuntu1</td><td>simple .INI file parser for C++</td><td align="right">52 KiB</td></tr>
<tr><td align="right">1216</td><td><code>libnl-genl-3-200</code></td><td>3.12.0-2</td><td>library for dealing with netlink sockets - generic netlink</td><td align="right">52 KiB</td></tr>
<tr><td align="right">1217</td><td><code>libpipewire-0.3-common</code></td><td>1.6.2-1ubuntu1</td><td>libraries for the PipeWire multimedia server - common files</td><td align="right">52 KiB</td></tr>
<tr><td align="right">1218</td><td><code>libtss2-tcti-cmd0t64</code></td><td>4.1.3-6</td><td>TPM2 Software stack library - TSS and TCTI libraries</td><td align="right">52 KiB</td></tr>
<tr><td align="right">1219</td><td><code>libtss2-tcti-device0t64</code></td><td>4.1.3-6</td><td>TPM2 Software stack library - TSS and TCTI libraries</td><td align="right">52 KiB</td></tr>
<tr><td align="right">1220</td><td><code>libtss2-tcti-libtpms0t64</code></td><td>4.1.3-6</td><td>TPM2 Software stack library - TSS and TCTI libraries</td><td align="right">52 KiB</td></tr>
<tr><td align="right">1221</td><td><code>libtss2-tcti-spi-helper0t64</code></td><td>4.1.3-6</td><td>TPM2 Software stack library - TSS and TCTI libraries</td><td align="right">52 KiB</td></tr>
<tr><td align="right">1222</td><td><code>libtss2-tctildr0t64</code></td><td>4.1.3-6</td><td>TPM2 Software stack library - TSS and TCTI libraries</td><td align="right">52 KiB</td></tr>
<tr><td align="right">1223</td><td><code>libwayland-cursor0</code></td><td>1.24.0-2</td><td>wayland compositor infrastructure - cursor library</td><td align="right">52 KiB</td></tr>
<tr><td align="right">1224</td><td><code>libxau-dev</code></td><td>1:1.0.11-1build2</td><td>X11 authorisation library (development headers)</td><td align="right">52 KiB</td></tr>
<tr><td align="right">1225</td><td><code>network-manager-l10n</code></td><td>1.54.3-2ubuntu3</td><td>network management framework (translation files)</td><td align="right">52 KiB</td></tr>
<tr><td align="right">1226</td><td><code>libdaemon0</code></td><td>0.14-7.1ubuntu5</td><td>lightweight C library for daemons - runtime library</td><td align="right">51 KiB</td></tr>
<tr><td align="right">1227</td><td><code>libnfnetlink0</code></td><td>1.0.2-3build1</td><td>Netfilter netlink library</td><td align="right">51 KiB</td></tr>
<tr><td align="right">1228</td><td><code>libxss-dev</code></td><td>1:1.2.3-1build4</td><td>X11 Screen Saver extension library (development headers)</td><td align="right">51 KiB</td></tr>
<tr><td align="right">1229</td><td><code>gir1.2-gly-2</code></td><td>2.1.1+ds-0ubuntu1</td><td>sandboxed image loaders for GNOME</td><td align="right">50 KiB</td></tr>
<tr><td align="right">1230</td><td><code>hostname</code></td><td>3.25build1</td><td>utility to set/show the host name or domain name</td><td align="right">50 KiB</td></tr>
<tr><td align="right">1231</td><td><code>libeditorconfig0</code></td><td>0.12.10+~0.17.1-3ubuntu0.1</td><td>coding style indenter across editors - library</td><td align="right">50 KiB</td></tr>
<tr><td align="right">1232</td><td><code>libatomic1</code></td><td>16-20260322-1ubuntu1</td><td>support library providing __atomic built-in functions</td><td align="right">49 KiB</td></tr>
<tr><td align="right">1233</td><td><code>libdebuginfod-common</code></td><td>0.194-4</td><td>configuration to enable the Debian debug info server</td><td align="right">49 KiB</td></tr>
<tr><td align="right">1234</td><td><code>libhttp-cookies-perl</code></td><td>6.11-1</td><td>HTTP cookie jars</td><td align="right">49 KiB</td></tr>
<tr><td align="right">1235</td><td><code>libndp0</code></td><td>1.9-1build1</td><td>Library for Neighbor Discovery Protocol</td><td align="right">49 KiB</td></tr>
<tr><td align="right">1236</td><td><code>plymouth-theme-ubuntu-text</code></td><td>24.004.60+git20250831.4a3c171d-0ubuntu8</td><td>boot animation, logger and I/O multiplexer - ubuntu text theme</td><td align="right">49 KiB</td></tr>
<tr><td align="right">1237</td><td><code>libasyncns0</code></td><td>0.8-7</td><td>Asynchronous name service query library</td><td align="right">48 KiB</td></tr>
<tr><td align="right">1238</td><td><code>libdotconf0</code></td><td>1.4.1-1build1</td><td>Configuration file parser library - runtime files</td><td align="right">48 KiB</td></tr>
<tr><td align="right">1239</td><td><code>libheif-plugin-aomdec</code></td><td>1.21.2-3</td><td>HEIF and AVIF file format decoder and encoder - aomdec plugin</td><td align="right">48 KiB</td></tr>
<tr><td align="right">1240</td><td><code>libijs-0.35</code></td><td>0.35-16</td><td>IJS raster image transport protocol: shared library</td><td align="right">48 KiB</td></tr>
<tr><td align="right">1241</td><td><code>libmtdev1t64</code></td><td>1.1.7-1build1</td><td>Multitouch Protocol Translation Library - shared library</td><td align="right">48 KiB</td></tr>
<tr><td align="right">1242</td><td><code>liboeffis1</code></td><td>1.5.0-3</td><td>RemoteDesktop portal DBus helper library</td><td align="right">48 KiB</td></tr>
<tr><td align="right">1243</td><td><code>libpwquality1</code></td><td>1.4.5-5build1</td><td>library for password quality checking and generation</td><td align="right">48 KiB</td></tr>
<tr><td align="right">1244</td><td><code>libsharpyuv0</code></td><td>1.5.0-0.1build1</td><td>Library for sharp RGB to YUV conversion</td><td align="right">48 KiB</td></tr>
<tr><td align="right">1245</td><td><code>libsoup-3.0-common</code></td><td>3.6.6-1</td><td>HTTP library implementation in C -- Common files</td><td align="right">48 KiB</td></tr>
<tr><td align="right">1246</td><td><code>libtext-iconv-perl</code></td><td>1.7-8.1</td><td>module to convert between character sets in Perl</td><td align="right">48 KiB</td></tr>
<tr><td align="right">1247</td><td><code>libbytesize1</code></td><td>2.12-1</td><td>library for common operations with sizes in bytes</td><td align="right">47 KiB</td></tr>
<tr><td align="right">1248</td><td><code>libmnl0</code></td><td>1.0.5-3build1</td><td>minimalistic Netlink communication library</td><td align="right">47 KiB</td></tr>
<tr><td align="right">1249</td><td><code>libraqm0</code></td><td>0.10.4-1</td><td>Library for complex text layout</td><td align="right">47 KiB</td></tr>
<tr><td align="right">1250</td><td><code>libxcb-util1</code></td><td>0.4.1-1build1</td><td>utility libraries for X C Binding -- atom, aux and event</td><td align="right">47 KiB</td></tr>
<tr><td align="right">1251</td><td><code>libxcomposite-dev</code></td><td>1:0.4.6-1build1</td><td>X11 Composite extension library (development headers)</td><td align="right">47 KiB</td></tr>
<tr><td align="right">1252</td><td><code>linux-sound-base</code></td><td>1.0.25+dfsg-0ubuntu9</td><td>base package for ALSA and OSS sound systems</td><td align="right">47 KiB</td></tr>
<tr><td align="right">1253</td><td><code>python3-jaraco.functools</code></td><td>4.1.0-1build1</td><td>additional functools in the spirit of stdlib&#x27;s functools</td><td align="right">47 KiB</td></tr>
<tr><td align="right">1254</td><td><code>python3-jaraco.text</code></td><td>4.0.0-1build1</td><td>jaraco text manipulation functions</td><td align="right">47 KiB</td></tr>
<tr><td align="right">1255</td><td><code>tpm-udev</code></td><td>4.1.3-6</td><td>TPM2 Software stack library - udev rules for TPM modules</td><td align="right">47 KiB</td></tr>
<tr><td align="right">1256</td><td><code>libfile-desktopentry-perl</code></td><td>0.23-1</td><td>Perl module to handle freedesktop .desktop files</td><td align="right">46 KiB</td></tr>
<tr><td align="right">1257</td><td><code>libspeechd-module0</code></td><td>0.12.1-2ubuntu1</td><td>Speech Dispatcher: Shared libraries for modules</td><td align="right">46 KiB</td></tr>
<tr><td align="right">1258</td><td><code>libxtst6</code></td><td>2:1.2.5-1build1</td><td>X11 Testing -- Record extension library</td><td align="right">46 KiB</td></tr>
<tr><td align="right">1259</td><td><code>glib-networking-services</code></td><td>2.80.1-1build2</td><td>network-related giomodules for GLib - D-Bus services</td><td align="right">45 KiB</td></tr>
<tr><td align="right">1260</td><td><code>libcanberra-pulse</code></td><td>0.30-18ubuntu3</td><td>PulseAudio backend for libcanberra</td><td align="right">45 KiB</td></tr>
<tr><td align="right">1261</td><td><code>libva-drm2</code></td><td>2.23.0-1ubuntu1</td><td>Video Acceleration (VA) API for Linux -- DRM runtime</td><td align="right">45 KiB</td></tr>
<tr><td align="right">1262</td><td><code>libxxf86dga1</code></td><td>2:1.1.5-1build2</td><td>X11 Direct Graphics Access extension library</td><td align="right">45 KiB</td></tr>
<tr><td align="right">1263</td><td><code>python3-zipp</code></td><td>3.23.0-1build1</td><td>pathlib-compatible Zipfile object wrapper - Python 3.x</td><td align="right">45 KiB</td></tr>
<tr><td align="right">1264</td><td><code>user-session-migration</code></td><td>0.5.1</td><td>Tool to migrate in-session user data</td><td align="right">45 KiB</td></tr>
<tr><td align="right">1265</td><td><code>glib-networking-common</code></td><td>2.80.1-1build2</td><td>network-related giomodules for GLib - data files</td><td align="right">44 KiB</td></tr>
<tr><td align="right">1266</td><td><code>libblockdev-swap3</code></td><td>3.4.0-1</td><td>Swap plugin for libblockdev</td><td align="right">44 KiB</td></tr>
<tr><td align="right">1267</td><td><code>libgdk-pixbuf2.0-common</code></td><td>2.44.5+dfsg-4ubuntu1</td><td>GDK Pixbuf library - data files</td><td align="right">44 KiB</td></tr>
<tr><td align="right">1268</td><td><code>libgpm2</code></td><td>1.20.7-12build1</td><td>General Purpose Mouse - shared library</td><td align="right">44 KiB</td></tr>
<tr><td align="right">1269</td><td><code>libiniparser4</code></td><td>4.2.6-1build1</td><td>runtime library for the iniParser INI file reader/writer</td><td align="right">44 KiB</td></tr>
<tr><td align="right">1270</td><td><code>liblouisutdml-bin</code></td><td>2.12.0-8build1</td><td>Braille UTDML translation utilities</td><td align="right">44 KiB</td></tr>
<tr><td align="right">1271</td><td><code>libteamdctl0</code></td><td>1.31-1build4</td><td>library for communication with `teamd` process</td><td align="right">44 KiB</td></tr>
<tr><td align="right">1272</td><td><code>libtry-tiny-perl</code></td><td>0.32-1</td><td>module providing minimalistic try/catch</td><td align="right">44 KiB</td></tr>
<tr><td align="right">1273</td><td><code>libwebpdemux2</code></td><td>1.5.0-0.1build1</td><td>Lossy compression of digital photographic images (libwebpdemux library)</td><td align="right">44 KiB</td></tr>
<tr><td align="right">1274</td><td><code>libxfixes3</code></td><td>1:6.0.0-2build2</td><td>X11 miscellaneous &#x27;fixes&#x27; extension library</td><td align="right">44 KiB</td></tr>
<tr><td align="right">1275</td><td><code>mscompress</code></td><td>0.4-10build2</td><td>Microsoft &quot;compress.exe/expand.exe&quot; compatible (de)compressor</td><td align="right">44 KiB</td></tr>
<tr><td align="right">1276</td><td><code>libfreeaptx0</code></td><td>0.2.2-1build1</td><td>Free implementation of aptX</td><td align="right">43 KiB</td></tr>
<tr><td align="right">1277</td><td><code>libnotify-bin</code></td><td>0.8.8-1</td><td>sends desktop notifications to a notification daemon (Utilities)</td><td align="right">43 KiB</td></tr>
<tr><td align="right">1278</td><td><code>libnpth0t64</code></td><td>1.8-3build1</td><td>replacement for GNU Pth using system threads</td><td align="right">43 KiB</td></tr>
<tr><td align="right">1279</td><td><code>python3-blinker</code></td><td>1.9.0-2build1</td><td>Fast, simple object-to-object and broadcast signaling (Python3)</td><td align="right">43 KiB</td></tr>
<tr><td align="right">1280</td><td><code>tcl8.6</code></td><td>8.6.17+dfsg-1build1</td><td>Tcl (the Tool Command Language) v8.6 - shell</td><td align="right">43 KiB</td></tr>
<tr><td align="right">1281</td><td><code>ca-certificates-java</code></td><td>20260311</td><td>Common CA certificates (JKS keystore)</td><td align="right">42 KiB</td></tr>
<tr><td align="right">1282</td><td><code>gir1.2-secret-1</code></td><td>0.21.7-2build1</td><td>Secret store (GObject-Introspection)</td><td align="right">42 KiB</td></tr>
<tr><td align="right">1283</td><td><code>libclone-perl</code></td><td>0.47-1</td><td>module for recursively copying Perl datatypes</td><td align="right">42 KiB</td></tr>
<tr><td align="right">1284</td><td><code>libkeyutils1</code></td><td>1.6.3-6ubuntu3</td><td>Linux Key Management Utilities (library)</td><td align="right">42 KiB</td></tr>
<tr><td align="right">1285</td><td><code>libtext-charwidth-perl</code></td><td>0.04-11build4</td><td>get display widths of characters on the terminal</td><td align="right">42 KiB</td></tr>
<tr><td align="right">1286</td><td><code>libxv1</code></td><td>2:1.0.13-1</td><td>X11 Video extension library</td><td align="right">42 KiB</td></tr>
<tr><td align="right">1287</td><td><code>python3-certifi</code></td><td>2026.1.4+ds-1</td><td>root certificates for validating SSL certs and verifying TLS hosts (python3)</td><td align="right">42 KiB</td></tr>
<tr><td align="right">1288</td><td><code>python3-distro-info</code></td><td>1.15</td><td>information about distributions&#x27; releases (Python 3 module)</td><td align="right">42 KiB</td></tr>
<tr><td align="right">1289</td><td><code>python3-mdurl</code></td><td>0.1.2-1build1</td><td>Python port of the JavaScript mdurl package</td><td align="right">42 KiB</td></tr>
<tr><td align="right">1290</td><td><code>python3-uc-micro</code></td><td>1.0.3-1build1</td><td>python port of uc.micro</td><td align="right">42 KiB</td></tr>
<tr><td align="right">1291</td><td><code>secureboot-db</code></td><td>1.9build2</td><td>Secure Boot updates for DB and DBX</td><td align="right">42 KiB</td></tr>
<tr><td align="right">1292</td><td><code>gir1.2-gdkpixbuf-2.0</code></td><td>2.44.5+dfsg-4ubuntu1</td><td>GDK Pixbuf library - GObject-Introspection</td><td align="right">41 KiB</td></tr>
<tr><td align="right">1293</td><td><code>libdrm-common</code></td><td>2.4.131-1</td><td>Userspace interface to kernel DRM services -- common files</td><td align="right">41 KiB</td></tr>
<tr><td align="right">1294</td><td><code>libxcb-dri3-0</code></td><td>1.17.0-2ubuntu1</td><td>X C Binding, dri3 extension</td><td align="right">41 KiB</td></tr>
<tr><td align="right">1295</td><td><code>libxcb-record0</code></td><td>1.17.0-2ubuntu1</td><td>X C Binding, record extension</td><td align="right">41 KiB</td></tr>
<tr><td align="right">1296</td><td><code>libxdmcp6</code></td><td>1:1.1.5-2</td><td>X11 Display Manager Control Protocol library</td><td align="right">41 KiB</td></tr>
<tr><td align="right">1297</td><td><code>g++-15</code></td><td>15.2.0-16ubuntu1</td><td>GNU C++ compiler</td><td align="right">40 KiB</td></tr>
<tr><td align="right">1298</td><td><code>libblockdev-loop3</code></td><td>3.4.0-1</td><td>Loop device plugin for libblockdev</td><td align="right">40 KiB</td></tr>
<tr><td align="right">1299</td><td><code>libbytesize-common</code></td><td>2.12-1</td><td>library for common operations with sizes in bytes - translations</td><td align="right">40 KiB</td></tr>
<tr><td align="right">1300</td><td><code>libio-html-perl</code></td><td>1.004-3</td><td>open an HTML file with automatic charset detection</td><td align="right">40 KiB</td></tr>
<tr><td align="right">1301</td><td><code>libjson-glib-1.0-common</code></td><td>1.10.8+ds-2</td><td>GLib JSON manipulation library (common files)</td><td align="right">40 KiB</td></tr>
<tr><td align="right">1302</td><td><code>libsecret-common</code></td><td>0.21.7-2build1</td><td>Secret store (common files)</td><td align="right">40 KiB</td></tr>
<tr><td align="right">1303</td><td><code>libxinerama-dev</code></td><td>2:1.1.4-3build2</td><td>X11 Xinerama extension library (development headers)</td><td align="right">40 KiB</td></tr>
<tr><td align="right">1304</td><td><code>libxres-dev</code></td><td>2:1.2.1-1build2</td><td>X11 Resource extension library (development headers)</td><td align="right">40 KiB</td></tr>
<tr><td align="right">1305</td><td><code>plymouth-label</code></td><td>24.004.60+git20250831.4a3c171d-0ubuntu8</td><td>boot animation, logger and I/O multiplexer - label control</td><td align="right">40 KiB</td></tr>
<tr><td align="right">1306</td><td><code>libdebconfclient0</code></td><td>0.280ubuntu1</td><td>Debian Configuration Management System (C-implementation library)</td><td align="right">39 KiB</td></tr>
<tr><td align="right">1307</td><td><code>libhtml-tagset-perl</code></td><td>3.24-1</td><td>data tables pertaining to HTML</td><td align="right">39 KiB</td></tr>
<tr><td align="right">1308</td><td><code>libxmuu1</code></td><td>2:1.1.3-4</td><td>X11 miscellaneous micro-utility library</td><td align="right">39 KiB</td></tr>
<tr><td align="right">1309</td><td><code>libfile-basedir-perl</code></td><td>0.09-2</td><td>Perl module to use the freedesktop basedir specification</td><td align="right">38 KiB</td></tr>
<tr><td align="right">1310</td><td><code>libpcaudio0</code></td><td>1.3-1build1</td><td>C API to different audio devices - shared library</td><td align="right">38 KiB</td></tr>
<tr><td align="right">1311</td><td><code>libwhoopsie0</code></td><td>0.2.82ubuntu</td><td>Ubuntu error tracker submission - shared library</td><td align="right">38 KiB</td></tr>
<tr><td align="right">1312</td><td><code>libxxf86vm1</code></td><td>1:1.1.4-2</td><td>X11 XFree86 video mode extension library</td><td align="right">38 KiB</td></tr>
<tr><td align="right">1313</td><td><code>printer-driver-sag-gdi</code></td><td>0.1-8build1</td><td>printer driver for Ricoh Aficio SP 1000s/SP 1100s</td><td align="right">38 KiB</td></tr>
<tr><td align="right">1314</td><td><code>cpp</code></td><td>4:15.2.0-5ubuntu1</td><td>GNU C preprocessor (cpp)</td><td align="right">37 KiB</td></tr>
<tr><td align="right">1315</td><td><code>gcc</code></td><td>4:15.2.0-5ubuntu1</td><td>GNU C compiler</td><td align="right">37 KiB</td></tr>
<tr><td align="right">1316</td><td><code>libsonic0</code></td><td>0.2.0-13build2</td><td>Simple library to speed up or slow down speech</td><td align="right">37 KiB</td></tr>
<tr><td align="right">1317</td><td><code>libxcb-res0</code></td><td>1.17.0-2ubuntu1</td><td>X C Binding, res extension</td><td align="right">37 KiB</td></tr>
<tr><td align="right">1318</td><td><code>libpcre2-posix3</code></td><td>10.46-1build1</td><td>New Perl Compatible Regular Expression Library - posix-compatible runtime files</td><td align="right">36 KiB</td></tr>
<tr><td align="right">1319</td><td><code>libextutils-depends-perl</code></td><td>0.8002-1</td><td>Perl module for building extensions that depend on other extensions</td><td align="right">35 KiB</td></tr>
<tr><td align="right">1320</td><td><code>libgdbm-compat4t64</code></td><td>1.26-1build1</td><td>GNU dbm database routines (legacy support runtime version)</td><td align="right">35 KiB</td></tr>
<tr><td align="right">1321</td><td><code>netbase</code></td><td>6.5build1</td><td>Basic TCP/IP networking system</td><td align="right">35 KiB</td></tr>
<tr><td align="right">1322</td><td><code>libestr0</code></td><td>0.1.11-2build1</td><td>Helper functions for handling strings (lib)</td><td align="right">34 KiB</td></tr>
<tr><td align="right">1323</td><td><code>libhttp-negotiate-perl</code></td><td>6.01-2</td><td>implementation of content negotiation</td><td align="right">34 KiB</td></tr>
<tr><td align="right">1324</td><td><code>libppd2-common</code></td><td>2:2.1.1-0ubuntu2</td><td>OpenPrinting libppd - Auxiliary files</td><td align="right">34 KiB</td></tr>
<tr><td align="right">1325</td><td><code>libqmi-proxy</code></td><td>1.38.0-1</td><td>Proxy to communicate with QMI ports</td><td align="right">34 KiB</td></tr>
<tr><td align="right">1326</td><td><code>libwww-robotrules-perl</code></td><td>6.02-1build1</td><td>database of robots.txt-derived permissions</td><td align="right">34 KiB</td></tr>
<tr><td align="right">1327</td><td><code>mysql-common</code></td><td>5.8+1.1.1ubuntu2</td><td>MySQL database common files, e.g. /etc/mysql/my.cnf</td><td align="right">34 KiB</td></tr>
<tr><td align="right">1328</td><td><code>python3-jaraco.context</code></td><td>6.0.1-2</td><td>jaraco contextlib extensions</td><td align="right">34 KiB</td></tr>
<tr><td align="right">1329</td><td><code>libdecor-0-dev</code></td><td>0.2.5-1</td><td>client-side window decoration library - development files</td><td align="right">33 KiB</td></tr>
<tr><td align="right">1330</td><td><code>libmbim-proxy</code></td><td>1.32.0-2ubuntu1</td><td>Proxy to communicate with MBIM ports</td><td align="right">33 KiB</td></tr>
<tr><td align="right">1331</td><td><code>libtie-ixhash-perl</code></td><td>1.23-4</td><td>Perl module to order associative arrays</td><td align="right">33 KiB</td></tr>
<tr><td align="right">1332</td><td><code>libxau6</code></td><td>1:1.0.11-1build2</td><td>X11 authorisation library</td><td align="right">33 KiB</td></tr>
<tr><td align="right">1333</td><td><code>libxres1</code></td><td>2:1.2.1-1build2</td><td>X11 Resource extension library</td><td align="right">33 KiB</td></tr>
<tr><td align="right">1334</td><td><code>libatk-bridge2.0-dev</code></td><td>2.60.0-1</td><td>Development files for the AT-SPI 2 toolkit bridge</td><td align="right">32 KiB</td></tr>
<tr><td align="right">1335</td><td><code>libxcb-present0</code></td><td>1.17.0-2ubuntu1</td><td>X C Binding, present extension</td><td align="right">32 KiB</td></tr>
<tr><td align="right">1336</td><td><code>libxcb-shape0</code></td><td>1.17.0-2ubuntu1</td><td>X C Binding, shape extension</td><td align="right">32 KiB</td></tr>
<tr><td align="right">1337</td><td><code>libxcb-xinerama0</code></td><td>1.17.0-2ubuntu1</td><td>X C Binding, xinerama extension</td><td align="right">32 KiB</td></tr>
<tr><td align="right">1338</td><td><code>libencode-locale-perl</code></td><td>1.05-3</td><td>utility to determine the locale encoding</td><td align="right">31 KiB</td></tr>
<tr><td align="right">1339</td><td><code>libfile-listing-perl</code></td><td>6.16-1</td><td>module to parse directory listings</td><td align="right">31 KiB</td></tr>
<tr><td align="right">1340</td><td><code>libinih1</code></td><td>61-1ubuntu1</td><td>simple .INI file parser</td><td align="right">31 KiB</td></tr>
<tr><td align="right">1341</td><td><code>libwayland-egl1</code></td><td>1.24.0-2</td><td>wayland compositor infrastructure - EGL library</td><td align="right">31 KiB</td></tr>
<tr><td align="right">1342</td><td><code>libxcb-composite0</code></td><td>1.17.0-2ubuntu1</td><td>X C Binding, composite extension</td><td align="right">31 KiB</td></tr>
<tr><td align="right">1343</td><td><code>libxcb-damage0</code></td><td>1.17.0-2ubuntu1</td><td>X C Binding, damage extension</td><td align="right">31 KiB</td></tr>
<tr><td align="right">1344</td><td><code>libxcb-dpms0</code></td><td>1.17.0-2ubuntu1</td><td>X C Binding, dpms extension</td><td align="right">31 KiB</td></tr>
<tr><td align="right">1345</td><td><code>libxcb-shm0</code></td><td>1.17.0-2ubuntu1</td><td>X C Binding, shm extension</td><td align="right">31 KiB</td></tr>
<tr><td align="right">1346</td><td><code>libxcvt0</code></td><td>0.1.3-1build1</td><td>VESA CVT standard timing modelines generator -- shared library</td><td align="right">31 KiB</td></tr>
<tr><td align="right">1347</td><td><code>libxinerama1</code></td><td>2:1.1.4-3build2</td><td>X11 Xinerama extension library</td><td align="right">31 KiB</td></tr>
<tr><td align="right">1348</td><td><code>libxss1</code></td><td>1:1.2.3-1build4</td><td>X11 Screen Saver extension library</td><td align="right">31 KiB</td></tr>
<tr><td align="right">1349</td><td><code>javascript-common</code></td><td>12+nmu1build1</td><td>Base support for JavaScript library packages</td><td align="right">30 KiB</td></tr>
<tr><td align="right">1350</td><td><code>libldacbt-abr2</code></td><td>2.0.2.3+git20200429+ed310a0-5build1</td><td>LDAC Bluetooth ABR library (shared library)</td><td align="right">30 KiB</td></tr>
<tr><td align="right">1351</td><td><code>libxcomposite1</code></td><td>1:0.4.6-1build1</td><td>X11 Composite extension library</td><td align="right">30 KiB</td></tr>
<tr><td align="right">1352</td><td><code>libxshmfence1</code></td><td>1.3.3-1build1</td><td>X shared memory fences - shared library</td><td align="right">30 KiB</td></tr>
<tr><td align="right">1353</td><td><code>remmina-plugin-secret</code></td><td>1.4.40+dfsg-2ubuntu1</td><td>Secret plugin for Remmina</td><td align="right">30 KiB</td></tr>
<tr><td align="right">1354</td><td><code>finalrd</code></td><td>9build2</td><td>final runtime directory for shutdown</td><td align="right">29 KiB</td></tr>
<tr><td align="right">1355</td><td><code>libhttp-date-perl</code></td><td>6.06-1</td><td>module of date conversion routines</td><td align="right">29 KiB</td></tr>
<tr><td align="right">1356</td><td><code>libmtp-runtime</code></td><td>1.1.22-1ubuntu1</td><td>Media Transfer Protocol (MTP) runtime tools</td><td align="right">29 KiB</td></tr>
<tr><td align="right">1357</td><td><code>libxdamage1</code></td><td>1:1.1.7-1</td><td>X11 damaged region extension library</td><td align="right">29 KiB</td></tr>
<tr><td align="right">1358</td><td><code>ubuntu-keyring</code></td><td>2023.11.28.1build1</td><td>GnuPG keys of the Ubuntu archive</td><td align="right">29 KiB</td></tr>
<tr><td align="right">1359</td><td><code>java-common</code></td><td>0.77</td><td>Base package for Java runtimes</td><td align="right">28 KiB</td></tr>
<tr><td align="right">1360</td><td><code>libsensors-config</code></td><td>1:3.6.2-2build1</td><td>lm-sensors configuration files</td><td align="right">28 KiB</td></tr>
<tr><td align="right">1361</td><td><code>libtirpc-common</code></td><td>1.3.7-0.1</td><td>transport-independent RPC library - common files</td><td align="right">27 KiB</td></tr>
<tr><td align="right">1362</td><td><code>libxdamage-dev</code></td><td>1:1.1.7-1</td><td>X11 damaged region extension library (development headers)</td><td align="right">27 KiB</td></tr>
<tr><td align="right">1363</td><td><code>perl-openssl-defaults</code></td><td>7build4</td><td>version compatibility baseline for Perl OpenSSL packages</td><td align="right">27 KiB</td></tr>
<tr><td align="right">1364</td><td><code>libpython3-stdlib</code></td><td>3.14.3-0ubuntu2</td><td>interactive high-level object-oriented language (default python3 version)</td><td align="right">26 KiB</td></tr>
<tr><td align="right">1365</td><td><code>sudo-common</code></td><td>1.2ubuntu</td><td>Configuration files for sudo</td><td align="right">26 KiB</td></tr>
<tr><td align="right">1366</td><td><code>gcc-x86-64-linux-gnu</code></td><td>4:15.2.0-5ubuntu1</td><td>GNU C compiler for the amd64 architecture</td><td align="right">25 KiB</td></tr>
<tr><td align="right">1367</td><td><code>unixodbc-common</code></td><td>2.3.14-1</td><td>Common ODBC configuration files</td><td align="right">25 KiB</td></tr>
<tr><td align="right">1368</td><td><code>liblwp-protocol-https-perl</code></td><td>6.14-1</td><td>HTTPS driver for LWP::UserAgent</td><td align="right">24 KiB</td></tr>
<tr><td align="right">1369</td><td><code>libtext-wrapi18n-perl</code></td><td>0.06-10</td><td>internationalized substitute of Text::Wrap</td><td align="right">24 KiB</td></tr>
<tr><td align="right">1370</td><td><code>wireless-regdb</code></td><td>2026.02.04-0ubuntu1</td><td>wireless regulatory database</td><td align="right">24 KiB</td></tr>
<tr><td align="right">1371</td><td><code>apt-config-icons</code></td><td>1.1.2-1</td><td>APT configuration snippet to enable icon downloads</td><td align="right">23 KiB</td></tr>
<tr><td align="right">1372</td><td><code>apt-config-icons-hidpi</code></td><td>1.1.2-1</td><td>APT configuration snippet to enable HiDPI icon downloads</td><td align="right">23 KiB</td></tr>
<tr><td align="right">1373</td><td><code>apt-config-icons-large</code></td><td>1.1.2-1</td><td>APT configuration snippet to enable large icon downloads</td><td align="right">23 KiB</td></tr>
<tr><td align="right">1374</td><td><code>apt-config-icons-large-hidpi</code></td><td>1.1.2-1</td><td>APT configuration snippet to enable large HiDPI icon downloads</td><td align="right">23 KiB</td></tr>
<tr><td align="right">1375</td><td><code>pipewire-audio</code></td><td>1.6.2-1ubuntu1</td><td>recommended set of PipeWire packages for a standard audio desktop use</td><td align="right">23 KiB</td></tr>
<tr><td align="right">1376</td><td><code>cpp-x86-64-linux-gnu</code></td><td>4:15.2.0-5ubuntu1</td><td>GNU C preprocessor (cpp) for the amd64 architecture</td><td align="right">21 KiB</td></tr>
<tr><td align="right">1377</td><td><code>gir1.2-notify-0.7</code></td><td>0.8.8-1</td><td>sends desktop notifications to a notification daemon (Introspection files)</td><td align="right">21 KiB</td></tr>
<tr><td align="right">1378</td><td><code>grub-gfxpayload-lists</code></td><td>0.7build3</td><td>GRUB gfxpayload blacklist</td><td align="right">21 KiB</td></tr>
<tr><td align="right">1379</td><td><code>libopengl-dev</code></td><td>1.7.0-3</td><td>Vendor neutral GL dispatch library -- OpenGL development files</td><td align="right">21 KiB</td></tr>
<tr><td align="right">1380</td><td><code>policykit-desktop-privileges</code></td><td>0.22build1</td><td>run common desktop actions without password</td><td align="right">21 KiB</td></tr>
<tr><td align="right">1381</td><td><code>distro-info-data</code></td><td>0.68ubuntu0.1</td><td>information about the distributions&#x27; releases (data files)</td><td align="right">20 KiB</td></tr>
<tr><td align="right">1382</td><td><code>libao-common</code></td><td>1.2.2+20180113-1.2ubuntu2</td><td>Cross Platform Audio Output Library (Common files)</td><td align="right">20 KiB</td></tr>
<tr><td align="right">1383</td><td><code>libsemanage-common</code></td><td>3.9-1build1</td><td>Common files for SELinux policy management libraries</td><td align="right">20 KiB</td></tr>
<tr><td align="right">1384</td><td><code>tcl</code></td><td>8.6.16build1</td><td>Tool Command Language (default version) - shell</td><td align="right">20 KiB</td></tr>
<tr><td align="right">1385</td><td><code>dns-root-data</code></td><td>2025080400build1</td><td>DNS root hints and DNSSEC trust anchor</td><td align="right">19 KiB</td></tr>
<tr><td align="right">1386</td><td><code>libglvnd-dev</code></td><td>1.7.0-3</td><td>Vendor neutral GL dispatch library -- development files</td><td align="right">19 KiB</td></tr>
<tr><td align="right">1387</td><td><code>libnet-smtp-ssl-perl</code></td><td>1.04-2</td><td>Perl module providing SSL support to Net::SMTP</td><td align="right">19 KiB</td></tr>
<tr><td align="right">1388</td><td><code>linux-sysctl-defaults</code></td><td>4.15ubuntu5</td><td>default sysctl configuration for Linux</td><td align="right">19 KiB</td></tr>
<tr><td align="right">1389</td><td><code>python3-debconf</code></td><td>1.5.92</td><td>interact with debconf from Python 3</td><td align="right">19 KiB</td></tr>
<tr><td align="right">1390</td><td><code>laptop-detect</code></td><td>0.16+nmu1build1</td><td>system chassis type checker</td><td align="right">18 KiB</td></tr>
<tr><td align="right">1391</td><td><code>lsb-release</code></td><td>12.1-2build1</td><td>Linux Standard Base version reporting utility (minimal implementation)</td><td align="right">18 KiB</td></tr>
<tr style="font-style:italic;color:#666"><td align="right">1392</td><td><code>init</code><span style="font-size:0.7em;font-weight:normal;background:#e0e0e0;color:#555;border-radius:3px;padding:1px 4px;margin-left:5px;font-style:normal;vertical-align:middle">meta</span></td><td>1.69</td><td>metapackage ensuring an init system is installed</td><td align="right">17 KiB</td></tr>
<tr><td align="right">1393</td><td><code>libaudit-common</code></td><td>1:4.1.2-1build1</td><td>Dynamic library for security auditing - common files</td><td align="right">17 KiB</td></tr>
<tr><td align="right">1394</td><td><code>systemd-hwe-hwdb</code></td><td>259.5.3ubuntu</td><td>udev rules for hardware enablement (HWE)</td><td align="right">17 KiB</td></tr>
<tr><td align="right">1395</td><td><code>hwdata</code></td><td>0.394-1build1</td><td>hardware identification / configuration data</td><td align="right">15 KiB</td></tr>
<tr style="font-style:italic;color:#666"><td align="right">1396</td><td><code>ubuntu-minimal</code><span style="font-size:0.7em;font-weight:normal;background:#e0e0e0;color:#555;border-radius:3px;padding:1px 4px;margin-left:5px;font-style:normal;vertical-align:middle">meta</span></td><td>1.570</td><td>Minimal core of Ubuntu</td><td align="right">14 KiB</td></tr>
<tr><td align="right">1397</td><td><code>g++</code></td><td>4:15.2.0-5ubuntu1</td><td>GNU C++ compiler</td><td align="right">13 KiB</td></tr>
<tr><td align="right">1398</td><td><code>linux-image-generic</code></td><td>7.0.0-22.22</td><td>Generic Linux kernel image</td><td align="right">12 KiB</td></tr>
<tr><td align="right">1399</td><td><code>cpp-15</code></td><td>15.2.0-16ubuntu1</td><td>GNU C preprocessor</td><td align="right">11 KiB</td></tr>
<tr><td align="right">1400</td><td><code>g++-x86-64-linux-gnu</code></td><td>4:15.2.0-5ubuntu1</td><td>GNU C++ compiler for the amd64 architecture</td><td align="right">11 KiB</td></tr>
<tr><td align="right">1401</td><td><code>coreutils</code></td><td>9.5-1ubuntu2+0.0.0~ubuntu25</td><td>coreutils meta package</td><td align="right">10 KiB</td></tr>
<tr><td align="right">1402</td><td><code>language-pack-en</code></td><td>1:26.04+20260417</td><td>translation updates for language English</td><td align="right">9 KiB</td></tr>
<tr><td align="right">1403</td><td><code>libjpeg-dev</code></td><td>8c-2ubuntu12</td><td>Independent JPEG Group&#x27;s JPEG runtime library (dependency package)</td><td align="right">9 KiB</td></tr>
<tr><td align="right">1404</td><td><code>libjpeg8</code></td><td>8c-2ubuntu12</td><td>Independent JPEG Group&#x27;s JPEG runtime library (dependency package)</td><td align="right">9 KiB</td></tr>
<tr><td align="right">1405</td><td><code>libjpeg8-dev</code></td><td>8c-2ubuntu12</td><td>Independent JPEG Group&#x27;s JPEG runtime library (dependency package)</td><td align="right">9 KiB</td></tr>
<tr><td align="right">1406</td><td><code>linux-firmware</code></td><td>20260319.git217ca6e4.1ubuntu</td><td>Firmware for Linux kernel drivers (metapackage)</td><td align="right">8 KiB</td></tr>
<tr><td align="right">1407</td><td><code>native-architecture</code></td><td>0.2.6build1</td><td>Declarative native architecture constraint</td><td align="right">8 KiB</td></tr>
<tr><td></td><td><strong>Ubuntu main total (1,407)</strong></td><td></td><td></td><td align="right"><strong>3.46 GiB</strong></td></tr>
</tbody></table>

</details>

<details>
<summary><strong>apt.foundrylinux.org</strong> — 19 packages · 8 meta · 923.6 MiB</summary>

<table>
<thead><tr><th align="right">#</th><th align="left">Package</th><th align="left">Version</th><th align="left">Description</th><th align="right">Installed Size</th></tr></thead>
<tbody>
<tr><td align="right">1</td><td><code>ghidra</code></td><td>12.1-1foundry1</td><td>NSA software reverse engineering framework</td><td align="right">864.0 MiB</td></tr>
<tr><td align="right">2</td><td><code>ppsspp</code></td><td>1.20.4-2foundry1</td><td>fast and portable PSP emulator</td><td align="right">36.2 MiB</td></tr>
<tr><td align="right">3</td><td><code>snes9x-gtk</code></td><td>1.63-1foundry1</td><td>portable Super Nintendo Entertainment System emulator</td><td align="right">6.9 MiB</td></tr>
<tr><td align="right">4</td><td><code>calamares-settings-foundry-linux</code></td><td>1.0.8</td><td>Calamares installer branding and settings for Foundry Linux</td><td align="right">5.2 MiB</td></tr>
<tr><td align="right">5</td><td><code>foundry-kde-theme</code></td><td>1.0.1</td><td>KDE Plasma desktop theme for Foundry Linux</td><td align="right">4.4 MiB</td></tr>
<tr><td align="right">6</td><td><code>libvgm</code></td><td>0.1+git20260406.d115188-1foundry1</td><td>chip-register VGM playback library with player utilities</td><td align="right">3.6 MiB</td></tr>
<tr><td align="right">7</td><td><code>vgmstream</code></td><td>2083-1foundry5</td><td>Video-game audio decoder (CLI)</td><td align="right">2.5 MiB</td></tr>
<tr><td align="right">8</td><td><code>blender-asset-finder</code></td><td>0.2.0+git0a19d26c-2foundry1</td><td>search/download 3D assets from online repos, in Blender</td><td align="right">349 KiB</td></tr>
<tr><td align="right">9</td><td><code>f9dasm</code></td><td>1.83-1foundry3</td><td>Motorola 6800/6809/6309 family disassembler</td><td align="right">177 KiB</td></tr>
<tr><td align="right">10</td><td><code>blender-asset-finder-cli</code></td><td>0.1.0+git0a19d26c-1foundry1</td><td>CLI companion to the blender-asset-finder Blender add-on</td><td align="right">57 KiB</td></tr>
<tr><td align="right">11</td><td><code>foundry-welcome</code></td><td>1.0.5</td><td>Welcome Center for Foundry Linux</td><td align="right">50 KiB</td></tr>
<tr style="font-style:italic;color:#666"><td align="right">12</td><td><code>foundry-anvil</code><span style="font-size:0.7em;font-weight:normal;background:#e0e0e0;color:#555;border-radius:3px;padding:1px 4px;margin-left:5px;font-style:normal;vertical-align:middle">meta</span></td><td>1.0.4</td><td>Foundry Linux ISO edition — developer toolkit + KDE desktop</td><td align="right">8 KiB</td></tr>
<tr style="font-style:italic;color:#666"><td align="right">13</td><td><code>foundry-core</code><span style="font-size:0.7em;font-weight:normal;background:#e0e0e0;color:#555;border-radius:3px;padding:1px 4px;margin-left:5px;font-style:normal;vertical-align:middle">meta</span></td><td>1.0.1</td><td>Foundry Linux developer toolkit — desktop-agnostic</td><td align="right">8 KiB</td></tr>
<tr style="font-style:italic;color:#666"><td align="right">14</td><td><code>foundry-desktop</code><span style="font-size:0.7em;font-weight:normal;background:#e0e0e0;color:#555;border-radius:3px;padding:1px 4px;margin-left:5px;font-style:normal;vertical-align:middle">meta</span></td><td>1.0.0</td><td>Foundry Linux KDE desktop integration (ISO / KDE hosts only)</td><td align="right">8 KiB</td></tr>
<tr style="font-style:italic;color:#666"><td align="right">15</td><td><code>foundry-emulators-computers</code><span style="font-size:0.7em;font-weight:normal;background:#e0e0e0;color:#555;border-radius:3px;padding:1px 4px;margin-left:5px;font-style:normal;vertical-align:middle">meta</span></td><td>1.0.0</td><td>vintage home-computer emulators (no bundled ROMs)</td><td align="right">8 KiB</td></tr>
<tr style="font-style:italic;color:#666"><td align="right">16</td><td><code>foundry-emulators-consoles</code><span style="font-size:0.7em;font-weight:normal;background:#e0e0e0;color:#555;border-radius:3px;padding:1px 4px;margin-left:5px;font-style:normal;vertical-align:middle">meta</span></td><td>1.0.1</td><td>light console and adventure-engine emulators</td><td align="right">8 KiB</td></tr>
<tr style="font-style:italic;color:#666"><td align="right">17</td><td><code>foundry-game-frameworks</code><span style="font-size:0.7em;font-weight:normal;background:#e0e0e0;color:#555;border-radius:3px;padding:1px 4px;margin-left:5px;font-style:normal;vertical-align:middle">meta</span></td><td>1.0.1</td><td>2D/3D game-dev frameworks, headers, and shader tools</td><td align="right">8 KiB</td></tr>
<tr style="font-style:italic;color:#666"><td align="right">18</td><td><code>foundry-image-cli</code><span style="font-size:0.7em;font-weight:normal;background:#e0e0e0;color:#555;border-radius:3px;padding:1px 4px;margin-left:5px;font-style:normal;vertical-align:middle">meta</span></td><td>1.0.0</td><td>command-line image manipulation utilities</td><td align="right">8 KiB</td></tr>
<tr style="font-style:italic;color:#666"><td align="right">19</td><td><code>foundry-retro-tools</code><span style="font-size:0.7em;font-weight:normal;background:#e0e0e0;color:#555;border-radius:3px;padding:1px 4px;margin-left:5px;font-style:normal;vertical-align:middle">meta</span></td><td>1.0.6</td><td>arcade reverse-engineering and porting toolchain</td><td align="right">8 KiB</td></tr>
<tr><td></td><td><strong>apt.foundrylinux.org total (19)</strong></td><td></td><td></td><td align="right"><strong>923.6 MiB</strong></td></tr>
</tbody></table>

</details>

<details>
<summary><strong>Cloudsmith (go-task)</strong> — 1 packages · 47.9 MiB</summary>

<table>
<thead><tr><th align="right">#</th><th align="left">Package</th><th align="left">Version</th><th align="left">Description</th><th align="right">Installed Size</th></tr></thead>
<tbody>
<tr><td align="right">1</td><td><code>task</code></td><td>3.51.1</td><td>A fast, cross-platform build tool inspired by Make, designed for modern workflows.</td><td align="right">47.9 MiB</td></tr>
<tr><td></td><td><strong>Cloudsmith (go-task) total (1)</strong></td><td></td><td></td><td align="right"><strong>47.9 MiB</strong></td></tr>
</tbody></table>

</details>

<details>
<summary><strong>Ubuntu multiverse</strong> — 1 packages · 40.8 MiB</summary>

<table>
<thead><tr><th align="right">#</th><th align="left">Package</th><th align="left">Version</th><th align="left">Description</th><th align="right">Installed Size</th></tr></thead>
<tbody>
<tr><td align="right">1</td><td><code>fonts-ibm-plex</code></td><td>6.1.1-1</td><td>extensive typeface family designed by IBM</td><td align="right">40.8 MiB</td></tr>
<tr><td></td><td><strong>Ubuntu multiverse total (1)</strong></td><td></td><td></td><td align="right"><strong>40.8 MiB</strong></td></tr>
</tbody></table>

</details>

<details>
<summary><strong>apt.worldfoundry.org</strong> — 13 packages · 3 meta · 4.3 MiB</summary>

<table>
<thead><tr><th align="right">#</th><th align="left">Package</th><th align="left">Version</th><th align="left">Description</th><th align="right">Installed Size</th></tr></thead>
<tbody>
<tr><td align="right">1</td><td><code>textile</code></td><td>0.1.0+git0a19d26c-1foundry2</td><td>World Foundry texture-atlas packer (TGA/BMP/PNG)</td><td align="right">712 KiB</td></tr>
<tr><td align="right">2</td><td><code>levcomp</code></td><td>0.1.0+git0a19d26c-1foundry2</td><td>World Foundry .lev level compiler (iff2lvl successor)</td><td align="right">571 KiB</td></tr>
<tr><td align="right">3</td><td><code>iffcomp</code></td><td>0.1.0+git0a19d26c-1foundry2</td><td>World Foundry IFF text-DSL compiler</td><td align="right">511 KiB</td></tr>
<tr><td align="right">4</td><td><code>oas2oad</code></td><td>0.1.0+git0a19d26c-1foundry2</td><td>World Foundry OAS-to-OAD compiler (Linux successor to pigtool)</td><td align="right">458 KiB</td></tr>
<tr><td align="right">5</td><td><code>lvldump</code></td><td>0.1.0+git0a19d26c-1foundry2</td><td>World Foundry .lvl level-file inspector</td><td align="right">448 KiB</td></tr>
<tr><td align="right">6</td><td><code>iffdump</code></td><td>0.1.0+git0a19d26c-1foundry2</td><td>World Foundry IFF binary inspector</td><td align="right">397 KiB</td></tr>
<tr><td align="right">7</td><td><code>oaddump</code></td><td>0.1.0+git0a19d26c-1foundry2</td><td>World Foundry .oad attribute-blob inspector</td><td align="right">393 KiB</td></tr>
<tr><td align="right">8</td><td><code>cdpack</code></td><td>0.1.0+git0a19d26c-1foundry2</td><td>World Foundry CD master image packer</td><td align="right">358 KiB</td></tr>
<tr><td align="right">9</td><td><code>worldfoundry-blender-editor-exporter</code></td><td>0.2.1+git3fa94cbe-2foundry2</td><td>World Foundry Level Editor add-on for Blender</td><td align="right">316 KiB</td></tr>
<tr><td align="right">10</td><td><code>prep</code></td><td>0.103+git0a19d26c-1foundry2</td><td>WorldFoundry macro preprocessor (Kevin T. Seghetti)</td><td align="right">224 KiB</td></tr>
<tr style="font-style:italic;color:#666"><td align="right">11</td><td><code>worldfoundry</code><span style="font-size:0.7em;font-weight:normal;background:#e0e0e0;color:#555;border-radius:3px;padding:1px 4px;margin-left:5px;font-style:normal;vertical-align:middle">meta</span></td><td>1.1.4</td><td>full WorldFoundry game-development install (umbrella)</td><td align="right">9 KiB</td></tr>
<tr style="font-style:italic;color:#666"><td align="right">12</td><td><code>worldfoundry-blender-addons</code><span style="font-size:0.7em;font-weight:normal;background:#e0e0e0;color:#555;border-radius:3px;padding:1px 4px;margin-left:5px;font-style:normal;vertical-align:middle">meta</span></td><td>1.1.3</td><td>WorldFoundry Blender add-ons + Blender itself</td><td align="right">9 KiB</td></tr>
<tr style="font-style:italic;color:#666"><td align="right">13</td><td><code>worldfoundry-cli</code><span style="font-size:0.7em;font-weight:normal;background:#e0e0e0;color:#555;border-radius:3px;padding:1px 4px;margin-left:5px;font-style:normal;vertical-align:middle">meta</span></td><td>1.0.3</td><td>WorldFoundry authoring CLI tools (headless / build-server set)</td><td align="right">8 KiB</td></tr>
<tr><td></td><td><strong>apt.worldfoundry.org total (13)</strong></td><td></td><td></td><td align="right"><strong>4.3 MiB</strong></td></tr>
</tbody></table>

</details>
