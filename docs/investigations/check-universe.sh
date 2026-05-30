#!/usr/bin/env bash
# Check which recommended python game-dev packages are in Ubuntu 26.04 universe.
# Run inside: docker run --rm -v $PWD:/work ubuntu:26.04 bash /work/docs/investigations/check-universe.sh
set -euo pipefail

apt-get update -qq 2>/dev/null

# Format: "pypi-name ubuntu-pkg-name"
PACKAGES=(
  # Tier 1 — Essential
  "numpy              python3-numpy"
  "scipy              python3-scipy"
  "cffi               python3-cffi"
  "pillow             python3-pil"
  "imageio            python3-imageio"
  "setuptools         python3-setuptools"
  "pip                python3-pip"
  "wheel              python3-wheel"
  # Tier 2 — Recommended: Frameworks
  "pygame             python3-pygame"
  "pyglet             python3-pyglet"
  "pyopengl           python3-opengl"
  "glfw               python3-glfw"
  # Tier 2 — Audio
  "sounddevice        python3-sounddevice"
  "soundfile          python3-soundfile"
  "pydub              python3-pydub"
  # Tier 2 — Serialization
  "pyyaml             python3-yaml"
  "tomli              python3-tomli"
  "tomli-w            python3-tomli-w"
  "msgpack            python3-msgpack"
  "lz4                python3-lz4"
  # Tier 2 — Networking
  "websockets         python3-websockets"
  "aiohttp            python3-aiohttp"
  "pyzmq              python3-zmq"
  # Tier 2 — Dev Tools
  "click              python3-click"
  "rich               python3-rich"
  "tqdm               python3-tqdm"
  "pytest             python3-pytest"
  "ruff               ruff"
  "mypy               python3-mypy"
  "loguru             python3-loguru"
  "psutil             python3-psutil"
  # Tier 2 — Performance
  "cython             cython3"
  "numba              python3-numba"
  # Tier 2 — Geometry
  "shapely            python3-shapely"
  # Tier 3 — Optional
  "opencv-python      python3-opencv"
  "av                 python3-av"
  "imageio-ffmpeg     python3-imageio-ffmpeg"
  "moviepy            python3-moviepy"
  "librosa            python3-librosa"
  "fonttools          python3-fonttools"
  "freetype-py        python3-freetype"
  "mss                python3-mss"
  "pynput             python3-pynput"
  "pyserial           python3-serial"
  "maturin            maturin"
  "networkx           python3-networkx"
  "sortedcontainers   python3-sortedcontainers"
  "attrs              python3-attr"
  "pyzstd             python3-pyzstd"
  "zstd               python3-zstd"
  "cairocffi          python3-cairocffi"
)

printf "%-22s %-26s %-12s %s\n" "PyPI Name" "Ubuntu Package" "Status" "Version"
printf "%-22s %-26s %-12s %s\n" "----------" "--------------" "------" "-------"

for entry in "${PACKAGES[@]}"; do
  pypi=$(echo "$entry" | awk '{print $1}')
  pkg=$(echo "$entry" | awk '{print $2}')
  policy=$(apt-cache policy "$pkg" 2>/dev/null || true)
  candidate=$(echo "$policy" | grep "Candidate:" | awk '{print $2}' || true)
  if [[ -z "$candidate" || "$candidate" == "(none)" ]]; then
    printf "%-22s %-26s %-12s %s\n" "$pypi" "$pkg" "NOT IN REPO" ""
  else
    printf "%-22s %-26s %-12s %s\n" "$pypi" "$pkg" "in universe" "$candidate"
  fi
done
