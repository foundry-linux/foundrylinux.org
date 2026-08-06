# ITP draft: halfempty

## Upstream packaging audit

Audited the pinned upstream payload on 2026-08-05 with
`foundry-apt/scripts/audit-upstream-packaging.sh`. **No upstream packaging signals found:** no
upstream-maintained `debian/`, `.deb`/`.dsc` builder, `PKGBUILD`/`.spec`, or PPA/Launchpad/OBS/COPR
packaging reference.

## wnpp check

Searched wnpp for "halfempty" (Google Project Zero test-case minimizer). **No existing wnpp bug found** — the one near-hit ("half" — a C++ half-precision float library, bug #1049960) is an unrelated package with a similar name.

## ITP draft

```
Package name    : halfempty
Version         : 0.40
Upstream contact: Tavis Ormandy <taviso@gmail.com>
URL             : https://github.com/googleprojectzero/halfempty
Licence         : Apache-2.0
Programming lang: C
Description     : fast, parallel file bisection / test-case minimizer
```

halfempty (by Tavis Ormandy / Google Project Zero) reduces a large input
file to a minimal one that still satisfies a test, using a lazy,
parallel bisection algorithm. The user supplies a script that exits
successfully while the input is still "interesting" (e.g. still
triggers a crash) plus the file to minimize. Unlike source-aware
reducers such as cvise and creduce, halfempty treats the input as an
opaque byte stream, so it can minimize any file — source, binary blobs,
ROMs, crash inputs, fuzzer output — and parallelizes aggressively for
speed.

Debian already packages `cvise` and (per our own notes) `creduce`
predecessors, but nothing that minimizes arbitrary binary input the way
halfempty does. Given Google Project Zero's reach and halfempty's
established use in the fuzzing/security-research community, this is a
solid, well-scoped ITP with an unambiguous license and no unusual build
requirements.

## Notes

- Single license (Apache-2.0), DFSG-clean, from a well-known, credible
  upstream (Google Project Zero) — low risk on the licensing/provenance
  front.
- Build-Depends: `pkg-config`, `libglib2.0-dev`, `bsdextrautils` — all
  standard Debian main packages.
