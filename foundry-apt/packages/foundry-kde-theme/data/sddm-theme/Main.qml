// Foundry Linux SDDM login theme
// Requires SDDM 0.20+ (KDE Plasma 6)

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root
    color: "#0a0a0a"

    // ── wallpaper ──────────────────────────────────────────────────────
    Image {
        id: bg
        anchors.fill: parent
        source: Qt.resolvedUrl("background.png")
        fillMode: Image.PreserveAspectCrop
        asynchronous: true
        cache: false
    }

    // Scrim so the panel pops
    Rectangle {
        anchors.fill: parent
        color: "#800a0a0a"
    }

    // ── login panel ────────────────────────────────────────────────────
    Rectangle {
        id: panel
        anchors.centerIn: parent
        width:  380
        height: 280
        radius: 4
        color:  "#e00e0e0e"
        border.color: "#ff5b1a"
        border.width: 1

        ColumnLayout {
            anchors { fill: parent; margins: 32 }
            spacing: 16

            // Wordmark
            Text {
                Layout.fillWidth: true
                text:  "FOUNDRY LINUX"
                color: "#f7f7f7"
                font { pixelSize: 22; weight: Font.Bold; letterSpacing: 2 }
                horizontalAlignment: Text.AlignHCenter
            }

            Text {
                Layout.fillWidth: true
                text:  "26.04 LTS"
                color: "#ff5b1a"
                font { pixelSize: 12 }
                horizontalAlignment: Text.AlignHCenter
                bottomPadding: 8
            }

            // Username
            TextField {
                id: userInput
                Layout.fillWidth: true
                placeholderText: "Username"
                text: userModel.lastUser
                font.pixelSize: 14
                background: Rectangle { color: "#1a1a1a"; radius: 2 }
                color: "#ebe8e2"
                KeyNavigation.tab: passInput
                Keys.onReturnPressed: passInput.forceActiveFocus()
            }

            // Password
            TextField {
                id: passInput
                Layout.fillWidth: true
                placeholderText: "Password"
                echoMode: TextInput.Password
                font.pixelSize: 14
                background: Rectangle { color: "#1a1a1a"; radius: 2 }
                color: "#ebe8e2"
                Keys.onReturnPressed: sddm.login(userInput.text, passInput.text, 0)
            }

            // Login button
            Button {
                Layout.fillWidth: true
                text: "Sign in"
                font { pixelSize: 14; weight: Font.Medium }
                contentItem: Text {
                    text: parent.text
                    color: "#0a0a0a"
                    font: parent.font
                    horizontalAlignment: Text.AlignHCenter
                }
                background: Rectangle {
                    color: parent.pressed ? "#cc4a14" : "#ff5b1a"
                    radius: 2
                }
                onClicked: sddm.login(userInput.text, passInput.text, 0)
            }
        }
    }

    Component.onCompleted: userInput.forceActiveFocus()
}
