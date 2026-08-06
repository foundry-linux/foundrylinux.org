#!/usr/bin/env bash
# Build every foundry-apt .deb inside an ubuntu:26.04 container.
#
# All .debs MUST be built inside ubuntu:26.04 so ${shlibs:Depends} resolves to
# 26.04's library sonames (libavcodec62, etc.) and the build deps (debhelper,
# cmake, qt6) are present -- mirrors publish.yml and build-iso.sh. Building on
# the host silently pins to the host's sonames / fails on missing debhelper.
# build-all.sh re-owns dist/*.deb back to the host user at the end.
set -euo pipefail
cd "$(dirname "$0")/.."   # foundry-apt/

# Optional single-package filter, matching build-all.sh.
PKG_FILTER="${1:-}"

docker run --rm \
  -v "$(pwd):/work" \
  -w /work \
  -e DEBIAN_FRONTEND=noninteractive \
  -e PKG_FILTER="$PKG_FILTER" \
  ubuntu:26.04 \
  bash -c '
    set -euo pipefail
    apt-get update -q
    apt-get install -y --no-install-recommends \
      build-essential debhelper dpkg-dev devscripts fakeroot lintian \
      curl ca-certificates pkg-config sudo \
      zip python3 \
      cmake qt6-base-dev qt6-declarative-dev
    bash scripts/build-all.sh "$PKG_FILTER"
  '
