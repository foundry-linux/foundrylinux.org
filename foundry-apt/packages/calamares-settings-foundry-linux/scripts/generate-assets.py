#!/usr/bin/env python3
"""
Generate all Foundry Linux branding PNG assets for calamares-settings-foundry-linux.
Run from the package root:
    python3 scripts/generate-assets.py

Requires: Pillow, pycairo (both present on the dev host).
Falls back to DejaVu system fonts if Google Fonts download fails.
"""

import math
import os
import re
import shutil
import sys
import urllib.request
from pathlib import Path

import cairo
from PIL import Image, ImageDraw, ImageEnhance, ImageFilter, ImageFont

# ── paths ────────────────────────────────────────────────────────────────────

ROOT = Path(__file__).parent.parent
BRANDING   = ROOT / "data/branding/foundry-linux"
CONFIG     = ROOT / "data/config"
GRUB       = CONFIG / "grub"
PLYMOUTH   = CONFIG / "plymouth"
SDDM       = CONFIG / "sddm"
WALLPAPER  = CONFIG / "foundry-linux-wallpaper.png"

for d in (BRANDING, GRUB, PLYMOUTH, SDDM):
    d.mkdir(parents=True, exist_ok=True)

# ── palette ──────────────────────────────────────────────────────────────────

BG         = (0x0a, 0x0a, 0x0a)          # near-black
ACCENT     = (0xff, 0x5b, 0x1a)          # ember orange #ff5b1a
SILVER_HI  = (0xf7, 0xf7, 0xf7)          # text / logo fill
TEXT       = (0xeb, 0xe8, 0xe2)          # warm off-white body text
DARK_PANEL = (0x0e, 0x0e, 0x0e, 0xd0)   # semi-transparent panel (RGBA)


def hex2rgb(h):
    h = h.lstrip("#")
    return tuple(int(h[i:i+2], 16) for i in (0, 2, 4))


def rgb_f(rgb):
    """Return (r, g, b) as 0..1 floats for pycairo."""
    return rgb[0]/255, rgb[1]/255, rgb[2]/255


# ── fonts ────────────────────────────────────────────────────────────────────

FONT_CACHE = ROOT / "scripts/.font-cache"
FONT_CACHE.mkdir(exist_ok=True)

FONTS = {
    "wordmark": FONT_CACHE / "BigShouldersDisplay-Black.ttf",
    "body":     FONT_CACHE / "SpaceGrotesk-Medium.ttf",
    "bold":     FONT_CACHE / "SpaceGrotesk-Bold.ttf",
}

GOOGLE_FONTS_CSS_URLS = {
    "wordmark": "https://fonts.googleapis.com/css2?family=Big+Shoulders+Display:wght@900",
    "body":     "https://fonts.googleapis.com/css2?family=Space+Grotesk:wght@500",
    "bold":     "https://fonts.googleapis.com/css2?family=Space+Grotesk:wght@700",
}

SYSTEM_FALLBACKS = {
    "wordmark": "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf",
    "body":     "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
    "bold":     "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf",
}


def fetch_font(key):
    dest = FONTS[key]
    if dest.exists():
        return str(dest)
    try:
        req = urllib.request.Request(
            GOOGLE_FONTS_CSS_URLS[key],
            headers={"User-Agent": "Mozilla/5.0 (X11; Linux x86_64; rv:109.0)"},
        )
        css = urllib.request.urlopen(req, timeout=8).read().decode()
        # Prefer TTF; Google CSS v1 returns TTF for generic user-agents.
        url = re.search(r"src:\s*url\(([^)]+\.ttf)\)", css)
        if not url:
            # CSS v2 format: src: url(...) format('truetype')
            url = re.search(r"url\((https://[^)]+)\)\s*format\(['\"]truetype['\"]\)", css)
        if url:
            data = urllib.request.urlopen(url.group(1), timeout=15).read()
            dest.write_bytes(data)
            print(f"  ✓ Downloaded {key} font")
            return str(dest)
    except Exception as e:
        print(f"  ⚠ Font download failed for {key}: {e} — using system fallback")
    fb = SYSTEM_FALLBACKS[key]
    if Path(fb).exists():
        return fb
    return None  # Pillow will use its internal bitmap font


def font(key, size):
    path = fetch_font(key)
    if path:
        try:
            return ImageFont.truetype(path, size)
        except Exception:
            pass
    return ImageFont.load_default()


