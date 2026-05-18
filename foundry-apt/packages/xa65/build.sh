#!/usr/bin/env bash
# Build xa65 (6502/65816 cross-assembler) as a Debian .deb for amd64 from
# the upstream xa-2.4.1 source release.
#
# Upstream:  https://github.com/fachat/xa65 (André Fachat; mirrors releases
#            from Cameron Kaiser at http://www.floodgap.com/retrotech/xa/)
# License:   GPL-2.0-only (xa/COPYING)
#
# Pinned version + sha256 — bump together. Re-pin with:
#   curl -fsSL https://github.com/fachat/xa65/archive/refs/tags/xa-<VER>.tar.gz \
#     | sha256sum
#
# Builds: dist/xa65_<DEB_VERSION>_amd64.deb
# arm64 follow-up tracked in docs/plans/2026-05-18-package-xa65.md "Out of scope".

set -euo pipefail

# -h / --help short-circuit before any work — per ~/SRC/CLAUDE.md
for arg in "$@"; do
    case "$arg" in
        -h|--help)
            cat <<'EOF'
Build xa65 as a Debian .deb (amd64 only for now).

Usage: build.sh [-h|--help]

Environment overrides (rarely needed):
  XA65_VERSION       upstream tag without the "xa-" prefix (default: 2.4.1)
  XA65_SHA256        sha256 of the upstream tarball (must match XA65_VERSION)
  XA65_DEB_REVISION  packaging revision (default: 1foundry1)

Output: $REPO_ROOT/dist/xa65_<UPSTREAM>-<REVISION>_amd64.deb
EOF
            exit 0
            ;;
    esac
done

UPSTREAM_VERSION="${XA65_VERSION:-2.4.1}"
SHA256="${XA65_SHA256:-8fecdb9e64da84eb77d3572e73033bf3d1ca533c7291fb5d0d9735c5440eb67e}"
DEB_REVISION="${XA65_DEB_REVISION:-1foundry1}"
DEB_VERSION="${UPSTREAM_VERSION}-${DEB_REVISION}"

cd "$(dirname "$0")"
REPO_ROOT="$(cd ../.. && pwd)"
mkdir -p "$REPO_ROOT/dist"

build_one() {
    local arch="$1"
    local tag="xa65_${DEB_VERSION}_${arch}"
    local workdir
    workdir=$(mktemp -d)
    # shellcheck disable=SC2064  # expand $workdir now so the trap captures it
    trap "rm -rf '$workdir'" RETURN

    echo "=== Building $tag ==="

    local url="https://github.com/fachat/xa65/archive/refs/tags/xa-${UPSTREAM_VERSION}.tar.gz"
    echo "Fetching $url"
    curl -fsSL -o "$workdir/xa65.tar.gz" "$url"

    echo "Verifying sha256..."
    echo "$SHA256  $workdir/xa65.tar.gz" | sha256sum -c -

    tar -xzf "$workdir/xa65.tar.gz" -C "$workdir"
    local src="$workdir/xa65-xa-${UPSTREAM_VERSION}/xa"
    [[ -f "$src/Makefile" ]] || { echo "ERROR: $src/Makefile not found" >&2; exit 1; }

    echo "Compiling (cd $src && make)..."
    make -C "$src" >/dev/null

    local stage="$workdir/stage"
    mkdir -p "$stage/DEBIAN" "$stage/usr/share/doc/xa65"

    # xa's Makefile uses DESTDIR as a prefix (not staging dir), so DESTDIR=$stage/usr
    # lands binaries at $stage/usr/bin and man pages at $stage/usr/share/man/man1.
    make -C "$src" install DESTDIR="$stage/usr" >/dev/null

    install -m 0644 "$src/COPYING"    "$stage/usr/share/doc/xa65/copyright"
    install -m 0644 "$src/README.1st" "$stage/usr/share/doc/xa65/README.1st"
    install -m 0644 "$src/ChangeLog"  "$stage/usr/share/doc/xa65/changelog"
    gzip -n9 "$stage/usr/share/doc/xa65/changelog"

    # xa's Makefile installs man pages with +x; Debian wants 0644.
    find "$stage/usr/share/man" -type f -exec chmod 0644 {} +
    # Compress man pages per Debian policy (gzip -n9, deterministic).
    find "$stage/usr/share/man" -type f ! -name '*.gz' -exec gzip -n9 {} +

    local installed_size
    installed_size=$(du -sk --apparent-size "$stage/usr" | awk '{print $1}')

    cat > "$stage/DEBIAN/control" <<EOF
Package: xa65
Version: ${DEB_VERSION}
Architecture: ${arch}
Maintainer: Foundry Linux <packages@foundrylinux.org>
Section: devel
Priority: optional
Installed-Size: ${installed_size}
Depends: libc6
Homepage: https://www.floodgap.com/retrotech/xa/
Description: 6502/65816 cross-assembler (Fachat/Kaiser xa)
 xa is a small, fast, portable two-pass cross-assembler for the 6502 and
 65816 CPU families. It builds binaries for retro platforms (Atari, C64,
 Apple ][, NES, etc.) from a Unix host. Includes xa, reloc65, ldo65,
 file65, printcbm, and uncpk. Distributed under the GNU GPL v2.
 .
 Repackaged from upstream xa-${UPSTREAM_VERSION} at
 https://github.com/fachat/xa65 for inclusion in the Foundry Linux APT
 repo. xa65 is not in the Ubuntu archive.
EOF

    local out="$REPO_ROOT/dist/${tag}.deb"
    dpkg-deb --root-owner-group --build "$stage" "$out" >/dev/null
    echo "OK   $out  ($(stat -c%s "$out") bytes)"
}

# Fail loudly if upstream is unreachable.
if ! curl -fsI -o /dev/null https://github.com/; then
    echo "ERROR: cannot reach github.com — skipping xa65 package build" >&2
    exit 1
fi

# Native architecture only for the first iteration. arm64 will be added in a
# follow-up plan (cross-compile via gcc-aarch64-linux-gnu in CI).
HOST_ARCH=$(dpkg --print-architecture)
if [[ "$HOST_ARCH" != "amd64" ]]; then
    echo "ERROR: this build.sh currently produces amd64 only; host is $HOST_ARCH" >&2
    exit 1
fi
build_one amd64
