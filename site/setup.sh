#!/usr/bin/env bash
# Foundry Linux archive setup — idempotent curl-bash payload.
# https://foundrylinux.org/setup.sh
#
# Wires up the four apt sources the foundry-* editions need:
#   1. apt.foundrylinux.org           — Foundry metapackages
#   2. apt.worldfoundry.org           — WorldFoundry stack (cross-repo Depends)
#   3. dl.cloudsmith.io/public/task   — go-task runner (Cloudsmith)
#   4. Ubuntu multiverse component    — ROM-bundled emulators, NDK, etc.
#
# After this runs, `sudo apt install foundry-anvil` (or sprite/atelier)
# resolves cleanly. Safe to re-run.
#
# Plan: docs/plans/2026-05-21-phase-2-devbox-image.md §"Source-wiring prereqs"

set -euo pipefail

# ─── 1. apt.foundrylinux.org ────────────────────────────────────────────────
if [[ ! -f /etc/apt/sources.list.d/foundry.list ]]; then
  echo "→ Adding apt.foundrylinux.org"
  sudo install -d /etc/apt/keyrings
  curl -fsSL https://apt.foundrylinux.org/key.gpg \
    | sudo gpg --dearmor -o /etc/apt/keyrings/foundry.gpg
  echo "deb [signed-by=/etc/apt/keyrings/foundry.gpg] https://apt.foundrylinux.org stable main" \
    | sudo tee /etc/apt/sources.list.d/foundry.list > /dev/null
else
  echo "✓ apt.foundrylinux.org already configured"
fi

# ─── 2. apt.worldfoundry.org ────────────────────────────────────────────────
if [[ ! -f /etc/apt/sources.list.d/worldfoundry.list ]]; then
  echo "→ Adding apt.worldfoundry.org"
  sudo install -d /etc/apt/keyrings
  curl -fsSL https://apt.worldfoundry.org/key.gpg \
    | sudo gpg --dearmor -o /etc/apt/keyrings/worldfoundry.gpg
  echo "deb [signed-by=/etc/apt/keyrings/worldfoundry.gpg] https://apt.worldfoundry.org stable main" \
    | sudo tee /etc/apt/sources.list.d/worldfoundry.list > /dev/null
else
  echo "✓ apt.worldfoundry.org already configured"
fi

# ─── 3. Cloudsmith task repo ────────────────────────────────────────────────
if ! ls /etc/apt/sources.list.d/task-task.* >/dev/null 2>&1; then
  echo "→ Adding Cloudsmith task repo"
  curl -1sLf 'https://dl.cloudsmith.io/public/task/task/setup.deb.sh' | sudo bash
else
  echo "✓ Cloudsmith task repo already configured"
fi

# ─── 4. Ubuntu multiverse component ─────────────────────────────────────────
if grep -hsE '^(deb |Components:)' /etc/apt/sources.list /etc/apt/sources.list.d/* 2>/dev/null \
    | grep -qw multiverse; then
  echo "✓ Ubuntu multiverse already enabled"
else
  echo "→ Enabling Ubuntu multiverse"
  sudo add-apt-repository -y multiverse
fi

# ─── refresh ────────────────────────────────────────────────────────────────
sudo apt-get update -q

cat <<'NEXT'

Done. Install an edition:

  sudo apt install foundry-anvil      # base layer — devbox image preinstall
  sudo apt install foundry-sprite     # + heavy graphics + audio
  sudo apt install foundry-atelier    # + everything (~10 GB)
NEXT
