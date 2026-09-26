#!/usr/bin/env node
// Validate the images and icons referenced by a generated static site.
// External image URLs are forbidden: a third-party outage must not leave a
// broken-image glyph in a Foundry page.
'use strict';

const fs = require('fs');
const path = require('path');

const args = process.argv.slice(2);
const rootFlag = args.indexOf('--site-root');
if (rootFlag === -1 || !args[rootFlag + 1]) {
  throw new Error('Usage: check-web-assets.js HTML... --site-root DIRECTORY');
}
const root = path.resolve(args[rootFlag + 1]);
const htmlFiles = args.slice(0, rootFlag);
if (!htmlFiles.length) throw new Error('Provide at least one generated HTML file.');

function attrs(tag) {
  return Object.fromEntries([...tag.matchAll(/([\w:-]+)\s*=\s*(["'])(.*?)\2/g)]
    .map(([, key, , value]) => [key.toLowerCase(), value]));
}

function assertImage(file, reference) {
  const bytes = fs.readFileSync(file);
  if (!bytes.length) throw new Error(`${reference}: image is empty`);
  const ext = path.extname(file).toLowerCase();
  const text = bytes.toString('utf8', 0, Math.min(bytes.length, 512));
  const valid = (ext === '.svg' && /<svg[\s>]/i.test(text))
    || (ext === '.ico' && bytes.length >= 4 && bytes[0] === 0 && bytes[1] === 0 && bytes[2] === 1 && bytes[3] === 0)
    || (ext === '.png' && bytes.length >= 8 && bytes.subarray(0, 8).equals(Buffer.from([137, 80, 78, 71, 13, 10, 26, 10])))
    || (ext === '.jpg' || ext === '.jpeg') && bytes[0] === 0xff && bytes[1] === 0xd8
    || ext === '.webp' && text.slice(0, 4) === 'RIFF' && text.slice(8, 12) === 'WEBP';
  if (!valid) throw new Error(`${reference}: ${file} is not a valid ${ext} image`);
}

let checked = 0;
for (const htmlFile of htmlFiles) {
  const html = fs.readFileSync(htmlFile, 'utf8');
  for (const match of html.matchAll(/<(img|link)\b[^>]*>/gi)) {
    const tagName = match[1].toLowerCase();
    const a = attrs(match[0]);
    const isIcon = tagName === 'link' && /(^|\s)icon(\s|$)/i.test(a.rel || '');
    if (tagName !== 'img' && !isIcon) continue;
    const source = a.src || a.href;
    if (!source) throw new Error(`${htmlFile}: ${tagName} has no source`);
    if (/^(https?:)?\/\//i.test(source)) {
      throw new Error(`${htmlFile}: external ${isIcon ? 'icon' : 'image'} is forbidden: ${source}`);
    }
    if (source.startsWith('data:')) { checked++; continue; }
    const pathname = source.split(/[?#]/, 1)[0];
    const local = path.resolve(root, `.${pathname.startsWith('/') ? pathname : `/${pathname}`}`);
    if (!local.startsWith(`${root}${path.sep}`) || !fs.existsSync(local)) {
      throw new Error(`${htmlFile}: missing local asset ${source}`);
    }
    assertImage(local, `${htmlFile}: ${source}`);
    checked++;
  }
}
console.log(`PASS: ${checked} local image/icon asset${checked === 1 ? '' : 's'} validated.`);
