# Package publication includes wald3n.com/open-source

Publishing a new or updated Foundry APT package is not complete when the APT repository alone is
live. The package inventory rendered at `https://wald3n.com/open-source` is a checked-in snapshot in
the sibling `~/wald3n.com` repository and must be refreshed and deployed as part of the same release
process.

Required completion sequence:

1. Publish the package to `apt.foundrylinux.org`.
2. Verify the exact live package version installs successfully.
3. In a clean `~/wald3n.com` worktree, run `task open-source:refresh`.
4. Review the generated `src/data/open-source.json` diff; do not sweep unrelated wald3n.com changes
   into the inventory commit.
5. Commit and deploy the wald3n.com refresh using its normal publication workflow.
6. Verify the package appears on `https://wald3n.com/open-source`.

Do not report package publication as fully complete until both public surfaces pass verification.
