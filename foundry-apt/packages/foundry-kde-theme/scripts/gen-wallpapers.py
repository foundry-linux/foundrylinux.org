#!/usr/bin/env python3
"""Generate three WorldFoundry KDE wallpapers at 1920x1080, 3840x2160, and 400x225.

Brand palette (worldfoundry.org/src/styles/global.css):
  surface      #1a1714   (26, 23, 20)   warm near-black
  surface-hi   #241f1b   (36, 31, 27)   raised surfaces
  on-surface   #f2ede6   (242, 237, 230) warm white text
  accent       #f80000   (248, 0, 0)     molten red
  border       #3a322c   (58, 50, 44)    warm dark border
"""

import json
import math
import random
from pathlib import Path

from PIL import Image, ImageDraw

SCRIPT_DIR = Path(__file__).parent
DATA_DIR = SCRIPT_DIR.parent / "data" / "wallpapers"

FULL  = (1920, 1080)
HI    = (3840, 2160)
THUMB = (400, 225)

# ---------------------------------------------------------------------------
# Brand colours
# ---------------------------------------------------------------------------

C_BG     = (26, 23, 20)    # #1a1714 — surface
C_RAISED = (36, 31, 27)    # #241f1b — raised
C_TEXT   = (242, 237, 230) # #f2ede6 — on-surface
C_MUTED  = (184, 174, 163) # #b8aea3
C_ACCENT = (248, 0, 0)     # #f80000 — molten red
C_ACCENT_DARK = (180, 0, 0)# #b40000
C_BORDER = (58, 50, 44)    # #3a322c

# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

def clamp(v, lo=0, hi=255):
    return max(lo, min(hi, int(v)))

def rgba(r, g, b, a=255):
    return (clamp(r), clamp(g), clamp(b), clamp(a))

def lerp(a, b, t):
    return a + (b - a) * t

def lerp_color(c1, c2, t):
    t = max(0.0, min(1.0, t))
    return tuple(clamp(lerp(c1[i], c2[i], t)) for i in range(3))

# ---------------------------------------------------------------------------
# Wallpaper 1 — Forge Horizon
#
# Near-black warm surface, thin horizontal red glow stripe at ~45 % from
# bottom with wide bloom below and narrow bloom above; radial hotspot on
# the left third intensifies the glow.
# ---------------------------------------------------------------------------

