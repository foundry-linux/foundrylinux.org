import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Item {
    Flickable {
        anchors.fill: parent
        anchors.margins: Kirigami.Units.largeSpacing
        contentHeight: col.implicitHeight
        clip: true

        ColumnLayout {
            id: col
            width: parent.width
            spacing: Kirigami.Units.largeSpacing

            Kirigami.Heading {
                Layout.fillWidth: true
                text: "What's Installed"
                level: 2
            }

            QQC2.Label {
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                text: "Foundry Linux comes pre-loaded with a full indie game dev stack:"
            }

            ToolCategory {
                title: "3D & Art"
                tools: ["Blender 4 — 3D modelling, animation, rendering",
                        "WorldFoundry Blender add-ons — level editor, asset exporter",
                        "Blender Asset Finder — browse community asset libraries"]
            }

            ToolCategory {
                title: "Retro Game Dev"
                tools: ["MAME — arcade hardware emulator + dev target",
                        "cc65 — 6502/65C02/65816 cross-compiler suite",
                        "DASM — 6502/6800 macro assembler",
                        "f9dasm — 6800/6809 disassembler",
                        "Ghidra — NSA reverse-engineering suite",
                        "radare2 — hex editor + binary analysis framework",
                        "vgmstream — VGM audio playback library + CLI",
                        "libvgm — VGM music playback engine"]
            }

            ToolCategory {
                title: "Emulators"
                tools: ["PPSSPP — PlayStation Portable emulator",
                        "SNES9x — Super Nintendo emulator"]
            }

            ToolCategory {
                title: "Dev Tooling"
                tools: ["Task — modern Makefile replacement (go-task)",
                        "WorldFoundry CLI tools — cdpack, iffcomp, lvldump, and more"]
            }
        }
    }

    component ToolCategory: ColumnLayout {
        id: cat
        required property string title
        required property var tools

        Layout.fillWidth: true
        spacing: Kirigami.Units.smallSpacing

        Kirigami.Heading {
            Layout.fillWidth: true
            text: cat.title
            level: 4
            color: Kirigami.Theme.highlightColor
        }

        Repeater {
            model: cat.tools
            QQC2.Label {
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                text: "• " + modelData
            }
        }
    }
}
