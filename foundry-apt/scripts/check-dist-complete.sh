#!/usr/bin/env bash
# Refuse to rebuild the published repo from a truncated dist/ directory.
set -euo pipefail
cd "$(dirname "$0")/.."

PACKAGES_URL="${PACKAGES_URL:-https://apt.foundrylinux.org/dists/resolute/main/binary-amd64/Packages.gz}"
DIST_DIR="${DIST_DIR:-dist}"
packages_file=$(mktemp)
trap 'rm -f "$packages_file"' EXIT

# Fail closed: an unreachable or malformed live index must stop publication.
curl -fsSL "$PACKAGES_URL" -o "$packages_file"
gzip -t "$packages_file"

LIVE=$(gzip -dc "$packages_file" | awk '/^Package: / { count++ } END { print count+0 }')
LOCAL=$(find "$DIST_DIR" -maxdepth 1 -type f -name '*.deb' -printf '.\n' 2>/dev/null | wc -l)

if (( LIVE == 0 )); then
    echo "ERROR: live repo reports no binary packages — refusing to publish without a valid baseline." >&2
    exit 1
fi

if (( LOCAL < LIVE )); then
    echo "ERROR: dist/ has $LOCAL .debs but the live repo publishes $LIVE — refusing to publish a truncated index." >&2
    exit 1
fi

echo "dist/ completeness check passed: $LOCAL local .debs, $LIVE live packages"
