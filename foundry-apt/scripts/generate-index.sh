#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
OUT_DIR="${PUBLISH_DIR:-$REPO_ROOT/public}"
OUT="$OUT_DIR/index.html"

SITE_TITLE="Foundry APT Repository"
SITE_URL="https://apt.foundrylinux.org"
GITHUB_URL="https://github.com/foundry-linux/foundry-apt"
PUBLISHED="$(date -u +%Y-%m-%dT%H:%M:%SZ)"

# Ensure meta files exist for all packages (fallback for local preview
# without a prior 'task generate-meta' run).
mkdir -p "$OUT_DIR/meta"
for pkgdir in "$REPO_ROOT"/packages/*/; do
  name="$(basename "$pkgdir")"
  [[ -f "$pkgdir/debian/control" && -f "$pkgdir/debian/changelog" ]] || continue
  if [[ ! -f "$OUT_DIR/meta/${name}.json" ]]; then
    bash "$SCRIPT_DIR/generate-meta.sh" "$name"
  fi
done

# Read all public/meta/*.json → emit one <tr> per package (alphabetical).
# Python handles HTML escaping, arch resolution, and the <details> block.
PKG_ROWS=$(python3 - "$REPO_ROOT" "$OUT_DIR" <<'PYEOF'
import glob, html, json, os, re, sys

repo_root, out_dir = sys.argv[1], sys.argv[2]
meta_dir = os.path.join(out_dir, "meta")
dist_dir = os.path.join(repo_root, "dist")

def esc(s):
    return html.escape(str(s or ""), quote=True)

def size_human(kb):
    if kb is None:
        return None
    return f"~{round(kb / 1024)} MB" if kb >= 1024 else f"~{kb} KB"

def fmt_desc(text):
    """Escape HTML then convert `backtick` spans to <code> elements."""
    escaped = esc(text)
    return re.sub(r'`([^`]+)`', lambda m: f'<code>{m.group(1)}</code>', escaped)

icon_3  = '<svg viewBox="0 0 16 16" width="12" height="12" fill="currentColor"><path d="M8 1.5 L1 8.5 L3 8.5 L3 14.5 L7 14.5 L7 10.5 L9 10.5 L9 14.5 L13 14.5 L13 8.5 L15 8.5 Z"/></svg>'
icon_10 = '<svg viewBox="0 0 16 16" width="12" height="12" fill="none" stroke="currentColor" stroke-width="1.4" stroke-linecap="round" stroke-linejoin="round"><path d="M8 2 L14 8 L12 8 L12 14 L4 14 L4 8 L2 8 Z"/></svg>'

for i, fname in enumerate(sorted(os.listdir(meta_dir))):
    if not fname.endswith(".json"):
        continue
    with open(os.path.join(meta_dir, fname)) as f:
        p = json.load(f)

    name       = p["name"]
    ver        = p["version"]
    arch       = p["architecture"]
    hp         = p.get("homepage", "")
    desc_short = p.get("description_short", "")
    desc_long  = (p.get("description_long") or "").strip()
    depends    = p.get("depends") or []
    inst_kb    = p.get("installed_size_kb")
    changelog  = p.get("changelog_latest", "")
    repology   = p.get("repology_project", "")

    # Name cell — bold plain text only; meta badge goes on the version line
    section    = p.get("section", "")
    meta_badge = '<span class="pkg-meta">meta</span> ' if section == "metapackages" else ''
    name_cell = f'<b class="pkg-name">{esc(name)}</b>'
    # Home button goes at the start of description cell — alternate icons for preview
    home_svg = icon_3 if i % 2 == 0 else icon_10
    home_btn = f'<a class="pkg-home" href="{esc(hp)}" title="{esc(hp)}" aria-label="Homepage">{home_svg}</a> ' if hp else ''

    # Version cell with .deb download link(s)
    letter = name[0]
    if arch == "all":
        deb_url  = f"/pool/main/{letter}/{name}/{name}_{ver}_all.deb"
        ver_cell = f'<a href="{deb_url}">{esc(ver)}</a>'
    else:
        if arch == "any":
            built = [
                os.path.basename(f)[len(f"{name}_{ver}_"):-4]
                for f in sorted(glob.glob(
                    os.path.join(dist_dir, f"{name}_{ver}_*.deb")
                ))
            ]
            arches = built if built else ["amd64"]
        else:
            arches = arch.split()
        links = " ".join(
            f'<a href="/pool/main/{letter}/{name}/{name}_{ver}_{a}.deb">{a}</a>'
            for a in arches
        )
        ver_cell = f"{esc(ver)} ({links})"

    # Wrap ver_cell with changelog hover tooltip
    if changelog:
        ver_cell = f'<span class="ver-wrap">{ver_cell}<span class="ver-tip">{esc(changelog)}</span></span>'

    # Repology badge for vendored packages
    if repology:
        badge_url  = f"https://repology.org/badge/latest-versions/{esc(repology)}.svg"
        badge_href = f"https://repology.org/project/{esc(repology)}/versions"
        ver_cell  += (f'<a class="repology-badge" href="{badge_href}" target="_blank"'
                      f' rel="noopener" aria-label="Repology versions">'
                      f'<img src="{badge_url}" alt="latest versions" height="14"></a>')

    # Description cell with optional <details> for long desc + dep chips
    if desc_long or depends or inst_kb is not None:
        parts = []
        if desc_long:
            for para in re.split(r'\n\n+', desc_long.strip()):
                lines = [line.strip() for line in para.splitlines() if line.strip()]
                if not lines:
                    continue
                if all(l.startswith('* ') for l in lines):
                    items = ''.join(
                        f'<li>{fmt_desc(re.sub(r"  +", " ", l[2:]))}</li>' for l in lines
                    )
                    parts.append(f'<ul class="pkg-long">{items}</ul>')
                else:
                    parts.append(f'<p class="pkg-long">{fmt_desc(" ".join(lines))}</p>')
        if depends:
            chips = "".join(f'<span class="dep">{esc(d)}</span>' for d in depends)
            parts.append(f'<div class="pkg-deps">{chips}</div>')
        if inst_kb is not None:
            parts.append(f'<p class="pkg-size">{esc(size_human(inst_kb))} installed</p>')
        details = (
            '<details class="pkg-details"><summary>details</summary>'
            + "".join(parts)
            + "</details>"
        )
        desc_cell = f"{home_btn}{esc(desc_short)} {details}"
    else:
        desc_cell = f"{home_btn}{esc(desc_short)}"

    cve_link = (
        f'<a class="cve-link" href="https://ubuntu.com/security/cves?package={esc(name)}"'
        f' target="_blank" rel="noopener" aria-label="CVE tracker" title="Ubuntu CVE tracker">'
        f'<svg viewBox="0 0 16 16" width="12" height="12" fill="none" stroke="currentColor"'
        f' stroke-width="1.4" stroke-linecap="round" stroke-linejoin="round">'
        f'<path d="M8 2 L13 4 L13 8.5 C13 11.5 8 14 8 14 C8 14 3 11.5 3 8.5 L3 4 Z"/>'
        f'</svg></a>'
        if repology else ''
    )
    print(
        f'<tr data-name="{esc(name)}" data-ver="{esc(ver)}" data-desc="{esc(desc_short)}">'
        f'<td class="col-pkg">{name_cell}{cve_link}<span class="col-ver">{meta_badge}{ver_cell}</span></td>'
        f'<td class="col-desc">{desc_cell}</td>'
        f'</tr>'
    )
PYEOF
)

PKG_COUNT=$(grep -c '^<tr' <<< "$PKG_ROWS" || true)

# Copy tracked static assets (favicon, index.js) into the publish dir.
if [[ -d "$REPO_ROOT/gen/static" ]]; then
  cp -a "$REPO_ROOT/gen/static/." "$OUT_DIR/"
fi

cat > "$OUT" <<HTML
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>${SITE_TITLE}</title>
<meta property="og:type"        content="website" />
<meta property="og:url"         content="${SITE_URL}/" />
<meta property="og:title"       content="${SITE_TITLE}" />
<meta property="og:description" content="${PKG_COUNT} signed packages for Ubuntu 26.04 (resolute)" />
<meta name="twitter:card"       content="summary" />
<link rel="icon" type="image/svg+xml" href="/favicon.svg" />
<link rel="preconnect" href="https://fonts.googleapis.com" />
<link rel="preconnect" href="https://fonts.gstatic.com" crossorigin />
<link rel="stylesheet" href="https://fonts.googleapis.com/css2?family=Space+Grotesk:wght@400;500;600&family=JetBrains+Mono:wght@400;500&display=swap" />
<link rel="stylesheet" href="https://foundrylinux.org/styles.css" />
<style>
  /* apt-index layout — colour/font variables from foundrylinux.org/styles.css */
  * { box-sizing: border-box; margin: 0; padding: 0; }
  html { scrollbar-gutter: stable; }
  body { background: var(--bg); color: var(--ink); }
  a { color: var(--accent); text-decoration: none; }
  a:hover { text-decoration: underline; }
  .wrap { max-width: 1080px; margin: 0 auto; padding: 2rem 1rem; }
  h1 {
    font-family: var(--font-mono);
    font-size: 11px;
    letter-spacing: 0.25em;
    text-transform: uppercase;
    color: var(--ink-faint);
    margin-bottom: .5rem;
  }
  .site-title {
    font-family: var(--font-wordmark);
    font-size: 2.2rem;
    font-weight: 900;
    text-transform: uppercase;
    letter-spacing: -0.01em;
    color: var(--ink);
    margin-bottom: .25rem;
  }
  .subtitle { color: var(--ink-soft); margin-bottom: 2rem; font-size: 14px; }
  .back-nav {
    display: flex; align-items: center; gap: 6px;
    font-family: var(--font-mono); font-size: 10.5px; letter-spacing: 0.12em;
    text-transform: uppercase; color: var(--ink-faint);
    margin-bottom: 1.5rem;
  }
  .back-nav a { color: var(--ink-faint); }
  .back-nav a:hover { color: var(--accent); text-decoration: none; }
  .site-title a { color: inherit; text-decoration: none; }
  .site-title a:hover { color: var(--accent); }
  h2 {
    font-family: var(--font-mono);
    font-size: 10.5px;
    color: var(--ink-faint);
    text-transform: uppercase;
    letter-spacing: 0.2em;
    margin: 2.5rem 0 .75rem;
  }
  .pre-wrap { position: relative; }
  .pre-wrap .copy {
    position: absolute; top: .5rem; right: .5rem;
    font-family: var(--font-mono); font-size: 10px; letter-spacing: .08em;
    text-transform: uppercase; padding: .2rem .5rem;
    background: rgba(255,255,255,0.06); border: 1px solid var(--hairline-strong);
    color: var(--ink-faint); cursor: pointer; border-radius: 2px;
  }
  .pre-wrap .copy:hover { color: var(--ink); border-color: var(--accent); }
  .pre-wrap .copy[data-copied] { color: var(--accent); }
  pre {
    background: rgba(255,255,255,0.015);
    border: 1px solid var(--hairline-strong);
    padding: 1rem 1.25rem;
    overflow-x: auto;
    font-family: var(--font-mono);
    font-size: 13px;
    line-height: 1.7;
    color: var(--ink);
  }
  .filter-bar {
    display: flex; align-items: center; gap: .5rem;
    margin-bottom: .5rem;
  }
  .filter-bar label {
    font-family: var(--font-mono); font-size: 10px;
    letter-spacing: .2em; text-transform: uppercase; color: var(--ink-faint);
    white-space: nowrap;
  }
  #filter-q {
    flex: 1; background: rgba(255,255,255,0.04);
    border: 1px solid var(--hairline-strong); color: var(--ink);
    font-family: var(--font-mono); font-size: 12px;
    padding: .3rem .6rem; outline: none;
  }
  #filter-q:focus { border-color: var(--accent); }
  .filter-clear {
    background: none; border: none; color: var(--ink-faint);
    font-size: 14px; cursor: pointer; padding: 0 .25rem; line-height: 1;
  }
  .filter-clear:hover { color: var(--ink); }
  .listing-bar {
    display: flex; justify-content: space-between; align-items: baseline;
    font-family: var(--font-mono); font-size: 10px; letter-spacing: .15em;
    text-transform: uppercase; color: var(--ink-faint);
    margin-bottom: .25rem;
  }
  .table-wrap { border: 1px solid var(--hairline); }
  table { width: 100%; border-collapse: collapse; }
  td.col-pkg, th:first-child { width: 1%; }
  th {
    font-family: var(--font-mono);
    font-size: 10px;
    letter-spacing: 0.2em;
    text-transform: uppercase;
    color: var(--ink-faint);
    padding: .5rem .75rem;
    text-align: left;
    border-bottom: 1px solid var(--hairline-strong);
    user-select: none;
  }
  th[data-sort]:hover { color: var(--ink); }
  .sort-ind { margin-left: .3em; font-size: 9px; }
  td { padding: .6rem .75rem; border-top: 1px solid var(--hairline); font-size: 14px; vertical-align: top; }
  td.col-pkg { font-family: var(--font-mono); font-size: 13px; }
  .pkg-name { color: var(--accent); font-weight: 700; white-space: nowrap; }
  .pkg-meta {
    display: inline-block; font-family: var(--font-mono); font-size: 9px;
    letter-spacing: 0.12em; text-transform: uppercase;
    color: var(--ink-faint); border: 1px solid var(--hairline-strong);
    padding: 1px 5px; border-radius: 2px; vertical-align: middle;
    margin-right: 5px; font-weight: 400;
  }
  .pkg-home {
    display: inline-flex; align-items: center; justify-content: center;
    color: var(--ink-faint); text-decoration: none;
    margin-right: 6px; vertical-align: middle;
    border: 1px solid var(--hairline-strong);
    padding: 3px 5px; border-radius: 2px;
    transition: color 0.15s, border-color 0.15s;
  }
  .pkg-home:hover { color: var(--accent); border-color: var(--accent); text-decoration: none; }
  .col-ver { display: block; color: var(--ink-faint); font-size: 11px; margin-top: 4px; overflow-wrap: anywhere; }
  .ver-wrap { position: relative; display: inline; }
  .ver-tip {
    display: none; position: absolute; left: 0; top: calc(100% + 3px);
    z-index: 100; min-width: 260px; max-width: 420px;
    background: #1c1c1c; border: 1px solid var(--hairline-strong);
    padding: .5rem .75rem; font-size: 10.5px; line-height: 1.55;
    color: var(--ink-soft); font-family: var(--font-mono);
    white-space: pre-wrap; word-break: break-word; pointer-events: none;
  }
  .ver-wrap:hover .ver-tip { display: block; }
  .repology-badge { display: block; margin-top: 5px; }
  .repology-badge img { vertical-align: middle; opacity: 0.8; }
  .repology-badge:hover img { opacity: 1; }
  .cve-link { display: inline-flex; align-items: center; margin-left: 5px; color: var(--ink-faint); text-decoration: none; opacity: 0.5; vertical-align: middle; }
  .cve-link:hover { color: var(--accent); opacity: 1; }
  td.col-desc { word-break: break-word; }
  /* ── Package details (long desc + dep chips) ── */
  .pkg-details { margin-top: .3rem; }
  .pkg-details summary {
    cursor: pointer; font-size: 11px; color: var(--ink-faint);
    font-family: var(--font-mono); letter-spacing: .05em;
    list-style: none; display: inline;
  }
  .pkg-details summary::before { transition: color 0.15s; }
  .pkg-details summary:hover::before { color: var(--accent); }
  .pkg-details summary::marker,
  .pkg-details summary::-webkit-details-marker { display: none; }
  .pkg-details summary::before { content: "▸ "; }
  .pkg-details[open] summary::before { content: "▾ "; }
  .pkg-details[open] summary { color: var(--ink-soft); }
  .pkg-long {
    margin-top: .5rem; font-size: 11px; line-height: 1.55;
    color: var(--ink-soft); word-break: break-word;
  }
  ul.pkg-long { padding-left: 1.2em; }
  .pkg-long code {
    font-family: var(--font-mono); font-size: 10.5px;
    background: rgba(255,255,255,0.07); padding: .1em .3em;
    border-radius: 2px; color: var(--ink);
  }
  .pkg-deps {
    display: flex; flex-wrap: wrap; gap: .3rem; margin-top: .5rem;
  }
  .dep {
    font-family: var(--font-mono); font-size: 10.5px;
    border: 1px solid var(--hairline-strong);
    padding: .1rem .4rem; color: var(--accent);
    white-space: nowrap;
  }
  .pkg-size {
    margin-top: .4rem; font-size: 11px; color: var(--ink-faint);
    font-family: var(--font-mono);
  }
  footer {
    margin-top: 3rem;
    color: var(--ink-faint);
    font-family: var(--font-mono);
    font-size: 10.5px;
    letter-spacing: 0.14em;
    text-transform: uppercase;
    border-top: 1px solid var(--hairline);
    padding-top: 1rem;
  }

  /* ── Narrow screens: card layout ──────────────────────────────────── */
  @media (max-width: 639px) {
    .wrap { padding: 1.5rem 0.75rem; }
    .site-title { font-size: 1.75rem; }
    pre { white-space: pre-wrap; word-break: break-all; }
    table thead { display: none; }
    table, tbody { display: block; }
    tbody tr {
      display: grid;
      grid-template-columns: 1fr;
      grid-template-areas: "pkg" "desc";
      gap: 0.2rem;
      padding: 0.75rem 0.6rem;
      border-top: 1px solid var(--hairline);
    }
    tbody tr td { display: block; padding: 0; border: 0; white-space: normal; }
    td.col-pkg  { grid-area: pkg; }
    td.col-desc { grid-area: desc; }
  }
