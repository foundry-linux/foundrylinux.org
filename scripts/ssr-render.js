#!/usr/bin/env node
// Renders the React app to a fully-static site/index.html (no React runtime needed).
// Run via: node scripts/ssr-render.js  (or: task site-build)
'use strict';

const esbuild = require('esbuild');
const fs = require('fs');
const path = require('path');
const os = require('os');

const root = path.resolve(__dirname, '..');
const bundle = path.join(os.tmpdir(), 'foundry-ssr.cjs');
const entry = path.join(os.tmpdir(), 'foundry-ssr-entry.jsx');

// Temp entry: import App + react-dom/server and export the rendered HTML string
fs.writeFileSync(entry, [
  `import { App } from '${path.join(root, 'site/app.jsx')}';`,
  `import { renderToStaticMarkup } from 'react-dom/server';`,
  `import { createElement } from 'react';`,
  `export const html = renderToStaticMarkup(createElement(App));`,
].join('\n'));

esbuild.buildSync({
  entryPoints: [entry],
  bundle: true,
  platform: 'node',
  format: 'cjs',
  jsx: 'automatic',
  outfile: bundle,
  // Tell esbuild where to find react/react-dom
  nodePaths: [path.join(root, 'node_modules')],
});

delete require.cache[bundle];
const { html: body } = require(bundle);

const page = `<!doctype html>
<html lang="en">
<head>
  <meta charset="utf-8" />
  <meta name="viewport" content="width=device-width, initial-scale=1" />
  <title>foundrylinux.org · FOUNDRY LINUX</title>
  <meta name="description" content="Foundry Linux — a Linux distribution for game development, reverse engineering, and retro tooling. Curated packages and a devbox built on Ubuntu 26.04." />
  <link rel="preload" href="fonts/big-shoulders-display-900-latin.woff2" as="font" type="font/woff2" crossorigin>
  <link rel="stylesheet" href="styles.css" />
</head>
<body data-bg="pure">
  <div id="root">${body}</div>
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
  </script>
  <script>
    /* Copy button — replaces the React useState handler */
    document.querySelector('.copy').addEventListener('click', function() {
      navigator.clipboard?.writeText('sudo apt install foundry-base');
      this.lastChild.textContent = 'COPIED';
      var btn = this;
      setTimeout(function() { btn.lastChild.textContent = 'COPY'; }, 1400);
    });
  </script>
  <script src="embers.js"></script>
</body>
</html>`;

fs.writeFileSync(path.join(root, 'site/index.html'), page);
const kb = Math.round(body.length / 1024);
console.log(`✓ site/index.html rendered (${kb} KB of markup, no React runtime)`);
