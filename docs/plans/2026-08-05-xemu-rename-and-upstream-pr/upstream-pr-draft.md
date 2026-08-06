# DRAFT upstream PR — replace `build/deb-build-simple.sh` with a Debian source package

> ## ⛔ DO NOT POST — user approval required
>
> Target: [lgblgblgb/xemu](https://github.com/lgblgblgb/xemu). Posting is user-triggered, same rule as the
> Debian ITP and the llvm-mos review comment.
>
> **Prerequisite:** send the upstream heads-up first (see
> [`docs/itp-drafts/xemu.md`](../../itp-drafts/xemu.md) → "Before filing: give upstream a heads-up"). This
> PR lands better as a follow-up to a conversation than as a surprise 20-file diff.
>
> **Command, once approved:**
>
> ```bash
> gh repo fork lgblgblgb/xemu --clone --remote
> cd xemu && git checkout -b debian-source-package
> cp -a /home/will/foundrylinux.org/foundry-apt/packages/xemu/debian .
> # drop the two Foundry-only bits: patches/ (already upstream as #448) and
> # the vendored xemu.xpm (upstream already ships build/xemu-48x48.xpm)
> rm -rf debian/patches debian/xemu.xpm
> git add debian && git commit -m "build: add a Debian source package"
> git push -u origin debian-source-package
> gh pr create --title "build: add a proper Debian source package" --body-file <this file, body only>
> ```

---

<!-- PR BODY BELOW -->

Hi — following up on #448.

While packaging Xemu for a small Debian derivative I ended up writing a full `debian/` source tree for it,
and it seemed worth offering back rather than keeping downstream. This replaces `build/deb-build-simple.sh`
with a standard Debian source package.

I want to be upfront that your script already made several decisions correctly, and this keeps them:

- **The `.desktop` generation approach is yours, and it is better than what I originally wrote.** I had
  seven hand-written `.desktop` files; you generate them from `build/xemu.desktop` plus `PRG_TARGET` /
  `EMU_DESCRIPTION` in each `targets/*/Makefile`. Yours picks up new targets automatically and uses your
  own machine descriptions, so I threw mine away and ported yours into `debian/rules`.
- **Not shipping a ROM downloader.** Your comment explains why ("a legality problem to ship the package
  with a helper inside which downloads ROM images copyrighted by some angry companies"). Agreed — no ROMs,
  no fetcher.
- The icon at `/usr/share/pixmaps/xemu-48x48.xpm`, which your `.desktop` template points `Icon=` at.

What the source package adds over the binary-level build:

| | `deb-build-simple.sh` | `debian/` |
|---|---|---|
| Source package (`.dsc`) | none — the `README.Debian` it writes says so | yes, `apt-get source` works |
| `Depends:` | hand-maintained, with a `dpkg -s libreadline-dev` shell-out at build time | resolved by `dh_shlibdeps` from the actual linked sonames |
| Version | `cdate` (`20260129235930`) | a normal Debian version |
| Changelog | `git log --max-count=100` | a real `debian/changelog` |
| Copyright | LICENSE concatenated | machine-readable DEP-5 |
| Man pages | none | one per shipped binary (Debian Policy §12.1 requires them) |
| Hardening | none | PIE, relro, bindnow via `dpkg-buildflags` |
| Reproducible | build timestamp baked in | honours `SOURCE_DATE_EPOCH` (that is #448) |
| `lintian` | not clean | clean on both the `.deb` and the `.dsc` |

Two implementation notes that might be of interest regardless of whether you take this:

1. **Hardening flags cannot be passed on the `make` command line.** `build/Makefile.common` assigns
   `CFLAGS` with a plain `=`, so a command-line `CFLAGS=` clobbers the flags the build needs. `debian/rules`
   instead runs `build/configure/configure` itself and appends `dpkg-buildflags` output to the generated
   `config-native.make`, which is `include`d before `CONFIG_CFLAGS` is used.
2. **`make clean` leaves the configure probe's scratch files** (`build/objs/configure--script--native--test.*`)
   and `cdate.data` behind, which makes `dpkg-source -b` fail with *"unrepresentable changes to source"*.
   `debian/rules` removes everything in `build/objs` and `build/bin` that is not tracked. You may want that
   in `make clean` itself.

Also worth knowing, since it is invisible from your side: **there is an unrelated original-Xbox emulator
also called "xemu"** ([xemu.app](https://xemu.app)), and it is now packaged in some distros. Because your
script emits `Package: xemu` and versions it with a 14-digit `cdate`, a user who installs both gets a
silent replacement — your build always outranks theirs, so the Xbox emulator disappears. Nothing for you to
fix necessarily, but if you ever want to defend against it, qualifying the name in `deb-build-simple.sh`
would do it. (In our own repo we resolved it the other way round: your project keeps `xemu`, since it has
had that package name since 2016, and the Xbox one is `xemu-xbox`.)

`debian/patches/` is intentionally absent from this PR — the only patch we carry is the
`SOURCE_DATE_EPOCH` fix already proposed in #448.

Happy to adjust anything here, split it up, or drop it entirely if you would rather keep the simple script.
