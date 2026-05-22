# Plan: replace sleep with post-upload consistency verification

## Context

`sleep 5` between Pass 1 and Pass 2 in `publish.yml` is a time-based guess, not a guarantee.
The root cause is that apt sees new `Packages.gz` and old `Release` (or vice versa) during
the brief window between the two rclone passes. The correct fix is deterministic: after Pass 2
completes, fetch `Release` and `Packages.gz` from the live URL and verify their SHA256s agree.
Retry up to 10 times (2 s apart) before failing the build.

## File to modify

`foundry-apt/.github/workflows/publish.yml` — one step changed, one step added.

## Changes

### 1. Remove `sleep 5` from the Sync step (lines 76-78)

Delete:
```yaml
          # Brief pause so R2 propagates Packages.gz to all edges before
          # Release goes live — prevents a narrow apt hash-mismatch window.
          sleep 5
```

### 2. Add "Verify repo consistency" step immediately after "Sync to Cloudflare R2"

```yaml
      - name: Verify repo consistency
        if: ${{ !inputs.dry_run }}
        run: |
          for i in $(seq 1 10); do
            EXPECTED=$(curl -fsSL https://apt.foundrylinux.org/dists/resolute/Release \
              | awk '/^SHA256:/{found=1} found && /main\/binary-amd64\/Packages\.gz/{print $1; exit}')
            ACTUAL=$(curl -fsSL https://apt.foundrylinux.org/dists/resolute/main/binary-amd64/Packages.gz \
              | sha256sum | awk '{print $1}')
            if [[ "$EXPECTED" == "$ACTUAL" ]]; then
              echo "Repo consistent (attempt ${i})"
              exit 0
            fi
            echo "Attempt ${i}/10: expected ${EXPECTED}, got ${ACTUAL} — retrying in 2s"
            sleep 2
          done
          echo "Repo never became consistent after 10 attempts" >&2
          exit 1
```

## Verification

Run `task bump` — watch Actions. The new "Verify repo consistency" step should appear
and exit 0 on the first attempt (or quickly converge). Smoke-install should be green.

---

# (previous) Plan: foundry-apt live install tests

## Context

`apt.foundrylinux.org` is live with five metapackages. The existing `smoke-install` CI job
(publish.yml:91) tests only `worldfoundry-engine-build-deps` with `--download-only`.
The user wants a local test script that installs each published package in a fresh container,
using the exact repo-setup commands from the project docs.

## Files to create / modify

| File | Action |
|---|---|
| `foundry-apt/test/run-test.sh` | **new** — test runner |
| `foundry-apt/Taskfile.yml` | **edit** — add `live-test` task |

## Script: `foundry-apt/test/run-test.sh`

Pattern mirrors `foundry-setup/test/run-test.sh`.

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
- **Repo setup** (exact commands from the user, no sudo inside container since we run as root):
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
