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

# Parse packages/*/DEBIAN/control into parallel arrays (bash 3 compat)
PKG_NAMES=()
PKG_VERSIONS=()
PKG_DESCS=()
PKG_HOMEPAGES=()

for control in "$REPO_ROOT"/packages/*/DEBIAN/control; do
  [[ -f "$control" ]] || continue
  pkg=$(grep '^Package:'     "$control" | sed 's/^Package: *//')
  ver=$(grep '^Version:'     "$control" | sed 's/^Version: *//')
  desc=$(grep '^Description:' "$control" | sed 's/^Description: *//')
  homepage=$(grep '^Homepage:' "$control" | sed 's/^Homepage: *//' || true)
  PKG_NAMES+=("$pkg")
  PKG_VERSIONS+=("$ver")
  PKG_DESCS+=("$desc")
  PKG_HOMEPAGES+=("${homepage:-}")
done

# Build the package table rows
PKG_ROWS=""
for i in "${!PKG_NAMES[@]}"; do
  name="${PKG_NAMES[$i]}"
  ver="${PKG_VERSIONS[$i]}"
  desc="${PKG_DESCS[$i]}"
  hp="${PKG_HOMEPAGES[$i]}"
  if [[ -n "$hp" ]]; then
    name_cell="<a href=\"${hp}\">${name}</a>"
  else
    name_cell="$name"
  fi
  PKG_ROWS="${PKG_ROWS}
      <tr><td>${name_cell}</td><td>${ver}</td><td>${desc}</td></tr>"
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
