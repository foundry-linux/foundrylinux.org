#!/bin/sh
# macOS-style natural scrolling for the touchpad (foundry-kde-theme).
# KDE stores scroll direction per input device (keyed by vendor/product/name in
# ~/.config/kcminputrc), so it can't be shipped as a static /etc/xdg default.
# Instead we flip it once at first login via KWin's D-Bus input interface, which
# both applies the change live and persists it. Sentinel keeps a later user
# override: touchpad only, mice are left on the traditional direction.
SENTINEL="${XDG_CONFIG_HOME:-$HOME/.config}/foundry-natural-scroll-set"
[ -f "$SENTINEL" ] && exit 0

# The Qt6 qdbus binary is named differently across builds; pick what exists.
QDBUS=
for c in qdbus6 qdbus-qt6 qdbus; do
    if command -v "$c" >/dev/null 2>&1; then QDBUS="$c"; break; fi
done
[ -n "$QDBUS" ] || exit 0

KWIN=org.kde.KWin
MGR=/org/kde/KWin/InputDevice

# Retry: KWin's D-Bus service may not be up yet at autostart phase 2.
i=0
while [ $i -lt 60 ]; do
    devs=$("$QDBUS" "$KWIN" "$MGR" devicesSysNames 2>/dev/null | tr ',\n' '  ')
    if [ -n "$devs" ]; then
        for d in $devs; do
            path="$MGR/$d"
            [ "$("$QDBUS" "$KWIN" "$path" touchpad 2>/dev/null)" = "true" ] || continue
            "$QDBUS" "$KWIN" "$path" naturalScroll true >/dev/null 2>&1
        done
        # KWin answered => devices enumerated; record we ran (even on a
        # desktop with no touchpad) so we don't retry for 60s every login.
        touch "$SENTINEL"
        exit 0
    fi
    sleep 1
    i=$((i + 1))
done
exit 0
