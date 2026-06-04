#!/bin/bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

if [[ "${1:-}" == "-h" || "${1:-}" == "--help" ]]; then
    echo "Usage: DEVICE=/dev/sdX [EDITION=anvil] bash scripts/write-usb.sh"
    echo "       task iso-write DEVICE=/dev/sdX [EDITION=anvil]"
    echo ""
    echo "Writes the latest foundry-<EDITION> ISO to a USB block device."
    echo "EDITION defaults to 'anvil'. Requires sudo (or run as root)."
    exit 0
fi

DEVICE="${DEVICE:?DEVICE env var required — e.g.  DEVICE=/dev/sdb  task iso-write DEVICE=/dev/sdb}"
EDITION="${EDITION:-anvil}"

# Find latest ISO for the requested edition
ISO=$(ls -t "$REPO_ROOT/dist/foundry-${EDITION}-"*"-amd64.iso" 2>/dev/null | head -1 || true)
if [[ -z "$ISO" ]]; then
    echo "ERROR: no ISO found for edition '$EDITION' in $REPO_ROOT/dist/" >&2
    echo "       Run 'task iso-build EDITION=$EDITION' first." >&2
    exit 1
fi

ISO_BYTES=$(stat -c '%s' "$ISO")
ISO_GIB=$(python3 -c "print(f'{$ISO_BYTES/2**30:.2f}')")

# Validate block device
if [[ ! -b "$DEVICE" ]]; then
    echo "ERROR: $DEVICE is not a block device" >&2
    echo "       Run 'lsblk' to list available devices." >&2
    exit 1
fi

# Refuse to write to a mounted device or any of its partitions
if grep -qE "^${DEVICE}p?[0-9]* " /proc/mounts 2>/dev/null; then
    echo "ERROR: $DEVICE (or a partition on it) is mounted — unmount it first:" >&2
    grep -E "^${DEVICE}p?[0-9]* " /proc/mounts | awk '{print "       umount "$2}' >&2
    exit 1
fi

DEV_BYTES=$(blockdev --getsize64 "$DEVICE" 2>/dev/null || echo 0)
DEV_GIB=$(python3 -c "print(f'{$DEV_BYTES/2**30:.2f}')")

if [[ "$DEV_BYTES" -lt "$ISO_BYTES" ]]; then
    echo "ERROR: device $DEVICE ($DEV_GIB GiB) is smaller than the ISO ($ISO_GIB GiB)" >&2
    exit 1
fi

echo "ISO:     $(basename "$ISO")  ($ISO_GIB GiB)"
echo "Device:  $DEVICE  ($DEV_GIB GiB)"
echo ""
lsblk -o NAME,SIZE,MODEL,TRAN "$DEVICE" 2>/dev/null || true
echo ""
echo "WARNING: ALL DATA ON $DEVICE WILL BE PERMANENTLY ERASED."
read -rp "Type YES to continue: " CONFIRM
if [[ "$CONFIRM" != "YES" ]]; then
    echo "Aborted."
    exit 0
fi

echo ""
echo "Writing $ISO → $DEVICE …"
dd if="$ISO" of="$DEVICE" bs=4M status=progress conv=fsync
sync
echo ""
echo "Done. $DEVICE can be safely removed."
