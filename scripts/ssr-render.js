#!/usr/bin/env node
// SSR-renders each entry in PAGES → a fully-static .html file (no React runtime).
// Run via: node scripts/ssr-render.js  (or: task site-build).
// See docs/plans/2026-05-21-packages-page.md §4.
'use strict';

const esbuild = require('esbuild');
const fs = require('fs');
const path = require('path');
const os = require('os');

const root = path.resolve(__dirname, '..');

// ─── PAGES table ─────────────────────────────────────────────────────────────
const PAGES = [
  {
    entry: 'site/app.jsx',
    exportName: 'App',
    out: 'site/index.html',
    title: 'foundrylinux.org · FOUNDRY LINUX',
    description: 'Foundry Linux — a Linux distribution for game development, reverse engineering, and retro tooling. Curated packages and a devbox built on Ubuntu 26.04.',
    extraBodyScripts: true, // copy-button + embers + scroll-shrink (home only)
  },
  {
    entry: 'site/packages.jsx',
    exportName: 'PackagesApp',
    out: 'site/packages.html',
    title: 'Packages · FOUNDRY LINUX',
    description: 'Every package Foundry Linux installs on top of Kubuntu 26.04 — three nested editions, nine domain categories, six vendored standalones.',
    extraBodyScripts: false,
  },
];

// ─── HTML wrapper ────────────────────────────────────────────────────────────
function homePageScripts() {
  return `
  <script>
    /* Copy button — replaces the React useState handler */
    var _copyBtn = document.querySelector('.copy');
    if (_copyBtn) {
      _copyBtn.addEventListener('click', function() {
        navigator.clipboard?.writeText('sudo apt install foundry-anvil');
        this.lastChild.textContent = 'COPIED';
        var btn = this;
        setTimeout(function() { btn.lastChild.textContent = 'COPY'; }, 1400);
      });
    }
  </script>
  <script src="embers.js"></script>
  <script>
    /* Scroll-shrink: --header-shrink ramps 0→1 as scrollY reaches half
     * a viewport. @property + html transition eases every change. */
    if (!matchMedia("(prefers-reduced-motion: reduce)").matches) {
      var _hsPending = false;
      var _hsUpdate = function() {
        _hsPending = false;
        var raw = Math.min(1, window.scrollY / (window.innerHeight / 2));
        var eased = 1 - (1 - raw) * (1 - raw);
        document.documentElement.style.setProperty("--header-shrink", eased.toFixed(3));
      };
      var _hsTick = function() {
        if (_hsPending) return;
        _hsPending = true;
        requestAnimationFrame(_hsUpdate);
      };
      window.addEventListener("scroll", _hsTick, { passive: true });
      window.addEventListener("resize", _hsTick, { passive: true });
    }
  </script>`;
}

function wrap(body, { title, description, extraBodyScripts }) {
  return `<!doctype html>
<html lang="en">
<head>
  <meta charset="utf-8" />
  <meta name="viewport" content="width=device-width, initial-scale=1" />
  <title>${title}</title>
  <meta name="description" content="${description}" />
  <link rel="icon" type="image/svg+xml" href="/favicon.svg" />
  <link rel="preload" href="fonts/big-shoulders-display-900-latin.woff2" as="font" type="font/woff2" crossorigin>
  <link rel="stylesheet" href="styles.css" />
</head>
<body data-bg="pure">
  <div id="root">${body}</div>
${extraBodyScripts ? homePageScripts() : ''}
  <script src="/copy.js" defer></script>
</body>
</html>`;
}

// ─── per-page renderer ───────────────────────────────────────────────────────
function renderPage(page) {
  const entryPath = path.join(root, page.entry);
  const bundle = path.join(os.tmpdir(), `foundry-ssr-${page.exportName}.cjs`);
  const tmpEntry = path.join(os.tmpdir(), `foundry-ssr-entry-${page.exportName}.jsx`);

  fs.writeFileSync(tmpEntry, [
    `import { ${page.exportName} } from '${entryPath}';`,
    `import { renderToStaticMarkup } from 'react-dom/server';`,
    `import { createElement } from 'react';`,
    `export const html = renderToStaticMarkup(createElement(${page.exportName}));`,
  ].join('\n'));

  esbuild.buildSync({
    entryPoints: [tmpEntry],
    bundle: true,
    platform: 'node',
    format: 'cjs',
    jsx: 'automatic',
    loader: { '.json': 'json' },
    outfile: bundle,
    nodePaths: [path.join(root, 'node_modules')],
  });

  delete require.cache[bundle];
  const { html: body } = require(bundle);

  const fullHtml = wrap(body, page);
  const outPath = path.join(root, page.out);
  fs.writeFileSync(outPath, fullHtml);
  const kb = Math.round(body.length / 1024);
  console.log(`✓ ${page.out.padEnd(20)} rendered (${kb} KB of markup, no React runtime)`);
}

// ─── main ────────────────────────────────────────────────────────────────────
for (const page of PAGES) {
  renderPage(page);
}
