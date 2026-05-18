#!/usr/bin/env bash
# Live install test: add apt.foundrylinux.org, then apt-get install each package.
# Runs each package in its own fresh Ubuntu container.
#
# Usage:
#   bash test/run-test.sh                              # all packages, full install
#   bash test/run-test.sh --download-only              # resolution check only
#   bash test/run-test.sh --package foundry-linux-retro-tools
#   bash test/run-test.sh --version 24.04              # Ubuntu version (default: 26.04)

set -euo pipefail

cd "$(dirname "$0")/.."

VERSION="26.04"
DOWNLOAD_ONLY=false
FILTER_PKG=""

while [[ $# -gt 0 ]]; do
    case "$1" in
        --download-only)   DOWNLOAD_ONLY=true ;;
        --package)         shift; FILTER_PKG="$1" ;;
        --package=*)       FILTER_PKG="${1#*=}" ;;
        --version)         shift; VERSION="$1" ;;
        --version=*)       VERSION="${1#*=}" ;;
        -h|--help)
            sed -n '2,8p' "$0" | sed 's/^# //; s/^#//'
            exit 0
            ;;
        *) echo "Unknown arg: $1" >&2; exit 1 ;;
    esac
    shift
done

if command -v podman &>/dev/null; then
    RUNTIME=podman
elif command -v docker &>/dev/null; then
    RUNTIME=docker
else
    echo "ERROR: podman or docker required" >&2
    exit 1
fi

# Packages exempt from the post-install man-page assertion.
# xa65: upstream Fachat/Kaiser pages have WARNING-level mandoc issues
# (unparseable TH date, `.br` before text lines). xa65 is slated for
# retirement in favour of Ubuntu 26.04 universe's xa65 (TODO.md, Phase 1) —
# drop this entry once xa65 leaves foundry-apt.
MANPAGE_CHECK_SKIP=(xa65)

IMAGE="ubuntu:${VERSION}"

# Discover packages from control files — always in sync with what's published.
# Glob both layouts: DEBIAN/control (legacy xa65) AND debian/control (canonical
# metapackages + /package-skill-built upstreams like f9dasm).
mapfile -t ALL_PACKAGES < <(
    {
        grep -h "^Package:" packages/*/DEBIAN/control 2>/dev/null || true
        grep -h "^Package:" packages/*/debian/control  2>/dev/null || true
    } | awk '{print $2}' | sort -u
)

if [[ -n "$FILTER_PKG" ]]; then
    PACKAGES=("$FILTER_PKG")
else
    PACKAGES=("${ALL_PACKAGES[@]}")
fi

INSTALL_FLAG=""
$DOWNLOAD_ONLY && INSTALL_FLAG="--download-only"

pass=0
fail=0

for pkg in "${PACKAGES[@]}"; do
    echo
    echo "=== Testing: apt-get install ${pkg} ==="

    # Locate the package's in-tree control file (canonical or legacy layout).
    control_file=""
    for cand in "packages/${pkg}/debian/control" "packages/${pkg}/DEBIAN/control"; do
        [[ -f "$cand" ]] && { control_file="$cand"; break; }
    done
    if [[ -z "$control_file" ]]; then
        echo "[FAIL] ${pkg} (no control file found in either layout)"
        fail=$((fail + 1))
        continue
    fi

    # Architecture drives whether we run the post-install binary+manpage assertion.
    # `Architecture: any` packages own /usr/bin/ binaries; `Architecture: all`
    # packages are pure metapackages and own no files of their own.
    arch=$(awk -F': ' '/^Architecture:/ {print $2; exit}' "$control_file")

    # If this package depends on `task`, set up the Cloudsmith task repo first.
    EXTRA_REPOS=""
    if grep -qE '(^Depends:|^\s)[^#]*\btask\b' "$control_file"; then
        EXTRA_REPOS="curl -1sLf 'https://dl.cloudsmith.io/public/task/task/setup.deb.sh' | bash"
    fi

    # Post-install assertion body: every /usr/bin/<name> file the package owns
    # must have a /usr/share/man/man1/<name>.1.gz that mandoc lints clean at
    # warning level or higher.
    #
    # Why mandoc over `man -l`: groff (man's default renderer) silently accepts
    # unknown macros, empty .SH lines, and most structural breakage, returning 0.
    # `mandoc -Tlint -W warning -man` is strict on real bugs and quiet about
    # cosmetic STYLE issues (trailing whitespace, line >80 cols, unparseable
    # dates) which are common in upstream-vendored pages. Clean pages emit zero
    # output; any warning/error/unsupp prints a line. We treat any non-empty
    # output as failure (mandoc itself exits 0, so we check the output).
    #
    # Applies to every non-metapackage not on the skip-list; metapackages
    # (Architecture: all) and skip-list entries get an empty check body.
    MANPAGE_CHECK=""
    skip_manpage=false
    for s in "${MANPAGE_CHECK_SKIP[@]}"; do
        [[ "$s" == "$pkg" ]] && { skip_manpage=true; break; }
    done
    if [[ "$arch" != "all" ]] && ! $DOWNLOAD_ONLY && ! $skip_manpage; then
        MANPAGE_CHECK="
        echo '--- man-page assertion ---'
        miss=0
        while read -r bin; do
            [[ -z \$bin ]] && continue
            name=\$(basename \"\$bin\")
            page=/usr/share/man/man1/\${name}.1.gz
            if [[ ! -f \$page ]]; then
                echo \"MISS man page: \$page\" >&2
                miss=1
                continue
            fi
            lint=\$(zcat \"\$page\" | mandoc -Tlint -W warning -man 2>&1 || true)
            if [[ -n \$lint ]]; then
                echo \"BAD mandoc lint: \$page\" >&2
                echo \"\$lint\" | sed 's/^/    /' >&2
                miss=1
            else
                echo \"  ok \$page\"
            fi
        done < <(dpkg -L ${pkg} | grep -E '^/usr/bin/[^/]+\$' || true)
        (( miss == 0 )) || exit 1"
    fi

    if $RUNTIME run --rm "$IMAGE" bash -c "
        set -euo pipefail
        export DEBIAN_FRONTEND=noninteractive
        # Drop ubuntu:26.04's minbase path-exclude rules so man pages and
        # doc files actually land on disk — otherwise the post-install
        # assertion below would always trip on missing /usr/share/man/* files
        # that the .deb DID install but dpkg silently dropped.
        rm -f /etc/dpkg/dpkg.cfg.d/excludes
        apt-get update -q
        apt-get install -y --no-install-recommends curl gpg ca-certificates man-db mandoc
        mkdir -p /etc/apt/keyrings
        curl -fsSL https://apt.foundrylinux.org/key.gpg \
            | gpg --dearmor -o /etc/apt/keyrings/foundry.gpg
        echo 'deb [signed-by=/etc/apt/keyrings/foundry.gpg] https://apt.foundrylinux.org resolute main' \
            > /etc/apt/sources.list.d/foundry.list
        ${EXTRA_REPOS}
        apt-get update -q
        apt-get install -y ${INSTALL_FLAG} ${pkg}
        ${MANPAGE_CHECK}
    "; then
        echo "[PASS] ${pkg}"
        pass=$((pass + 1))
    else
        echo "[FAIL] ${pkg}"
        fail=$((fail + 1))
    fi
done

echo
echo "Results: ${pass} passed, ${fail} failed"
[[ $fail -eq 0 ]]
