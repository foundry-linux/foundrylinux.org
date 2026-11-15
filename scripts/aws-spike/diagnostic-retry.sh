#!/usr/bin/env bash
# One explicit repair attempt on the existing diagnostic instance only.
set -Eeuo pipefail
base=/srv/foundry-build
container=foundry-spike-packages-cold
run="$base/runs/diagnostic-retry"
test ! -e "$run"
mkdir -p "$run"
exec > >(tee "$run/runner.log") 2>&1
while [[ $(docker inspect -f '{{.State.Running}}' "$container") == true ]]; do
    sleep 10
done
# The original controller may schedule shutdown or advance after docker attach;
# stop it before starting an explicitly labelled repair attempt.
systemctl stop foundry-spike-cold-resume2.service || true
shutdown -c || true
finish() {
    result=$?
    trap - EXIT
    printf '%s\n' "$result" > "$run/exit-code"
    date -u +%FT%TZ > "$run/finished-at"
    sync
    shutdown -h +2 "Diagnostic repair complete; retain disks"
    exit "$result"
}
trap finish EXIT
date -u +%FT%TZ > "$run/started-at"
# Do not hide an additional package failure behind the two known repairs.
grep '^FAIL ' "$base/runs/cold/packages.log" > "$run/original-failures.txt" || true
if grep -Ev '^FAIL (flycast|rpcs3) ' "$run/original-failures.txt"; then
    echo 'Additional failures require investigation before retry' >&2
    exit 1
fi
cd "$base/source"
git diff --binary > "$run/source.patch"
/usr/bin/time -v -o "$run/packages.time" docker start -a "$container" > "$run/packages.log" 2>&1
status=$(docker inspect -f '{{.State.ExitCode}}' "$container")
printf '%s\n' "$status" > "$run/container-exit-code"
test "$status" = 0
date -u +%FT%TZ > "$run/packages-finished-at"
find foundry-apt/dist -maxdepth 1 -type f -print0 | sort -z | xargs -0 -r sha256sum > "$run/packages.sha256"
mkdir -p foundry-iso/local-debs
cp foundry-apt/dist/*.deb foundry-iso/local-debs/
/usr/bin/time -v -o "$run/iso.time" env EDITION=anvil bash foundry-iso/scripts/build-iso.sh > "$run/iso.log" 2>&1
find foundry-iso/dist -maxdepth 1 -name '*.iso' -exec cp -al '{}' "$run/" \;
find "$run" -maxdepth 1 -name '*.iso' -exec sha256sum '{}' \; > "$run/iso.sha256"
echo 'Diagnostic build complete; real VM validation remains pending.'
