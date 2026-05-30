#!/usr/bin/env python3
"""Fetch top 4000 PyPI packages with metadata and output a markdown table.

Metadata is cached under .pypi-cache/<name>.json so repeat runs are fast.
Cache is intentionally never auto-expired — delete files manually to refresh.
"""
import json
import os
import sys
import urllib.request
import concurrent.futures
from pathlib import Path

CACHE_DIR = Path(__file__).parent / ".pypi-cache"
CACHE_DIR.mkdir(exist_ok=True)

GAME_DEV_KEYWORDS = {
    "pygame", "pyglet", "arcade", "panda3d", "pyopengl", "opengl", "glfw", "sdl",
    "vulkan", "directx", "gamepad", "joystick", "controller", "xinput", "evdev",
    "audio", "sound", "music", "ogg", "wav", "mp3", "flac", "midi", "alsa",
    "image", "texture", "sprite", "pixel", "png", "jpeg", "bmp", "tga", "webp",
    "math", "vector", "matrix", "quaternion", "linear algebra", "glm",
    "physics", "collision", "rigid", "box2d", "bullet", "chipmunk",
    "network", "socket", "udp", "tcp", "multiplayer", "gameserver", "enet",
    "tiled", "tmx", "ldtk", "tilemap", "spritesheet",
    "shader", "glsl", "hlsl", "spirv",
    "animation", "skeletal", "tweening", "tween",
    "ai", "pathfinding", "astar", "navmesh", "behavior tree",
    "steam", "steamworks", "itch",
    "3d", "mesh", "gltf", "fbx", "obj", "blender",
    "retro", "emulator", "rom", "nes", "snes", "gameboy",
    "controller", "haptic", "rumble",
    "ui", "widget", "gui", "hud", "imgui",
    "numpy", "scipy", "cython",  # core scientific
    "noise", "perlin", "simplex",  # procedural generation
    "compression", "zlib", "lz4", "zstd",
    "serial", "usb", "hid",
    "profil", "benchmark", "perf",
    "scripting", "lua", "wasm",
    "ecs", "entity", "component",
}

GAME_DEV_TOPICS = {
    "Games/Entertainment",
    "Multimedia", "Multimedia :: Sound/Audio", "Multimedia :: Graphics",
    "Software Development :: Libraries",
    "Scientific/Engineering :: Mathematics",
    "Scientific/Engineering :: Visualization",
}


def fetch_json(url, timeout=15):
    try:
        with urllib.request.urlopen(url, timeout=timeout) as r:
            return json.loads(r.read())
    except Exception:
        return None


def get_top_packages(n=4000):
    # Cache the package list too — it changes daily, but good enough for a session
    cache_file = CACHE_DIR / "_top-packages.json"
    if cache_file.exists():
        data = json.loads(cache_file.read_text())
    else:
        url = "https://hugovk.github.io/top-pypi-packages/top-pypi-packages-30-days.min.json"
        data = fetch_json(url)
        if not data:
            print("Failed to fetch top-packages list", file=sys.stderr)
            sys.exit(1)
        cache_file.write_text(json.dumps(data))
    return data["rows"][:n]


def get_package_info(row):
    name = row["project"]
    downloads = row["download_count"]

    cache_file = CACHE_DIR / f"{name}.json"
    if cache_file.exists():
        data = json.loads(cache_file.read_text())
    else:
        url = f"https://pypi.org/pypi/{name}/json"
        data = fetch_json(url)
        if data:
            cache_file.write_text(json.dumps(data))
    if not data:
        return {"name": name, "downloads": downloads, "version": "?", "size": 0,
                "category": "", "summary": "", "requires_python": "", "game_dev": False}

    info = data.get("info", {})
    version = info.get("version", "?")
    summary = (info.get("summary", "") or "").strip()
    requires_python = info.get("requires_python", "") or ""

    # Category from classifiers
    classifiers = info.get("classifiers", [])
    topics = []
    for c in classifiers:
        if c.startswith("Topic :: "):
            parts = c.split(" :: ")
            # Use the most specific level available (up to 3 levels)
            topic = " :: ".join(parts[1:min(4, len(parts))])
            if topic not in topics:
                topics.append(topic)
    category = "; ".join(topics[:3]) if topics else "General"

    # Size: largest file in latest release
    releases = data.get("releases", {})
    latest_files = releases.get(version, [])
    size = max((f.get("size", 0) for f in latest_files), default=0)

    # Game-dev relevance heuristic
    text = f"{name} {summary} {category}".lower()
    game_dev = any(kw in text for kw in GAME_DEV_KEYWORDS) or \
               any(t in category for t in GAME_DEV_TOPICS)

    return {
        "name": name,
        "downloads": downloads,
        "version": version,
        "size": size,
        "category": category,
        "summary": summary[:120],
        "requires_python": requires_python,
        "game_dev": game_dev,
    }


NBSP = " "  # non-breaking space — keeps number+unit on one line

def fmt_size(n):
    if n <= 0:
        return "—"
    if n < 1024:
        return f"{n}{NBSP}B"
    if n < 1024 ** 2:
        return f"{n // 1024}{NBSP}KB"
    return f"{n // (1024 ** 2)}{NBSP}MB"


def fmt_dl(n):
    if n >= 1_000_000_000:
        return f"{n / 1_000_000_000:.1f}B"
    if n >= 1_000_000:
        return f"{n / 1_000_000:.0f}M"
    if n >= 1_000:
        return f"{n / 1_000:.0f}K"
    return str(n)


def main():
    print("Fetching top 4000 packages list...", file=sys.stderr)
    packages = get_top_packages(4000)
    total = len(packages)
    print(f"Got {total} packages. Fetching metadata (20 workers)...", file=sys.stderr)

    cached = sum(1 for p in packages if (CACHE_DIR / f"{p['project']}.json").exists())
    print(f"  {cached}/{total} already cached, {total - cached} to fetch.", file=sys.stderr)

    results = []
    done = 0
    with concurrent.futures.ThreadPoolExecutor(max_workers=20) as ex:
        futures = {ex.submit(get_package_info, pkg): pkg for pkg in packages}
        for future in concurrent.futures.as_completed(futures):
            done += 1
            if done % 200 == 0:
                print(f"  {done}/{total}...", file=sys.stderr)
            result = future.result()
            if result:
                results.append(result)

    results.sort(key=lambda x: x["downloads"], reverse=True)

    lines = [
        "# Top 4000 PyPI Packages by Monthly Downloads",
        "",
        "Source: [hugovk/top-pypi-packages](https://hugovk.github.io/top-pypi-packages/) + PyPI JSON API",
        "",
        "**Game Dev** column: `★` = likely relevant to game development (keyword/topic heuristic).",
        "",
        "| Rank | Package | Downloads | Version | Size | Game Dev | Category | Description |",
        "|-----:|---------|----------:|---------|-----:|:--------:|----------|-------------|",
    ]

    for i, r in enumerate(results, 1):
        summary = r["summary"].replace("|", "\\|")
        category = r["category"].replace("|", "\\|")
        game = "★" if r["game_dev"] else ""
        lines.append(
            f"| {i} | `{r['name']}` | {fmt_dl(r['downloads'])} | {r['version']} "
            f"| {fmt_size(r['size'])} | {game} | {category} | {summary} |"
        )

    print("\n".join(lines))
    print(f"\nDone. {len(results)} rows.", file=sys.stderr)


if __name__ == "__main__":
    main()
