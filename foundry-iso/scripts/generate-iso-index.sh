#!/usr/bin/env bash
# Generate dist/index.html for iso.foundrylinux.org.
#
# Reads dist/manifest-anvil.json and dist/manifest-atelier.json (produced by
# sign-iso.sh) to get version, sha256, and file size. Generates a branded
# download page matching the foundrylinux.org visual identity.
#
# Usage:
#   bash scripts/generate-iso-index.sh
#
# Output: dist/index.html

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
DIST_DIR="$SCRIPT_DIR/../dist"
OUT="$DIST_DIR/index.html"
PUBLISHED="$(date -u +%Y-%m-%dT%H:%M:%SZ)"

mkdir -p "$DIST_DIR"

# ── Read manifest JSON fields ─────────────────────────────────────────────────
# Returns empty string if manifest is absent (allows local preview without
# a full build).

manifest_field() {
  local file="$DIST_DIR/manifest-${1}.json" field="$2"
  if [[ -f "$file" ]]; then
    python3 -c "import json,sys; d=json.load(open('$file')); print(d.get('$field',''))"
  else
    echo ""
  fi
}

human_size() {
  local bytes="$1"
  if [[ -z "$bytes" || "$bytes" == "0" ]]; then echo "—"; return; fi
  python3 -c "
b = int('$bytes')
for unit in ['B','KB','MB','GB']:
    if b < 1024:
        print(f'{b:.0f} {unit}')
        break
    b //= 1024
else:
    print(f'{b:.1f} GB')
"
}

anvil_ver="$(manifest_field anvil version)"
anvil_sha="$(manifest_field anvil sha256)"
anvil_bytes="$(manifest_field anvil size_bytes)"
anvil_size="$(human_size "$anvil_bytes")"
anvil_filename="$(manifest_field anvil filename)"
[[ -z "$anvil_filename" ]] && anvil_filename="foundry-anvil-latest-amd64.iso"

atelier_ver="$(manifest_field atelier version)"
atelier_sha="$(manifest_field atelier sha256)"
atelier_bytes="$(manifest_field atelier size_bytes)"
atelier_size="$(human_size "$atelier_bytes")"
atelier_filename="$(manifest_field atelier filename)"
[[ -z "$atelier_filename" ]] && atelier_filename="foundry-atelier-latest-amd64.iso"

# Version label shown on the page (prefer anvil's version; both are the same).
VERSION="${anvil_ver:-latest}"

# ── HTML ──────────────────────────────────────────────────────────────────────

