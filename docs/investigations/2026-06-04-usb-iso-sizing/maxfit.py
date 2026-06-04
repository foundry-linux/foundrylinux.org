import re
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
def size(n):
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
    return None
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
# who depends on X (installed)?
def rdeps(X):
    out=[]
    for p,d in by.items():
        if X in edges(d): out.append(p)
    return out
for X in ['ghidra','openjdk-21-jre-headless','openjdk-21-jdk-headless','default-jre','mame']:
    if X in by: print(f"rdeps({X}) = {rdeps(X)}")

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
            if e not in seen and e not in blocked: st.append(e)
    return seen
full=closure(roots)
def gib(s):return sum(size(n) for n in s)/1024
def freed(blocked):
    f=full-closure(roots,blocked=set(blocked)); return f
# ISO impact heuristic: jars/images/firmware ~0.95x, code ~0.5x. classify by name.
def iso_mib(n):
    s=size(n)
    if any(k in n for k in ['ghidra','wallpaper','-data','firmware','fonts-','mame']): return s*0.92
    return s*0.5
print(f"\nFULL closure: {gib(full):.2f} GiB uncompressed, ~{sum(iso_mib(n) for n in full)/1024:.2f} GiB est-ISO (actual 4.53)")
# free strips (ISO-level, not via metapackages)
strips=['snapd','snap-store','plasma-workspace-wallpapers','breeze-wallpaper','linux-firmware-mellanox-spectrum']
strips=[s for s in strips if s in by]
print(f"\nfree strips {strips}: -{sum(iso_mib(s) for s in strips)/1024:.2f} GiB ISO")
# drop ghidra (+ what it frees)
fg=freed(['ghidra'])
print(f"drop ghidra frees {len(fg)} pkgs: {sorted(fg)} = -{sum(iso_mib(n) for n in fg)/1024:.2f} GiB ISO")
def proj(blocked):
    rem=full-closure(roots,blocked=set(blocked))
    iso=sum(iso_mib(n) for n in full)-sum(iso_mib(n) for n in rem)
    return iso/1024
base=sum(iso_mib(n) for n in full)/1024
for name,bl in [("free strips only",strips),
                ("free strips + ghidra",strips+['ghidra']),
                ("free strips + ghidra + openjdk",strips+['ghidra','openjdk-21-jre-headless','openjdk-21-jdk-headless'])]:
    print(f"  {name:38} -> ~{proj(bl):.2f} GiB est-ISO")
print("\n4 GB stick usable ~3.6 GiB")
