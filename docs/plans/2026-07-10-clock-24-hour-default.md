# Default the Foundry Linux desktop clock to a 24-hour clock

**Date:** 2026-07-10
**Status:** Implemented (working tree — pending build/live verification)

## Context

Foundry Linux's KDE digital clock had **no** 12h/24h format set anywhere, so it
fell through to "follow locale". Since no `LC_TIME`/`LANG` is configured anywhere
in the repo either, the base system locale (typically `en_US`) won and the panel
clock showed a 12-hour time. We want a 24-hour clock to be the shipped default.

The clock is provisioned declaratively at first login by the Foundry
Look-and-Feel desktop-layout script, so the fix is a single `writeConfig` line
where the clock applet is already configured — no autostart/D-Bus dance needed
(unlike the sibling touchpad natural-scroll change, which had to be per-device).

## Approach

The digital clock is built in the first-login desktop-layout script alongside its
`showDate` config. The Plasma digital clock exposes `use24hFormat` in its
`[Appearance]` config group:

- `0` → 12-hour
- `1` → 24-hour (what we want)
- `2` → follow locale (the applet default, which is why nothing was set)

Setting `use24hFormat = 1` forces the 24-hour clock regardless of locale.

## Changes

**`foundry-apt/packages/foundry-kde-theme/data/look-and-feel/org.foundrylinux.foundry-linux/contents/layouts/org.kde.plasma.desktop-layout.js`**

Add one line in the existing clock block:

```js
const clock = panel.addWidget("org.kde.plasma.digitalclock");
clock.currentConfigGroup = ["Appearance"];
clock.writeConfig("showDate", "true");
clock.writeConfig("use24hFormat", 1);   // 0=12h, 1=24h, 2=follow locale
```

**`foundry-apt/packages/foundry-kde-theme/debian/changelog`** — bump `1.0.6` →
`1.0.7` (dep-unaffecting behavior tweak → patch bump) with a new top stanza
describing the change.

## Verification

1. `cd foundry-apt && task build` — confirm the `foundry-kde-theme` `.deb` builds
   and the `1.0.7` changelog version is picked up.
2. Confirm the layout script ships to
   `usr/share/plasma/look-and-feel/org.foundrylinux.foundry-linux/contents/layouts/`:
   `dpkg-deb -c dist/foundry-kde-theme_1.0.7_*.deb | grep layout.js`.
3. Real desktop check (needs a KDE VM/ISO): on a **fresh user's first login** the
   layout script runs once — confirm the panel clock reads 24-hour (e.g. 14:30,
   not 2:30 PM). An already-provisioned user won't retroactively change (expected;
   same as every other layout default).

## Notes / out of scope

- Locale-wide `LC_TIME` in `/etc/default/locale` was considered and rejected:
  broader blast radius (affects every app's date/time formatting), whereas the
  ask was the clock. The applet-level `use24hFormat` is the targeted lever.
- No ISO-side change needed; the ISO installs `foundry-kde-theme`, which carries
  this.
