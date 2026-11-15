#!/usr/bin/env python3
"""Fingerprint the controlled A/B inputs and verify complete retained outputs.

Only record after the full package stage (including explicit repairs) succeeds.
This is a paired experiment cache, not an upstream-update policy.
"""
import argparse
import hashlib
import json
import pathlib
import subprocess

p = argparse.ArgumentParser()
p.add_argument("action", choices=["record", "verify"])
p.add_argument("--repo", type=pathlib.Path, required=True)
p.add_argument("--receipt", type=pathlib.Path, required=True)
a = p.parse_args()
repo = a.repo.resolve()


def digest(path):
    with path.open("rb") as f:
        return hashlib.file_digest(f, "sha256").hexdigest()


paths = subprocess.check_output(["git", "-C", str(repo), "ls-files", "-z", "--",
    "foundry-apt/packages", "foundry-apt/docker", "foundry-apt/scripts"]).decode().split("\0")
inputs = {path: digest(repo / path) for path in paths if path and (repo / path).is_file()}
builder = subprocess.check_output(["docker", "image", "inspect", "--format", "{{.Id}}",
    "foundry-spike/packages:run"], text=True).strip()
actual = {"inputs": inputs, "builder_image": builder}
dist = repo / "foundry-apt/dist"
if a.action == "record":
    outputs = {p.name: digest(p) for p in sorted(dist.iterdir()) if p.is_file()}
    if not any(name.endswith(".deb") for name in outputs):
        raise SystemExit("No package outputs; refusing to record an empty cache")
    actual["outputs"] = outputs
    a.receipt.write_text(json.dumps(actual, indent=2) + "\n")
    print(f"Recorded {len(inputs)} inputs and {len(outputs)} outputs")
else:
    recorded = json.loads(a.receipt.read_text())
    if any(actual[key] != recorded[key] for key in actual):
        raise SystemExit("Source/recipe/builder inputs changed; not a valid warm-cache comparison")
    if {p.name for p in dist.iterdir() if p.is_file()} != set(recorded["outputs"]):
        raise SystemExit("Cached artifact inventory changed")
    for name, expected in recorded["outputs"].items():
        if pathlib.Path(name).name != name:
            raise SystemExit("Invalid output path in cache receipt")
        path = dist / name
        if not path.is_file() or digest(path) != expected:
            raise SystemExit(f"Missing or modified cached artifact: {name}")
    print(f"Verified unchanged inputs and {len(recorded['outputs'])} cached artifacts")
