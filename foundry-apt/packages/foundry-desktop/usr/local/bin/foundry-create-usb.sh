#!/bin/bash
set -euo pipefail

# foundry-create-usb.sh — download (or use a local) Foundry Linux ISO and
# write it to a USB stick.
#
# Desktop use (no args):
#   foundry-create-usb.sh
#     Downloads the latest anvil ISO, verifies sha256, launches isoimagewriter.
#
# Terminal / dev use (task iso-write):
#   foundry-create-usb.sh --iso /path/to/foundry-anvil.iso --device /dev/sdX
#     Skips download, writes directly with dd.

EDITION="${EDITION:-anvil}"
ISO_PATH=""
DEVICE=""
BASE_URL="https://iso.foundrylinux.org"

usage() {
    cat <<EOF
Usage: foundry-create-usb.sh [OPTIONS]

  --iso PATH      Use a local ISO instead of downloading
  --device DEV    Write directly to a block device (e.g. /dev/sdb) with dd.
                  Without this flag the ISO is opened in isoimagewriter (GUI).
  --edition NAME  Edition to download if --iso is not given (default: anvil)
  -h, --help      Show this help and exit

Examples:
  foundry-create-usb.sh                              # download + GUI
  foundry-create-usb.sh --iso foundry-anvil.iso      # local ISO + GUI
  foundry-create-usb.sh --iso foundry-anvil.iso --device /dev/sdb  # dd write
EOF
}

# ---- argument parsing ----
while [[ $# -gt 0 ]]; do
    case "$1" in
        --iso)      ISO_PATH="$2"; shift 2 ;;
        --device)   DEVICE="$2";   shift 2 ;;
        --edition)  EDITION="$2";  shift 2 ;;
        -h|--help)  usage; exit 0 ;;
        *) echo "ERROR: unknown argument: $1" >&2; usage; exit 1 ;;
    esac
done

# ---- get the ISO ----
if [[ -z "$ISO_PATH" ]]; then
    # Download mode — used when run from the desktop without a local ISO.
    ISO_URL="${BASE_URL}/foundry-${EDITION}-latest-amd64.iso"
    SHA_URL="${BASE_URL}/foundry-${EDITION}-latest-amd64.iso.sha256"
    DEST="$HOME/Downloads/foundry-${EDITION}-latest-amd64.iso"
    mkdir -p "$(dirname "$DEST")"

    echo "Fetching sha256 …"
    EXPECTED_SHA=$(curl -fsSL "$SHA_URL" | awk '{print $1}')

    if [[ -f "$DEST" ]]; then
        ACTUAL_SHA=$(sha256sum "$DEST" | awk '{print $1}')
        if [[ "$ACTUAL_SHA" == "$EXPECTED_SHA" ]]; then
            echo "Cached ISO matches sha256 — skipping download."
        else
            echo "Cached ISO sha256 mismatch — re-downloading …"
            curl -fL --progress-bar -o "$DEST" "$ISO_URL"
        fi
    else
        echo "Downloading $ISO_URL …"
        curl -fL --progress-bar -o "$DEST" "$ISO_URL"
    fi

    ACTUAL_SHA=$(sha256sum "$DEST" | awk '{print $1}')
    if [[ "$ACTUAL_SHA" != "$EXPECTED_SHA" ]]; then
        echo "ERROR: sha256 mismatch after download — aborting." >&2
        echo "  expected: $EXPECTED_SHA" >&2
        echo "  actual:   $ACTUAL_SHA" >&2
        rm -f "$DEST"
        exit 1
    fi
    echo "sha256 OK."
    ISO_PATH="$DEST"
fi

[[ -f "$ISO_PATH" ]] || { echo "ERROR: ISO not found: $ISO_PATH" >&2; exit 1; }

ISO_BYTES=$(stat -c '%s' "$ISO_PATH")
ISO_GIB=$(python3 -c "print(f'{$ISO_BYTES/2**30:.2f}')")

# ---- write or launch GUI ----
if [[ -z "$DEVICE" ]]; then
    # GUI mode — open the ISO in isoimagewriter (preferred) or usb-creator-kde.
    if command -v isoimagewriter &>/dev/null; then
        exec isoimagewriter "$ISO_PATH"
    elif command -v usb-creator-kde &>/dev/null; then
        exec usb-creator-kde "$ISO_PATH"
    else
        echo "ERROR: neither isoimagewriter nor usb-creator-kde found." >&2
        echo "       Install one: sudo apt install isoimagewriter" >&2
        exit 1
    fi
fi

# dd mode — validate device, confirm, write.
[[ -b "$DEVICE" ]] || {
    echo "ERROR: $DEVICE is not a block device" >&2
    echo "       Run 'lsblk' to list available devices." >&2
    exit 1
}

# Refuse if any partition of the device is mounted.
if grep -qE "^${DEVICE}p?[0-9]* " /proc/mounts 2>/dev/null; then
    echo "ERROR: $DEVICE (or a partition on it) is mounted — unmount first:" >&2
    grep -E "^${DEVICE}p?[0-9]* " /proc/mounts | awk '{print "       umount "$2}' >&2
    exit 1
fi

DEV_BYTES=$(blockdev --getsize64 "$DEVICE" 2>/dev/null || echo 0)
DEV_GIB=$(python3 -c "print(f'{$DEV_BYTES/2**30:.2f}')")

if [[ "$DEV_BYTES" -lt "$ISO_BYTES" ]]; then
    echo "ERROR: $DEVICE ($DEV_GIB GiB) is smaller than the ISO ($ISO_GIB GiB)" >&2
    exit 1
fi

echo "ISO:     $(basename "$ISO_PATH")  ($ISO_GIB GiB)"
echo "Device:  $DEVICE  ($DEV_GIB GiB)"
echo ""
lsblk -o NAME,SIZE,MODEL,TRAN "$DEVICE" 2>/dev/null || true
echo ""
echo "WARNING: ALL DATA ON $DEVICE WILL BE PERMANENTLY ERASED."
read -rp "Type YES to continue: " CONFIRM
[[ "$CONFIRM" == "YES" ]] || { echo "Aborted."; exit 0; }

echo ""
echo "Writing $(basename "$ISO_PATH") → $DEVICE …"
dd if="$ISO_PATH" of="$DEVICE" bs=4M status=progress conv=fsync
sync
echo ""
echo "Done. $DEVICE can be safely removed."
