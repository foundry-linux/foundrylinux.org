# Plan: publish source packages from apt.foundrylinux.org

## Context

[`2026-06-21-repology-tracking-foundry-apt.md`](../investigations/2026-06-21-repology-tracking-foundry-apt.md)
found that every Debian/Ubuntu derivative Repology tracks parses a **source** index
(`DebianSourcesParser` on `Sources.gz`) — we publish **binary only**. Independently, a real
apt repo should ship source packages so users can `apt-get source <pkg>` and rebuild
(reproducibility mandate; transparency). Will: build source packages too — **prove it on one
foundry-apt package, then roll out to all** — and bake it into the `/package` +
`new-web-apt-repo` skills.

Sibling change (already landed): the aptly publish now sets `Origin`/`Label` = `Foundry Linux`
(`publish-local.sh`), enforced by `scripts/check-publish-metadata.sh` + a Claude PostToolUse
hook.

## Current state

- `scripts/build-all.sh` runs `dpkg-buildpackage -us -uc -b -d --no-sign` — **`-b` =
  binary-only**, so `dist/` gets only `.deb`s.
- `scripts/publish-local.sh` does `aptly repo add dist/` then `aptly publish repo`. aptly
  emits a `Sources` index **iff** source packages were added — currently none.
- Two build paths: **(a)** canonical Debian layout (metapackages + simple, mostly
  `3.0 (native)`); **(b)** `packages/<name>/build.sh` wrapper (vendored upstreams, mostly
  `3.0 (quilt)`).

## The crux — source-format requirements

| `debian/source/format` | What a source build needs | Our packages |
|---|---|---|
| `3.0 (native)` | nothing extra — tars the whole tree | metapackages (`foundry-*`) |
| `3.0 (quilt)` | `<src>_<upstreamver>.orig.tar.{gz,xz}` in the **parent** dir | vendored upstreams |

For quilt, `build.sh` already fetches the upstream tarball (sha256-pinned) — the work is
placing/naming it as the `.orig.` tarball `dpkg-source` expects. **Binary-repack** packages
(ghidra, mesen2, m8te, ldtk — pre-built zips) will produce a source package whose "source" is
the repack tree, not true upstream source; acceptable (it's our packaging + the redistributed
binary), but note it.

## Approach (incremental)

### Phase 1 — prove it on ONE package (the skill test case)

Pick a representative **vendored `3.0 (quilt)`** package — `halfempty` (small, recent,
Apache-2.0, I know its build) — and run it end to end in `ubuntu:26.04`:

1. Build a source package (`.dsc` + `.orig.` + `.debian.tar`) alongside the binary.
2. `dpkg-source -x foo.dsc` round-trips cleanly; `lintian` on the `.dsc` is clean.
3. `aptly repo add` the `.dsc`, publish, and confirm `dists/resolute/main/source/Sources.gz`
   exists and lists `halfempty`.
4. `apt-get source halfempty` against the local repo fetches + unpacks.

Also sanity-check **one `3.0 (native)`** metapackage (e.g. `foundry-retro-tools`) — should be
trivial (no orig tarball).

### Phase 2 — roll out to ALL packages

- `build-all.sh`: build the source package for every package (switch `-b` → build source too,
  e.g. add a `dpkg-buildpackage -S` pass or use `-F`); ensure the `.orig.` tarball is present
  for quilt in **both** build paths; land `.dsc` + tarballs in `dist/`.
- `publish-local.sh`: aptly auto-publishes sources once they're in the repo — verify the
  `Sources` index appears; no flag change expected (confirm).
- `publish.yml` (CI): the in-container `build-all.sh` now emits source artifacts → they sync
  to R2 with the rest. Verify the published `Release` gains `main/source/Sources`.
- `prune-dist.sh` / `generate-*`: confirm they tolerate `.dsc`/tarballs in `dist/` (don't
  prune or choke on non-`.deb` artifacts).

### Phase 3 — skills

- **`/package` skill** (`~/.claude/skills/package/SKILL.md`): Step 4 — build a source package
  (handle the quilt `.orig.` tarball); Step 6 — sources publish via aptly; add the
  **Origin/Label = vendor-name** publish convention (generic, both repos).
- **`new-web-apt-repo` skill**: scaffold source build + publish + Origin/Label for new repos.

## Verification

Run from `foundry-apt/`. Keep these steps; paste raw output + PASS/FAIL.

1. **One package, source build** — in `ubuntu:26.04`, `halfempty` yields
   `halfempty_*.dsc` + `.orig.` + `.debian.tar.*`; `dpkg-source -x` round-trips; `lintian`
   on the `.dsc` is clean.
2. **Sources index published** — `publish-local.sh` →
   `public/dists/resolute/main/source/Sources.gz` exists and lists the test package(s).
3. **`apt-get source` works** — against `file://…/public`, `apt-get source halfempty`
   fetches + unpacks with a verified checksum.
4. **Full build** — `build-all.sh` emits a `.dsc` for every package (or a logged, documented
   exception for a binary-repack); `dist/` has matching `.dsc`/tarballs.
5. **Release advertises sources** — published `Release` lists `main/source/Sources` (md5 +
   sha256/512), alongside the existing `binary-*`.
6. **Repology parse dry-run** — `DebianSourcesParser` (or a manual parse) reads our
   `Sources.gz` without error; source names map to the projects in our
   `X-Repology-Project` fields.
7. **Skills updated** — `/package` Step 4/6 + `new-web-apt-repo` instruct source build +
   publish + Origin/Label.

## Risks

- **Quilt `.orig.` tarball** availability/naming across every `build.sh` — the main unknown;
  Phase 1 derisks it.
- **Binary-repack packages** — source semantics; may need a per-package note or
  `--no-source` exception.
- **`dpkg-source` strictness** — uncommitted changes, file permissions, `.gitignore`d build
  cruft in the tree can break source builds that binary builds tolerate.
- **CI artifact size** — re-hosting `.orig.` tarballs grows the R2 footprint (modest).

## Rollout / commits

Phase 1 (proof, one package) → commit · Phase 2 (build-all + publish, all) → commit ·
Phase 3 (skills) → global skill edits (outside the repo). Per
`feedback_start_build_immediately`, an ISO build is **not** triggered — this changes the apt
repo's source index, not any `.deb` shipped onto the ISO.

## Decision log

- **2026-06-21 — pursue source publishing** (Will), off the back of the Repology
  investigation; prove on one package first, then all.
