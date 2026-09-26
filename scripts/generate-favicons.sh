#!/usr/bin/env bash
# Render the canonical anvil as a multi-size ICO and sync both formats to
# the independently published APT and ISO sites. Requires librsvg and ImageMagick.
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

if command -v magick >/dev/null; then
  CONVERT=magick
else
  CONVERT=convert
fi

# librsvg preserves the clipping and thin outline of the two anvil halves.
PNG="$(mktemp --suffix=.png)"
trap 'rm -f "$PNG"' EXIT
rsvg-convert --keep-aspect-ratio -w 256 -h 256 "$ROOT/site/favicon.svg" -o "$PNG"

# Preserve the SVG's proportions on a transparent square canvas.
"$CONVERT" "$PNG" -background none -gravity center -extent 256x256 \
  -define icon:auto-resize=64,48,32,16 "$ROOT/site/favicon.ico"

for dest in "$ROOT/foundry-apt/gen/static" "$ROOT/foundry-iso/gen/static"; do
  mkdir -p "$dest"
  cp "$ROOT/site/favicon.svg" "$ROOT/site/favicon.ico" "$dest/"
  cp "$ROOT/site/favicon.svg" "$dest/favicon-anvil.svg"
  cp "$ROOT/site/favicon.ico" "$dest/favicon-anvil.ico"
done

cp "$ROOT/site/favicon.svg" "$ROOT/site/favicon-anvil.svg"
cp "$ROOT/site/favicon.ico" "$ROOT/site/favicon-anvil.ico"
