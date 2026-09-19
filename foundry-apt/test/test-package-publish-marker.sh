#!/usr/bin/env bash
# Marker lifecycle test for the package-publish -> wald3n.com completion
# guard:
#   .claude/hooks/mark-package-publish.sh       (PostToolUse hook) creates it
#   .claude/hooks/check-package-publish-complete.sh (Stop hook) blocks on it
#   scripts/complete-package-publish.sh          clears it once verified
#
# Runs entirely inside a throwaway temp git repo with copies of the three
# scripts and a stub wald3n.com layout (WALD3N_ROOT override) plus PATH
# shims for `task` and `curl` — never touches this repo's real
# .claude/state/, the real wald3n.com checkout, or the network.
#
# Usage: bash test/test-package-publish-marker.sh

set -euo pipefail

if [[ "${1:-}" == "-h" || "${1:-}" == "--help" ]]; then
    sed -n '2,12p' "$0" | sed 's/^# \{0,1\}//'
    exit 0
fi

MONOREPO_ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
MARK_SCRIPT="$MONOREPO_ROOT/.claude/hooks/mark-package-publish.sh"
CHECK_SCRIPT="$MONOREPO_ROOT/.claude/hooks/check-package-publish-complete.sh"
COMPLETE_SCRIPT="$MONOREPO_ROOT/scripts/complete-package-publish.sh"

for f in "$MARK_SCRIPT" "$CHECK_SCRIPT" "$COMPLETE_SCRIPT"; do
    [[ -f "$f" ]] || { echo "ERROR: missing $f" >&2; exit 1; }
done
command -v jq >/dev/null || { echo "ERROR: jq is required" >&2; exit 1; }

WORK=$(mktemp -d)
trap 'rm -rf "$WORK"' EXIT

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

# --- fixture: a throwaway git repo standing in for the foundrylinux.org
# monorepo, and a throwaway wald3n.com layout the scripts read/write via
# WALD3N_ROOT. Neither is the real checkout. ---
FIXTURE="$WORK/foundrylinux.org"
mkdir -p "$FIXTURE/foundry-apt/packages/pkg-a"
: > "$FIXTURE/foundry-apt/packages/pkg-a/build.sh"
git -C "$FIXTURE" init -q -b main
git -C "$FIXTURE" config user.email "test@example.invalid"
git -C "$FIXTURE" config user.name "package-publish marker test"
git -C "$FIXTURE" -c commit.gpgsign=false add -A
git -C "$FIXTURE" -c commit.gpgsign=false commit -q -m "fixture: one package"

WALD_ROOT="$WORK/wald3n.com"
mkdir -p "$WALD_ROOT/src/data"

marker="$FIXTURE/.claude/state/package-publish-wald3n.pending"

# PATH shims: `task` never needs a real Taskfile, and `curl` never leaves
# the machine. Both are looked up via PATH before the real binaries.
mkdir -p "$WORK/bin"
cat > "$WORK/bin/task" <<'EOF'
#!/usr/bin/env bash
# Test stub: pretend every `task <name>` invocation (open-source:refresh:check,
# open-source:verify) succeeds, without a real Taskfile or wald3n.com toolchain.
exit 0
EOF
cat > "$WORK/bin/curl" <<'EOF'
#!/usr/bin/env bash
# Test stub: never touches the network. Prints the fixture's canned HTML for
# complete-package-publish.sh's `curl https://wald3n.com/open-source` check.
cat "${STUB_CURL_OUTPUT_FILE:?STUB_CURL_OUTPUT_FILE not set}"
EOF
chmod +x "$WORK/bin/task" "$WORK/bin/curl"
export PATH="$WORK/bin:$PATH"
export WALD3N_ROOT="$WALD_ROOT"

echo "=== (a) mark-package-publish.sh: no-ops for a non-release command ==="
write_snapshot() { echo "$1" > "$WALD_ROOT/src/data/open-source.json"; }
write_snapshot '{"packages":[]}'
rm -f "$marker"
echo '{"tool_input":{"command":"task build"}}' | (cd "$FIXTURE" && assert_status "non-release command exits 0" 0 -- bash "$MARK_SCRIPT")
if [[ -f "$marker" ]]; then
    echo "FAIL: marker should not exist after a non-release command" >&2
    fail=$((fail + 1))
else
    echo "  ok no marker written"
    pass=$((pass + 1))
fi

echo
echo "=== (a) mark-package-publish.sh: creates the marker for a release command ==="
echo '{"tool_input":{"command":"task bump"}}' | (cd "$FIXTURE" && assert_status "release command exits 0" 0 -- bash "$MARK_SCRIPT")
if [[ -f "$marker" ]]; then
    echo "  ok marker file created"
    pass=$((pass + 1))
else
    echo "FAIL: marker file was not created" >&2
    fail=$((fail + 1))
