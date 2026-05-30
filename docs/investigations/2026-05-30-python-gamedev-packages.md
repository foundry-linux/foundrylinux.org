# Python Game-Dev Package Recommendations for Foundry Linux

**Source:** Top 4000 PyPI packages by monthly downloads (May 2026),
cross-referenced against game-development relevance by hand.  
**Script:** `docs/investigations/fetch-pypi-top4000.py`  
**Full table:** `docs/investigations/2026-05-30-pypi-top4000.md`

---

## How to read this

Three tiers:

| Tier | Meaning |
|------|---------|
| **Essential** | Pre-install unconditionally — either a direct game dependency or so universally pulled in that having it missing causes immediate breakage |
| **Recommended** | Pre-install — saves the user a noticeable install step and is clearly within the game-dev mission |
| **Optional** | Include only if the ISO/devbox has room — useful for a subset of game devs, or large enough to justify a separate metapackage |

The audience is **game developers new to Linux** who already know how to make games — they just want the tools to work on first boot. Don't make them fight pip.

---

## Tier 1 — Essential

### Math / Core Science

| Package | Rank | Monthly DLs | Version | Notes |
|---------|-----:|------------:|---------|-------|
| `numpy` | 20 | 907 M | 2.4.6 | Virtually every game library depends on it. Non-negotiable. |
| `scipy` | 87 | 373 M | 1.17.1 | Physics, signal processing, spatial data. Large (35 MB) but foundational. |
| `cffi` | 16 | 957 M | 2.0.0 | C-extension glue. Most native-binding packages (audio, input, graphics) pull it. |

### Image / Graphics

| Package | Rank | Monthly DLs | Version | Notes |
|---------|-----:|------------:|---------|-------|
| `pillow` | 69 | 442 M | 12.2.0 | Image loading/saving. Every game loads textures. 44 MB but mandatory. |
| `imageio` | 540 | 47 M | 2.37.3 | Unified image + video reader/writer; wraps PIL, ffmpeg, etc. |

### Python Infrastructure

| Package | Rank | Monthly DLs | Version | Notes |
|---------|-----:|------------:|---------|-------|
| `setuptools` | 9 | 1.3 B | 82.0.1 | Build backend for almost everything. |
| `pip` | 42 | 566 M | 26.1.1 | Obviously. |
| `wheel` | *(stdlib-adjacent)* | — | — | Needed to install binary wheels. |

---

## Tier 2 — Recommended

### Game Frameworks

| Package | Rank | Monthly DLs | Version | Notes |
|---------|-----:|------------:|---------|-------|
| `pygame` | 2593 | 3 M | 2.6.1 | The classic entry point. Beginners land here first. Ships SDL2 + audio + input. |
| `pyglet` | 3816 | 2 M | 2.1.14 | OpenGL-based, more modern than pygame; `arcade` depends on it. |
| `pyopengl` | 2425 | 3 M | 3.1.10 | Raw OpenGL bindings; needed for custom renderers and shader work. |
| `glfw` | 2596 | 3 M | 2.10.0 | Window + context + input for OpenGL/Vulkan. Pairs with moderngl, wgpu. |

> **Note:** `arcade` and `moderngl` are not yet in the top 4000 but are the natural next step after pygame. Track for inclusion when they cross the threshold.

### Audio

| Package | Rank | Monthly DLs | Version | Notes |
|---------|-----:|------------:|---------|-------|
| `sounddevice` | 1278 | 11 M | 0.5.5 | PortAudio bindings; play/record PCM. Lightweight, works without a display. |
| `soundfile` | 1025 | 17 M | 0.13.1 | Read/write WAV, OGG, FLAC via libsndfile. Works alongside sounddevice. |
| `pydub` | 900 | 21 M | 0.25.1 | High-level audio slicing, mixing, format conversion. Calls ffmpeg under the hood. |

### Serialization / Data Formats

These are used for config files, save data, asset manifests, and network protocols:

| Package | Rank | Monthly DLs | Version | Notes |
|---------|-----:|------------:|---------|-------|
| `pyyaml` | 15 | 982 M | 6.0.3 | YAML. Ubiquitous for game config, asset pipelines. |
| `tomli` | 88 | 369 M | 2.4.1 | TOML reader (stdlib `tomllib` only appeared in 3.11; this backports it). |
| `tomli-w` | 468 | 55 M | 1.2.0 | TOML writer — needed alongside tomli for round-trip config. |
| `msgpack` | 134 | 238 M | 1.1.2 | Compact binary serialization. Fast for network messages and save files. |
| `lz4` | 320 | 85 M | 4.4.5 | Fast compression for asset bundles and save data. |

