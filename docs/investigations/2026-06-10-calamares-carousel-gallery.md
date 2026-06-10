# Calamares installer carousel — slide gallery

**Date**: 2026-06-10
**Package**: `calamares-settings-foundry-linux`
**Source**: `foundry-apt/packages/calamares-settings-foundry-linux/data/branding/foundry-linux/`

The carousel is the slideshow shown in the Calamares installer's right-hand pane *while packages
are unpacking*. It cycles **4 slides, 20 s each**. Each slide is a **self-contained 800×440 PNG**
with its heading and body typeset into the artwork — the QML only cross-fades between them (no
separate text overlay; see the [take-6 clipping fix](#why-the-text-is-baked-in-take-6)).

> Images below are copies captured 2026-06-10. The live files are the `slide-0*.png` in the
> source dir above; regenerate these copies if the slides change.

---

## The four slides

### 1 · The Forge — `slide-01-forge.png`

<img src="screenshots/carousel-slide-01-forge.png" width="760">

> **The Forge** — WorldFoundry GDK, retro toolkit, Blender add-ons, emulators — struck in from
> first boot.

### 2 · Stay current. — `slide-02-apt.png`

<img src="screenshots/carousel-slide-02-apt.png" width="760">

> **Stay current.** — apt.foundrylinux.org keeps every Foundry tool signed, rebuilt, and
> re-tested on every push.

### 3 · Distrobox-ready. — `slide-03-devbox.png`

<img src="screenshots/carousel-slide-03-devbox.png" width="760">

> **Distrobox-ready.** — ghcr.io/foundry-linux/devbox:26.04 — the same environment, containerised.

### 4 · Where to go next. — `slide-04-docs.png`

<img src="screenshots/carousel-slide-04-docs.png" width="760">

> **Where to go next.** — foundrylinux.org/docs — package catalogue, install guide, and
> contribution notes.

---

## `slideshow.qml`

The QML driver. A `Timer` advances a non-interactive horizontal `ListView`; each delegate is the
slide PNG shown with `Image.PreserveAspectFit`.

```qml
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
                fillMode:     Image.PreserveAspectFit
                asynchronous: true
            }
        }
    }
}
```

---

## Why the text is baked in (take-6)

The model is `src`-only — no `title`/`body` strings — because the heading and body are part of
each PNG, not rendered by QML. Earlier revisions overlaid a QML `Text` caption *and* baked the
text into the PNG, and the visible left-edge clipping ("Where"→"Vhere") came from the **PNG**
being cropped by `fillMode: Image.PreserveAspectCrop` (the installer pane is taller than the
slides' 1.818:1, so Crop scaled-to-fill and ate ~77 px off each side). Five "text-margin" patches
chased the wrong element (the QML caption) before take-6 switched to `PreserveAspectFit` and
deleted the redundant caption. Full write-up: the carousel rows in
[`foundry-iso/docs/howto-kubuntu-remix.md`](../../foundry-iso/docs/howto-kubuntu-remix.md) and the
[installer-polish plan](../plans/2026-06-09-foundry-welcome-and-installer-polish.md).

---

## Related branding files (not carousel, same dir)

| File | Role |
|------|------|
| `banner.png` (800×??) | Calamares sidebar banner |
| `logo.png` (256×256) | Installer window logo / branding mark (the anvil) |
| `branding.desc` | Calamares branding descriptor (strings, colors, slide list) |

Slide dimensions: **800×440** (aspect 1.818:1). Keep new slides at that size and the same
near-black background so the `PreserveAspectFit` letterbox stays invisible.
