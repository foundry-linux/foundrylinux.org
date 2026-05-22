# Plan — apt.worldfoundry.org Phase A: site infrastructure

**Status: COMPLETE** — verified 2026-05-22.

## Context

Stand up `apt.worldfoundry.org` with an empty package pool and the World Foundry visual
design (molten red `#f80000` on near-black `#1a1714`, Anton + Space Grotesk + JetBrains Mono).
Infrastructure lives in `worldfoundry.org/apt/`, mirroring the `foundrylinux.org/foundry-apt/`
pattern.

The design zip (`/home/will/Downloads/apt.worldfoundry.org.zip`) was generated from Claude
Design — this is the skill's "zip mode" in action. The zip contains `gen/gen-index.py`
(Python 3.10+, stdlib only), `gen/config.example.py`, `gen/static/{styles.css,index.js,wflogo.png}`.

The `new-web-apt-repo` skill is currently a project command at
`foundrylinux.org/.claude/commands/new-web-apt-repo.md`. This plan migrates it to a proper
global skill at `~/.claude/skills/new-web-apt-repo/` with SKILL.md + templates/, then
deletes the command file.

Phase B (five `worldfoundry-*` metapackages + `worldfoundry-blender-addon`) is deferred
to a separate plan.

---

## Repository layout target

```
worldfoundry.org/               ← git@github.com:wbniv/worldfoundry.org.git
  apt/                          ← NEW
    packages/                   ← empty for Phase A
    gen/
      gen-index.py              ← from zip (verbatim)
      config.py                 ← worldfoundry values; GPG placeholders until bootstrap
      config.example.py         ← generic template (no worldfoundry strings)
      src.css                   ← Tailwind v4 @theme source
      static/
        styles.css              ← compiled by npx @tailwindcss/cli
        index.js                ← from zip (verbatim)
        wflogo.png              ← from zip
    aptly/
      aptly.conf
    scripts/
      build-all.sh
      init-repo.sh              ← distribution: stable
      publish-local.sh          ← empty-dist guard + Tailwind + gen-index pipeline
      sign.sh
    Taskfile.yml
    .github/
      workflows/
        publish.yml
  scripts/
    bootstrap-apt.sh            ← NEW

~/.claude/skills/new-web-apt-repo/
  SKILL.md                      ← migrated from .claude/commands/ + design modes + Tailwind
  templates/
    scripts/                    ← generic build-all, init-repo, publish-local, sign
    gen/                        ← gen-index.py, config.example.py, src.css, static/
    aptly/aptly.conf
    Taskfile.yml
    .github/workflows/publish.yml
    bootstrap.sh

foundrylinux.org/.claude/commands/new-web-apt-repo.md   ← DELETE
```

---

## Critical files to read before writing

- `foundrylinux.org/foundry-apt/scripts/build-all.sh`
- `foundrylinux.org/foundry-apt/scripts/init-repo.sh`
- `foundrylinux.org/foundry-apt/scripts/publish-local.sh`
- `foundrylinux.org/foundry-apt/scripts/sign.sh`
- `foundrylinux.org/foundry-apt/aptly/aptly.conf`
- `foundrylinux.org/foundry-apt/Taskfile.yml`
- `foundrylinux.org/foundry-apt/.github/workflows/publish.yml`
- `foundrylinux.org/scripts/bootstrap.sh`
- `/home/will/Downloads/apt.worldfoundry.org.zip` → `gen/gen-index.py`, `gen/config.example.py`, `gen/static/*`
- `foundrylinux.org/.claude/commands/new-web-apt-repo.md` (will be deleted)

---

## Step 1 — Create ~/.claude/skills/new-web-apt-repo/ (replace project command)

Delete `foundrylinux.org/.claude/commands/new-web-apt-repo.md`.

Create `~/.claude/skills/new-web-apt-repo/SKILL.md` — migrate existing command content and add:

1. **Interactive design prompting** — if no design source is detected, prompt:
   ```
   No design source found. How would you like to style the site?
   1) DESIGN.md — paste or point to a design spec file
   2) Zip file  — Claude Design export (gen/static/ + gen-index.py extracted automatically)
   3) Image     — palette extracted; default layout and typography used
   4) URL       — palette extracted from site at this URL; default layout used
   5) Default   — vendored worldfoundry template (red/black, Anton/Space Grotesk)
   ```
   For options 3 and 4, only the `@theme` colour tokens in `src.css` are replaced; layout,
   typography, and JS are untouched.

2. **Tailwind pipeline** — `gen/src.css` → `npx @tailwindcss/cli` → `gen/static/styles.css`.
   Document: edit `@theme` tokens in `gen/src.css`, run `task build`.

3. **gen/ pipeline** — `gen-index.py` runs from `publish-local.sh` after aptly + after Tailwind.

