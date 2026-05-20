#!/usr/bin/env bash
# blender-asset-finder-cli: Python CLI for 3D-asset search/download. Vendors:
#   SOURCE_DIR=blender_asset_finder → wftools/blender_asset_finder/*
#                                     (providers.py + JSON catalogs that
#                                     providers.py needs)
#   EXTRA_FILES=wf_asset.py         → wftools/wf_asset.py
#                                     (the CLI script that imports
#                                     providers via sys.path injection)
set -euo pipefail
export NAME=blender-asset-finder-cli
export SOURCE_DIR=blender_asset_finder
export EXTRA_FILES="wf_asset.py"
exec bash "$(dirname "$0")/../../scripts/build-rust-tool.sh"
