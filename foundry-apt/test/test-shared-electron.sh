#!/usr/bin/env bash
# Verify the versioned Electron runtime and split LosslessCut package in a clean
# Ubuntu 26.04 container. This does not require a display server.
set -euo pipefail

cd "$(dirname "$0")/.."
runtime_deb=$(find dist -maxdepth 1 -name 'electron-runtime-42_*_amd64.deb' -print | sort -V | tail -1)
app_deb=$(find dist -maxdepth 1 -name 'losslesscut_3.69.0-1foundry2_amd64.deb' -print -quit)
[[ -n "$runtime_deb" && -n "$app_deb" ]] || {
    echo "ERROR: build electron-runtime-42 and losslesscut first" >&2
    exit 1
}

docker run --rm -i \
    --security-opt seccomp=unconfined \
    -e DEBIAN_FRONTEND=noninteractive \
    -v "$PWD/dist:/debs:ro" \
    ubuntu:26.04 bash -s -- "$(basename "$runtime_deb")" "$(basename "$app_deb")" <<'EOF'
set -euo pipefail
runtime_deb=$1
app_deb=$2
apt-get update -qq
apt-get install -y --no-install-recommends "/debs/$runtime_deb" "/debs/$app_deb" >/dev/null

test "$(dpkg-query -W -f='${db:Status-Status}' electron-runtime-42)" = installed
test "$(dpkg-query -W -f='${db:Status-Status}' losslesscut)" = installed
test "$(stat -c %a /usr/lib/electron-runtime-42/chrome-sandbox)" = 4755
test -x /usr/lib/losslesscut/resources/ffmpeg
test -z "$(find /usr/lib/losslesscut -maxdepth 1 -type f \
    \( -name electron -o -name chrome-sandbox \) -print -quit)"

useradd --create-home --shell /bin/bash electron-test
runuser -u electron-test -- /usr/lib/electron-runtime-42/electron --version \
    | grep -Fx 'v42.9.3'
/usr/lib/losslesscut/resources/ffmpeg -version | grep -m1 '^ffmpeg version '

echo "PASS: shared Electron packages install and basic executables run"
EOF
