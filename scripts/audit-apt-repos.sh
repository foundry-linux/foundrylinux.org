#!/usr/bin/env bash
# audit-apt-repos.sh — regenerate the Foundry/WorldFoundry package-inventory
# markdown doc from live apt repo state + local source trees + Phase 0
# installer scripts.
#
# Output: a markdown document matching the shape of
#   docs/investigations/YYYY-MM-DD-package-inventory.md
#
# Usage:
#   bash scripts/audit-apt-repos.sh                       # stdout
#   bash scripts/audit-apt-repos.sh -o <path>             # write to file
#   bash scripts/audit-apt-repos.sh -h | --help           # usage
#
# Per Taskfile:
#   task audit-apt-repos                                  # writes to docs/investigations/$(date +%F)-package-inventory.md
#
# Inputs:
#   1. Live apt metadata over HTTP from both apt repos.
#   2. Each repo's local debian/{control,changelog} source trees.
#   3. foundry-setup/install-*.sh — extract the `apt-get install` lines.
#   4. scripts/audit-apt-repos.notes.yaml (optional) — narrative the
#      script can't infer from metadata (why-vendored, open-issues,
#      arch-specific caveats, etc.).
#
# Behaviour:
#   - set -euo pipefail; -h/--help short-circuits before any fetching.
#   - Fails fast (non-zero) if either apt repo is unreachable; never
#     emits a silent partial doc.
#   - No host deps beyond `bash`, `curl`, `python3` (stdlib only).

set -euo pipefail

usage() {
    cat <<'EOF'
audit-apt-repos.sh — regenerate the package-inventory markdown doc

Usage:
  bash scripts/audit-apt-repos.sh                   # markdown to stdout
  bash scripts/audit-apt-repos.sh -o <path>         # write to file
  bash scripts/audit-apt-repos.sh --notes <path>    # custom notes.yaml
  bash scripts/audit-apt-repos.sh -h | --help       # this help

Defaults:
  --notes  ./scripts/audit-apt-repos.notes.yaml  (silently absent → empty)

Exit codes:
  0  success
  2  bad CLI argument
  3  apt repo unreachable / unparseable

Designed to be invoked via `task audit-apt-repos` (writes to
docs/investigations/$(date +%F)-package-inventory.md).
EOF
}

OUT=""
NOTES=""

# Resolve the repo root from this script's location so the script works
# from any cwd (e.g. invoked via task, which runs from the Taskfile dir).
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

while [[ $# -gt 0 ]]; do
    case "$1" in
        -h|--help) usage; exit 0 ;;
        -o|--out)  OUT="$2"; shift 2 ;;
        --notes)   NOTES="$2"; shift 2 ;;
        *)         echo "ERROR: unknown arg: $1" >&2; usage; exit 2 ;;
    esac
done

# Default notes path: alongside this script, optional.
[[ -z "$NOTES" ]] && NOTES="$SCRIPT_DIR/audit-apt-repos.notes.yaml"
[[ -f "$NOTES" ]] || NOTES=""   # missing is OK

# Locate the sibling worldfoundry.org repo (the WF apt source tree).
WF_REPO_DEFAULT="$(cd "$REPO_ROOT/../worldfoundry.org" 2>/dev/null && pwd || true)"
WF_REPO="${WF_REPO:-$WF_REPO_DEFAULT}"
if [[ -z "$WF_REPO" || ! -d "$WF_REPO/apt/packages" ]]; then
    echo "WARN: worldfoundry.org repo not found at \$WF_REPO or ../worldfoundry.org/" >&2
    echo "      Live apt.worldfoundry.org data still works; local source-tree cross-check skipped." >&2
    WF_REPO=""
fi

PY="${PYTHON:-python3}"
"$PY" - "$REPO_ROOT" "$WF_REPO" "$NOTES" <<'PYEOF' > "${OUT:-/dev/stdout}"
"""Embedded python3 driver for audit-apt-repos.sh.

Pulls live apt repo state for both Foundry/WorldFoundry repos + reads
local source trees + parses Phase 0 install scripts, emits a single
markdown document with the same shape as the 2026-05-22 inventory.

Standard library only.
"""
from __future__ import annotations