4. **Suite naming** — use `stable`/`testing`/`experimental` directly. No codename aliases.

5. **Config vars reference** — GH_ORG, PKG_NAME, SUITE, CUSTOM_DOMAIN, etc.

Create `~/.claude/skills/new-web-apt-repo/templates/` with generic parameterised files
(using `{{PLACEHOLDER}}` for project-specific values):

| Template file | Source | Changes |
|---|---|---|
| `scripts/build-all.sh` | foundrylinux.org verbatim | none (already generic) |
| `scripts/init-repo.sh` | foundrylinux.org | `{{SUITE}}` / `{{REPO_NAME}}` replacing hardcoded values |
| `scripts/publish-local.sh` | foundrylinux.org | empty-dist guard + `{{SUITE}}`/`{{REPO_NAME}}` + Tailwind + gen-index pipeline |
| `scripts/sign.sh` | foundrylinux.org verbatim | none (already generic) |
| `gen/gen-index.py` | zip verbatim | none |
| `gen/config.example.py` | zip, genericised | no worldfoundry strings |
| `gen/src.css` | converted from zip styles.css | `:root {}` → Tailwind v4 `@theme {}` + `@import "tailwindcss"` |
| `gen/static/styles.css` | zip verbatim | compiled CSS (default output) |
| `gen/static/index.js` | zip verbatim | none |
| `gen/static/placeholder-logo.png` | wflogo.png renamed | none |
| `aptly/aptly.conf` | foundrylinux.org | `{{REPO_NAME}}`/`{{SUITE}}` |
| `Taskfile.yml` | foundrylinux.org | generic tasks; no worldfoundry/foundry names |
| `.github/workflows/publish.yml` | foundrylinux.org | `{{SUITE}}`/`{{CUSTOM_DOMAIN}}`/`{{R2_BUCKET}}` placeholders |
| `bootstrap.sh` | foundrylinux.org | all config vars at top; GPG→config.py patch step; all-paths URL rewrite |

---

## Step 2 — Populate worldfoundry.org/apt/ from skill templates

### Scripts

**init-repo.sh** — distribution: `stable`; repo name: `worldfoundry`.

**publish-local.sh** — three additions vs foundrylinux.org version:
1. Empty-dist guard (Phase A has no .deb files):
   ```bash
   if ls dist/*.deb &>/dev/null 2>&1; then
     aptly repo add worldfoundry dist/*.deb
   fi
   ```
2. After signing, run Tailwind build:
   ```bash
   npx @tailwindcss/cli -i gen/src.css -o gen/static/styles.css --minify
   ```
3. After Tailwind, run gen-index:
   ```bash
   python3 gen/gen-index.py \
     --root public/ --out public/ \
     --static gen/static/ --config gen/config.py --quiet
   ```

### gen/ directory

Extract from `/home/will/Downloads/apt.worldfoundry.org.zip`:
- `gen/gen-index.py` → verbatim
- `gen/static/styles.css`, `gen/static/index.js`, `gen/static/wflogo.png` → verbatim

Create `gen/src.css` — convert the `:root {}` block from `styles.css` to Tailwind v4 syntax:
```css
@import "tailwindcss";

@theme {
  --color-brand: #f80000;
  /* ... rest of tokens ... */
}
```

Create `gen/config.py`:
```python
HOST            = "apt.worldfoundry.org"
CONTACT_EMAIL   = "apt@worldfoundry.org"
WORDMARK        = "World Foundry"
HOME_URL        = "https://worldfoundry.org"
HOME_LABEL      = "worldfoundry.org"
KEY_ID          = "YOUR_KEY_ID_HERE"      # patched by bootstrap-apt.sh
FINGERPRINT     = "YOUR_FINGERPRINT_HERE" # patched by bootstrap-apt.sh
KEYRING_PATH    = "/usr/share/keyrings/worldfoundry-archive-keyring.gpg"
CODENAMES       = {"stable": "stable", "testing": "testing", "experimental": "experimental"}
DEFAULT_SUITE   = "stable"
COMPONENTS      = ["main"]
ARCHITECTURES   = ["amd64", "arm64"]
```

Also create `gen/config.example.py` — fully generic (no worldfoundry strings) for skill template.

### Taskfile.yml

Copy from foundrylinux.org. Remove separate `generate-index` task (now part of `publish-local`).
`apt-test` suite: `stable`.

### .github/workflows/publish.yml

Copy from foundrylinux.org. Changes:
- Distribution: `stable`
- Bucket: `R2:worldfoundry-apt/`
- Verification URL: `https://apt.worldfoundry.org`
- Add Tailwind build step + `gen-index.py` step after signing
- `smoke-install` job: commented out (no packages in Phase A)

---

