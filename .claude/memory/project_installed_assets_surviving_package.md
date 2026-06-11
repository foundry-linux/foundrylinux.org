---
name: project-installed-assets-surviving-package
description: Any asset the INSTALLED Foundry system renders must ship from a package that survives the Calamares install
metadata: 
  node_type: memory
  type: project
  originSessionId: 786f77e8-30e0-47c9-9b9f-7917fb351bd8
---

Calamares's `modules/packages.conf` runs `remove: calamares` on the installed target. `calamares-settings-foundry-linux` `Depends: calamares`, so apt drops it (and everything it ships) from the installed system. **Anything the installed desktop/greeter renders must therefore come from a package that survives** — use `foundry-kde-theme` (part of foundry-anvil, not removed), never `calamares-settings-*`.

**Bugs this caused (both 2026-06):**
- Desktop wallpaper reverted to Kubuntu "cones": the wallpaper path pointed at `/usr/share/sddm/themes/foundry-linux/background.png` (calamares-settings → purged) → dead path → fallback. Fixed by pointing at `/usr/share/wallpapers/FoundryLinux-ForgeHorizon/` (foundry-kde-theme).
- SDDM **login screen** cones: the greeter theme shipped from calamares-settings → purged → `[Theme] Current=foundry-linux` pointed at nothing → Kubuntu breeze greeter. Fixed by moving the SDDM theme to foundry-kde-theme.

**How to apply:** When wiring any installed-system asset (wallpaper, SDDM greeter, lock screen, color scheme, icons), confirm the package shipping it is NOT `calamares-settings-*`. calamares-settings is installer-only (branding, slideshow, Calamares module configs) — correct to purge. Diagnose installed-vs-live divergence with `foundry-iso/scripts/inspect-installed-disk.sh`. See [[iso-bump-greedy-commit]].
