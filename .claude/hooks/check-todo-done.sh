#!/usr/bin/env bash
# PostToolUse hook: reject TODO.md edits that leave [x] items in ## Open.
# Fires after every Edit/Write; exits 1 (blocks) when violations are found.
set -euo pipefail

fp="${CLAUDE_TOOL_INPUT_FILE_PATH:-}"
[[ "$fp" == *TODO.md ]] || exit 0
[[ -f "$fp" ]] || exit 0

# Count [x] lines that appear in ## Open or ## Parked (before ## Done).
violations=$(awk '/^## Done/{exit} /^- \[x\]/{n++} END{print n+0}' "$fp")

if [[ "$violations" -gt 0 ]]; then
    echo "TODO.md: $violations [x] item(s) remain in ## Open." >&2
    echo "  Move each to ## Done with a one-line summary (~130 chars)." >&2
    echo "  Format: - YYYY-MM-DD — [slug] description. See [plan](path)." >&2
    exit 1
fi
