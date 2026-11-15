#!/usr/bin/env bash
# Build packages under packages/ into dist/.
#
# Usage:
#   bash scripts/build-all.sh            # build all packages (skip already-current .debs)
#   bash scripts/build-all.sh foundry-welcome   # build one package by name
#   bash scripts/build-all.sh xemu rpcs3 # build several packages by name
#
# Two layouts supported:
#
#   1. Canonical Debian source-package layout (preferred):
#        packages/<name>/debian/{control,changelog,rules,source/format,copyright}
#      Built with dpkg-buildpackage from a tmp-staged copy of debian/.
#      Used by every metapackage (3.0 native) and by /package-skill-built
#      vendored upstreams (3.0 quilt).
#
#   2. Vendored upstream with a build.sh wrapper:
#        packages/<name>/{build.sh, debian/...}
#      build.sh is responsible for fetching the upstream tarball
#      (sha256-pinned), extracting it, overlaying debian/, running
#      dpkg-buildpackage, and moving the .deb into dist/. /package generates
#      this layout for vendored upstreams (f9dasm, future libvgm/vgmstream).

set -euo pipefail
cd "$(dirname "$0")/.."

REPO_ROOT="$(pwd)"
mkdir -p dist
exec 8>dist/.build.lock
flock -n 8 || { echo 'Another package build owns dist/' >&2; exit 1; }
source scripts/build-resources.sh

# Optional package filters. No arguments preserves the all-package release path.
PKG_FILTERS=("$@")

package_selected() {
    local candidate="$1" filter

    (( ${#PKG_FILTERS[@]} == 0 )) && return 0
    for filter in "${PKG_FILTERS[@]}"; do
        [[ "$candidate" == "$filter" ]] && return 0
    done
    return 1
}

build_canonical() {
    local pkgdir="$1" name="$2"
    local ver builddir deb

    if ! command -v dpkg-buildpackage >/dev/null; then
        echo "FAIL $name (dpkg-buildpackage not installed — apt install dpkg-dev debhelper)" >&2
        return 1
    fi

    ver=$(dpkg-parsechangelog -l "$pkgdir/debian/changelog" -SVersion)
    if [[ -z "$ver" ]]; then
        echo "FAIL $name (could not parse version from debian/changelog)" >&2
        return 1
    fi

    builddir=$(mktemp -d -t "${name}-build-XXXXXX")
    # shellcheck disable=SC2064  # expand $builddir now so trap captures the value
    trap "rm -rf '$builddir'" RETURN

    mkdir -p "${builddir}/${name}-${ver}"
    # Copy full source tree so dh_install can find data/ files in native packages
    cp -a "${pkgdir}/." "${builddir}/${name}-${ver}/"

    if ! ( cd "${builddir}/${name}-${ver}" && dpkg-buildpackage -us -uc -b -d --no-sign ) >/dev/null 2>&1; then
        echo "FAIL $name (dpkg-buildpackage exited non-zero)" >&2
        ( cd "${builddir}/${name}-${ver}" && dpkg-buildpackage -us -uc -b -d --no-sign 2>&1 | tail -10 ) >&2
        return 1
    fi

    # The .deb may be amd64-arch'd or all-arch'd depending on debian/control
    for deb in "${builddir}/${name}_${ver}_"*.deb; do
        [[ -f "$deb" ]] || continue
        mv "$deb" "${REPO_ROOT}/dist/"
        echo "OK   dist/$(basename "$deb")  ($(stat -c%s "${REPO_ROOT}/dist/$(basename "$deb")") bytes)"
    done

    # Source package → dist/ so the repo publishes a Sources index (apt-get source +
    # Repology DebianSourcesParser). Canonical-layout packages are all 3.0 (native),
    # so the source pass needs no orig tarball — it tars the whole tree.
    if ( cd "${builddir}/${name}-${ver}" && dpkg-buildpackage -us -uc -S -d ) >/dev/null 2>&1; then
        for src in "${builddir}/${name}_${ver}.dsc" "${builddir}/${name}_${ver}.tar."*; do
            [[ -f "$src" ]] && mv "$src" "${REPO_ROOT}/dist/" && echo "OK   dist/$(basename "$src")"
        done
    else
        echo "WARN $name: source package build failed (binary .deb still produced)" >&2
    fi
}

fail=0
ccache_disabled_by_caller=${CCACHE_DISABLE:-}
for pkgdir in packages/*/; do
    name=$(basename "$pkgdir")

    # Shared Electron is published through its separately signed repository.
    # Never let the general Foundry release pick these packages up implicitly.
    if [[ -f "$pkgdir/.shared-electron-only" && "${INCLUDE_SHARED_ELECTRON:-0}" != 1 ]]; then
        echo "SKIP $name (Shared Electron repository only)"
        continue
    fi

    # Optional one-or-more-package filter.
    if ! package_selected "$name"; then
        continue
    fi

    # Reuse only a validated per-recipe receipt. A same-version source edit,
    # changed dependency candidate, missing output or corrupt artifact rebuilds.
    if [[ "${FOUNDRY_FORCE_REBUILD:-0}" != 1 ]] &&
       python3 scripts/recipe-cache.py check --recipe "$name"; then
        continue
    fi
    # The compiler cache is deliberately disabled for RPCS3's C++ modules until
    # the installed ccache/module combination has been validated.
    export CCACHE_NAMESPACE="$name"
    if [[ "$name" == rpcs3 || -n "$ccache_disabled_by_caller" ]]; then
        export CCACHE_DISABLE=${ccache_disabled_by_caller:-1}
    else
        unset CCACHE_DISABLE
    fi
    export TMPDIR="${FOUNDRY_BUILD_TMP:-/tmp/foundry-build}/$name"
    mkdir -p "$TMPDIR"
    build_started_ns=$(date +%s%N)
    if [[ -x "$pkgdir/build.sh" ]]; then
        echo "=== Running $name/build.sh ==="
        if bash "$pkgdir/build.sh"; then
            python3 scripts/recipe-cache.py record --recipe "$name" --built-after-ns "$build_started_ns" ||
                echo "NOTE: $name built, but no reusable cache receipt was recorded"
        else
            echo "FAIL $name (build.sh exited non-zero)" >&2
            fail=1
        fi
        continue
    fi

    if [[ -f "$pkgdir/debian/control" && -f "$pkgdir/debian/changelog" ]]; then
        echo "=== Building $name (canonical debian/ source format) ==="
        if build_canonical "$pkgdir" "$name"; then
            python3 scripts/recipe-cache.py record --recipe "$name" --built-after-ns "$build_started_ns" ||
                echo "NOTE: $name built, but no reusable cache receipt was recorded"
        else
            fail=1
        fi
        continue
    fi

    echo "SKIP $name (no debian/control and no build.sh)"
done

if (( fail )); then
    echo "ERROR: one or more builds failed" >&2
    exit 1
fi

echo
echo "=== dist/ ==="
ls -lh dist/

# When running as root inside a Docker container, bind-mounted files land as
# root on the host. Re-own dist/*.deb to match the directory's host owner so
# the caller doesn't end up with root-owned artifacts.
if [[ $EUID -eq 0 ]]; then
    volume_owner=$(stat -c '%u:%g' .)
    if [[ "$volume_owner" != "0:0" ]]; then
        chown "$volume_owner" dist/*.deb 2>/dev/null || true
    fi
fi
