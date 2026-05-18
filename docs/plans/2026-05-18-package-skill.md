# Plan: `/package` skill + new-web-apt-repo skill fixes

**Date:** 2026-05-18
**Status:** In progress

## Context

[`2026-05-18-package-xa65.md`](2026-05-18-package-xa65.md) packaged xa65 as a `.deb` via a hand-rolled `build.sh` (fetch → make → stage → `dpkg-deb --build`). It works, but comparing to Ubuntu's xa65 package reveals it falls short of Debian packaging policy on several axes:

| Metric | Ours (build.sh) | Ubuntu (debhelper) | Why theirs is better |
|---|---|---|---|
| `.deb` size | 223 KB | 95 KB | We never stripped debug info |
| Installed-Size | 564 KB | 296 KB | Same — `dh_strip` runs by default |
| `xa` binary size | 322 KB | 109 KB | We shipped unstripped binaries |
| `Depends:` libc6 | bare `libc6` | `libc6 (>= 2.38)` | `dpkg-shlibdeps` auto-resolves the actual symbol-version floor we need; bare `libc6` installs on incompatible systems then segfaults at runtime |
| Build hardening (PIE, stack-protector, FORTIFY_SOURCE) | none | yes (via `/usr/share/dpkg/buildflags.mk`) | Security baseline |
| `debian/copyright` format | one-line free text | machine-readable DEP-5 | Tooling-parseable; legal clarity |
| `debian/watch` (upstream tracker) | none | uscan format | Bumping versions = `uscan` + `dch -v` |
| `debian/source/format` | none (no source pkg) | `3.0 (quilt)` | Source package exportable to a PPA |
| Quilt patches dir | none | `debian/patches/series` | We can carry distro fixes without forking upstream |
| `Rules-Requires-Root: no` | absent | present | Debian best practice; faster builds |

Ubuntu's full `debian/rules` is **10 lines** because `dh $@` dispatches to ~40 debhelper helpers (auto-strip, auto-compress-docs, auto-shlibdeps, auto-fixperms, auto-installman, etc.). Our `build.sh` re-implements a fraction of this by hand and misses the rest.

## Goal

Replace the ad-hoc `build.sh` pattern with a `/package` skill that produces **Debian-policy-compliant** packages via `dh_make` + `dpkg-buildpackage`. Use it for the next four retro tools (f9dasm, libvgm, vgmstream, ghidra), then back-port xa65 to it (or rip xa65 once the universe-fallback path lands — see [`2026-05-18-package-xa65.md`](2026-05-18-package-xa65.md) postscript).

## Inputs accepted

The skill takes **one of** these forms (auto-detect by shape):

| Form | Example | What the skill does |
|---|---|---|
| Program name | `/package xa65` | Search `apt-cache`, GitHub, Debian QA tracker; surface candidates and ask which is upstream |
| GitHub URL | `/package https://github.com/Arakula/f9dasm` | `gh api` for latest tag/release; download archive tarball |
| Release tarball URL | `/package https://example.org/foo-1.2.3.tar.gz` | Download + sha256 pin; treat as the orig tarball |
| Debian source .dsc URL | `/package https://deb.debian.org/.../foo_1.2.3-1.dsc` | `dget` to fetch the existing source package; re-pack with Foundry revision |
| Local source tree | `/package /home/me/src/foo` | Treat as `orig` tree; assume it's at the right tag |

Args after the first are name/version overrides: `/package <input> name=<deb-name> upstream=<ver> revision=<deb-rev>`.

## Pipeline

