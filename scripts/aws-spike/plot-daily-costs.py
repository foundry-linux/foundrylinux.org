#!/usr/bin/env python3
"""Plot an already computed simulation; no network or pricing assumptions."""
import argparse
import json
from pathlib import Path

import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import numpy as np

p = argparse.ArgumentParser(description=__doc__)
p.add_argument('--simulation', type=Path, required=True)
p.add_argument('--output', type=Path, required=True, help='Output filename stem, without extension')
a = p.parse_args()
result = json.loads(a.simulation.read_text())
rows = result['daily']
components = [('compute_usd', 'Compute + IPv4', '#275dad'),
              ('ebs_usd', 'Retained EBS', '#49a082'),
              ('egress_usd', 'Publication/download transfer', '#db671f'),
              ('artifact_storage_usd', 'S3 storage', '#8c70b4')]
plt.rcParams.update({'font.family': 'DejaVu Sans', 'font.size': 10,
                     'axes.spines.top': False, 'axes.spines.right': False})
fig, axes = plt.subplots(1, 2, figsize=(15, 6), gridspec_kw={'width_ratios': [3, 1]})
x = np.arange(len(rows))
bottom = np.zeros(len(rows))
for key, name, color in components:
    values = np.array([r[key] for r in rows])
    axes[0].bar(x, values, bottom=bottom, color=color, label=name, width=.8)
    bottom += values
axes[0].set_xticks(x, [r['date'][5:] for r in rows], rotation=60, ha='right')
axes[0].set_ylabel('Modeled USD per day')
axes[0].set_ylim(0, max(bottom) * 1.14)
axes[0].set_title('Per-day cost components', loc='left', weight='bold')
axes[0].yaxis.grid(True, alpha=.2)
axes[0].set_axisbelow(True)
axes[0].legend(loc='upper left', bbox_to_anchor=(0, -.23), ncol=4, frameon=False)
totals = [result['totals'][c[0]] for c in components]
bars = axes[1].barh([c[1] for c in components], totals, color=[c[2] for c in components], height=.55)
axes[1].invert_yaxis()
axes[1].set_xlim(0, max(totals) * 1.3)
axes[1].set_xlabel('Modeled USD over window')
axes[1].set_title(f'{len(rows)}-day totals', loc='left', weight='bold')
axes[1].xaxis.grid(True, alpha=.2)
axes[1].set_axisbelow(True)
axes[1].bar_label(bars, labels=[f'${v:.2f}' for v in totals], padding=4)
partial = 'Partial known-cost estimate' if result.get('is_partial') else 'Modeled estimate'
fig.suptitle(f'{partial}: ${result["totals"]["total_usd"]:.2f} / {len(rows)} days',
             x=.055, ha='left', fontsize=19)
fig.text(.055, .88, result['scenario'].get('name', 'Saved simulation'), fontsize=11)
fig.text(.055, .035, 'Historical activity × measured timing proxies + explicit scenario allowances. Not an AWS invoice.\n'
         'See the accompanying daily table for coverage, source-size proxies, retention and free-allowance assumptions.', fontsize=10, color='#444444')
fig.subplots_adjust(left=.055, right=.965, top=.79, bottom=.28, wspace=.5)
a.output.parent.mkdir(parents=True, exist_ok=True)
for extension in ['png', 'svg']:
    fig.savefig(a.output.with_suffix('.' + extension), dpi=180, facecolor='white')
