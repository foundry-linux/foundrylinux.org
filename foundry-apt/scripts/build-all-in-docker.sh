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

# Optional package filters, matching build-all.sh. Package names cannot contain
# whitespace, so a space-separated environment value is safe to split below.
PKG_FILTERS="$*"
BUILDER_IMAGE="foundry-apt-builder:ubuntu-26.04"
BUILDER_CACHE="$(pwd)/dist/.builder-cache"
mkdir -p "$BUILDER_CACHE/npm" "$BUILDER_CACHE/pnpm" "$BUILDER_CACHE/yarn"

# Keep the expensive Ubuntu/Node/Qt toolchain in a versioned Docker layer. The
# Dockerfile is still rebuilt when its inputs change, while repeated package
# builds no longer reinstall hundreds of packages in throwaway containers.
docker build --pull=false \
  -f docker/package-builder.Dockerfile \
  -t "$BUILDER_IMAGE" \
  docker

docker run --rm \
  -v "$(pwd):/work" \
  -v "$BUILDER_CACHE/npm:/root/.npm" \
  -v "$BUILDER_CACHE/pnpm:/root/.local/share/pnpm/store" \
  -v "$BUILDER_CACHE/yarn:/root/.cache/yarn" \
    -w /work \
    -e DEBIAN_FRONTEND=noninteractive \
    -e INCLUDE_SHARED_ELECTRON="${INCLUDE_SHARED_ELECTRON:-0}" \
    -e PKG_FILTERS="$PKG_FILTERS" \
  "$BUILDER_IMAGE" \
  bash -c '
    set -euo pipefail
    read -r -a filters <<< "$PKG_FILTERS"
    bash scripts/build-all.sh "${filters[@]}"
  '
