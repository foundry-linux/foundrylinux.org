#!/bin/sh
# ForgeHorizon wallpaper (foundry-kde-theme) — survives the Calamares install
# that purges calamares-settings. Sentinel keeps user wallpaper changes.
WALLPAPER=/usr/share/wallpapers/FoundryLinux-ForgeHorizon/
SENTINEL="${XDG_CONFIG_HOME:-$HOME/.config}/foundry-wallpaper-set"
[ -f "$SENTINEL" ] && exit 0
i=0
while [ $i -lt 60 ]; do
    plasma-apply-wallpaperimage "$WALLPAPER" >/dev/null 2>&1 && break
    sleep 1
    i=$((i+1))
done
touch "$SENTINEL"
