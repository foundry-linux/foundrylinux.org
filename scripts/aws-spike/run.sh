#!/usr/bin/env bash
# Temporary measurement runner. Keep all outputs/caches for the paired run.
set -Eeuo pipefail
mode=${1:?cold or warm}
[[ $mode == cold || $mode == warm ]]
base=/srv/foundry-build
source /opt/foundry-spike/config.sh
run="$base/runs/$mode"
mkdir -p "$run"
exec 9>/run/foundry-spike.lock
flock -n 9
exec > >(tee -a "$run/runner.log") 2>&1
finish() {
    result=$?
    trap - EXIT
    if [[ -n ${observer:-} ]]; then kill "$observer" 2>/dev/null || true; fi
    if [[ -n ${monitor:-} ]]; then kill "$monitor" 2>/dev/null || true; fi
    printf '%s\n' "$result" > "$run/exit-code"
    date -u +%FT%TZ > "$run/finished-at"
    docker ps -a > "$run/containers.txt" 2>&1 || true
    df -h > "$run/disk-final.txt"
    sync
    shutdown -h +2 "Foundry spike $mode finished (exit $result); disks retained"
    exit "$result"
}
trap finish EXIT
# Preserve every attempt instead of overwriting a failed run's measurements.
if [[ -e "$run/started-at" ]]; then
    echo "Run directory already used; preserve it and choose a new attempt before rerunning" >&2
    trap - EXIT
    exit 1
fi
python3 /opt/foundry-spike/observe.py --output "$run/raw-metrics.jsonl" &
observer=$!
date -u +%FT%TZ > "$run/started-at"
stage() {
    local name=$1
    shift
    printf '%s START %s\n' "$(date -u +%FT%TZ)" "$name" | tee -a "$run/stages.tsv"
    /usr/bin/time -v -o "$run/$name.time" "$@" > >(tee "$run/$name.log") 2>&1
    printf '%s PASS %s\n' "$(date -u +%FT%TZ)" "$name" | tee -a "$run/stages.tsv"
}
export DEBIAN_FRONTEND=noninteractive
cd "$base"
if [[ ! -d source/.git ]]; then
    stage clone git clone https://github.com/foundry-linux/foundrylinux.org.git source
fi
cd source
git checkout --detach "$SOURCE_COMMIT"
git rev-parse HEAD > "$run/source-commit"
for patch in /opt/foundry-spike/patches/*.patch; do
    if git apply --reverse --check "$patch" 2>/dev/null; then
        : # Already applied on the retained warm checkout.
    else
        git apply --check "$patch"
        git apply "$patch"
    fi
    sha256sum "$patch" >> "$run/patches.sha256"
done
git diff --binary > "$run/source.patch"
cp /opt/foundry-spike/stacks-source.txt "$run/stacks-source.txt"
docker images --digests > "$run/images-before.txt"
df -h > "$run/disk-before.txt"
sar -u -r -b -d 5 > "$run/resources.sar.txt" &
monitor=$!

root_image=$(python3 -c 'import json; print(json.load(open("/opt/foundry-spike/stacks/stacks.lock.json"))["roots"]["ubuntu"]["upstream"])')
stage stack-base docker build --pull=false --build-arg "BASE=$root_image" -t foundry-spike/stack-base:run /opt/foundry-spike/stacks/stack-base-ubuntu
stage stack-build docker build --pull=false --build-arg STACK_BASE=foundry-spike/stack-base:run -t foundry-spike/stack-build:run /opt/foundry-spike/stacks/stack-build-ubuntu
stage stack-deb docker build --pull=false --build-arg STACK_BASE=foundry-spike/stack-build:run -t foundry-spike/stack-deb:run /opt/foundry-spike/stacks/stack-deb
# Reuse the project's recipe with the freshly rebuilt standard parent, without
# pretending it is the unavailable registry's original image digest.
sed 's|^FROM registry.lab.home.arpa:5000/stacks/stack-deb@.*|FROM foundry-spike/stack-deb:run|' foundry-apt/docker/package-builder.Dockerfile > "$base/package-builder.Dockerfile"
stage package-builder docker build --pull=false -f "$base/package-builder.Dockerfile" -t foundry-spike/packages:run foundry-apt/docker
docker image inspect foundry-spike/packages:run > "$run/builder-image.json"
mkdir -p "$base/cache/apt" "$base/cache/npm" "$base/cache/pnpm" "$base/cache/yarn"
if [[ $mode == warm ]]; then
    stage packages-cache python3 /opt/foundry-spike/cache.py verify --repo "$PWD" --receipt "$base/packages-cache.json"
elif docker container inspect "foundry-spike-packages-$mode" >/dev/null 2>&1; then
    echo "Existing package container invalidates a clean cold attempt" >&2
    exit 1
else
stage packages docker run --name "foundry-spike-packages-$mode" \
    -v "$PWD/foundry-apt:/work" -v "$base/cache/apt:/var/cache/apt/archives" \
    -v "$base/cache/npm:/root/.npm" -v "$base/cache/pnpm:/root/.local/share/pnpm/store" \
    -v "$base/cache/yarn:/root/.cache/yarn" -w /work \
    -e DEBIAN_FRONTEND=noninteractive -e CMAKE_BUILD_PARALLEL_LEVEL=2 \
    -e DEB_BUILD_OPTIONS=parallel=2 -e CARGO_BUILD_JOBS=2 \
    foundry-spike/packages:run bash -c 'set -euo pipefail; rm -f /etc/apt/apt.conf.d/docker-clean; apt-get update; bash scripts/build-all.sh'
fi
if [[ $mode == cold ]]; then
    # docker start -a may return success even when the container command failed.
    test "$(docker inspect -f '{{.State.ExitCode}}' "foundry-spike-packages-$mode")" = 0
    python3 /opt/foundry-spike/cache.py record --repo "$PWD" --receipt "$base/packages-cache.json"
fi
find foundry-apt/dist -maxdepth 1 -type f -print0 | sort -z | xargs -0 -r sha256sum > "$run/packages.sha256"
mkdir -p foundry-iso/local-debs
cp foundry-apt/dist/*.deb foundry-iso/local-debs/
stage iso env EDITION=anvil bash foundry-iso/scripts/build-iso.sh
find foundry-iso/dist -maxdepth 1 -name '*.iso' -exec cp -al '{}' "$run/" \;
find "$run" -maxdepth 1 -name '*.iso' -exec sha256sum '{}' \; > "$run/iso.sha256"
kill "$monitor" 2>/dev/null || true
echo 'Build stages finished; ISO VM validation is a separate measured stage.'
