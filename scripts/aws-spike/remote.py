#!/usr/bin/env python3
"""Execute a diagnostic or stage command on the one receipted spike instance."""
import argparse
import json
import pathlib
import subprocess
import time

p = argparse.ArgumentParser()
p.add_argument("--state", type=pathlib.Path, required=True)
p.add_argument("--script", type=pathlib.Path)
p.add_argument("--command")
p.add_argument("--command-id")
a = p.parse_args()
s = json.loads(a.state.read_text())


def aws(action, data):
    r = subprocess.run(["aws", "--profile", s["profile"], "--region", s["region"],
        "--no-cli-pager", "--output", "json", "ssm", action,
        "--cli-input-json", json.dumps(data)], text=True, capture_output=True)
    if r.returncode:
        raise RuntimeError(r.stderr.strip())
    return json.loads(r.stdout)


if a.command_id:
    command_id = a.command_id
else:
    command = a.script.read_text() if a.script else a.command
    if not command:
        p.error("--script, --command or --command-id required")
    command_id = aws("send-command", {"InstanceIds": [s["instance_id"]],
        "DocumentName": "AWS-RunShellScript", "Parameters": {"commands": [command],
        "executionTimeout": ["120"]}})["Command"]["CommandId"]
    print("Command:", command_id, flush=True)
for attempt in range(18):
    try:
        r = aws("get-command-invocation", {"CommandId": command_id,
            "InstanceId": s["instance_id"]})
    except RuntimeError as e:
        if "InvocationDoesNotExist" not in str(e):
            raise
    else:
        if r["Status"] not in ("Pending", "InProgress", "Delayed"):
            print(r.get("StandardOutputContent", ""))
            print(r.get("StandardErrorContent", ""))
            print("Status:", r["Status"])
            raise SystemExit(0 if r["Status"] == "Success" else 1)
    time.sleep(3)
print("Still running; poll with --command-id", command_id)
