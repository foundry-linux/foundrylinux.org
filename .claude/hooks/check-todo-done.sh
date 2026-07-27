#!/usr/bin/env bash
# PostToolUse hook: reject TODO.md edits that leave [x] items in ## Open.
# Fires after every Edit/Write; exits 1 (blocks) when violations are found.
set -euo pipefail

fp="${CLAUDE_TOOL_INPUT_FILE_PATH:-}"
[[ "$fp" == *TODO.md ]] || exit 0
[[ -f "$fp" ]] || exit 0

# Count [x] lines that appear in ## Open, ## Watch, or ## Parked (before ## Done).
# The bracket may also carry a delegation tier — `[x T3]` from a TUI toggle is
# just as much a violation as a bare `[x]`, so match the status, not the
# literal bracket. Open markers ([ ], [T4], [wip T2], [verify T3]) never match.
violations=$(awk '/^## Done/{exit} /^- \[[[:space:]]*[xX]([[:space:]]+T[0-5])?[[:space:]]*\]/{n++} END{print n+0}' "$fp")

if [[ "$violations" -gt 0 ]]; then
    echo "TODO.md: $violations [x] item(s) remain in ## Open." >&2
    echo "  Move each to ## Done with a one-line summary (~130 chars)." >&2
    echo "  Format: - YYYY-MM-DD — [slug] description. See [plan](path)." >&2
    exit 1
fi
