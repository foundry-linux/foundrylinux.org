#!/usr/bin/env bash
# Read the shipped filesystem, after live-build has removed its staging chroot.
set -Eeuo pipefail
iso=$(realpath "${1:?ISO path required}")
output=${2:?Output path required}
test ! -e "$output"
mountpoint=$(mktemp -d /tmp/foundry-iso-inventory.XXXXXX)
cleanup() {
    if mountpoint -q "$mountpoint"; then umount "$mountpoint"; fi
    rmdir "$mountpoint"
}
trap cleanup EXIT
mount -o loop,ro "$iso" "$mountpoint"
unsquashfs -cat "$mountpoint/live/filesystem.squashfs" var/lib/dpkg/status > "$output"
test -s "$output"
