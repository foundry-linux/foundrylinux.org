#!/usr/bin/env bash
# Rebuild the complete local Foundry Linux ISO set against the current Ubuntu
# 26.04 archive and the locally rebuilt Foundry APT overlay. This deliberately
# stops before signing or publishing: those are explicit release actions.

set -euo pipefail

usage() {
    cat <<'EOF'
Refresh the local Foundry Linux distribution artifacts.

Usage:
  bash scripts/refresh-distro.sh [--edition anvil|atelier|all] [--no-smoke]
                                  [--inventory <path>] [--force-inventory]

Defaults:
  --edition all
  --inventory docs/investigations/<UTC-timestamp>-ubuntu-26.04-refresh.md

The refresh:
  1. records Ubuntu 26.04 archive candidates relative to the stock base image;
  2. checks every APT source used by the ISO;
  3. rebuilds all Foundry APT packages and stages them into the ISO;
  4. force-rebuilds the selected ISO editions from fresh APT indexes; and
  5. boots each resulting ISO in the QEMU smoke test, unless --no-smoke.

It does not sign, upload, tag, or publish any artifact. The existing iso-build
task increments foundry-iso/VERSION and creates its local version-bump commit.
EOF
}

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
EDITION="all"
SMOKE=1
INVENTORY=""
FORCE_INVENTORY=0

while [[ $# -gt 0 ]]; do
    case "$1" in
        -h|--help) usage; exit 0 ;;
        --edition)
            [[ $# -ge 2 ]] || { echo "ERROR: --edition needs a value" >&2; exit 2; }
            EDITION="$2"
            shift 2
            ;;
        --no-smoke) SMOKE=0; shift ;;
        --inventory)
            [[ $# -ge 2 ]] || { echo "ERROR: --inventory needs a path" >&2; exit 2; }
            INVENTORY="$2"
            shift 2
            ;;
        --force-inventory) FORCE_INVENTORY=1; shift ;;
        *) echo "ERROR: unknown argument: $1" >&2; usage >&2; exit 2 ;;
    esac
done

case "$EDITION" in anvil|atelier|all) ;; *)
    echo "ERROR: --edition must be anvil, atelier, or all" >&2
    exit 2
esac

cd "$REPO_ROOT"

command -v docker >/dev/null || { echo "ERROR: docker is required" >&2; exit 3; }
command -v task >/dev/null || { echo "ERROR: task is required" >&2; exit 3; }
docker info >/dev/null 2>&1 || { echo "ERROR: cannot reach the Docker daemon" >&2; exit 3; }

if [[ -z "$INVENTORY" ]]; then
    INVENTORY="docs/investigations/$(date -u +%Y-%m-%dT%H%M%SZ)-ubuntu-26.04-refresh.md"
fi
[[ "$INVENTORY" = /* ]] || INVENTORY="$REPO_ROOT/$INVENTORY"
if [[ -e "$INVENTORY" && $FORCE_INVENTORY -ne 1 ]]; then
    echo "ERROR: inventory already exists: $INVENTORY" >&2
    echo "       Choose --inventory <path> or pass --force-inventory." >&2
    exit 4
fi
mkdir -p "$(dirname "$INVENTORY")"

echo "=== Capturing Ubuntu 26.04 base-image update candidates ==="
{
    printf '# Ubuntu 26.04 refresh evidence\n\n'
    printf 'Generated: %s\n\n' "$(date -u +%FT%TZ)"
    printf 'This compares the stock `ubuntu:26.04` image with the current Ubuntu archive. '
    printf 'The ISO build below resolves its package set from the same fresh archive indexes.\n\n'
    printf '## Simulated base-image upgrade\n\n```text\n'
    docker run --rm --dns 8.8.8.8 ubuntu:26.04 bash -c '
        set -euo pipefail
        apt-get update -qq
        apt-get -s dist-upgrade
    '
    printf '```\n'
} > "$INVENTORY"

echo "Wrote $INVENTORY"
echo "=== Checking ISO APT sources ==="
task check-apt-repos

echo "=== Rebuilding Foundry APT overlay and ISO edition(s): $EDITION ==="
# --force bypasses Task's ISO output cache. iso-build itself rebuilds every
# local Foundry package, mirrors those .debs into local-debs/, and resolves the
# Kubuntu/Ubuntu package set from fresh indexes inside Ubuntu 26.04.
task --force iso-build EDITION="$EDITION"

if (( SMOKE )); then
    echo "=== Boot smoke test: $EDITION ==="
    task iso-smoke EDITION="$EDITION"
fi

{
    printf '\n## Built artifacts\n\n```text\n'
    shopt -s nullglob
    for iso in "$REPO_ROOT"/foundry-iso/dist/foundry-{anvil,atelier}-*-amd64.iso; do
        [[ "$EDITION" == all || "$(basename "$iso")" == "foundry-${EDITION}-"* ]] || continue
        sha256sum "$iso"
    done
    printf '```\n\n'
    printf 'Smoke test: %s\n' "$([[ $SMOKE -eq 1 ]] && echo passed || echo skipped)"
    printf '\nThe local refresh is complete. Sign and publish only after reviewing these artifacts.\n'
} >> "$INVENTORY"

echo "Refresh complete. Evidence: $INVENTORY"
