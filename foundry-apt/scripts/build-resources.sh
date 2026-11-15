#!/usr/bin/env bash
# Shared resource controls. These are limits to benchmark, not a 4-GiB claim.
export FOUNDRY_BUILD_JOBS=${FOUNDRY_BUILD_JOBS:-2}
export FOUNDRY_COMPRESSION_THREADS=${FOUNDRY_COMPRESSION_THREADS:-2}
export FOUNDRY_XZ_MEMLIMIT=${FOUNDRY_XZ_MEMLIMIT:-512MiB}
[[ $FOUNDRY_BUILD_JOBS =~ ^[1-9][0-9]*$ ]] || { echo 'Invalid FOUNDRY_BUILD_JOBS' >&2; return 1; }
[[ $FOUNDRY_COMPRESSION_THREADS =~ ^[1-9][0-9]*$ ]] || { echo 'Invalid FOUNDRY_COMPRESSION_THREADS' >&2; return 1; }
export CMAKE_BUILD_PARALLEL_LEVEL=$FOUNDRY_BUILD_JOBS
export CARGO_BUILD_JOBS=$FOUNDRY_BUILD_JOBS
export DEB_BUILD_OPTIONS=${DEB_BUILD_OPTIONS:-parallel=$FOUNDRY_BUILD_JOBS}
export CCACHE_DIR=${CCACHE_DIR:-/cache/ccache}
export CCACHE_MAXSIZE=${CCACHE_MAXSIZE:-2G}
# Ignore working-directory identity only through ccache's supported base-dir
# normalization. Do not enable sloppiness or trust cached compiler output blindly.
export CCACHE_BASEDIR=${CCACHE_BASEDIR:-/tmp/foundry-build}
export CCACHE_COMPILERCHECK=content
if command -v ccache >/dev/null; then
    # Use compiler-name wrappers once, not both wrappers and CMake launchers.
    export PATH="/usr/lib/ccache:$PATH"
fi
