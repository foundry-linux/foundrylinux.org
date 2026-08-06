#!/usr/bin/env bash
# Vendored-upstream Debian package build wrapper for RPCS3 (PlayStation 3 emulator).
#
# WHY THIS DIFFERS FROM EVERY OTHER packages/*/build.sh
#
# Every other vendored package here fetches one upstream release tarball. RPCS3
# publishes NO release assets at all (every GitHub release carries zero files;
# the only prebuilt is an AppImage in the separate RPCS3/rpcs3-binaries-linux
# repo), and its plain tag archive is NOT buildable because it omits the 28 git
# submodules the project vendors.
#
# So we synthesise the orig tarball ourselves from a recursive clone at ONE
# pinned superproject commit. That is sufficient: a superproject commit records
# the exact commit of every submodule via its gitlinks, so pinning UPSTREAM_COMMIT
# pins all 32 (recursive) submodule checkouts too. There is no need for the
# per-submodule pin table the AUR recipe carries. Flathub pins the same way.
#
# The synthesis is byte-deterministic (sorted entries, fixed mtime from the
# changelog date, numeric root owner, no VCS metadata, xz which stores no
# timestamp), so ORIG_SHA256 below is a meaningful pin: a re-synthesis that does
# not reproduce it is a hard error, not a warning.
#
# DFSG REPACK — what the synthesised orig deliberately omits, and why
#
#   3rdparty/llvm/llvm      llvm-project. Not fetched at all. BUILD_LLVM defaults
#                           OFF and we build against Ubuntu 26.04's LLVM 21, so
#                           this submodule is unused; cloning it would add
#                           gigabytes for nothing.
#   3rdparty/ffmpeg         RPCS3/ffmpeg-core. Contains prebuilt Windows import
#                           libraries, and its CMakeLists DOWNLOADS a prebuilt
#                           ffmpeg zip from a GitHub release at configure time —
#                           an unpinned network fetch of a binary blob mid-build,
#                           which is not acceptable here. We pass
#                           USE_SYSTEM_FFMPEG=ON, under which 3rdparty/CMakeLists
#                           never add_subdirectory()s it.
#   3rdparty/opencv/opencv  Megamouse/opencv_minimal: a prebuilt Windows
#                           opencv_world4130.dll/.lib. The 3rdparty/opencv
#                           wrapper never references it on Linux; USE_SYSTEM_OPENCV
#                           defaults ON and we use Ubuntu's OpenCV 4.10.
#   assorted sample/test    A Visual Studio sample .exe (VulkanMemoryAllocator),
#   binaries                stb's oversample.exe, and SoundTouch's C#-example
#                           DLLs plus a Lazarus .so. None are referenced by the
#                           build; removing them keeps the published source
#                           package free of prebuilt binaries.
#
# Hence the +dfsg upstream version. debian/copyright records the same list in a
# DEP-5 Files-Excluded field.
#
# To bump the upstream version:
#   1. UPSTREAM_VERSION + UPSTREAM_COMMIT below (commit = `gh api
#      repos/RPCS3/rpcs3/commits/v<VER> --jq .sha`)
#   2. re-check .gitmodules for newly added submodules carrying prebuilt binaries
#   3. run this script once with FOUNDRY_PRINT_ORIG_SHA=1 to get the new
#      ORIG_SHA256, paste it in, and run again to confirm it reproduces
#   4. dch -v <NEW>+dfsg-1foundry1 -D resolute

set -euo pipefail

for arg in "$@"; do
    case "$arg" in
        -h|--help)
            cat <<EOF
Build RPCS3 as a Debian .deb via dpkg-buildpackage.

Usage: build.sh [-h|--help]

Environment overrides (rarely needed):
  RPCS3_VERSION           upstream version (default: 0.0.42)
  RPCS3_COMMIT            upstream superproject commit pinning all submodules
  RPCS3_ORIG_SHA256       sha256 of the synthesised orig tarball
  FOUNDRY_PRINT_ORIG_SHA  set to 1 to print the synthesised sha256 and exit 0
                          without building (used when re-pinning)
  RPCS3_ORIG_CACHE        directory to cache the synthesised orig tarball in.
                          A cached copy is only reused when its sha256 matches
                          ORIG_SHA256, so reuse is indistinguishable from a
                          fresh synthesis. Unset by default; CI always
                          synthesises from scratch.

Note: RPCS3 is a very large C++ codebase. Expect the clone to take several
minutes and tens of GB of scratch space, and the compile to take on the order
of an hour on a many-core machine.

Output: \$REPO_ROOT/dist/rpcs3_<DEB_VERSION>_amd64.deb
EOF
            exit 0
            ;;
    esac
done

UPSTREAM_VERSION="${RPCS3_VERSION:-0.0.42}"
UPSTREAM_COMMIT="${RPCS3_COMMIT:-cd814f8c926400cf62dab7924f723e4eb7719e50}"
ORIG_SHA256="${RPCS3_ORIG_SHA256:-195780af1a22d79258cde3b8664570eb70f7bf8385cc43b3c907aefb31f97d36}"
DFSG_VERSION="${UPSTREAM_VERSION}+dfsg"

