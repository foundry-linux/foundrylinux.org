# Kiosk mode (gamescope + wf-launcher) — blocked, contract only

**Date:** 2026-09-03
**Status:** **BLOCKED — not started.** No design, no code, no packaging.
**Blocked on:** `wf-launcher`, which does not exist. Re-confirmed 2026‑09‑03 (evidence below).
**TODO entry:** `## Open` → *Phase 3 — Foundry Linux ISO* → `[T4] Kiosk mode (gamescope + wf-launcher)`
**Parent plan:** [Phase 3 — Foundry Linux ISO](2026-05-22-phase-3-foundry-iso.md) §"Out of scope (follow-up plans)"

---

## TL;DR

Kiosk mode cannot be planned yet, and this document exists so that fact stops costing a fresh
investigation every time the item is dispatched. `wf-launcher` — the tile-grid game launcher that
the kiosk session is built *around* — has never been written. Everything downstream of it
(`gamescope-session.target`, the `worldfoundry-live-kiosk` metapackage, auto-login) is 30 lines of
systemd unit and a `Depends:` line; none of it can be specified, let alone tested, without knowing
what process it is supposed to host.

What this document does instead: record the confirmed block with a **re-runnable** evidence
command, write down the **contract** `wf-launcher` must satisfy to be a valid gamescope-session
target, and define the **unblock gate** — a mechanical checklist a future dispatch can evaluate in
one command rather than re-deriving from scratch.

This plan stays in `## Open`, un-implemented, until the gate passes.

---

## Evidence the block still holds (2026‑09‑03)

`wf-launcher` appears **only as prose**, in three documents, all in this repo. There is no source
tree, no package, no repository, and no renamed successor.

```bash
# 1. No source file, anywhere under ~, mentions it — only docs and Claude session transcripts.
grep -rIl --exclude-dir=.git --exclude-dir=node_modules --exclude-dir=dist \
     --exclude-dir=target --exclude-dir=.venv --exclude-dir=build --exclude-dir=public \
     -i 'wf-launcher' ~/ 2>/dev/null
#   → only ~/.config/claude/**/*.jsonl (session transcripts) and, inside this repo:
#       docs/plans/2026-05-22-phase-3-foundry-iso.md
#       docs/investigations/2026-05-16-foundry-linux-distro-proposal.md
#       TODO.md

# 2. Nothing on disk is even *named* like it.
find ~ -maxdepth 6 \( -name node_modules -o -name .git -o -name dist \) -prune -o \
     -iname '*wf-launcher*' -print 2>/dev/null
#   → (no output)

# 3. Neither GitHub org has such a repo.
gh repo list foundry-linux --limit 100 --json name --jq '.[].name'
#   → foundrylinux.org, foundry-apt, foundry-iso, foundry-devbox
gh repo list wbniv --limit 200 --json name,description --jq '.[] | .name' | grep -i 'launch\|kiosk\|front\|shell'
#   → (no matches)

# 4. Neither apt repo ships it, and no package supersedes it.
ls ~/worldfoundry.org/apt/packages/          # 15 packages: the 9 CLIs, the Blender add-on, 4 umbrellas, editor-dev
ls foundry-apt/packages/ | grep -i 'launch\|kiosk'
#   → (no output)
```

**Checked for a rename / successor, found none.** The candidate that comes closest is
`foundry-welcome` (Qt6/Kirigami, `foundry-apt/packages/foundry-welcome/`), which is a *first-login
information panel* — three static panels of intro text and links, gated by a
`~/.config/foundry-welcome-shown` sentinel. It neither enumerates games nor launches them, so it
does not fill the role. No WorldFoundry roadmap doc describes a differently-named launcher.

**One thing did change in our favour:** `gamescope` is packaged in Ubuntu 26.04 — version
`3.16.20+ds-1`, in **multiverse** (verified in a fresh `ubuntu:26.04` container). It does not need
vendoring. It does mean the kiosk path has to enable multiverse, the way
[`install-foundry-android-development.sh`](../../foundry-setup/install-foundry-android-development.sh)
already does.

---

## Why a stub rather than a speculative plan

Rejected: designing the kiosk session against a hypothetical launcher and letting `wf-launcher`
conform to it later. That inverts the dependency. The session unit is the trivial half — the hard
constraints (input handling, child-process lifetime, exit semantics, empty state) all live *inside*
the launcher, and a spec written before the launcher exists would be guessing at them and would be
rewritten on first contact. Writing the contract instead captures the part that is genuinely
knowable now and is what the launcher's own implementation plan needs as input.

Also rejected: expanding the phase‑3 plan's out-of-scope bullet in place. That plan is shipped
history; a blocked future-phase contract buried in its out-of-scope list is a document nobody
re-opens. The bullet gets a pointer here instead.

---

## The contract `wf-launcher` must satisfy

For `wf-launcher` to be hostable as `gamescope -e -f -- wf-launcher` under a
`gamescope-session.target`, it must meet all of the following. Each item exists because violating
it produces a *black screen with no way out* on a machine that may have no keyboard — the kiosk
failure mode has no recovery path, which is why the bar is this specific.

### 1. Process lifetime — the launcher **is** the session

- Runs in the foreground; must not fork-and-exit or hand off to a separate long-lived process.
  gamescope terminates when its direct child terminates, and the session unit is `Type=simple`.
  An Electron-style relauncher or a `exec`-into-daemon pattern collapses the session at boot.