```
┌─────────────────────────────────────────────────────────────────────────┐
│ 1. Parse input + resolve upstream                                       │
│    - github → tag → archive URL                                         │
│    - name → search and ask                                              │
│    - already a .dsc → dget                                              │
│    - local tree → tar -czf upstream.orig.tar.gz                         │
├─────────────────────────────────────────────────────────────────────────┤
│ 2. Universe check (LEARNED THE HARD WAY)                                │
│    docker run ubuntu:26.04 apt-cache policy <name>                      │
│    If candidate is non-empty → STOP, print:                             │
│      "xxxxxx is already in Ubuntu universe as <ver>."                   │
│      "Add it to the apt-install list in install-<metapackage>.sh and    │
│       to the metapackage's Depends:. Don't duplicate."                  │
├─────────────────────────────────────────────────────────────────────────┤
│ 3. Vendor the source                                                    │
│    - Download tarball, sha256sum, pin in packages/<name>/build.sh       │
│    - tar layout becomes <name>-<upstream>/                              │
│    - Save .orig.tar.gz alongside for source-package export              │
├─────────────────────────────────────────────────────────────────────────┤
│ 4. Generate debian/ tree                                                │
│    cd <name>-<upstream> && dh_make --createorig --packagename           │
│        <name>_<upstream> --single --copyright gpl2 --yes                │
│    Then patch the generated files:                                      │
│      debian/control     ← Foundry maintainer, Description, Depends      │
│      debian/copyright   ← DEP-5 with upstream copyright + GPL-2 text    │
│      debian/changelog   ← initial Foundry release entry                 │
│      debian/rules       ← keep minimal "dh $@"; add overrides if needed │
│      debian/watch       ← uscan tracker for future version bumps        │
│      debian/source/format = 3.0 (quilt)                                 │
│      remove example/dummy files dh_make creates                         │
├─────────────────────────────────────────────────────────────────────────┤
│ 5. Build                                                                │
│    dpkg-buildpackage -us -uc -b   # binary-only, unsigned               │
│    (CI may build -S source pkg too, for PPA export)                     │
│    dh_strip runs by default → debug info gone, binaries stripped        │
│    dh_shlibdeps runs by default → ${shlibs:Depends} resolved            │
├─────────────────────────────────────────────────────────────────────────┤
│ 6. Verify                                                               │
│    dpkg-deb -I / -c sanity                                              │
│    lintian (if installed)                                               │
│    Docker apt-install + functional smoke                                │
├─────────────────────────────────────────────────────────────────────────┤
│ 7. Wire into foundry-apt                                                │
│    - Copy resulting .deb to dist/ (or build-all.sh does this)           │
│    - Add packages/<name>/{build.sh,debian/}                             │
│    - Bump metapackage version + add to Depends                          │
│    - Commit, task sync, task bump                                       │
└─────────────────────────────────────────────────────────────────────────┘
```

## What changes in `packages/<name>/`

| Old layout (xa65 today) | New layout (post-skill) |
|---|---|
| `build.sh` (fetch + make + dpkg-deb) | `build.sh` (small wrapper: dpkg-buildpackage) |
| `DEBIAN/control` (static index metadata) | `debian/control` (Debian source format) |
| — | `debian/changelog` |
| — | `debian/copyright` (DEP-5) |
| — | `debian/rules` (`dh $@`) |
| — | `debian/source/format` |
| — | `debian/watch` |
| — | `debian/patches/series` |

**Generate-index.sh impact:** the index generator currently parses `packages/*/DEBIAN/control`. With the new layout, the file is at `packages/*/debian/control` (lowercase d, no source-package metadata). Two options:

1. **Update generate-index.sh** to parse `debian/control` first, fall back to `DEBIAN/control` for legacy packages.
2. **Keep a hybrid:** generate-index.sh reads from a tiny `packages/*/index.info` file or from aptly's published Packages list.

Decision: pick **option 1** (parse `debian/control`) — it's a one-line change and reads the actual source-package metadata, which is more authoritative.

`debian/control`'s field names are slightly different from the binary control: `Source:` instead of `Package:`, and there are *multiple* binary stanzas (separated by blank lines). generate-index.sh will parse the binary stanzas, ignoring the `Source:` stanza at top.

## Debian policy compliance — what we get, what we skip

**What we get for free (debhelper):**

