# Plan: Phase 0 configures `apt.foundrylinux.org` as an apt source

**Date:** 2026-05-18
**Status:** In progress

## Context

Phase 0 (`foundry-linux-setup/install.sh` + per-metapackage scripts) currently runs on stock Ubuntu and `apt install`s a hardcoded list of Ubuntu universe packages, plus source-builds the upstream tools not in universe (Ghidra, libvgm, vgmstream — and until recently, f9dasm and xa65) into `~/opt/`. Each per-meta script's apt-install list is a near-duplicate of the corresponding `foundry-apt/packages/<metapackage>/debian/control` `Depends:`.

Now that **Phase 1 is live** at `apt.foundrylinux.org` and **f9dasm and xa65 ship via apt** (f9dasm from foundry-apt, xa65 from Ubuntu universe), the two sources of truth diverge with every commit. The fix is the "Phase 1 collapse" rehearsed in [`2026-05-17-per-metapackage-install-scripts.md`](2026-05-17-per-metapackage-install-scripts.md) §"Phase 1 collapse rehearsal": each per-meta script body collapses to `apt install <metapackage>`. That collapse requires Phase 0 to first add foundry-apt to `sources.list.d`.

This plan ships that prerequisite — a single source-of-truth-aligning step.

## What changes

### 1. New script: `foundry-linux-setup/setup-foundry-apt-source.sh`

Idempotent. Pattern mirrors `install-task.sh` (which already does the same thing for the Cloudsmith `task` repo).

```bash
#!/usr/bin/env bash
# Add apt.foundrylinux.org to /etc/apt/sources.list.d. Idempotent.
#
# Once this runs, `apt install foundry-linux-*` and the vendored
# packages (f9dasm, future libvgm/vgmstream/ghidra) resolve from
# Foundry's apt repo.

set -euo pipefail

KEYRING=/etc/apt/keyrings/foundry.gpg
LIST=/etc/apt/sources.list.d/foundry.list
URL=https://apt.foundrylinux.org
SUITE=resolute

for arg in "$@"; do
    case "$arg" in
        -h|--help)
            cat <<EOF
Configure apt.foundrylinux.org as an apt source.

Usage: $(basename "$0") [-h|--help] [--dry-run|-n]

Idempotent — safe to re-run. Skips if the keyring + sources.list.d
entry already exist and the key fingerprint matches.
EOF
            exit 0
            ;;
    esac
done

# (... source lib.sh, parse --dry-run, etc., similar to install-task.sh ...)

if [[ -f "$KEYRING" && -f "$LIST" ]]; then
    info "Foundry apt source already configured at $LIST — skipping"
    exit 0
fi

step "Configuring $URL as an apt source"
run_sudo install -d /etc/apt/keyrings
run_sudo bash -c "curl -fsSL '$URL/key.gpg' | gpg --dearmor -o '$KEYRING'"
echo "deb [signed-by=$KEYRING] $URL $SUITE main" \
  | run_sudo tee "$LIST" >/dev/null
apt_update
ok "Foundry apt source live (key at $KEYRING, list at $LIST)"
```

### 2. Wire into `install.sh` orchestrator

After distro check / before per-meta dispatch, insert a call to `setup-foundry-apt-source.sh`. Runs unconditionally for every `--role`, since every per-meta script downstream now expects the source to be configured.

```diff
 # (after distro check)
 
+# Phase 1 collapse: foundry-apt as a source must be configured before
+# any per-meta script can `apt install` from it.
+run bash "$SCRIPT_DIR/setup-foundry-apt-source.sh" $DRY_RUN_FLAG
+
 case "$ROLE" in
     engine-dev)
         ...
```

### 2.5 Prerequisite: publish libvgm

The local `foundry-linux-retro-tools/debian/control` already has `libvgm` in `Depends:`.
`libvgm_*.deb` is built and in `dist/` but not yet published. The retro-tools collapse in step 3
will cause `apt install foundry-linux-retro-tools` to fail unless libvgm is resolvable.

Action: bump `foundry-linux-retro-tools` changelog (move `libvgm` into `Depends:` was already
done locally), cut a release tag — the publish workflow will add `libvgm` to the repo.
This unblocks the collapse.

### 3. Collapse per-meta scripts (5 of them)

Each per-meta script's apt-install list collapses to `apt install <metapackage>` since the
metapackage's `debian/control` `Depends:` is now the source of truth.

| Script | Before | After |
|---|---|---|
| `install-foundry-linux-engine-build-deps.sh` | `apt-get install -y build-essential cmake libx11-dev ...` (13 packages) | `apt-get install -y worldfoundry-engine-build-deps` |
| `install-foundry-linux-blender.sh` | `apt-get install -y blender python3` | `apt-get install -y worldfoundry-blender` |
| `install-foundry-linux-retro-tools.sh` | `apt-get install -y mame mame-tools ...xa65` (12 packages) | `apt-get install -y foundry-linux-retro-tools` (resolves same 12 + f9dasm + libvgm transitively) |
| `install-foundry-linux-android-dev.sh` | `apt-get install -y openjdk-17-jdk adb ...` | `apt-get install -y worldfoundry-android-dev` |
| `install-foundry-linux-dev.sh` | orchestrator: calls the others | `apt-get install -y worldfoundry-dev` (single call gets everything) |

