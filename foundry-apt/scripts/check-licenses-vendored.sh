#!/usr/bin/env bash
# check-licenses-vendored.sh — verify every packages/*/build.sh has an entry
# in LICENSES-VENDORED.md. Exits non-zero and lists missing packages if any.
#
# Run manually or via `task check-licenses`.
# Also wired as a Claude Code PostToolUse hook so adding a new build.sh
# immediately flags the missing entry.
set -euo pipefail

if [[ "${1:-}" == "-h" || "${1:-}" == "--help" ]]; then
    echo "Usage: check-licenses-vendored.sh"
    echo "Checks that every packages/*/build.sh has an entry in LICENSES-VENDORED.md."
    exit 0
fi

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
LICENSES_FILE="$REPO_ROOT/LICENSES-VENDORED.md"
PACKAGES_DIR="$REPO_ROOT/packages"

# Foundry-authored packages with build.sh — not third-party upstreams.
FOUNDRY_AUTHORED=(foundry-welcome)

missing=()
for build_sh in "$PACKAGES_DIR"/*/build.sh; do
    pkg="$(basename "$(dirname "$build_sh")")"
    # Skip foundry-authored packages.
    for authored in "${FOUNDRY_AUTHORED[@]}"; do
        [[ "$pkg" == "$authored" ]] && continue 2
    done
    # Check for a table row mentioning the package name.
    # Normalize U+2011 non-breaking hyphens to ASCII hyphens before matching
    # (package names in the table may use U+2011 to prevent line-break wrapping).
    if ! LC_ALL=C sed 's/\xe2\x80\x91/-/g' "$LICENSES_FILE" | grep -qE "^\| \`${pkg}\`"; then
        missing+=("$pkg")
    fi
done

if (( ${#missing[@]} == 0 )); then
    echo "PASS: all vendored packages are listed in LICENSES-VENDORED.md"
    exit 0
fi

echo "FAIL: the following packages have build.sh but no entry in LICENSES-VENDORED.md:" >&2
for pkg in "${missing[@]}"; do
    echo "  - $pkg" >&2
done
echo "" >&2
echo "Add a row to foundry-apt/LICENSES-VENDORED.md for each missing package." >&2
exit 1
