#!/usr/bin/env python3
"""Retain raw host/cgroup counters, not extrapolated resource claims.

One-second samples are approximate; container counters can disappear between
samples. memory.peak covers a cgroup lifetime, not earlier destroyed cgroups.
"""
import argparse
import datetime
import json
import pathlib
import time

p = argparse.ArgumentParser()
p.add_argument("--output", type=pathlib.Path, required=True)
a = p.parse_args()
a.output.parent.mkdir(parents=True, exist_ok=True)


def read(path):
    try:
        return path.read_text().strip()
    except (FileNotFoundError, ProcessLookupError):
        return None


with a.output.open("a", buffering=1) as output:
    while True:
        started = time.monotonic()
        sample = {"utc": datetime.datetime.now(datetime.timezone.utc).isoformat(),
                  "monotonic_ns": time.monotonic_ns(), "containers": {}}
        for name in ["stat", "meminfo", "diskstats", "net/dev"]:
            sample[name] = read(pathlib.Path("/proc") / name)
        for group in pathlib.Path("/sys/fs/cgroup/system.slice").glob("docker-*.scope"):
            sample["containers"][group.name] = {name: read(group / name) for name in
                ["cpu.stat", "cpu.max", "memory.current", "memory.peak", "memory.events",
                 "memory.max", "memory.stat", "io.stat", "cpu.pressure", "memory.pressure", "io.pressure"]}
        output.write(json.dumps(sample) + "\n")
        time.sleep(max(0, 1 - (time.monotonic() - started)))
