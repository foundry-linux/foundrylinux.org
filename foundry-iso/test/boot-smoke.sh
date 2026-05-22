#!/usr/bin/env bash
# Headless QEMU boot smoke test: boots the ISO, waits for Calamares + getty.
#
# Usage:
#   bash test/boot-smoke.sh dist/foundry-anvil-1.0-amd64.iso
#
# Requires: qemu-system-x86_64

set -euo pipefail

ISO="${1:?Usage: $0 <path-to-iso>}"
TIMEOUT=120   # seconds to wait for boot signals
PIDFILE=/tmp/foundry-boot-smoke-$$.pid
LOGFILE=/tmp/foundry-boot-smoke-$$.log

if [[ ! -f "$ISO" ]]; then
  echo "ERROR: ISO not found: $ISO" >&2
  exit 1
fi

if ! command -v qemu-system-x86_64 &>/dev/null; then
  echo "ERROR: qemu-system-x86_64 not found — install qemu-system-x86" >&2
  exit 1
fi

echo "=== Boot smoke: $ISO (timeout: ${TIMEOUT}s) ==="

qemu-system-x86_64 \
  -m 2048 \
  -cdrom "$ISO" \
  -boot d \
  -nographic \
  -serial mon:stdio \
  -no-reboot \
  -device virtio-net,netdev=n0 \
  -netdev user,id=n0 \
  > "$LOGFILE" 2>&1 &

QEMU_PID=$!
echo "$QEMU_PID" > "$PIDFILE"

cleanup() {
  kill "$QEMU_PID" 2>/dev/null || true
  rm -f "$PIDFILE" "$LOGFILE"
}
trap cleanup EXIT

echo "  QEMU PID: $QEMU_PID — watching log for signals..."

CALAMARES_SEEN=false
GETTY_SEEN=false
ELAPSED=0

while (( ELAPSED < TIMEOUT )); do
  sleep 2
  ELAPSED=$(( ELAPSED + 2 ))

  if grep -q "calamares\|Calamares" "$LOGFILE" 2>/dev/null; then
    CALAMARES_SEEN=true
  fi
  if grep -qE "login:|getty|tty1" "$LOGFILE" 2>/dev/null; then
    GETTY_SEEN=true
  fi

  if $CALAMARES_SEEN && $GETTY_SEEN; then
    echo "  [${ELAPSED}s] Calamares: ✓   getty: ✓"
    echo "=== PASS: ISO booted successfully ==="
    exit 0
  fi

  if ! kill -0 "$QEMU_PID" 2>/dev/null; then
    echo "ERROR: QEMU exited unexpectedly" >&2
    tail -20 "$LOGFILE" >&2
    exit 1
  fi

  printf "  [%3ds] Calamares: %s   getty: %s\r" \
    "$ELAPSED" \
    "$($CALAMARES_SEEN && echo '✓' || echo '…')" \
    "$($GETTY_SEEN && echo '✓' || echo '…')"
done

echo
echo "ERROR: timeout after ${TIMEOUT}s — signals not detected" >&2
echo "  Calamares: $CALAMARES_SEEN   getty: $GETTY_SEEN" >&2
tail -30 "$LOGFILE" >&2
exit 1
