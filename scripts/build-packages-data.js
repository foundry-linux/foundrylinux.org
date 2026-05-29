#!/usr/bin/env node
// Fetches Packages.gz from both Foundry apt repos + Ubuntu's resolute (26.04)
// main/universe/multiverse, parses them, computes the transitive Depends
// closure of every category/edition metapackage, emits site/packages-data.json.
//
// Pure Node — no docker, no shell-out. ~5 seconds total. See
// docs/plans/2026-05-21-packages-page.md §1.
'use strict';

const fs = require('fs');
const path = require('path');
const zlib = require('zlib');
const crypto = require('crypto');
const { promisify } = require('util');
const gunzip = promisify(zlib.gunzip);

const ROOT = process.env.WORK_ROOT || path.resolve(__dirname, '..');
const CATEGORIES_PATH = path.join(ROOT, 'data/categories.json');
const UPSTREAM_PATH   = path.join(ROOT, 'data/upstream.yml');
const OUT_PATH        = path.join(ROOT, 'site/packages-data.json');
const META_DIR        = path.join(ROOT, 'foundry-apt/public/meta');

function loadMeta(name) {
  try { return JSON.parse(fs.readFileSync(path.join(META_DIR, `${name}.json`), 'utf8')); }
  catch { return null; }
}

// ─── archive list ────────────────────────────────────────────────────────────
const UBUNTU_SUITE = 'resolute'; // Ubuntu 26.04 LTS

const ARCHIVES = [
  { origin: 'foundry',           url: 'https://apt.foundrylinux.org/dists/resolute/main/binary-amd64/Packages.gz',                          release: 'https://apt.foundrylinux.org/dists/resolute/Release' },
  { origin: 'worldfoundry',      url: 'https://apt.worldfoundry.org/dists/stable/main/binary-amd64/Packages.gz',                            release: 'https://apt.worldfoundry.org/dists/stable/Release' },
  { origin: 'ubuntu-main',       url: `http://archive.ubuntu.com/ubuntu/dists/${UBUNTU_SUITE}/main/binary-amd64/Packages.gz` },
  { origin: 'ubuntu-universe',   url: `http://archive.ubuntu.com/ubuntu/dists/${UBUNTU_SUITE}/universe/binary-amd64/Packages.gz` },
  { origin: 'ubuntu-multiverse', url: `http://archive.ubuntu.com/ubuntu/dists/${UBUNTU_SUITE}/multiverse/binary-amd64/Packages.gz` },
];

// ─── tiny yaml parser for our flat data/upstream.yml ─────────────────────────
function parseUpstreamYaml(text) {
  const out = {};
  let currentKey = null;
  for (const rawLine of text.split('\n')) {
    const line = rawLine.replace(/\r$/, '');
    if (!line.trim() || line.trim().startsWith('#')) continue;
    if (/^[A-Za-z0-9._+-]+:\s*$/.test(line)) {
      currentKey = line.split(':')[0].trim();
      out[currentKey] = {};
      continue;
    }
    const m = line.match(/^\s+([A-Za-z_][A-Za-z0-9_-]*):\s*(.+?)\s*$/);
    if (m && currentKey) {
      let val = m[2];
      if ((val.startsWith('"') && val.endsWith('"')) ||
          (val.startsWith("'") && val.endsWith("'"))) {
        val = val.slice(1, -1);
      }
      out[currentKey][m[1]] = /^-?\d+$/.test(val) ? Number(val) : val;
    }
  }
  return out;
}

// ─── fetch + parse Packages.gz ───────────────────────────────────────────────
async function fetchPackages(archive) {
  process.stderr.write(`  · fetching ${archive.origin.padEnd(20)} `);
  const t0 = Date.now();
  const res = await fetch(archive.url);
  if (!res.ok) throw new Error(`${archive.origin}: HTTP ${res.status} ${res.statusText}`);
  const buf = Buffer.from(await res.arrayBuffer());
  const decompressed = await gunzip(buf);
  const map = parsePackages(decompressed.toString('utf8'), archive.origin);
  process.stderr.write(`${(buf.length / 1024).toFixed(0).padStart(5)} KB gz · ${map.size.toString().padStart(5)} packages · ${Date.now() - t0}ms\n`);
  return map;
}

