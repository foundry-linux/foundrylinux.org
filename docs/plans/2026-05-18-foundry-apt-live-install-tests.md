# foundry-apt live install tests

**Date:** 2026-05-18  
**Status:** Done

## Context

`apt.foundrylinux.org` is live with five metapackages. The existing `smoke-install` CI job
(`publish.yml:91`) tests only `worldfoundry-engine-build-deps` with `--download-only`.
The user wants a local test script that installs each published package in a fresh container,
using the exact repo-setup commands from the project docs.

## Files to create / modify

| File | Action |
|---|---|
| `foundry-apt/test/run-test.sh` | **new** — test runner |
| `foundry-apt/Taskfile.yml` | **edit** — add `live-test` task |

## Script: `foundry-apt/test/run-test.sh`

Pattern mirrors `foundry-linux-setup/test/run-test.sh`.

### Usage
```
bash test/run-test.sh                              # all packages, full install
bash test/run-test.sh --download-only              # resolution check, no actual install
bash test/run-test.sh --package worldfoundry-engine-build-deps
bash test/run-test.sh --version 24.04              # Ubuntu version (default: 26.04)
-h / --help
```

### Key design points

- **Package discovery**: glob `packages/*/DEBIAN/control`, extract `Package:` field —
  always in sync with what's published, no hardcoded list.
- **One container per package**: better isolation; each failure is independent.
- **Runtime auto-detect**: prefer podman if present, fall back to docker.
- **Base image**: `ubuntu:${VERSION}` — no custom Dockerfile needed.
- **Repo setup** (exact commands from the project docs, no sudo needed — containers run as root):
  ```bash
  apt-get update -q
  apt-get install -y --no-install-recommends curl gpg ca-certificates
  curl -fsSL https://apt.foundrylinux.org/key.gpg \
    | gpg --dearmor -o /etc/apt/keyrings/foundry.gpg
  echo "deb [signed-by=/etc/apt/keyrings/foundry.gpg] https://apt.foundrylinux.org resolute main" \
    > /etc/apt/sources.list.d/foundry.list
  apt-get update -q
  apt-get install -y [--download-only] <package>
  ```
- **Summary**: `N passed, N failed` at end; exit 1 if any failed.

### Size note

`worldfoundry-blender` and `worldfoundry-dev` pull large dependencies. Use `--download-only`
for a fast resolution pass during development.

## Taskfile addition

```yaml
live-test:
  desc: "Install each published package from apt.foundrylinux.org in a fresh container"
  dir: "{{.TASKFILE_DIR}}"
  cmds:
    - bash test/run-test.sh {{.CLI_ARGS}}
```

## Verification

```bash
cd foundry-apt

# Fast smoke (resolution only, ~30 s)
task live-test -- --download-only

# Single package (full install, quick)
task live-test -- --package worldfoundry-engine-build-deps

# Full suite (slow — blender + deps)
task live-test
```

Expected output: `[PASS]` per package, `N passed, 0 failed` at the end.

## Test results

### `--download-only` (resolution check) — 2026-05-18

All five packages resolve correctly against the live repo:

| Package | Result | Notes |
|---|---|---|
| `foundry-linux-retro-tools` | PASS | |
| `worldfoundry-android-dev` | PASS | |
| `worldfoundry-blender` | PASS | |
| `worldfoundry-dev` | PASS | Cloudsmith task repo added automatically (detected via `Depends: task`) |
| `worldfoundry-engine-build-deps` | PASS | |

**Finding during test development:** `worldfoundry-dev` depends on `task` (go-task), which is
not in the Ubuntu archive. Test script detects `task` in the `Depends:` field and adds the
Cloudsmith APT repo (`dl.cloudsmith.io/public/task/task/setup.deb.sh`) before installing.

### Full install — 2026-05-18

All five packages install successfully from the live repo in a fresh Ubuntu 26.04 container:

| Package | Result | Notes |
|---|---|---|
| `foundry-linux-retro-tools` | PASS | ~483 MB download |
| `worldfoundry-android-dev` | PASS | ~253 MB download |
| `worldfoundry-blender` | PASS | ~475 MB download |
| `worldfoundry-dev` | PASS | ~900 MB download (all deps + Cloudsmith task repo) |
| `worldfoundry-engine-build-deps` | PASS | ~209 MB download |

**Bug found and fixed during test development:**
`((pass++))` with `set -euo pipefail` exits the script when the counter is 0 (falsy expression).
Fixed by using `pass=$((pass + 1))` instead.

### CI smoke-install — v0.0.10 — 2026-05-18

`build-and-publish` green; `smoke-install` failed once with:

```
E: Failed to fetch …/Packages.gz  File has unexpected size (2800 != 2798).
Mirror sync in progress?
```

`Packages.gz` and `Release` are currently consistent at 2798 bytes each.
`cf-cache-status: BYPASS` (Packages.gz) and `DYNAMIC` (Release) — no CDN caching active.
**Re-run passed (both jobs green).** Confirmed transient: the smoke-install job started
immediately after `build-and-publish` (via `needs:`) and caught R2 mid-upload — new
Packages.gz was live but the old Release was still in place during Pass 2. Adding a
`sleep 5` between Pass 1 and Pass 2 in the workflow would close this window if it recurs.
