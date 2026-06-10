/* Foundry Linux default desktop layout.
 *
 * Runs once on a fresh user's first login (via the
 * org.foundrylinux.foundry-linux Look-and-Feel package). Builds the panel with
 * the Foundry pinned launchers (Kate out, Blender in) and sets the ForgeHorizon
 * wallpaper. Uses the standard Plasma desktop-scripting globals: new Panel,
 * panel.addWidget, gridUnit, desktops().
 */

const panel = new Panel;
panel.location = "bottom";
panel.height = Math.round(gridUnit * 3);

// Application launcher (Kickoff).
const kickoff = panel.addWidget("org.kde.plasma.kickoff");
kickoff.currentConfigGroup = ["General"];
kickoff.writeConfig("icon", "start-here-kde");

// Task manager with the Foundry pinned launchers.
const tasks = panel.addWidget("org.kde.plasma.icontasks");
tasks.currentConfigGroup = ["General"];
tasks.writeConfig("launchers", [
    "applications:systemsettings.desktop",
    "applications:org.kde.discover.desktop",
    "applications:org.kde.dolphin.desktop",
    "applications:org.kde.konsole.desktop",
    "applications:blender.desktop",
    "preferred://browser"
]);

// Push the tray + clock to the right edge.
panel.addWidget("org.kde.plasma.marginsseparator");
panel.addWidget("org.kde.plasma.systemtray");

const clock = panel.addWidget("org.kde.plasma.digitalclock");
clock.currentConfigGroup = ["Appearance"];
clock.writeConfig("showDate", "true");

panel.addWidget("org.kde.plasma.showdesktop");

// ForgeHorizon wallpaper on every desktop containment (belt-and-suspenders with
// the per-user autostart, which is the load-bearing lever).
const allDesktops = desktops();
for (let i = 0; i < allDesktops.length; i++) {
    const d = allDesktops[i];
    d.wallpaperPlugin = "org.kde.image";
    d.currentConfigGroup = ["Wallpaper", "org.kde.image", "General"];
    d.writeConfig("Image", "file:///usr/share/wallpapers/FoundryLinux-ForgeHorizon/");
}
