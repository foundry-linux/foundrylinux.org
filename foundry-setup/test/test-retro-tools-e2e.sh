#!/usr/bin/env bash
# End-to-end test: install foundry-retro-tools in a fresh Ubuntu 26.04
# container and verify every tool from the metapackage Depends: actually runs.
#
# Usage: bash foundry-setup/test/test-retro-tools-e2e.sh [--version 26.04]
#
# Exits 0 on full pass, 1 on any signal-terminated tool or install failure.

set -euo pipefail

cd "$(dirname "$0")/../.."

VERSION="26.04"

while [[ $# -gt 0 ]]; do
    case "$1" in
        --version)   shift; VERSION="$1" ;;
        --version=*) VERSION="${1#*=}" ;;
        -h|--help)
            sed -n '2,7p' "$0" | sed 's/^# //; s/^#//'
            exit 0
            ;;
        *) echo "Unknown arg: $1" >&2; exit 1 ;;
    esac
    shift
done

if command -v podman &>/dev/null; then
    RUNTIME=podman
elif command -v docker &>/dev/null; then
    RUNTIME=docker
else
    echo "ERROR: podman or docker required" >&2
    exit 1
fi

echo "=== retro-tools E2E on ubuntu:${VERSION} via ${RUNTIME} ==="
exec $RUNTIME run --rm \
    -v "$(pwd):/work:ro" \
    "ubuntu:${VERSION}" \
    bash /work/foundry-setup/test/test-retro-tools-e2e-inner.sh