- **Exit codes are session control:** `0` = user chose Quit → stop the target cleanly (return to
  greeter or power off, per the unit's `OnSuccess=`); non-zero = crash → `Restart=on-failure`.
  A launcher that returns 0 on crash makes the kiosk silently shut down instead of recovering.

### 2. Display — a plain client, never a compositor

- Native Wayland (xdg-shell) client, or an X11 client that behaves under gamescope's embedded
  Xwayland. It must not attempt to be a compositor or a session manager itself.
- **No desktop-environment dependency.** No running `plasmashell`/`kwin`, no `org.kde.*` session
  services, no systray, no notification daemon, no `xdg-desktop-portal` unless the kiosk seed
  explicitly pulls one in. This is the constraint that rules out reusing `foundry-welcome`'s
  Kirigami stack unexamined.
- Takes its size from gamescope's nested resolution (`-W`/`-H`) and tolerates being resized.
  No hardcoded 1920×1080 — the live USB boots on whatever panel is in front of it.

### 3. Input — gamepad-first is a hard requirement, not a nicety

- Full navigation from a gamepad alone via evdev / `SDL_GameController`: A = select, B = back,
  Start = quit-to-launcher. Keyboard is the fallback (Enter/Esc), mouse is optional.
  A kiosk on a TV may have no keyboard attached at all.
- Handles hotplug — a controller connected *after* launch must work.
- Works as the unprivileged auto-login user, via logind seat ACLs on `/dev/input/event*`
  (or `input` group membership). Never root.

### 4. Child processes — launching a game must be survivable

This is the subtle half and the one most likely to be got wrong:

- Spawns `wf_game -L<path>` (and emulators) as children **inside the same gamescope instance**;
  reaps them; does not exit while a game is running.
- Restores focus to itself when the game exits.
- Provides an **escape hatch for a hung game** — e.g. hold Start for 3 s to `SIGKILL` the child.
  Without this, one crashed game bricks the kiosk: no TTY, no window manager, no way back.

### 5. Game discovery — declarative, so packaging stays decoupled

- Reads a manifest from disk rather than knowing about specific titles: a per-game descriptor
  (`game.json` or a `.desktop`-style file) under scan roots `/opt/wf-games/`,
  `/media/*/wf-games/`, `~/.local/share/wf-games/`.
- This is what lets `worldfoundry-live-kiosk` be a *metapackage* — games ship as independent
  `.deb`s that just drop a manifest — instead of a monolithic bundle that must be rebuilt per title.
  It is also what makes the bring-your-own-ROMs path from the proposal work.

### 6. Degrades on a read-only, offline live USB

- Starts with a read-only `$HOME` and no `casper-rw`: no saves, but no failure.
- Never blocks startup on network.
- **Non-empty empty state.** Zero games found must render something useful ("insert a USB stick
  with `wf-games/`…"), not a blank grid. On a live USB this is a *likely* state, not an edge case.

### 7. Packaged, and usable outside the kiosk

- Ships as a `wf-launcher` `.deb` on **`apt.worldfoundry.org`** (it is a WorldFoundry tenant tool,
  not distro toolchain — per the repo-split rule in `CLAUDE.md`), with a `.desktop` entry so it
  also runs windowed inside a normal Plasma session.

---

## Unblock gate

A future dispatch should evaluate this before doing anything else. All four must hold:

- [ ] A `wf-launcher` source tree exists and builds.
- [ ] It satisfies contract items **1, 2, 3, 4** — the four that have no recovery path if wrong.
      Items 5–7 can be finished during the kiosk plan itself.
- [ ] It runs standalone under `gamescope -e -f -- wf-launcher` on a 26.04 box: launches a game,
      returns to the grid on exit, and quits cleanly on Start.
- [ ] It is published on `apt.worldfoundry.org`, or is at least buildable into a `.deb`.

If the gate fails, re-run the evidence block above, update the date, and leave the item in
`## Open`. Do not design around the absence.

---

## Decisions already taken — do not re-litigate

- **Launcher implementation: (a) custom app, written from scratch.** Recorded 2026‑05‑16
  (`~/WorldFoundry-wbniv/docs/transcripts/2026-05-16-session.md`, turn 167): the options were
  (a) custom SDL/GTK4, (b) EmulationStation‑DE, (c) Pegasus Frontend, (d) Steam Big Picture; the
  answer was *"yes, (a) to start. investigate others later"*. A future evaluation of (b)/(c) is
  allowed; silently adopting one is not.
- **Kiosk is live‑ISO only.** `worldfoundry-live-kiosk` is seeded into the live build and is never
  installed on a regular system ([proposal §515-545](../investigations/2026-05-16-foundry-linux-distro-proposal.md)).
- **Not blocking v1.** The v1 ISO ships two boot modes — Install (Calamares) and Try Live (full
  Plasma). Kiosk is a third, added later.

## Open questions for the real plan

- **Which repo owns `worldfoundry-live-kiosk`?** The name says `apt.worldfoundry.org`, but the
  content — gamescope, auto-login, a systemd target, a live-build seed — is foundry-linux distro
  infrastructure, and the ISO builder lives in `foundry-iso`. The proposal never resolved this.
  Likeliest answer is a split: a distro-side `foundry-kiosk-session` (session plumbing) that
  `worldfoundry-live-kiosk` depends on alongside `wf-launcher` and the games.
- **Auto-login mechanism** — SDDM autologin vs a `getty@tty1` override vs a dedicated greeter-less
  user. Interacts with how Calamares-installed systems must *not* inherit the kiosk.
- **multiverse enablement** — gamescope is in multiverse; decide whether the ISO seed enables it
  archive-wide or pins just this package.

## Mockups

None yet, deliberately. This document designs no visible surface — it is a contract and a gate.
The launcher UI states (boot splash → tile grid → in-game → empty state → hung-game escape) are the
mockup set for the *real* plan, and they cannot be drawn honestly before the launcher's own scope
exists.

## Verification

Not applicable — nothing is implemented. The evidence block above is the only runnable content, and
it is the check to re-run at next dispatch.
