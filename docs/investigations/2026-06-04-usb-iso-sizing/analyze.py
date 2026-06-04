import re, collections
recs=[]
cur={}
for block in open('dpkg-status',encoding='utf-8',errors='replace').read().split('\n\n'):
    if not block.strip(): continue
    d={}
    key=None
    for line in block.split('\n'):
        m=re.match(r'^(\S[^:]*):\s?(.*)$',line)
        if m: key=m.group(1); d[key]=m.group(2)
        elif key: d[key]+=' '+line.strip()
    if 'Package' in d: recs.append(d)
by={d['Package']:d for d in recs}
def size(d): 
    try: return int(d.get('Installed-Size','0'))
    except: return 0

# webengine packages installed
we=[p for p in by if 'webengine' in p.lower()]
print("=== installed webengine packages ===")
for p in sorted(we): print(f"  {p:40} {size(by[p])/1024:8.1f} MiB")

# reverse deps: who Depends/Recommends a webengine pkg
def deps(d,field): 
    s=d.get(field,'')
    return set(re.findall(r'[a-z0-9][a-z0-9+.\-]+', s.split(':')[0] if False else s))
wenames=set(we)
print("\n=== reverse-deps (installed pkgs that Depend/Recommend a webengine pkg) ===")
found=False
for p,d in by.items():
    for f in ('Depends','Recommends','Pre-Depends'):
        body=d.get(f,'')
        # split on comma, each alt on |
        for clause in body.split(','):
            names={re.match(r'\s*([a-z0-9][a-z0-9+.\-]*)',a).group(1) for a in clause.split('|') if re.match(r'\s*([a-z0-9][a-z0-9+.\-]*)',a)}
            if names & wenames:
                print(f"  {p:35} [{f}] -> {clause.strip()}"); found=True
if not found: print("  (none — nothing installed depends on QtWebEngine; it is orphaned)")

print("\n=== top 30 packages by installed size ===")
for d in sorted(recs,key=size,reverse=True)[:30]:
    print(f"  {size(d)/1024:8.1f} MiB  {d['Package']}")