import datetime as dt
import os
import re
import subprocess
import sys
import urllib.error
import urllib.request
from collections import defaultdict
from pathlib import Path

REPO_ROOT = Path(sys.argv[1])
WF_REPO   = Path(sys.argv[2]) if sys.argv[2] else None
NOTES     = Path(sys.argv[3]) if sys.argv[3] else None

REPOS = [
    {
        "host": "apt.worldfoundry.org",
        "suite": "stable",
        "scope": "WorldFoundry game-authoring tooling (CLIs + Blender add-ons + umbrella metapackages)",
        "maintainer": "WorldFoundry Packages <packages@worldfoundry.org>",
        "local_packages_dir": WF_REPO / "apt" / "packages" if WF_REPO else None,
        "keyring_filename": "worldfoundry.gpg",
    },
    {
        "host": "apt.foundrylinux.org",
        "suite": "resolute",
        "scope": "Broader Foundry Linux distro packages (vendored upstreams + creative-software + dev metapackages)",
        "maintainer": "Foundry Linux Packages <packages@foundrylinux.org>",
        "local_packages_dir": REPO_ROOT / "foundry-apt" / "packages",
        "keyring_filename": "foundrylinux.gpg",
    },
]

# ─── hard-coded template chunks (preserved boilerplate) ─────────────────

CONVENTIONS_RECAP = """\
## Conventions recap (for anyone authoring a new package)

- **Always check Ubuntu 26.04 universe first** (`apt-cache policy <pkg>` in a fresh `ubuntu:26.04` container) before vendoring. The xa65 episode (re-packaging a tool that was already in universe) is the cautionary tale.
- **Canonical Debian layout only**: `debian/{control,changelog,rules,source/format,copyright}`. No hand-rolled `dpkg-deb --build`; no uppercase `DEBIAN/` in source.
- **Changelog is authoritative for versions** — no `Version:` in `debian/control`. Bump via `dch -v <new> -D resolute "what changed"` (foundrylinux.org) or `... -D stable ...` (worldfoundry.org). `1.0.x` for dep changes; `1.x.0` for new packages.
- **Vendored upstreams** pin `UPSTREAM_VERSION` + `SHA256` at the top of `build.sh`; re-pin with `curl -fsSL <url> | sha256sum`.
- **Always build in a Docker container** — host deps silently satisfy Build-Depends and mask CI failures. CI mandates the ubuntu:26.04 base for both repos.
- **Use the `/package` skill** for new vendored packages — it generates the `dh_make` skeleton and patches in Foundry-customised fields.
- **Verify new version pins on stock ubuntu:26.04** — assumptions about "what Ubuntu ships" rot fast; `docker run --rm ubuntu:26.04 apt-cache madison <pkg>` is the 30-second check that beats extrapolation.
"""

SEE_ALSO = """\
## See also

- [`2026-05-19-package-inventory.md`](2026-05-19-package-inventory.md) — predecessor snapshot, captures the moment apt.foundrylinux.org was still 🚧 planned.
- [`2026-05-16-foundry-distro-proposal.md`](2026-05-16-foundry-distro-proposal.md) — strategic rationale, four delivery channels, Phases 2 (Distrobox) and 3 (ISO).
- [`docs/plans/2026-05-20-move-asset-packages.md`](../plans/2026-05-20-move-asset-packages.md) — the move of `blender-asset-finder` + rename of `wf-asset` → `blender-asset-finder-cli`.
- [`docs/plans/2026-05-22-audit-apt-repos-script.md`](../plans/2026-05-22-audit-apt-repos-script.md) — the plan this doc was built under (manual refresh → automate via script → compare).
- [`CLAUDE.md`](../../CLAUDE.md) §Conventions — authoring rules in one screen.
"""

