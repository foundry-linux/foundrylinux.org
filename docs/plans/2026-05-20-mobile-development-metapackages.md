# Plan: `foundry-android-development` + `foundry-ios-development` metapackages

## Context

Two mobile-dev metapackages to add to `apt.foundrylinux.org` (`foundrylinux.org/foundry-apt/`):

**Android:** `install-foundry-android-development.sh` currently does a direct `apt install
openjdk-17-jdk adb google-android-ndk-r26c-installer` without a backing metapackage — the last
install script without one. All three deps are in Ubuntu multiverse/universe. Adding the
metapackage completes the pattern every other install script already follows.

**iOS:** WorldFoundry has an active iOS port (Metal backend, UIKit, Codemagic cloud CI on macOS).
The actual compile happens on Codemagic — Linux contributes editing, git, and device-side
tooling only. Four device-communication packages (`libimobiledevice-utils`, `ideviceinstaller`,
`usbmuxd`, `ifuse`) are all in Ubuntu universe and go into the metapackage.

`codemagic-cli-tools` (Codemagic's Python CLI for App Store Connect, code signing, and artifact
management) is pip-only — not in any apt source — so it cannot be a `Depends:` in the deb.
Instead, the **Phase 0 install script** (`install-foundry-ios-development.sh`, a new file)
handles it: the script does `apt install foundry-ios-development` for the apt packages,
then `pipx install codemagic-cli-tools` as a second step. This two-layer split (metapackage for
apt deps, install script for the rest) is the same pattern used by the android script, which
separately enables multiverse for the NDK — something also inexpressible in a `Depends:`.

Both packages get added to the `maintainer` role in `install.sh`.

## Target shapes

```
foundry-android-development  (1.0.0)   ← apt.foundrylinux.org
└─ Depends: openjdk-17-jdk, adb, google-android-ndk-r26c-installer
   (google-android-ndk-r26c-installer is in Ubuntu multiverse)

foundry-ios-development  (1.0.0)       ← apt.foundrylinux.org
└─ Depends: libimobiledevice-utils, ideviceinstaller, usbmuxd, ifuse
   (all Ubuntu universe; Codemagic CLI tools installed separately via pipx)
```

## Files to create / modify

### New: `foundry-apt/packages/foundry-android-development/debian/` (5 files)

Reference: `foundry-apt/packages/foundry-retro-tools/debian/` — identical structure.

**control:**
```
Source: foundry-android-development
Section: metapackages
Priority: optional
Maintainer: World Foundry <packages@worldfoundry.org>
Build-Depends: debhelper-compat (= 13)
Standards-Version: 4.7.0
Homepage: https://foundrylinux.org/
Rules-Requires-Root: no

Package: foundry-android-development
Architecture: all
Depends: ${misc:Depends}, openjdk-17-jdk, adb, google-android-ndk-r26c-installer
Description: Android development tools for Foundry Linux contributors
 Metapackage that installs the apt-available Android toolchain needed
 to build and deploy WorldFoundry to Android devices:
 .
   * openjdk-17-jdk                    — Java 17, required by Gradle
   * adb                               — Android Debug Bridge
   * google-android-ndk-r26c-installer — NDK r26c (26.2.11394342),
                                         installed to /usr/lib/android-sdk/ndk/
 .
 Note: google-android-ndk-r26c-installer is in Ubuntu multiverse. Enable
 it first: sudo add-apt-repository multiverse
 .
 After installing, bootstrap the Android SDK from the WorldFoundry source
 tree (platforms;android-34, build-tools;34.0.0, cmake;3.22.1):
 .
   task android-sdk-install
 .
 Gradle 8.9 is vendored in the WorldFoundry repo (gradlew wrapper).
```

**changelog:** 1.0.0 initial entry (same style as foundry-retro-tools)
**copyright:** GPL-2.0-or-later, packaging © World Foundry
**rules:** `%: dh $@`
**source/format:** `3.0 (native)`

### New: `foundry-apt/packages/foundry-ios-development/debian/` (5 files)

Same structure as above.

**control:**
```
Source: foundry-ios-development
Section: metapackages
Priority: optional
Maintainer: World Foundry <packages@worldfoundry.org>
Build-Depends: debhelper-compat (= 13)
Standards-Version: 4.7.0
Homepage: https://foundrylinux.org/
Rules-Requires-Root: no

Package: foundry-ios-development
Architecture: all
Depends: ${misc:Depends}, libimobiledevice-utils, ideviceinstaller, usbmuxd, ifuse
Description: iOS device tools for Foundry Linux contributors
 Metapackage that installs Linux-side tooling for WorldFoundry iOS
 development. The iOS build itself runs on macOS via Codemagic cloud CI
 (codemagic.io); this package covers the device-communication side on Linux:
 .
   * libimobiledevice-utils — query, backup, and manage iOS devices over USB
   * ideviceinstaller        — sideload .ipa files to a connected device
   * usbmuxd                 — USB multiplexing daemon (required by the above)
   * ifuse                   — mount iPhone/iPod filesystem via FUSE
 .
 The install script (install-foundry-ios-development.sh) also installs
 the Codemagic CLI tools via pipx — these provide the app-store-connect(1)
 command for App Store Connect API access and build artifact management.
```

