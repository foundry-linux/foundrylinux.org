#!/usr/bin/env bash
# lib-source-build.sh — sourced by foundry-apt/packages/*/build.sh to ALSO emit a
# Debian SOURCE package (.dsc + tarballs) into dist/, so the apt repo publishes a
# Sources index (apt-get source + Repology's DebianSourcesParser). build-all.sh's
# canonical (native) path has its own inline equivalent.
#
# Sourced, not executed. Best-effort by contract: callers invoke it as
# `emit_source_package ... || true` so a source-build hiccup never blocks the
# binary build. Call it while the staged tree is CLEAN — after debian/ is
# overlaid, before the binary build dirties the tree.
set -euo pipefail

# emit_source_package SRC_DIR WORKDIR REPO_ROOT
#   SRC_DIR   staged source tree (<name>-<ver>/, upstream + debian/)
#   WORKDIR   parent of SRC_DIR (dpkg-source writes artifacts here)
#   REPO_ROOT repo root; artifacts move to $REPO_ROOT/dist/
emit_source_package() {
    local src_dir="$1" workdir="$2" repo_root="$3"
    local srcname fullver upver fmt base
    srcname=$(dpkg-parsechangelog -l "$src_dir/debian/changelog" -SSource 2>/dev/null) || return 0
    fullver=$(dpkg-parsechangelog -l "$src_dir/debian/changelog" -SVersion 2>/dev/null) || return 0
    fullver=${fullver#*:}            # strip epoch if present
    upver=${fullver%-*}              # strip Debian revision → upstream version (native: unchanged)
    fmt=$(tr -d '[:space:]' < "$src_dir/debian/source/format" 2>/dev/null || echo "")
    base=$(basename "$src_dir")

    # 3.0 (quilt) needs <srcname>_<upver>.orig.tar.* beside the source dir. Reuse the
    # pristine tarball build.sh staged if it's named to match; else synthesise one from
    # the staged upstream tree (everything but debian/). Native needs no orig.
    if [[ "$fmt" == *quilt* ]] && ! ls "$workdir/${srcname}_${upver}.orig.tar."* >/dev/null 2>&1; then
        tar -C "$workdir" --exclude="${base}/debian" \
            -czf "$workdir/${srcname}_${upver}.orig.tar.gz" "$base" 2>/dev/null || {
            echo "WARN ${srcname}: could not stage orig tarball — source package skipped" >&2
            return 0
        }
    fi

    if ! ( cd "$src_dir" && dpkg-buildpackage -us -uc -S -d ) >/dev/null 2>&1; then
        echo "WARN ${srcname}: source package build failed (binary .deb still produced)" >&2
        return 0
    fi

    # Move exactly the artifacts for this version (don't sweep up an unused, differently
    # named orig tarball build.sh may have left, e.g. vgmstream's r-prefixed one).
    local s moved=0
    for s in "$workdir/${srcname}_${fullver}.dsc" \
             "$workdir/${srcname}_${fullver}.debian.tar."* \
             "$workdir/${srcname}_${fullver}.tar."* \
             "$workdir/${srcname}_${upver}.orig.tar."*; do
        [[ -f "$s" ]] || continue
        mv -f "$s" "$repo_root/dist/" && echo "OK   dist/$(basename "$s")" && moved=1
    done
    [[ "$moved" == 1 ]] || echo "WARN ${srcname}: no source artifacts produced" >&2
    return 0
}
