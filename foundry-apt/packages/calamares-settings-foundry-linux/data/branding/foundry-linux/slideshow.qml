/* Foundry Linux installer slideshow — shown while packages install.
   4 slides, 20 s each. Slide images live alongside this file. */
import QtQuick 2.15

Item {
    id: root
    width:  800
    height: 440

    Timer {
        id: timer
        interval: 20000
        running:  true
        repeat:   true
        onTriggered: {
            var next = (view.currentIndex + 1) % view.count
            view.currentIndex = next
        }
    }

    ListView {
        id:           view
        anchors.fill: parent
        orientation:  ListView.Horizontal
        snapMode:     ListView.SnapOneItem
        interactive:  false
        clip:         true

        model: ListModel {
            ListElement { src: "slide-01-forge.png"; title: "The Forge";             body: "WorldFoundry GDK, retro toolkit, Blender add-ons, emulators — struck in from first boot." }
            ListElement { src: "slide-02-apt.png";   title: "Stay current.";         body: "apt.foundrylinux.org keeps every Foundry tool signed, rebuilt, and re-tested on every push." }
            ListElement { src: "slide-03-devbox.png";title: "Distrobox-ready.";      body: "ghcr.io/foundry-linux/devbox:26.04 — the same environment, containerised." }
            ListElement { src: "slide-04-docs.png";  title: "Where to go next.";     body: "foundrylinux.org/docs — package catalogue, install guide, and contribution notes." }
        }

        delegate: Item {
            width:  view.width
            height: view.height

            Image {
                anchors.fill: parent
                source:       model.src
                fillMode:     Image.PreserveAspectCrop
                asynchronous: true
            }

            Rectangle {
                anchors { left: parent.left; right: parent.right; bottom: parent.bottom }
                height:  90
                color:   "#cc0e0e0e"

                Column {
                    anchors { left: parent.left; right: parent.right; verticalCenter: parent.verticalCenter }
                    leftPadding:  24
                    rightPadding: 24
                    spacing:       4

                    Text {
                        text:  model.title
                        color: "#e87a00"
                        font { pixelSize: 18; weight: Font.Bold; family: "sans-serif" }
                    }
                    Text {
                        text:       model.body
                        color:      "#e0e0e0"
                        wrapMode:   Text.WordWrap
                        width:      parent.width - 48
                        font { pixelSize: 13; family: "sans-serif" }
                    }
                }
            }
        }
    }
}