### Networking / Persistence (Multiplayer / Game Servers)

| Package | Rank | Monthly DLs | Version | Notes |
|---------|-----:|------------:|---------|-------|
| `websockets` | 98 | 337 M | 16.0 | WebSocket client + server. Most browser-playable multiplayer games use this. |
| `aiohttp` | 49 | 531 M | 3.13.5 | Async HTTP. Game backends, leaderboards, asset CDN fetches. |
| `pyzmq` | 301 | 96 M | 27.1.0 | ZeroMQ bindings. High-throughput game server messaging, local IPC. |
| `aiosqlite` | 513 | 48 M | 0.22.1 | Async wrapper around stdlib `sqlite3`. Save data, leaderboards, inventory DBs without blocking the event loop. In universe. |

### Dev Tools

| Package | Rank | Monthly DLs | Version | Notes |
|---------|-----:|------------:|---------|-------|
| `click` | 19 | 914 M | 8.4.1 | CLI framework. Used by essentially every game tool and build script. |
| `rich` | 47 | 546 M | 15.0.0 | Terminal output with tables, progress, syntax highlighting. |
| `tqdm` | 70 | 441 M | 4.67.3 | Progress bars. Asset packing, batch processing. |
| `pytest` | 32 | 680 M | 9.0.3 | Testing. Serious game tools need it. |
| `ruff` | 163 | 204 M | 0.15.15 | Extremely fast linter + formatter. Replaces flake8 + black in one binary. |
| `mypy` | 224 | 149 M | 2.1.0 | Type checking. Game engines with Python scripting layers benefit enormously. |
| `loguru` | 357 | 76 M | 0.7.3 | Drop-in logging with sensible defaults. Much better DX than stdlib logging. |
| `psutil` | 101 | 334 M | 7.2.2 | Process/system stats. Profiling, performance monitoring, memory pressure. |

### Performance

| Package | Rank | Monthly DLs | Version | Notes |
|---------|-----:|------------:|---------|-------|
| `cython` | 270 | 123 M | 3.2.5 | Compile hot Python paths to C. Essential for performance-critical game code. |
| `numba` | 433 | 61 M | 0.65.1 | JIT-compile numerical Python to native code via LLVM. Physics loops, pathfinding. |

### Geometry / Spatial

| Package | Rank | Monthly DLs | Version | Notes |
|---------|-----:|------------:|---------|-------|
| `shapely` | 373 | 73 M | 2.1.2 | 2D geometry operations. Collision detection, procedural level geometry. |

---

## Tier 3 — Optional / Specialized

Include in a separate `foundry-python-gamedev-extras` metapackage or install on demand:

| Package | Rank | Monthly DLs | Version | Category | Notes |
|---------|-----:|------------:|---------|----------|-------|
| `opencv-python` | 518 | 48 M | 4.13.0.92 | Vision | Webcam input, AR, motion detection in games. 69 MB — too large for default. |
| `av` | 825 | 25 M | 17.0.1 | Video | Pythonic FFmpeg bindings. Cutscene playback, video recording. 38 MB. |
| `imageio-ffmpeg` | 1315 | 11 M | 0.6.0 | Video | Lighter FFmpeg wrapper if `av` is overkill. 29 MB binary. |
| `moviepy` | 1734 | 6 M | 2.2.1 | Video | High-level video editing. Trailer/devlog tooling. 55 MB. |
| `librosa` | 1369 | 10 M | 0.11.0 | Audio | Audio analysis, beat detection, DSP. Rhythm game beat maps, BPM detection, audio visualizers. |
| `fonttools` | 150 | 221 M | 4.63.0 | Fonts | Parse/modify font files. Needed for custom UI font pipelines. |
| `freetype-py` | 2140 | 4 M | 2.5.1 | Fonts | FreeType bindings. Glyph rendering outside of pygame/pyglet. |
| `mss` | 2837 | 2 M | 10.2.0 | Capture | Ultra-fast multi-monitor screenshots. Game recording, QA harnesses, screen-capture overlays. |
| `pynput` | 3789 | 2 M | 1.8.2 | Input | Global keyboard/mouse monitoring. Accessibility overlays, dev shortcuts. |
| `pyserial` | 1124 | 14 M | 3.5 | Hardware | Serial port access. Arduino controllers, custom peripherals. |
| `maturin` | 1245 | 12 M | 1.13.3 | Build | Build Rust extensions as Python wheels. Relevant for Foundry's Rust pipeline. |
| `networkx` | 132 | 242 M | 3.6.1 | Graphs | Graph algorithms. AI pathfinding, dungeon generation, dialogue trees. |
| `sortedcontainers` | 154 | 215 M | 2.4.0 | Data | Sorted list/dict/set. Priority queues, spatial indexes. No deps. |
| `attrs` | 28 | 777 M | 26.1.0 | Data | Boilerplate-free classes. Alternative to dataclasses; popular in game ECS patterns. |
| `pyzstd` / `zstd` | 1404/1650 | 9 M / 7 M | — | Compress | Zstandard compression — better than lz4 for large asset archives. |
| `cairocffi` | 1184 | 13 M | 1.7.1 | Vector | Cairo 2D graphics. SVG rendering, UI drawing outside of game engines. |

