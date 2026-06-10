# Can Foundry Linux distribute Visual Studio Code?

**Date**: 2026-06-10
**Question**: Can we ship the VS Code editor in `apt.foundrylinux.org` and/or the Foundry Linux ISO?
**Short answer**: **Not Microsoft's official build — its EULA forbids redistribution. But we can ship
VSCodium**, the MIT-licensed de-branded rebuild, which is what other distros do. This is a
*redistribution-rights* problem, not a copyright/open-source problem — the source is MIT; the
binaries Microsoft ships are not.

---

## The trap: "VS Code is open source" is half true

The phrase "VS Code" refers to three legally distinct artifacts. Conflating them is the whole trap.

| Artifact | What it is | License | Can we redistribute? |
|----------|-----------|---------|----------------------|
| **Code - OSS** | The source tree at [github.com/microsoft/vscode](https://github.com/microsoft/vscode) | **MIT** | ✅ Source yes — but you must *build it yourself* and strip MS branding/telemetry/marketplace |
| **Visual Studio Code** | Microsoft's official build (`.deb` from [code.visualstudio.com](https://code.visualstudio.com), `packages.microsoft.com`) | **Proprietary Microsoft EULA** ([code.visualstudio.com/license](https://code.visualstudio.com/license)) | ❌ **No** — EULA prohibits bundling/redistributing the installer |
| **VSCodium** | Community reproducible build of Code-OSS, branding removed, telemetry off | **MIT** (binaries) | ✅ **Yes** |

The mechanism: Microsoft clones the MIT `vscode` repo, lays down a customised `product.json`
(telemetry endpoints, the Microsoft Extension Marketplace gallery, the MS logo and name), compiles
it, and ships that binary **under a separate proprietary license**. So:

- The **source** is MIT — anyone can build and redistribute a derivative.
- The **official binary** is not — it's governed by the EULA, and the Microsoft name + logo are
  trademarks that MIT does not grant.

> "Visual Studio Code is a distribution of the Code - OSS repository with Microsoft-specific
> customizations released under a traditional Microsoft product license."
> — [code.visualstudio.com/docs/supporting/faq](https://code.visualstudio.com/docs/supporting/faq)

"Free to use" ≠ "free to redistribute." VS Code is free of charge for commercial development, but
the EULA still forbids us from putting Microsoft's binary in our apt repo or baking it into the ISO.
This is exactly the [non-redistributable, not copyrighted](../../.claude/memory/feedback_copyright_vs_redistributable.md)
distinction — GPL code is copyrighted too; the issue here is distribution rights.

---

## The answer: VSCodium

[VSCodium](https://vscodium.com) ([github.com/VSCodium/vscodium](https://github.com/VSCodium/vscodium))
is precisely the "build Code-OSS yourself, strip the proprietary bits" path, already done and
maintained:

- **MIT-licensed binaries** — redistributable.
- **Telemetry disabled** at build time (`product.json` patched, `ENABLE_TELEMETRY=false`).
- **Microsoft branding removed** — no MS trademarks, so no trademark exposure.
- **Ships `.deb`/`.rpm`/`.tar.gz`** on its GitHub releases page, plus its own apt/rpm repos.

This is the standard distro answer. Debian ships `code-oss`; Arch packages both `code` (OSS build)
and AUR `vscodium-bin`; most "VS Code on $DISTRO" guides for a *bundled* editor point at VSCodium or
code-oss, never the MS binary.

### The catch you must handle: the Extension Marketplace

The **Microsoft Extension Marketplace is itself license-restricted**, independent of the editor
binary. Per the [Visual Studio Marketplace Terms of Use](https://aka.ms/vsmarketplace-ToU):

> "You may only install and use Marketplace Offerings with Visual Studio Products and Services."

So a non-Microsoft build (VSCodium, code-oss) **may not legally point at the MS Marketplace**.
VSCodium ships pointed at **[Open VSX Registry](https://open-vsx.org)** (run by the Eclipse
Foundation) instead. Consequences:

- Open-source extensions: ✅ available via Open VSX.
- **Microsoft-proprietary extensions won't work** — by license, not just availability. This includes
  **C/C++ (cpptools), C# Dev Kit, Pylance, the Remote-SSH/Containers/WSL pack, Live Share, and
  GitHub Copilot**. Their licenses restrict them to official MS builds.

For our audience — game devs on Python/C++/Blender — the practical gaps are **Pylance**
(use open-source Python LSP / `python-lsp-server` instead) and **cpptools** (use `clangd` via the
open `llvm-vs-code-extensions.vscode-clangd`, which *is* on Open VSX). Worth documenting, not a
blocker.

---

## Options compared

| Option | Redistributable? | Branding | Marketplace | Verdict |
|--------|------------------|----------|-------------|---------|
| Bundle MS `.deb` in apt repo / ISO | ❌ EULA violation | MS | MS (full) | **No — do not** |
| Ship a script that pulls MS's apt repo at install time | ⚠️ user fetches from MS, we don't redistribute | MS | MS (full) | Legal grey-ish; defeats offline ISO; not our style |
| **Package VSCodium as a vendored upstream** | ✅ MIT | de-branded | Open VSX | **Recommended** |
| Build code-oss ourselves from source | ✅ MIT | must de-brand ourselves | we choose | Reinvents VSCodium — no reason to |

The middle option (a `setup-vscode.sh` that wires `packages.microsoft.com`) is how you'd let a user
*opt in* to the genuine MS build on their own machine without us redistributing anything — Microsoft
distributes it, we just add the source. That's defensible but: (a) it can't go in the ISO's offline
package set, and (b) it pulls telemetry-on, MS-EULA software, which clashes with the distro's
posture. Keep it in our back pocket as a documented opt-in, not the default.

---

## Recommendation

1. **Package VSCodium** as a vendored upstream in `foundry-apt/` via the `/package` skill —
   sha256-pinned `.deb` (or tarball) from
   [github.com/VSCodium/vscodium/releases](https://github.com/VSCodium/vscodium/releases), overlay
   `debian/`, rebuild in the `ubuntu:26.04` container. Default it to Open VSX (VSCodium already does).
2. **Edition tier**: a code editor belongs in the desktop-agnostic dev base. Candidate for inclusion
   in **`foundry-core`** (so it lands in both the Phase 2 devbox and every ISO edition), most likely
   under a new `editors` category metapackage. Confirm against the
   [edition table](../../foundry-apt/README.md) before wiring `Depends:`.
3. **Document the Marketplace limitation** in the package description and/or `foundry-welcome`:
   extensions come from Open VSX; MS-proprietary extensions (Pylance, cpptools, Remote-*, Copilot)
   are unavailable by license. Point users at the open equivalents (clangd, python-lsp).
4. **Check Ubuntu 26.04 universe first** (house rule) — run `apt-cache policy code-oss vscodium` in a
   fresh `ubuntu:26.04` container. If universe ships `code-oss` at a usable version, prefer the
   universe package over vendoring (one less upstream to track), and only vendor VSCodium if we want
   its specific branding/Open-VSX defaults or a newer version.

---

## Sources

- [Visual Studio Code License (Microsoft EULA)](https://code.visualstudio.com/license)
- [VS Code FAQ — "Why is VS Code licensed differently from the source"](https://code.visualstudio.com/docs/supporting/faq)
- [VSCodium homepage](https://vscodium.com) · [VSCodium on GitHub](https://github.com/VSCodium/vscodium)
- [Visual Studio Marketplace Terms of Use](https://aka.ms/vsmarketplace-ToU)
- [Open VSX Registry](https://open-vsx.org) · [Open VSX FAQ (Eclipse Foundation)](https://www.eclipse.org/legal/open-vsx-registry-faq/)
- [Arch Wiki — Visual Studio Code (code vs code-oss vs vscodium)](https://wiki.archlinux.org/title/Visual_Studio_Code)
- [Keyboard Playing — "VS Code May Not Be As Open Source As You Think"](https://keyboardplaying.org/blog/2022/01/visual-studio-code-not-open-source/)