- `dh_strip` → debug-info stripped from binaries
- `dh_shlibdeps` → `${shlibs:Depends}` accurately resolved
- `dh_fixperms` → man pages 0644, binaries 0755
- `dh_compress` → docs and man pages gzipped
- `dh_installdocs` → README/NEWS/etc into `/usr/share/doc/<pkg>/`
- `dh_installman` → man pages into the right section
- `dh_installchangelogs` → changelog handled
- Build flags: `-fPIC -Wformat -Werror=format-security -D_FORTIFY_SOURCE=2 -fstack-protector-strong` (via `dpkg-buildflags`)
- Reproducible builds (SOURCE_DATE_EPOCH respected by `dh`)
- `lintian`-clean structure (Debian's QA linter)

**What we *don't* get and why:**

- **Upload to Debian archive proper.** That requires a Debian maintainer to sponsor + dput; outside our scope. We host on R2.
- **`debian/control` with `Maintainer: Debian QA Group`-style addresses.** We use `Foundry Linux <packages@foundrylinux.org>` — that's a third-party repo convention, not a policy violation per se.
- **Strict `Standards-Version` compliance audits.** We pin `Standards-Version: 4.7.0` (current Debian Policy at time of writing). Lintian will flag drift; we bump per upstream Debian.
- **Multi-distribution support (Debian sid, testing, stable, Ubuntu).** Our `debian/changelog` targets `resolute` (Ubuntu 26.04). Source package is exportable to a PPA if we ever want Debian/older Ubuntu compatibility; cross-distro builds become a tag-and-build-matrix issue.

If we ever want to ship to a PPA, the skill's `dpkg-buildpackage -S` output is already PPA-ready. That's the *value* of going Debian-native — we keep the door open.

## new-web-apt-repo skill — what's wrong

Two gaps surfaced while debugging the xa65 release:

1. **No "consume this repo" instructions.** The skill walks through bootstrap but never tells a downstream user *how to add the published repo to apt sources*. We had to guess the keyring URL (it's `/key.gpg`; we tried 7 wrong paths first). The fix: add a section near the end documenting:

    ```bash
    install -d /etc/apt/keyrings
    curl -fsSL https://<CUSTOM_DOMAIN>/key.gpg \
      | gpg --dearmor -o /etc/apt/keyrings/<slug>.gpg
    echo "deb [signed-by=/etc/apt/keyrings/<slug>.gpg] https://<CUSTOM_DOMAIN> <SUITE> main" \
      | sudo tee /etc/apt/sources.list.d/<slug>.list
    sudo apt-get update
    ```

    With concrete `<SUITE>` value (e.g. `resolute` for Ubuntu 26.04; whatever's in `aptly/aptly.conf`).

2. **Static-control vs build.sh vs debian/ source format gotcha.** The skill assumes packages have `packages/<name>/DEBIAN/control` (capital DEBIAN, binary-package format). It doesn't account for:
   - `build.sh`-driven packages (generated control at build time → invisible to generate-index.sh until we shipped a duplicate static `DEBIAN/control` for xa65)
   - `debian/` source-format packages (lowercase debian, source-package format, multiple binary stanzas)

   Fix: rewrite Step 5.5 ("Verify landing page") to call out the three layouts and recommend `debian/` (source format) via the `/package` skill.

## Verification

1. **Skill discoverable.** `~/.claude/skills/package/SKILL.md` exists with valid YAML frontmatter; `name: package`, `description:` includes trigger phrases ("package this", "build a deb", "vendor and package").

2. **Universe-check abort path.** Run the skill against `xa65` (which we know is in universe). Expected output: stop with message naming the universe version, suggest the apt-install-list alternative, do NOT generate a debian/ tree.

3. **Generate path against a not-in-universe target.** Run the skill against `f9dasm` (next in TODO, confirmed not in universe). Expected output:
   - Vendors `github.com/Arakula/f9dasm` tarball at sha256-pinned tag
   - Generates `packages/f9dasm/{build.sh, debian/{control,changelog,copyright,rules,source/format,watch}}` with Foundry-customized fields
   - `dpkg-buildpackage -us -uc -b` produces `f9dasm_<ver>-1foundry1_amd64.deb`
   - `dpkg-deb -I` shows `Depends: libc6 (>= X.Y), ...` (resolved by shlibdeps, not hardcoded)
   - `xa` binary inside is stripped (`file` says "stripped")
   - `lintian -i` reports no errors (warnings allowed but documented)
   - Docker install from local .deb + apt install from live repo both succeed

4. **Compared to manual approach** — `.deb` size for f9dasm should be 2-3× smaller than a hypothetical hand-rolled equivalent (debug info stripped + shared-symbol-table compression).

5. **new-web-apt-repo skill updated.** Re-read the skill end-to-end; "consume this repo" snippet present with correct `/key.gpg` URL + `<SUITE>` placeholder.

6. **generate-index.sh handles `debian/control`.** After f9dasm ships via the new layout, the live index page shows f9dasm with the correct version + .deb sub-link.

## Out of scope (next plans)

- **Multi-arch builds via CI matrix.** Skill produces native-arch builds. arm64 cross-compile is a CI step, not a skill step.
- **PPA upload.** Skill output is PPA-ready but actually pushing to a PPA is a separate step.
- **xa65 retrofit.** Don't migrate xa65 to the new layout; rip it when the Phase 0 apt-source TODO lands (see [`2026-05-18-package-xa65.md`](2026-05-18-package-xa65.md) postscript).

## Open questions (decide while implementing)

- **Where does the skill live?** User-level (`~/.claude/skills/package/`) for cross-project reuse, OR project-local (`.claude/commands/package.md`) so the conventions stay project-specific. → **user-level**, because the packaging machinery is the same regardless of which apt repo it feeds.
- **Single SKILL.md or templates/ dir?** dh_make generates most files; only `debian/control`, `debian/copyright`, `debian/changelog` need templating. → **templates/** with three files; SKILL.md does the orchestration.
- **Does the skill run `dh_make` or template-everything itself?** `dh_make` requires upstream layout sniffing (Makefile vs autotools vs cmake vs cargo) — running it gets that for free. → **run dh_make then patch its output**.
