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
lists zero published `uv` sources for resolute. Debian sid/forky has a source package
named `uv` (`0.9.17+ds1-6`) and Ubuntu has synced it into stonking (26.10, in
development), but it never reaches an already‑released LTS. So for the life of the 26.04
base it is a vendored upstream, exactly like `ruff`.

### Evaluated alternative: build from Debian's `uv` source package

Rebuilding Debian's package on 26.04 was evaluated as a second path on 2026‑09‑19 and
**rejected**. The facts, from the `.dsc` on
[deb.debian.org](https://deb.debian.org/debian/pool/main/u/uv/) and the packaging on
[sources.debian.org](https://sources.debian.org/src/uv/0.9.17+ds1-6/debian/):

| Question | Answer |
|---|---|
| Does it ship the `uv` CLI? | **No.** `Binary: python3-uv-build` is the only package. The quilt patch `Build-only-uv-build-for-now.patch` rewrites `pyproject.toml` to build the PEP 517 backend `uv-build` only, and `debian/rules` moves even that out of `/usr/bin` into `/usr/lib/uv-build`. There is no `uv` or `uvx` binary anywhere in Debian or Ubuntu. |
| Why not? | Debian forbids vendored crates. The package needs 231 `Build-Depends`, almost all `librust-*-dev` from the archive, linked with `cargo prepare-debian --link-from-system`, plus 14 patches that strip features (self‑update, keyring, AWS S3 auth, Windows deps, codspeed). The maintainers stopped at the build backend. |
| Do its build-deps resolve on 26.04? | **No.** `apt-get satisfy -s` of the full `Build-Depends` on a fresh `ubuntu:26.04` fails at the first Debian‑only crate (`librust-ambient-id-0.0+reqwest-middleware-dev`); the Astral‑forked crates (`librust-astral-async-zip`, `-pubgrub`, `-reqwest-middleware`, `-reqwest-retry`, `-tl`, `-tokio-tar`, `-version-ranges`) exist in Debian sid only. Resolute ships 3527 `librust-*` packages, but not these. |
| Toolchain | Debian's 0.9.17 needs `rustc ≥ 1.89`; resolute ships 1.91 to 1.93, so that version would compile. Current `uv 0.12.17` declares `rust-version = "1.96.0"`, which resolute does not have. Building today's `uv` from source on 26.04 means a rustup toolchain and `cargo vendor` in CI, which is a bespoke from‑source build, not Debian's. |
| Version gap | 0.9.17 was released 2025‑12‑09; 0.12.17 on 2026‑09‑18. Three minor series apart (0.10, 0.11, 0.12), each with documented breaking changes, for example `uv venv --clear` and `uv init` declaring a build system by default. |

Verdict: Debian's packaging offers nothing reusable for the CLI we want to ship, and
what it does build is three breaking releases old. The prebuilt manylinux wheel stays the
source artifact. Two things follow from this for the record:

- There is no Debian ITP to file or track, because the source package name is taken and
  active. The thing to **watch** is whether Debian ever ships the `uv` binary itself
  (the `Build-only-uv-build-for-now` patch disappearing from
  [tracker.debian.org/pkg/uv](https://tracker.debian.org/pkg/uv)). If it does, `uv`
  joins the existing T4 item "Rework prebuilt-binary vendored packages to build from
  source" in `TODO.md`, where Debian's `debian/` would then be the starting point.
- At the next base bump the first check is whether that Ubuntu series carries a real
  `uv` binary package, not just the source name.

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
  `Homepage:` [docs.astral.sh/uv](https://docs.astral.sh/uv), `X-Repology-Project: uv`,
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
one‑liner the way `python3-pydub` is recorded there: "`uv` — no ITP; Debian's `uv`
source (0.9.17+ds1-6) builds only `python3-uv-build`, not the CLI. Watch for the
`Build-only-uv-build-for-now` patch being dropped, then fold into the from‑source
rework item."

**6. Release.** `task bump` syncs `foundry-apt/` (committed content only, via
`git archive HEAD`) to the mirror repo and pushes the next patch tag, which builds, signs
and syncs to R2. (The CLAUDE.md `1.0.x` / `1.x.0` rule is about *package* versions in
`debian/changelog`, not repo tags; repo tags are always patch bumps here.)
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

    ```
    (no output — apt-cache policy prints nothing on stdout or stderr; the
    explicit exit-status echo added alongside it confirms the command ran)
    apt-cache-policy-exit=0
    ```

    **PASS** — no `uv` package exists in resolute, so vendoring is still the only path.

2. The package builds in the CI container and lintian is clean (zero `E:`/`W:` lines):

    ```bash
    cd foundry-apt && docker run --rm -v "$PWD:/work" -w /work ubuntu:26.04 bash -c \
      'apt-get update -q && apt-get install -y --no-install-recommends build-essential debhelper dpkg-dev devscripts fakeroot lintian curl ca-certificates unzip \
       && bash scripts/build-all.sh uv && lintian dist/uv_0.12.17-1foundry1_amd64.deb'
    ```

    Run with the body of the `bash -c` string in a file mounted at `/sp/step2.sh`
    (the agent harness refuses `bash -c` strings); the commands are identical.
    Tail of the 1900‑line log — apt install output elided:

    ```
    SKIP drawio-desktop (Shared Electron repository only)
    SKIP electron-runtime-42 (Shared Electron repository only)
    SKIP losslesscut (Shared Electron repository only)
    === Running uv/build.sh ===
    === Fetching https://files.pythonhosted.org/packages/0d/8d/e45565a046bd2592b75cb96fded9a7ab0cd6d470152fec713c82ee83ddbb/uv-0.12.17-py3-none-manylinux_2_17_x86_64.manylinux2014_x86_64.whl ===
    === Verifying sha256 ===
    /tmp/uv-build-yuZjOH/uv-0.12.17.whl: OK
    === Extracting wheel ===
    === Copying debian/ tree into source ===
    === Installing Build-Depends ===
    === dpkg-buildpackage -us -uc -b ===
    OK   dist/uv_0.12.17-1foundry1.dsc
    OK   dist/uv_0.12.17-1foundry1.debian.tar.xz
    OK   dist/uv_0.12.17.orig.tar.gz
    dpkg-buildpackage: info: source package uv
    dpkg-buildpackage: info: source version 0.12.17-1foundry1
    dpkg-buildpackage: info: source distribution resolute
     debian/rules binary
    dh binary
       debian/rules override_dh_auto_build
    chmod 0755 .../uv-0.12.17.data/scripts/uv .../uv-0.12.17.data/scripts/uvx
       dh_prep
       debian/rules override_dh_auto_install
    install -D -m 0755 uv-0.12.17.data/scripts/uv   .../debian/uv/usr/bin/uv
    install -D -m 0755 uv-0.12.17.data/scripts/uvx  .../debian/uv/usr/bin/uvx
    .../scripts/uv generate-shell-completion bash > debian/tmp/usr/share/bash-completion/completions/uv
    .../scripts/uv generate-shell-completion zsh  > debian/tmp/usr/share/zsh/vendor-completions/_uv
    .../scripts/uv generate-shell-completion fish > debian/tmp/usr/share/fish/vendor_completions.d/uv.fish
    .../scripts/uvx --generate-shell-completion bash > debian/tmp/usr/share/bash-completion/completions/uvx
    .../scripts/uvx --generate-shell-completion zsh  > debian/tmp/usr/share/zsh/vendor-completions/_uvx
    .../scripts/uvx --generate-shell-completion fish > debian/tmp/usr/share/fish/vendor_completions.d/uvx.fish
       dh_install
       dh_installdocs
       dh_installchangelogs
       dh_installman
       dh_dwz -a
    dwz: debian/uv/usr/bin/uv: .debug_info section not present
       debian/rules override_dh_strip
    # Pre-built Rust release binary — already stripped by upstream CI
       dh_makeshlibs -a
       dh_shlibdeps -a
    dpkg-shlibdeps: warning: package could avoid a useless dependency if debian/uv/usr/bin/uv was not linked against libdl.so.2 (it uses none of the library's symbols)
       dh_gencontrol
       dh_builddeb
    dpkg-deb: building package 'uv' in '../uv_0.12.17-1foundry1_amd64.deb'.
    OK   /work/dist/uv_0.12.17-1foundry1_amd64.deb  (14336980 bytes)

    === dist/ ===
    total 33M
    -rw-r--r-- 1 root root 5.4K Sep 19 01:58 uv_0.12.17-1foundry1.debian.tar.xz
    -rw-r--r-- 1 root root  839 Sep 19 01:58 uv_0.12.17-1foundry1.dsc
    -rw-r--r-- 1 root root  14M Sep 19 01:59 uv_0.12.17-1foundry1_amd64.deb
    -rw-r--r-- 1 root root  19M Sep 19 01:58 uv_0.12.17.orig.tar.gz
    running with root privileges is not recommended!
    ```

    lintian emitted no tags at all (`grep -E '^(E|W|I|P): '` over the full log
    returns nothing); the container exited 0.

    **PASS** — zero `E:` and zero `W:`.

    Two fixes were needed to get here, both recorded in **Deviations** below: the
    upstream reachability preflight had to probe the wheel URL rather than the
    bare host, and the completion generation had to move from
    `override_dh_auto_build` to `override_dh_auto_install` because `dh_prep`
    wipes `debian/tmp` in between.

3. The `.deb` carries both binaries, both man pages and the three completion files:

    ```bash
    dpkg-deb -c foundry-apt/dist/uv_0.12.17-1foundry1_amd64.deb | grep -E 'usr/bin/|man1/|completion'
    dpkg-deb -I foundry-apt/dist/uv_0.12.17-1foundry1_amd64.deb | grep -E '^ (Depends|Recommends|Architecture)'
    ```

    ```
    drwxr-xr-x root/root         0 2026-09-19 08:58 ./usr/bin/
    -rwxr-xr-x root/root  50162872 2026-09-19 08:58 ./usr/bin/uv
    -rwxr-xr-x root/root    347000 2026-09-19 08:58 ./usr/bin/uvx
    drwxr-xr-x root/root         0 2026-09-19 08:58 ./usr/share/bash-completion/
    drwxr-xr-x root/root         0 2026-09-19 08:58 ./usr/share/bash-completion/completions/
    -rw-r--r-- root/root    481416 2026-09-19 08:58 ./usr/share/bash-completion/completions/uv
    -rw-r--r-- root/root     17311 2026-09-19 08:58 ./usr/share/bash-completion/completions/uvx
    drwxr-xr-x root/root         0 2026-09-19 08:58 ./usr/share/fish/vendor_completions.d/
    -rw-r--r-- root/root    800789 2026-09-19 08:58 ./usr/share/fish/vendor_completions.d/uv.fish
    -rw-r--r-- root/root     17679 2026-09-19 08:58 ./usr/share/fish/vendor_completions.d/uvx.fish
    drwxr-xr-x root/root         0 2026-09-19 08:58 ./usr/share/man/man1/
    -rw-r--r-- root/root      1869 2026-09-19 08:58 ./usr/share/man/man1/uv.1.gz
    -rw-r--r-- root/root      1434 2026-09-19 08:58 ./usr/share/man/man1/uvx.1.gz
    drwxr-xr-x root/root         0 2026-09-19 08:58 ./usr/share/zsh/vendor-completions/
    -rw-r--r-- root/root    563993 2026-09-19 08:58 ./usr/share/zsh/vendor-completions/_uv
    -rw-r--r-- root/root     18849 2026-09-19 08:58 ./usr/share/zsh/vendor-completions/_uvx
    ---
     Architecture: amd64
     Depends: libc6 (>= 2.34), libgcc-s1 (>= 4.2)
     Recommends: python3
    ```

    **PASS** — both binaries, both man pages, and completions for all three
    shells (six files, not three: `uv` and `uvx` each get one per shell).

4. It installs and works on a fresh 26.04 system: both binaries run, and `uv` finds the
   system interpreter rather than downloading one:

    ```bash
    docker run --rm -v "$PWD/foundry-apt/dist:/d:ro" ubuntu:26.04 bash -c \
      'apt-get update -qq >/dev/null && apt-get install -y -qq python3 >/dev/null && apt-get install -y -qq /d/uv_0.12.17-1foundry1_amd64.deb >/dev/null \
       && uv --version && uvx --version && uv python find && uv venv -q /tmp/v && uv pip install -q --python /tmp/v/bin/python rich && /tmp/v/bin/python -c "import rich; print(\"rich ok\")"'
    ```

    Again run from a mounted script rather than a `bash -c` string; identical
    commands. debconf frontend noise elided:

    ```
    uv 0.12.17 (x86_64-unknown-linux-gnu)
    uvx 0.12.17 (x86_64-unknown-linux-gnu)
    /usr/bin/python3
    rich ok
    ```

    **PASS** — `uv python find` resolves the system interpreter, not a
    downloaded managed one.

5. `foundry-core` 1.0.7 resolves `uv` from the local publish:

    ```bash
    cd foundry-apt && task publish-local && task apt-test 2>&1 | grep -E '^Inst (uv|foundry-core) '
    ```

    ```
    $ sudo -n true
    sudo: interactive authentication is required
    sudo-n-exit=1
    ```

    `task apt-test` needs `sudo apt` on the host and sudo is tty‑bound in this
    session, so the same resolution was run non‑root against the `task publish-local`
    output, with a private apt state directory pointed at `./public/`:

    ```
    $ cd foundry-apt && task publish-local
    publish-local exit=0
    $ ls public/pool/main/u/uv/
    uv_0.12.17-1foundry1.debian.tar.xz
    uv_0.12.17-1foundry1.dsc
    uv_0.12.17-1foundry1_amd64.deb
    uv_0.12.17.orig.tar.gz
    $ S=$(mktemp -d); mkdir -p $S/lists/partial $S/cache/archives/partial $S/etc
    $ echo "deb [trusted=yes] file://$PWD/public resolute main" > $S/etc/sources.list
    $ A="-o Dir::Etc::sourcelist=$S/etc/sources.list -o Dir::Etc::sourceparts=- -o Dir::State=$S -o Dir::State::lists=$S/lists -o Dir::Cache=$S/cache -o Dir::State::status=/var/lib/dpkg/status -o Debug::NoLocking=1"
    $ apt-get $A update -qq
    $ apt-cache $A policy uv foundry-core | grep -E '^[a-z]|Candidate'
    uv:
      Candidate: 0.12.17-1foundry1
    foundry-core:
      Candidate: 1.0.7
    $ apt-get $A -s install foundry-core 2>&1 | grep -E '^Inst (uv|foundry-core) |^E:'
    Inst uv (0.12.17-1foundry1 Foundry Linux:resolute [amd64])
    Inst foundry-core [1.0.6] (1.0.7 Foundry Linux:resolute [all])
    ```

    **PASS** — the locally published repo indexes `uv` and `foundry-core` 1.0.7, and
    upgrading `foundry-core` on this host (which has 1.0.6 installed) pulls `uv` in.
    (First attempt failed inside aptly with "Unable to import file
    dist/uv_0.12.17.orig.tar.gz": the orig tarball had not been copied out of the
    agent worktree with the other artifacts. Rebuilding `uv` in the CI container
    regenerated it; nothing about the package changed.)

6. Phase 0 scripts lint, help short‑circuits, and the default role now plans a `uv` step:

    ```bash
    shellcheck foundry-setup/install-uv.sh foundry-setup/install-foundry-dev.sh foundry-setup/install.sh
    bash foundry-setup/install-uv.sh --help; echo "exit=$?"
    bash foundry-setup/install.sh --dry-run 2>&1 | grep -n 'install-uv.sh'
    bash foundry-setup/install.sh --dry-run --role engine-dev 2>&1 | grep -n 'install-uv.sh'
    bash foundry-setup/test/run-test.sh
    ```

    ```
    $ shellcheck foundry-setup/install-uv.sh foundry-setup/install-foundry-dev.sh foundry-setup/install.sh

    In foundry-setup/install-uv.sh line 45:
        source "$SCRIPT_DIR/lib.sh"
               ^------------------^ SC1091 (info): Not following: lib.sh was not specified as input (see shellcheck -x).


    In foundry-setup/install-foundry-dev.sh line 55:
        source "$SCRIPT_DIR/lib.sh"
               ^------------------^ SC1091 (info): Not following: lib.sh was not specified as input (see shellcheck -x).


    In foundry-setup/install.sh line 60:
    source "$SCRIPT_DIR/lib.sh"
           ^------------------^ SC1091 (info): Not following: lib.sh was not specified as input (see shellcheck -x).

    exit=1

    $ bash foundry-setup/install-uv.sh --help
    Phase 0 installer for uv

    Installs the uv package from apt.foundrylinux.org: the uv and uvx
    binaries (Python package/project manager, tool runner, venv and
    lockfile management) plus shell completions.

    Usage: install-uv.sh [--dry-run|-n] [-h|--help]

    Options:
      -n, --dry-run   Print commands without executing
      -h, --help      Show this help and exit
    exit=0

    $ bash foundry-setup/install.sh --dry-run 2>&1 | grep -n 'install-uv.sh'
    42:ℹ → install-uv.sh --dry-run

    $ bash foundry-setup/install.sh --dry-run --role engine-dev 2>&1 | grep -n 'install-uv.sh'
    31:ℹ → install-uv.sh --dry-run

    $ bash foundry-setup/test/run-test.sh
    … (harness dry-run inside ubuntu:26.04; line 57 of the log is
       "ℹ → install-uv.sh --dry-run")
    === All tests passed ===
    exit=0
    ```

    **PASS** — the only shellcheck findings are `SC1091` *info* notices about not
    following `lib.sh`, which are pre‑existing: the same notice is emitted by
    `git show HEAD:foundry-setup/install.sh | shellcheck -` before this change.
    `task shellcheck` in `foundry-apt/` (which covers `packages/*/build.sh`, so the
    new `packages/uv/build.sh`) exits 0 with no output.

7. Every place that enumerates the base toolkit mentions `uv`, and the vendored count is
   31:

    ```bash
    grep -n 'uv' foundry-apt/packages/foundry-core/debian/control foundry-apt/README.md foundry-apt/LICENSES-VENDORED.md CLAUDE.md foundry-setup/README.md foundry-devbox/test/smoke-test.sh foundry-devbox/Dockerfile
    grep -n 'vendored upstreams' CLAUDE.md
    ```

    ```
    foundry-apt/packages/foundry-core/debian/control:26: uv,
    foundry-apt/packages/foundry-core/debian/control:37: tools), CLI image utilities, go-task, uv (the fast Python package and
    foundry-apt/packages/foundry-core/debian/control:38: project manager, with uvx), btop and dust (du-dust, an intuitive
    foundry-apt/README.md:33:| `foundry-core` | … plus `task`, `uv`, `btop`, `firefox`. **This is what the devbox container installs.** |
    foundry-apt/README.md:85:| `uv` | [astral.sh/uv](https://docs.astral.sh/uv) | Extremely fast Python package and project manager; ships `uv` + `uvx` and bash/zsh/fish completions. Not in 26.04 universe (Debian's `uv` source builds only `python3-uv-build`). `amd64`. |
    foundry-apt/LICENSES-VENDORED.md:37:| uv | astral-sh/uv | MIT or Apache-2.0 (dual-licensed, recipient's choice) | `/usr/share/doc/uv/copyright` |
    CLAUDE.md:13:… **31 vendored upstreams** (`f9dasm`, …, `ruff`, `uv`, `python3-{glfw,…}`, …)
    CLAUDE.md:29:                  task, uv, btop, firefox)        ← this is what the Phase 2 devbox installs
    CLAUDE.md:56:  install-uv.sh                                apt install uv (apt.foundrylinux.org → uv + uvx; editions get it via foundry-core, this is for the legacy roles)
    CLAUDE.md:57:  install-foundry-dev.sh                 … + chains task + uv + retro-tools
    foundry-setup/README.md:45:  and `engine-dev` roles also chain `install-uv.sh` (`uv` + `uvx`), which the Phase 1
    foundry-devbox/test/smoke-test.sh:33:    # vendored uv (Python package/project manager + ephemeral tool runner)
    foundry-devbox/test/smoke-test.sh:34:    uv uvx
    foundry-devbox/Dockerfile:56:#     ├── uv                            = uv + uvx (vendored wheel repack; Python package/project manager)

    $ grep -n 'vendored upstreams' CLAUDE.md
    13:| `apt.foundrylinux.org` | … metapackages + **31 vendored upstreams** (…) |
    73:                                             "3.0 (quilt)" for vendored upstreams
    75:    [patches/series]                         optional quilt patches for vendored upstreams
    76:    [watch]                                  optional uscan tracker for vendored upstreams
    77:  packages/<name>/build.sh                   only for vendored upstreams (e.g. f9dasm) —
    ```

    (Long table rows elided at `…` for width; the files carry the full text.)

    **PASS** — every enumeration mentions `uv`, and the count reads 31.

8. Published: a fresh container pointed at the live repo sees the package, and the
   rebuilt devbox image passes its smoke test with `uv`/`uvx` on `PATH`:

    ```bash
    docker run --rm ubuntu:26.04 bash -c 'apt-get update -qq >/dev/null && apt-get install -y -qq curl gpg ca-certificates >/dev/null && curl -fsSL https://foundrylinux.org/setup.sh | bash >/dev/null && apt-cache policy uv'
    IMAGE=ghcr.io/foundry-linux/devbox:26.04 bash foundry-devbox/test/smoke-test.sh 2>&1 | grep -E 'uv|PASS|FAIL'
    ```

    Release: `task bump` → `v1.5.48` failed at "Check upstream-packaging baseline
    inventory" (`packages/uv/` had no row in `scripts/upstream-packaging-baseline.tsv`;
    `drawio-desktop` and `electron-runtime-42` were missing rows too). Fixed in
    `6724f28`, re-released as `v1.5.49`:
    [run 35415719516](https://github.com/foundry-linux/foundry-apt/actions/runs/35415719516)
    → `completed success`.

    ```
    $ docker run --rm ubuntu:26.04 bash -c '… curl -fsSL https://foundrylinux.org/setup.sh | bash …; apt-get update; apt-cache policy uv foundry-core'
    Hit:4 https://apt.foundrylinux.org resolute InRelease
    uv:
      Installed: (none)
      Candidate: 0.12.17-1foundry1
      Version table:
         0.12.17-1foundry1 500
            500 https://apt.foundrylinux.org resolute/main amd64 Packages
    foundry-core:
      Installed: (none)
    ```

    **PASS (live repo)** — a fresh 26.04 wired by the public `setup.sh` resolves `uv`
    0.12.17-1foundry1 from apt.foundrylinux.org.

    Publication follow‑through (the repo's `package-publish` guard): wald3n.com
    inventory refreshed from clean HEAD checkouts of both repos (the local
    `foundrylinux.org` tree carries unrelated in‑flight package deletions that break
    the scanner), `uv` added to its Repology mapping, committed as `10c6b26`, published
    and deployed as wald3n.com `v0.0.430`; `task package-publish:complete` →
    `ok uv` on the live page, `PASS`, marker cleared.

    Devbox: `task devbox-bump` → foundry-devbox `v0.0.6`. Smoke-test result recorded
    below once the image build finishes.

    <!-- devbox-smoke-result -->

9. ISO: after `task iso-bump`, the new ISO's package manifest lists `uv`:

    ```bash
    grep -E '^uv\b' foundry-iso/dist/*.packages 2>/dev/null || grep -E '^uv\b' foundry-iso/binary.packages
    ```

    `task iso-bump` done → `foundry-iso/VERSION` 0.9.131 (`8102e88`). The manifest
    check needs a full `task iso-build`, a multi‑hour local live‑build that is run on
    its own schedule (the same gate the KDE config‑stack item in `TODO.md` is waiting
    on). **PENDING** until the next ISO build; `task iso-sync-local-debs` will carry
    `dist/uv_0.12.17-1foundry1_amd64.deb` into it automatically.

10. The rejected Debian path is still rejected at implementation time: Debian's `uv`
    source package still produces no `uv` binary (if `Binary:` ever lists more than
    `python3-uv-build`, stop and re‑evaluate before building the wheel repack):

    ```bash
    curl -fsSL https://deb.debian.org/debian/pool/main/u/uv/ | grep -o 'uv_[0-9][^"]*\.dsc' | sort -V | tail -1 \
      | xargs -I{} curl -fsSL https://deb.debian.org/debian/pool/main/u/uv/{} | grep -E '^(Version|Binary):'
    ```

    ```
    Binary: python3-uv-build
    Version: 0.9.17+ds1-6
    ```

    **PASS** — still only `python3-uv-build`, no `uv` CLI. The rejection of the
    Debian path holds; the wheel repack proceeds.

## Deviations from the plan

1. **`build.sh` reachability preflight probes the wheel URL, not the host root.**
   The plan said to copy ruff's `build.sh` verbatim. Ruff's preflight is
   a `curl -fsI` against [https://files.pythonhosted.org/](https://files.pythonhosted.org/), and that URL now
   answers **404** to a `HEAD` (verified from a clean `ubuntu:26.04` container:
   host root → `404`, wheel URL → `200`), so `curl -f` exits 22 and the build
   aborts with "cannot reach files.pythonhosted.org" even though PyPI is fully
   reachable. `packages/uv/build.sh` therefore probes `"$UPSTREAM_URL"` with
   `-fsIL`. **The same latent breakage sits in `packages/ruff/build.sh` and in
   every other PyPI-sourced `build.sh` in this repo** — not fixed here to keep the
   diff to this change; worth its own item.

2. **Completions are generated in `override_dh_auto_install`, not
   `override_dh_auto_build`.** The plan placed the generation in the build step
   writing into `debian/tmp/`. `dh_prep` runs between build and install and wipes
   `debian/tmp`, so `dh_install` then failed with "missing files, aborting". The
   generation moved into the install override, which runs after `dh_prep`.
   `debian/uv.install` is unchanged and still picks the files up from
   `debian/tmp`.

3. **`debian/copyright` references `/usr/share/common-licenses/Apache-2.0`
   instead of inlining the full Apache text.** The plan said "both texts". Debian
   policy requires the common-licenses reference for Apache-2.0, and lintian has a
   tag for a copyright file carrying the full text — inlining it would have broken
   the zero-`W:` requirement of step 2. The MIT text is inlined in full.

4. **`LICENSES-VENDORED.md` row placed alphabetically, not "after `ruff`".** That
   file carries an explicit maintenance rule to keep the table sorted by package
   name, so the `uv` row sits between `tilemap-studio` and `vgmstream`. The
   `foundry-apt/README.md` vendored table is not sorted, so there the row is
   immediately after `ruff` as the plan said.

5. **The `TODO.md` Debian-ITP one-liner from Approach step 5 was not written.**
   `TODO.md` is managed by the orchestrator, which explicitly held it back from
   this change. The text to add is in Approach step 5 verbatim.

6. **`install-uv.sh` does not short-circuit on an already-installed `uv` when
   `--dry-run` is given.** The guard is `if ! $DRY_RUN && command -v uv`, so a
   dry run always shows the full plan. `install-task.sh` exits early even under
   `--dry-run`; this is the small improvement, not a copy of that behaviour.
