# Plan: document how to consume the source packages (deb-src / apt-get source)

## Context

v1.5.36 made `apt.foundrylinux.org` ship a complete source index (53/53 packages, verified
`apt-get source` round-trips). But **no consumer-facing instruction mentions it** — the apt
index, the live site, and the Phase 0 setup script all show only the binary `deb …` line. A
user/packager can't `apt-get source <pkg>` without the `deb-src` line, which is documented
nowhere. Close that gap on the two surfaces where it belongs; leave the marketing site alone
(its audience is game devs running `sudo apt install`, not fetching Debian source).

## Scope

1. **apt index** (`foundry-apt/scripts/generate-index.sh`) — add a **"Source packages"**
   section: the `deb-src` line + an `apt-get source` example. Goes live on the next publish
   (the index regenerates in CI).
2. **Phase 0** (`foundry-setup/setup-foundry-apt-source.sh`) — add an opt-in **`--with-source`**
   flag that also writes the `deb-src` line. Off by default (keeps installs lean / `apt update`
   faster).
3. **Not** the marketing site (`site/*.jsx`) — wrong audience.

## Implementation

### 1. `generate-index.sh` — "Source packages" section

Insert after the existing **"GPG key"** `<h2>` block (before **"Packages"**), mirroring the
"Quick install" pre-block + copy-button style already there. The keyring path + `${SITE_URL}`
match the binary snippet:

```html
<h2>Source packages</h2>
<p>Every package is published with source. Add the deb-src line, then fetch &amp; unpack any
package's Debian source:</p>
<pre id="source-setup">echo "deb-src [signed-by=/etc/apt/keyrings/foundry.gpg] ${SITE_URL} resolute main" \
  | sudo tee -a /etc/apt/sources.list.d/foundry.list
sudo apt-get update
apt-get source bsnes-jg</pre>
```
(Use `tee -a` to append, not clobber the binary line. Add a copy-button matching the other
`pre` blocks for consistency.)

### 2. `setup-foundry-apt-source.sh` — `--with-source` flag

- Add `WITH_SOURCE=false`; parse `--with-source) WITH_SOURCE=true`.
- Document it in the `--help` usage.
- **Fix the idempotency early-exit (line 57)** so `--with-source` on an already-configured box
  still appends `deb-src`: skip only when `! $WITH_SOURCE` *or* the list already has a
  `^deb-src ` line; otherwise fall through.
- Guard the key fetch with `[[ -f "$KEYRING" ]] ||` so the re-run path doesn't re-dearmor.
- Write the list as one block — always the `deb` line, plus the `deb-src` line when
  `$WITH_SOURCE`:
  ```bash
  {
      echo "deb [signed-by=$KEYRING] $URL $SUITE main"
      $WITH_SOURCE && echo "deb-src [signed-by=$KEYRING] $URL $SUITE main"
  } | run_sudo tee "$LIST" > /dev/null
  ```
- Update the closing `ok` line to note when deb-src was added.

## Verification

1. **Index** — `bash foundry-apt/scripts/generate-index.sh` (or inspect the emitted
   `public/index.html`) shows the new "Source packages" block with the right `deb-src` line +
   `apt-get source` example; HTML still well-formed.
2. **Phase 0 dry-run** — `setup-foundry-apt-source.sh --with-source --dry-run` prints both a
   `deb` and a `deb-src` tee; without the flag, only `deb`. `--help` lists `--with-source`.
   `shellcheck` clean.
3. **Real round-trip** (container) — run `setup-foundry-apt-source.sh --with-source` in
   `ubuntu:26.04`, then `apt-get update` + `apt-get source bsnes-jg` unpacks (proves the
   generated `deb-src` line is correct end to end). Re-run the script → idempotent (deb-src not
   duplicated).
4. **Default unchanged** — without `--with-source`, the list has only the `deb` line (no
   regression to existing installs).

## Execution log — 2026-06-21 ✅ done

Both surfaces implemented; marketing site left alone (audience). Verified in `ubuntu:26.04`.

- **`generate-index.sh`** — added the "Source packages" `<h2>` block (the `deb-src` line +
  `apt-get source bsnes-jg` example, copy-button matching the others). Goes live on the next
  publish. shellcheck clean.
- **`setup-foundry-apt-source.sh`** — `--with-source` flag (documented in `--help`); the
  idempotency early-exit now falls through to append `deb-src` when asked + the list lacks it;
  key fetch guarded with `[[ -f "$KEYRING" ]] ||`; list written as one block (`deb` + optional
  `deb-src`); dry-run prints the lines.
- **Verified:** `--help` lists the flag; `--with-source --dry-run` shows both lines; plain
  `--dry-run` idempotent-skips an already-configured box. **Real round-trip:** `--with-source`
  in a fresh `ubuntu:26.04` wrote both lines → `apt-get source bsnes-jg` unpacked end-to-end;
  re-run idempotent (one `deb-src` line). shellcheck: only the pre-existing `lib.sh` SC1091
  (info), same as every `foundry-setup` script.

## Follow-up (note, not this change)

The `new-web-apt-repo` skill's `gen/gen-index.py` + its `publish-local.sh` template now build
source packages (skill update earlier) — its generated index should grow the same
"Source packages" block so *new* repos document it too. Add a TODO. (apt.worldfoundry.org
itself stays binary-only until Phase 2 is applied there, so its index correctly omits it.)
