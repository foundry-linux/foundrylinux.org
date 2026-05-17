#!/usr/bin/env bash
# Live install test: add apt.foundrylinux.org, then apt-get install each package.
# Runs each package in its own fresh Ubuntu container.
#
# Usage:
#   bash test/run-test.sh                              # all packages, full install
#   bash test/run-test.sh --download-only              # resolution check only
#   bash test/run-test.sh --package worldfoundry-engine-build-deps
#   bash test/run-test.sh --version 24.04              # Ubuntu version (default: 26.04)

set -euo pipefail

cd "$(dirname "$0")/.."

VERSION="26.04"
DOWNLOAD_ONLY=false
FILTER_PKG=""

while [[ $# -gt 0 ]]; do
    case "$1" in
        --download-only)   DOWNLOAD_ONLY=true ;;
        --package)         shift; FILTER_PKG="$1" ;;
        --package=*)       FILTER_PKG="${1#*=}" ;;
        --version)         shift; VERSION="$1" ;;
        --version=*)       VERSION="${1#*=}" ;;
        -h|--help)
            sed -n '2,8p' "$0" | sed 's/^# //; s/^#//'
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

IMAGE="ubuntu:${VERSION}"

# Discover packages from control files — always in sync with what's published.
mapfile -t ALL_PACKAGES < <(
    grep -h "^Package:" packages/*/DEBIAN/control | awk '{print $2}' | sort
)

if [[ -n "$FILTER_PKG" ]]; then
    PACKAGES=("$FILTER_PKG")
else
    PACKAGES=("${ALL_PACKAGES[@]}")
fi

INSTALL_FLAG=""
$DOWNLOAD_ONLY && INSTALL_FLAG="--download-only"

pass=0
fail=0

for pkg in "${PACKAGES[@]}"; do
    echo
    echo "=== Testing: apt-get install ${pkg} ==="

    # If this package depends on `task`, set up the Cloudsmith task repo first.
    EXTRA_REPOS=""
    control_file="packages/${pkg}/DEBIAN/control"
    if [[ -f "$control_file" ]] && grep -qE '(^Depends:|^\s)[^#]*\btask\b' "$control_file"; then
        EXTRA_REPOS="curl -1sLf 'https://dl.cloudsmith.io/public/task/task/setup.deb.sh' | bash"
    fi

    if $RUNTIME run --rm "$IMAGE" bash -c "
        set -euo pipefail
        export DEBIAN_FRONTEND=noninteractive
        apt-get update -q
        apt-get install -y --no-install-recommends curl gpg ca-certificates
        mkdir -p /etc/apt/keyrings
        curl -fsSL https://apt.foundrylinux.org/key.gpg \
            | gpg --dearmor -o /etc/apt/keyrings/foundry.gpg
        echo 'deb [signed-by=/etc/apt/keyrings/foundry.gpg] https://apt.foundrylinux.org resolute main' \
            > /etc/apt/sources.list.d/foundry.list
        ${EXTRA_REPOS}
        apt-get update -q
        apt-get install -y ${INSTALL_FLAG} ${pkg}
    "; then
        echo "[PASS] ${pkg}"
        pass=$((pass + 1))
    else
        echo "[FAIL] ${pkg}"
        fail=$((fail + 1))
    fi
done

echo
echo "Results: ${pass} passed, ${fail} failed"
[[ $fail -eq 0 ]]
