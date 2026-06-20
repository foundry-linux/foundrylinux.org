#!/usr/bin/env bash
# check-publish-metadata.sh — enforce a clean Origin/Label on the published Release.
#
# aptly defaults Origin/Label to ". <suite>" (e.g. ". resolute") unless you pass
# -origin/-label to `aptly publish repo`. apt clients and Repology surface those
# fields, so a derivative repo should present its own vendor name. publish-local.sh
# MUST pass:
#     -origin="Foundry Linux" -label="Foundry Linux"
#
# Wired as a Claude Code PostToolUse hook (runs when publish-local.sh is edited)
# and runnable via `task check-publish-meta`. GUARD: exits non-zero if either flag
# is missing, so the convention can't silently regress.
set -euo pipefail

case "${1:-}" in
    -h|--help)
        echo "Usage: $(basename "$0")"
        echo "Exit non-zero if scripts/publish-local.sh lacks the Origin/Label publish flags."
        exit 0
        ;;
esac

cd "$(dirname "$0")/.."
PUBLISH="scripts/publish-local.sh"
ORIGIN="Foundry Linux"
LABEL="Foundry Linux"

fail=0
if ! grep -qF -- "-origin=\"${ORIGIN}\"" "$PUBLISH"; then
    echo "FAIL: ${PUBLISH} is missing  -origin=\"${ORIGIN}\"  on 'aptly publish repo'" >&2
    fail=1
fi
if ! grep -qF -- "-label=\"${LABEL}\"" "$PUBLISH"; then
    echo "FAIL: ${PUBLISH} is missing  -label=\"${LABEL}\"  on 'aptly publish repo'" >&2
    fail=1
fi

if (( fail )); then
    echo "  The published Release must carry Origin/Label \"${ORIGIN}\" (clean apt-client" >&2
    echo "  presentation + Repology). Restore the flags on the 'aptly ... publish repo' line." >&2
    exit 1
fi

echo "PASS: publish-local.sh sets Origin/Label = \"${ORIGIN}\""
