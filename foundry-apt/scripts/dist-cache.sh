#!/usr/bin/env bash
# Hydrate/persist the durable Foundry APT dist/ mirror at R2:foundry-apt/.dist-cache/.
# Extracted from publish.yml's hydrate/persist steps so the same code path
# runs in CI and under test/test-dist-cache.sh — see that file for the
# fail-closed behavior this implements.
#
# Usage:
#   dist-cache.sh hydrate   # pull the durable mirror into DIST_DIR
#   dist-cache.sh persist   # push DIST_DIR back to the durable mirror
#
# Reads the rclone remote config from the environment (RCLONE_CONFIG_R2_*, as
# publish.yml sets them). REMOTE and DIST_DIR override the rclone remote path
# and local directory (defaults: R2:foundry-apt/.dist-cache/ and ./dist/) —
# used by tests to point at a local fake bucket instead of Cloudflare R2.
# hydrate additionally reads TARGET_PACKAGES / DIST_CACHE_KEY to decide
# whether a hydration failure is fatal (see the three-branch guard below).
set -euo pipefail

usage() {
    sed -n '2,15p' "$0" | sed 's/^# \{0,1\}//'
}

if [[ "${1:-}" == "-h" || "${1:-}" == "--help" ]]; then
    usage
    exit 0
fi

REMOTE="${REMOTE:-R2:foundry-apt/.dist-cache/}"
DIST_DIR="${DIST_DIR:-./dist/}"

hydrate() {
    if rclone copy "$REMOTE" "$DIST_DIR" --checksum --progress --transfers=8; then
        return 0
    fi
    if [[ -n "${TARGET_PACKAGES:-}" && -z "${DIST_CACHE_KEY:-}" ]]; then
        echo "ERROR: R2 dist hydration failed during a targeted publish — refusing to continue." >&2
        exit 1
    fi
    if [[ -n "${TARGET_PACKAGES:-}" ]]; then
        echo "R2 dist hydration failed, but a secondary dist cache was restored; the completeness gate will verify it before publication."
        return 0
    fi
    echo "R2 dist mirror is not available yet; continuing only because this is a full bootstrap build."
}

persist() {
    # No fallback here by design: a persist failure (e.g. an unauthorized or
    # missing remote) must fail the workflow step closed, not swallow the
    # error and report a silently incomplete durable mirror.
    rclone sync "$DIST_DIR" "$REMOTE" --checksum --progress --transfers=8
}

case "${1:-}" in
    hydrate) hydrate ;;
    persist) persist ;;
    "")
        echo "ERROR: missing subcommand (hydrate|persist)" >&2
        usage >&2
        exit 1
        ;;
    *)
        echo "ERROR: unknown subcommand: $1" >&2
        usage >&2
        exit 1
        ;;
esac
