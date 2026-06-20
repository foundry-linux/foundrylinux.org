#!/usr/bin/env bash
# set-repology-badge.sh — opt a vendored package into the apt-index Repology
# badge (or change / clear it) by setting its X-Repology-Project field.
#
# Usage:
#   bash scripts/set-repology-badge.sh <package> <repology-project>
#   bash scripts/set-repology-badge.sh <package> none      # opt out (no badge)
#
# Edits packages/<package>/debian/control's Source stanza in place. This is
# source-only metadata — no .deb rebuild is needed; generate-meta.sh picks it up
# on the next publish and the badge appears in the index. After running:
#   task audit-badges      # review
#   task bump              # publish so the badge goes live
set -euo pipefail

if [[ "${1:-}" == "-h" || "${1:-}" == "--help" || $# -lt 2 ]]; then
    cat <<EOF
Usage: set-repology-badge.sh <package> <repology-project|none>

Set (or change/clear) a vendored package's X-Repology-Project field — i.e. opt it
into the apt-index Repology badge. Find the project name at
https://repology.org/projects/?search=<term> (use the exact project slug, e.g.
'python:picire' for PyPI packages). Use 'none' to opt out.

Examples:
  set-repology-badge.sh task go-task
  set-repology-badge.sh m8te none
EOF
    [[ $# -lt 2 && "${1:-}" != "-h" && "${1:-}" != "--help" ]] && exit 2
    exit 0
fi

PKG="$1"
PROJ="$2"

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
CTL="$REPO_ROOT/packages/$PKG/debian/control"

if [[ ! -f "$CTL" ]]; then
    echo "ERROR: no debian/control for package '$PKG' (looked at $CTL)" >&2
    echo "       Is the name right? See: ls $REPO_ROOT/packages/" >&2
    exit 1
fi

if grep -q '^X-Repology-Project:' "$CTL"; then
    old="$(grep -m1 '^X-Repology-Project:' "$CTL" | sed 's/^X-Repology-Project:[[:space:]]*//')"
    sed -i "s|^X-Repology-Project:.*|X-Repology-Project: ${PROJ}|" "$CTL"
    echo "✓ ${PKG}: X-Repology-Project ${old} → ${PROJ}"
elif grep -q '^Homepage:' "$CTL"; then
    sed -i "/^Homepage:/a X-Repology-Project: ${PROJ}" "$CTL"
    echo "✓ ${PKG}: added X-Repology-Project: ${PROJ} (after Homepage)"
else
    sed -i "/^Maintainer:/a X-Repology-Project: ${PROJ}" "$CTL"
    echo "✓ ${PKG}: added X-Repology-Project: ${PROJ} (after Maintainer)"
fi

if [[ "${PROJ,,}" == "none" ]]; then
    echo "  (opt-out — the index will render no badge for ${PKG})"
else
    echo "  badge → https://repology.org/project/${PROJ}/versions"
    echo "  publish to make it live:  task bump"
fi
