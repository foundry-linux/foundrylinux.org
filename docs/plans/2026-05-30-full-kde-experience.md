# Foundry Linux — guarantee a full KDE Plasma experience

## Context

While testing the `claude-usage` KDE plasmoid live on the Foundry anvil ISO
(`foundry-anvil-0.9.30-amd64.iso`, Plasma 6 / Qt 6.10), a question surfaced: does
Foundry ship a *complete* KDE/Plasma config stack, or does the bloat-strip remove
pieces apps rely on?

**Audit result (2026-05-30): the strip is conservative and safe.**
`config/hooks/0020-strip-kubuntu-bloat.hook.chroot` + `strip.list.chroot.purge`
remove only PIM (kontact/kmail/akonadi/kdepim), office (libreoffice), games,
kdeconnect/krdc/krfb/ktorrent, snap, and a few applets — **none** of the QML
config infrastructure. Verified present in the live chroot:
`qml6-module-org-kde-kquickcontrols`, `qml6-module-qtquick-dialogs`, and the
`org.kde.kcmutils` / `org.kde.kquickcontrols` / `QtQuick.Dialogs` QML modules.

So the plasmoid's empty-config-dialog bug was **not** a Foundry gap — it was a
Plasma-5-idiom bug in claude-usage (fixed there). This plan's job is narrower:
**lock in** that Foundry deliberately ships a working KDE config experience and
**guard** against a future strip-list edit silently removing it.

## Goals

1. Make the "full KDE config stack present" property explicit and tested, not
   incidental.
2. Decide, deliberately, which KDE apps round out the default kit (the current
   strip is intentional for a dev-focused distro — this is a review, not a
   reversal).

## Changes

1. **Build-time assertion** (in `scripts/build-iso.sh`, after `lb chroot`, beside
   the existing autologin/casper.conf chroot checks): assert the KDE config QML
   modules exist in the chroot, e.g.
   ```sh
   for m in org/kde/kcmutils org/kde/kquickcontrols QtQuick/Dialogs; do
     test -d "chroot/usr/lib/x86_64-linux-gnu/qt6/qml/$m" \
       || { echo "ERROR: KDE config QML module missing: $m"; exit 1; }
   done
   ```
   A future strip-list edit that pulls one (e.g. via an over-broad `kde-*`
   glob auto-remove) then fails the build instead of shipping a broken config UI.

2. **Smoke-test extension** (`test/boot-smoke.sh` or a new assertion): over the
   live-ssh session, `qmllint` a tiny QML that imports the three modules — proves
   they not only exist but load. (Optional; the build-time dir check is the cheap
   guard.)

3. **App-kit review (decision, not necessarily a change).** Document in
   `docs/investigations/` which KDE apps Foundry intentionally omits vs. ships.
   The current strip (PIM/office/games) is appropriate for a build-focused
   distro; record that rationale so it isn't second-guessed. No code change
   unless we decide to add apps.

## Critical files

| File | Action | Status |
|---|---|---|
| `foundry-iso/scripts/build-iso.sh` | add KDE-config-module presence assertion after `lb chroot` | ✅ done (2026-06-04) |
| `foundry-iso/config/hooks/0020-strip-kubuntu-bloat.hook.chroot` | add a comment: never strip kquickcontrols / qtquick-dialogs / kcmutils | ✅ done (2026-06-04) |
| `foundry-iso/config/package-lists/strip.list.chroot.purge` | same guard comment | ✅ done (2026-06-04) |
| `docs/investigations/2026-05-30-kde-app-kit.md` | (new) record the deliberate omit/ship list | ✅ done (2026-06-04) |

(Paths corrected 2026-06-04: the iso tree lives under `foundry-iso/`.)

## Verification

1. `EDITION=anvil task iso-build` → the new chroot assertion passes (modules present).

   _PENDING — requires a full ISO build (Docker + network, ~slow). Static checks done
   2026-06-04: the assertion block is shellcheck-clean (the only shellcheck findings on
   `build-iso.sh`/hook 0020 are pre-existing SC1091/SC2015 on lines 3 and 25, not the new
   QML loop). The probed module dirs (`org/kde/kcmutils`, `org/kde/kquickcontrols`,
   `QtQuick/Dialogs` under `qt6/qml`) match what the 2026-05-30 chroot audit found present._

2. Temporarily add `qml6-module-org-kde-kquickcontrols` to the purge list →
   `task iso-build` **fails** at the assertion (guard works) → revert.

   _PENDING — same full-build dependency as step 1._

3. `EDITION=atelier task iso-build` → same assertion passes (both editions).

   _PENDING — same full-build dependency as step 1. (Note: anvil-only strip of digikam/showfoto
   does not touch the QML stack, so both editions exercise the same assertion.)_

4. Boot via `task iso-smoke`; over live-ssh, `qmllint` a 3-import probe → clean.

   _PENDING — optional; depends on a built ISO from step 1._
