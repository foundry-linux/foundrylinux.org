import re,json
recs=[]
for block in open('dpkg-status',encoding='utf-8',errors='replace').read().split('\n\n'):
    if not block.strip(): continue
    d={};key=None
    for line in block.split('\n'):
        m=re.match(r'^(\S[^:]*):\s?(.*)$',line)
        if m: key=m.group(1); d[key]=m.group(2)
        elif key: d[key]+=' '+line.strip()
    if 'Package' in d: recs.append(d)
by={d['Package']:d for d in recs}
def size(n):
    try:return int(by[n].get('Installed-Size','0'))
    except:return 0
prov={}
for d in recs:
    for p in re.findall(r'[a-z0-9][a-z0-9+.\-]*',d.get('Provides','')):
        prov.setdefault(p,set()).add(d['Package'])
def fd(name):
    if name in by:return name
    if name in prov:
        for c in prov[name]:
            if c in by:return c
    return None
def edges(d):
    out=set()
    for f in ('Depends','Pre-Depends','Recommends'):
        for clause in d.get(f,'').split(','):
            for alt in clause.split('|'):
                m=re.match(r'\s*([a-z0-9][a-z0-9+.\-]*)',alt)
                if m:
                    r=fd(m.group(1))
                    if r:out.add(r);break
    return out
def closure(seeds):
    seen=set();st=[s for s in seeds if s in by]
    while st:
        n=st.pop()
        if n in seen:continue
        seen.add(n)
        st+=[e for e in edges(by[n]) if e not in seen]
    return seen
def kib(s):return sum(size(n) for n in s)
def mib(s):return kib(s)/1024
def gib(s):return kib(s)/1024/1024
RATIO=9.48/4.534  # uncompressed GiB / squashfs GiB (global)

kern=[p for p in by if p.startswith('linux-image') or p.startswith('linux-modules')]
base_roots=set([r for r in ['kubuntu-desktop','casper','ubuntu-minimal','ubuntu-standard'] if r in by]+kern)
subs=['foundry-retro-tools','foundry-emulators-consoles','foundry-emulators-vintage',
 'foundry-emulators-computers','foundry-game-frameworks','foundry-python-gamedev',
 'foundry-python-gamedev-extras','foundry-image-cli','worldfoundry']
subs=[m for m in subs if m in by]
roots=base_roots|set(subs)|{'foundry-desktop'}
full=closure(roots)

print("== marginal cost of each foundry metapackage (drop just it) ==")
for m in sorted(subs,key=lambda m:-kib(full-closure(roots-{m}))):
    rem=full-closure(roots-{m})
    print(f"  {mib(rem):7.0f} MiB  {m:30} ({len(rem):3} excl pkgs)")

# grouped: drop the whole 'heavy toolkit' (retro+all emulators+python-extras)
heavy={'foundry-retro-tools','foundry-emulators-consoles','foundry-emulators-vintage','foundry-emulators-computers','foundry-python-gamedev-extras'}
rem=full-closure(roots-heavy)
print(f"\n  GROUP drop {{retro-tools + all emulators + python-extras}}: {mib(rem):.0f} MiB uncompressed, {len(rem)} pkgs")

def scen(name,keep):
    c=closure(keep); u=gib(c); sq=u/RATIO
    print(f"  {name:34} {u:5.2f} GiB uncompressed  -> ~{sq:4.2f} GiB squashfs/ISO  ({len(c)} pkgs)")
    return sq
print("\n== scenario ISO projections (global %.2f:1 compression) =="%RATIO)
scen("FULL anvil (current)",roots)
scen("SLIM: drop heavy toolkit",roots-heavy)
scen("SLIM+: also drop game-frameworks,image-cli",roots-heavy-{'foundry-game-frameworks','foundry-image-cli','foundry-python-gamedev'})
scen("MINI: KDE + Blender/WF only",base_roots|{'foundry-desktop','worldfoundry'})
scen("BASE: KDE desktop only (no foundry)",base_roots|{'foundry-desktop'})
scen("NODESK: kernel+minimal+casper",base_roots-{'kubuntu-desktop'})
print("\nStick usable (approx): 1GB~0.9GiB  2GB~1.8GiB  4GB~3.6GiB  8GB~7.4GiB")