cd "$(dirname "$0")"
PKG_DIR="$(pwd)"
NAME="rpcs3"
REPO_ROOT="$(cd ../.. && pwd)"
mkdir -p "$REPO_ROOT/dist"

if ! curl -fsI -o /dev/null https://github.com/; then
    echo "ERROR: cannot reach github.com — skipping $NAME build" >&2
    exit 1
fi

WORKDIR=$(mktemp -d -t "${NAME}-build-XXXXXX")
# shellcheck disable=SC2064
trap "rm -rf '$WORKDIR'" EXIT

SRC_DIR="$WORKDIR/${NAME}-${DFSG_VERSION}"
ORIG_TARBALL="$WORKDIR/${NAME}_${DFSG_VERSION}.orig.tar.xz"

echo "=== Installing Build-Depends ==="
_apt() { if [[ $EUID -eq 0 ]]; then apt-get "$@"; else sudo apt-get "$@"; fi; }
if [[ "${FOUNDRY_PRINT_ORIG_SHA:-0}" == "1" ]]; then
    # Re-pinning only synthesises the orig tarball; it never compiles, so skip
    # the ~450-package build-dependency set and install just what packing needs.
    command -v apt-get >/dev/null && \
        _apt install -y --no-install-recommends git ca-certificates xz-utils dpkg-dev
elif command -v apt-get >/dev/null; then
    # debian/control is the single source of truth for compile dependencies.
    # Keeping a second hand-written list here caused the well-known
    # libxkbcommon-dev/libxkbcommon-x11-dev split to be missed until step 1628
    # of a clean build. Bootstrap only the source-fetch/packing tools and the
    # helper that materialises Build-Depends, then install exactly that set.
    _apt install -y --no-install-recommends \
        git ca-certificates xz-utils dpkg-dev devscripts equivs
    root_args=()
    [[ $EUID -ne 0 ]] && root_args=(--root-cmd sudo)
    deps_dir="$WORKDIR/build-deps"
    mkdir -p "$deps_dir"
    # mk-build-deps expects the generated helper .deb in its current directory.
    # Do not inherit a caller-wide TMPDIR, which makes equivs place the .deb
    # elsewhere and leaves mk-build-deps trying to unpack a nonexistent path.
    (
        cd "$deps_dir"
        unset TMPDIR
        mk-build-deps --install --remove \
            --tool 'apt-get -y --no-install-recommends' \
            "${root_args[@]}" \
            "$PKG_DIR/debian/control"
    )
fi

# Optional local cache. Reuse is gated on the tarball matching ORIG_SHA256
# exactly, so a cached orig is indistinguishable from a freshly synthesised one;
# a stale or tampered file simply fails the check and is re-synthesised. Unset in
# CI, where every build synthesises from scratch.
CACHE_HIT=0
if [[ -n "${RPCS3_ORIG_CACHE:-}" && -f "$RPCS3_ORIG_CACHE/$(basename "$ORIG_TARBALL")" ]]; then
    echo "=== Checking cached orig tarball ==="
    if echo "$ORIG_SHA256  $RPCS3_ORIG_CACHE/$(basename "$ORIG_TARBALL")" | sha256sum -c - >/dev/null 2>&1; then
        cp "$RPCS3_ORIG_CACHE/$(basename "$ORIG_TARBALL")" "$ORIG_TARBALL"
        echo "OK   reusing cached orig (sha256 matches the pin)"
        tar -C "$WORKDIR" -xf "$ORIG_TARBALL"
        CACHE_HIT=1
    else
        echo "WARN cached orig does not match the pin — re-synthesising" >&2
    fi
fi

if [[ "$CACHE_HIT" == "0" ]]; then
echo "=== Synthesising orig tarball from pinned commit $UPSTREAM_COMMIT ==="
CLONE="$WORKDIR/clone"
# Blobless partial clone: we need one checkout, not the history of 32 repos.
git clone --quiet --filter=blob:none --no-checkout \
    https://github.com/RPCS3/rpcs3.git "$CLONE"
git -C "$CLONE" checkout --quiet "$UPSTREAM_COMMIT"

# Skip the llvm submodule entirely — BUILD_LLVM=OFF, we link system LLVM.
git -C "$CLONE" config submodule."3rdparty/llvm/llvm".update none

git -C "$CLONE" submodule update --init --recursive \
    --filter=blob:none --jobs 8 --quiet

echo "=== Submodule inventory (pinned transitively by the superproject commit) ==="
git -C "$CLONE" submodule status --recursive | awk '{printf "  %s %s\n", $1, $2}'