# ── LOGO ─────────────────────────────────────────────────────────────────────
# FoundryMark (32×32 viewbox) scaled to 180×180 centred on a 256×256 canvas.
# White stroke on near-black disc, with spark lines in accent orange.

def draw_logo():
    W, H = 256, 256
    surface = cairo.ImageSurface(cairo.FORMAT_ARGB32, W, H)
    ctx = cairo.Context(surface)

    # Clear to transparent
    ctx.set_source_rgba(0, 0, 0, 0)
    ctx.paint()

    # Dark disc
    ctx.arc(W/2, H/2, 118, 0, 2*math.pi)
    ctx.set_source_rgb(*rgb_f(BG))
    ctx.fill()

    # Subtle ember glow at disc edge
    grad = cairo.RadialGradient(W/2, H/2+20, 60, W/2, H/2+20, 120)
    grad.add_color_stop_rgba(0, *rgb_f(ACCENT), 0.15)
    grad.add_color_stop_rgba(1, *rgb_f(ACCENT), 0.0)
    ctx.arc(W/2, H/2, 118, 0, 2*math.pi)
    ctx.set_source(grad)
    ctx.fill()

    # Scale/translate: 32×32 viewbox → 160×160, centred at (48, 48)
    SCALE = 160 / 32
    OX = (W - 160) / 2
    OY = (H - 160) / 2 + 8  # shift slightly down to account for sparks above

    ctx.save()
    ctx.translate(OX, OY)
    ctx.scale(SCALE, SCALE)

    ctx.set_line_cap(cairo.LINE_CAP_ROUND)
    ctx.set_line_join(cairo.LINE_JOIN_ROUND)
    ctx.set_line_width(1.4 / SCALE)  # keep visual stroke proportional

    # Anvil body
    ctx.set_source_rgb(*rgb_f(SILVER_HI))
    ctx.move_to(4, 12); ctx.line_to(28, 12); ctx.line_to(26, 16)
    ctx.line_to(22, 16); ctx.line_to(22, 20); ctx.line_to(10, 20)
    ctx.line_to(10, 16); ctx.line_to(6, 16); ctx.close_path()
    ctx.stroke()

    # Base taper
    ctx.move_to(12, 20); ctx.line_to(11, 24)
    ctx.line_to(21, 24); ctx.line_to(20, 20)
    ctx.stroke()

    # Stand
    ctx.move_to(9, 24); ctx.line_to(23, 24)
    ctx.line_to(23, 27); ctx.line_to(9, 27); ctx.close_path()
    ctx.stroke()

    # Spark lines (accent orange)
    ctx.set_source_rgb(*rgb_f(ACCENT))
    ctx.set_line_width(1.2 / SCALE)
    ctx.move_to(16, 4); ctx.line_to(16, 8); ctx.stroke()
    ctx.move_to(11, 6); ctx.line_to(13, 9); ctx.stroke()
    ctx.move_to(21, 6); ctx.line_to(19, 9); ctx.stroke()

    ctx.restore()

    out = str(BRANDING / "logo.png")
    surface.write_to_png(out)
    print(f"  ✓ {out}")
    return out


# ── BANNER ───────────────────────────────────────────────────────────────────

def draw_banner():
    W, H = 800, 200
    img = Image.new("RGB", (W, H), BG)
    draw = ImageDraw.Draw(img)

    # Subtle horizontal ember gradient strip across the centre
    for y in range(H):
        t = 1.0 - abs(y - H/2) / (H/2)
        t = max(0, t * 0.08)
        r = int(BG[0] + (ACCENT[0] - BG[0]) * t)
        g = int(BG[1] + (ACCENT[1] - BG[1]) * t)
        b = int(BG[2] + (ACCENT[2] - BG[2]) * t)
        draw.line([(0, y), (W, y)], fill=(r, g, b))

    # Accent rule
    draw.rectangle([(0, 140), (W, 142)], fill=ACCENT)

    # Wordmark
    f_word = font("wordmark", 68)
    draw.text((40, 38), "FOUNDRY LINUX", font=f_word, fill=SILVER_HI)

    # Sub-label
    f_sub = font("body", 18)
    draw.text((42, 152), "26.04 LTS · ANVIL", font=f_sub, fill=ACCENT)

    out = str(BRANDING / "banner.png")
    img.save(out)
    print(f"  ✓ {out}")


# ── SLIDES ───────────────────────────────────────────────────────────────────

