#!/usr/bin/env bash
# Stop hook: block completion while a Foundry APT publish is waiting for the
# wald3n.com open-source refresh/deploy verification.
set -euo pipefail

cat >/dev/null || true
root=$(git rev-parse --show-toplevel 2>/dev/null || pwd)
marker="$root/.claude/state/package-publish-wald3n.pending"
[[ -f "$marker" ]] || exit 0

reason="A Foundry APT release was started, but wald3n.com/open-source has not been verified. Do not report publication complete. Wait for the APT workflow, refresh and review the clean wald3n.com snapshot, deploy it, then run 'task package-publish:complete'."
jq -n --arg reason "$reason" '{decision:"block", reason:$reason}'
