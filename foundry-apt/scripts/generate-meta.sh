#!/usr/bin/env bash
# generate-meta.sh — emit public/meta/{name}.json per package.
#
# Usage:
#   generate-meta.sh              # all packages (skips up-to-date)
#   generate-meta.sh PACKAGE      # single package
#
# Environment:
#   PUBLISH_DIR   path to the published public/ tree (default: <repo>/public)
set -euo pipefail

if [[ "${1:-}" == "-h" || "${1:-}" == "--help" ]]; then
  echo "Usage: generate-meta.sh [PACKAGE_NAME]"
  echo "Generate public/meta/{name}.json from debian/control + debian/changelog."
  echo "Without PACKAGE_NAME, processes all packages, skipping up-to-date ones."
  exit 0
fi

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
OUT_DIR="${PUBLISH_DIR:-$REPO_ROOT/public}"
SINGLE_PKG="${1:-}"

mkdir -p "$OUT_DIR/meta"

python3 - "$REPO_ROOT" "$OUT_DIR" "$SINGLE_PKG" <<'PYEOF'
import json, os, re, sys
from datetime import datetime, timezone

repo_root, out_dir, single_pkg = sys.argv[1], sys.argv[2], sys.argv[3]
meta_dir   = os.path.join(out_dir, "meta")
pkg_root   = os.path.join(repo_root, "packages")

def parse_control(path):
    """Parse a debian/control file into a list of stanza dicts."""
    stanzas, cur = [], {}
    with open(path) as f:
        field, val_lines = None, []
        def flush():
            if field:
                cur[field] = "\n".join(val_lines).rstrip()
        for line in f:
            line = line.rstrip("\n")
            if line == "":
                flush()
                if cur:
                    stanzas.append(cur)
                cur, field, val_lines = {}, None, []
            elif line[0] in (" ", "\t"):
                val_lines.append(line[1:])  # strip one leading space
            else:
                flush()
                field, _, rest = line.partition(":")
                field = field.strip()
                val_lines = [rest.strip()]
        flush()
        if cur:
            stanzas.append(cur)
    return stanzas

def parse_changelog_version(path):
    """Extract version from the first line of debian/changelog."""
    with open(path) as f:
        line = f.readline()
    m = re.match(r"^\S+\s+\(([^)]+)\)", line)
    return m.group(1) if m else ""

def clean_depends(raw):
    """Strip ${...} substitution vars and version constraints; return name list."""
    pkgs = []
    for token in raw.split(","):
        token = token.strip()
        if not token or token.startswith("${"):
            continue
        # strip "(>= 1.0)" style constraints and alternatives (|)
        name = token.split("|")[0].split("(")[0].strip()
        if name:
            pkgs.append(name)
    return pkgs

def parse_long_desc(raw):
    """
    Debian long-description: each line has a leading space stripped already
    by parse_control. ' .' lines are paragraph separators.
    Returns plain-text with blank lines between paragraphs.
    """
    paras, cur = [], []
    for line in raw.splitlines():
        if line == ".":
            if cur:
                paras.append("\n".join(cur))
            cur = []
        else:
            cur.append(line)
    if cur:
        paras.append("\n".join(cur))
    return "\n\n".join(paras).strip()

def installed_size_kb(pkg_name, out_dir):
    """Read Installed-Size from the published Packages files; return int or None."""
    import glob
    for pf in glob.glob(os.path.join(out_dir, "dists", "*", "*", "binary-*", "Packages")):
        stanza = None
        cur = {}
        with open(pf) as f:
            for line in f:
                line = line.rstrip("\n")
                if line == "":
                    if cur.get("Package") == pkg_name:
                        stanza = cur
                        break
                    cur = {}
                elif line[0] not in (" ", "\t"):
                    k, _, v = line.partition(":")
                    cur[k.strip()] = v.strip()
        if stanza:
            val = stanza.get("Installed-Size", "")
            return int(val) if val.isdigit() else None
    return None

def process(name, pkg_dir):
    control_path   = os.path.join(pkg_dir, "debian", "control")
    changelog_path = os.path.join(pkg_dir, "debian", "changelog")
    if not (os.path.isfile(control_path) and os.path.isfile(changelog_path)):
        return False

    out_path = os.path.join(meta_dir, name + ".json")

    # Skip if meta is newer than both source files
    if os.path.isfile(out_path):
        out_mtime = os.path.getmtime(out_path)
        if out_mtime > os.path.getmtime(control_path) and \
           out_mtime > os.path.getmtime(changelog_path):
            print(f"meta/{name}.json  up to date — skipping")
            return True

    stanzas = parse_control(control_path)
    version = parse_changelog_version(changelog_path)

    # Source stanza is first; find our Package: stanza
    source_stanza = stanzas[0] if stanzas else {}
    homepage = source_stanza.get("Homepage", "")

    pkg_stanza = next(
        (s for s in stanzas[1:] if s.get("Package") == name),
        stanzas[1] if len(stanzas) > 1 else {}
    )
    if not pkg_stanza:
        print(f"WARNING: no Package: stanza for {name} — skipping", file=sys.stderr)
        return False

    desc_raw   = pkg_stanza.get("Description", "")
    desc_lines = desc_raw.splitlines()
    desc_short = desc_lines[0] if desc_lines else ""
    desc_long  = parse_long_desc("\n".join(desc_lines[1:])) if len(desc_lines) > 1 else ""

    depends_raw = pkg_stanza.get("Depends", "")
    depends     = clean_depends(depends_raw)

    inst_size = installed_size_kb(name, out_dir)

    deb_arch = pkg_stanza.get("Architecture", "all")
    letter   = name[0]
    if deb_arch == "all":
        deb_url = f"/pool/main/{letter}/{name}/{name}_{version}_all.deb"
    else:
        deb_url = f"/pool/main/{letter}/{name}/{name}_{version}_amd64.deb"

    meta = {
        "generated":         datetime.now(timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ"),
        "name":              name,
        "version":           version,
        "architecture":      deb_arch,
        "section":           pkg_stanza.get("Section", source_stanza.get("Section", "")),
        "homepage":          homepage,
        "depends":           depends,
        "description_short": desc_short,
        "description_long":  desc_long,
        "installed_size_kb": inst_size,
        "deb_url":           deb_url,
    }

    with open(out_path, "w") as f:
        json.dump(meta, f, indent=2, ensure_ascii=False)
        f.write("\n")
    print(f"meta/{name}.json  written")
    return True

if single_pkg:
    pkg_dir = os.path.join(pkg_root, single_pkg)
    if not os.path.isdir(pkg_dir):
        print(f"ERROR: packages/{single_pkg}/ not found", file=sys.stderr)
        sys.exit(1)
    ok = process(single_pkg, pkg_dir)
    sys.exit(0 if ok else 1)
else:
    for entry in sorted(os.listdir(pkg_root)):
        pkg_dir = os.path.join(pkg_root, entry)
        if os.path.isdir(pkg_dir):
            process(entry, pkg_dir)
PYEOF