CROSS_REPO_DIAGRAM = """\
## Cross-repo dependency picture

```
       ┌──────────────────────── apt.worldfoundry.org (live) ────────────────────────┐
       │                                                                              │
       │  worldfoundry-development ─► worldfoundry ──┬─► worldfoundry-cli ─► 9 CLIs   │
       │           │                                 │                                │
       │           │                                 └─► worldfoundry-blender-addons  │
       │           │                                       │       │                  │
       │           ▼                                       │       ▼                  │
       │   ubuntu universe                                 │   worldfoundry-blender-  │
       │   (build-essential,                               │   editor-exporter        │
       │    cmake, libx11-dev,                             │                          │
       │    libgl-dev, …)                                  │                          │
       │                                                   │                          │
       └───────────────────────────────────────────────────┼──────────────────────────┘
                                                           │
                                  cross-repo Depends ──────┘
                                                           │
       ┌──────────────────────── apt.foundrylinux.org (live) ────────────────────────┐
       │                                                   │                          │
       │                                                   ▼                          │
       │                                       blender-asset-finder                   │
       │                                                                              │
       │  foundry-retro-tools ─► mame, cc65, dasm, z80*, radare2, binwalk, …          │
       │      (+ vendored f9dasm, libvgm, vgmstream, ghidra)                          │
       │                                                                              │
       │  foundry-{atelier,anvil,sprite,art,pixel-art,daw,trackers,…}                 │
       │  foundry-{android,ios}-development                                           │
       │                                                                              │
       └──────────────────────────────────────────────────────────────────────────────┘
                                                           │
                                                           ▼
                                                  blender 5.0.1+dfsg-1ubuntu1
                                                    (Ubuntu universe / resolute)
```

The two repos remain deliberately separate in scope — foundrylinux ships the distro toolchain, worldfoundry ships the authoring stack — but they're no longer entirely disjoint: `worldfoundry-blender-addons → blender-asset-finder` is a real cross-repo dep edge that resolves cleanly when both apt sources are configured.
"""


# ─── helpers ────────────────────────────────────────────────────────────

def fetch(url: str, *, timeout: int = 20) -> str:
    """HTTP GET → str. Aborts (exit 3) on any error.

    Cloudflare R2 (which hosts both apt repos) returns 403 to clients
    sending the default Python urllib User-Agent, so we mimic a generic
    curl/wget. Same UA both apt repos use in their own bootstrap docs."""
    req = urllib.request.Request(url, headers={"User-Agent": "audit-apt-repos/1.0"})
    try:
        with urllib.request.urlopen(req, timeout=timeout) as r:
            return r.read().decode("utf-8", errors="replace")
    except (urllib.error.URLError, urllib.error.HTTPError) as e:
        sys.stderr.write(f"ERROR: failed to fetch {url}: {e}\n")
        sys.exit(3)


def parse_packages(text: str) -> list[dict]:
    """Parse a Debian Packages file into a list of dicts."""
    out = []
    for stanza in text.strip().split("\n\n"):
        if not stanza.strip():
            continue
        record: dict[str, str] = {}
        key = None
        for line in stanza.splitlines():
            if line.startswith((" ", "\t")) and key is not None:
                record[key] += "\n" + line.strip()
            elif ":" in line:
                key, _, val = line.partition(":")
                key = key.strip()
                record[key] = val.strip()
        if "Package" in record:
            out.append(record)
    return out


def parse_release(text: str) -> dict:
    """Parse a Release file (first-line headers only — we don't need the digests)."""
    out: dict[str, str] = {}
    for line in text.splitlines():
        if line.startswith((" ", "\t")):
            continue
        if ":" not in line:
            continue
        k, _, v = line.partition(":")
        out[k.strip()] = v.strip()
    return out


