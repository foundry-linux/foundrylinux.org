#!/usr/bin/env bash
# Catch known Debian/Ubuntu development-package splits before a long compile.

set -euo pipefail
cd "$(dirname "$0")/.."

fail=0
for control in packages/*/debian/control; do
    [[ -f "$control" ]] || continue

    # X11 integration is deliberately split from libxkbcommon's core headers:
    # xkbcommon/xkbcommon.h       -> libxkbcommon-dev
    # xkbcommon/xkbcommon-x11.h   -> libxkbcommon-x11-dev
    # A package using both xkbcommon and X11 almost invariably compiles the
    # latter backend too. Require the known companion explicitly instead of
    # rediscovering the missing header late in a large C/C++ build.
    if grep -Eq '^[[:space:]]*libxkbcommon-dev([,[:space:]]|$)' "$control" &&
       grep -Eq '^[[:space:]]*libx11-dev([,[:space:]]|$)' "$control" &&
       ! grep -Eq '^[[:space:]]*libxkbcommon-x11-dev([,[:space:]]|$)' "$control"; then
        echo "FAIL: $control declares libxkbcommon-dev + libx11-dev but not libxkbcommon-x11-dev" >&2
        fail=1
    fi
done

if (( fail )); then
    exit 1
fi

echo "PASS: known Build-Depends companion-package pairs are complete"
