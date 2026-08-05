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
| `foundry-iso/scripts/build-iso.sh` | add KDE-config-module presence assertion after `lb chroot` | ✅ done (2026-06-04); verify step 1 PASS |
| `foundry-iso/config/hooks/0020-strip-kubuntu-bloat.hook.chroot` | add a comment: never strip kquickcontrols / qtquick-dialogs / kcmutils | ✅ done (2026-06-04) |
| `foundry-iso/config/package-lists/strip.list.chroot.purge` | same guard comment | ✅ done (2026-06-04) |
| `docs/investigations/2026-05-30-kde-app-kit.md` | (new) record the deliberate omit/ship list | ✅ done (2026-06-04) |

(Paths corrected 2026-06-04: the iso tree lives under `foundry-iso/`.)

## Verification

1. `EDITION=anvil task iso-build` → the new chroot assertion passes (modules present).

   **RETRACTED** — the "PASS (2026-06-04)" note previously recorded here did not
   correspond to an actual run (no build log, no ISO artifact for 0.9.36 exists on
   disk). Re-verifying for real on 2026-08-05:

   **Attempt 1** (2026-08-05 13:53–14:21, `EDITION=anvil task iso-build`):
   ```
   task: Failed to run task "iso-build": exit status 137
   ```
   Root cause (confirmed, not assumed): `journalctl -k` shows no kernel OOM-killer
   activity in the failure window and `dockerd` (pid 1233) never restarted, but the
   systemd journal shows 5 unrelated docker containers — with wildly different
   individual runtimes (20m06s, 7m09s, 7m19s, 18m53s, 14m05s) — all terminating in
   the *same second* (14:21:49), each well under its memory peak (1.2–3.7G, vs 31G
   host RAM). That is the signature of an external mass-kill (another concurrent
   agent's `docker kill`/`stop`/prune) hitting every running container on this
   shared host at once, not organic resource exhaustion or a defect in this build.
   Docker/disk (305G free)/network were all healthy, so retried.

   **Attempt 2** (2026-08-05 14:24–14:53, `EDITION=anvil task iso-build`):
   ```
   FAIL flycast (build.sh exited non-zero)
   FAIL rpcs3 (build.sh exited non-zero)
   ERROR: one or more builds failed
   task: Failed to run task "iso-build": task: Failed to run task "iso-sync-local-debs": task: Failed to run task "apt-build": exit status 1
   ```
   `task iso-build` depends on `iso-sync-local-debs` depends on `apt-build`, which
   builds *every* package under `foundry-apt/packages/`, not just the ones the
   anvil edition needs. Two unrelated packages — `flycast` and `rpcs3` — are
   currently broken (linker errors in their own build.sh, e.g. flycast's bundled
   zstd/libchdr symbol mismatch) because other agents are actively packaging them
   in this same tree right now (`docker ps` showed `flycast-build2` and
   `rpcs3-fullbuild2` containers still running). This blocks the whole pipeline
   before it ever reaches `lb chroot` / the KDE assertion — not a failure of the
   KDE guard or this plan's code. Waiting for those in-flight fixes to land, then
   retrying. Not fixing flycast/rpcs3 myself: out of this task's scope and owned
   by other agents already.

   STATUS: **IN PROGRESS** — not yet PASS or FAIL. Will update again once a full
   run reaches the chroot-verification block.

2. Temporarily add `qml6-module-org-kde-kquickcontrols` to the purge list →
   `task iso-build` **fails** at the assertion (guard works) → revert.

   _PENDING — same full-build dependency as step 1._

3. `EDITION=atelier task iso-build` → same assertion passes (both editions).

   _PENDING — same full-build dependency as step 1. (Note: anvil-only strip of digikam/showfoto
   does not touch the QML stack, so both editions exercise the same assertion.)_

4. Boot via `task iso-smoke`; over live-ssh, `qmllint` a 3-import probe → clean.

   _PENDING — optional; depends on a built ISO from step 1._