def fingerprint_from_keygpg(host: str) -> dict[str, str]:
    """gpg --show-keys parsing on the repo's /key.gpg. Returns dict with
    fingerprint, keyid, created, expires, uid. Missing gpg → empty."""
    url = f"https://{host}/key.gpg"
    req = urllib.request.Request(url, headers={"User-Agent": "audit-apt-repos/1.0"})
    try:
        with urllib.request.urlopen(req, timeout=20) as r:
            blob = r.read()
    except Exception as e:
        sys.stderr.write(f"WARN: failed to fetch {url}: {e}\n")
        return {}
    try:
        out = subprocess.run(
            ["gpg", "--show-keys", "--with-fingerprint", "--with-colons"],
            input=blob, capture_output=True, timeout=10,
        )
    except (FileNotFoundError, subprocess.TimeoutExpired):
        return {}
    info = {}
    def _fmt_ts(s: str) -> str:
        # gpg --with-colons emits epoch seconds; convert to YYYY-MM-DD.
        try:
            return dt.date.fromtimestamp(int(s)).isoformat()
        except (ValueError, OverflowError, OSError):
            return s
    for line in out.stdout.decode("utf-8", errors="replace").splitlines():
        parts = line.split(":")
        if parts[0] == "pub" and len(parts) >= 7:
            info["created"] = _fmt_ts(parts[5])
            info["expires"] = _fmt_ts(parts[6])
            info["keyid"]   = parts[4]
        elif parts[0] == "fpr" and len(parts) >= 10:
            info.setdefault("fingerprint", parts[9])
        elif parts[0] == "uid" and len(parts) >= 10:
            info.setdefault("uid", parts[9])
    return info


def load_notes(path: Path | None) -> dict:
    """Optional minimal YAML loader (key: value, no nesting, no anchors —
    keeps the script stdlib-only). Returns {pkg_name: {field: text, ...}}."""
    if path is None or not path.exists():
        return {}
    out: dict[str, dict[str, str]] = defaultdict(dict)
    cur_pkg = None
    cur_key = None
    for raw in path.read_text().splitlines():
        if not raw.strip() or raw.lstrip().startswith("#"):
            continue
        if not raw.startswith((" ", "\t")):
            cur_pkg = raw.split(":", 1)[0].strip()
            cur_key = None
        else:
            stripped = raw.strip()
            if ":" in stripped:
                k, _, v = stripped.partition(":")
                v = v.strip().strip('"').strip("'")
                cur_key = k.strip()
                if cur_pkg:
                    out[cur_pkg][cur_key] = v
    return dict(out)


def read_local_control(packages_dir: Path | None, pkg: str) -> dict[str, str]:
    """Parse <packages_dir>/<pkg>/debian/control's first stanza (Source:),
    returning a flat dict. Returns {} if the dir doesn't exist."""
    if packages_dir is None:
        return {}
    ctl = packages_dir / pkg / "debian" / "control"
    if not ctl.exists():
        return {}
    return parse_packages(ctl.read_text())[0] if ctl.read_text().strip() else {}


def read_local_changelog_top(packages_dir: Path | None, pkg: str) -> str:
    """First line of debian/changelog for context (version + suite)."""
    if packages_dir is None:
        return ""
    cl = packages_dir / pkg / "debian" / "changelog"
    if not cl.exists():
        return ""
    return cl.read_text().splitlines()[0] if cl.read_text() else ""


# ─── per-repo fetch + organise ──────────────────────────────────────────

def collect_repo(repo: dict) -> dict:
    host  = repo["host"]
    suite = repo["suite"]
    release_text = fetch(f"https://{host}/dists/{suite}/Release")
    release      = parse_release(release_text)

    # Pull binary-* Packages indices for each declared arch.
    arches = release.get("Architectures", "").split()
    packages_by_arch: dict[str, list[dict]] = {}
    for arch in arches:
        text = fetch(f"https://{host}/dists/{suite}/main/binary-{arch}/Packages")
        packages_by_arch[arch] = parse_packages(text)

    return {
        **repo,
        "release": release,
        "arches":  arches,
        "packages_by_arch": packages_by_arch,
        "key": fingerprint_from_keygpg(host),
    }


# ─── Phase 0 scan ───────────────────────────────────────────────────────

INSTALL_LINE_RE = re.compile(r"apt[- ]get? +install +[^|;\n]*", re.IGNORECASE)

