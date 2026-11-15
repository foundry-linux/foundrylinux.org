#!/usr/bin/env python3
"""Fetch a small evidence file through SSM, checking size and SHA256.

No public listener, bucket credentials or inbound security-group rule required.
Large build artifacts remain on retained EBS; use this for screenshots/receipts.
"""
import argparse
import base64
import hashlib
import concurrent.futures
import json
import pathlib
import shlex
import subprocess
import time

p = argparse.ArgumentParser()
p.add_argument('--state', type=pathlib.Path, required=True)
p.add_argument('--remote', required=True)
p.add_argument('--output', type=pathlib.Path, required=True)
a = p.parse_args()
s = json.loads(a.state.read_text())
if not a.remote.startswith('/') or a.output.exists():
    p.error('Use an absolute remote path and a new local output path')


def aws(action, data):
    r = subprocess.run(['aws', '--profile', s['profile'], '--region', s['region'],
        '--no-cli-pager', '--output', 'json', 'ssm', action,
        '--cli-input-json', json.dumps(data)], capture_output=True, text=True, check=True)
    return json.loads(r.stdout)


def remote(code):
    command = 'python3 -c ' + shlex.quote(code) + ' ' + shlex.quote(a.remote)
    cid = aws('send-command', {'InstanceIds': [s['instance_id']],
        'DocumentName': 'AWS-RunShellScript', 'Parameters': {'commands': [command]}})['Command']['CommandId']
    for _ in range(60):
        try:
            result = aws('get-command-invocation', {'CommandId': cid, 'InstanceId': s['instance_id']})
        except subprocess.CalledProcessError as e:
            if 'InvocationDoesNotExist' not in e.stderr:
                raise
        else:
            if result['Status'] == 'Success':
                return result['StandardOutputContent'].strip()
            if result['Status'] not in ('Pending', 'InProgress', 'Delayed'):
                raise RuntimeError(result)
        time.sleep(1)
    raise TimeoutError(cid)


meta = json.loads(remote('import sys,pathlib,hashlib,json; p=pathlib.Path(sys.argv[1]); '
    'print(json.dumps({"bytes":p.stat().st_size,"sha256":hashlib.sha256(p.read_bytes()).hexdigest()}))'))
if meta['bytes'] > 8 * 1024 * 1024:
    raise SystemExit('Evidence exceeds 8 MiB; preserve large artifacts on EBS')
a.output.parent.mkdir(parents=True, exist_ok=True)
def chunk(offset):
    data = remote('import sys,base64; f=open(sys.argv[1],"rb"); '
        f'f.seek({offset}); print(base64.b64encode(f.read(12000)).decode())')
    return base64.b64decode(data, validate=True)


with a.output.open('xb') as output, concurrent.futures.ThreadPoolExecutor(max_workers=4) as pool:
    for data in pool.map(chunk, range(0, meta['bytes'], 12000)):
        output.write(data)
actual = hashlib.sha256(a.output.read_bytes()).hexdigest()
if a.output.stat().st_size != meta['bytes'] or actual != meta['sha256']:
    raise SystemExit('Evidence changed or transfer incomplete; output is NOT verified')
print(json.dumps({'output': str(a.output), **meta}))
