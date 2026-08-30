#!/usr/bin/env bash
# Install both enrolled applications with Electron 42 and exercise draw.io's
# real CLI export path in a clean Ubuntu 26.04 container.
set -euo pipefail

cd "$(dirname "$0")/.."
runtime_deb=$(find dist -maxdepth 1 -name 'electron-runtime-42_*_amd64.deb' -print | sort -V | tail -1)
losslesscut_deb=$(find dist -maxdepth 1 -name 'losslesscut_3.69.0-1foundry2_amd64.deb' -print -quit)
drawio_deb=$(find dist -maxdepth 1 -name 'drawio-desktop_31.3.1-1foundry1_amd64.deb' -print -quit)
for artifact in "$runtime_deb" "$losslesscut_deb" "$drawio_deb"; do
    [[ -n "$artifact" ]] || { echo "ERROR: required shared-Electron package is not built" >&2; exit 1; }
done

# Electron creates nested PID/network namespaces for its sandbox. Docker's
# default seccomp profile blocks that operation before Electron can test it.
docker run --rm -i \
    --security-opt seccomp=unconfined \
    -e DEBIAN_FRONTEND=noninteractive \
    -v "$PWD/dist:/debs:ro" \
    -v "$PWD/test/fixtures:/fixtures:ro" \
    ubuntu:26.04 bash -s -- \
    "$(basename "$runtime_deb")" "$(basename "$losslesscut_deb")" \
    "$(basename "$drawio_deb")" <<'EOF'
set -euo pipefail
apt-get update -qq
apt-get install -y --no-install-recommends file xvfb xauth \
    "/debs/$1" "/debs/$2" "/debs/$3" >/dev/null

test "$(dpkg-query -W -f='${db:Status-Status}' electron-runtime-42)" = installed
test "$(dpkg-query -W -f='${db:Status-Status}' losslesscut)" = installed
test "$(dpkg-query -W -f='${db:Status-Status}' drawio-desktop)" = installed
test "$(stat -c %a /usr/lib/electron-runtime-42/chrome-sandbox)" = 4755
test ! -e /usr/lib/drawio-desktop/electron
test ! -e /usr/lib/drawio-desktop/chrome-sandbox
test ! -e /usr/lib/drawio-desktop/resources/app-update.yml
test -f /usr/lib/drawio-desktop/resources/app.asar
grep -q '^Exec=/usr/bin/drawio %U$' /usr/share/applications/drawio.desktop
dpkg-query -W -f='${Depends}\n' drawio-desktop \
    | grep -F 'electron-runtime-42 (>= 42.9.3)'

useradd --create-home --shell /bin/bash electron-test
runuser -u electron-test -- /usr/lib/electron-runtime-42/electron --version \
    | grep -Fx 'v42.9.3'
install -o electron-test -g electron-test -m 0644 \
    /fixtures/shared-electron.drawio /tmp/shared-electron.drawio
runuser -u electron-test -- xvfb-run -a /usr/bin/drawio \
    --export --format png --output /tmp/shared-electron.png \
    /tmp/shared-electron.drawio
test -s /tmp/shared-electron.png
file /tmp/shared-electron.png | grep -F 'PNG image data'
/usr/lib/losslesscut/resources/ffmpeg -version | grep -m1 '^ffmpeg version '

echo "PASS: draw.io and LosslessCut share Electron 42; draw.io CLI export works"
EOF