async function fetchReleaseSha(url) {
  try {
    const res = await fetch(url);
    if (!res.ok) return null;
    const text = await res.text();
    return crypto.createHash('sha256').update(text).digest('hex');
  } catch {
    return null;
  }
}

function parsePackages(text, origin) {
  const map = new Map();
  for (const stanza of text.split(/\n\n/)) {
    if (!stanza.trim()) continue;
    const fields = {};
    let currentKey = null;
    for (const line of stanza.split('\n')) {
      const m = line.match(/^([A-Z][A-Za-z0-9-]*?):\s?(.*)$/);
      if (m) {
        currentKey = m[1];
        fields[currentKey] = m[2];
      } else if (currentKey && (line.startsWith(' ') || line.startsWith('\t'))) {
        fields[currentKey] += '\n' + line.slice(1);
      }
    }
    if (!fields.Package) continue;
    fields._origin = origin;
    map.set(fields.Package, fields);
  }
  return map;
}

// ─── dependency resolution ───────────────────────────────────────────────────
// Parse a Depends string into an array of dep groups, each group being an
// array of alternatives. Each alternative is a bare package name (no version).
function parseDepends(depString) {
  if (!depString) return [];
  return depString.split(',').map(piece => {
    return piece.trim().split('|').map(alt => {
      return alt.trim().split(/[\s(]/)[0];
    }).filter(Boolean);
  }).filter(grp => grp.length > 0);
}

// Build a provides-map: virtual_name → [real_pkg_names...].
function buildProvidesMap(allPackages) {
  const provides = new Map();
  for (const [name, fields] of allPackages) {
    if (!fields.Provides) continue;
    for (const grp of parseDepends(fields.Provides)) {
      for (const virtName of grp) {
        if (!provides.has(virtName)) provides.set(virtName, []);
        provides.get(virtName).push(name);
      }
    }
  }
  return provides;
}

// Resolve a dep group (with alternatives) to a real package name, or null.
function resolveDep(group, all, provides) {
  for (const alt of group) {
    if (all.has(alt)) return alt;
    const realVia = provides.get(alt);
    if (realVia && realVia.length > 0) return realVia[0];
  }
  return null;
}

// Transitive Depends closure of `rootPkg`. Excludes the root itself.
function closure(rootPkg, all, provides) {
  const seen = new Set();
  const stack = [rootPkg];
  while (stack.length) {
    const cur = stack.pop();
    if (seen.has(cur)) continue;
    seen.add(cur);
    const fields = all.get(cur);
    if (!fields) continue;
    for (const grp of parseDepends(fields.Depends || '')) {
      const resolved = resolveDep(grp, all, provides);
      if (resolved && !seen.has(resolved)) stack.push(resolved);
    }
    for (const grp of parseDepends(fields['Pre-Depends'] || '')) {
      const resolved = resolveDep(grp, all, provides);
      if (resolved && !seen.has(resolved)) stack.push(resolved);
    }
  }
  seen.delete(rootPkg);
  return seen;
}

// "Display closure" — direct Depends of `rootPkg`, expanded one more level
// for any dep that's itself a foundry/worldfoundry metapackage. Stops at
// Ubuntu packages so the table stays user-facing (no transitive libc6 etc.).
function displayClosure(rootPkg, all, provides) {
  const out = new Set();
  const root = all.get(rootPkg);
  if (!root) return out;
  for (const grp of parseDepends(root.Depends || '')) {
    const resolved = resolveDep(grp, all, provides);
    if (!resolved) continue;
    const f = all.get(resolved);
    if (!f) continue;
    // If the dep is itself a metapackage we own, expand one more level.
    if ((f._origin === 'foundry' || f._origin === 'worldfoundry') &&
        (f.Section === 'metapackages')) {
      for (const subGrp of parseDepends(f.Depends || '')) {
        const subResolved = resolveDep(subGrp, all, provides);
        if (subResolved) out.add(subResolved);
      }
    } else {
      out.add(resolved);
    }
  }
  return out;
}

// ─── per-package record ──────────────────────────────────────────────────────
function summary(description) {
  if (!description) return '';
  // apt Description: "<one-line>\n <body>". Take line 1, trim.
  return description.split('\n')[0].trim();
}

function defaultUpstreamFor(origin) {
  switch (origin) {
    case 'foundry':            return { status: 'vendored' };
    case 'worldfoundry':       return { status: 'n/a-firstparty' };
    case 'ubuntu-universe':    return { status: 'in-ubuntu-universe' };
    case 'ubuntu-multiverse':  return { status: 'in-ubuntu-multiverse' };
    case 'ubuntu-main':        return { status: 'in-ubuntu-main' };
    default:                   return { status: 'unknown' };
  }
}

function buildPackageRecord(name, all, upstreamYaml) {
  const f = all.get(name);
  if (!f) return null;
  let upstream;
  if (upstreamYaml[name]) {
    upstream = { ...upstreamYaml[name] };
  } else {
    upstream = defaultUpstreamFor(f._origin);
    if ((f._origin === 'foundry' || f._origin === 'worldfoundry') &&
        upstream.status === 'vendored') {
      console.error(`! ${name}: no upstream.yml entry; defaulting to vendored`);
    }
  }
  const meta = (f._origin === 'foundry' || f._origin === 'worldfoundry') ? loadMeta(name) : null;
  return {
    name,
    origin: f._origin,
    section: f.Section || '',
    version: f.Version || '',
    installed_size_kb: Number(f['Installed-Size'] || 0),
    summary: summary(f.Description || ''),
    upstream,
    ...(meta?.changelog_latest ? { changelog_latest: meta.changelog_latest } : {}),
    ...(meta?.repology_project  ? { repology_project:  meta.repology_project  } : {}),
  };
}

// ─── main ────────────────────────────────────────────────────────────────────
async function main() {
  console.error('· fetching Packages.gz from 5 archives (parallel)');
  const fetched = await Promise.all(ARCHIVES.map(fetchPackages));

  // Merge with precedence: archives later in the array DO NOT override earlier
  // (foundry wins over ubuntu-universe for a same-named pkg). We iterate
  // last→first and `set()` so the FIRST archive (foundry) wins.
  const all = new Map();
  for (let i = ARCHIVES.length - 1; i >= 0; i--) {
    for (const [k, v] of fetched[i]) all.set(k, v);
  }
  const provides = buildProvidesMap(all);
  console.error(`  · merged ${all.size} packages, ${provides.size} virtual provides`);

  const releaseShas = {
    foundry:       await fetchReleaseSha(ARCHIVES[0].release),
    worldfoundry:  await fetchReleaseSha(ARCHIVES[1].release),
  };

  const categoriesDoc = JSON.parse(fs.readFileSync(CATEGORIES_PATH, 'utf8'));
  const upstreamYaml  = parseUpstreamYaml(fs.readFileSync(UPSTREAM_PATH, 'utf8'));

  // Editions — full transitive closure for accurate size/count
  console.error('· resolving editions');
  const editions = [];
  for (const ed of categoriesDoc.editions) {
    const cl = closure(ed.metapackage, all, provides);
    let totalKb = 0;
    for (const dep of cl) {
      const f = all.get(dep);
      if (f && f['Installed-Size']) totalKb += Number(f['Installed-Size']);
    }
    const root = all.get(ed.metapackage);
    const directDepends = root ? parseDepends(root.Depends || '')
      .map(g => resolveDep(g, all, provides))
      .filter(Boolean) : [];
    editions.push({
      slug: ed.slug,
      title: ed.title,
      blurb: ed.blurb,
      install_command: ed.install_command,
      metapackage: ed.metapackage,
      installed_size_kb: totalKb,
      package_count: cl.size,
      direct_depends: directDepends,
    });
    console.error(`  · ${ed.slug.padEnd(8)}  ${cl.size.toString().padStart(4)} pkgs  ${formatSize(totalKb)}`);
  }

  // Categories — display-closure (direct + 1 level through our metapackages)
  console.error('· resolving categories');
  // Pre-seed allReached with every metapackage we own — they're the entry
  // points of the catalogue, not orphan packages to flag as unowned.
  const allReached = new Set([
    ...categoriesDoc.editions.map(e => e.metapackage),
    ...categoriesDoc.categories.flatMap(c => c.metapackages),
  ]);
  const categories = [];
  for (const cat of categoriesDoc.categories) {
    const reached = new Set();
    for (const mp of cat.metapackages) {
      for (const d of displayClosure(mp, all, provides)) reached.add(d);
    }
    const packages = [];
    for (const name of [...reached].sort()) {
      const rec = buildPackageRecord(name, all, upstreamYaml);
      if (rec) {
        packages.push(rec);
        allReached.add(name);
      }
    }
    const total = packages.reduce((a, p) => a + p.installed_size_kb, 0);
    categories.push({
      slug: cat.slug,
      title: cat.title,
      role: cat.role,
      blurb: cat.blurb,
      icon: cat.icon,
      metapackages: cat.metapackages,
      in_edition_tier: cat.in_edition_tier,
      installed_size_kb: total,
      package_count: packages.length,
      packages,
    });
    console.error(`  · ${cat.slug.padEnd(20)}  ${packages.length.toString().padStart(4)} pkgs  ${formatSize(total)}`);
  }

  // CVE live counts — Ubuntu Security API, ubuntu-origin packages only.
  // Set SKIP_CVE=1 to bypass for fast local rebuilds.
  if (process.env.SKIP_CVE !== '1') {
    const ubuntuPkgs = new Set();
    for (const cat of categories) {
      for (const p of cat.packages) {
        if (p.origin?.startsWith('ubuntu-')) ubuntuPkgs.add(p.name);
      }
    }
    console.error(`· fetching CVE counts for ${ubuntuPkgs.size} ubuntu-origin packages`);
    const cveCounts = new Map();
    for (const pkgName of ubuntuPkgs) {
      try {
        const url = `https://ubuntu.com/security/cves.json?package=${encodeURIComponent(pkgName)}&limit=1`;
        const res = await fetch(url);
        if (res.ok) {
          const data = await res.json();
          cveCounts.set(pkgName, data.total_results ?? 0);
        }
      } catch { /* degrade silently — no badge if API unreachable */ }
      await new Promise(r => setTimeout(r, 80)); // ~12 req/s
    }
    for (const cat of categories) {
      for (const p of cat.packages) {
        if (cveCounts.has(p.name)) p.cve_count = cveCounts.get(p.name);
      }
    }
    console.error(`  · done (${cveCounts.size}/${ubuntuPkgs.size} packages answered)`);
  }

  // Vendored standalones — every leaf package in apt.foundrylinux.org
  // (origin === 'foundry'). These are the packages we maintain ourselves,
  // either because Ubuntu doesn't ship them (vendored) or because they're
  // first-party WF tooling that bridges both repos (blender-asset-finder).
  // WF CLIs in apt.worldfoundry.org are the WorldFoundry GDK proper and
  // belong in that category's table, not here.
  console.error('· surfacing vendored standalones');
  const vendoredStandalones = [];
  for (const name of [...allReached].sort()) {
    const rec = buildPackageRecord(name, all, upstreamYaml);
    if (!rec) continue;
    if (rec.origin !== 'foundry') continue;
    if (rec.section === 'metapackages') continue;
    const pulledBy = [];
    for (const c of categories) {
      if (c.packages.some(p => p.name === name)) pulledBy.push(c.metapackages[0]);
    }
    vendoredStandalones.push({
      name,
      pulled_by: [...new Set(pulledBy)],
      upstream: rec.upstream,
      ...(rec.repology_project ? { repology_project: rec.repology_project } : {}),
    });
  }

  // Upstream summary — count unique packages across categories
  const upstreamSummary = {
    'vendored': 0, 'debian-itp': 0, 'in-debian-unstable': 0,
    'in-ubuntu-universe': 0, 'in-ubuntu-multiverse': 0, 'in-ubuntu-main': 0,
    'n/a-firstparty': 0, 'n/a-proprietary': 0, 'unknown': 0,
  };
  const counted = new Set();
  for (const c of categories) {
    for (const p of c.packages) {
      if (counted.has(p.name)) continue;
      counted.add(p.name);
      upstreamSummary[p.upstream.status] = (upstreamSummary[p.upstream.status] || 0) + 1;
    }
  }

  // Audit
  const declaredMetapackages = [
    ...categoriesDoc.editions.map(e => e.metapackage),
    ...categoriesDoc.categories.flatMap(c => c.metapackages),
  ];
  const missingMetapackages = declaredMetapackages.filter(mp => !all.has(mp));

  // Unowned: anything in atelier's full closure that is foundry/worldfoundry
  // origin (we own it) but isn't reached by any category and isn't in the
  // out_of_catalogue list. Don't flag ubuntu transitive deps.
  const atelierClosure = closure('foundry-atelier', all, provides);
  const unowned = [];
  for (const name of atelierClosure) {
    if (allReached.has(name)) continue;
    if (categoriesDoc.out_of_catalogue.includes(name)) continue;
    const f = all.get(name);
    if (!f) continue;
    if (f._origin !== 'foundry' && f._origin !== 'worldfoundry') continue;
    unowned.push(name);
  }

  const grandTotal = editions.length > 0
    ? editions[editions.length - 1].installed_size_kb
    : 0;

  const out = {
    generated_at: new Date().toISOString().replace(/\.\d{3}Z$/, 'Z'),
    foundry_release_sha: releaseShas.foundry,
    worldfoundry_release_sha: releaseShas.worldfoundry,
    inputs_sha: process.env.INPUTS_SHA || null,
    editions,
    categories,
    vendored_standalones: vendoredStandalones,
    grand_total_installed_size_kb: grandTotal,
    upstream_summary: upstreamSummary,
    audit: {
      unowned_packages: unowned.sort(),
      missing_metapackages: missingMetapackages.sort(),
      out_of_catalogue: categoriesDoc.out_of_catalogue,
    },
  };

  fs.mkdirSync(path.dirname(OUT_PATH), { recursive: true });
  fs.writeFileSync(OUT_PATH, JSON.stringify(out, null, 2) + '\n');
  console.error(`✓ wrote ${OUT_PATH}`);
  console.error(`  grand total: ${formatSize(grandTotal)}`);
  console.error(`  upstream summary: ${JSON.stringify(upstreamSummary)}`);
  console.error(`  audit: unowned=${unowned.length} missing-metapackages=${missingMetapackages.length}`);

  if (missingMetapackages.length > 0) {
    console.error(`✗ missing metapackages — build FAIL: ${missingMetapackages.join(' ')}`);
    process.exit(2);
  }
  if (unowned.length > 0) {
    console.error(`✗ unowned packages — build FAIL: ${unowned.join(' ')}`);
    process.exit(3);
  }
}

function formatSize(kb) {
  if (kb >= 1024 * 1024) return (kb / 1024 / 1024).toFixed(2) + ' GiB';
  if (kb >= 1024)        return (kb / 1024).toFixed(0) + ' MiB';
  return kb + ' KiB';
}

main().catch(e => {
  console.error(`✗ ${e.stack || e.message}`);
  process.exit(1);
});
