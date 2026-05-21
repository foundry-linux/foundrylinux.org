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
  # dpkg-parsechangelog is the canonical way to read debian/changelog
  # (works with mawk; gawk's match(..., a) does not).
  if command -v dpkg-parsechangelog >/dev/null; then
    ver=$(dpkg-parsechangelog -l "$src_changelog" -SVersion)
  else
    # Fallback for environments without dpkg-dev installed (e.g. local preview)
    ver=$(sed -n '1s/^[^(]*(\([^)]*\)).*/\1/p' "$src_changelog")
  fi
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
    # Architecture: "any" means "build per host arch" — Debian's wildcard.
    # The actual .debs in dist/ have concrete arch suffixes (amd64, arm64, …),
    # so resolve "any" by looking at what's actually been built. Multi-token
    # lists like "amd64 arm64" pass through verbatim.
    if [[ "$arch" == "any" ]]; then
      built=""
      for f in "${REPO_ROOT}/dist/${name}_${ver}_"*.deb; do
        [[ -f "$f" ]] || continue
        a=$(basename "$f" .deb | sed "s/^${name}_${ver}_//")
        built="${built}${built:+ }${a}"
      done
      arch="${built:-amd64}"  # fallback if dist/ is empty (e.g. local preview)
    fi
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
<link rel="preconnect" href="https://fonts.googleapis.com" />
<link rel="preconnect" href="https://fonts.gstatic.com" crossorigin />
<link rel="stylesheet" href="https://fonts.googleapis.com/css2?family=Space+Grotesk:wght@400;500;600&family=JetBrains+Mono:wght@400;500&display=swap" />
<link rel="stylesheet" href="https://foundrylinux.org/styles.css" />
<style>
  /* apt-index layout — colour/font variables from foundrylinux.org/styles.css */
  * { box-sizing: border-box; margin: 0; padding: 0; }
  a { color: var(--accent); text-decoration: none; }
  a:hover { text-decoration: underline; }
  .wrap { max-width: 860px; margin: 0 auto; padding: 2rem 1rem; }
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
  h2 {
    font-family: var(--font-mono);
    font-size: 10.5px;
    color: var(--ink-faint);
    text-transform: uppercase;
    letter-spacing: 0.2em;
    margin: 2.5rem 0 .75rem;
  }
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
  .table-wrap { border: 1px solid var(--hairline); }
  table { width: 100%; border-collapse: collapse; }
  th {
    font-family: var(--font-mono);
    font-size: 10px;
    letter-spacing: 0.2em;
    text-transform: uppercase;
    color: var(--ink-faint);
    padding: .5rem .75rem;
    text-align: left;
    border-bottom: 1px solid var(--hairline-strong);
  }
  td { padding: .5rem .75rem; border-top: 1px solid var(--hairline); font-size: 14px; }
  td:nth-child(1) { white-space: nowrap; font-family: var(--font-mono); font-size: 13px; }
  td:nth-child(2) { color: var(--ink-soft); white-space: nowrap; font-family: var(--font-mono); font-size: 12px; }
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
</style>
</head>
<body>
<div class="wrap">
  <h1>APT Repository</h1>
  <p class="site-title">Foundry Linux</p>
  <p class="subtitle">${SITE_URL} &mdash; signed packages for Ubuntu 26.04 (resolute)</p>

  <h2>Quick install</h2>
  <pre>curl -fsSL ${SITE_URL}/key.gpg \\
  | gpg --dearmor -o /etc/apt/keyrings/foundry.gpg
echo "deb [signed-by=/etc/apt/keyrings/foundry.gpg] ${SITE_URL} resolute main" \\
  | sudo tee /etc/apt/sources.list.d/foundry.list
sudo apt-get update
sudo apt-get install foundry-retro-tools</pre>

  <h2>GPG key</h2>
  <p><a href="/key.gpg">↓ key.gpg</a> &mdash; verify before trusting:
     <code>gpg --show-keys /etc/apt/keyrings/foundry.gpg</code></p>

  <h2>Packages</h2>
  <div class="table-wrap">
  <table>
    <thead><tr><th>Package</th><th>Version</th><th>Description</th></tr></thead>
    <tbody>${PKG_ROWS}
    </tbody>
  </table>
  </div>

  <h2>Source</h2>
  <p><a href="${GITHUB_URL}">${GITHUB_URL}</a></p>

  <footer>Published ${PUBLISHED}</footer>
</div>
</body>
</html>
HTML

echo "Generated $OUT"