def make_forge_horizon(w, h):
    img = Image.new("RGBA", (w, h), rgba(*C_BG))
    px = img.load()

    # Subtle warm noise across whole image
    rng = random.Random(42)
    for y in range(h):
        for x in range(w):
            n = rng.randint(0, 4)
            # warm-tinted noise: slightly more red than blue
            px[x, y] = rgba(C_BG[0] + n, C_BG[1] + max(0, n - 1), C_BG[2] + max(0, n - 2))

    horizon_y   = int(h * 0.55)
    bloom_down  = int(h * 0.18)
    bloom_up    = int(h * 0.07)
    hotspot_x   = int(w * 0.30)

    glow = Image.new("RGBA", (w, h), (0, 0, 0, 0))
    gd   = glow.load()

    for y in range(h):
        dy = y - horizon_y
        if dy >= 0:
            t  = dy / bloom_down
            if t > 1.0:
                continue
            base_alpha = (1.0 - t * t) * 155
            deep_color = (60, 8, 8)
        else:
            t  = (-dy) / bloom_up
            if t > 1.0:
                continue
            base_alpha = (1.0 - t * t) * 190
            deep_color = C_ACCENT_DARK

        for x in range(w):
            dx = (x - hotspot_x) / (w * 0.35)
            boost = math.exp(-dx * dx * 0.5) * 0.65
            alpha = clamp(base_alpha * (1.0 + boost))

            if dy >= 0:
                color = lerp_color(C_ACCENT, deep_color, min(t, 1.0))
            else:
                color = lerp_color((255, 60, 60), C_ACCENT, min(-dy / bloom_up, 1.0))

            gd[x, y] = rgba(*color, alpha)

    # Sharp core line
    lthick = max(1, h // 540)
    core = Image.new("RGBA", (w, h), (0, 0, 0, 0))
    cd   = core.load()
    for y in range(horizon_y - lthick, horizon_y + lthick + 1):
        if 0 <= y < h:
            for x in range(w):
                dx = (x - hotspot_x) / (w * 0.4)
                b  = 0.5 + 0.5 * math.exp(-dx * dx * 0.3)
                cd[x, y] = rgba(255, clamp(40 * b), clamp(40 * b), clamp(230 * b))

    result = Image.alpha_composite(img, glow)
    result = Image.alpha_composite(result, core)
    return result.convert("RGB")


# ---------------------------------------------------------------------------
# Wallpaper 2 — Ember Rise
#
# Radial red gradient from bottom-left, scattered spark particles in the
# lower third.
# ---------------------------------------------------------------------------

def make_ember_rise(w, h):
    img = Image.new("RGB", (w, h), C_BG)
    px  = img.load()

    for y in range(h):
        for x in range(w):
            dx = x / w
            dy = (h - y) / h
            dist = math.sqrt(dx * dx + dy * dy) / math.sqrt(2)

            if dist < 0.05:
                c = C_ACCENT
            elif dist < 0.35:
                t2 = (dist - 0.05) / 0.30
                c  = lerp_color(C_ACCENT, (80, 10, 10), t2)
            elif dist < 0.65:
                t2 = (dist - 0.35) / 0.30
                c  = lerp_color((80, 10, 10), (40, 22, 18), t2)
            else:
                t2 = min((dist - 0.65) / 0.35, 1.0)
                c  = lerp_color((40, 22, 18), C_BG, t2)

            px[x, y] = c

    # Spark particles
    rng = random.Random(7)
    particle_layer = Image.new("RGBA", (w, h), (0, 0, 0, 0))

    n = max(60, w * h // 34560)
    for _ in range(n):
        px_x = int(rng.gauss(w * 0.22, w * 0.28))
        px_y = int(rng.uniform(h * 0.50, h))
        size = rng.randint(max(1, w // 960), max(2, w // 480))
        bright = rng.uniform(0.7, 1.0)
        r  = clamp(255 * bright)
        g  = clamp(rng.randint(0, 50) * bright)
        b  = clamp(rng.randint(0, 20) * bright)
        a0 = clamp(rng.randint(160, 245))

        sz = size * 6 + 1
        glow = Image.new("RGBA", (sz, sz), (0, 0, 0, 0))
        gd2  = glow.load()
        cx = cy = size * 3
        for gy in range(sz):
            for gx in range(sz):
                d2 = math.sqrt((gx - cx) ** 2 + (gy - cy) ** 2)
                if d2 <= size:
                    gd2[gx, gy] = (r, g, b, a0)
                elif d2 <= size * 3:
                    t = (d2 - size) / (size * 2)
                    gd2[gx, gy] = (r, g, b, clamp(a0 * (1.0 - t) * 0.4))

        bx = px_x - size * 3
        by = px_y - size * 3
        if 0 <= bx < w and 0 <= by < h:
            particle_layer.paste(glow, (bx, by), glow)

    base_rgba = img.convert("RGBA")
    return Image.alpha_composite(base_rgba, particle_layer).convert("RGB")


# ---------------------------------------------------------------------------
# Wallpaper 3 — Cast Iron
#
# Warm dark base with a subtle hexagonal grid. Lower-left quadrant glows
# faintly red — hot cast iron cooling in a mould.
# ---------------------------------------------------------------------------

def make_cast_iron(w, h):
    img  = Image.new("RGB", (w, h), C_BG)
    draw = ImageDraw.Draw(img)

    hex_r = max(40, w // 48)
    hex_h = int(hex_r * math.sqrt(3))

    def hex_verts(cx, cy, r):
        return [
            (cx + r * math.cos(math.radians(60 * i - 30)),
             cy + r * math.sin(math.radians(60 * i - 30)))
            for i in range(6)
        ]

    cols = w // hex_r + 3
    rows = h // hex_h + 3

    for row in range(-1, rows + 1):
        for col in range(-1, cols + 1):
            cx = col * hex_r * 1.5
            cy = row * hex_h + (hex_h // 2 if col % 2 == 1 else 0)

            rel_x = cx / w
            rel_y = cy / h
            # hot factor: lower-left quadrant, fades off toward centre/right/top
            hot = max(0.0, min(1.0, (1.0 - rel_x * 2.2) * ((rel_y - 0.28) / 0.72)))

            # cell fill (hot cells get a red tint above the warm surface)
            if hot > 0.05:
                fr = clamp(C_BG[0] + hot * 28)
                fg = clamp(C_BG[1] + hot * 4)
                fb = clamp(C_BG[2] + hot * 2)
                verts = hex_verts(cx, cy, hex_r - 1)
                draw.polygon(verts, fill=(fr, fg, fb))

            # grid lines — base warm-border colour, brightened in hot zone
            lc = lerp_color(C_BORDER, (90, 35, 30), hot)
            verts = hex_verts(cx, cy, hex_r)
            for i in range(6):
                x0, y0 = verts[i]
                x1, y1 = verts[(i + 1) % 6]
                draw.line([(x0, y0), (x1, y1)], fill=lc, width=max(1, w // 1920))

    return img


# ---------------------------------------------------------------------------
# Metadata
# ---------------------------------------------------------------------------

METADATA = {
    "FoundryLinux-ForgeHorizon": {
        "KPlugin": {
            "Authors": [{"Name": "World Foundry"}],
            "Description": "Dark warm surface, molten red horizon glow.",
            "Id": "FoundryLinux-ForgeHorizon",
            "License": "CC-BY-SA-4.0",
            "Name": "Forge Horizon",
        }
    },
    "FoundryLinux-EmberRise": {
        "KPlugin": {
            "Authors": [{"Name": "World Foundry"}],
            "Description": "Red embers rising from the lower-left corner.",
            "Id": "FoundryLinux-EmberRise",
            "License": "CC-BY-SA-4.0",
            "Name": "Ember Rise",
        }
    },
    "FoundryLinux-CastIron": {
        "KPlugin": {
            "Authors": [{"Name": "World Foundry"}],
            "Description": "Hexagonal cast-iron mould, glowing red in the lower-left.",
            "Id": "FoundryLinux-CastIron",
            "License": "CC-BY-SA-4.0",
            "Name": "Cast Iron",
        }
    },
}

GENERATORS = {
    "FoundryLinux-ForgeHorizon": make_forge_horizon,
    "FoundryLinux-EmberRise":    make_ember_rise,
    "FoundryLinux-CastIron":     make_cast_iron,
}

# ---------------------------------------------------------------------------
# Save
# ---------------------------------------------------------------------------

def save_wallpaper(name, img_full):
    base = DATA_DIR / name
    images_dir = base / "contents" / "images"
    images_dir.mkdir(parents=True, exist_ok=True)

    img_full.save(images_dir / "1920x1080.png", optimize=True)
    print(f"  saved {name}/contents/images/1920x1080.png")

    img_full.resize(HI, Image.LANCZOS).save(images_dir / "3840x2160.png", optimize=True)
    print(f"  saved {name}/contents/images/3840x2160.png")

    img_full.resize(THUMB, Image.LANCZOS).save(base / "contents" / "screenshot.png", optimize=True)
    print(f"  saved {name}/contents/screenshot.png")

    with open(base / "metadata.json", "w") as f:
        json.dump(METADATA[name], f, indent=2)
        f.write("\n")
    print(f"  saved {name}/metadata.json")


def main():
    for name, gen in GENERATORS.items():
        print(f"\nGenerating {name} …")
        img = gen(*FULL)
        save_wallpaper(name, img)
    print("\nDone.")


if __name__ == "__main__":
    main()
