#!/usr/bin/env bash
# Increment the patch component of foundry-iso/VERSION (e.g. 0.9.0 → 0.9.1).
# Does not commit — the caller (task iso-bump) does that.
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
VERSION_FILE="$SCRIPT_DIR/../VERSION"

BINARY_FILE="$SCRIPT_DIR/../config/binary"

OLD="$(cat "$VERSION_FILE")"
IFS='.' read -r MAJOR MINOR PATCH <<< "$OLD"
NEW="${MAJOR}.${MINOR}.$((PATCH + 1))"

echo "$NEW" > "$VERSION_FILE"
# Keep config/binary's LB_ISO_VOLUME in sync so it doesn't need a manual edit.
sed -i "s/LB_ISO_VOLUME=\"FOUNDRY-ANVIL-[^\"]*\"/LB_ISO_VOLUME=\"FOUNDRY-ANVIL-${NEW}\"/" "$BINARY_FILE"
echo "Bumped $OLD → $NEW"
