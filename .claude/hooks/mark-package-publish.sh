#!/usr/bin/env bash
# PostToolUse[Bash]: mark a Foundry APT release as incomplete until the
# wald3n.com open-source inventory has been refreshed, deployed, and verified.
set -euo pipefail

input=$(cat || true)
command=$(jq -r '.tool_input.command // empty' <<<"$input" 2>/dev/null || true)

# These are the release entrypoints in the root Taskfile. Avoid matching the
# reusable devbox/ISO tasks, which publish different artifacts.
if ! grep -qE '(^|[;&|[:space:]])task[[:space:]]+(bump|release|sync-and-release)([[:space:]]|$)' <<<"$command"; then
    exit 0
fi

root=$(git rev-parse --show-toplevel 2>/dev/null || pwd)
state_dir="$root/.claude/state"
marker="$state_dir/package-publish-wald3n.pending"
snapshot="${WALD3N_ROOT:-$root/../wald3n.com}/src/data/open-source.json"
mkdir -p "$state_dir"

# Record package directories absent from the pre-refresh snapshot. The
# completion check uses this list for precise live-page assertions.
{
    printf 'published_at=%s\n' "$(date -u +%Y-%m-%dT%H:%M:%SZ)"
    printf 'command=%s\n' "$command"
    git -C "$root" ls-tree -d --name-only HEAD:foundry-apt/packages \
        | while read -r package_path; do
            package=${package_path##*/}
            if [[ -f "$snapshot" ]] && jq -e --arg package "$package" \
                '.packages[] | select(.name == $package)' \
                "$snapshot" >/dev/null 2>&1; then
                continue
            fi
            printf 'package=%s\n' "$package"
        done
} >"$marker"

echo "Foundry APT release detected: wald3n.com/open-source verification is now required." >&2
echo "After the APT workflow succeeds, refresh/deploy wald3n.com, then run:" >&2
echo "  task package-publish:complete" >&2
