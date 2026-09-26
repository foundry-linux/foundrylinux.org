# Foundry anvil favicons

Use the existing anvil mark for `apt.foundrylinux.org` and check the other
Foundry websites for missing favicons.

## Audit — 26 September 2026

| Site | Existing SVG favicon | Existing `/favicon.ico` |
|---|---|---|
| `foundrylinux.org` | Linked and served as SVG | Returns HTML instead of an icon |
| `apt.foundrylinux.org` | Linked and served as SVG | 404 |
| `iso.foundrylinux.org` | Links to the main site's SVG; no local copy | 404 |
| `worldfoundry.org` | Linked and served as SVG | 404 |
| `apt.worldfoundry.org` | Linked and served as SVG | 404 |

`www.worldfoundry.org` redirects to `worldfoundry.org`.
`www.foundrylinux.org` did not resolve during this check.
The WorldFoundry sites are audited only; edits cover the three Foundry Linux
sites in this repository.

## Work

- [x] Check the live favicon links and HTTP responses.
- [x] Add an ICO fallback with 16, 32, 48, and 64 px images alongside the SVG.
- [x] Add local favicon assets to the ISO site and include them in its upload script.
- [x] Update all three HTML generators to link the ICO fallback and SVG.
- [x] Document how to regenerate assets from `site/favicon.svg`.
- [x] Finish visual checks of the generated icon and verify generated HTML.
- [x] Publish the favicon assets and HTML changes.
- [x] Verify live icon content types, image dimensions, and HTML links.

## Validation notes

All three site generators passed. The main site was rebuilt with its existing
published package data, and its local link check passed. Shell syntax checks
passed. ShellCheck passed for the changed scripts with the ISO generator's
three existing unused manifest variable warnings excluded (`SC2034`).

The ICO was rendered with librsvg and ImageMagick, visually checked, and
verified to contain 16, 32, 48, and 64 px images. The APT and ISO live HTML
was updated only at the favicon links, preserving the published listings.

The main site was deployed through Cloudflare Pages:
[deployment 30a2ac07](https://30a2ac07.foundrylinux-org.pages.dev).
APT and ISO assets and HTML were uploaded directly to their R2 buckets.

## Verified live result

**PASS:** All nine live checks (HTML, SVG, and ICO on each site) passed.

| Site | HTML links both formats | SVG response | ICO response |
|---|---|---|---|
| [foundrylinux.org](https://foundrylinux.org/) | Yes | 200, `image/svg+xml` | 200, `image/vnd.microsoft.icon` |
| [apt.foundrylinux.org](https://apt.foundrylinux.org/) | Yes | 200, `image/svg+xml` | 200, `image/vnd.microsoft.icon` |
| [iso.foundrylinux.org](https://iso.foundrylinux.org/) | Yes | 200, `image/svg+xml` | 200, `image/vnd.microsoft.icon` |

Every downloaded icon matched the corresponding source asset byte for byte.
The related WorldFoundry sites already have working SVG favicons; their ICO
fallbacks remain absent. The `www.foundrylinux.org` DNS result is separate
from this favicon change.
