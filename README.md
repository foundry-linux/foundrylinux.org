# Foundry Linux

[Foundry Linux](https://foundrylinux.org) is an Ubuntu 26.04 based distribution for game development. This repository contains its website, APT packages, setup scripts, development container, and bootable ISO build.

## Refresh packages and build an ISO

Run from the repository root:

```bash
task distro-refresh
```

This checks the ISO's APT sources, rebuilds the local Foundry packages, resolves current Ubuntu packages from fresh APT indexes, builds an **Anvil** ISO, and runs a QEMU boot smoke test. The ISO and build log are written to `foundry-iso/dist/`. An inventory of the refresh is written to `docs/investigations/`.

To build a different edition, pass it to the refresh task:

```bash
task distro-refresh -- --edition atelier
task distro-refresh -- --edition all
```

The refresh creates local artifacts. It does not sign, upload, or publish them. The ISO build also increments `foundry-iso/VERSION` and makes a local version bump commit.

For a direct rebuild without the APT source check, inventory, or smoke test, run:

```bash
task --force iso-build EDITION=all
```

Use `EDITION=anvil` or `EDITION=atelier` to build one edition. Docker and the Go Task runner (`task`) are required; the smoke test also needs QEMU and OVMF. On a supported development host, `task dev-setup` installs the development tools, and `task iso-deps` installs the ISO build and smoke test dependencies.

## Repository layout

| Path | Contents |
| --- | --- |
| [`site/`](site/) | Static website source and generated pages |
| [`foundry-apt/`](foundry-apt/README.md) | Foundry packages and APT repository tooling |
| [`foundry-iso/`](foundry-iso/README.md) | Live ISO configuration, build scripts, and output |
| [`foundry-setup/`](foundry-setup/README.md) | Installation and APT source setup scripts |
| [`foundry-devbox/`](foundry-devbox/README.md) | Development container |
| [`Taskfile.yml`](Taskfile.yml) | Repository build and maintenance commands |

Run `task --list` to see all available commands.
