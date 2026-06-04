import re
recs=[]
for b in open('dpkg-status',encoding='utf-8',errors='replace').read().split('\n\n'):
    if not b.strip():continue
    d={};k=None
    for ln in b.split('\n'):
        m=re.match(r'^(\S[^:]*):\s?(.*)$',ln)
        if m:k=m.group(1);d[k]=m.group(2)
        elif k:d[k]+=' '+ln.strip()
    if 'Package' in d:recs.append(d)
by={d['Package']:d for d in recs}
def usz(n):
    try:return int(by.get(n,{}).get('Installed-Size','0'))/1024
    except:return 0
prov={}
for d in recs:
    for p in re.findall(r'[a-z0-9][a-z0-9+.\-]*',d.get('Provides','')):
        prov.setdefault(p,set()).add(d['Package'])
def fd(n):
    if n in by:return n
    if n in prov:
        for c in prov[n]:
            if c in by:return c
def edges(d):
    o=set()
    for f in ('Depends','Pre-Depends','Recommends'):
        for cl in d.get(f,'').split(','):
            for alt in cl.split('|'):
                m=re.match(r'\s*([a-z0-9][a-z0-9+.\-]*)',alt)
                if m:
                    r=fd(m.group(1))
                    if r:o.add(r);break
    return o
kern=[p for p in by if p.startswith('linux-image') or p.startswith('linux-modules')]
roots=set([r for r in ['kubuntu-desktop','casper','ubuntu-minimal','ubuntu-standard'] if r in by]+kern)
roots|= {m for m in ['foundry-retro-tools','foundry-emulators-consoles','foundry-emulators-vintage','foundry-emulators-computers','foundry-game-frameworks','foundry-python-gamedev','foundry-python-gamedev-extras','foundry-image-cli','worldfoundry','foundry-desktop'] if m in by}
def closure(seeds,blocked=set()):
    seen=set();st=[s for s in seeds if s in by and s not in blocked]
    while st:
        n=st.pop()
        if n in seen:continue
        seen.add(n)
        for e in edges(by[n]):
            if e not in seen and e not in blocked:st.append(e)
    return seen
full=closure(roots)
# calibrate 2-class ISO model to actual 4.534 GiB
INC=re.compile(r'ghidra|firmware|fonts-|wallpaper|mame|scummvm|-data$|unicode-data|ibus-data|icon-theme|cursor-theme|breeze')
def isincomp(n): return bool(INC.search(n))
Ui=sum(usz(n) for n in full if isincomp(n)); Uc=sum(usz(n) for n in full if not isincomp(n))
TARGET=4.534*1024
# ISO = 0.92*Ui + r*Uc = TARGET  -> solve r
r=(TARGET-0.92*Ui)/Uc
def iso(n): return usz(n)*(0.92 if isincomp(n) else r)
print(f"calib: incompressible {Ui/1024:.2f} GiB, compressible {Uc/1024:.2f} GiB, r={r:.3f}; model total {sum(iso(n) for n in full)/1024:.3f} GiB (actual 4.534)")
def drop_iso(blocked):
    rem=full-closure(roots,blocked=set(blocked))
    return sum(iso(n) for n in rem)/1024, rem
base=sum(iso(n) for n in full)/1024
strips=['snapd','snap-store','plasma-workspace-wallpapers','breeze-wallpaper','linux-firmware-mellanox-spectrum']
print(f"\nFULL anvil model ISO: {base:.2f} GiB   (4 GB stick usable ~3.6 GiB)\n")
print("KEEPING ghidra+JDK. Cumulative drops (each line ADDS to the previous):")
steps=[("free strips (snapd/wallpapers/datacenter-fw)",strips),
       ("+ python-gamedev-extras (VTK/numba/scipy/librosa)",['foundry-python-gamedev-extras']),
       ("+ emulators-consoles (ScummVM…)",['foundry-emulators-consoles']),
       ("+ emulators-vintage + computers (MAME…)",['foundry-emulators-vintage','foundry-emulators-computers','mame-tools']),
       ("+ game-frameworks",['foundry-game-frameworks'])]
acc=[]
for label,add in steps:
    acc+=add
    sz,rem=drop_iso(acc)
    res=base-sz if False else None
    cur=base - (sum(iso(n) for n in rem)/1024)
    print(f"  {cur:5.2f} GiB  after {label}   (cut {base-cur:.2f} GiB)")
