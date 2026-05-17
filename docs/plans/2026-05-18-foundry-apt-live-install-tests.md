# foundry-apt live install tests

**Date:** 2026-05-18  
**Status:** In progress

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
