#!/usr/bin/env bash
# Increment the patch component of foundry-iso/VERSION (e.g. 0.9.0 → 0.9.1).
# Does not commit — the caller (task iso-bump) does that.
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
VERSION_FILE="$SCRIPT_DIR/../VERSION"

OLD="$(cat "$VERSION_FILE")"
IFS='.' read -r MAJOR MINOR PATCH <<< "$OLD"
NEW="${MAJOR}.${MINOR}.$((PATCH + 1))"

echo "$NEW" > "$VERSION_FILE"
echo "Bumped $OLD → $NEW"
