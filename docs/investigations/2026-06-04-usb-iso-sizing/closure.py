import re
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
    try: return int(by[n].get('Installed-Size','0'))
    except: return 0
# provides map
prov={}
for d in recs:
    for p in re.findall(r'[a-z0-9][a-z0-9+.\-]*', d.get('Provides','')):
        prov.setdefault(p,set()).add(d['Package'])
def firstdep(name):  # resolve a dep token to an installed concrete pkg
    if name in by: return name
    if name in prov:
        for c in prov[name]:
            if c in by: return c
    return None
def edges(d, fields=('Depends','Pre-Depends','Recommends')):
    out=set()
    for f in fields:
        for clause in d.get(f,'').split(','):
            for alt in clause.split('|'):
                m=re.match(r'\s*([a-z0-9][a-z0-9+.\-]*)',alt)
                if m:
                    r=firstdep(m.group(1))
                    if r: out.add(r); break
    return out
def closure(seeds):
    seen=set(); stack=[s for s in seeds if s in by]
    while stack:
        n=stack.pop()
        if n in seen: continue
        seen.add(n)
        for e in edges(by[n]):
            if e not in seen: stack.append(e)
    return seen
def gib(kib): return kib/1024/1024
def setsize(s): return sum(size(n) for n in s)

base_roots=[r for r in ['kubuntu-desktop','casper','ubuntu-minimal','ubuntu-standard'] if r in by]
# kernel
base_roots+=[p for p in by if p.startswith('linux-image') or p.startswith('linux-generic')]
foundry_meta=['foundry-retro-tools','foundry-emulators-consoles','foundry-emulators-vintage',
 'foundry-emulators-computers','foundry-game-frameworks','foundry-python-gamedev',
 'foundry-python-gamedev-extras','foundry-image-cli','worldfoundry']
foundry_meta=[m for m in foundry_meta if m in by]
all_roots=set(base_roots+foundry_meta+['foundry-core','foundry-desktop','foundry-anvil'])

TOT=setsize(set(by))
full=closure(all_roots)
base=closure(set(base_roots)+ {'foundry-desktop'} if False else set(base_roots))
print(f"TOTAL installed (all {len(by)} pkgs): {gib(TOT):.2f} GiB")
print(f"closure(all roots): {len(full)} pkgs, {gib(setsize(full)):.2f} GiB")
print(f"BASE floor closure(kubuntu-desktop+kernel+casper): {len(base)} pkgs, {gib(setsize(base)):.2f} GiB\n")

print("=== marginal savings if each foundry metapackage is DROPPED (DESC) ===")
rows=[]
for m in foundry_meta:
    keep=set(all_roots)-{m}
    removed=full-closure(keep)
    rows.append((m, setsize(removed), len(removed), sorted(removed,key=size,reverse=True)[:4]))
for m,sz,n,top in sorted(rows,key=lambda r:-r[1]):
    print(f"  {gib(sz)*1024:7.0f} MiB  {m:32} ({n:4} pkgs)  e.g. "+", ".join(f"{t}:{size(t)//1024}M" for t in top))

# desktop floor: base minus the desktop (server-class)
nondesk=closure([r for r in base_roots if 'kubuntu' not in r])
print(f"\nBASE without KDE desktop (kernel+minimal+casper): {gib(setsize(nondesk)):.2f} GiB uncompressed")
