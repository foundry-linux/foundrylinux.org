# Make the lock-screen clock use 24-hour time

**Date:** 2026-10-10

**Status:** Package built; Anvil ISO rebuild and installed-system verification pending

## Problem and cause

The Foundry panel clock already uses `use24hFormat=2` in the first-login Plasma layout. That setting belongs to the **panel digital-clock applet** and does not configure the screen locker. Will still sees a 12-hour time after locking the session.

In Plasma 6.6, the lock screen creates Breeze's `Clock` component. Its time label uses `Qt.formatTime(timeSource.dateTime, Qt.locale(), Locale.ShortFormat)`, so it follows the session's time locale. With `en_US`, that format includes an AM/PM marker. Foundry ships wallpaper settings in `kscreenlockerrc`, but no time-locale default. Changing `kscreenlockerrc` or repeating the panel applet setting cannot change this clock. [KDE lock-screen source](https://github.com/KDE/plasma-desktop/blob/Plasma/6.6/desktoppackage/contents/lockscreen/LockScreenUi.qml), [KDE Breeze clock source](https://github.com/KDE/plasma-workspace/blob/Plasma/6.6/lookandfeel/components/Clock.qml), [Qt locale behavior](https://doc.qt.io/qt-6/qml-qtqml-locale.html).

## Immediate fix for an installed account

In **System Settings → Region & Language**, set **Time** to **English (United Kingdom)**, then sign out and sign in again before locking the screen. KDE documents that Time can have its own regional format and stores the choice in `~/.config/plasma-localerc`. [KDE Region & Language guide](https://docs.kde.org/stable_kf6/en/plasma-workspace/kcontrol/region_language/).

Will ran this command for the current account on 2026-10-10; the result after signing out and back in has not yet been reported:

```bash
kwriteconfig6 --file plasma-localerc --group Formats --key LC_TIME en_GB.UTF-8
```

This changes the user's time locale, so applications using locale defaults can also change their **date** presentation. It does not change the language of the desktop or the panel's explicit 24-hour clock setting. Keep that tradeoff visible in the user-facing instructions.

## Distribution fix implemented

- [x] Add `[Formats] LC_TIME=en_GB.UTF-8` as a system default at `/etc/xdg/plasma-localerc`, owned by `foundry-kde-theme` 1.0.10. This matches Will's per-user command. It leaves the panel's explicit `use24hFormat=2` and the wallpaper's `kscreenlockerrc` intact.
- [x] Check for a package-owner conflict: `dpkg -S /etc/xdg/plasma-localerc` found none on the Ubuntu 26.04 host. In isolated KConfig reads, the system value is visible to a user with no Time choice (including one whose `plasma-localerc` sets only `LANG=en_US.UTF-8`), while a user `LC_TIME=en_US.UTF-8` takes precedence.
- [x] Build `foundry-kde-theme_1.0.10_all.deb` in the Ubuntu 26.04 package builder and inspect its contents: `/etc/xdg/plasma-localerc` is included. The ISO already installs `foundry-kde-theme` through `foundry-desktop`/`foundry-anvil`, so a subsequent ISO build will carry the default.
- [x] Stage the 1.0.10 package in the external Anvil build workspace and bump the local ISO version to 0.9.138.
- [ ] Confirm the generated `en_GB.UTF-8` locale is present in both the live ISO and a Calamares-installed system. The ISO package list includes `language-pack-en`, whose locale generation should be checked in the actual image.
- [ ] Confirm the lock greeter uses the merged locale default after a new login. If it does not, investigate Plasma's session locale export before adding an account-file workaround. Do not overwrite users' `~/.config/plasma-localerc` during upgrades.
- [ ] Document the installed-account control in the user guide so users who already selected a Time region can change it deliberately.

## Anvil rebuild status

- [x] Start the Anvil build on the external ext4 drive. The first run reached package installation, then failed during the `dictionaries-common` trigger because rootless Docker exposed the workspace as `nodev`; `/dev/null` inside the chroot was an unusable regular file.
- [x] Update `build-iso.sh` to switch rootless sessions to the standard Docker daemon through desktop authorization, enable device nodes on removable build mounts, and stop early for read-only or `nodev` workspaces. Correct the cleanup ownership mapping for rootless Docker. `bash -n`, ShellCheck, and `git diff --check` pass. The rootful end-to-end build is not yet verified.
- [ ] Replace or secure the external drive cable, then confirm the drive remains mounted read-write without USB disconnect or ext4 journal errors. The kernel logged repeated disconnects through 21:26 on 2026-10-10, and the drive disappeared after the last one. Do not run another long build until the connection is stable.
- [ ] Sync the updated build script into the external workspace, build Anvil 0.9.138, retain its build log, and verify the ISO and checksum. Do not start Atelier as part of this verification.

## Verification and acceptance

- [x] Build the `foundry-kde-theme` package and inspect the `.deb` to confirm the chosen default is installed at the intended path; check file ownership against Kubuntu packages.
- [ ] Boot a fresh Anvil ISO and an installed-system VM. Lock each session and visually confirm the lock-screen clock shows a 24-hour time after 13:00, with no AM/PM. Confirm the panel remains 24-hour and unlock still works.
- [ ] Test a fresh user, an existing user with no Time override, and an existing user who explicitly selected a 12-hour Time region. The packaged default should affect the first two; the explicit choice must remain intact.
- [ ] Check the date format in Dolphin or another locale-aware application and record the effect of the chosen Time region. The same setting affects locale-formatted dates as well as the lock-screen time.
- [ ] Verify both the live ISO and a Calamares-installed system after a package upgrade; the theme package must survive installation and the config must not trigger a `dpkg` conffile prompt.

**Done when:** a fresh Foundry session displays 24-hour time on both the panel and lock screen, installed users have a documented fix, and a user's explicit regional choice remains respected.
