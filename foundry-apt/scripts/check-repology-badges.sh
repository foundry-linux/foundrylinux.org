#!/usr/bin/env bash
# check-repology-badges.sh — verify every packages/*/build.sh (vendored upstream)
# declares X-Repology-Project in its debian/control Source stanza, so the apt
# index renders the Repology version badge for the tools we package.
#
# Use `X-Repology-Project: none` for Foundry-authored or not-on-Repology
# packages — generate-index.sh skips the badge for `none`, but the field must
# still be present so the choice is deliberate (and new packages can't silently
# ship without a badge).
#
# Exits non-zero and lists offenders if any. Run manually or via
# `task check-badges`. Also wired as a git pre-commit hook and a Claude Code
# PostToolUse hook (fires when a packages/*/build.sh is edited).
set -euo pipefail

if [[ "${1:-}" == "-h" || "${1:-}" == "--help" ]]; then
    echo "Usage: check-repology-badges.sh"
    echo "Checks every packages/*/build.sh has X-Repology-Project in debian/control."
    echo "Use 'X-Repology-Project: none' to opt a package out of the Repology badge."
    exit 0
fi

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
PACKAGES_DIR="$REPO_ROOT/packages"

missing=()
for build_sh in "$PACKAGES_DIR"/*/build.sh; do
    pkg="$(basename "$(dirname "$build_sh")")"
    ctrl="$(dirname "$build_sh")/debian/control"
    if [[ ! -f "$ctrl" ]] || ! grep -qE "^X-Repology-Project:[[:space:]]*\S" "$ctrl"; then
        missing+=("$pkg")
    fi
done

if (( ${#missing[@]} == 0 )); then
    echo "PASS: all vendored packages declare X-Repology-Project"
    exit 0
fi

echo "FAIL: the following packages have build.sh but no X-Repology-Project in debian/control:" >&2
for pkg in "${missing[@]}"; do
    echo "  - $pkg" >&2
done
echo "" >&2
echo "Add 'X-Repology-Project: <repology-project>' to each Source stanza so the apt index" >&2
echo "shows the Repology badge. Use 'X-Repology-Project: none' for Foundry-authored or" >&2
echo "not-on-Repology packages (the badge is then skipped)." >&2
exit 1
