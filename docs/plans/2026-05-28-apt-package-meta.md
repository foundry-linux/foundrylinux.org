---
title: Per-package meta-info JSON + richer apt index
date: 2026-05-28
status: done
---

# Per-package meta-info JSON + richer apt index

## Context

[apt.foundrylinux.org](https://apt.foundrylinux.org) currently shows only package name,
version, and a one-line description. The
[2026-05-22 manual package inventory](../investigations/2026-05-22-package-inventory.manual.md)
documents much richer content (full descriptions, annotated dependency lists) that
should be visible on the index page. The goal is to:

1. Auto-generate per-package `public/meta/{name}.json` files at publish time (from
   `debian/control` + aptly Packages data)
2. Have `generate-index.sh` consume them to render the long description + Depends list
3. Fallback: if a meta file doesn't exist (local preview), generate it on the fly
4. Not link or expose meta file names anywhere in the rendered HTML

## What data is available

All the content needed is already in `debian/control`:

- **`Description:`** — first line = short desc; indented continuation lines = long desc;
  blank separator = ` .` (space + dot). Many metapackages already have an annotated
  bullet list in the long desc (e.g. `foundry-daw` has `* ardour   multitrack DAW`).
- **`Depends:`** — machine-readable dep list; strip `${misc:Depends}` and version
  constraints to get a clean package name list shown as chips.
- **`Homepage:`**, **`Architecture:`**, **`Section:`** — already parsed.

`Installed-Size:` is available from `public/dists/resolute/main/binary-*/Packages`
after `publish-local.sh` runs (aptly injects it from the built .deb). Enriched from
there if the file exists; omitted otherwise.

## Critical files

| File | Change |
|---|---|
| `foundry-apt/scripts/generate-meta.sh` | **New** — emits `public/meta/{name}.json` per package |
| `foundry-apt/scripts/generate-index.sh` | **Modified** — reads meta files, renders long desc + deps |
| `foundry-apt/Taskfile.yml` | **Modified** — add `generate-meta` task |
| `foundry-apt/.github/workflows/publish.yml` | **Modified** — add generate-meta step |

## Storage layout

```
public/
  meta/
    blender-asset-finder.json
    blender-asset-finder-cli.json
    f9dasm.json
    foundry-anvil.json
    foundry-daw.json
    …
  index.html          ← aggregates all meta/*.json at generation time
```

One file per package: only the affected file is regenerated when a single package
changes. The index generator reads and aggregates all of them at HTML-generation time.

## 1. `scripts/generate-meta.sh` (new)

```
Usage: generate-meta.sh [PACKAGE_NAME]
```

With a package name: generates/updates `public/meta/{name}.json` for that one package.
Without arguments: iterates all packages, skipping any whose `public/meta/{name}.json`
is newer than both `debian/control` and `debian/changelog` (mtime comparison).

Pure Python 3 (pre-installed on ubuntu-latest and in the ubuntu:26.04 build container).
Bash wrapper provides `set -euo pipefail`, `-h/--help`, and `PUBLISH_DIR` env var.

**Per-package algorithm:**

```
locate packages/{NAME}/debian/control + debian/changelog
out = ${PUBLISH_DIR}/meta/{NAME}.json

# Skip if meta-info is up to date
if out exists AND mtime(out) > mtime(control) AND mtime(out) > mtime(changelog):
  print "meta/{NAME}.json up to date" and skip

parse control Source stanza → homepage
parse binary Package: stanza → name, arch, section, short_desc, long_desc_raw, depends_raw
parse changelog first entry → version

clean depends:   strip ${misc:Depends}, strip "(>= 1.0)" version constraints → list of names
parse long_desc: strip leading space, replace " ." with paragraph break

if ${PUBLISH_DIR}/dists/resolute/main/binary-*/Packages exists:
  find stanza for this package → Installed-Size

write ${PUBLISH_DIR}/meta/{name}.json:
{
  "generated":         "2026-05-28T12:00:00Z",
  "name":              "foundry-daw",
  "version":           "1.0.3",
  "architecture":      "all",
  "section":           "metapackages",
  "homepage":          "https://foundrylinux.org/",
  "depends":           ["ardour", "audacity", "lmms", …],
  "description_short": "Digital Audio Workstation stack and trackers",
  "description_long":  "Full audio-production stack — DAWs, MIDI…\n\n  * ardour…",
  "installed_size_kb": 358400,
  "deb_url":           "/pool/main/f/foundry-daw/foundry-daw_1.0.3_all.deb"
}
```

## 2. `scripts/generate-index.sh` (modified)

Before the parsing loop, ensure all meta files exist (fallback for local preview
without a prior `generate-meta` run):

```bash
mkdir -p "$OUT_DIR/meta"
for pkgdir in "$REPO_ROOT"/packages/*/; do
  name=$(basename "$pkgdir")
  [[ -f "$pkgdir/debian/control" ]] || continue
  if [[ ! -f "$OUT_DIR/meta/${name}.json" ]]; then
    bash "$SCRIPT_DIR/generate-meta.sh" "$name"
  fi
done
```

Replace the current parallel-arrays + awk parser with a Python call that reads all
`public/meta/*.json` files and emits TSV (name, version, arch, homepage,
description_short, description_long, deps pipe-joined, installed_size_kb):

```bash
mapfile -t PKG_DATA < <(
  python3 - "$OUT_DIR/meta" <<'PYEOF'
import json, os, sys
meta_dir = sys.argv[1]
for f in sorted(os.listdir(meta_dir)):
    if not f.endswith(".json"): continue
    p = json.load(open(os.path.join(meta_dir, f)))
    deps = "|".join(p.get("depends") or [])
    print("\t".join([p["name"], p["version"], p["architecture"],
                     p.get("homepage") or "", p["description_short"],
                     p.get("description_long") or "", deps,
                     str(p.get("installed_size_kb") or "")]))
PYEOF
)
```

Extend the description `<td>` to include a `<details>` disclosure (closed by default,
no JS required):

```html
<td class="col-desc">
  Short desc text
  <details class="pkg-details">
    <summary>details</summary>
    <p class="pkg-long">…long description paragraphs (HTML-escaped)…</p>
    <!-- metapackages only, when depends is non-empty: -->
    <div class="pkg-deps">
      <span class="dep">ardour</span>
      <span class="dep">audacity</span> …
    </div>
    <!-- when installed_size_kb present: -->
    <p class="pkg-size">~350 MB installed</p>
  </details>
</td>
```

Omit `<details>` entirely when both `description_long` and `depends` are empty.

Add CSS: `.pkg-details summary` (cursor pointer, muted), `.pkg-long` (small, prose),
`.pkg-deps` (flex-wrap gap), `.dep` (monospace chip — hairline border, `var(--accent)`
tint, small padding), `.pkg-size` (muted, right-aligned or below deps). Mobile card
layout already wraps `col-desc` full-width; `<details>` works unchanged there.

## 3. `Taskfile.yml` (modified)

Add `generate-meta` task mirroring the `build` task's `PKG=` pattern:

```yaml
  generate-meta:
    desc: "Generate public/meta/{name}.json for all packages (PKG=name for one; skips up-to-date)"
    cmds:
      - bash scripts/generate-meta.sh {{.PKG}}
```

`generate-index` gains `generate-meta` as a dep so `task generate-index` covers the
full chain without needing to call scripts manually:

```yaml
  generate-index:
    desc: "Generate public/index.html from package meta files"
    deps: [publish-local, generate-meta]
    cmds:
      - bash scripts/generate-index.sh
```

The in-script fallback loop remains for CI (Task not used there).

## 4. `.github/workflows/publish.yml` (modified)

Insert one step between `Publish to ./public/` and `Sign Release`:

```yaml
      - name: Generate per-package metadata
        run: bash scripts/generate-meta.sh
```

Runs on the host runner (ubuntu-latest, python3 pre-installed). Reads
`packages/*/debian/` (source) and enriches with `Installed-Size` from the
`public/dists/resolute/main/binary-*/Packages` files produced by the prior step.
Skips packages whose meta file is already newer than source.

## 5. `public/meta/` directory

- Lives in `public/` → uploaded by the existing `rclone sync` step automatically
- Each file available at e.g. [`https://apt.foundrylinux.org/meta/f9dasm.json`](https://apt.foundrylinux.org/meta/f9dasm.json)
- Not linked or named anywhere in `index.html`
- Included in CI artifacts (30-day retention)

## Mockups

### Table — default state (all rows collapsed)

```
┌─────────────────────────────┬──────────────────────┬──────────────────────────────────────────┐
│ PACKAGE                     │ VERSION              │ DESCRIPTION                              │
├─────────────────────────────┼──────────────────────┼──────────────────────────────────────────┤
│ blender-asset-finder        │ 0.2.1                │ Blender multi-source asset browser add-on│
│                             │                      │ ▸ details                                │
├─────────────────────────────┼──────────────────────┼──────────────────────────────────────────┤
│ foundry-daw                 │ 1.0.3                │ Digital Audio Workstation stack and      │
│                             │                      │ trackers  ▸ details                      │
├─────────────────────────────┼──────────────────────┼──────────────────────────────────────────┤
│ f9dasm                      │ 1.83-1foundry3       │ Motorola 6800/6809/6309 family           │
│                             │ (amd64)              │ disassembler  ▸ details                  │
└─────────────────────────────┴──────────────────────┴──────────────────────────────────────────┘
```

### Table — metapackage row expanded (foundry-daw)

The annotated list comes from `Description:` long-text in `debian/control`; the dep
chips come from the cleaned `Depends:` field.

```
├─────────────────────────────┼──────────────────────┼──────────────────────────────────────────┤
│ foundry-daw                 │ 1.0.3                │ Digital Audio Workstation stack and      │
│                             │                      │ trackers  ▾ details                      │
│                             │                      │ ┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈ │
│                             │                      │ Full audio-production stack — DAWs,      │
│                             │                      │ MIDI sequencers, drum machines, the DJ   │
│                             │                      │ tool, and the chiptune trackers:         │
│                             │                      │                                          │
│                             │                      │   * ardour       multitrack DAW          │
│                             │                      │   * audacity     waveform editor         │
│                             │                      │   * lmms         FL Studio-like          │
│                             │                      │   * rosegarden   MIDI + audio sequencer  │
│                             │                      │   * hydrogen     drum machine            │
│                             │                      │   * qtractor     MIDI/audio sequencer    │
│                             │                      │   * mixxx        DJ interface            │
│                             │                      │   * milkytracker Amiga MOD tracker       │
│                             │                      │   * schism       Impulse Tracker clone   │
│                             │                      │   * furnace      multi-chip chiptune     │
│                             │                      │   * openmpt123   module-format player    │
│                             │                      │                                          │
│                             │                      │ INSTALLS                                 │
│                             │                      │ [ardour] [audacity] [lmms] [rosegarden]  │
│                             │                      │ [hydrogen] [qtractor] [mixxx]            │
│                             │                      │ [milkytracker] [schism] [furnace]        │
│                             │                      │ [openmpt123]                             │
│                             │                      │                                          │
│                             │                      │ ~350 MB installed                        │
├─────────────────────────────┼──────────────────────┼──────────────────────────────────────────┤
```

### Table — binary package row expanded (f9dasm)

Binary packages have no dep chips — just prose paragraphs from the long description.

```
├─────────────────────────────┼──────────────────────┼──────────────────────────────────────────┤
│ f9dasm                      │ 1.83-1foundry3       │ Motorola 6800/6809/6309 family           │
│                             │ (amd64)              │ disassembler  ▾ details                  │
│                             │                      │ ┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈ │
│                             │                      │ f9dasm is a portable disassembler for    │
│                             │                      │ the Motorola 6800, 6801, 6802, 6803,     │
│                             │                      │ 6808, 6809 and Hitachi 6301, 6303, 6309  │
│                             │                      │ microprocessors. Reads Intel Hex,         │
│                             │                      │ Motorola S-record, FLEX9 binary, and raw │
│                             │                      │ binary; emits TSC assembler-compatible   │
│                             │                      │ output.                                  │
│                             │                      │                                          │
│                             │                      │ Three companion utilities included:       │
│                             │                      │ hex2bin, mot2bin, cmd2mot.               │
│                             │                      │                                          │
│                             │                      │ ~680 KB installed                        │
├─────────────────────────────┼──────────────────────┼──────────────────────────────────────────┤
```

### Mobile card — collapsed / expanded

```
┌──────────────────────────────────┐   ┌──────────────────────────────────┐
│ foundry-daw                      │   │ foundry-daw                      │
│ 1.0.3                            │   │ 1.0.3                            │
│ Digital Audio Workstation stack  │   │ Digital Audio Workstation stack  │
│ and trackers  ▸ details          │   │ and trackers  ▾ details          │
│                                  │   │ ┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈ │
│                                  │   │ Full audio-production stack…     │
│                                  │   │                                  │
│                                  │   │   * ardour       multitrack DAW  │
│                                  │   │   * audacity     waveform editor │
│                                  │   │   * lmms         FL Studio-like  │
│                                  │   │   …                              │
│                                  │   │                                  │
│                                  │   │ INSTALLS                         │
│                                  │   │ [ardour] [audacity] [lmms]       │
│                                  │   │ [rosegarden] [hydrogen]…         │
│                                  │   │ ~350 MB installed                │
└──────────────────────────────────┘   └──────────────────────────────────┘
       collapsed                                   expanded
```

**Notes:**
- `▸ details` / `▾ details` is the native HTML `<details>`/`<summary>` — no JS
- Dep chips `[ardour]` are `<span class="dep">` — monospace pill, hairline border
- Long desc prose rendered without chip section for non-metapackages
- `~N MB installed` shown only when `installed_size_kb` present in JSON

## Verification

1. ```bash
   cd foundry-apt
   task build
   task generate-meta
   python3 -m json.tool public/meta/foundry-daw.json
   python3 -m json.tool public/meta/f9dasm.json
   ```
   Confirm: valid JSON, `depends` array clean (no `${misc:Depends}`, no version
   constraints), `description_long` non-empty, `description_short` matches first
   `Description:` line.

2. Re-run and confirm timestamp skip fires:
   ```bash
   task generate-meta   # second run: all "up to date"
   task generate-meta PKG=foundry-daw   # single-package mode
   ```

3. ```bash
   task publish-local   # builds .debs + publishes → public/
   task generate-meta   # now has Packages file → installed_size_kb populated
   python3 -m json.tool public/meta/f9dasm.json | grep installed_size_kb
   task generate-index
   ```
   Open `public/index.html` in browser. Confirm: each row has `▸ details`; expanding
   foundry-daw shows annotated bullet list + dep chips; expanding f9dasm shows prose;
   `~N MB installed` appears; no `meta/` path or JSON filename visible anywhere on page.

4. Fallback test:
   ```bash
   rm -rf public/meta
   bash scripts/generate-index.sh   # must regenerate all meta files then succeed
   ```

5. Validate CI: trigger `workflow_dispatch` with `dry_run: true`. Confirm
   `Generate per-package metadata` step appears in log between publish and sign steps,
   python3 needs no installation, no errors.
