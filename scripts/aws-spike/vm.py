#!/usr/bin/env python3
"""Diskless, offline UEFI boot observation. A screenshot is not an automatic PASS."""
import argparse
import json
import pathlib
import shutil
import socket
import subprocess
import time

p = argparse.ArgumentParser()
p.add_argument("action", choices=["start", "capture", "keys", "type", "stop"])
p.add_argument("--output", type=pathlib.Path, required=True)
p.add_argument("--iso", type=pathlib.Path)
p.add_argument("--keys", help="QEMU sendkey expression, e.g. ctrl-alt-t")
p.add_argument("--text", help="Lowercase text to type into the guest, without pressing Enter")
a = p.parse_args()
out = a.output.resolve()
out.mkdir(parents=True, exist_ok=True)
sock = out / "qmp.sock"
if a.action == "start":
    if not a.iso or not a.iso.is_file():
        p.error("start requires an existing --iso")
    if sock.exists():
        raise SystemExit("Existing VM socket; inspect/stop it before starting another VM")
    shutil.copyfile("/usr/share/OVMF/OVMF_VARS_4M.fd", out / "OVMF_VARS.fd")
    subprocess.run(["qemu-system-x86_64", "-name", "foundry-spike-boot",
        "-machine", "q35,accel=kvm", "-cpu", "host", "-smp", "2", "-m", "4096",
        "-drive", "if=pflash,format=raw,readonly=on,file=/usr/share/OVMF/OVMF_CODE_4M.fd",
        "-drive", f"if=pflash,format=raw,file={out / 'OVMF_VARS.fd'}",
        "-device", "virtio-vga", "-display", "none", "-nic", "none",
        "-cdrom", str(a.iso.resolve()), "-boot", "d",
        "-qmp", f"unix:{sock},server=on,wait=off", "-serial", f"file:{out / 'serial.log'}",
        "-pidfile", str(out / "qemu.pid"), "-daemonize"], check=True)
    (out / "started-at.json").write_text(json.dumps({"unix_time": time.time(),
        "iso": str(a.iso.resolve()), "guest_memory_mib": 4096, "guest_vcpus": 2}) + "\n")
    print("VM started; observe desktop and installer before recording any PASS")
    raise SystemExit(0)

s = socket.socket(socket.AF_UNIX)
s.settimeout(15)
s.connect(str(sock))
f = s.makefile("rwb", buffering=0)
f.readline()


def call(command):
    f.write((json.dumps(command) + "\n").encode())
    while True:
        line = f.readline()
        if not line:
            raise RuntimeError("QMP disconnected")
        response = json.loads(line)
        if "error" in response:
            raise RuntimeError(response["error"])
        if "return" in response:
            return response["return"]


call({"execute": "qmp_capabilities"})
if a.action == "capture":
    call({"execute": "screendump", "arguments": {"filename": str(out / "screen.png"), "format": "png"}})
    print(out / "screen.png")
elif a.action == "keys":
    if not a.keys or any(c not in "abcdefghijklmnopqrstuvwxyz0123456789-_ " for c in a.keys):
        raise SystemExit("Invalid QEMU key expression")
    print(call({"execute": "human-monitor-command", "arguments": {"command-line": "sendkey " + a.keys}}))
elif a.action == "type":
    if not a.text or any(c not in "abcdefghijklmnopqrstuvwxyz0123456789 -./" for c in a.text):
        raise SystemExit("Unsupported guest text; use lowercase letters, digits, spaces or -./")
    for char in a.text:
        key = {" ": "spc", ".": "dot", "/": "slash", "-": "minus"}.get(char, char)
        call({"execute": "human-monitor-command", "arguments": {"command-line": "sendkey " + key}})
        time.sleep(0.08)
    print("Typed guest text; Enter is a separate explicit key action")
else:
    call({"execute": "quit"})
    print("Test VM stopped; screenshots and logs retained")