SLIDES = [
    ("slide-01-forge.png",   "The Forge.",
     "WorldFoundry GDK, retro toolkit, Blender add-ons — struck in from first boot."),
    ("slide-02-apt.png",     "Stay current.",
     "apt.foundrylinux.org keeps every Foundry tool signed, rebuilt, and re-tested on every push."),
    ("slide-03-devbox.png",  "Distrobox-ready.",
     "ghcr.io/foundry-linux/devbox:26.04 — the same environment, containerised."),
    ("slide-04-docs.png",    "Where to go next.",
     "foundrylinux.org/docs — package catalogue, install guide, and contribution notes."),
]


def _wallpaper_bg(w, h, brightness=0.45):
    wp = Image.open(WALLPAPER).convert("RGB")
    # Scale to cover target
    wp_ratio = wp.width / wp.height
    tgt_ratio = w / h
    if tgt_ratio > wp_ratio:
        nw, nh = w, int(w / wp_ratio)
    else:
        nw, nh = int(h * wp_ratio), h
    wp = wp.resize((nw, nh), Image.LANCZOS)
    # Centre crop
    x0 = (nw - w) // 2
    y0 = (nh - h) // 2
    wp = wp.crop((x0, y0, x0 + w, y0 + h))
    return ImageEnhance.Brightness(wp).enhance(brightness)


def _wrap_text(text, font_obj, max_w, draw):
    """Word-wrap text, return list of lines."""
    words = text.split()
    lines, line = [], []
    for word in words:
        test = " ".join(line + [word])
        bbox = draw.textbbox((0, 0), test, font=font_obj)
        if bbox[2] > max_w and line:
            lines.append(" ".join(line))
            line = [word]
        else:
            line.append(word)
    if line:
        lines.append(" ".join(line))
    return lines


def draw_slides():
    W, H = 800, 440
    bg_base = _wallpaper_bg(W, H)

    f_title = font("bold", 34)
    f_body  = font("body", 17)

    for filename, title, body in SLIDES:
        img = bg_base.copy()
        draw = ImageDraw.Draw(img)

        # Dark scrim over lower third for text legibility
        scrim = Image.new("RGBA", (W, 180), (0x0a, 0x0a, 0x0a, 200))
        img.paste(scrim, (0, H - 180), scrim)

        draw = ImageDraw.Draw(img)

        # Accent rule above text area
        draw.rectangle([(0, H - 180), (W, H - 178)], fill=ACCENT)

        # Title
        draw.text((32, H - 166), title, font=f_title, fill=ACCENT)

        # Body (word-wrapped)
        lines = _wrap_text(body, f_body, W - 64, draw)
        y = H - 122
        for ln in lines:
            draw.text((32, y), ln, font=f_body, fill=TEXT)
            y += 26

        out = str(BRANDING / filename)
        img.save(out)
        print(f"  ✓ {out}")


# ── GRUB BACKGROUND ──────────────────────────────────────────────────────────

def draw_grub_bg():
    bg = _wallpaper_bg(1920, 1080, brightness=0.85)
    out = str(GRUB / "background.png")
    bg.save(out)
    print(f"  ✓ {out}")
    return out


# ── GRUB THEME.TXT ───────────────────────────────────────────────────────────

GRUB_THEME = """\
# Foundry Linux GRUB2 theme
# Place in /usr/share/grub/themes/foundry-linux/

desktop-image: "background.png"
desktop-color: "#0a0a0a"
title-color: "#f7f7f7"
message-color: "#ff5b1a"

+ boot_menu {
  left   = 10%
  top    = 22%
  width  = 80%
  height = 55%
  item_color          = "#ebe8e2"
  selected_item_color = "#ffffff"
  item_height         = 34
  item_padding        = 12
  item_spacing        = 4
  menu_pixmap_style   = ""
  scrollbar           = false
}

+ label {
  top    = 83%
  left   = 50%-140
  width  = 280
  align  = "center"
  text   = "FOUNDRY LINUX 26.04 LTS"
  color  = "#ff5b1a"
  font   = "sans Regular 13"
}
"""


def write_grub_theme():
    out = GRUB / "theme.txt"
    out.write_text(GRUB_THEME)
    print(f"  ✓ {out}")


# ── PLYMOUTH ─────────────────────────────────────────────────────────────────