echo "=== Applying DFSG exclusions (see header) ==="
rm -rf "$CLONE/3rdparty/ffmpeg" \
       "$CLONE/3rdparty/opencv/opencv" \
       "$CLONE/3rdparty/GPUOpen/VulkanMemoryAllocator/bin" \
       "$CLONE/3rdparty/stblib/stb/tests/oversample" \
       "$CLONE/3rdparty/SoundTouch/soundtouch/source/csharp-example" \
       "$CLONE/3rdparty/SoundTouch/soundtouch/source/SoundTouchDLL/LazarusTest"

# No VCS metadata in the orig: it must not vary with clone mechanics, and
# dpkg-source rejects .git directories in a 3.0 (quilt) tree anyway.
find "$CLONE" -name .git -prune -exec rm -rf {} + 2>/dev/null || true
find "$CLONE" -name .gitmodules -delete

remaining=$(find "$CLONE" \( -name '*.lib' -o -name '*.dll' -o -name '*.exe' \
    -o -name '*.so' -o -name '*.a' -o -name '*.dylib' \) | wc -l)
if [[ "$remaining" -ne 0 ]]; then
    echo "ERROR: $remaining prebuilt binaries remain in the source tree:" >&2
    find "$CLONE" \( -name '*.lib' -o -name '*.dll' -o -name '*.exe' \
        -o -name '*.so' -o -name '*.a' -o -name '*.dylib' \) >&2
    echo "Upstream added a new vendored binary; extend the exclusion list." >&2
    exit 1
fi

mv "$CLONE" "$SRC_DIR"

echo "=== Packing deterministically ==="
# Fixed mtime from the changelog date, sorted entries, numeric root ownership,
# and xz (which records no timestamp) make this reproducible byte for byte.
SOURCE_DATE_EPOCH=$(date -u -d "$(dpkg-parsechangelog -l "$PKG_DIR/debian/changelog" -SDate)" +%s)
export SOURCE_DATE_EPOCH
tar --create \
    --sort=name \
    --format=gnu \
    --mtime="@$SOURCE_DATE_EPOCH" \
    --owner=root --group=root --numeric-owner \
    -C "$WORKDIR" "$(basename "$SRC_DIR")" \
  | xz -6 -T0 > "$ORIG_TARBALL"

if [[ -n "${RPCS3_ORIG_CACHE:-}" && -d "$RPCS3_ORIG_CACHE" && -w "$RPCS3_ORIG_CACHE" ]]; then
    cp "$ORIG_TARBALL" "$RPCS3_ORIG_CACHE/" && echo "OK   cached orig for reuse"
fi
fi   # end: synthesise-from-scratch branch

ACTUAL_SHA=$(sha256sum "$ORIG_TARBALL" | awk '{print $1}')
if [[ "${FOUNDRY_PRINT_ORIG_SHA:-0}" == "1" ]]; then
    echo "ORIG_SHA256=$ACTUAL_SHA"
    exit 0
fi
if [[ "$ORIG_SHA256" == "PLACEHOLDER" ]]; then
    echo "ERROR: ORIG_SHA256 is unset. Re-run with FOUNDRY_PRINT_ORIG_SHA=1" >&2
    exit 1
fi
if [[ "$ACTUAL_SHA" != "$ORIG_SHA256" ]]; then
    echo "ERROR: synthesised orig tarball does not match the pin." >&2
    echo "  expected $ORIG_SHA256" >&2
    echo "  actual   $ACTUAL_SHA" >&2
    exit 1
fi
echo "OK   orig tarball reproduces the pin ($ACTUAL_SHA)"

echo "=== Copying debian/ tree into source ==="
cp -a "$PKG_DIR/debian" "$SRC_DIR/"

echo "=== dpkg-buildpackage -us -uc -b ==="
# Emit the source package (.dsc + tarballs) -> dist/ for the apt Sources index
# (best-effort; never blocks the binary build below). emit_source_package reuses
# the orig tarball we just staged, because it is named exactly as it expects.
# See scripts/lib-source-build.sh.
if [[ -f "$REPO_ROOT/scripts/lib-source-build.sh" ]]; then
    # shellcheck source=/dev/null
    source "$REPO_ROOT/scripts/lib-source-build.sh"
    emit_source_package "$SRC_DIR" "$WORKDIR" "$REPO_ROOT" || true
fi
( cd "$SRC_DIR" && dpkg-buildpackage -us -uc -b )

DEB_VERSION=$(sed -n '1s/.*(\(.*\)).*/\1/p' "$PKG_DIR/debian/changelog")
# Architecture is fixed: RPCS3 refuses to compile on non-64-bit and upstream
# supports x86_64 Linux (Architecture: amd64).
DEB="$WORKDIR/${NAME}_${DEB_VERSION}_amd64.deb"
[[ -f "$DEB" ]] || { echo "ERROR: expected .deb not found: $DEB" >&2; ls -la "$WORKDIR"; exit 1; }

mv "$DEB" "$REPO_ROOT/dist/"
OUT="$REPO_ROOT/dist/$(basename "$DEB")"
echo "OK   $OUT  ($(stat -c%s "$OUT") bytes)"