def scan_foundry_setup() -> list[dict]:
    """Walk foundry-setup/*.sh, extract install lines + brief description."""
    setup = REPO_ROOT / "foundry-setup"
    if not setup.exists():
        return []
    out = []
    for path in sorted(setup.glob("install-*.sh")) + sorted(setup.glob("setup-*.sh")):
        first_comment = ""
        for line in path.read_text().splitlines()[:30]:
            if line.startswith("# ") and not line.startswith("#!"):
                first_comment = line[2:].strip()
                if first_comment and not first_comment.startswith(
                    ("set ", "lib.sh", "shellcheck", "shellcheck disable")
                ):
                    break
        installs = []
        for raw_line in path.read_text().splitlines():
            m = INSTALL_LINE_RE.search(raw_line)
            if m:
                # Strip trailing comments and surrounding sudo wrappers.
                line = m.group(0).rstrip()
                line = re.sub(r"\s+#.*$", "", line)
                installs.append(line)
        out.append({
            "script": path.name,
            "comment": first_comment,
            "installs": installs,
        })
    return out


# ─── rendering ──────────────────────────────────────────────────────────

def md_escape(s: str) -> str:
    return s.replace("|", "\\|")


def _annotations(pkg_notes: dict) -> str:
    """Build the inline annotations string (rename / stale flag / extra note)."""
    bits = []
    if "rename_from" in pkg_notes:
        bits.append(f"**Renamed from {pkg_notes['rename_from']}.**")
    if "stale_note" in pkg_notes:
        bits.append(pkg_notes["stale_note"])
    if "note" in pkg_notes:
        bits.append(pkg_notes["note"])
    return " " + " ".join(bits) if bits else ""


def render_metapackage_row(pkg: dict, notes: dict) -> str:
    pkg_notes = notes.get(pkg["Package"], {})
    depends = pkg.get("Depends", "").replace("\n", " ")
    depends = re.sub(r"\s+", " ", depends)
    return (
        f"| `{pkg['Package']}` | {pkg.get('Version','')} | {pkg.get('Architecture','')} | "
        f"{md_escape(depends)}{_annotations(pkg_notes)} |"
    )


def render_binary_row(pkg: dict, notes: dict) -> str:
    pkg_notes = notes.get(pkg["Package"], {})
    desc_first_line = pkg.get("Description", "").splitlines()[0] if pkg.get("Description") else ""
    what = pkg_notes.get("what_it_does", desc_first_line)
    extra = []
    if "why_vendored" in pkg_notes:
        extra.append(f"**Vendored:** {pkg_notes['why_vendored']}")
    if "upstream_url" in pkg_notes:
        extra.append(f"Upstream: `{pkg_notes['upstream_url']}`")
    annot = _annotations(pkg_notes)
    extra_text = (" — " + " · ".join(extra)) if extra else ""
    return (
        f"| `{pkg['Package']}` | {pkg.get('Version','')} | {pkg.get('Architecture','')} | "
        f"{pkg.get('Section','')} | {md_escape(what + extra_text)}{annot} |"
    )


