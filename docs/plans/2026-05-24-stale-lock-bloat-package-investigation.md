# Fix: stale .lock + bloat package investigation

## Immediate fix: stale live-build lock

Stale `.lock` file at `foundry-iso/.lock` (root-owned) from the killed build.
The `build-iso.sh` cleanup inside Docker removes `.build/` and `chroot/` but never `.lock`.

Two changes:
1. **Host**: `sudo rm foundry-iso/.lock` (unblock current build)
2. **`build-iso.sh`**: add `rm -f .lock` to the cleanup block inside the Docker `bash -c` (after the `chattr`/`rm -rf` lines) so future killed builds don't re-lock

Critical file: `foundry-iso/scripts/build-iso.sh` — add `rm -f .lock` at line ~54 (after `rm -rf .build/ chroot/`)

---

# Investigation: Bloat packages on ISO?

## Finding

The packages (LibreOffice, Kontact, DigiKam, Snapd, KDE Games) are **NOT on the final ISO**. Hook `0020-strip-kubuntu-bloat` is working correctly.

### How live-build ordering works

live-build installs packages first (lb chroot, ~line 952 in log: "Begin installing packages"), then runs hooks (line 12899: "Begin executing hooks"). So:

1. `kubuntu-desktop` pulls in LibreOffice, Kontact, DigiKam, etc.
2. Hook `0020` runs `apt-get purge` on those packages — **after** they're installed
3. The packages are gone from the final squashfs

### Why the build log says "not installed, so not removed"

The wildcard `libreoffice-\*` in the purge list matches names from apt's package database that include **old package names** from prior Ubuntu/Kubuntu releases — things like `libreoffice-filter-mobiledev`, `libreoffice-l10n-4.3` — that don't exist in Ubuntu 26.04. Those trigger "not installed" warnings.

The packages that ARE installed (`libreoffice-calc`, `libreoffice-writer`, etc.) are successfully purged.

### Verified via `filesystem.packages`

Docker inspection of the built ISO's `filesystem.packages` confirmed:
- `libreoffice` → **NONE**
- `kontact` → **NONE**
- `digikam` → **NONE**
- `snapd` → **NONE**
- `kde-games` → **NONE**

## Next step

Nothing to fix. Run `task iso-build` in `foundry-iso/` to produce the build-3 ISO with all branding fixes (GRUB labels, install entry, no plasma-welcome popup, install desktop shortcut).
