#!/usr/bin/env bash
# blender-asset-finder-install — install the Blender Asset Finder
# extension into the running user's Blender (4.2+) and enable it.
#
# Same shape as wf-blender-install(1) but for the asset-finder addon.

set -euo pipefail

PKGSHARE="/usr/share/blender-asset-finder"
# shellcheck disable=SC2012  # version-sorted pick of a controlled shipped filename; ls|sort -V is intentional
ZIP=$(ls "$PKGSHARE"/blender_asset_finder-*.zip 2>/dev/null | sort -V | tail -1 || true)

usage() {
    cat <<EOF
Usage: blender-asset-finder-install [--blender PATH] [--repo NAME] [--no-enable]
       blender-asset-finder-install -h | --help

Install the Blender Asset Finder extension into the current user's
Blender (4.2+) and enable it.

Options:
  --blender PATH   Path to the Blender executable (default: \`blender\`
                   from PATH).
  --repo NAME      Extension repository to install into (default:
                   \`user_default\`).
  --no-enable      Install the extension but do not enable it.
  -h, --help       Show this help and exit.

The extension zip is shipped at:
  $PKGSHARE/blender_asset_finder-<version>.zip

After installation, the add-on appears in Blender as "Asset Finder"
under Edit > Preferences > Add-ons.
EOF
}

BLENDER=blender
REPO=user_default
ENABLE_FLAG=--enable
while [[ $# -gt 0 ]]; do
    case "$1" in
        -h|--help) usage; exit 0 ;;
        --blender) BLENDER="$2"; shift 2 ;;
        --repo)    REPO="$2"; shift 2 ;;
        --no-enable) ENABLE_FLAG=; shift ;;
        *) echo "blender-asset-finder-install: unknown argument: $1" >&2; usage >&2; exit 2 ;;
    esac
done

if [[ -z "${ZIP:-}" ]]; then
    echo "blender-asset-finder-install: no extension zip found under $PKGSHARE" >&2
    exit 1
fi

if ! command -v "$BLENDER" >/dev/null 2>&1; then
    cat >&2 <<EOF
blender-asset-finder-install: blender not found on PATH.

This package recommends but does not strictly depend on Blender (you
may have installed it from snap, flatpak, or a tarball). Install
Blender 4.2+ first, then re-run this command, or pass
--blender /path/to/blender if it is in a non-standard location.
EOF
    exit 1
fi

echo "blender-asset-finder-install: installing $ZIP into Blender repo '$REPO'"
"$BLENDER" --background \
           --command extension install-file \
           --repo "$REPO" \
           ${ENABLE_FLAG} \
           "$ZIP"

echo "blender-asset-finder-install: done. Open Blender > Edit > Preferences"
echo "                              > Add-ons and search for 'Asset Finder'"
echo "                              to confirm."
