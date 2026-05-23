#!/usr/bin/env bash
# Create a .torrent file and magnet URI for a built ISO.
#
# Usage:
#   EDITION=anvil bash scripts/create-torrents.sh
#
# Requires: mktorrent (apt install mktorrent)
#
# Output:
#   dist/foundry-{EDITION}-1.0-amd64.iso.torrent
#   dist/foundry-{EDITION}-1.0-amd64.iso.infohash   (hex SHA1 of info dict)
#   dist/foundry-{EDITION}-1.0-amd64.iso.magnet      (magnet URI, one line)

set -euo pipefail

if [[ "${1:-}" == "-h" || "${1:-}" == "--help" ]]; then
  echo "Usage: EDITION=anvil|atelier bash $0"
  echo "Creates .torrent + magnet URI for the built ISO."
  exit 0
fi

EDITION="${EDITION:?EDITION env var required: anvil or atelier}"
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
DIST_DIR="$(cd "$SCRIPT_DIR/../dist" && pwd)"
ISO_VERSION="$(cat "$SCRIPT_DIR/../VERSION")"

ISO="$DIST_DIR/foundry-${EDITION}-${ISO_VERSION}-amd64.iso"
TORRENT="$ISO.torrent"
INFOHASH_FILE="$ISO.infohash"
MAGNET_FILE="$ISO.magnet"

if [[ ! -f "$ISO" ]]; then
  echo "ERROR: $ISO not found — run build-iso.sh first" >&2
  exit 1
fi

if ! command -v mktorrent &>/dev/null; then
  echo "ERROR: mktorrent not found — apt install mktorrent" >&2
  exit 1
fi

TRACKERS=(
  "http://tracker.openbittorrent.com:80/announce"
  "http://tracker.opentracker.info:80/announce"
)
WEBSEED="https://iso.foundrylinux.org/foundry-${EDITION}-latest-amd64.iso"
DISPLAY_NAME="foundry-${EDITION}-${ISO_VERSION}-amd64"

echo "=== Creating torrent: $TORRENT ==="
TRACKER_ARGS=()
for t in "${TRACKERS[@]}"; do
  TRACKER_ARGS+=(-a "$t")
done

rm -f "$TORRENT"
# -l 22 = 4 MiB piece size (good for files 4-16 GB)
mktorrent -v -l 22 "${TRACKER_ARGS[@]}" -w "$WEBSEED" -n "$DISPLAY_NAME" -o "$TORRENT" "$ISO"

echo "=== Extracting infohash ==="
python3 - "$TORRENT" <<'PYEOF'
import hashlib, sys

def end_of(data, pos):
    """Return index of first byte past the bencode value starting at pos."""
    b = data[pos]
    if b == ord('i'):
        return data.index(ord('e'), pos + 1) + 1
    elif b in (ord('d'), ord('l')):
        pos += 1
        while data[pos] != ord('e'):
            pos = end_of(data, pos)
        return pos + 1
    else:
        colon = data.index(ord(':'), pos)
        length = int(data[pos:colon])
        return colon + 1 + length

with open(sys.argv[1], 'rb') as f:
    data = f.read()

# Find 4:info key in the outer dict, then hash the value
key = b'4:info'
idx = data.index(key) + len(key)
end = end_of(data, idx)
infohash = hashlib.sha1(data[idx:end]).hexdigest()
print(infohash)
PYEOF

INFOHASH=$(python3 - "$TORRENT" <<'PYEOF'
import hashlib, sys

def end_of(data, pos):
    b = data[pos]
    if b == ord('i'):
        return data.index(ord('e'), pos + 1) + 1
    elif b in (ord('d'), ord('l')):
        pos += 1
        while data[pos] != ord('e'):
            pos = end_of(data, pos)
        return pos + 1
    else:
        colon = data.index(ord(':'), pos)
        length = int(data[pos:colon])
        return colon + 1 + length

with open(sys.argv[1], 'rb') as f:
    data = f.read()

key = b'4:info'
idx = data.index(key) + len(key)
end = end_of(data, idx)
print(hashlib.sha1(data[idx:end]).hexdigest())
PYEOF
)

echo "$INFOHASH" > "$INFOHASH_FILE"
echo "  infohash: $INFOHASH"

# Build magnet URI
MAGNET="magnet:?xt=urn:btih:${INFOHASH}&dn=${DISPLAY_NAME}&ws=$(python3 -c "import urllib.parse,sys; print(urllib.parse.quote(sys.argv[1],safe=''))" "$WEBSEED")"
for t in "${TRACKERS[@]}"; do
  ENCODED=$(python3 -c "import urllib.parse,sys; print(urllib.parse.quote(sys.argv[1],safe=''))" "$t")
  MAGNET="${MAGNET}&tr=${ENCODED}"
done

echo "$MAGNET" > "$MAGNET_FILE"

echo "=== Torrent ready ==="
echo "  $TORRENT"
echo "  infohash: $INFOHASH"
echo "  magnet:   ${MAGNET:0:80}..."
