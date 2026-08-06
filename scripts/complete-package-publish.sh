#!/usr/bin/env bash
# Clear the package-publish guard only after the sibling wald3n.com snapshot is
# current and every newly discovered package is visible on the live page.
set -euo pipefail

root=$(git rev-parse --show-toplevel 2>/dev/null || pwd)
marker="$root/.claude/state/package-publish-wald3n.pending"
wald_root=${WALD3N_ROOT:-"$root/../wald3n.com"}
snapshot="$wald_root/src/data/open-source.json"

if [[ ! -f "$marker" ]]; then
    echo "No pending Foundry APT publication."
    exit 0
fi
if [[ ! -f "$snapshot" ]]; then
    echo "ERROR: wald3n.com snapshot not found: $snapshot" >&2
    exit 1
fi

echo "=== Checking refreshed wald3n.com inventory ==="
(cd "$wald_root" && task open-source:refresh:check)
(cd "$wald_root" && task open-source:verify)

mapfile -t packages < <(sed -n 's/^package=//p' "$marker")
for package in "${packages[@]}"; do
    if ! jq -e --arg package "$package" \
        '.packages[] | select(.name == $package)' \
        "$snapshot" >/dev/null; then
        echo "ERROR: $package is absent from the refreshed wald3n.com snapshot" >&2
        exit 1
    fi
done

echo "=== Checking live https://wald3n.com/open-source ==="
live=$(curl -fsSL https://wald3n.com/open-source)
for package in "${packages[@]}"; do
    if ! grep -Fq "data-sort-name=\"$package\"" <<<"$live"; then
        echo "ERROR: $package is absent from the live wald3n.com/open-source page" >&2
        exit 1
    fi
    echo "  ok $package"
done

rm -f "$marker"
echo "PASS: APT publication and wald3n.com/open-source are both complete."
