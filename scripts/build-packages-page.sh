#!/usr/bin/env bash
# build-packages-page.sh — regenerate site/packages-data.json from the live
# apt indexes of apt.foundrylinux.org + apt.worldfoundry.org + Ubuntu archive.
# Pass --force to skip the Release-SHA cache check.
#
# Pure Node — no docker. ~5 seconds. See docs/plans/2026-05-21-packages-page.md §1.
set -euo pipefail

FORCE=0
for a in "$@"; do
  case "$a" in
    -h|--help)
      cat <<'EOF'
build-packages-page.sh — regenerate site/packages-data.json

Usage: build-packages-page.sh [--force]

  --force    Skip the Release-SHA cache check and always re-fetch.
  -h, --help Show this help and exit.

Output: site/packages-data.json
EOF
      exit 0 ;;
    --force) FORCE=1 ;;
    *) echo "unknown arg: $a" >&2; exit 2 ;;
  esac
done

ROOT="$(git rev-parse --show-toplevel 2>/dev/null || pwd)"
cd "$ROOT"

foundry_sha="$(curl -fsSL https://apt.foundrylinux.org/dists/resolute/Release 2>/dev/null | sha256sum | awk '{print $1}' || true)"
world_sha="$(  curl -fsSL https://apt.worldfoundry.org/dists/stable/Release    2>/dev/null | sha256sum | awk '{print $1}' || true)"

if [[ -z "$foundry_sha" || -z "$world_sha" ]]; then
  echo "✗ could not fetch Release files from both apt repos" >&2
  exit 4
fi

inputs_sha="$(cat data/categories.json data/upstream.yml | sha256sum | awk '{print $1}')"

if [[ -f site/packages-data.json && $FORCE -eq 0 ]]; then
  prev_foundry="$(jq -r .foundry_release_sha      site/packages-data.json 2>/dev/null || true)"
  prev_world="$(  jq -r .worldfoundry_release_sha site/packages-data.json 2>/dev/null || true)"
  prev_inputs="$( jq -r '.inputs_sha // ""'       site/packages-data.json 2>/dev/null || true)"
  if [[ "$foundry_sha" == "$prev_foundry" && "$world_sha" == "$prev_world" && "$inputs_sha" == "$prev_inputs" ]]; then
    echo "✓ no change in apt repos or local config — skipping"
    exit 0
  fi
fi

export INPUTS_SHA="$inputs_sha"

if ! command -v node >/dev/null 2>&1; then
  echo "✗ node required (Node 18+; uses built-in fetch + zlib)" >&2
  exit 5
fi

WORK_ROOT="$ROOT" node scripts/build-packages-data.js
echo "✓ site/packages-data.json regenerated"
