import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Item {
    ColumnLayout {
        anchors.centerIn: parent
        width: Math.min(parent.width - Kirigami.Units.gridUnit * 6, 480)
        spacing: Kirigami.Units.largeSpacing

        Kirigami.Heading {
            Layout.fillWidth: true
            text: "Community & Support"
            level: 2
            horizontalAlignment: Text.AlignHCenter
        }

        QQC2.Label {
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignHCenter
            text: "Foundry Linux is open-source and community-driven.\n" +
                  "Here's where to find help and get involved:"
        }

        // Link cards
        Repeater {
            model: [
                { icon: "globe", label: "Website & Docs",
                  url: "https://foundrylinux.org" },
                { icon: "system-help", label: "File a Bug or Request a Feature",
                  url: "https://github.com/foundry-linux/foundrylinux.org/issues" },
                { icon: "internet-services", label: "Community Chat (Discord / Matrix)",
                  url: "https://foundrylinux.org/community" },
            ]

            QQC2.ItemDelegate {
                required property var modelData
                Layout.fillWidth: true
                icon.name: modelData.icon
                text: modelData.label + "\n" + modelData.url
                onClicked: Qt.openUrlExternally(modelData.url)
            }
        }

        QQC2.Label {
            Layout.fillWidth: true
            Layout.topMargin: Kirigami.Units.smallSpacing
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignHCenter
            opacity: 0.7
            text: "You can reopen this window any time from the application menu."
        }
    }
}
