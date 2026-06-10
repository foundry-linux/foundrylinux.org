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

## The other branding files (same dir)

### `logo.png` — 256×256

<img src="screenshots/branding-logo.png" width="256">

Used as both `productLogo` (installer window header / welcome page) and `productIcon`. The Foundry
anvil mark on a circular near-black field with orange sparks. **Same image as the user avatar**
(`foundry-kde-theme`'s `/usr/share/foundry-linux/avatar.png`) — one mark across the installer
logo, the app-menu/installer icon, and the login/logout avatar.

### `banner.png` — 800×200 (4:1)

<img src="screenshots/branding-banner.png" width="760">

Used as `productWelcome` — the header strip on the installer's Welcome page. "FOUNDRY LINUX" in
bold with the orange rule and "26.04 LTS · ANVIL" beneath.

### `branding.desc` — the Calamares branding descriptor

The YAML that ties it all together: window geometry, product strings/URLs, which images to use,
the sidebar palette, and the slideshow entrypoint.

```yaml
---
componentName: foundry-linux

welcomeStyleCalamares: false
welcomeExpandingLogo:  true

windowExpanding:    normal
windowSize:         1100px,680px
windowPlacement:    center

sidebar:    widget
navigation: widget

strings:
    productName:      "Foundry Linux"
    shortProductName: "Foundry"
    version:          "26.04"
    shortVersion:     "26.04"
    versionedName:    "Foundry Linux 26.04"
    shortVersionedName: "Foundry 26.04"
    bootloaderEntryName: "Foundry Linux"
    productUrl:       "https://foundrylinux.org/"
    supportUrl:       "https://foundrylinux.org/docs"
    knownIssuesUrl:   "https://github.com/foundry-linux/foundrylinux.org/issues"
    releaseNotesUrl:  "https://foundrylinux.org/"

images:
    productLogo:    "logo.png"
    productIcon:    "logo.png"
    productWelcome: "banner.png"

style:
    SidebarBackground:        "#0a0a0a"
    SidebarText:              "#f7f7f7"
    SidebarTextCurrent:       "#0a0a0a"
    SidebarBackgroundCurrent: "#ff5b1a"

slideshow:    "slideshow.qml"
slideshowAPI: 2
```

Field guide:

| Key | Effect |
|-----|--------|
| `componentName: foundry-linux` | Branding dir name — must match `/usr/share/calamares/branding/foundry-linux/` |
| `windowSize: 1100px,680px` | Installer window size (the carousel pane scales with it — see the `PreserveAspectFit` note) |
| `strings:` | Product name, version, and the URLs shown on Welcome/Finish pages |
| `images:` | `productLogo`/`productIcon` → `logo.png` (anvil); `productWelcome` → `banner.png` |
| `style:` | Sidebar palette — Foundry near-black `#0a0a0a` + orange accent `#ff5b1a` (the current step's highlight) |
| `slideshow` / `slideshowAPI: 2` | The carousel QML and its API version (2 = the `Presentation`-less `ListView` form used here) |

---

## Conventions

Slide dimensions: **800×440** (aspect 1.818:1). Keep new slides at that size and the same
near-black background so the `PreserveAspectFit` letterbox stays invisible. `logo.png` is square
(256×256); `banner.png` is the 4:1 (800×200) welcome strip. Orange accent throughout is
`#ff5b1a`; backgrounds are `#0a0a0a`–`#0e0e0e`.
