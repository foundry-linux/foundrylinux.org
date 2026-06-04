---
title: Foundry Linux — KDE app kit (what we deliberately omit vs. ship)
date: 2026-05-30
status: Decision recorded — current strip is intentional and appropriate for a dev-focused distro; no reversal
---

# KDE app kit — deliberate omit/ship list

## Context

Foundry Linux is a Kubuntu 26.04 (Plasma 6.6 / Qt 6.10) remix aimed at **game
developers new to Linux** — see `project_foundry_audience`. The default app kit
is the stock Kubuntu set **minus** a deliberate bloat-strip. This document
records what the strip removes, what it keeps, and *why*, so the decision isn't
silently second-guessed (or silently broadened) in a future build.

The strip is implemented in two places, both run during `lb chroot`:

| File | Role |
|---|---|
| `foundry-iso/config/package-lists/strip.list.chroot.purge` | bulk package removal (live-build purges these) |
| `foundry-iso/config/hooks/0020-strip-kubuntu-bloat.hook.chroot` | explicit `apt purge` + `autoremove` pass for glob groups and conditionals |

## What we deliberately OMIT

| Category | Packages | Rationale |
|---|---|---|
| Office suite | `libreoffice-*` | Not a content-authoring need for game dev; ~hundreds of MB. |
| KDE PIM | `kontact`, `kmail`, `akonadi-*`, `kdepim-*`, `kaddressbook`, `kalarm`, `kjots`, `knotes`, `korganizer`, `merkuro-calendar` | Mail/calendar/contacts stack is irrelevant to the workstation use case and pulls the heavy Akonadi server. |
| Games | `kde-games-*`, `kalgebra`, `kgeography`, `kig`, `kbruch` | Bundled KDE games + edu apps; off-message for a *build* distro. |
| Snap | `snapd`, `snap-store` | Packaging policy is `.deb`-only (no snap/flatpak) — see distro proposal. |
| First-run welcome | `plasma-welcome` | Replaced by `foundry-welcome`. Recommends-only on both kubuntu-desktop and kubuntu-settings-desktop, so the purge is clean. |
| Photo management (anvil only) | `digikam`, `showfoto` | Stripped from **anvil** only; **kept** in `sprite`/`atelier` editions where image/asset work is in scope. |
| Language nag | `language-selector` autostart | Suppressed (not removed): live session has no guaranteed network and the audience doesn't need it. |

Also removed from the live skeleton: Kubuntu-specific desktop shortcuts
(`org.kfocus.web.howtos.desktop`, `org.kubuntu.web.home.desktop`).

## What we deliberately SHIP (and guard)

The strip is **conservative** — it touches end-user *applications*, never the
Plasma config infrastructure. The audit of 2026-05-30 verified the following are
present in the live chroot and must stay that way:

- `qml6-module-org-kde-kquickcontrols`
- `qml6-module-qtquick-dialogs`
- the `org.kde.kcmutils` / `org.kde.kquickcontrols` / `QtQuick.Dialogs` QML modules

These are the QML modules that **plasmoid and KCM config dialogs load at
runtime**. Stripping any one of them ships a distro whose config UIs come up
blank. They are now protected by:

1. A build-time assertion in `foundry-iso/scripts/build-iso.sh` (after `lb chroot`)
   that fails the build if any of the three module directories is missing from
   the chroot.
2. "Never strip" comments in both strip files pointing back here.

The core Plasma developer kit stays in place: Dolphin (file manager), Konsole
(terminal), Kate (editor), Spectacle (screenshots), Ark, Gwenview, System
Settings, and the full Plasma shell.

## Why this matters (the trigger)

This investigation was prompted while testing the `claude-usage` Plasma 6
plasmoid live on `foundry-anvil-0.9.30`: its config dialog came up empty. The
question was whether Foundry's strip had removed a config module the plasmoid
needed. **It had not** — the empty dialog was a Plasma-5-idiom bug in
claude-usage itself (fixed there). The strip was already conservative and safe.

The remaining work was therefore not a fix but a **guard**: make the
"full KDE config stack present" property explicit and tested, so a future
over-broad strip-list glob (e.g. a `kde-*` auto-remove) can't silently
reintroduce the failure. See plan
[`docs/plans/2026-05-30-full-kde-experience.md`](../plans/2026-05-30-full-kde-experience.md).

## Decision

The current omit list is **appropriate and stays as-is**. No KDE apps are added
back. The editioned exception for digikam/showfoto (anvil-strip, sprite/atelier-keep)
is intentional. Revisit only if a concrete user need surfaces.
