import QtQuick

Rectangle {
    id: root
    color: "#0a0a0a"

    property int stage

    onStageChanged: {
        if (stage === 1) logoAnim.running = true
    }

    // Background wallpaper. MUST be a survives-install path (foundry-kde-theme),
    // NOT the old /usr/share/backgrounds/foundry-linux-wallpaper.png which is
    // purged with calamares-settings on install (was a dead path → blank splash).
    Image {
        anchors.fill: parent
        source: "file:///usr/share/wallpapers/FoundryLinux-ForgeHorizon/contents/images/1920x1080.png"
        fillMode: Image.PreserveAspectCrop
        asynchronous: true
        cache: false
    }

    // Dark scrim
    Rectangle {
        anchors.fill: parent
        color: "#990a0a0a"
    }

    // Logo — the anvil avatar shipped by foundry-kde-theme (survives install).
    // The old /usr/share/plymouth/themes/foundry-linux/logo.png was ALSO purged
    // with calamares-settings on install (the second dead path).
    Image {
        id: logo
        anchors.centerIn: parent
        anchors.verticalCenterOffset: -48
        width: 160
        height: 160
        source: "file:///usr/share/foundry-linux/avatar.png"
        fillMode: Image.PreserveAspectFit
        opacity: 0

        NumberAnimation on opacity {
            id: logoAnim
            running: false
            from: 0; to: 1
            duration: 400
            easing.type: Easing.OutCubic
        }
    }

    // Wordmark
    Text {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: logo.bottom
        anchors.topMargin: 20
        text: "FOUNDRY LINUX"
        color: "#f7f7f7"
        font.pixelSize: 18
        font.weight: Font.Bold
        font.letterSpacing: 3
        opacity: logo.opacity
    }

    // Stage dots
    Row {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 80
        spacing: 10

        Repeater {
            model: 5
            Rectangle {
                width: 6; height: 6; radius: 3
                color: index < root.stage ? "#ff5b1a" : "#2a2a2a"
                Behavior on color { ColorAnimation { duration: 200 } }
            }
        }
    }
}
