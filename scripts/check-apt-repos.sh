#!/usr/bin/env bash
# check-apt-repos.sh — health-check every apt source Foundry ships.
#
# For each `deb` line, resolve its release-index URL (InRelease, falling back
# to Release) and fail loudly on a non-200. This catches the silent-rot class
# where an upstream rotates its repo layout — e.g. go-task's Cloudsmith repo
# moved `any-distro/any-version` -> `ubuntu/<codename>`, so the old source line
# started 404'ing and broke `apt update` on built images. A package-inventory
# tool (scripts/audit-apt-repos.sh) won't catch a dead source line; this does.
#
# Usage:
#   scripts/check-apt-repos.sh                 # check foundry-iso/config/archives/*.list.chroot
#   scripts/check-apt-repos.sh FILE...         # check specific sources files
#   scripts/check-apt-repos.sh --host          # also check this machine's /etc/apt sources
#   scripts/check-apt-repos.sh -h | --help
#
# Exit 0 = every source's InRelease (or Release) returned HTTP 200.
# Exit 1 = one or more sources unreachable (offenders listed).
#
# No host deps beyond bash + curl.

set -euo pipefail

usage() { sed -n '2,21p' "$0" | sed 's/^# \{0,1\}//'; exit "${1:-0}"; }

REPO_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
HOST=0
FILES=()

for arg in "$@"; do
    case "$arg" in
        -h|--help) usage 0 ;;
        --host)    HOST=1 ;;
        -*)        echo "unknown option: $arg" >&2; usage 1 ;;
        *)         FILES+=("$arg") ;;
    esac
done

# Default target set: the source lines the ISO bakes into built images.
if [[ ${#FILES[@]} -eq 0 ]]; then
    while IFS= read -r f; do FILES+=("$f"); done \
        < <(find "$REPO_ROOT/foundry-iso/config/archives" -name '*.list.chroot' 2>/dev/null | sort)
fi
if [[ "$HOST" -eq 1 ]]; then
    for f in /etc/apt/sources.list /etc/apt/sources.list.d/*.list; do
        [[ -f "$f" ]] && FILES+=("$f")
    done
fi

[[ ${#FILES[@]} -gt 0 ]] || { echo "check-apt-repos: no sources files to check" >&2; exit 1; }

# Resolve a one-line `deb [opts] URL SUITE [components...]` to its index URL.
index_url() {
    local line="$1" url suite
    line="${line#deb }"
    line="$(sed -E 's/^\[[^]]*\][[:space:]]*//' <<<"$line")"   # drop [signed-by=...] block
    read -r url suite _ <<<"$line"
    [[ -z "$url" ]] && return 1
    url="${url%/}"
    case "$suite" in
        "")  echo "$url/InRelease" ;;             # flat repo, no trailing slash given
        */)  echo "$url/${suite}InRelease" ;;     # flat repo (suite ends in /)
        *)   echo "$url/dists/$suite/InRelease" ;; # standard repo
    esac
}

http_code() { curl -s -o /dev/null -w '%{http_code}' --max-time 15 "$1?cb=$(date +%s%N)"; }

fail=0
checked=0
for f in "${FILES[@]}"; do
    [[ -f "$f" ]] || { echo "  ⚠ skip (missing): $f" >&2; continue; }
    while IFS= read -r line; do
        line="${line%%#*}"                       # strip comments
        line="${line#"${line%%[![:space:]]*}"}"  # ltrim
        [[ "$line" == deb\ * ]] || continue       # only binary deb lines
        idx="$(index_url "$line")" || continue
        checked=$((checked + 1))
        code="$(http_code "$idx")"
        if [[ "$code" == "200" ]]; then
            printf '  ✓ %s\n' "$idx"
        else
            # Some repos ship only Release (+Release.gpg), no InRelease.
            rel="${idx%InRelease}Release"
            rcode="$(http_code "$rel")"
            if [[ "$rcode" == "200" ]]; then
                printf '  ✓ %s (Release; no InRelease)\n' "$rel"
            else
                printf '  ✗ %s → HTTP %s (Release: %s)   [%s]\n' "$idx" "$code" "$rcode" "$(basename "$f")" >&2
                fail=1
            fi
        fi
    done < "$f"
done

echo
if [[ "$fail" -eq 0 ]]; then
    echo "check-apt-repos: OK ($checked source(s) reachable)"
else
    echo "check-apt-repos: FAIL — one or more sources unreachable (see ✗ above)" >&2
    echo "  A 404 usually means the upstream rotated its repo layout/suite." >&2
    echo "  Update the offending .list.chroot (or vendor the package into foundry-apt)." >&2
fi
exit "$fail"
