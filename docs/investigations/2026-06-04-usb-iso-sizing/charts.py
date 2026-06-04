import re
import matplotlib; matplotlib.use('Agg')
import matplotlib.pyplot as plt
from matplotlib.patches import Patch
recs=[]
for block in open('dpkg-status',encoding='utf-8',errors='replace').read().split('\n\n'):
    if not block.strip():continue
    d={};k=None
    for line in block.split('\n'):
        m=re.match(r'^(\S[^:]*):\s?(.*)$',line)
        if m:k=m.group(1);d[k]=m.group(2)
        elif k:d[k]+=' '+line.strip()
    if 'Package' in d:recs.append(d)
by={d['Package']:d for d in recs}
def sz(n):
    try:return int(by.get(n,{}).get('Installed-Size','0'))/1024
    except:return 0
def grp(names):return sum(sz(n) for n in names if n in by)

EMBER='#e8662a'; RED='#c0392b'; AMBER='#e0a800'; GREEN='#2e8b57'; GRAY='#888'
# ---- Chart 1: cut list (removable clusters), DESC ----
clusters=[  # (label, MiB, action)
 ("foundry-retro-tools\n(ghidra 864 · MAME 553 · JDK 286)",1890,"drop"),
 ("python-gamedev-extras\n(VTK · numba/llvmlite · scipy · librosa)",554,"drop"),
 ("emulators-consoles\n(ScummVM …)",296,"drop"),
 ("stock Plasma wallpapers\n(we ship our own)",grp(['plasma-workspace-wallpapers','breeze-wallpaper']),"strip"),
 ("snapd  (already in strip-list — BUG: still present)",grp(['snapd','snap-store']),"strip"),
 ("emulators-computers",75,"drop"),
 ("exotic firmware\n(mellanox-spectrum datacenter sw …)",grp(['linux-firmware-mellanox-spectrum']),"strip"),
 ("foundry-image-cli (CLI image utils)",29,"keep"),
 ("worldfoundry  (Blender + WF tools)",360,"keep"),
 ("game-frameworks (SDL/SFML/LÖVE/Tiled…)",217,"keep"),
]
clusters.sort(key=lambda x:-x[1])
labels=[c[0] for c in clusters]; vals=[c[1] for c in clusters]
cmap={"drop":RED,"strip":AMBER,"keep":GREEN}; cols=[cmap[c[2]] for c in clusters]
fig,ax=plt.subplots(figsize=(11,6.5))
y=range(len(clusters))
ax.barh(list(y),vals,color=cols)
ax.set_yticks(list(y)); ax.set_yticklabels(labels,fontsize=9)
ax.invert_yaxis()
for i,v in enumerate(vals): ax.text(v+12,i,f"{v:.0f} MiB",va='center',fontsize=8.5)
ax.set_xlabel("Uncompressed installed size (MiB)")
ax.set_title("Foundry anvil 0.9.36 — trim candidates, largest first\n(uncompressed; ghidra/wallpapers compress ~1:1 so on-ISO savings are even larger)",fontsize=11)
ax.legend(handles=[Patch(color=RED,label='drop (heavy toolkit → heavier edition)'),
                   Patch(color=AMBER,label='strip (free / no functional loss)'),
                   Patch(color=GREEN,label='KEEP (core game-making)')],fontsize=9,loc='lower right')
ax.set_xlim(0,2100); plt.tight_layout(); plt.savefig("chart1_cutlist.png",dpi=130); plt.close()

# ---- Chart 2: scenario ISO sizes vs stick capacity ----
scen=[("NODESK\nnet-installer",0.60),("BASE KDE\ndesktop floor",2.44),
      ("anvil NEW\n(ghidra→atelier)",3.30),("anvil OLD\n(0.9.36)",4.49)]
names=[s[0] for s in scen]; isos=[s[1] for s in scen]
fig,ax=plt.subplots(figsize=(11,6.5))
bars=ax.bar(names,isos,color=[GRAY,GRAY,GREEN,EMBER],width=0.62)
for b,v in zip(bars,isos): ax.text(b.get_x()+b.get_width()/2,v+0.05,f"{v:.2f} GiB",ha='center',fontsize=9)
sticks=[("1 GB stick (~0.9 GiB)",0.9,RED),("2 GB stick (~1.8 GiB)",1.8,AMBER),
        ("4 GB stick (~3.6 GiB)",3.6,GREEN),("8 GB stick (~7.4 GiB)",7.4,'#2e6da4')]
for lbl,cap,c in sticks:
    ax.axhline(cap,color=c,ls='--',lw=1.4)
    ax.text(3.55,cap+0.03,lbl,color=c,fontsize=8.5,ha='right')
ax.set_ylabel("Projected ISO size (GiB)")
ax.set_ylim(0,5.0)
ax.set_title("Trimming anvil to a 4 GB stick — move ghidra to atelier (keep the rest)\nKDE-desktop floor (2.44 GiB) means 4 GB is the smallest stick for ANY KDE live image",fontsize=11)
plt.tight_layout(); plt.savefig("chart2_scenarios.png",dpi=130); plt.close()
print("charts written")