### New: `foundry-setup/install-foundry-ios-development.sh`

Pattern: mirrors `install-foundry-android-development.sh`.

Steps:
1. `set -euo pipefail`, `--help` / `--dry-run` handling via `lib.sh`
2. Call `setup-foundry-apt-source.sh` — wire apt.foundrylinux.org
3. `apt install foundry-ios-development` — device tools
4. `pipx install codemagic-cli-tools` — Codemagic CLI (pip-only, not in apt)

### Modify: `foundry-setup/install-foundry-android-development.sh`

Replace the direct `apt-get install openjdk-17-jdk adb google-android-ndk-r26c-installer` block with:
1. Call `setup-foundry-apt-source.sh`
2. `add-apt-repository multiverse -y` (needed for NDK)
3. `apt install foundry-android-development`

### Modify: `foundry-setup/install.sh`

Add `install-foundry-ios-development.sh` to the `maintainer` role block, after the existing `install-foundry-android-development.sh` call (~line 228).

### Modify: `CLAUDE.md`

Update the script table:
- android-development line: `apt install foundry-android-development (apt.foundrylinux.org → JDK 17, adb, NDK r26c)`
- Add ios-development line: `apt install foundry-ios-development (apt.foundrylinux.org → libimobiledevice, ideviceinstaller, usbmuxd, ifuse) + pipx codemagic-cli-tools`

## Implementation steps

1. Scaffold `foundry-apt/packages/foundry-android-development/debian/` (5 files).
2. Scaffold `foundry-apt/packages/foundry-ios-development/debian/` (5 files).
3. `task foundry-apt:build` — expect 2 new debs in dist/.
4. Lintian both debs — expect empty output each.
5. Write `foundry-setup/install-foundry-ios-development.sh`.
6. Update `install-foundry-android-development.sh` to route through the metapackage.
7. Add ios-development to `maintainer` role in `install.sh`.
8. Update CLAUDE.md script table.
9. Commit, push, bump foundry-apt tag → CI publishes.

## Verification

1. **Both debs build.**
   ```
   ls foundry-apt/dist/foundry-android-development_1.0.0_all.deb
   ls foundry-apt/dist/foundry-ios-development_1.0.0_all.deb
   ```

2. **Both lintian-clean.**
   Empty output from lintian on each.

3. **Android resolved-install set** (ubuntu:26.04 + multiverse + local foundry-apt source):
   `adb`, `google-android-ndk-r26c-installer`, `openjdk-17-jdk` all present.

4. **iOS resolved-install set** (ubuntu:26.04 + local foundry-apt source):
   `libimobiledevice-utils`, `ideviceinstaller`, `usbmuxd`, `ifuse` all present.

5. **Script dry-runs correct.**
   ```
   bash foundry-setup/install-foundry-android-development.sh --dry-run
   bash foundry-setup/install-foundry-ios-development.sh --dry-run
   bash foundry-setup/install.sh --role maintainer --dry-run
   ```
   Android: shows `foundry-android-development` (not bare package names).
   iOS: shows `foundry-ios-development` + `pipx install codemagic-cli-tools`.
   Maintainer: both scripts appear in the chain.

6. **Complete package inventory — `apt.foundrylinux.org`** (after publish):
   ```
   curl -s https://apt.foundrylinux.org/dists/stable/main/binary-amd64/Packages \
     | grep '^Package:' | awk '{print $2}' | sort
   ```
   Expected (6 packages):
   ```
   f9dasm
   foundry-android-development
   foundry-ios-development
   foundry-retro-tools
   ghidra
   libvgm
   vgmstream
   ```

7. **Complete package inventory — `apt.worldfoundry.org`** (unchanged by this work):
   ```
   curl -s https://apt.worldfoundry.org/dists/stable/main/binary-amd64/Packages \
     | grep '^Package:' | awk '{print $2}' | sort
   ```
   Expected (14 packages — `blender-asset-finder` and `blender-asset-finder-cli` moved to apt.foundrylinux.org; `wf-asset`→`blender-asset-finder-cli`, `wf-blender`→`worldfoundry-blender-editor-exporter`, `worldfoundry-blender`→`worldfoundry-blender-addons` all renamed 2026-05-21):
   ```
   cdpack
   iffcomp
   iffdump
   levcomp
   lvldump
   oaddump
   oas2oad
   prep
   textile
   worldfoundry
   worldfoundry-blender-addons
   worldfoundry-blender-editor-exporter
   worldfoundry-cli
   worldfoundry-development
   ```

8. **Live publish** — bump foundry-apt tag → CI green.
