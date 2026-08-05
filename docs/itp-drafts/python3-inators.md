# ITP draft: python3-inators

## wnpp check

Searched wnpp for "inators" (Renata Hodovan's utility library). **No existing wnpp bug found.**

## ITP draft

```
Package name    : python3-inators
Version         : 2.1.1
Upstream contact: Renata Hodovan <hodovan@inf.u-szeged.hu>
URL             : https://github.com/renatahodovan/inators
Licence         : BSD-3-Clause
Programming lang: Python 3
Description     : small utility helpers for the inators test-case-reduction tools
```

inators is a tiny pure-Python utility library (argument-parsing helpers,
logging shims, import utilities) shared by Renata Hodovan and Akos
Kiss's test-case-reduction tooling — most notably `picire`, the parallel
Delta Debugging framework (also queued in this ITP batch). It has no
runtime dependencies of its own.

**File this together with (or immediately before) `python3-picire`** —
picire depends on inators, so filing inators first (or in the same
sponsorship pass) avoids blocking picire's own ITP on a missing
dependency. See `python3-picire.md` in this batch.

## Notes

- Single license (BSD-3-Clause), DFSG-clean.
- Architecture `all` (pure Python), minimal build complexity — one of
  the simplest ITPs in this batch.
- Recommend maintaining under the Debian Python Team umbrella given
  `python3-librosa` and `ruff`'s existing ITPs (see their drafts) are
  already tracked there — consistency with sibling packages simplifies
  review.