</style>
</head>
<body>
<div class="wrap">
  <nav class="back-nav">
    <a href="https://foundrylinux.org">← foundrylinux.org</a>
    <span>/</span>
    <span>apt</span>
  </nav>
  <h1>APT Repository</h1>
  <p class="site-title"><a href="https://foundrylinux.org">Foundry Linux</a></p>
  <p class="subtitle">${SITE_URL} &mdash; signed packages for Ubuntu 26.04 (resolute)</p>

  <h2>Quick install</h2>
  <div class="pre-wrap">
  <button class="copy" type="button" data-copy="quick-install" title="Copy"><svg viewBox="0 0 16 16" width="12" height="12" fill="none" stroke="currentColor" stroke-width="1.4" stroke-linecap="round" stroke-linejoin="round"><rect x="5" y="5" width="9" height="9"/><path d="M5 11 L2 11 L2 2 L11 2 L11 5"/></svg></button>
  <pre id="quick-install">curl -fsSL ${SITE_URL}/key.gpg \\
  | gpg --dearmor -o /etc/apt/keyrings/foundry.gpg
echo "deb [signed-by=/etc/apt/keyrings/foundry.gpg] ${SITE_URL} resolute main" \\
  | sudo tee /etc/apt/sources.list.d/foundry.list
sudo apt-get update
sudo apt-get install foundry-retro-tools</pre>
  </div>

  <h2>GPG key</h2>
  <p><a href="/key.gpg">↓ key.gpg</a> &mdash; verify before trusting:
     <code>gpg --show-keys /etc/apt/keyrings/foundry.gpg</code></p>

  <h2>Packages</h2>
  <div class="filter-bar">
    <label for="filter-q">Filter</label>
    <input id="filter-q" type="text" placeholder="name or description" autocomplete="off" spellcheck="false">
    <button class="filter-clear" type="button" aria-label="Clear filter" hidden>×</button>
  </div>
  <div class="listing-bar">
    <span>Packages</span>
    <span data-count><b>${PKG_COUNT}</b> packages</span>
  </div>
  <div class="table-wrap">
  <table class="listing-table">
    <thead><tr>
      <th data-sort="name">Package <span class="sort-ind"></span></th>
      <th data-sort="desc">Description <span class="sort-ind"></span></th>
    </tr></thead>
    <tbody>
${PKG_ROWS}
    </tbody>
  </table>
  </div>

  <h2>Source</h2>
  <p><a href="${GITHUB_URL}">${GITHUB_URL}</a></p>

  <footer>Published ${PUBLISHED}</footer>
</div>
<script src="index.js" defer></script>
</body>
</html>
HTML

echo "Generated $OUT"
