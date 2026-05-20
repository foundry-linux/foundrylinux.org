#!/usr/bin/env bash
# blender-asset-finder-cli wrapper: run the CLI under its install-tree home
# so the script's sys.path injection finds
# /usr/share/blender-asset-finder-cli/blender_asset_finder/ (providers module).
set -euo pipefail
exec python3 /usr/share/blender-asset-finder-cli/wf_asset.py "$@"
