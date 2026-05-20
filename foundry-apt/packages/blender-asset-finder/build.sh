#!/usr/bin/env bash
# blender-asset-finder: Blender extension repack. Vendors the
# wftools/blender_asset_finder/ source dir from upstream and the deb's
# debian/rules zips it into a Blender 4.2+ extension archive.
set -euo pipefail
export NAME=blender-asset-finder
export SOURCE_DIR=blender_asset_finder
exec bash "$(dirname "$0")/../../scripts/build-rust-tool.sh"
