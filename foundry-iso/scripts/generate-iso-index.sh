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
[[ -z "$anvil_size" || "$anvil_size" == "—" ]] && anvil_size="~5 GB"
anvil_filename="$(manifest_field anvil filename)"
[[ -z "$anvil_filename" ]] && anvil_filename="foundry-anvil-latest-amd64.iso"

atelier_ver="$(manifest_field atelier version)"
atelier_sha="$(manifest_field atelier sha256)"
atelier_bytes="$(manifest_field atelier size_bytes)"
atelier_size="$(human_size "$atelier_bytes")"
[[ -z "$atelier_size" || "$atelier_size" == "—" ]] && atelier_size="~10 GB"
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
    transition: border-color .2s, box-shadow .2s;
  }
  .edition:hover {
    border-color: var(--accent);
    box-shadow: 0 0 40px rgba(255,99,32,.45), 0 0 12px rgba(255,99,32,.25);
  }
  .edition.recommended {
    border-color: var(--accent);
    box-shadow: 0 0 28px rgba(255,99,32,.25), 0 0 6px rgba(255,99,32,.12);
  }
  .edition.recommended:hover {
    box-shadow: 0 0 56px rgba(255,99,32,.6), 0 0 16px rgba(255,99,32,.35);
  }
  .edition-icon {
    color: var(--ink-faint);
    transition: color .2s, filter .2s;
  }
  .edition:hover .edition-icon {
    color: var(--accent);
    filter: drop-shadow(0 0 8px rgba(255,99,32,.8));
  }
  .edition-size {
    font-family: var(--font-mono);
    font-size: 2rem;
    font-weight: 600;
    color: var(--ink);
    letter-spacing: -0.02em;
  }
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
    letter-spacing: 0;
    color: var(--ink);
    transition: color .2s, text-shadow .2s;
  }
  .edition:hover .edition-name {
    color: var(--accent);
    text-shadow: 0 0 10px rgba(255,99,32,.6), 0 0 24px rgba(255,99,32,.3);
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

  @media (max-width: 600px) {
    .editions { grid-template-columns: 1fr; }
    .wrap { padding: 1.5rem .75rem; }
    .site-title { font-size: 1.75rem; }
  }
</style>
</head>
<body>
<header class="topbar-wrap">
  <div class="shell">
    <div class="topbar">
      <a class="topbar-mark" href="https://foundrylinux.org" style="text-decoration:none;color:inherit;">
        <svg viewBox="0 0 32 32" width="22" height="22" aria-hidden="true"><g fill="none" stroke="currentColor" stroke-width="1.4" stroke-linecap="round" stroke-linejoin="round"><path d="M4 12 L28 12 L26 16 L22 16 L22 20 L10 20 L10 16 L6 16 Z"></path><path d="M12 20 L11 24 L21 24 L20 20"></path><path d="M9 24 L23 24 L23 27 L9 27 Z"></path><line x1="16" y1="4" x2="16" y2="8"></line><line x1="11" y1="6" x2="13" y2="9"></line><line x1="21" y1="6" x2="19" y2="9"></line></g></svg>
        <b>foundrylinux.org</b>
      </a>
      <nav class="topbar-nav">
        <a href="https://foundrylinux.org/#forge">The Forge</a>
        <a href="https://foundrylinux.org/#install">Install</a>
        <a href="https://foundrylinux.org/#editions">Editions</a>
        <a href="https://foundrylinux.org/packages">Packages</a>
      </nav>
      <div class="topbar-version">
        <span class="dot"></span>FOUNDRY 26.04 &middot; &ldquo;ANVIL&rdquo; &middot; LTS
      </div>
    </div>
  </div>
</header>
<div class="wrap">
  <p class="eyebrow">Downloads</p>
  <p class="site-title">Foundry Linux</p>
  <p class="subtitle">Ubuntu 26.04 LTS &mdash; built for game developers &mdash; version ${VERSION}</p>

  <h2>Choose your edition</h2>

  <div class="editions">
    <!-- Anvil -->
    <div class="edition recommended">
      <div class="edition-icon"><svg viewBox="0 0 64 64" width="36" height="36" aria-hidden="true"><g fill="none" stroke="currentColor" stroke-width="1.8" stroke-linecap="round" stroke-linejoin="round"><path d="M6 22 L58 22 L52 32 L46 32 L46 38 L18 38 L18 32 L12 32 Z"></path><path d="M22 38 L20 50 L44 50 L42 38"></path><path d="M14 50 L50 50 L50 56 L14 56 Z"></path><line x1="8" y1="14" x2="6" y2="8"></line><line x1="14" y1="14" x2="14" y2="6"></line><line x1="20" y1="14" x2="22" y2="8"></line></g></svg></div>
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
      <div class="edition-size">${anvil_size}</div>
      <div class="edition-meta">
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
      <div class="edition-icon"><svg viewBox="0 0 64 64" width="36" height="36" aria-hidden="true"><g fill="none" stroke="currentColor" stroke-width="1.4" stroke-linecap="round" stroke-linejoin="round"><path d="M16 24 Q8 24 8 36 Q8 48 16 48 L20 44 L44 44 L48 48 Q56 48 56 36 Q56 24 48 24 Z"></path><circle cx="20" cy="34" r="3"></circle><line x1="17" y1="34" x2="23" y2="34"></line><line x1="20" y1="31" x2="20" y2="37"></line><circle cx="42" cy="32" r="1.4"></circle><circle cx="46" cy="36" r="1.4"></circle><circle cx="38" cy="36" r="1.4"></circle></g></svg></div>
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
      <div class="edition-size">${atelier_size}</div>
      <div class="edition-meta">
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
    <li><span><strong>Download the ISO above.</strong> If you're not sure which edition, pick Anvil &mdash; it has everything you need to build and run games.</span></li>
    <li><span><strong>Flash it to a USB drive (8 GB+).</strong> Use <a href="https://rufus.ie">Rufus</a> on Windows or <a href="https://etcher.balena.io">Balena Etcher</a> on Mac. Select the ISO, select your USB drive, write. Done in a couple of minutes.</span></li>
    <li><span><strong>Boot from the USB.</strong> Plug it in and restart. If your PC doesn't boot from USB automatically, enter your firmware menu (usually <code>F12</code> or <code>Del</code> at the manufacturer splash screen) and select the USB drive. <em>Tip: if you see a Windows fast-startup screen instead, hold Shift while clicking Restart in Windows first.</em></span></li>
    <li><span><strong>Install.</strong> Foundry Linux boots to a live desktop. Click <em>Install Foundry Linux</em>, follow the prompts &mdash; takes about 15 minutes. Your game projects and files stay on whatever drive you point the installer at.</span></li>
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

</div>
<footer class="shell">
  <div class="foot">
    <div class="foot-mark">
      <svg viewBox="0 0 32 32" width="28" height="28" aria-hidden="true"><g fill="none" stroke="currentColor" stroke-width="1.4" stroke-linecap="round" stroke-linejoin="round"><path d="M4 12 L28 12 L26 16 L22 16 L22 20 L10 20 L10 16 L6 16 Z"></path><path d="M12 20 L11 24 L21 24 L20 20"></path><path d="M9 24 L23 24 L23 27 L9 27 Z"></path><line x1="16" y1="4" x2="16" y2="8"></line><line x1="11" y1="6" x2="13" y2="9"></line><line x1="21" y1="6" x2="19" y2="9"></line></g></svg>
      <span>FOUNDRY &middot; LINUX</span>
    </div>
    <nav class="foot-links">
      <a href="https://foundrylinux.org/#docs">Docs</a>
      <a href="https://foundrylinux.org/packages">Packages</a>
      <a href="https://foundrylinux.org/#mirrors">Mirrors</a>
      <a href="https://foundrylinux.org/#git">Git</a>
      <a href="https://foundrylinux.org/#bugs">Bugs</a>
    </nav>
    <div class="foot-meta">
      Published ${PUBLISHED}<br>
      Forged in the open &middot; MMXXVI
    </div>
  </div>
</footer>
</body>
</html>
HTML

echo "Generated $OUT"
