#!/usr/bin/env bash
# check-repology-badges.sh — verify (or report) X-Repology-Project coverage
# across vendored packages (everything with a packages/*/build.sh wrapper).
#
# Modes:
#   (default)   GUARD  — exit non-zero if any vendored package lacks the field.
#               Wired as a git pre-commit hook + Claude Code PostToolUse hook.
#   --report    AUDIT  — print every vendored package + its badge status; exit 0.
#               Surfaced as `task audit-badges`.
#
# The apt index (generate-index.sh) renders a Repology version badge for any
# package whose debian/control Source stanza declares X-Repology-Project. Use
# `X-Repology-Project: none` to opt out (Foundry-authored / not-on-Repology):
# the field is still required (so the choice is deliberate) but generate-index
# skips the badge.
set -euo pipefail

REPORT=false
case "${1:-}" in
    -h|--help)
        echo "Usage: check-repology-badges.sh [--report]"
        echo "  (default)  guard: fail if any packages/*/build.sh lacks X-Repology-Project"
        echo "  --report   audit: list every vendored package + its badge status, exit 0"
        echo "Use 'X-Repology-Project: none' to opt a package out of the Repology badge."
        exit 0
        ;;
    --report|-r) REPORT=true ;;
    "") ;;
    *) echo "ERROR: unknown arg: $1 (try --help)" >&2; exit 2 ;;
esac

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
PACKAGES_DIR="$REPO_ROOT/packages"

missing=()
have=0
optout=0
report_rows=()
for build_sh in "$PACKAGES_DIR"/*/build.sh; do
    pkg="$(basename "$(dirname "$build_sh")")"
    ctrl="$(dirname "$build_sh")/debian/control"
    val=""
    if [[ -f "$ctrl" ]]; then
        val="$(grep -m1 '^X-Repology-Project:' "$ctrl" | sed 's/^X-Repology-Project:[[:space:]]*//' || true)"
    fi
    if [[ -z "$val" ]]; then
        missing+=("$pkg")
        report_rows+=("⚠️ MISSING|$pkg|—")
    elif [[ "${val,,}" == "none" ]]; then
        optout=$((optout + 1))
        report_rows+=("➖ opt-out|$pkg|none")
    else
        have=$((have + 1))
        report_rows+=("✅ badge  |$pkg|$val")
    fi
done

if $REPORT; then
    printf '%-12s  %-28s  %s\n' "STATUS" "PACKAGE" "REPOLOGY-PROJECT"
    printf '%-12s  %-28s  %s\n' "------" "-------" "----------------"
    for row in "${report_rows[@]}"; do
        IFS='|' read -r st pk vl <<<"$row"
        printf '%-12s  %-28s  %s\n' "$st" "$pk" "$vl"
    done
    echo "---"
    echo "badge: ${have}   opt-out(none): ${optout}   MISSING: ${#missing[@]}   total: ${#report_rows[@]}"
    exit 0
fi

if (( ${#missing[@]} == 0 )); then
    echo "PASS: all vendored packages declare X-Repology-Project (${have} badged, ${optout} opt-out)"
    exit 0
fi

echo "FAIL: the following packages have build.sh but no X-Repology-Project in debian/control:" >&2
for pkg in "${missing[@]}"; do
    echo "  - $pkg" >&2
done
echo "" >&2
echo "Add 'X-Repology-Project: <repology-project>' to each Source stanza so the apt index" >&2
echo "shows the Repology badge. Use 'X-Repology-Project: none' for Foundry-authored or" >&2
echo "not-on-Repology packages (the badge is then skipped). Run 'task audit-badges' to review." >&2
exit 1
