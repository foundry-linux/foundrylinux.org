# Package python3-glfw, python3-pydub, ruff, python3-librosa, python3-mss

**Status: COMPLETE** — all five packages shipped in foundry-apt v1.5.7.
Metapackages shipped in v1.5.8/v1.5.9/v1.5.10. foundry-core updated to pull both.

---

## What shipped

| Package | Version | Arch | Notes |
|---------|---------|------|-------|
| `python3-glfw` | 2.10.0-1foundry1 | all | ctypes wrapper; `Depends: libglfw3 \| libglfw3-wayland` |
| `python3-pydub` | 0.25.1-1foundry1 | all | `Depends: python3-audioop-lts` (Python 3.14 compat) |
| `ruff` | 0.15.15-1foundry1 | amd64 | pre-built Rust binary from manylinux wheel |
| `python3-librosa` | 0.11.0-1foundry1 | all | ISC license; all 13 runtime deps in ubuntu universe |
| `python3-mss` | 10.2.0-1foundry1 | all | hatchling backend; ships `mss(1)` man page |
| `foundry-python-gamedev` | 1.0.0 | all | 31-package base metapackage |
| `foundry-python-gamedev-extras` | 1.0.0 | all | 16-package extras metapackage (opencv, av, librosa, mss, …) |

`foundry-core` 1.0.2 now `Depends:` both metapackages — full Python game-dev
stack ships with all editions and existing installs.

---

## Key discoveries (update /package skill with these)

### 1. Changelog distribution is `resolute`, not `stable`
The skill template said `stable`. All existing packages in foundry-apt use `resolute`.
Confirmed by reading f9dasm, vgmstream, foundry-retro-tools changelogs.

### 2. setuptools + debhelper compat 13 requires `--buildsystem=pybuild`
Using just `dh $@ --with python3` fails at `dh_auto_clean` with:
> "This feature was removed in compat 12."
Fix: always use `--buildsystem=pybuild` for Python packages. Also add
`override_dh_auto_test:` (empty) to skip tests that need a display or network.

### 3. hatchling packages need extra Build-Depends
`pybuild-plugin-pyproject` + `python3-hatchling` — both in ubuntu:26.04 universe.
Check `pyproject.toml [build-system]` to identify the backend.

### 4. Python 3.14 removed audioop — fix is `python3-audioop-lts`
Ubuntu 26.04 ships Python 3.14. `audioop` was removed in Python 3.13.
pydub 0.25.1 already has the `try: import audioop; except: import pyaudioop`
fallback, but `pyaudioop` does NOT exist on PyPI. The correct package is
`python3-audioop-lts` (ubuntu universe 0.2.2-2), which reinstates `audioop` directly.

### 5. PyPI wheel binary path is `<name>-<ver>.data/scripts/<binary>`
Not `<name>/bin/<binary>` as assumed. Unzip the wheel flat into the source dir,
then `install -D -m 0755 ruff-0.15.15.data/scripts/ruff ...`.
The wheel URL in the plan was stale (404) — always re-fetch from PyPI JSON API.

### 6. `python3-audioop-lts`, `standard-aifc`, `standard-sunau` are all in universe
These Python 3.13 stdlib-removal backports are all packaged in Ubuntu 26.04 universe.
librosa needs the latter two for Python ≥ 3.13 (`standard-aifc`, `standard-sunau`).
