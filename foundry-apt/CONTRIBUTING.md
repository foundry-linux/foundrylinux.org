# Contributing to foundry-apt

Thanks for considering a contribution! This repo is the signed [APT repo](https://wiki.debian.org/DebianRepository) + metapackage source for [Foundry Linux](https://foundrylinux.org).

## What to contribute

| Most useful | Examples |
|---|---|
| **Bug fixes** | A metapackage `Depends:` line references the wrong package; CI runs out of disk; sign.sh fails on a key with a passphrase. |
| **New vendored upstream packages** | A new tool we want shipped (`vgmstream`, `libvgm`, Ghidra…) or a bump of an existing one (`f9dasm`). Each goes through the [`/package`](#) Claude Code skill which generates the Debian source-package tree. |
| **New metapackages** | A focused bundle (e.g. `foundry-linux-shaders-dev`) that pulls in a curated set of system deps. |
| **CI hardening** | shellcheck-clean scripts; arm64 build matrix; a `--dry-run` mode for sign.sh that uses a throwaway GPG key. |

## Workflow

1. **Open an issue first** for anything bigger than a one-line fix — it's cheaper to argue about scope before code than after.
2. Fork → branch from `main` → push → open a PR. The [test workflow](.github/workflows/test.yml) runs shellcheck + builds every `.deb` + checks the dependency closure. PRs that fail CI don't get reviewed.
3. **Don't bump versions in your PR** — the maintainer bumps them at tag time so versioning stays linear.
4. Sign your commits if you can (`git commit -S`). Not required, encouraged.

Every package in this repo uses the **canonical Debian source-package layout** — `packages/<name>/debian/{control,changelog,rules,source/format,copyright}` — built with `dpkg-buildpackage`. That's the same layout Debian and Ubuntu use for every package in their archives. We don't author uppercase `DEBIAN/control` (it's a *generated* artifact inside built `.deb`s, not authored input).

## Adding a metapackage

1. `mkdir -p packages/foundry-linux-<thing>/debian/source`
2. Create `debian/control`:

    ```
    Source: foundry-linux-<thing>
    Section: metapackages
    Priority: optional
    Maintainer: World Foundry <packages@worldfoundry.org>
    Build-Depends: debhelper-compat (= 13)
    Standards-Version: 4.7.0
    Homepage: https://foundrylinux.org/
    Rules-Requires-Root: no

    Package: foundry-linux-<thing>
    Architecture: all
    Depends: ${misc:Depends},
     <your-deps-here>
    Description: <one-line summary>
     <multi-line description, each continuation line begins with single space>
    ```

3. Create `debian/changelog` (use `dch --create -v 1.0.0 -D resolute --package foundry-linux-<thing>`, or hand-write the first entry).
4. Create `debian/rules` (executable, mode 0755):

    ```make
    #!/usr/bin/make -f
    %:
    	dh $@
    ```

5. Create `debian/source/format`:

    ```
    3.0 (native)
    ```

6. Create `debian/copyright` (DEP-5 format — see existing metapackages for the MIT-for-packaging boilerplate).
7. `task build` to verify it builds; `task verify` to inspect the produced `.deb`.
8. Update `packages/foundry-linux-dev/debian/control` if the new package should be pulled in by the top-level metapackage.
9. Mention it in `README.md`'s table.

## Adding a vendored upstream package

**Step 0 — always check Ubuntu universe first.** Run `apt-cache policy <pkg>` on a fresh `ubuntu:26.04` container. If universe ships it, the right move is to add it to the Phase 0 apt-install list and the metapackage `Depends:` — don't duplicate. (xa65 turned out to already be in 26.04 universe; we wasted a packaging cycle before catching it.)

For packages *genuinely* missing from Ubuntu, use the [`/package`](#) Claude Code skill (`~/.claude/skills/package/SKILL.md`). It:

1. Re-checks Ubuntu universe (blocking).
2. Vendors the upstream tarball (sha256-pinned).
3. Generates the `debian/` source tree via `dh_make`.
4. Patches in Foundry-customized fields (Maintainer, Section, etc.) from the templates.
5. Writes `packages/<name>/{build.sh, debian/}` — `build.sh` is a thin wrapper that fetches the tarball, overlays our `debian/`, and runs `dpkg-buildpackage -us -uc -b`.

The skill gives you a Debian-policy-compliant package for free: `dh_strip` runs by default (debug info stripped), `dh_shlibdeps` resolves `${shlibs:Depends}` accurately, build hardening flags (PIE, stack-protector, FORTIFY_SOURCE) come from `/usr/share/dpkg/buildflags.mk`, and the resulting source package is exportable to a PPA.

If you can't use `/package` (no Claude Code, working offline, etc.), the manual steps are the same as what the skill does — see [`docs/plans/2026-05-18-package-skill.md`](../docs/plans/2026-05-18-package-skill.md) for the exact pipeline and [`docs/plans/2026-05-18-canonical-debian-layout.md`](../docs/plans/2026-05-18-canonical-debian-layout.md) for the layout specifics.

After packaging:

1. `task build` to verify; `dpkg-deb -I` shows resolved `${shlibs:Depends}` with version constraints; `file usr/bin/<binary>` reports "stripped".
2. `lintian dist/<name>_*.deb` — should be clean before committing.
3. Add the upstream's licence to `LICENSES-VENDORED.md`.
4. Add the new package to the relevant metapackage's `debian/control` `Depends:` (and bump the metapackage's `debian/changelog` with `dch -v <new-ver> ...`).

## Releasing

Maintainer-only:

1. `git tag -s v1.0.X -m "..."` (signed tag).
2. `git push origin v1.0.X`.
3. Watch [the publish workflow](.github/workflows/publish.yml) — it builds, signs, syncs to R2, and smoke-installs `foundry-linux-dev` in a clean Ubuntu 26.04 container.
4. If smoke-install fails, the apt repo is still uploaded — fix forward with `v1.0.X+1`, don't try to delete a published version.

## Licence

By contributing you agree your work is licensed under [GPL-2.0](LICENSE) (matching the rest of the project).