def render_repo_section(state: dict, notes: dict) -> str:
    host  = state["host"]
    suite = state["suite"]
    key   = state["key"]
    key_line = (
        f"`0x{key['keyid'][-16:]}` — fingerprint `{key['fingerprint']}` "
        f"(rsa4096, {key.get('created','?')}, expires {key.get('expires','?')})"
        if key
        else "(unable to fetch key.gpg)"
    )
    # Partition packages by category.
    all_pkgs = []
    seen = set()
    for arch, lst in state["packages_by_arch"].items():
        for p in lst:
            ident = (p["Package"], p.get("Version"), p.get("Architecture"))
            if ident in seen:
                continue
            seen.add(ident)
            all_pkgs.append(p)
    all_pkgs.sort(key=lambda p: p["Package"])

    metapackages = [p for p in all_pkgs if p.get("Section") == "metapackages"]
    binaries     = [p for p in all_pkgs if p.get("Section") != "metapackages"]

    out = []
    out.append(f"## Repository: `{host}` — live\n")
    out.append("| | |")
    out.append("|---|---|")
    out.append(f"| **URL** | https://{host} |")
    out.append(f"| **Suite / Codename** | `{suite}` / `{state['release'].get('Codename', suite)}` |")
    out.append(f"| **Components** | `{state['release'].get('Components', 'main')}` |")
    out.append(f"| **Architectures** | `{state['release'].get('Architectures', '')}` |")
    out.append(f"| **Signing key** | {key_line} |")
    # Maintainer string has `<...>` which is a valid HTML tag in markdown;
    # backtick-wrap to render literally.
    out.append(f"| **Maintainer** | `{state['maintainer']}` |")
    out.append(f"| **Release date (snapshot)** | {state['release'].get('Date','')} |")
    out.append(f"| **Hosting** | Cloudflare R2 |")
    out.append("")

    # deb822 add-it block — derivable from suite + keyring filename.
    out.append("Add it with deb822:\n")
    out.append("```")
    out.append("Types: deb")
    out.append(f"URIs: https://{host}")
    out.append(f"Suites: {suite}")
    out.append(f"Components: {state['release'].get('Components', 'main')}")
    out.append(f"Signed-By: /etc/apt/keyrings/{state['keyring_filename']}")
    archs = " ".join(a for a in state['release'].get('Architectures','').split() if a != 'all')
    out.append(f"Architectures: {archs}")
    out.append("```\n")

    if metapackages:
        out.append(f"### Metapackages ({len(metapackages)})\n")
        out.append("| Package | Version | Arch | Depends |")
        out.append("|---|---|---|---|")
        for p in metapackages:
            out.append(render_metapackage_row(p, notes))
        out.append("")
        # Per-metapackage full-field dump. All fields from the Packages
        # index EXCEPT hash fields (noise on visual inspection) and
        # Description (handled separately after the stanza as a
        # collapsible <details> element, so the multi-line body doesn't
        # dominate the scroll).
        SKIP = {"MD5sum", "SHA1", "SHA256", "SHA512"}
        out.append(f"#### Metapackage stanzas — all fields\n")
        for p in metapackages:
            out.append(f"##### `{p['Package']}`\n")
            out.append("```")
            for k in p:
                if k in SKIP or k == "Description":
                    continue
                v = p[k]
                if "\n" in v:
                    first, *rest = v.split("\n")
                    out.append(f"{k}: {first}")
                    for line in rest:
                        out.append(f" {line}")
                else:
                    out.append(f"{k}: {v}")
            out.append("```")
            # Description: collapsible <details> below the stanza so the
            # one-liner is always visible but the long body is opt-in.
            if "Description" in p:
                desc = p["Description"]
                desc_summary, _, desc_body = desc.partition("\n")
                # Debian control format prefixes continuation lines with a
                # single space; strip it so the rendered body reads naturally.
                body_clean = "\n".join(
                    line[1:] if line.startswith(" ") else line
                    for line in desc_body.split("\n")
                ).strip()
                if body_clean:
                    out.append("")
                    out.append("<details>")
                    out.append(f"<summary><strong>Description:</strong> {md_escape(desc_summary)}</summary>")
                    out.append("")
                    # Indent inside <details> isn't required for markdown to
                    # render, but a blank line before + after lets paragraph
                    # detection work.
                    out.append(body_clean)
                    out.append("")
                    out.append("</details>")
                else:
                    out.append("")
                    out.append(f"**Description:** {md_escape(desc_summary)}")
            out.append("")
    if binaries:
        out.append(f"### Binary packages ({len(binaries)})\n")
        out.append("| Package | Version | Arch | Section | What it does |")
        out.append("|---|---|---|---|---|")
        for p in binaries:
            out.append(render_binary_row(p, notes))
        out.append("")
    return "\n".join(out)


DEP_FIELDS = ["Pre-Depends", "Depends", "Recommends", "Suggests"]


def parse_dep_names(val: str) -> list[str]:
    """Extract bare package names from a Depends/Recommends/etc field value.
    Strips version constraints, handles alternatives (|) and arch restrictions."""
    names = []
    for entry in re.split(r",\s*", val.replace("\n", " ")):
        for alt in entry.split("|"):
            alt = alt.strip()
            if not alt:
                continue
            m = re.match(r"([a-z0-9][a-z0-9.+\-]*)", alt)
            if m:
                names.append(m.group(1))
    return names


def _safe_id(name: str) -> str:
    """Mermaid node ID — replace chars that trip the parser."""
    return re.sub(r"[^a-zA-Z0-9_]", "_", name)


