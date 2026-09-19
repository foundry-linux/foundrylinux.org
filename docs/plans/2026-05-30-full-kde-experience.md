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

   **Re-verified 2026-09-19 against a real, complete build** — `foundry-anvil-0.9.133-amd64.iso`
   (`EDITION=anvil task iso-build`, run 21:11–22:44, log
   `foundry-iso/dist/build-anvil-0.9.133.log`):

   ```
   $ grep -n "PASS: KDE config QML stack" foundry-iso/dist/build-anvil-0.9.133.log
   17394:PASS: KDE config QML stack present (kcmutils, kquickcontrols, QtQuick.Dialogs)
   $ grep -n "PASS: KDE config QML stack" /tmp/claude-1000/-home-will-foundrylinux-org/ef75e775-3e8a-401a-a09b-a21ee7044809/scratchpad/iso-build.log
   17794:PASS: KDE config QML stack present (kcmutils, kquickcontrols, QtQuick.Dialogs)
   ```

   The assertion (`foundry-iso/scripts/build-iso.sh:243-257`) ran after `lb chroot`
   and printed its PASS line in both the per-build log and the full task-runner
   log, confirming all three modules (`org/kde/kcmutils`, `org/kde/kquickcontrols`,
   `QtQuick/Dialogs`) were present in the chroot for this build. The ISO itself
   landed at `foundry-iso/dist/foundry-anvil-0.9.133-amd64.iso` (4.4G).

   **PASS.**

2. Temporarily add `qml6-module-org-kde-kquickcontrols` to the purge list →
   `task iso-build` **fails** at the assertion (guard works) → revert.

   **NOT RUN 2026-09-19.** This is a negative test that requires its own full
   `lb chroot` cycle (edit `strip.list.chroot.purge`, rerun `task iso-build`,
   observe the failure, revert, and — to leave a clean state — rebuild again or
   discard the run). The 0.9.133 build that step 1 verifies against took ~93
   minutes (21:11–22:44) end to end; a second full rebuild solely to exercise
   this negative path was judged out of scope for this verification pass and
   was not run. Additionally, the `foundry-iso/chroot/` tree this task was
   dispatched with the promise of ("the live-build chroot tree is still
   present") is no longer on disk — confirmed absent (`ls chroot` → "No such
   file or directory"; only the manifest files `chroot.packages.install`,
   `chroot.packages.live`, `binary.packages`, `chroot.headers` remain, and the
   build log has no explicit `rm -rf chroot` at that point, so it's unclear
   whether `lb binary`'s own packaging stage reclaims it or another process on
   this shared host removed it). That rules out a cheaper substitute (editing
   the on-disk chroot directly and re-running just the assertion snippet)
   without first reproducing a chroot. Left **PENDING** — needs a dedicated
   full rebuild scheduled on its own, same as step 1's original gating.

3. `EDITION=atelier task iso-build` → same assertion passes (both editions).

   **NOT RUN 2026-09-19.** Same full-build dependency as step 2: only the
   anvil edition was built this session (0.9.133). Exercising this step means
   a separate `EDITION=atelier task iso-build`, another ~90+ minute run, not
   performed here. Left **PENDING**. (Note: anvil-only strip of digikam/showfoto
   does not touch the QML stack, so both editions exercise the same assertion.)

4. Boot via `task iso-smoke`; over live-ssh, `qmllint` a 3-import probe → clean.

   **NOT RUNNABLE 2026-09-19.** `task iso-smoke` → `foundry-iso/test/boot-smoke.sh`
   launches `qemu-system-x86_64 ... -display gtk,gl=on`, i.e. it opens a real,
   interactive GTK/OpenGL window. This session is running on the user's own
   live desktop, not a headless CI box — confirmed via `ps aux`, which shows an
   active `kwin_wayland` / `Xwayland :0` session (`DISPLAY=:0` reachable,
   `glxinfo` reports `direct rendering: Yes`). Launching `iso-smoke` here would
   pop an uninvited GUI window on the user's actual screen, which this task
   does not have standing to do. `task iso-qemu` has the same
   `-display gtk,gl=on` shape (`Taskfile.yml:518-537`) and shares the problem.
   Recording as NOT RUNNABLE rather than faking a boot/qmllint transcript;
   this needs a human at the display (or a headless variant of
   `boot-smoke.sh`, e.g. `-display none` + serial-only, which would be a
   change to the test script itself and out of this verification task's
   scope).
