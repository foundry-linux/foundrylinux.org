#!/usr/bin/env bash
# Continue observing the live package container after a host service restart.
set -Eeuo pipefail
base=/srv/foundry-build
container=foundry-spike-packages-cold
run="$base/runs/diagnostic-continuation"
test ! -e "$run"
mkdir -p "$run"
exec > >(tee "$run/runner.log") 2>&1
finish() {
    result=$?
    trap - EXIT
    printf '%s\n' "$result" > "$run/exit-code"
    date -u +%FT%TZ > "$run/finished-at"
    sync
    shutdown -h +2 "Diagnostic repair complete; retain disks"
    exit "$result"
}
trap 'exit 143' TERM
trap 'exit 130' INT
trap finish EXIT
date -u +%FT%TZ > "$run/started-at"
cd "$base/source"
git diff --binary > "$run/source.patch"
docker logs --timestamps --follow --since 2026-11-15T06:19:50Z "$container" > "$run/packages.log" 2>&1 &
log_pid=$!
/usr/bin/time -v -o "$run/packages-wait.time" docker wait "$container" > "$run/container-wait-status"
wait "$log_pid" || true
status=$(docker inspect -f '{{.State.ExitCode}}' "$container")
printf '%s\n' "$status" > "$run/container-exit-code"
test "$status" = 0
date -u +%FT%TZ > "$run/packages-finished-at"
python3 /opt/foundry-spike/audit-packages.py --apt-root "$PWD/foundry-apt" > "$run/package-inventory.json"
find foundry-apt/dist -maxdepth 1 -type f -print0 | sort -z | xargs -0 -r sha256sum > "$run/packages.sha256"
mkdir -p foundry-iso/local-debs
cp foundry-apt/dist/*.deb foundry-iso/local-debs/
/usr/bin/time -v -o "$run/iso.time" env EDITION=anvil bash foundry-iso/scripts/build-iso.sh > "$run/iso.log" 2>&1
find foundry-iso/dist -maxdepth 1 -name '*.iso' -exec cp -al '{}' "$run/" \;
find "$run" -maxdepth 1 -name '*.iso' -exec sha256sum '{}' \; > "$run/iso.sha256"
echo 'Diagnostic build complete; real VM validation remains pending.'
