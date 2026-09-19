# Add `uv` to the base installation

## Context

`uv` (Astral's Python package and project manager) is not part of any Foundry Linux
edition today. The base toolkit `foundry-core` pulls a full Python game‑dev stack
(`foundry-python-gamedev` → numpy, pygame, pytest, `ruff`, …) but the only way to install
a Python tool outside apt is `pipx`, which two Phase 0 scripts already lean on:
`install-foundry-retro-tools.sh` (`pipx install shrinkray`) and
`install-foundry-ios-development.sh` (`pipx install codemagic-cli-tools`). `uv` covers
that use (`uv tool install`), plus venvs, lockfiles, `uvx` one‑shot runs and managed
interpreters, and is the de‑facto standard the rest of the Python ecosystem now assumes.

**It cannot come from Ubuntu.** Checked on a fresh `ubuntu:26.04` container on
2026‑09‑19: `apt-cache policy uv` has no candidate, and the Launchpad primary archive
lists zero published `uv` sources for resolute. Debian does carry it in sid/forky
(`uv 0.9.17+ds1-6`, a from‑source build by the Rust team) but that never reaches an
already‑released LTS, and it is fifteen minor versions behind upstream. So for the life
of the 26.04 base it is a vendored upstream, exactly like `ruff`.

**Upstream ships no Debian packaging** (the audit in
[2026-08-05-audit-upstream-packaging.md](2026-08-05-audit-upstream-packaging.md)
step 2.5): Astral publishes a curl‑bash installer, cargo‑dist tarballs and PyPI wheels.
The manylinux wheel is the right source artifact, for the same reasons the ruff plan
([2026-05-30](2026-05-30-package-glfw-pydub-ruff-librosa-mss.md)) chose it: sha256 on
PyPI, no Rust toolchain in CI, stripped release binary. Verified today on
`uv 0.12.17` (released 2026‑09‑18):

| Fact | Value |
|---|---|
| Wheel | `uv-0.12.17-py3-none-manylinux_2_17_x86_64.manylinux2014_x86_64.whl` |
| sha256 | `9e25bb39e1674799c408345a6397ebc2c7c719d498be0ce9d935466d36ceacf5` |
| Download | 20.4 MB (wheel), 50.2 MB (`uv` binary) + 0.3 MB (`uvx`) unpacked |
| Layout | `uv-0.12.17.data/scripts/{uv,uvx}` — same `.data/scripts/` shape as ruff |
| Links | libc, libm, libdl, libpthread, librt, libgcc_s only — runs as‑is on 26.04 |
| Licence | `MIT OR Apache-2.0`; both texts and a CycloneDX SBOM ship inside the wheel |

Outcome: every edition (`foundry-core` and everything above it), the Phase 2 devbox and
the Phase 3 ISO carry `uv` and `uvx` on `PATH`, and the Phase 0 installer puts it on an
existing Ubuntu install for every role, including the legacy default role `both`.

## Approach

No visible surface (a `.deb`, a `Depends:` line and a shell wrapper), so no mockup
bundle; the scaffolded one was deleted.

**1. Vendor `uv` as `foundry-apt/packages/uv/` — clone the `ruff` package.** Run
`/package uv` for the universe check and `dh_make` scaffold, then make the result match
[`packages/ruff/`](../../foundry-apt/packages/ruff/) file‑for‑file:

- `build.sh`: copy ruff's verbatim, rename the `RUFF_*` env overrides to `UV_VERSION` /
  `UV_SHA256`, pin the wheel URL and sha256 from the table above. Keep the
  `emit_source_package … || true` call so the Sources index gets a `.dsc`.
- `debian/control`: `Source: uv`, `Section: devel`, `Architecture: amd64` (the repo
  publishes amd64 only; the aarch64 wheel exists if that ever changes),
  `Homepage: https://docs.astral.sh/uv`, `X-Repology-Project: uv`,
  `Build-Depends: debhelper-compat (= 13), unzip`,
  `Depends: ${shlibs:Depends}, ${misc:Depends}`, `Recommends: python3` (uv will
  download a managed interpreter without it, which is not what an Ubuntu user expects).
- `debian/rules`: ruff's, with `UPSTREAM = 0.12.17` and two `install -D` lines
  (`uv`, `uvx`). Keep the empty `override_dh_strip` (upstream already strips). Add an
  `override_dh_auto_build` that runs the freshly unpacked binary to emit shell
  completions — `uv generate-shell-completion {bash,zsh,fish}` and
  `uvx --generate-shell-completion {bash,zsh,fish}` — into `debian/tmp/`, installed via
  `debian/uv.install` to `usr/share/bash-completion/completions/`,
  `usr/share/zsh/vendor-completions/`, `usr/share/fish/vendor_completions.d/`. This is
  safe because CI builds in an amd64 `ubuntu:26.04` container and the binary runs there
  (verified locally).
- `debian/man/uv.1` and `debian/man/uvx.1` listed in `debian/uv.manpages` — hand‑authored
  from `--help` like ruff's, per `/package` step 7. Not optional: lintian errors on a
  binary without a man page.
- `debian/copyright`: DEP‑5, `Upstream-Contact: Astral Software <support@astral.sh>`,
  `Files: * / License: MIT or Apache-2.0` with both texts; `Files: debian/*` MIT.
- `debian/changelog`: `uv (0.12.17-1foundry1) resolute; urgency=medium`.
- `debian/source/format`: `3.0 (quilt)`. `debian/watch`: ruff's, pointed at
  `astral-sh/uv` tags.

**2. Put it in the base metapackage.** Add `uv,` to `Depends:` in
[`packages/foundry-core/debian/control`](../../foundry-apt/packages/foundry-core/debian/control)
directly, beside `task` and `btop`, and mention it in the long description (`go-task,
uv, btop and dust …`). Bump `foundry-core` to `1.0.7` with `dch -v 1.0.7 -D resolute`.
Rejected alternative: hanging it off `foundry-python-gamedev` next to `ruff`. That
package is a *library* stack; `uv` is base tooling in the same class as `task`, and
belongs where a reader looks for base tooling.

**3. Phase 0 wrapper for the legacy roles.** Editions (`anvil`/`sprite`/`atelier`) get
`uv` transitively through `install_edition`, but `install.sh`'s default role `both` and
the `game-dev`/`engine-dev` roles never install `foundry-core`; they chain
`install-task.sh`. Add `foundry-setup/install-uv.sh` as a copy of
[`install-task.sh`](../../foundry-setup/install-task.sh) that runs
`setup-foundry-apt-source.sh` then `apt-get install -y uv` (idempotent: exit early if
`command -v uv`; `--help` before anything else; `-n/--dry-run`; source `lib.sh` with the
inline shim fallback; `apt_update` not raw `apt-get update`). Chain it wherever
`install-task.sh` is chained: `install-foundry-dev.sh` line 94 and the `game-dev` /
`engine-dev` cases in `install.sh`'s `install_metapackages`.

**4. Devbox and ISO pick it up through `foundry-core`; only the checks change.** Add
`uv uvx` to `TOOLS` in
[`foundry-devbox/test/smoke-test.sh`](../../foundry-devbox/test/smoke-test.sh) and a
`├── uv` line to the dependency tree comment in `foundry-devbox/Dockerfile`. The ISO
needs nothing: `task iso-sync-local-debs` copies every `dist/*.deb` in, and
`foundry-anvil` resolves `uv` from the local pool.

**5. Docs in the same change.** `foundry-apt/README.md` (a row in the vendored table
after `ruff`; add `uv` to the `foundry-core` row's tool list),
`foundry-apt/LICENSES-VENDORED.md` (row after `ruff`, `MIT OR Apache-2.0`), this repo's
`CLAUDE.md` (the "30 vendored upstreams" count and list → 31; the edition diagram line
`task, btop, firefox`; the Phase 0 script table gains `install-uv.sh`),
`foundry-setup/README.md` ("What it does"), and the `TODO.md` Debian ITP section gets a
one‑liner "`uv` — already in Debian (0.9.17+ds1-6, sid/forky), no ITP" the way
`python3-pydub` is recorded there.

**6. Release.** New package → next *minor* tag per the CLAUDE.md version rule
(`1.x.0`, not `1.0.x`); `task sync-and-release TAG=…` builds, signs and syncs to R2.
Then `task devbox-bump` and `task iso-bump` so the two consumers rebuild against the
published pool. Publish and the two bumps are the last three verification steps.

## Out of scope

- **Replace `pipx` with `uv tool install` in Phase 0.** Once `uv` is in base,
  `install-foundry-retro-tools.sh` (shrinkray) and `install-foundry-ios-development.sh`
  (codemagic-cli-tools) can drop the extra `pipx` apt dependency and use
  `uv tool install`. Separate T2 item; touches the retro‑tools E2E test.
- **aarch64 build.** The `manylinux_2_17_aarch64` wheel exists, but the repo and ISO are
  amd64‑only; revisit with any multi‑arch decision, not here.
- **Managed Python via `uv python install`.** Deliberately not pre‑seeded; system
  `python3` from universe stays the interpreter Foundry supports.

## Verification

1. Universe check on a fresh `ubuntu:26.04` container shows no `uv` candidate (the
   reason to vendor):

    ```bash
    docker run --rm ubuntu:26.04 bash -c 'apt-get update -qq >/dev/null; apt-cache policy uv'
    ```

2. The package builds in the CI container and lintian is clean (zero `E:`/`W:` lines):

    ```bash
    cd foundry-apt && docker run --rm -v "$PWD:/work" -w /work ubuntu:26.04 bash -c \
      'apt-get update -q && apt-get install -y --no-install-recommends build-essential debhelper dpkg-dev devscripts fakeroot lintian curl ca-certificates unzip \
       && bash scripts/build-all.sh uv && lintian dist/uv_0.12.17-1foundry1_amd64.deb'
    ```

3. The `.deb` carries both binaries, both man pages and the three completion files:

    ```bash
    dpkg-deb -c foundry-apt/dist/uv_0.12.17-1foundry1_amd64.deb | grep -E 'usr/bin/|man1/|completion'
    dpkg-deb -I foundry-apt/dist/uv_0.12.17-1foundry1_amd64.deb | grep -E '^ (Depends|Recommends|Architecture)'
    ```

4. It installs and works on a fresh 26.04 system: both binaries run, and `uv` finds the
   system interpreter rather than downloading one:

    ```bash
    docker run --rm -v "$PWD/foundry-apt/dist:/d:ro" ubuntu:26.04 bash -c \
      'apt-get update -qq >/dev/null && apt-get install -y -qq python3 >/dev/null && apt-get install -y -qq /d/uv_0.12.17-1foundry1_amd64.deb >/dev/null \
       && uv --version && uvx --version && uv python find && uv venv -q /tmp/v && uv pip install -q --python /tmp/v/bin/python rich && /tmp/v/bin/python -c "import rich; print(\"rich ok\")"'
    ```

5. `foundry-core` 1.0.7 resolves `uv` from the local publish:

    ```bash
    cd foundry-apt && task publish-local && task apt-test 2>&1 | grep -E '^Inst (uv|foundry-core) '
    ```

6. Phase 0 scripts lint, help short‑circuits, and the default role now plans a `uv` step:

    ```bash
    shellcheck foundry-setup/install-uv.sh foundry-setup/install-foundry-dev.sh foundry-setup/install.sh
    bash foundry-setup/install-uv.sh --help; echo "exit=$?"
    bash foundry-setup/install.sh --dry-run 2>&1 | grep -n 'install-uv.sh'
    bash foundry-setup/install.sh --dry-run --role engine-dev 2>&1 | grep -n 'install-uv.sh'
    bash foundry-setup/test/run-test.sh
    ```

7. Every place that enumerates the base toolkit mentions `uv`, and the vendored count is
   31:

    ```bash
    grep -n 'uv' foundry-apt/packages/foundry-core/debian/control foundry-apt/README.md foundry-apt/LICENSES-VENDORED.md CLAUDE.md foundry-setup/README.md foundry-devbox/test/smoke-test.sh foundry-devbox/Dockerfile
    grep -n 'vendored upstreams' CLAUDE.md
    ```

8. Published: a fresh container pointed at the live repo sees the package, and the
   rebuilt devbox image passes its smoke test with `uv`/`uvx` on `PATH`:

    ```bash
    docker run --rm ubuntu:26.04 bash -c 'apt-get update -qq >/dev/null && apt-get install -y -qq curl gpg ca-certificates >/dev/null && curl -fsSL https://foundrylinux.org/setup.sh | bash >/dev/null && apt-cache policy uv'
    IMAGE=ghcr.io/foundry-linux/devbox:26.04 bash foundry-devbox/test/smoke-test.sh 2>&1 | grep -E 'uv|PASS|FAIL'
    ```

9. ISO: after `task iso-bump`, the new ISO's package manifest lists `uv`:

    ```bash
    grep -E '^uv\b' foundry-iso/dist/*.packages 2>/dev/null || grep -E '^uv\b' foundry-iso/binary.packages
    ```