def render_dep_graph(states: list[dict]) -> str:
    """Dep-graph section: Mermaid chart (meta→meta only, stays renderable)
    + per-metapackage external-dep tables for the full picture.

    The Mermaid chart is scoped to inter-meta edges only so it stays under
    the ~2 KB URL limit of mermaid.ink.  External Ubuntu universe deps are
    listed in the per-package tables below it.
    """
    # Build lookup: name → "wf-meta" | "fl-meta" | "wf-bin" | "fl-bin"
    pkg_cat: dict[str, str] = {}
    meta_pkgs: list[tuple[str, dict]] = []  # (host, pkg_dict)

    for state in states:
        seen: set = set()
        is_wf = "worldfoundry" in state["host"]
        host  = state["host"]
        for lst in state["packages_by_arch"].values():
            for p in lst:
                ident = (p["Package"], p.get("Version"), p.get("Architecture"))
                if ident in seen:
                    continue
                seen.add(ident)
                name    = p["Package"]
                is_meta = p.get("Section") == "metapackages"
                cat     = ("wf" if is_wf else "fl") + ("-meta" if is_meta else "-bin")
                pkg_cat[name] = cat
                if is_meta:
                    meta_pkgs.append((host, p))

    # Collect ALL edges (src must be a metapackage)
    all_edges: list[tuple[str, str, str]] = []
    seen_ids: set = set()
    for state in states:
        for lst in state["packages_by_arch"].values():
            for p in lst:
                ident = (p["Package"], p.get("Version"), p.get("Architecture"))
                if ident in seen_ids:
                    continue
                seen_ids.add(ident)
                if p.get("Section") != "metapackages":
                    continue
                src = p["Package"]
                for field in DEP_FIELDS:
                    val = p.get(field, "")
                    if not val:
                        continue
                    for dep in parse_dep_names(val):
                        all_edges.append((src, dep, field))
    all_edges = list(dict.fromkeys(all_edges))

    if not all_edges:
        return ""

    our_pkgs  = set(pkg_cat.keys())
    meta_names = {n for n, c in pkg_cat.items() if c.endswith("-meta")}

    # ── Mermaid: meta→meta + meta→our-binary edges only ──────────────────
    graph_edges = [
        (s, d, f) for s, d, f in all_edges
        if s in meta_names and d in our_pkgs
    ]

    # meta→meta edges only (binary nodes in tables below)
    meta_edges = [
        (s, d, f) for s, d, f in all_edges
        if s in meta_names and d in meta_names
    ]
    all_meta_nodes = {n for e in meta_edges for n in (e[0], e[1])}

    out = []
    out.append("## Metapackage dependency graph\n")
    out.append(
        "`[WF]` = `apt.worldfoundry.org` · `[FL]` = `apt.foundrylinux.org`. "
        "Indented children are Depends unless noted otherwise.\n"
    )
    out.append("```")

    from collections import defaultdict as _dd
    adj: dict = _dd(list)
    for s, d, f in meta_edges:
        adj[s].append((d, f))

    targets = {d for _, d, _ in meta_edges}
    sources = {s for s, _, _ in meta_edges}
    roots   = sorted(
        (n for n in sources if n not in targets),
        key=lambda n: (pkg_cat.get(n, ""), n),
    )

    def _tree(node: str, indent: int = 0, visited: frozenset = frozenset()) -> list[str]:
        pfx  = "  " * indent + ("└── " if indent else "")
        repo = "WF" if "wf" in pkg_cat.get(node, "") else "FL"
        line = f"{pfx}{node}  [{repo}]"
        if node in visited:
            return [line + "  (↑ cycle)"]
        visited = visited | {node}
        lines = [line]
        for dep, field in sorted(adj.get(node, []), key=lambda x: x[0]):
            suffix = f"  ← {field}" if field != "Depends" else ""
            child  = _tree(dep, indent + 1, visited)
            child[0] += suffix
            lines.extend(child)
        return lines

    for root in roots:
        out.extend(_tree(root))
        out.append("")
    orphans = sorted(
        n for n in meta_names
        if n not in {s for s, _, _ in meta_edges} and n not in targets
    )
    if orphans:
        out.append("(no outgoing deps)")
        for n in orphans:
            repo = "WF" if "wf" in pkg_cat.get(n, "") else "FL"
            out.append(f"  {n}  [{repo}]")
    out.append("```\n")

    # ── Per-package dep tables ────────────────────────────────────────────
    out.append("### Per-metapackage dependency tables\n")
    out.append(
        "All declared relationships. "
        "**Bold** = package in our repos · plain = Ubuntu universe / external.\n"
    )
    meta_pkgs.sort(key=lambda hp: hp[1]["Package"])
    for host, p in meta_pkgs:
        name = p["Package"]
        if not any(p.get(f) for f in DEP_FIELDS):
            continue
        out.append(f"#### `{name}` (`{host}`)\n")
        out.append("| Relationship | Package |")
        out.append("|---|---|")
        for field in DEP_FIELDS:
            val = p.get(field, "")
            if not val:
                continue
            for dep in parse_dep_names(val):
                cell = f"**`{dep}`**" if dep in our_pkgs else f"`{dep}`"
                out.append(f"| {field} | {cell} |")
        out.append("")

    return "\n".join(out)


