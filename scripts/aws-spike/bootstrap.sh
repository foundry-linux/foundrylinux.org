#!/usr/bin/env bash
# Invoked by cloud-init on the NEW spike instance only.
set -Eeuo pipefail
exec > >(tee /var/log/foundry-spike-bootstrap.log) 2>&1
trap 'r=$?; if ((r)); then shutdown -h +2 "Foundry bootstrap failed; disks retained"; fi' EXIT
export DEBIAN_FRONTEND=noninteractive
date -u +%FT%TZ > /opt/foundry-spike/bootstrap-started-at
apt-get update
apt-get install -y --no-install-recommends ca-certificates curl docker.io git jq ovmf python3 rsync sysstat time xorriso qemu-system-x86 squashfs-tools
# Select only the new, empty 260-GiB EBS device. Never format by guessed NVMe order.
device=$(lsblk -J -b -o PATH,TYPE,SIZE,FSTYPE,MOUNTPOINTS | python3 -c '
import json,sys
ds=[d for d in json.load(sys.stdin)["blockdevices"] if d["type"]=="disk" and d["size"]==260*1024**3 and not d.get("children") and not d.get("fstype") and not any(d.get("mountpoints") or [])]
assert len(ds)==1, "Expected exactly one new empty 260-GiB disk"
print(ds[0]["path"])
')
mkfs.ext4 -L foundry-spike-cache "$device"
mkdir -p /srv/foundry-build
uuid=$(blkid -s UUID -o value "$device")
printf 'UUID=%s /srv/foundry-build ext4 defaults,nofail 0 2\n' "$uuid" >> /etc/fstab
mount /srv/foundry-build
systemctl stop docker.service docker.socket
mkdir -p /srv/foundry-build/docker /etc/docker
printf '{"data-root":"/srv/foundry-build/docker"}\n' > /etc/docker/daemon.json
systemctl start docker.service
test -c /dev/kvm
lsblk -f > /opt/foundry-spike/volumes.txt
date -u +%FT%TZ > /opt/foundry-spike/bootstrap-finished-at
# No automatic run on later boots: the warm run and VM test are explicit.
systemd-run --unit=foundry-spike-cold --property=RuntimeMaxSec=7h /bin/bash /opt/foundry-spike/run.sh cold