PLYMOUTH_THEME = """\
[Plymouth Theme]
Name=Foundry Linux
Description=Boot animation for Foundry Linux 26.04
ModuleName=script

[script]
ImageDir=/usr/share/plymouth/themes/foundry-linux
ScriptFile=/usr/share/plymouth/themes/foundry-linux/foundry-linux.script
"""

PLYMOUTH_SCRIPT = """\
// Foundry Linux Plymouth boot animation
// Module: script (built-in)

// Background
bg_image = Image("background.png");
bg_sprite = Sprite(bg_image);
bg_sprite.SetX(0);
bg_sprite.SetY(0);
bg_sprite.SetZ(-100);

// Logo — centred on screen
logo_image  = Image("logo.png");
screen_w    = Window.GetWidth();
screen_h    = Window.GetHeight();
logo_sprite = Sprite(logo_image);
logo_sprite.SetX(screen_w / 2 - logo_image.GetWidth()  / 2);
logo_sprite.SetY(screen_h / 2 - logo_image.GetHeight() / 2 - 40);
logo_sprite.SetZ(1);

// Spinner arc (drawn as a small progress bar)
progress_image  = Image.Text("", 1, 0.36, 0.10);  // placeholder
spinner_angle   = 0.0;
spinner_radius  = 40;
spinner_x       = screen_w / 2;
spinner_y       = screen_h / 2 + logo_image.GetHeight() / 2 + 20;

fun refresh_callback() {
    spinner_angle = spinner_angle + 0.12;
    if (spinner_angle > Math.Pi * 2)
        spinner_angle = spinner_angle - Math.Pi * 2;
}

Plymouth.SetRefreshFunction(refresh_callback);
"""


def write_plymouth():
    (PLYMOUTH / "foundry-linux.plymouth").write_text(PLYMOUTH_THEME)
    print(f"  ✓ {PLYMOUTH}/foundry-linux.plymouth")
    (PLYMOUTH / "foundry-linux.script").write_text(PLYMOUTH_SCRIPT)
    print(f"  ✓ {PLYMOUTH}/foundry-linux.script")


# ── SDDM ─────────────────────────────────────────────────────────────────────

SDDM_METADATA = """\
[SddmGreeterTheme]
Name=Foundry Linux
Description=Foundry Linux 26.04 login screen
Author=Foundry Linux
Version=1.0
Website=https://foundrylinux.org
MainScript=Main.qml
"""

SDDM_QML = """\
// Foundry Linux SDDM login theme
// Requires SDDM 0.20+ (KDE Plasma 6)

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import SddmComponents 2.0

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
                Keys.onReturnPressed: sddm.login(userInput.text, passInput.text, sessionModel.index(0, 0))
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
                onClicked: sddm.login(userInput.text, passInput.text, sessionModel.index(0, 0))
            }
        }
    }

    Component.onCompleted: userInput.forceActiveFocus()
}
"""


def write_sddm():
    (SDDM / "metadata.desktop").write_text(SDDM_METADATA)
    print(f"  ✓ {SDDM}/metadata.desktop")
    (SDDM / "Main.qml").write_text(SDDM_QML)
    print(f"  ✓ {SDDM}/Main.qml")


# ── COPY SHARED IMAGES ───────────────────────────────────────────────────────

def copy_shared(logo_src, grub_bg_src):
    shutil.copy(logo_src,    str(PLYMOUTH / "logo.png"))
    print(f"  ✓ {PLYMOUTH}/logo.png")
    shutil.copy(grub_bg_src, str(PLYMOUTH / "background.png"))
    print(f"  ✓ {PLYMOUTH}/background.png")
    shutil.copy(grub_bg_src, str(SDDM / "background.png"))
    print(f"  ✓ {SDDM}/background.png")


# ── MAIN ─────────────────────────────────────────────────────────────────────

def main():
    print("=== Generating Foundry Linux branding assets ===\n")

    print("Logo:")
    logo = draw_logo()

    print("\nBanner:")
    draw_banner()

    print("\nSlides:")
    draw_slides()

    print("\nGRUB background:")
    grub_bg = draw_grub_bg()
    write_grub_theme()

    print("\nPlymouth:")
    write_plymouth()

    print("\nSDDM:")
    write_sddm()

    print("\nCopying shared images:")
    copy_shared(logo, grub_bg)

    print("\n=== Done. Remove scripts/.font-cache/ if you want to re-download fonts. ===")


if __name__ == "__main__":
    main()
