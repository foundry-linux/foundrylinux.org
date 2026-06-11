# Plan: Vendor abfs.pdf + post to wla-dx issue #589

## Context

The Amiga Binary File Structure (ABFS) specification — [http://amiga.rules.no/abfs/abfs.pdf](http://amiga.rules.no/abfs/abfs.pdf) — is dead. It is the only content of [vhelin/wla-dx issue #589](https://github.com/vhelin/wla-dx/issues/589) ("Add support for Amiga's (executable) binary file structure"). The Wayback Machine has an archived copy; we have a confirmed download at `/tmp/abfs.pdf`.

- **Title**: Amiga Binary File Structure
- **Compiler**: Paul René Jørgensen, last updated 19 April 2005
- **Copyright**: © 1991 Commodore Electronics Ltd · © 1990 Mikael Karlsson · © 2005 Paul René Jørgensen
- **Size**: 29 pages, 109 008 bytes
- **SHA256**: `b8e4916a86278499ab80712b414a39c9702f8a663135de49cd4b93c8b6b18d85`
- **Archive source**: [https://web.archive.org/web/20201101014627if_/http://amiga.rules.no/abfs/abfs.pdf](https://web.archive.org/web/20201101014627if_/http://amiga.rules.no/abfs/abfs.pdf)

## Steps

1. **Vendor the PDF** — copy `/tmp/abfs.pdf` to `foundry-apt/packages/wla-dx/docs/abfs.pdf` and commit it.

2. **Comment on wla-dx issue #589** — post a comment with the Wayback Machine URL so the upstream project has a working reference. The `gh` CLI can post comments but cannot attach binaries; the Wayback Machine link is sufficient.

## Verification

- `ls -lh foundry-apt/packages/wla-dx/docs/abfs.pdf` — confirms file present at expected size (~106 KiB).
- `gh api repos/vhelin/wla-dx/issues/589/comments --jq '.[].body'` — confirms our comment posted.
