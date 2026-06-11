# Plan: Package `asar` as a .deb

## Context

`asar` is the Electron archive utility — it packs/unpacks `.asar` bundles used by Electron-based games and apps. It's commonly used to mod or inspect Electron-packaged games (e.g. RPG Maker MV/MZ titles, many indie releases). Neither `asar` nor `node-asar` exist in Ubuntu 26.04 universe, so we vendor it.

Source: [@electron/asar@4.2.0](https://github.com/electron/asar) (npm registry; the legacy standalone `asar@3.2.0` is unmaintained).

**Key characteristics:**
- Pure TypeScript → compiled JavaScript; no native C/C++ addons
- Runtime deps (`glob@13`, `minimatch@10`, `commander@13`) are all pure JS
- `Architecture: all` is appropriate (no arch-specific code)
- Runtime: Node.js ≥ 22.12.0; Ubuntu 26.04 ships `nodejs` 22.22.1 in universe
- No pre-built binary to download — requires `npm install` at build time
- Standalone package; no metapackage dependency wire-up at this time

---

## Approach

### Why not `Architecture: any` or bundling Node.js?

Pure JavaScript → one `.deb` works everywhere. Mark `Architecture: all`. Runtime dep `nodejs (>= 22)` is satisfied by Ubuntu 26.04 universe (`nodejs` 22.22.1).

### Installation layout

```
/usr/lib/asar/          ← package root (bin/, lib/, node_modules/, package.json)
/usr/bin/asar           ← wrapper shell script: exec /usr/bin/node /usr/lib/asar/bin/asar.mjs "$@"
/usr/share/man/man1/asar.1.gz  ← man page (required by Policy §12.1)
```

### Build flow (`build.sh`)

1. Install build deps: `nodejs npm debhelper dpkg-dev`
2. Download [npm tarball for @electron/asar@4.2.0](https://registry.npmjs.org/@electron/asar/-/asar-4.2.0.tgz) (SHA256 `9eaf6ea29e3b67be7dd52a575f6a74e3835dce57576fe7b2a31a70f7f19553bd`)
3. Extract to `asar-4.2.0/` (npm tarballs always extract to `package/`; rename it)
4. `npm install --omit=dev --ignore-scripts --legacy-peer-deps` — populates `node_modules/`
5. Copy `packages/asar/debian/` into `asar-4.2.0/debian/`
6. `dpkg-buildpackage -us -uc -b`
7. Move `.deb` → `$REPO_ROOT/dist/`

**Note on `--legacy-peer-deps`:** Ubuntu 26.04's npm 9.2.0 applies strict peer-dep validation
even for devDependencies being omitted. `typedoc@0.25.13` (a devDep of @electron/asar) has a
peer constraint `typescript@"4.6.x || ... || 5.4.x"` which conflicts with Ubuntu's system
typescript 5.9.x. `--legacy-peer-deps` reverts to npm v6 behavior, bypassing this.

---

## Files created

| File | Notes |
|---|---|
| `foundry-apt/packages/asar/build.sh` | Fetches tarball, runs npm install, dpkg-buildpackage |
| `foundry-apt/packages/asar/debian/control` | `Architecture: all`, `Depends: nodejs (>= 22)` |
| `foundry-apt/packages/asar/debian/rules` | Installs to `/usr/lib/asar/`, creates wrapper script |
| `foundry-apt/packages/asar/debian/changelog` | Version `4.2.0-1foundry1`, distribution `resolute` |
| `foundry-apt/packages/asar/debian/copyright` | DEP-5, MIT (upstream GitHub Inc. 2014) + MIT (Foundry) |
| `foundry-apt/packages/asar/debian/source/format` | `3.0 (quilt)` |
| `foundry-apt/packages/asar/debian/watch` | GitHub tags, `v@ANY_VERSION@` |
| `foundry-apt/packages/asar/debian/man/asar.1` | Covers pack, list, extract-file, extract + all options |
| `foundry-apt/packages/asar/debian/asar.manpages` | `debian/man/asar.1` |

---

## Verification

1. **Build:** `bash foundry-apt/packages/asar/build.sh` → `dist/asar_4.2.0-1foundry1_all.deb`

```
=== Fetching https://registry.npmjs.org/@electron/asar/-/asar-4.2.0.tgz ===
=== Verifying sha256 ===
/tmp/asar-build-A7AaJQ/asar_4.2.0.orig.tar.gz: OK
=== Installing npm runtime dependencies ===
added 8 packages, and audited 9 packages in 22s
=== dpkg-buildpackage -us -uc -b ===
dpkg-deb: building package 'asar' in '../asar_4.2.0-1foundry1_all.deb'.
OK   /repo/foundry-apt/dist/asar_4.2.0-1foundry1_all.deb  (369830 bytes)
```

PASS

2. **Inspect:** `dpkg-deb -I dist/asar_*.deb` — confirm `Depends: nodejs (>= 22)`, Architecture: all

```
 Package: asar
 Version: 4.2.0-1foundry1
 Architecture: all
 Maintainer: Foundry Linux <packages@foundrylinux.org>
 Installed-Size: 6223
 Depends: nodejs (>= 22)
 Section: devel
 Priority: optional
 Homepage: https://github.com/electron/asar
```

PASS

3. **File listing:** `dpkg-deb -c dist/asar_*.deb` — confirm `/usr/bin/asar`, `/usr/lib/asar/bin/asar.mjs`, `/usr/lib/asar/node_modules/`, `/usr/share/man/man1/asar.1.gz`

```
-rwxr-xr-x root/root        61 ./usr/bin/asar
-rwxr-xr-x root/root      2806 ./usr/lib/asar/bin/asar.mjs
-rw-r--r-- root/root     12755 ./usr/lib/asar/lib/asar.js
drwxr-xr-x root/root         0 ./usr/lib/asar/node_modules/
-rw-r--r-- root/root     83721 ./usr/lib/asar/node_modules/commander/lib/command.js
... (commander, glob, minimatch, lru-cache, minipass, path-scurry, brace-expansion, balanced-match)
-rw-r--r-- root/root      1232 ./usr/share/man/man1/asar.1.gz
```

PASS

4. **Lintian:** `lintian dist/asar_*.deb` — zero E: or W: lines

```
running with root privileges is not recommended!
```

No E: or W: lines. PASS

5. **Smoke install + run:**

```
=== version ===
v4.2.0
```

Note: Ubuntu's Docker image suppresses man page extraction via
`/etc/dpkg/dpkg.cfg.d/excludes` (`path-exclude=/usr/share/man/*`). Use
`dpkg -L asar` to confirm the file is registered; `ls /usr/share/man/` will
show nothing in Docker. This is a Docker-image restriction, not a packaging defect.

PASS

6. **Functional test:** pack a directory, list contents, extract all

```
=== pack + list + extract ===
/hi.txt
/other.txt
hi.txt: hello asar
other.txt: world
PASS
```

PASS

---

## Commit

`05905d6` feat(foundry-apt): package asar 4.2.0 as a .deb
