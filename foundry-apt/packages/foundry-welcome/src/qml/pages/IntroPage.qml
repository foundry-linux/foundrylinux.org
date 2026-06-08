import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Item {
    ColumnLayout {
        anchors.centerIn: parent
        width: Math.min(parent.width - Kirigami.Units.gridUnit * 6, 480)
        spacing: Kirigami.Units.largeSpacing

        Image {
            Layout.alignment: Qt.AlignHCenter
            source: "file:///usr/share/calamares/branding/foundry-linux/logo.png"
            sourceSize.height: 88
            fillMode: Image.PreserveAspectFit
        }

        Kirigami.Heading {
            Layout.alignment: Qt.AlignHCenter
            Layout.fillWidth: true
            text: "Welcome to Foundry Linux"
            level: 1
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
        }

        QQC2.Label {
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignHCenter
            text: "The open-source game development workstation.\n" +
                  "Everything you need to build games — retro to modern — ready to go."
        }

        Text {
            Layout.alignment: Qt.AlignHCenter
            Layout.topMargin: Kirigami.Units.smallSpacing
            text: "foundrylinux.org"
            color: Kirigami.Theme.linkColor
            font.underline: true
            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: Qt.openUrlExternally("https://foundrylinux.org")
            }
        }
    }
}
