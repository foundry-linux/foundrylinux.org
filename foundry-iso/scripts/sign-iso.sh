#!/usr/bin/env bash
# GPG-sign and SHA-256 a built ISO.
#
# Usage (local):
#   EDITION=anvil bash scripts/sign-iso.sh
#
# Usage (CI): called by publish.yml with GPG_PRIVATE_KEY + GPG_PASSPHRASE env vars.

set -euo pipefail

EDITION="${EDITION:?EDITION env var required: anvil or atelier}"

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
DIST_DIR="$(cd "$SCRIPT_DIR/../dist" && pwd)"
ISO_VERSION="$(cat "$SCRIPT_DIR/../VERSION")"
ISO="$DIST_DIR/foundry-${EDITION}-${ISO_VERSION}-amd64.iso"

if [[ ! -f "$ISO" ]]; then
  echo "ERROR: $ISO not found — run build-iso.sh first" >&2
  exit 1
fi

# Import CI signing key if provided via env.
if [[ -n "${GPG_PRIVATE_KEY:-}" ]]; then
  echo "$GPG_PRIVATE_KEY" | gpg --batch --import
fi

PASSPHRASE_ARGS=()
if [[ -n "${GPG_PASSPHRASE:-}" ]]; then
  PASSPHRASE_ARGS=(--passphrase-fd 0 --pinentry-mode loopback)
fi

echo "=== SHA-256 ==="
sha256sum "$ISO" > "${ISO}.sha256"
cat "${ISO}.sha256"

echo "=== GPG sign ==="
rm -f "${ISO}.asc"
if [[ ${#PASSPHRASE_ARGS[@]} -gt 0 ]]; then
  echo "$GPG_PASSPHRASE" | gpg --batch "${PASSPHRASE_ARGS[@]}" \
      --armor --detach-sign --output "${ISO}.asc" "$ISO"
else
  gpg --batch --no-tty --pinentry-mode loopback \
      --armor --detach-sign --output "${ISO}.asc" "$ISO" < /dev/null
fi

echo "=== Creating torrent ==="
TORRENT_URL=""
INFOHASH=""
MAGNET_LINK=""
if command -v mktorrent &>/dev/null; then
  EDITION="$EDITION" bash "$SCRIPT_DIR/create-torrents.sh"
  INFOHASH=$(cat "${ISO}.infohash" 2>/dev/null || true)
  MAGNET_LINK=$(cat "${ISO}.magnet" 2>/dev/null || true)
  TORRENT_URL="https://iso.foundrylinux.org/foundry-${EDITION}-latest-amd64.iso.torrent"
else
  echo "  mktorrent not found — skipping torrent creation"
fi

echo "=== Writing manifest ==="
MANIFEST="$DIST_DIR/manifest-${EDITION}.json"
SHA=$(awk '{print $1}' "${ISO}.sha256")
SIZE=$(stat -c %s "$ISO")
cat > "$MANIFEST" <<EOF
{
  "edition":      "${EDITION}",
  "version":      "${ISO_VERSION}",
  "filename":     "foundry-${EDITION}-${ISO_VERSION}-amd64.iso",
  "sha256":       "${SHA}",
  "size_bytes":   ${SIZE},
  "download":     "https://iso.foundrylinux.org/foundry-${EDITION}-latest-amd64.iso",
  "torrent_url":  "${TORRENT_URL}",
  "infohash":     "${INFOHASH}",
  "magnet_link":  "${MAGNET_LINK}"
}
EOF

echo "=== Signed: ${ISO}.asc  |  Checksum: ${ISO}.sha256  |  Manifest: ${MANIFEST} ==="
