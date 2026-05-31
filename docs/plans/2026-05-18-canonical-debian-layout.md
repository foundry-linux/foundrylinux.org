# Plan: Migrate metapackages to canonical Debian source-package layout

**Date:** 2026-05-18
**Status:** Done

## Context

[`2026-05-18-package-skill.md`](2026-05-18-package-skill.md) shipped the `/package` skill and updated `generate-index.sh` to parse both `packages/<name>/debian/control` (source-package format) **and** `packages/<name>/DEBIAN/control` (binary-package format, our current metapackage layout). The dual-support was a transitional posture.

That dual-support is wrong. The canonical Debian layout is **`debian/`** (lowercase, source-package format) — that's what every package in the Debian/Ubuntu archives uses, including pure metapackages (e.g. `gnome` → `apt-cache show gnome` lists `Source: meta-gnome`). The uppercase `DEBIAN/` is a *generated* artifact that lives inside a built `.deb`, not a thing humans author.

We've been authoring `packages/<name>/DEBIAN/control` directly because pure metapackages have no compile step, so `dpkg-deb --build packages/<name>` works on the directory. It produces a valid `.deb`, but it's not the canonical workflow — and it forks our build pipeline into two paths (one for metapackages, one for the `/package` skill's vendored upstreams).

**This plan converts all five metapackages to the canonical `debian/` source-package layout**, drops `DEBIAN/control` support from `build-all.sh` and `generate-index.sh`, and consolidates the build pipeline to a single `dpkg-buildpackage` invocation. xa65 (the deprecated hand-rolled vendor case) keeps its `DEBIAN/control` until it's repackaged with `/package` — but no other package in the repo uses that layout afterward.

## Scope

Five metapackages, all `Architecture: all`:

| Package | Current version | Source name (chosen) |
|---|---|---|
| `foundry-retro-tools` | 1.0.1 | `foundry-retro-tools` |
| `worldfoundry-dev` | 1.0.0 | `worldfoundry-dev` |
| `worldfoundry-blender` | 1.0.0 | `worldfoundry-blender` |
| `worldfoundry-android-dev` | 1.0.0 | `worldfoundry-android-dev` |
| `worldfoundry-engine-build-deps` | 1.0.0 | `worldfoundry-engine-build-deps` |

Source name = package name (1:1). For native metapackages, the source produces a single binary stanza with the same name; this keeps things simple. (If we ever split into a multi-binary metapackage family, we'd rename the source to e.g. `meta-foundry` and emit multiple `Package:` stanzas — out of scope.)

Versions stay where they are. **No version bump** for this migration — the produced `.deb` is byte-identical metadata, just authored a different way. (Lintian-warnings-free output and `dpkg-buildpackage`-built provenance is the only difference.)

## Per-metapackage transformation

For each `packages/<name>/`:

### Delete

- `DEBIAN/control` (the old file)
- `DEBIAN/` (the directory, if empty after)

### Create

#### `debian/source/format`

```
3.0 (native)
```

Why native: there's no upstream tarball. Source = packaging itself. Native packages use a single-component version (e.g. `1.0.1`, no `-1foundry1` revision suffix).

#### `debian/control`

```
Source: <NAME>
Section: metapackages
Priority: optional
Maintainer: <existing maintainer from old DEBIAN/control>
Build-Depends: debhelper-compat (= 13)
Standards-Version: 4.7.0
Homepage: <existing homepage from old DEBIAN/control>
Rules-Requires-Root: no

Package: <NAME>
Architecture: all
Depends: <existing depends from old DEBIAN/control, dropping ${misc:Depends} since metapackages don't need it>,
 ${misc:Depends}
Recommends: <existing recommends, if any>
Suggests: <existing suggests, if any>
Description: <existing short description>
 <existing long description>
```

Note: `${misc:Depends}` belongs in `Depends:` even for metapackages — debhelper injects deps like `dpkg (>= 1.x)` via this substvar. Lintian flags its absence as a warning.

#### `debian/changelog`

```
<NAME> (<EXISTING_VERSION>) resolute; urgency=medium

  * Migrated from manually-built DEBIAN/control to canonical debian/
    source-package layout. No functional changes — same Depends,
    Recommends, Description.

 -- <Maintainer Name> <maintainer@email>  <RFC2822 date>
```

The `resolute` distribution name matches Ubuntu 26.04's codename and what aptly publishes. `urgency=medium` is the default for non-security uploads.

#### `debian/rules`

```
#!/usr/bin/make -f
%:
	dh $@
```

Three lines. `dh $@` invokes all the helpers (`dh_auto_configure`, `dh_auto_build`, `dh_auto_install`, `dh_install`, `dh_strip`, etc.). For a pure metapackage with no files, these all no-op gracefully. `dh_gencontrol` and `dh_builddeb` produce the `.deb`.

Make executable (`chmod +x debian/rules`).

#### `debian/copyright`

Metapackages don't ship any code, but Debian Policy still requires `debian/copyright`. Minimal valid file:

```
Format: https://www.debian.org/doc/packaging-manuals/copyright-format/1.0/
Upstream-Name: <NAME>
Source: https://foundrylinux.org/

Files: *
Copyright: 2026 Foundry Linux contributors
License: MIT
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 .
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 .
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
```

(MIT for the packaging itself — same as cloning-permissive expectations.)

## `build-all.sh` rewrite

Current logic:

```
for pkgdir in packages/*/; do
    if [[ -x "$pkgdir/build.sh" ]]; then bash build.sh; continue; fi
    if [[ ! -f "$pkgdir/DEBIAN/control" ]]; then SKIP; continue; fi
    dpkg-deb --build "$pkgdir" dist/...
done
```

New logic:

```
for pkgdir in packages/*/; do
    name=$(basename "$pkgdir")

    if [[ -x "$pkgdir/build.sh" ]]; then
        bash "$pkgdir/build.sh"   # vendor packages handle their own build
        continue
    fi

    if [[ -f "$pkgdir/debian/control" && -f "$pkgdir/debian/changelog" ]]; then
        # Canonical native metapackage: copy to versioned build dir, run dpkg-buildpackage
        ver=$(awk 'NR==1 {if (match($0, /\(([^)]+)\)/, a)) print a[1]; exit}' "$pkgdir/debian/changelog")
        builddir=$(mktemp -d -t "${name}-build-XXXXXX")
        cp -a "$pkgdir/." "${builddir}/${name}-${ver}/"
        ( cd "${builddir}/${name}-${ver}" && dpkg-buildpackage -us -uc -b -d --no-sign ) >/dev/null 2>&1
        # The .deb lands as ${name}_${ver}_all.deb at $builddir level
        deb="${builddir}/${name}_${ver}_all.deb"
        [[ -f "$deb" ]] || { echo "FAIL $name (no .deb produced)" >&2; rm -rf "$builddir"; continue; }
        mv "$deb" "${REPO_ROOT}/dist/"
        rm -rf "$builddir"
        echo "OK   dist/${name}_${ver}_all.deb"
        continue
    fi

    # DEBIAN/control direct-build path is REMOVED.
    # xa65 keeps its DEBIAN/control but has a build.sh, so it hits the first branch.

    echo "SKIP $name (no debian/control or build.sh)"
done
```

The `-d` flag skips build-dep checks (faster). `--no-sign` suppresses the changes-file signing prompt (we don't sign per-package; only the Release file gets signed at publish time).

## `generate-index.sh` simplification

Drop the `bin_control` fallback added in commit `bd37a2d`. Only parse `debian/control` + `debian/changelog`. xa65 will disappear from the index *temporarily* until it's repackaged with `/package` — that's the forcing function for finishing the xa65 cleanup.

Wait — that's wrong. xa65 is currently live (`v0.0.14` includes it). Dropping the fallback before xa65 is repackaged would orphan it.

**Decision:** keep the `DEBIAN/control` fallback in `generate-index.sh` *until xa65 is repackaged with /package*. Drop it then. This plan removes the fallback **only from `build-all.sh`**, which still works for xa65 because it has a `build.sh`.

## CI publish.yml changes

`publish.yml` step 26 currently installs `aptly gnupg rclone`. Add:

```yaml
sudo apt-get install -y aptly gnupg rclone dpkg-dev debhelper
```

`dpkg-dev` provides `dpkg-buildpackage`, `dpkg-source`, etc. `debhelper` provides `dh` and the `dh_*` helpers. Both are small (~3 MB combined).

## Skill doc updates

### `/package` skill

`SKILL.md` Step 5 ("Wire into foundry-apt") currently shows `packages/<name>/{build.sh, debian/, debian/...}`. That's right — no change needed. But the "Why debhelper" comparison should call out that even metapackages now use the same layout.

Add a short "Pure metapackages" subsection clarifying that for metapackages (no upstream source), use `3.0 (native)` source format and skip the vendoring step.

### `new-web-apt-repo` skill

The "Package layouts — two supported, one deprecated" table I just wrote needs a follow-up edit once this plan ships: the "Pure metapackage" row should also say `debian/` (lowercase, source format) not `DEBIAN/` (binary). Update after the migration lands.

## Verification

1. **Local build: each migrated metapackage produces a valid `.deb`.**

    ```bash
    cd foundry-apt && task build
    ls dist/*.deb
    ```

    Expect: five `<name>_<ver>_all.deb` files plus `xa65_*.deb`. No new packages, no missing packages.

2. **Per-metapackage `dpkg-deb -I` is functionally equivalent to the pre-migration version.**

    ```bash
    for pkg in foundry-retro-tools worldfoundry-dev worldfoundry-blender worldfoundry-android-dev worldfoundry-engine-build-deps; do
        echo "--- $pkg ---"
        dpkg-deb -I dist/${pkg}_*_all.deb | grep -E "Package|Version|Depends|Recommends|Description" | head -8
    done
    ```

    Expect: same `Depends:`, `Recommends:`, version, description as before the migration.

3. **Lintian clean (or close).**

    ```bash
    lintian dist/*_all.deb 2>&1 | grep -vE "^N:" | head -20
    ```

    Expect: no E: lines. Warnings are acceptable but should be reviewed.

4. **generate-index.sh emits all metapackages from the new layout + xa65 from the legacy.**

    ```bash
    PUBLISH_DIR=/tmp/idx-test bash foundry-apt/scripts/generate-index.sh
    grep -oE '<tr><td>[^<]*<a[^>]*>[^<]+</a>' /tmp/idx-test/index.html | head -10
    ```

    Expect: six rows (five metapackages + xa65), same as today.

5. **Live (post sync + bump).** `https://apt.foundrylinux.org/` shows the same six package rows; `docker run ubuntu:26.04 apt install foundry-retro-tools` from the live repo installs successfully (regression test).

6. **Skill docs updated.** `/package` skill's "Pure metapackages" section + new-web-apt-repo's table reflect the canonical-only state.

## Out of scope

- **Repackage xa65 with `/package` skill.** Separate work; tracked in TODO.md.
- **Multi-binary source packages** (one `Source:` producing several `Package:` stanzas, e.g. main + `-doc` + `-dev`). Not needed for current metapackages; the `/package` skill already handles it for vendored upstreams.

## Open questions

- **Build directory naming.** `dpkg-buildpackage` traditionally expects the dir to be `<source>-<version>`. We copy to `${name}-${ver}/` to satisfy this — but `dpkg-source` may still complain about extraneous files (e.g. no `debian/source/include-binaries`). If so, add `debian/source/options`:

    ```
    extend-diff-ignore = "(^|/)(\.gitignore|\.gitkeep)$"
    ```

  We'll only know if this is needed by running it. Resolve while implementing.
