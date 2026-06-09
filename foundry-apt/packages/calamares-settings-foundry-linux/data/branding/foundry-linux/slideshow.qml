/* Foundry Linux installer slideshow — shown while packages install.
   4 slides, 20 s each.  Each slide is a self-contained 800×440 PNG with its
   heading + body already typeset into the artwork; QML only cross-fades
   between them. */
import QtQuick 2.15

Item {
    id: root
    anchors.fill: parent

    // Dark backdrop so the PreserveAspectFit letterbox blends with the slides'
    // near-black background.  The slides are 1.818:1; the installer pane is
    // usually taller than that, so Fit leaves a thin bar top and bottom.
    Rectangle { anchors.fill: parent; color: "#0e0e0e" }

    Timer {
        id: timer
        interval: 20000
        running:  true
        repeat:   true
        onTriggered: view.currentIndex = (view.currentIndex + 1) % view.count
    }

    ListView {
        id:           view
        anchors.fill: parent
        orientation:  ListView.Horizontal
        snapMode:     ListView.SnapOneItem
        interactive:  false
        clip:         true

        model: ListModel {
            ListElement { src: "slide-01-forge.png" }
            ListElement { src: "slide-02-apt.png" }
            ListElement { src: "slide-03-devbox.png" }
            ListElement { src: "slide-04-docs.png" }
        }

        delegate: Item {
            width:  view.width
            height: view.height

            Image {
                anchors.fill: parent
                source:       model.src

                // PreserveAspectFit, NOT Crop.  The slide art bakes its heading
                // in ~32 px from the left edge of the 800-px canvas.  With
                // PreserveAspectCrop the art was scaled to FILL the taller
                // installer pane, which overflowed the width and cropped ~77 px
                // off each side — clipping the first glyph of every heading
                // ("Where"→"Vhere", "foundry"→"oundry").  This was the real
                // cause behind five failed "text margin" patches to a QML
                // caption that has now been removed: the clipped text was never
                // in the QML, it was in the PNG.  Fit shows the whole slide; the
                // dark backdrop above hides the top/bottom letterbox.
                fillMode:     Image.PreserveAspectFit
                asynchronous: true
            }
        }
    }
}
