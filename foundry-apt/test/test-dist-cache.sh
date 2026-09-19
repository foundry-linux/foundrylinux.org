#!/usr/bin/env bash
# Fail-closed test for scripts/dist-cache.sh (the hydrate/persist logic
# publish.yml delegates to for the R2:foundry-apt/.dist-cache/ durable
# mirror). Runs entirely against a local fake bucket — an rclone remote of
# type "local" pointed at a throwaway temp directory — so it never touches
# Cloudflare R2 or needs real credentials.
#
# Usage: bash test/test-dist-cache.sh

set -euo pipefail

if [[ "${1:-}" == "-h" || "${1:-}" == "--help" ]]; then
    sed -n '2,10p' "$0" | sed 's/^# \{0,1\}//'
    exit 0
fi

command -v rclone >/dev/null || { echo "ERROR: rclone is required" >&2; exit 1; }

cd "$(dirname "$0")/.."
DIST_CACHE_SH="$(pwd)/scripts/dist-cache.sh"

WORK=$(mktemp -d)
trap 'rm -rf "$WORK"' EXIT

# A local-backend rclone remote named FAKE stands in for the R2 remote named
# "R2" in publish.yml — same rclone verbs/flags, different backend, driven by
# the same RCLONE_CONFIG_*_* env-var convention the workflow uses.
export RCLONE_CONFIG_FAKE_TYPE=local

pass=0
fail=0

assert_eq() {
    local desc="$1" expected="$2" actual="$3"
    if [[ "$expected" == "$actual" ]]; then
        echo "  ok $desc"
        pass=$((pass + 1))
    else
        echo "FAIL: $desc (expected [$expected], got [$actual])" >&2
        fail=$((fail + 1))
    fi
}

assert_status() {
    # assert_status <desc> <expected exit code> -- <command...>
    local desc="$1" expected="$2"
    shift 2
    [[ "${1:-}" == "--" ]] && shift
    local status=0
    "$@" >"$WORK/last-stdout" 2>"$WORK/last-stderr" || status=$?
    if [[ "$status" -eq "$expected" ]]; then
        echo "  ok $desc (exit $status)"
        pass=$((pass + 1))
    else
        echo "FAIL: $desc (expected exit $expected, got $status)" >&2
        echo "  --- stdout ---" >&2; sed 's/^/  /' "$WORK/last-stdout" >&2
        echo "  --- stderr ---" >&2; sed 's/^/  /' "$WORK/last-stderr" >&2
        fail=$((fail + 1))
    fi
}

echo "=== hydrate pulls files from a populated remote ==="
bucket="$WORK/bucket-populated"
dist="$WORK/dist-hydrate"
mkdir -p "$bucket"
echo "package-a" > "$bucket/foundry-core_1.0.0_all.deb"
echo "package-b" > "$bucket/xemu_0.8_amd64.deb"
REMOTE="FAKE:$bucket/" DIST_DIR="$dist/" \
    assert_status "hydrate exits 0 against a populated remote" 0 -- bash "$DIST_CACHE_SH" hydrate
assert_eq "hydrate pulled foundry-core .deb" "package-a" "$(cat "$dist/foundry-core_1.0.0_all.deb" 2>/dev/null || echo MISSING)"
assert_eq "hydrate pulled xemu .deb" "package-b" "$(cat "$dist/xemu_0.8_amd64.deb" 2>/dev/null || echo MISSING)"

echo
echo "=== persist pushes files to the remote ==="
bucket="$WORK/bucket-persist-target"
dist="$WORK/dist-persist"
mkdir -p "$dist"
echo "built-here" > "$dist/foundry-core_1.0.1_all.deb"
REMOTE="FAKE:$bucket/" DIST_DIR="$dist/" \
    assert_status "persist exits 0 against a writable remote" 0 -- bash "$DIST_CACHE_SH" persist
assert_eq "persist pushed the .deb to the bucket" "built-here" "$(cat "$bucket/foundry-core_1.0.1_all.deb" 2>/dev/null || echo MISSING)"

