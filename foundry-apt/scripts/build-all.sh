#!/usr/bin/env bash
# Build packages under packages/ into dist/.
#
# Usage:
#   bash scripts/build-all.sh            # build all packages (skip already-current .debs)
#   bash scripts/build-all.sh foundry-welcome   # build one package by name
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

# Optional single-package filter (first positional arg).
PKG_FILTER="${1:-}"

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

    # Skip only if BOTH the .deb AND the source .dsc for this version are present
    # (we publish a Sources index too — a .deb without a .dsc must be rebuilt).
    local existing existing_dsc=""
    # shellcheck disable=SC2012  # dist/ filenames are controlled (name_version_arch.deb); ls is safe
    existing=$(ls "${REPO_ROOT}/dist/${name}_${ver}_"*.deb 2>/dev/null | head -1 || true)
    [[ -f "${REPO_ROOT}/dist/${name}_${ver}.dsc" ]] && existing_dsc=1
    if [[ -n "$existing" && -n "$existing_dsc" ]]; then
        echo "SKIP $name (dist/$(basename "$existing") + .dsc already current)"
        return 0
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
for pkgdir in packages/*/; do
    name=$(basename "$pkgdir")

    # Single-package filter.
    if [[ -n "$PKG_FILTER" && "$name" != "$PKG_FILTER" ]]; then
        continue
    fi

    if [[ -x "$pkgdir/build.sh" ]]; then
        # Skip if the current changelog version is already in dist/.
        if [[ -f "$pkgdir/debian/changelog" ]]; then
            _ver=$(dpkg-parsechangelog -l "$pkgdir/debian/changelog" -SVersion 2>/dev/null || true)
            if [[ -n "$_ver" ]]; then
                # shellcheck disable=SC2012  # controlled dist/ filenames; ls is safe
                _existing=$(ls "${REPO_ROOT}/dist/${name}_${_ver}_"*.deb 2>/dev/null | head -1 || true)
                if [[ -n "$_existing" ]]; then
                    echo "SKIP $name (dist/$(basename "$_existing") already current)"
                    continue
                fi
            fi
        fi
        echo "=== Running $name/build.sh ==="
        if ! bash "$pkgdir/build.sh"; then
            echo "FAIL $name (build.sh exited non-zero)" >&2
            fail=1
        fi
        continue
    fi

    if [[ -f "$pkgdir/debian/control" && -f "$pkgdir/debian/changelog" ]]; then
        echo "=== Building $name (canonical debian/ source format) ==="
        if ! build_canonical "$pkgdir" "$name"; then
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
