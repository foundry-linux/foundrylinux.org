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
DONE='#5b9bd5'   # blue = freed/done

# ---- Chart 1: cut list (removable clusters), DESC ----
# Updated for post-0.9.40 decisions:
#   opencv → foundry-cv (atelier); snapd unblocked via Mozilla firefox swap.
#   python-gamedev-extras row now shows only the non-opencv portion.
ghidra_mib = sz('ghidra')
jdk_mib = grp(['openjdk-21-jre-headless','openjdk-21-jdk-headless'])
mame_mib = grp(['mame','mame-data','mame-tools','mame-extra'])
wallpaper_mib = sz('plasma-workspace-wallpapers')
snapd_mib = grp(['snapd','snap-store','plasma-discover-backend-snap'])
cv_mib = sum(sz(n) for n in by if 'opencv' in n.lower() or 'vtk' in n.lower())
# python-gamedev-extras remaining (excluding opencv/vtk cluster)
extras_remain_mib = 554 - cv_mib  # 554 was original closure; cv_mib is now split off

clusters=[  # (label, MiB, action)
 (f"ghidra  ({ghidra_mib:.0f} MiB — near-incompressible jars)\n→ moved to atelier edition",
  ghidra_mib, "done"),
 (f"foundry-cv: opencv + libvtk9.5  ({cv_mib:.0f} MiB)\n→ new metapackage in atelier (VTK is the giant at 276 MiB)",
  cv_mib, "done"),
 (f"MAME + rest of foundry-retro-tools  ({mame_mib:.0f} MiB)\nMAME debugger, cc65, z80, dasm, radare2, f9dasm, libvgm, vgmstream …",
  mame_mib, "keep"),
 (f"foundry-python-gamedev-extras  ({extras_remain_mib:.0f} MiB)\nav · librosa · networkx · mss · fonttools · pynput · onnxruntime …",
  extras_remain_mib, "keep"),
 (f"worldfoundry  ({360:.0f} MiB)\nBlender + 10 WF CLIs + WF Blender add-ons",
  360, "keep"),
 (f"foundry-emulators-consoles  ({296:.0f} MiB)\nScummVM · PPSSPP · snes9x · dolphin · pcsx2 …",
  296, "keep"),
 (f"OpenJDK 21  ({jdk_mib:.0f} MiB — ghidra's only consumer)\n→ follows ghidra to atelier",
  jdk_mib, "done"),
 (f"stock Plasma wallpapers  ({wallpaper_mib:.0f} MiB)\n→ stripped (Recommends-only; we ship our own)",
  wallpaper_mib, "done"),
 (f"foundry-game-frameworks  ({217:.0f} MiB)\nSDL2/3 · SFML · LÖVE · Tiled · Allegro · libtcod",
  217, "keep"),
 (f"snapd  ({snapd_mib:.0f} MiB)\n→ unblocked: Mozilla firefox swap removes PreDepends: snapd",
  snapd_mib, "done"),
 (f"foundry-emulators-computers  ({75:.0f} MiB)\nDOSBox-X · hatari · fs-uae · vice · openmsx …",
  75, "keep"),
 (f"foundry-image-cli  ({29:.0f} MiB)\nCLI image utils",
  29, "keep"),
]
clusters.sort(key=lambda x:-x[1])
labels=[c[0] for c in clusters]; vals=[c[1] for c in clusters]
cmap={"done":DONE,"keep":GREEN,"nope":GRAY}
cols=[cmap[c[2]] for c in clusters]

fig,ax=plt.subplots(figsize=(12,8))
y=range(len(clusters))
ax.barh(list(y),vals,color=cols)
ax.set_yticks(list(y)); ax.set_yticklabels(labels,fontsize=8.5)
ax.invert_yaxis()
for i,v in enumerate(vals): ax.text(v+8,i,f"{v:.0f} MiB",va='center',fontsize=8)
ax.set_xlabel("Uncompressed installed size (MiB)")
done_total = sum(c[1] for c in clusters if c[2]=='done')
ax.set_title(
    f"Foundry anvil — trim decisions (source: 0.9.36 dpkg-status)\n"
    f"ghidra/opencv+VTK/wallpapers/snapd freed {done_total/1024:.1f} GiB uncompressed; MAME, emulators, sci-Python, Blender KEPT",
    fontsize=11)
ax.legend(handles=[
    Patch(color=DONE, label=f'freed — moved to atelier or stripped ({done_total/1024:.1f} GiB uncompressed)'),
    Patch(color=GREEN, label='KEEP — stays in anvil'),
], fontsize=9, loc='lower right')
ax.set_xlim(0, 1000); plt.tight_layout()
plt.savefig("chart1_cutlist.png", dpi=130); plt.close()

# ---- Chart 2: scenario ISO sizes vs stick capacity ----
# Baseline: 0.9.40 actual = 3.88 GiB (model was 0.65 GiB optimistic vs 0.9.40 build)
# Post-opencv+snapd projection uses model compression ratio (4.49/9.40 ≈ 0.48).
# onnxruntime adds ~20 MiB uncompressed → ~10 MiB on ISO.
RATIO = 4.49 / 9.40
iso_save = (cv_mib + snapd_mib - 20) * RATIO  # -20 for onnxruntime added
proj_gib = round(3.88 - iso_save/1024, 2)
scen=[
    ("NODESK\nnet-installer",   0.60),
    ("BASE KDE\ndesktop floor", 2.44),
    (f"anvil\n(projected ~{proj_gib:.2f} GiB)", proj_gib),
    ("anvil 0.9.40\n(actual)", 3.88),
    ("anvil 0.9.36\n(baseline)", 4.49),
]
names=[s[0] for s in scen]; isos=[s[1] for s in scen]
colors=[GRAY, GRAY, GREEN if proj_gib <= 3.6 else AMBER, EMBER, EMBER]
fig,ax=plt.subplots(figsize=(12,6.5))
bars=ax.bar(names,isos,color=colors,width=0.62)
for b,v in zip(bars,isos): ax.text(b.get_x()+b.get_width()/2,v+0.05,f"{v:.2f} GiB",ha='center',fontsize=9)
sticks=[("1 GB stick (~0.9 GiB)",0.9,RED),("2 GB stick (~1.8 GiB)",1.8,AMBER),
        ("4 GB stick (~3.6 GiB)",3.6,GREEN),("8 GB stick (~7.4 GiB)",7.4,'#2e6da4')]
for lbl,cap,c in sticks:
    ax.axhline(cap,color=c,ls='--',lw=1.4)
    ax.text(4.55,cap+0.03,lbl,color=c,fontsize=8.5,ha='right')
ax.set_ylabel("ISO size (GiB)")
ax.set_ylim(0,5.2)
ax.set_title(
    f"Anvil size history and projection — opencv+VTK+snapd removal, onnxruntime added\n"
    f"Projected ~{proj_gib:.2f} GiB; 4 GB stick target (≤3.60) needs build to confirm",
    fontsize=11)
plt.tight_layout()
plt.savefig("chart2_scenarios.png", dpi=130); plt.close()
print(f"charts written  (projected anvil: {proj_gib:.2f} GiB, target ≤3.60, gap {proj_gib-3.60:.2f} GiB)")
