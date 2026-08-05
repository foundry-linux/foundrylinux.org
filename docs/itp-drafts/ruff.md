# ITP draft: ruff

## wnpp check — EXISTING ITPs FOUND, do not file

Two prior/active ITPs exist for ruff:

- [Bug #1030835](https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=1030835) — "ITP: ruff -- linter for Python, written in Rust", filed by James Addison, February 2023, for version 0.0.243. Describes it as including implementations of many common checks from flake8, flake8 plugins, and pylint.
- [Bug #1088706](https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=1088706) — "ITP: ruff -- An extremely fast Python linter", filed by Ranjith Raj, November 2024, for **`python3-ruff` version 0.8.0**, under the **Debian Python Team**.
- Related: [Bug #1054205](https://bugs.debian.org/1054205) — "ITP: python-lsp-ruff -- Ruff linting plugin for pylsp" — a downstream consumer package, further evidence of live Debian interest in the ruff ecosystem.
- Also relevant: a prior **RFP** was marked done — "RFP: ruff -- an extremely fast Python linter and code formatter, written in Rust" — suggesting the request-for-packaging stage is already closed out in favor of the ITP(s) above.

**Do not file a new ITP.** The most recent one (#1088706, under the
Debian Python Team, named `python3-ruff` rather than plain `ruff`) looks
like the live effort. Note the **package-naming difference**: Debian's
effort names it `python3-ruff`, while our own Foundry package is named
plain `ruff` (matching upstream's own binary name and PyPI package name,
since ruff ships as a standalone Rust binary with no Python runtime
dependency — see our `debian/control`'s `Depends: ${shlibs:Depends}`,
not `${python3:Depends}`). Worth watching whether Debian's effort lands
as `python3-ruff` or gets renamed to plain `ruff`; the naming convention
matters if we ever want to depend on the Debian-shipped version instead
of our own.

## ITP draft (kept for reference only — not to be filed)

```
Package name    : ruff  [Debian effort in progress as "python3-ruff", see above]
Version         : 0.15.15
Upstream contact: Astral Software <support@astral.sh>
URL             : https://docs.astral.sh/ruff
Licence         : MIT
Programming lang: Rust
Description     : extremely fast Python linter and code formatter
```

Ruff is an extremely fast Python linter and code formatter, written in
Rust. It can replace Flake8 (plus dozens of plugins), Black, isort,
pyupgrade, and more, running 10-100x faster than each individual tool.
Supports linting, formatting, import sorting, and type-annotation
upgrades in a single binary with no Python runtime dependency.

## Notes

- Single license (MIT), DFSG-clean — not the blocker here, duplication
  with active Debian Python Team work is.
- Worth reaching out to the #1088706 filer/Debian Python Team directly
  if there's a strong internal need to accelerate this, rather than
  duplicating effort.