---

## Explicitly Excluded

These showed up in the heuristic scan but are **not** game-dev relevant for this distro:

| Package(s) | Reason |
|------------|--------|
| `torch`, `tensorflow`, `jax`, `onnxruntime`, `triton` | ML frameworks. Enormous (500 MB+). A separate AI-dev metapackage, not game-dev. |
| `boto3`, `aiobotocore`, `botocore`, all `mypy-boto3-*` | AWS SDK ecosystem. Not applicable. |
| `fastapi`, `starlette`, `uvicorn`, `werkzeug`, `flask` | Web frameworks. A game backend is possible but not what we're pre-installing for. |
| `sqlalchemy`, `asyncpg`, `aiosqlite`, `alembic` | Database layers. Not typical game-dev needs at install time. |
| `pyasn1`, `pyopenssl`, `cryptography`, `oauthlib` | TLS/auth stack. Pulled in transitively — no need to pre-install. |
| `langchain`, `openai`, `google-genai`, `pydantic-ai` | LLM tooling. Interesting but a separate category. |
| `opencv-python-headless` | Headless variant — wrong choice for a desktop distro. Use `opencv-python`. |
| `matplotlib` | Data visualization, not game graphics. Include only for tool/pipeline work, not by default. |
| `pandas`, `pyarrow` | Data analysis. Not game dev. |
| `six`, `python-dateutil` | Python 2 compat shims. 2026 — no. |
| `tree-sitter*` | Editor infrastructure. Not game dev. |
| `pywin32`, `pywin32-ctypes` | Windows-only. Irrelevant on Foundry Linux. |

---

## Ubuntu 26.04 Universe Check

Verified 2026-05-30 in a fresh `ubuntu:26.04` container via `apt-cache policy`.
Script: `docs/investigations/check-universe.sh`

### In Universe — use apt, no vendoring needed