fi
assert_eq "marker records the package" "package=pkg-a" "$(grep '^package=' "$marker" || true)"
assert_eq "marker records the triggering command" "command=task bump" "$(grep '^command=' "$marker" || true)"
if grep -qE '^published_at=[0-9]{4}-[0-9]{2}-[0-9]{2}T' "$marker"; then
    echo "  ok marker records an ISO-8601 published_at"
    pass=$((pass + 1))
else
    echo "FAIL: marker missing a well-formed published_at" >&2
    fail=$((fail + 1))
fi

echo
echo "=== (a) mark-package-publish.sh: merges without duplicating an already-current package ==="
write_snapshot '{"packages":[{"name":"pkg-a"}]}'
rm -f "$marker"
echo '{"tool_input":{"command":"task release"}}' | (cd "$FIXTURE" && assert_status "release command exits 0" 0 -- bash "$MARK_SCRIPT")
assert_eq "pkg-a already in the snapshot is omitted from a fresh marker" "" "$(grep '^package=' "$marker" || true)"
write_snapshot '{"packages":[]}'   # restore for the rest of the test

echo
echo "=== (b) check-package-publish-complete.sh: blocks while the marker exists ==="
echo '{"tool_input":{"command":"task bump"}}' | (cd "$FIXTURE" && bash "$MARK_SCRIPT") >/dev/null 2>&1
(cd "$FIXTURE" && assert_status "Stop hook exits 0 even while blocking" 0 -- bash "$CHECK_SCRIPT" < /dev/null)
decision=$(jq -r '.decision // empty' "$WORK/last-stdout" 2>/dev/null || true)
assert_eq "Stop hook reports decision=block while the marker exists" "block" "$decision"

echo
echo "=== (b) check-package-publish-complete.sh: silent + non-blocking once the marker is gone ==="
rm -f "$marker"
(cd "$FIXTURE" && assert_status "Stop hook exits 0 with no marker" 0 -- bash "$CHECK_SCRIPT" < /dev/null)
assert_eq "Stop hook prints nothing once the marker is gone" "" "$(cat "$WORK/last-stdout")"

echo
echo "=== (c) complete-package-publish.sh: refuses when the snapshot lacks the package ==="
echo '{"tool_input":{"command":"task bump"}}' | (cd "$FIXTURE" && bash "$MARK_SCRIPT") >/dev/null 2>&1
write_snapshot '{"packages":[{"name":"some-other-package"}]}'
STUB_CURL_OUTPUT_FILE="$WORK/live-page-irrelevant.html"
echo '<html></html>' > "$STUB_CURL_OUTPUT_FILE"
export STUB_CURL_OUTPUT_FILE
(cd "$FIXTURE" && assert_status "refuses when pkg-a is absent from the refreshed snapshot" 1 -- bash "$COMPLETE_SCRIPT")
if [[ -f "$marker" ]]; then
    echo "  ok marker retained after a refused completion"
    pass=$((pass + 1))
else
    echo "FAIL: marker must be retained when completion is refused" >&2
    fail=$((fail + 1))
fi
if grep -q "absent from the refreshed wald3n.com snapshot" "$WORK/last-stderr"; then
    echo "  ok error names the missing package"
    pass=$((pass + 1))
else
    echo "FAIL: expected an 'absent from the refreshed wald3n.com snapshot' error" >&2
    fail=$((fail + 1))
fi

echo
echo "=== (c) complete-package-publish.sh: clears the marker once snapshot + live page agree ==="
write_snapshot '{"packages":[{"name":"pkg-a"}]}'
STUB_CURL_OUTPUT_FILE="$WORK/live-page-with-pkg-a.html"
echo '<table><tr data-sort-name="pkg-a"><td>pkg-a</td></tr></table>' > "$STUB_CURL_OUTPUT_FILE"
export STUB_CURL_OUTPUT_FILE
(cd "$FIXTURE" && assert_status "completes once the package is verified live" 0 -- bash "$COMPLETE_SCRIPT")
if grep -q "^PASS:" "$WORK/last-stdout"; then
    echo "  ok prints a PASS line"
    pass=$((pass + 1))
else
    echo "FAIL: expected a PASS line on stdout" >&2
    fail=$((fail + 1))
fi
if [[ -f "$marker" ]]; then
    echo "FAIL: marker should be removed after a successful completion" >&2
    fail=$((fail + 1))
else
    echo "  ok marker removed"
    pass=$((pass + 1))
fi

echo
echo "=== (c) complete-package-publish.sh: no-ops cleanly when there is nothing pending ==="
(cd "$FIXTURE" && assert_status "no-op exits 0 when no marker exists" 0 -- bash "$COMPLETE_SCRIPT")
assert_eq "no-op reports nothing pending" "No pending Foundry APT publication." "$(cat "$WORK/last-stdout")"

echo
echo "Results: ${pass} passed, ${fail} failed"
[[ $fail -eq 0 ]]
