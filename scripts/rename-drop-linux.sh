#!/usr/bin/env bash
# rename-drop-linux.sh — one-shot rename committed for history.
#
# Drops the redundant "linux" infix from every Foundry Linux package and
# install-script name. The distro is still "Foundry Linux" — branding —
# but the package units don't need to repeat it.
#
# Run from the repo root. Idempotent: re-running on a renamed tree is a no-op
# (the foundry-linux-* paths simply don't exist anymore).
#
# See: docs/plans/2026-05-21-drop-linux-from-package-names.md

set -euo pipefail

ROOT="$(git rev-parse --show-toplevel)"
cd "$ROOT"

ts() { date -u +%Y-%m-%dT%H:%M:%SZ; }
log() { printf '[%s] %s\n' "$(ts)" "$*"; }

MAINT_NAME="$(git config user.name)"
MAINT_EMAIL="$(git config user.email)"
RFC2822_DATE="$(date -uR)"

prepend_changelog() {
  # Args: new_pkg_name new_version old_pkg_name path/to/debian/changelog
  # NOTE: write the stanza directly to the temp file with printf — do NOT
  # capture it through $(printf ...) first, because command substitution
  # strips trailing newlines and the separator before the next stanza
  # would be lost (caught 2026-05-21 — dpkg-parsechangelog warned on
  # "badly formatted trailer line" because " +0000foundry-…" had no \n).
  local pkg="$1" ver="$2" old="$3" file="$4"
  local tmp; tmp="$(mktemp)"
  printf '%s (%s) resolute; urgency=medium\n\n  * Rename from %s.\n\n -- %s <%s>  %s\n\n' \
    "$pkg" "$ver" "$old" "$MAINT_NAME" "$MAINT_EMAIL" "$RFC2822_DATE" > "$tmp"
  cat "$file" >> "$tmp"
  mv "$tmp" "$file"
}

# ─── 1. Rename foundry-apt/packages/foundry-linux-* source dirs ──────────────
for old in foundry-apt/packages/foundry-linux-*/; do
  [[ -d "$old" ]] || continue                        # nothing to do
  old="${old%/}"                                     # strip trailing /
  new="${old/foundry-linux-/foundry-}"
  old_name="${old##*/}"
  new_name="${new##*/}"
  log "rename source dir $old_name → $new_name"
  git mv "$old" "$new"

  # Rewrite Source: + Package: lines in debian/control
  sed -i 's/^\(Source\|Package\): foundry-linux-/\1: foundry-/' "$new/debian/control"

  # Bump changelog with a Rename entry. Compute next patch version from
  # whatever's at the top of the existing changelog.
  cur_ver="$(dpkg-parsechangelog -l "$new/debian/changelog" -SVersion)"
  next_ver="$(echo "$cur_ver" | awk -F. -v OFS=. '{$NF=$NF+1; print}')"
  prepend_changelog "$new_name" "$next_ver" "$old_name" "$new/debian/changelog"
done

# ─── 2. Rename install-foundry-linux-*.sh scripts ────────────────────────────
for s in foundry-linux-setup/install-foundry-linux-*.sh; do
  [[ -f "$s" ]] || continue
  new="${s/install-foundry-linux-/install-foundry-}"
  log "rename script $(basename "$s") → $(basename "$new")"
  git mv "$s" "$new"
done

# ─── 3. Rename foundry-linux-setup/ → foundry-setup/ ─────────────────────────
if [[ -d foundry-linux-setup ]]; then
  log "rename directory foundry-linux-setup/ → foundry-setup/"
  git mv foundry-linux-setup foundry-setup
fi

# ─── 4. Sweep all tracked files for stale references ─────────────────────────
# Exclude:
#   * changelog files — already authored by step 1
#   * this script — the renames it describes are the historical record
#   * transcripts — historical conversation logs stay verbatim
log "sweep tracked files for foundry-linux-* string references"
git ls-files \
  | grep -Ev '^(foundry-apt/packages/.*/debian/changelog|scripts/rename-drop-linux\.sh|docs/transcripts/.*)$' \
  | xargs grep -lE 'foundry-linux-([a-z]+-?)+|foundry-linux-setup' 2>/dev/null \
  | xargs sed -i -e 's|foundry-linux-setup|foundry-setup|g' \
                 -e 's/foundry-linux-/foundry-/g' \
                 || true

log "done. review with: git status; git diff --stat"
