#!/usr/bin/env python3
"""Summarize observed samples only; never infer missing start/end coverage."""
import argparse
import json
import pathlib

p = argparse.ArgumentParser()
p.add_argument('input', type=pathlib.Path)
a = p.parse_args()
first = last = None
samples = 0
filesystems = {}
containers = {}
for line in a.input.open():
    try:
        row = json.loads(line)
    except json.JSONDecodeError:
        # An actively written final line is not a complete sample.
        continue
    first = first or row
    last = row
    samples += 1
    for mount, values in row.get('filesystems', {}).items():
        filesystems[mount] = max(filesystems.get(mount, 0), values['used_bytes'])
    for name, counters in row['containers'].items():
        group = containers.setdefault(name, {'first_utc': row['utc'], 'peak_memory_bytes': 0,
            'max_sampled_anon_bytes': 0, 'max_sampled_file_bytes': 0})
        group['last_utc'] = row['utc']
        if counters.get('memory.peak'):
            group['peak_memory_bytes'] = max(group['peak_memory_bytes'], int(counters['memory.peak']))
        if counters.get('memory.stat'):
            memory = dict(line.split() for line in counters['memory.stat'].splitlines())
            for key in ('anon', 'file'):
                field = f'max_sampled_{key}_bytes'
                group[field] = max(group[field], int(memory.get(key, 0)))
        if counters.get('cpu.stat'):
            cpu = dict(line.split() for line in counters['cpu.stat'].splitlines())
            usage = int(cpu['usage_usec'])
            if 'first_cpu_usec' not in group:
                group['first_cpu_usec'] = usage
            if usage < group.get('last_cpu_usec', usage):
                group['counter_reset_detected'] = True
            group['last_cpu_usec'] = usage
if not samples:
    raise SystemExit('No complete samples')
print(json.dumps({'input': str(a.input), 'samples': samples, 'first_utc': first['utc'],
    'last_utc': last['utc'], 'observed_seconds': (last['monotonic_ns']-first['monotonic_ns'])/1e9,
    'max_sampled_filesystem_used_bytes': filesystems, 'containers': containers,
    'limitations': 'Observed interval only. memory.peak is cgroup-lifetime peak and includes file cache. '
        'Sampled anon/file maxima need not coincide. Missing earlier/later samples are not reconstructed.'}, indent=2))
