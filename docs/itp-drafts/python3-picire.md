# ITP draft: python3-picire

## Upstream packaging audit

Audited the pinned upstream payload on 2026-08-05 with
`foundry-apt/scripts/audit-upstream-packaging.sh`. **No upstream packaging signals found:** no
upstream-maintained `debian/`, `.deb`/`.dsc` builder, `PKGBUILD`/`.spec`, or PPA/Launchpad/OBS/COPR
packaging reference.

## wnpp check

Searched wnpp for "picire" (parallel Delta Debugging, Renata Hodovan / Akos Kiss). **No existing wnpp bug found** — search returned unrelated ITPs (deltachat-core, xdelta3-python, python-picologging) and a Wikipedia page on delta debugging generally, nothing for picire itself.

## ITP draft

```
Package name    : python3-picire
Version         : 21.8
Upstream contact: Renata Hodovan <hodovan@inf.u-szeged.hu>
URL             : https://github.com/renatahodovan/picire
Licence         : BSD-3-Clause
Programming lang: Python 3
Description     : parallel Delta Debugging framework — automatic test-case reducer
```

picire is a Python implementation of the (parallel) Delta Debugging
algorithm. Given a failing input and a test command that decides
whether a candidate is still "interesting" (e.g. still triggers a
crash), picire repeatedly removes chunks and keeps the smallest variant
that still fails — producing a minimal reproducer. Invaluable when
filing compiler, interpreter, or tooling bug reports. Unlike
cvise/creduce (C/C++-aware), picire is language-agnostic and reduces any
line- or character-delimited input. Ships the `picire(1)` CLI.

Debian already has `cvise` and `creduce` for source-aware reduction, but
nothing language-agnostic in this space. picire fills that gap and pairs
naturally with `halfempty` (also in this batch) for a complete
minimization toolchain covering both structured and opaque-binary
reduction use cases.

## Dependency note

**Depends on `python3-inators`** (also drafted in this batch — see
`python3-inators.md`). File inators first, or in the same pass, so
picire's ITP isn't blocked on a missing build/runtime dependency.

## Notes

- Single license (BSD-3-Clause), DFSG-clean.
- Our own `build.sh` notes quilt patches are carried
  (`Copies our debian/ tree into the upstream tree (incl. quilt
  patches)`) — worth reviewing what those patches do before using them
  as a starting point for the real Debian package, since Debian may
  prefer patches be pushed upstream rather than carried indefinitely.