def render_phase_0_section(scripts: list[dict], notes: dict) -> str:
    if not scripts:
        return ""
    out = []
    out.append("## Phase 0: `foundry-setup/` installer scripts\n")
    out.append(
        "Bash installers that run on stock Ubuntu 26.04 — each is the apt-expansion "
        "of one would-be metapackage. All have `set -euo pipefail`, `-h/--help` "
        "short-circuit, lib.sh sourcing + inline shim fallback, `--dry-run` support.\n"
    )
    out.append("| Script | Install line(s) | Phase 1 target | Status |")
    out.append("|---|---|---|---|")
    stale_seen = []
    for s in scripts:
        n = notes.get(s["script"], {})
        installs = "<br/>".join(f"`{md_escape(x)}`" for x in s["installs"]) or "—"
        target = n.get("phase1_target", "")
        status = n.get("status", "ok")
        if "STALE" in status or "⚠️" in status:
            stale_seen.append((s["script"], status))
        out.append(f"| `{s['script']}` | {installs} | {md_escape(target)} | {md_escape(status)} |")
    out.append("")
    if stale_seen:
        out.append("**Drift:**")
        for name, st in stale_seen:
            out.append(f"- `{name}` — {st}")
        out.append("")
    return "\n".join(out)


def render_doc(states: list[dict], scripts: list[dict], notes: dict) -> str:
    today = dt.date.today().isoformat()
    out = []
    out.append("---")
    out.append("title: Foundry Linux & WorldFoundry package inventory")
    out.append(f"date: {today}")
    out.append("status: auto-generated by scripts/audit-apt-repos.sh")
    out.append("---\n")
    out.append("# Package inventory\n")
    out.append(
        "Auto-generated snapshot of both Foundry/WorldFoundry apt repos + Phase 0 "
        "installer scripts. Regenerate via `task audit-apt-repos` or "
        "`bash scripts/audit-apt-repos.sh`.\n"
    )
    out.append("| Repository | Status | Scope |")
    out.append("|---|---|---|")
    for st in states:
        out.append(f"| **`{st['host']}`** | ✅ live (Cloudflare R2, signed) | {st['scope']} |")
    out.append("")
    out.append(
        "Plus **Phase 0** bash installer scripts in `foundry-setup/` that bridge "
        "stock Ubuntu to a WF-ready dev box until Phase 1's apt repos cover everything.\n"
    )

    for st in states:
        out.append(render_repo_section(st, notes))
        out.append("---\n")
    if scripts:
        out.append(render_phase_0_section(scripts, notes))
        out.append("---\n")
    dep_graph = render_dep_graph(states)
    if dep_graph:
        out.append(dep_graph)
        out.append("---\n")
    out.append(CROSS_REPO_DIAGRAM)
    out.append("---\n")
    out.append(CONVENTIONS_RECAP)
    out.append("---\n")
    out.append(SEE_ALSO)
    return "\n".join(out)


# ─── main ───────────────────────────────────────────────────────────────

states = [collect_repo(r) for r in REPOS]
scripts = scan_foundry_setup()
notes   = load_notes(NOTES)
sys.stdout.write(render_doc(states, scripts, notes))
PYEOF
