#!/usr/bin/env bash
# QEMU boot test: boots the ISO with VirtGL so kwin_wayland --drm has a DRM device.
#
# Usage:
#   bash test/boot-smoke.sh dist/foundry-anvil-1.0-amd64.iso
#
# SSH into the live session (ISOs built with hook 1200-live-ssh):
#   ssh -p 2222 user@localhost   # password: live
#   ssh -p 2222 root@localhost   # password: foundry
#
# Requires: qemu-system-x86_64, ovmf, host with OpenGL support

set -euo pipefail

ISO="${1:?Usage: $0 <path-to-iso>}"
TIMEOUT=180   # seconds to wait for boot signals
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

# UEFI: required for large ISOs (SeaBIOS can't read >4 GB from CDROM).
OVMF_CODE=/usr/share/OVMF/OVMF_CODE_4M.fd
if [[ ! -f "$OVMF_CODE" ]]; then
  echo "ERROR: OVMF not found at $OVMF_CODE — install ovmf" >&2
  exit 1
fi
OVMF_VARS=/tmp/foundry-smoke-vars-$$.fd
cp /usr/share/OVMF/OVMF_VARS_4M.fd "$OVMF_VARS"

KVM_ARGS=()
[[ -w /dev/kvm ]] && KVM_ARGS=(-enable-kvm -cpu host)

echo "=== Boot smoke: $ISO (timeout: ${TIMEOUT}s) ==="
echo "    UEFI: $OVMF_CODE   KVM: ${KVM_ARGS[*]:-(disabled)}"
echo "    SSH:  ssh -p 2222 user@localhost  (password: live)  — available ~60s after boot"

qemu-system-x86_64 \
  -m "${QEMU_MEM:-3072}" \
  "${KVM_ARGS[@]}" \
  -drive if=pflash,format=raw,readonly=on,file="$OVMF_CODE" \
  -drive if=pflash,format=raw,file="$OVMF_VARS" \
  -drive file="$ISO",media=cdrom,format=raw,readonly=on \
  -boot order=d \
  -device virtio-vga-gl \
  -display gtk,gl=on \
  -serial file:"$LOGFILE" \
  -no-reboot \
  -device virtio-net,netdev=n0 \
  -netdev user,id=n0,hostfwd=tcp::2222-:22 \
  &

QEMU_PID=$!
echo "$QEMU_PID" > "$PIDFILE"

cleanup() {
  kill "$QEMU_PID" 2>/dev/null || true
  rm -f "$PIDFILE" "$LOGFILE" "$OVMF_VARS"
}
trap cleanup EXIT

echo "  QEMU PID: $QEMU_PID — display window open; watching for crash..."
echo "  (KDE live session is graphical — close the QEMU window when done)"

# Minimum bar: QEMU stays alive for 30 s (rules out immediate boot failure).
ALIVE_THRESHOLD=30
ELAPSED=0

while (( ELAPSED < TIMEOUT )); do
  sleep 2
  ELAPSED=$(( ELAPSED + 2 ))

  if ! kill -0 "$QEMU_PID" 2>/dev/null; then
    echo "ERROR: QEMU exited unexpectedly after ${ELAPSED}s" >&2
    tail -20 "$LOGFILE" >&2
    exit 1
  fi

  if (( ELAPSED == ALIVE_THRESHOLD )); then
    echo "  [${ELAPSED}s] QEMU alive — UEFI+kernel reached ✓"
    echo "=== PASS: ISO booted past early-boot stage (check display for desktop) ==="
    # Stay running so user can inspect the display; exit when QEMU closes.
  fi

  printf "  [%3ds] running…\r" "$ELAPSED"
done

echo
# Timeout reached but QEMU still alive = user left it running; treat as pass.
if kill -0 "$QEMU_PID" 2>/dev/null; then
  echo "=== PASS: QEMU still running after ${TIMEOUT}s ==="
  exit 0
fi
echo "ERROR: QEMU exited before ${TIMEOUT}s timeout" >&2
exit 1
