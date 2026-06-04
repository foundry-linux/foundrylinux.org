# /packages page polish — items 2–5

## Context

Five TODO items were identified for the /packages page. After exploration, **item 1 (copy-to-clipboard) is already done** — `packages.jsx` EditionsLadder (line 62) and CategorySection (line 91) both have `inline-copy-btn` buttons, and `generate-index.sh` has the quick-install copy button (line 394). It should be marked `[x]`.

Items 2–5 are genuine gaps:

- Item 2: edition dep lists visible but not expandable
- Items 3 & 4: changelog + Repology data already exist in `foundry-apt/public/meta/*.json` (32 files, generated locally, not tracked in git) but aren't surfaced on the site
- Item 5: CVE links are pure render additions requiring no data changes

`apt.foundrylinux.org` already has changelog tooltips (`.ver-wrap`/`.ver-tip` CSS hover) and Repology badges — items 3 & 4 on `/packages` are the only remaining gaps.

---

## Item 1 — Already done

Mark `[ ] Copy-to-clipboard buttons` → `[x]` in TODO.md. No code change needed.

---

## Item 2 — Metapackage dep expansion

**File:** `site/packages.jsx` — `EditionsLadder` component

Data already present: `ed.direct_depends` (array of strings, e.g. `["foundry-core", "foundry-desktop"]`).

Add a `<details class="edition-deps">` block inside each `.edition` card, below `.edition-pkg`:

```jsx
{ed.direct_depends?.length > 0 && (
  <details className="edition-deps">
    <summary>direct deps ({ed.direct_depends.length})</summary>
    <div className="edition-deps-list">
      {ed.direct_depends.map(d => <code key={d}>{d}</code>)}
    </div>
  </details>
)}
```

**CSS** (`site/styles.css`): add `.edition-deps` summary cursor + small text, `.edition-deps-list code` spaced chips — match existing `.dep` chip style from apt index.

---

## Item 3 & 4 — Changelog + Repology in packages-data.json

### 3a. Data pipeline — `scripts/build-packages-data.js`

In `buildPackageRecord()` (around line 208), after building the base record, optionally merge from the local meta file:

```js
const META_DIR = path.join(ROOT, 'foundry-apt/public/meta');

function loadMeta(name) {
  const p = path.join(META_DIR, `${name}.json`);
  try { return JSON.parse(fs.readFileSync(p, 'utf8')); } catch { return null; }
}
```

Add to the returned record for `foundry`-origin packages (and `worldfoundry` if applicable):
```js
...(meta?.changelog_latest  ? { changelog_latest: meta.changelog_latest }   : {}),
...(meta?.repology_project  ? { repology_project: meta.repology_project }   : {}),
```

These fields are optional — absent for Ubuntu-origin packages. Downstream renders check for their presence before using them.

### 3b. Changelog tooltip — `site/packages.jsx` CategorySection

Add a version column to the `<table>` and render a tooltip for packages that have `changelog_latest`:

```jsx
<th className="num">Version</th>
// ...
<td className="num pkg-ver">
  {p.version}
  {p.changelog_latest && (
    <span className="ver-wrap">
      <span className="ver-tip">{p.changelog_latest}</span>
    </span>
  )}
</td>
```

**CSS**: port `.ver-wrap` / `.ver-tip` tooltip styles from `foundry-apt/scripts/generate-index.sh` (inline CSS lines ~298–306) into `site/styles.css`.

### 4b. Repology badge — `site/packages.jsx`

In `CategorySection` table, for packages with `repology_project` add the badge after the version:

```jsx
{p.repology_project && (
  <a className="repology-badge"
     href={`https://repology.org/project/${p.repology_project}/versions`}
     target="_blank" rel="noopener" aria-label="Repology versions">
    <img src={`https://repology.org/badge/latest-versions/${p.repology_project}.svg`}
         alt="latest versions" height="14" />
  </a>
)}
```

Also add to `VendoredStandalones` list items (need `repology_project` on those records too — same meta-file lookup applies since they're all `foundry`-origin).

**CSS**: `.repology-badge` — port from `generate-index.sh` inline CSS (line ~308–310).

---

## Item 5 — CVE tracker links

**No data changes needed** — uses `p.name` and `p.origin` already in packages-data.json.

### `site/packages.jsx` — CategorySection table

Add a "CVE" column (or icon in the Package column) using a shield SVG:

```jsx
<th>CVE</th>
// ...
<td>
  {(['ubuntu-main','ubuntu-universe','ubuntu-multiverse'].includes(p.origin)) && (
    <a className="cve-link"
       href={`https://ubuntu.com/security/cves?package=${p.name}`}
       target="_blank" rel="noopener" aria-label="CVE tracker">
      <ShieldIcon />
    </a>
  )}
</td>
```

For `foundry`/`worldfoundry` origin: omit (no Ubuntu CVE page for our packages).

Add `ShieldIcon` to `site/icons.jsx` (simple shield outline SVG, ~16×16).

### `foundry-apt/scripts/generate-index.sh`

Add a CVE column to the package table. All packages in the foundry apt repo are either:
- vendored upstreams (have an Ubuntu equivalent name in many cases) — link anyway
- first-party foundry packages — no CVE page

Add conditionally: only link if `repology_project` is empty (i.e. first-party) — skip; otherwise link. Or simpler: always emit the link (ubuntu.com returns no results gracefully for unknown packages).

---

## Files to modify

| File | Change |
|---|---|
| `site/packages.jsx` | Items 2, 3b, 4b, 5 — dep toggle, version col, changelog tooltip, Repology badge, CVE link |
| `site/styles.css` | CSS for `.edition-deps`, `.ver-wrap`/`.ver-tip`, `.repology-badge`, `.cve-link` |
| `site/icons.jsx` | Add `ShieldIcon` |
| `scripts/build-packages-data.js` | Items 3a/4a — read meta/*.json to enrich foundry-origin package records |
| `foundry-apt/scripts/generate-index.sh` | Item 5 — add CVE link column |
| `TODO.md` | Mark item 1 `[x]`, move to Done |

---

## Verification

1. `task packages-page` — regenerates `site/packages-data.json`; confirm vendored packages (f9dasm, vgmstream, ghidra) now have `changelog_latest` and `repology_project` fields.
2. `task site-build` — SSR produces `site/packages.html`.
3. Open `http://localhost:7799/packages.html`:
   - Edition card: click "direct deps" expands dep chip list.
   - Retro-tools category table: f9dasm row shows version, changelog tooltip on hover, Repology badge.
   - Ubuntu-origin row (e.g. blender): CVE shield icon links to ubuntu.com/security/cves?package=blender.
   - No CVE icon on foundry-origin rows.
4. Open `http://localhost:7799` (apt index): CVE link present per package row.
5. `task verify` / `task shellcheck` pass.
