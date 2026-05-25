import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import "./pages"

Kirigami.ApplicationWindow {
    id: root

    title: "Welcome to Foundry Linux"
    width: 680
    height: 500
    minimumWidth: 480
    minimumHeight: 380

    property int currentPage: 0
    readonly property int pageCount: 3

    // Qt.quit() → engine.quit() → app.quit() → sentinel written in main.cpp
    onClosing: function(close) {
        close.accepted = true
        Qt.quit()
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // Page area
        StackLayout {
            id: pageStack
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: root.currentPage

            IntroPage {}
            ToolsPage {}
            CommunityPage {}
        }

        // Nav bar
        Rectangle {
            Layout.fillWidth: true
            height: navRow.implicitHeight + Kirigami.Units.largeSpacing * 2
            color: Kirigami.Theme.backgroundColor

            Kirigami.Separator { anchors { top: parent.top; left: parent.left; right: parent.right } }

            RowLayout {
                id: navRow
                anchors {
                    left: parent.left
                    right: parent.right
                    verticalCenter: parent.verticalCenter
                    margins: Kirigami.Units.largeSpacing
                }
                spacing: Kirigami.Units.smallSpacing

                QQC2.Button {
                    text: "Skip"
                    flat: true
                    visible: root.currentPage < root.pageCount - 1
                    onClicked: Qt.quit()
                }

                Item { Layout.fillWidth: true }

                // Page indicator dots
                Row {
                    spacing: Kirigami.Units.smallSpacing
                    Layout.alignment: Qt.AlignVCenter
                    Repeater {
                        model: root.pageCount
                        Rectangle {
                            width: 8; height: 8; radius: 4
                            color: index === root.currentPage
                                   ? Kirigami.Theme.highlightColor
                                   : Kirigami.Theme.disabledTextColor
                            Behavior on color { ColorAnimation { duration: 150 } }
                        }
                    }
                }

                Item { Layout.fillWidth: true }

                QQC2.Button {
                    text: root.currentPage === root.pageCount - 1 ? "Done" : "Next"
                    highlighted: true
                    onClicked: {
                        if (root.currentPage < root.pageCount - 1) {
                            root.currentPage++
                        } else {
                            Qt.quit()
                        }
                    }
                }
            }
        }
    }
}