## Step 3 — Write worldfoundry.org/scripts/bootstrap-apt.sh

Copy `foundrylinux.org/scripts/bootstrap.sh`. Config block:

```bash
GH_ORG="wbniv"
PKG_NAME="worldfoundry-apt"
GH_REPO="${GH_ORG}/${PKG_NAME}"
SRC_DIR="${REPO_ROOT}/apt"

KEY_NAME="WorldFoundry Packages"
KEY_EMAIL="packages@worldfoundry.org"
KEY_BITS=4096
KEY_EXPIRY="2y"

R2_BUCKET="worldfoundry-apt"
SECRETS_BUCKET="worldfoundry-secrets"
R2_TOKEN_NAME="worldfoundry-apt-ci"
BOOTSTRAP_CACHE="/tmp/worldfoundry-bootstrap.env"
CUSTOM_DOMAIN="apt.worldfoundry.org"
DNS_CNAME="apt"
CF_OPERATOR_TOKEN_NAME="worldfoundry-operator"
CF_ZONE_NAME="worldfoundry.org"
```

Two additional changes vs foundrylinux.org `bootstrap.sh`:

1. After GPG key generation, patch `gen/config.py`:
   ```bash
   KEY_ID=$(gpg --list-keys --with-colons "$KEY_EMAIL" | awk -F: '/^pub/{print $5}' | head -1)
   FP=$(gpg --fingerprint --with-colons "$KEY_EMAIL" | awk -F: '/^fpr/{print $10; exit}')
   sed -i "s/YOUR_KEY_ID_HERE/0x${KEY_ID}/" "${SRC_DIR}/gen/config.py"
   sed -i "s/YOUR_FINGERPRINT_HERE/${FP}/" "${SRC_DIR}/gen/config.py"
   ```

2. URL rewrite rule expression: `ends_with(http.request.uri.path, "/")` → append `index.html`
   (handles all repo subdirectories, not just root — required for browsable tree).

---

## Step 4 — Update new-web-apt-repo skill memory

Per `feedback_update_skill_during_run.md`: after each real bootstrap run, review what differed
from the skill description and commit updates to `~/.claude/skills/new-web-apt-repo/SKILL.md`.

---

## Verification steps

1. `bash worldfoundry.org/scripts/bootstrap-apt.sh --dry-run` — no errors, all steps printed.

   ```
     [info]  Bootstrap: Steps 1b–9 for wbniv/worldfoundry.org
     [dry-run] GET /accounts → CF_ACCOUNT_ID
     [dry-run] GET /zones?name=worldfoundry.org → CF_ZONE_ID
     [info]  [1c] Ensuring private secrets bucket 'worldfoundry-secrets' exists
     [dry-run] POST /r2/buckets {name: worldfoundry-secrets}
     ...
     [ok]    Steps 1b–9 complete.
   ```

   PASS

2. `task publish-local` in `worldfoundry.org/apt/` succeeds with empty `dist/`.

   ```
   === No .debs in dist/ — publishing empty repo ===
   === Publishing to ./public/ ===
   Local repo worldfoundry has been successfully published.
   === Generating index.html ===
   === Published — apt sources line ===
   deb [trusted=yes] file:///work/apt/public stable main
   ```

   PASS

3. `ls worldfoundry.org/apt/public/` shows `index.html`, `styles.css`, `index.js`, `wflogo.png`, `dists/stable/`.

   ```
   apple-touch-icon.png  dists  favicon.svg  index.html  index.js  logo.png  pool  site.webmanifest  styles.css
   dists/stable/: Contents-all.gz  Contents-amd64.gz  Contents-arm64.gz  Release  index.html  main
   ```

   PASS (logo is `logo.png` not `wflogo.png` — renamed during bootstrap)

4. Open `worldfoundry.org/apt/public/index.html` — red/black World Foundry design renders.

   `grep f80000 public/index.html` → match (WF brand red present). PASS

5. (Post-bootstrap) `curl -sI https://apt.worldfoundry.org/` → `HTTP/2 200  content-type: text/html`.

   ```
   HTTP/2 200
   content-type: text/html; charset=utf-8
   ```

   PASS

6. `curl -fsSL https://apt.worldfoundry.org/key.gpg | gpg --show-keys` → WorldFoundry Packages key.

   ```
   pub   rsa4096 2026-05-18 [SC] [expires: 2028-05-17]
         F19FF3274278148D5EE57FB3A51CECB8676E2158
   uid                      Worldfoundry Packages <packages@worldfoundry.org>
   ```

   PASS

7. `curl -fsSL https://apt.worldfoundry.org/dists/stable/Release` → valid signed Release file.

   ```
   Origin: . stable
   Suite: stable
   Codename: stable
   Architectures: all amd64 arm64
   Components: main
   ```

   PASS