**`--apt-only` remains valid** after the collapse — it now suppresses the remaining source-build
sidecars in `install-foundry-linux-retro-tools.sh`: ghidra and vgmstream (libvgm moves from
sidecar to apt dep; ghidra and vgmstream are not yet packaged and stay as sidecars).
The flag name and semantics are unchanged.

**Source-build sidecars stay** in `install-foundry-linux-retro-tools.sh` for now — Ghidra and
vgmstream still aren't packaged. libvgm moves from sidecar to apt dep (see §2.5). The
retro-tools script's *apt-install* half collapses; the *ghidra* and *vgmstream* sidecar
halves persist until each is packaged. (f9dasm sidecar is also dropped — see §4.)

**Also fix stale comment** on line 7 of `install-foundry-linux-retro-tools.sh`:
`DEBIAN/control` → `debian/control`.

### 4. Drop the f9dasm source-build sidecar

`install-foundry-linux-retro-tools.sh` lines ~135-140 (the `# --- f9dasm` block — clone + make) become dead code once Phase 0 apt-installs `foundry-linux-retro-tools` (which `Depends:` on `f9dasm`). Delete it. Same pattern as the xa65 sidecar retirement in [`2026-05-18-retire-xa65.md`](2026-05-18-retire-xa65.md), just for f9dasm.

### 5. Update worldfoundry-dev metapackage `Depends:` (if needed)

`worldfoundry-dev`'s `debian/control` already `Depends:` on `worldfoundry-engine-build-deps`, `worldfoundry-blender`, `foundry-linux-retro-tools`, etc. — so `apt install worldfoundry-dev` recursively pulls everything. No change needed; verifying in step 6.

### 6. `install-foundry-linux-dev.sh` collapses to a single apt call

Currently calls the constituent scripts in order. After this plan, it just
`apt install worldfoundry-dev` and lets the metapackage's `Depends:` resolve.
Becomes ~10 lines instead of ~30. Also fix the stale Phase 1 collapse comment
on line 14 which incorrectly says `foundry-linux-dev` (the real package is `worldfoundry-dev`).

## Verification

1. **`setup-foundry-apt-source.sh -h` short-circuits.** Exits 0, no apt operations, no sudo.

2. **First run configures the source.**

    ```bash
    # In a fresh ubuntu:26.04 container
    bash foundry-linux-setup/setup-foundry-apt-source.sh
    test -f /etc/apt/keyrings/foundry.gpg && echo "PASS keyring"
    test -f /etc/apt/sources.list.d/foundry.list && echo "PASS list"
    grep -q "apt.foundrylinux.org" /etc/apt/sources.list.d/foundry.list && echo "PASS contents"
    ```

3. **Re-run is idempotent.**

    ```bash
    bash foundry-linux-setup/setup-foundry-apt-source.sh
    # Expect: "Foundry apt source already configured ... skipping"
    # No re-write of keyring or list file.
    ```

4. **`apt-cache policy foundry-linux-retro-tools` resolves from foundry-apt.**

    ```bash
    apt-cache policy foundry-linux-retro-tools
    # Expect: Candidate 1.0.2 from https://apt.foundrylinux.org
    ```

5. **Collapsed retro-tools script installs the same package set as the old one.**

    Before: `apt list --installed | sort > /tmp/before`
    Run new script in fresh ubuntu:26.04 container.
    After: `apt list --installed | sort > /tmp/after`
    Diff should be empty (modulo `foundry-linux-retro-tools` itself appearing in the new run as an installed metapackage).

6. **Source-build sidecars for libvgm, vgmstream, Ghidra still execute.**

    ```bash
    bash foundry-linux-setup/install-foundry-linux-retro-tools.sh --dry-run | grep -E "git clone"
    # Expect: 3 git clone lines (libvgm, vgmstream, ghidra). NO f9dasm.
    ```

7. **`install-foundry-linux-dev.sh --dry-run` shows the single `apt install foundry-linux-dev` (instead of dispatching to constituents).**

8. **End-to-end live install in fresh ubuntu:26.04 container** — `bash install.sh --role engine-dev --force --apt-only` succeeds and `foundry-linux-retro-tools` is installed via apt.

## Out of scope

- **Packaging libvgm / vgmstream / ghidra.** Tracked separately in TODO.md.
- **Per-meta script removal.** The collapsed per-meta scripts still exist for users who only want a subset role (e.g. `--role engine-dev`). After all four retro tools ship as packages, the per-meta scripts may become 1-liners worth deleting in favor of the orchestrator directly invoking `apt install <metapackage>`.
- **install-task.sh.** Stays — `task` comes from Cloudsmith's apt repo, not foundry-apt, because Cloudsmith is the upstream-maintained source.

## Open questions

- **Should the setup script handle GPG-key rotation?** Today the foundry signing key is single-purpose, long-lived. If rotated, users would need to re-import. For now, the script writes the key if absent and trusts that the same fingerprint is canonical. A rotation playbook is out of scope.
- **Should we also publish `noble` alias as the README claims?** The README says `noble` aliases `resolute` for 24.04 backports. Aptly's `Suite` field is `resolute`; serving a `noble` alias requires either a second publish or an HTTP-level rewrite. Defer — current foundry-apt is 26.04-only.
