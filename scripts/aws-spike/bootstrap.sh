#!/usr/bin/env bash
# Invoked by cloud-init on the NEW spike instance only.
set -Eeuo pipefail
exec > >(tee /var/log/foundry-spike-bootstrap.log) 2>&1
trap 'r=$?; if ((r)); then shutdown -h +2 "Foundry bootstrap failed; disks retained"; fi' EXIT
source /opt/foundry-spike/config.sh
# Keep host changes outside measured workloads. Ubuntu's periodic upgrader can
# otherwise restart transient controller services after updating libc.
systemctl disable --now apt-daily.timer apt-daily-upgrade.timer
printf 'APT::Periodic::Enable "0";\n' > /etc/apt/apt.conf.d/99foundry-spike-no-periodic
export NEEDRESTART_MODE=l
export DEBIAN_FRONTEND=noninteractive
date -u +%FT%TZ > /opt/foundry-spike/bootstrap-started-at
# Start before dependency downloads; retain bootstrap counters on the root disk.
python3 /opt/foundry-spike/observe.py --output /opt/foundry-spike/bootstrap-metrics.jsonl &
bootstrap_monitor=$!
apt-get update
apt-get install -y --no-install-recommends ca-certificates curl docker.io git jq ovmf python3 rsync sysstat time xorriso qemu-system-x86 squashfs-tools
# Select only the new, empty configured EBS device. Never format by guessed NVMe order.
device=$(lsblk -J -b -o PATH,TYPE,SIZE,FSTYPE,MOUNTPOINTS | python3 -c '
import json,sys
ds=[d for d in json.load(sys.stdin)["blockdevices"] if d["type"]=="disk" and d["size"]==int(sys.argv[1])*1024**3 and not d.get("children") and not d.get("fstype") and not any(d.get("mountpoints") or [])]
assert len(ds)==1, "Expected exactly one new empty disk matching BUILD_GIB"
print(ds[0]["path"])
' "$BUILD_GIB")
mkfs.ext4 -L foundry-spike-cache "$device"
mkdir -p /srv/foundry-build
uuid=$(blkid -s UUID -o value "$device")
printf 'UUID=%s /srv/foundry-build ext4 defaults,nofail 0 2\n' "$uuid" >> /etc/fstab
mount /srv/foundry-build
systemctl stop docker.service docker.socket
systemctl stop containerd.service
mkdir -p /srv/foundry-build/docker /etc/docker
# Docker's containerd snapshotter has a separate store from Docker data-root.
mv /var/lib/containerd /srv/foundry-build/containerd
mkdir -p /var/lib/containerd
printf '/srv/foundry-build/containerd /var/lib/containerd none bind 0 0\n' >> /etc/fstab
mount /var/lib/containerd
printf '{"data-root":"/srv/foundry-build/docker"}\n' > /etc/docker/daemon.json
systemctl start containerd.service
systemctl start docker.service
test -c /dev/kvm
lsblk -f > /opt/foundry-spike/volumes.txt
date -u +%FT%TZ > /opt/foundry-spike/bootstrap-finished-at
kill "$bootstrap_monitor"
mkdir -p /srv/foundry-build/runs/bootstrap
cp /opt/foundry-spike/bootstrap-* /srv/foundry-build/runs/bootstrap/
docker images --digests > /srv/foundry-build/runs/bootstrap/images-before.txt
find /srv/foundry-build -maxdepth 2 -type d > /srv/foundry-build/runs/bootstrap/directory-inventory.txt
# No automatic run on later boots: the warm run and VM test are explicit.
systemd-run --unit=foundry-spike-cold --property=RuntimeMaxSec=7h /bin/bash /opt/foundry-spike/run.sh cold
