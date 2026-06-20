#!/usr/bin/env bash
# prune-dist.sh — keep only the newest version of each package in dist/.
#
# After incremental builds (or a restored CI build cache), dist/ can hold
# several versions of the same package. `aptly repo add dist/` would then
# publish the stale old versions alongside the new one. This removes every
# .deb superseded by a higher version of the same package, leaving exactly
# one (the newest) per package name — so the published repo stays clean and
# the saved cache doesn't grow unbounded.
set -euo pipefail

if [[ "${1:-}" == "-h" || "${1:-}" == "--help" ]]; then
    echo "Usage: prune-dist.sh [--dry-run|-n]"
    echo "Keep only the newest .deb per package in dist/; remove superseded ones."
    exit 0
fi

DRY_RUN=false
[[ "${1:-}" == "--dry-run" || "${1:-}" == "-n" ]] && DRY_RUN=true

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
DIST="$REPO_ROOT/dist"

shopt -s nullglob
debs=("$DIST"/*.deb)
if (( ${#debs[@]} == 0 )); then
    echo "prune-dist: no .debs in dist/ — nothing to do"
    exit 0
fi

# Find the newest version per package name.
declare -A best_ver best_file
for deb in "${debs[@]}"; do
    name="$(dpkg-deb -f "$deb" Package)"
    ver="$(dpkg-deb -f "$deb" Version)"
    if [[ -z "${best_ver[$name]:-}" ]] || dpkg --compare-versions "$ver" gt "${best_ver[$name]}"; then
        best_ver[$name]="$ver"
        best_file[$name]="$deb"
    fi
done

pruned=0
for deb in "${debs[@]}"; do
    name="$(dpkg-deb -f "$deb" Package)"
    if [[ "$deb" != "${best_file[$name]}" ]]; then
        echo "prune $(basename "$deb")  (superseded by ${name} ${best_ver[$name]})"
        $DRY_RUN || rm -f "$deb"
        pruned=$((pruned + 1))
    fi
done

suffix=""
$DRY_RUN && suffix=" (dry-run; nothing removed)"
echo "prune-dist: kept ${#best_ver[@]} package(s), ${pruned} superseded .deb(s) removed${suffix}"