cat > "$OUT" <<HTML
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Foundry Linux Downloads</title>
<link rel="icon" type="image/svg+xml" href="https://foundrylinux.org/favicon.svg" />
<link rel="preconnect" href="https://fonts.googleapis.com" />
<link rel="preconnect" href="https://fonts.gstatic.com" crossorigin />
<link rel="stylesheet" href="https://fonts.googleapis.com/css2?family=Space+Grotesk:wght@400;500;600&family=JetBrains+Mono:wght@400;500&display=swap" />
<link rel="stylesheet" href="https://foundrylinux.org/styles.css" />
<style>
  * { box-sizing: border-box; margin: 0; padding: 0; }
  a { color: var(--accent); text-decoration: none; }
  a:hover { text-decoration: underline; }

  .wrap { max-width: 860px; margin: 0 auto; padding: 2rem 1rem; }

  .eyebrow {
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
  .subtitle { color: var(--ink-soft); margin-bottom: 2.5rem; font-size: 14px; }

  h2 {
    font-family: var(--font-mono);
    font-size: 10.5px;
    color: var(--ink-faint);
    text-transform: uppercase;
    letter-spacing: 0.2em;
    margin: 2.5rem 0 1rem;
  }

  /* ── Edition cards ───────────────────────────────────────────────── */
  .editions {
    display: grid;
    grid-template-columns: 1fr 1fr;
    gap: 1rem;
    margin-bottom: 2.5rem;
  }
  .edition {
    border: 1px solid var(--hairline-strong);
    padding: 1.5rem;
    display: flex;
    flex-direction: column;
    gap: .75rem;
  }
  .edition.recommended { border-color: var(--accent); }
  .edition-badge {
    font-family: var(--font-mono);
    font-size: 10px;
    letter-spacing: 0.2em;
    text-transform: uppercase;
    color: var(--accent);
  }
  .edition-name {
    font-family: var(--font-wordmark);
    font-size: 1.4rem;
    font-weight: 900;
    text-transform: uppercase;
    color: var(--ink);
  }
  .edition-desc { font-size: 13px; color: var(--ink-soft); line-height: 1.6; }
  .edition-includes {
    font-size: 12px;
    color: var(--ink-faint);
    font-family: var(--font-mono);
    line-height: 1.7;
  }
  .edition-includes span { color: var(--accent); margin-right: .3em; }
  .edition-meta {
    font-family: var(--font-mono);
    font-size: 11px;
    color: var(--ink-faint);
    display: flex;
    gap: 1.5rem;
    flex-wrap: wrap;
  }
  .btn-download {
    display: block;
    text-align: center;
    padding: .7rem 1rem;
    background: var(--accent);
    color: #000;
    font-family: var(--font-mono);
    font-size: 12px;
    font-weight: 600;
    letter-spacing: 0.1em;
    text-transform: uppercase;
    text-decoration: none;
    margin-top: auto;
  }
  .btn-download:hover { background: #ff7a3a; text-decoration: none; }
  .edition-verify {
    font-family: var(--font-mono);
    font-size: 11px;
    color: var(--ink-faint);
    text-align: center;
  }
  .edition-verify a { font-size: 11px; }

  /* ── Steps list ──────────────────────────────────────────────────── */
  .steps { list-style: none; counter-reset: step; display: flex; flex-direction: column; gap: .75rem; }
  .steps li {
    counter-increment: step;
    display: grid;
    grid-template-columns: 2rem 1fr;
    gap: .5rem;
    font-size: 14px;
    color: var(--ink-soft);
    line-height: 1.6;
  }
  .steps li::before {
    content: counter(step);
    font-family: var(--font-mono);
    font-size: 11px;
    color: var(--accent);
    padding-top: .15rem;
    text-align: right;
  }
  .steps li strong { color: var(--ink); }

  pre, code {
    font-family: var(--font-mono);
    font-size: 12px;
    background: rgba(255,255,255,0.015);
  }
  pre {
    border: 1px solid var(--hairline-strong);
    padding: 1rem 1.25rem;
    overflow-x: auto;
    line-height: 1.7;
    color: var(--ink);
    margin: .5rem 0;
  }
  code { padding: .1em .3em; }

  .specs-table { width: 100%; border-collapse: collapse; font-size: 14px; }
  .specs-table td {
    padding: .5rem .75rem;
    border-top: 1px solid var(--hairline);
    color: var(--ink-soft);
    vertical-align: top;
  }
  .specs-table td:first-child {
    font-family: var(--font-mono);
    font-size: 12px;
    color: var(--ink-faint);
    white-space: nowrap;
    width: 9rem;
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

  @media (max-width: 600px) {
    .editions { grid-template-columns: 1fr; }
    .wrap { padding: 1.5rem .75rem; }
    .site-title { font-size: 1.75rem; }
  }
</style>
</head>
<body>
<div class="wrap">
  <p class="eyebrow">Downloads</p>
  <p class="site-title">Foundry Linux</p>
  <p class="subtitle">Ubuntu 26.04 LTS &mdash; built for game developers &mdash; version ${VERSION}</p>

  <h2>Choose your edition</h2>

  <div class="editions">
    <!-- Anvil -->
    <div class="edition recommended">
      <div class="edition-badge">&#9733; Recommended</div>
      <div class="edition-name">Anvil</div>
      <div class="edition-desc">Everything you need to develop games on Linux. Retro tools, Blender add-ons, WorldFoundry GDK, and the full Foundry toolchain — ready from first boot.</div>
      <div class="edition-includes">
        <span>+</span>WorldFoundry GDK<br>
        <span>+</span>Retro dev tools (MAME, DASM, Ghidra&hellip;)<br>
        <span>+</span>Blender + add-ons<br>
        <span>+</span>Android &amp; iOS dev<br>
        <span>+</span>KDE Plasma desktop
      </div>
      <div class="edition-meta">
        <span>~${anvil_size}</span>
        <span>amd64</span>
        <span>Ubuntu 26.04</span>
      </div>
      <a class="btn-download" href="/foundry-anvil-latest-amd64.iso">&#8595; Download Anvil ISO</a>
      <div class="edition-verify">
        <a href="/foundry-anvil-latest-amd64.iso.sha256">SHA256</a>
        &nbsp;&middot;&nbsp;
        <a href="/foundry-anvil-latest-amd64.iso.asc">GPG sig</a>
      </div>
    </div>

    <!-- Atelier -->
    <div class="edition">
      <div class="edition-badge">Full kit</div>
      <div class="edition-name">Atelier</div>
      <div class="edition-desc">Everything in Anvil plus every Foundry metapackage: emulators, trackers, DAW tools, pixel art, game reimplementations, and the free-games bundle.</div>
      <div class="edition-includes">
        <span>+</span>Everything in Anvil<br>
        <span>+</span>Full emulator suite<br>
        <span>+</span>Audio trackers &amp; DAW tools<br>
        <span>+</span>Pixel art &amp; game frameworks<br>
        <span>+</span>Free &amp; open-source game library
      </div>
      <div class="edition-meta">
        <span>~${atelier_size}</span>
        <span>amd64</span>
        <span>Ubuntu 26.04</span>
      </div>
      <a class="btn-download" href="/foundry-atelier-latest-amd64.iso">&#8595; Download Atelier ISO</a>
      <div class="edition-verify">
        <a href="/foundry-atelier-latest-amd64.iso.sha256">SHA256</a>
        &nbsp;&middot;&nbsp;
        <a href="/foundry-atelier-latest-amd64.iso.asc">GPG sig</a>
      </div>
    </div>
  </div>

  <h2>How to install</h2>
  <ol class="steps">
    <li><strong>Download</strong> the ISO above. Anvil is the right choice if you're not sure.</li>
    <li><strong>Write to USB.</strong> Use <a href="https://etcher.balena.io">Balena Etcher</a> (free, Windows/Mac/Linux) &mdash; select the ISO, select your USB drive (8 GB or larger), click Flash.</li>
    <li><strong>Boot from USB.</strong> Restart your machine. On most PCs press <code>F12</code> (or <code>F2</code>, <code>Del</code>, <code>Esc</code>) during startup to choose a boot device and select the USB drive.</li>
    <li><strong>Try or install.</strong> The live session lets you try Foundry Linux before installing. Click <em>Install Foundry Linux</em> on the desktop to run the installer. Takes about 15 minutes.</li>
  </ol>

  <h2>System requirements</h2>
  <table class="specs-table">
    <tr><td>CPU</td><td>64-bit (amd64 / x86_64). 4 cores recommended for game builds.</td></tr>
    <tr><td>RAM</td><td>4 GB minimum &mdash; 8 GB+ recommended for game development workloads.</td></tr>
    <tr><td>Storage</td><td>50 GB minimum free space for Anvil. 100 GB+ for Atelier.</td></tr>
    <tr><td>GPU</td><td>Any GPU with OpenGL 3.3 support. Discrete GPU recommended for Blender and emulators.</td></tr>
    <tr><td>Boot</td><td>UEFI or BIOS. Secure Boot: disable before booting the ISO if your machine requires it.</td></tr>
  </table>

  <h2>Verify your download</h2>
  <p style="font-size:14px;color:var(--ink-soft);margin-bottom:.75rem;">Compare the SHA256 checksum to confirm the download is complete and unmodified.</p>
  <pre># Linux / Mac
sha256sum foundry-anvil-latest-amd64.iso
# Windows (PowerShell)
Get-FileHash foundry-anvil-latest-amd64.iso -Algorithm SHA256</pre>
  <p style="font-size:13px;color:var(--ink-faint);margin-top:.5rem;">Compare the output against <a href="/foundry-anvil-latest-amd64.iso.sha256">foundry-anvil-latest-amd64.iso.sha256</a>.</p>

  <footer>
    Published ${PUBLISHED} &mdash;
    <a href="https://foundrylinux.org">foundrylinux.org</a> &mdash;
    <a href="https://github.com/foundry-linux/foundry-iso">source</a>
  </footer>
</div>
</body>
</html>
HTML

echo "Generated $OUT"
