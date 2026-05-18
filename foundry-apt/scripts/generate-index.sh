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

# Parse each packages/<name>/debian/ into parallel arrays (bash 3 compat).
#
# Canonical layout only: packages/<name>/debian/{control,changelog}
# (Debian source-package format). Homepage is in the Source: stanza;
# version comes from the first changelog entry; one row per Package:
# stanza. Packages without a debian/ tree (e.g. legacy xa65 with
# DEBIAN/control only) still build via packages/<name>/build.sh but
# do not appear on this landing page.
PKG_NAMES=()
PKG_VERSIONS=()
PKG_DESCS=()
PKG_HOMEPAGES=()
PKG_ARCHS=()

# Source-format parser: emits one TSV line per binary stanza:
#   pkg<TAB>arch<TAB>desc_short
# Skips the Source: stanza (the first one). Field continuations (lines
# starting with space) on Description are ignored — we only emit the
# short summary line.
parse_source_control_binaries() {
  awk -v RS='' -v ORS='\n' '
    NR == 1 { next }     # First stanza is "Source:" — skip
    {
      pkg=""; arch=""; desc=""
      n = split($0, lines, "\n")
      for (i = 1; i <= n; i++) {
        if      (lines[i] ~ /^Package:/)      { sub(/^Package: */,      "", lines[i]); pkg  = lines[i] }
        else if (lines[i] ~ /^Architecture:/) { sub(/^Architecture: */, "", lines[i]); arch = lines[i] }
        else if (lines[i] ~ /^Description:/)  { sub(/^Description: */,  "", lines[i]); desc = lines[i] }
      }
      if (pkg != "") print pkg "\t" arch "\t" desc
    }
  ' "$1"
}

for pkgdir in "$REPO_ROOT"/packages/*/; do
  [[ -d "$pkgdir" ]] || continue
  src_control="${pkgdir}debian/control"
  src_changelog="${pkgdir}debian/changelog"

  [[ -f "$src_control" && -f "$src_changelog" ]] || continue

  homepage=$(awk '/^Homepage:/ {sub(/^Homepage: */,""); print; exit}' "$src_control" || true)
  ver=$(awk 'NR==1 {if (match($0, /\(([^)]+)\)/, a)) print a[1]; exit}' "$src_changelog")
  if [[ -z "$ver" ]]; then
    echo "WARNING: $src_changelog missing version on line 1 — skipping $pkgdir" >&2
    continue
  fi
  while IFS=$'\t' read -r pkg arch desc; do
    [[ -n "$pkg" ]] || continue
    PKG_NAMES+=("$pkg")
    PKG_VERSIONS+=("$ver")
    PKG_DESCS+=("$desc")
    PKG_HOMEPAGES+=("${homepage:-}")
    PKG_ARCHS+=("${arch:-all}")
  done < <(parse_source_control_binaries "$src_control")
done

# Build the package table rows
PKG_ROWS=""
for i in "${!PKG_NAMES[@]}"; do
  name="${PKG_NAMES[$i]}"
  ver="${PKG_VERSIONS[$i]}"
  desc="${PKG_DESCS[$i]}"
  hp="${PKG_HOMEPAGES[$i]}"
  arch="${PKG_ARCHS[$i]}"
  if [[ -n "$hp" ]]; then
    name_cell="<a href=\"${hp}\">${name}</a>"
  else
    name_cell="$name"
  fi
  L="${name:0:1}"
  if [[ "$arch" == "all" ]]; then
    deb_url="/pool/main/${L}/${name}/${name}_${ver}_all.deb"
    ver_cell="<a href=\"${deb_url}\">${ver}</a>"
  else
    # Architecture may list multiple tokens (e.g. "amd64 arm64"). One sub-link per arch.
    arch_links=""
    for a in $arch; do
      [[ -n "$arch_links" ]] && arch_links="${arch_links} "
      arch_links="${arch_links}<a href=\"/pool/main/${L}/${name}/${name}_${ver}_${a}.deb\">${a}</a>"
    done
    ver_cell="${ver} (${arch_links})"
  fi
  PKG_ROWS="${PKG_ROWS}
      <tr><td>${name_cell}</td><td>${ver_cell}</td><td>${desc}</td></tr>"
done

cat > "$OUT" <<HTML
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>${SITE_TITLE}</title>
<style>
  :root {
    --bg:      #0d1117;
    --surface: #161b22;
    --border:  #30363d;
    --fg:      #e6edf3;
    --dim:     #8b949e;
    --green:   #3fb950;
    --blue:    #58a6ff;
    --mono:    "SF Mono", "Fira Code", "Cascadia Code", Consolas, monospace;
  }
  * { box-sizing: border-box; margin: 0; padding: 0; }
  body {
    background: var(--bg);
    color: var(--fg);
    font-family: var(--mono);
    font-size: 14px;
    line-height: 1.6;
    padding: 2rem 1rem;
  }
  a { color: var(--blue); text-decoration: none; }
  a:hover { text-decoration: underline; }
  .wrap { max-width: 860px; margin: 0 auto; }
  h1 { font-size: 1.4rem; color: var(--green); margin-bottom: .25rem; }
  .subtitle { color: var(--dim); margin-bottom: 2rem; }
  h2 { font-size: 1rem; color: var(--dim); text-transform: uppercase;
       letter-spacing: .08em; margin: 2rem 0 .75rem; }
  pre {
    background: var(--surface);
    border: 1px solid var(--border);
    border-radius: 6px;
    padding: 1rem 1.25rem;
    overflow-x: auto;
    white-space: pre;
  }
  table {
    width: 100%;
    border-collapse: collapse;
    background: var(--surface);
    border: 1px solid var(--border);
    border-radius: 6px;
    overflow: hidden;
  }
  th {
    background: var(--border);
    color: var(--dim);
    font-size: .8rem;
    text-transform: uppercase;
    letter-spacing: .06em;
    padding: .5rem .75rem;
    text-align: left;
  }
  td { padding: .5rem .75rem; border-top: 1px solid var(--border); }
  td:nth-child(1) { white-space: nowrap; }
  td:nth-child(2) { color: var(--dim); white-space: nowrap; }
  footer {
    margin-top: 3rem;
    color: var(--dim);
    font-size: .8rem;
    border-top: 1px solid var(--border);
    padding-top: 1rem;
  }
</style>
</head>
<body>
<div class="wrap">
  <h1>${SITE_TITLE}</h1>
  <p class="subtitle">${SITE_URL} &mdash; signed APT packages for Ubuntu 26.04 (resolute)</p>

  <h2>Quick install</h2>
  <pre>curl -fsSL ${SITE_URL}/key.gpg \\
  | gpg --dearmor -o /etc/apt/keyrings/foundry.gpg
echo "deb [signed-by=/etc/apt/keyrings/foundry.gpg] ${SITE_URL} resolute main" \\
  | sudo tee /etc/apt/sources.list.d/foundry.list
sudo apt-get update
sudo apt-get install worldfoundry-dev</pre>

  <h2>GPG key</h2>
  <p><a href="/key.gpg">↓ key.gpg</a> &mdash; verify before trusting:
     <code>gpg --show-keys /etc/apt/keyrings/foundry.gpg</code></p>

  <h2>Packages</h2>
  <table>
    <thead><tr><th>Package</th><th>Version</th><th>Description</th></tr></thead>
    <tbody>${PKG_ROWS}
    </tbody>
  </table>

  <h2>Source</h2>
  <p><a href="${GITHUB_URL}">${GITHUB_URL}</a></p>

  <footer>Published ${PUBLISHED}</footer>
</div>
</body>
</html>
HTML

echo "Generated $OUT"