| PyPI Name | Ubuntu Package | Version in Universe | Notes |
|-----------|---------------|---------------------|-------|
| `numpy` | `python3-numpy` | 1:2.3.5 | ✓ |
| `scipy` | `python3-scipy` | 1.16.3 | ✓ |
| `cffi` | `python3-cffi` | 2.0.0 | ✓ |
| `pillow` | `python3-pil` | 12.1.1 | ✓ |
| `imageio` | `python3-imageio` | 2.37.2 | ✓ |
| `setuptools` | `python3-setuptools` | 78.1.1 | ✓ |
| `pip` | `python3-pip` | 25.1.1 | ✓ |
| `wheel` | `python3-wheel` | 0.46.3 | ✓ |
| `pygame` | `python3-pygame` | 2.6.1 | ✓ |
| `pyglet` | `python3-pyglet` | 2.0.17 | ⚠ PyPI is 2.1.14 — one minor behind |
| `pyopengl` | `python3-opengl` | 3.1.10 | ✓ |
| `sounddevice` | `python3-sounddevice` | 0.5.3 | ✓ |
| `soundfile` | `python3-soundfile` | 0.13.1 | ✓ |
| `pyyaml` | `python3-yaml` | 6.0.3 | ✓ |
| `tomli` | `python3-tomli` | 2.4.0 | ✓ |
| `tomli-w` | `python3-tomli-w` | 1.2.0 | ✓ |
| `msgpack` | `python3-msgpack` | 1.1.2 | ✓ |
| `lz4` | `python3-lz4` | 4.4.5 | ✓ |
| `websockets` | `python3-websockets` | 15.0.1 | ⚠ PyPI is 16.0 |
| `aiohttp` | `python3-aiohttp` | 3.13.3 | ✓ |
| `pyzmq` | `python3-zmq` | 27.1.0 | ✓ |
| `aiosqlite` | `python3-aiosqlite` | 0.22.1 | ✓ |
| `click` | `python3-click` | 8.2.0 | ✓ |
| `rich` | `python3-rich` | 13.9.4 | ⚠ PyPI is 15.0.0 — significant gap |
| `tqdm` | `python3-tqdm` | 4.67.3 | ✓ |
| `pytest` | `python3-pytest` | 9.0.2 | ✓ |
| `mypy` | `python3-mypy` | 1.19.1 | ✓ |
| `loguru` | `python3-loguru` | 0.7.3 | ✓ |
| `psutil` | `python3-psutil` | 7.1.0 | ✓ |
| `cython` | `cython3` | 3.1.6 | ✓ |
| `numba` | `python3-numba` | 0.64.0 | ⚠ PyPI is 0.65.1 — one minor behind |
| `shapely` | `python3-shapely` | 2.1.2 | ✓ |
| `opencv-python` | `python3-opencv` | 4.10.0 | ⚠ PyPI is 4.13.0 — two minors behind |
| `av` | `python3-av` | 16.1.0 | ⚠ PyPI is 17.0.1 |
| `imageio-ffmpeg` | `python3-imageio-ffmpeg` | 0.6.0 | ✓ |
| `moviepy` | `python3-moviepy` | 2.1.2 | ✓ |
| `fonttools` | `python3-fonttools` | 4.61.1 | ✓ |
| `freetype-py` | `python3-freetype` | 2.5.1 | ✓ |
| `pynput` | `python3-pynput` | 1.8.1 | ✓ |
| `pyserial` | `python3-serial` | 3.5 | ✓ |
| `networkx` | `python3-networkx` | 3.2.1 | ✓ |
| `sortedcontainers` | `python3-sortedcontainers` | 2.4.0 | ✓ |
| `attrs` | `python3-attr` | 25.4.0 | ✓ |
| `zstd` | `python3-zstd` | 1.5.5.1 | ✓ (use this; `pyzstd` not in universe) |
| `cairocffi` | `python3-cairocffi` | 1.7.1 | ✓ |

### NOT in Universe — need vendoring via foundry-apt or pip

| PyPI Name | Ubuntu Package | Tier | Action |
|-----------|---------------|------|--------|
| `glfw` | `python3-glfw` | 2 — Recommended | **Vendor** — critical for OpenGL window/input outside pygame |
| `pydub` | `python3-pydub` | 2 — Recommended | **Vendor** — high-level audio manipulation; wraps ffmpeg |
| `ruff` | `ruff` | 2 — Recommended | **Vendor** — fast linter/formatter; replaces flake8+black |
| `librosa` | `python3-librosa` | 3 — Optional | **Vendor** — beat detection/DSP for rhythm games; all deps in universe |
| `mss` | `python3-mss` | 3 — Optional | **Vendor** — game recording & QA tooling; pure-ctypes, zero deps |
| `maturin` | `maturin` | 3 — Optional | **Vendor** — needed for Rust→Python extension builds |
| `pyzstd` | `python3-pyzstd` | 3 — Optional | Drop — `python3-zstd` covers it |

**Note on `rich`:** Universe ships 13.9.4; PyPI is 15.0.0. The 14→15 release dropped
several deprecated APIs. If any vendored tools pin to rich ≥ 14, the universe package will
break them. Monitor; may need to vendor a newer version.

---

## Summary Package List (for metapackage Depends:)

```
# foundry-python-gamedev — Essential + Recommended (all from ubuntu universe)
python3-numpy
python3-scipy
python3-cffi
python3-pil          # Pillow
python3-imageio
python3-pygame
python3-pyglet
python3-opengl       # PyOpenGL
python3-sounddevice
python3-soundfile
python3-yaml         # PyYAML
python3-tomli
python3-tomli-w
python3-msgpack
python3-lz4
python3-websockets
python3-aiohttp
python3-zmq          # pyzmq
python3-aiosqlite    # async sqlite3 for save data / leaderboards
python3-click
python3-rich
python3-tqdm
python3-pytest
python3-mypy
python3-loguru
python3-psutil
cython3
python3-numba
python3-shapely
# From foundry-apt (not in ubuntu universe):
python3-glfw
python3-pydub
ruff
```

```
# foundry-python-gamedev-extras — Tier 3 Optional
python3-opencv
python3-av
python3-imageio-ffmpeg
python3-moviepy
python3-fonttools
python3-freetype
python3-pynput
python3-serial
python3-networkx
python3-sortedcontainers
python3-attr
python3-zstd
python3-cairocffi
# From foundry-apt (not in ubuntu universe):
maturin
python3-librosa
python3-mss
```