echo
echo "=== persist FAILS CLOSED against a missing/unauthorized remote ==="
# Model "unauthorized" with a parent directory rclone cannot write into or
# create a child of — the same failure shape as a real 403/AccessDenied: the
# destination exists conceptually but this credential cannot write to it.
denied_parent="$WORK/denied-parent"
mkdir -p "$denied_parent"
chmod 000 "$denied_parent"
dist="$WORK/dist-denied"
mkdir -p "$dist"
echo "should-not-vanish-silently" > "$dist/rpcs3_0.0.42_amd64.deb"
REMOTE="FAKE:$denied_parent/sub/" DIST_DIR="$dist/" \
    assert_status "persist exits non-zero (fails closed) rather than silently succeeding" 1 -- \
    bash "$DIST_CACHE_SH" persist
chmod 755 "$denied_parent"

echo
echo "=== hydrate against an empty remote is a clean no-op ==="
# An existing-but-empty directory models an S3 prefix with no objects yet —
# the real first-bootstrap case. (A genuinely NONEXISTENT local directory
# behaves differently from an empty S3 prefix — rclone errors on it for the
# local backend but not for S3 — so this deliberately pre-creates the dir;
# see the branch below for the "prefix doesn't exist at all" case instead.)
bucket="$WORK/bucket-empty"
dist="$WORK/dist-empty"
mkdir -p "$bucket"
REMOTE="FAKE:$bucket/" DIST_DIR="$dist/" \
    assert_status "hydrate exits 0 against an empty remote" 0 -- bash "$DIST_CACHE_SH" hydrate
if [[ -d "$dist" ]] && [[ -n "$(find "$dist" -mindepth 1 2>/dev/null)" ]]; then
    echo "FAIL: hydrate against an empty remote should leave DIST_DIR empty" >&2
    fail=$((fail + 1))
else
    echo "  ok hydrate left DIST_DIR empty (clean no-op)"
    pass=$((pass + 1))
fi

echo
echo "=== hydrate FAILS CLOSED for a targeted publish with no fallback cache ==="
# publish.yml's three-branch guard: a targeted publish (TARGET_PACKAGES set)
# with no restored secondary cache (DIST_CACHE_KEY empty) must refuse to
# continue rather than build from a possibly-incomplete dist/.
dist="$WORK/dist-targeted-no-fallback"
mkdir -p "$dist"
TARGET_PACKAGES="xemu" DIST_CACHE_KEY="" REMOTE="FAKE:$WORK/does-not-exist/" DIST_DIR="$dist/" \
    assert_status "hydrate exits 1 for a targeted publish with a failed hydration and no fallback" 1 -- \
    bash "$DIST_CACHE_SH" hydrate
if grep -q "refusing to continue" "$WORK/last-stderr"; then
    echo "  ok error message names the refusal"
    pass=$((pass + 1))
else
    echo "FAIL: expected a 'refusing to continue' message on stderr" >&2
    fail=$((fail + 1))
fi

echo
echo "=== hydrate WARNS BUT CONTINUES for a targeted publish with a restored fallback cache ==="
dist="$WORK/dist-targeted-with-fallback"
mkdir -p "$dist"
TARGET_PACKAGES="xemu" DIST_CACHE_KEY="some-cache-key" REMOTE="FAKE:$WORK/does-not-exist/" DIST_DIR="$dist/" \
    assert_status "hydrate exits 0 for a targeted publish when a fallback cache was restored" 0 -- \
    bash "$DIST_CACHE_SH" hydrate

echo
echo "=== -h/--help exits 0 without touching rclone ==="
assert_status "dist-cache.sh -h exits 0" 0 -- bash "$DIST_CACHE_SH" -h
assert_status "dist-cache.sh with no subcommand exits non-zero" 1 -- bash "$DIST_CACHE_SH"

echo
echo "Results: ${pass} passed, ${fail} failed"
[[ $fail -eq 0 ]]
