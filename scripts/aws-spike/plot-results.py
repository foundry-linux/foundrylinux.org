#!/usr/bin/env python3
"""Render measured spike summaries; raw time-series remain on retained EBS."""
import json
from datetime import datetime
from pathlib import Path

import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import numpy as np

out = Path(__file__).resolve().parents[2] / 'docs/plans/assets/aws-spike-2026-11-15'
data = json.loads((out / 'measurements.json').read_text())
colors = ['#275dad', '#db671f']
plt.rcParams.update({'font.family': 'DejaVu Sans', 'font.size': 11,
                     'axes.spines.top': False, 'axes.spines.right': False,
                     'axes.titleweight': 'bold', 'savefig.facecolor': 'white'})

def duration(run, stage=None):
    record = data['runs'][run]
    if stage is None:
        return (datetime.fromisoformat(record['finished_at']) -
                datetime.fromisoformat(record['started_at'])).total_seconds()
    stamps = {}
    for line in record['stages'].splitlines():
        stamp, status, name = line.split()
        if name == stage:
            stamps[status] = datetime.fromisoformat(stamp)
    return (stamps['PASS'] - stamps['START']).total_seconds()

def label(seconds):
    seconds = round(seconds)
    if seconds < 60:
        return f'{seconds}s'
    return f'{seconds // 60}m {seconds % 60:02d}s'

def save(fig, name):
    fig.savefig(out / f'{name}.png', dpi=180)
    fig.savefig(out / f'{name}.svg')
    plt.close(fig)

fig, axes = plt.subplots(1, 3, figsize=(14, 5.4))
panels = [
    ('Build workflow', [duration('cold'), duration('warm')]),
    ('Packages / verified reuse', [duration('cold', 'packages'), duration('warm', 'packages-cache')]),
    ('Fresh ISO assembly', [duration('cold', 'iso'), duration('warm', 'iso')]),
]
for ax, (title, seconds) in zip(axes, panels):
    bars = ax.barh(['Cold', 'Warm'], np.array(seconds) / 60, color=colors, height=.5)
    ax.invert_yaxis()
    ax.set_xlim(0, max(seconds) / 60 * 1.35)
    ax.set_title(title, pad=16)
    ax.set_xlabel('Minutes • axis starts at zero')
    ax.xaxis.grid(True, alpha=.2)
    ax.set_axisbelow(True)
    for bar, value in zip(bars, seconds):
        ax.text(bar.get_width() + max(seconds) / 60 * .025,
                bar.get_y() + bar.get_height() / 2, label(value), va='center', fontsize=10)
fig.suptitle('Foundry AWS spike: cold vs retained-cache build', fontsize=18, x=.04, ha='left')
fig.text(.04, .12, 'Same m7i.xlarge • Mumbai • same package inputs and output hashes • forced ISO rebuild', fontsize=11)
fig.text(.04, .055, 'Cold workloads passed; post-build collector failed. Warm runner passed.\nWorkflow excludes provisioning and separate boot/installer validation. Warm reuses unchanged packages.', fontsize=10, color='#444444')
fig.subplots_adjust(left=.08, right=.98, top=.79, bottom=.29, wspace=.4)
save(fig, 'timings')

fig, axes = plt.subplots(1, 2, figsize=(12, 5.4))
for ax, mount, capacity, title in zip(axes, ['/', '/srv/foundry-build'], [16, 48], ['Root volume', 'Build and cache volume']):
    values = [data['runs'][r]['metrics']['max_sampled_filesystem_used_bytes'][mount] / 2**30 for r in ['cold', 'warm']]
    bars = ax.bar(['Cold peak', 'Warm peak', 'Provisioned'], values + [capacity], color=colors + ['#9aa3ad'], width=.55)
    ax.set_ylim(0, capacity * 1.18)
    ax.set_ylabel('GiB')
    ax.set_title(title)
    ax.yaxis.grid(True, alpha=.2)
    ax.set_axisbelow(True)
    ax.bar_label(bars, labels=[f'{v:.1f}' for v in values] + [str(capacity)], padding=5)
fig.suptitle('Measured peak space vs allocated capacity', fontsize=18, x=.06, ha='left')
fig.text(.06, .11, 'Warm peak includes the preserved cold ISO. Peaks are sampled filesystem usage, not cache requirements.', fontsize=10)
fig.text(.06, .055, '64 GiB allocated to the paired-run instance. Separate diagnostic 300 GiB is excluded from this chart.\nFilesystem formatting/reserve means usable space is smaller than provisioned capacity.', fontsize=10, color='#444444')
fig.subplots_adjust(left=.09, right=.97, top=.80, bottom=.27, wspace=.28)
save(fig, 'storage')
