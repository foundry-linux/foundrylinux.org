// Page sections — Hero, Forge, Install, Editions, Docs, Footer
import React from 'react';
import {
  FoundryMark, GearStackIcon, BigAnvilLogo, CopyIcon, DownloadIcon, ArrowRightIcon,
  WorldFoundryIcon, BlenderIcon, MameIcon, AssemblerIcon, LibvgmIcon, VgmstreamIcon,
  ReqCpuIcon, ReqMemoryIcon, ReqStorageIcon, ReqGpuIcon, ReqBaseIcon,
  AnvilEditionIcon, SpriteIcon, GamepadIcon, SparksIcon,
  TorrentIcon, MagnetIcon,
} from './icons.jsx';
import packagesData from './packages-data.json';

const ICONS = {
  WorldFoundryIcon, BlenderIcon, MameIcon, AssemblerIcon,
  LibvgmIcon, VgmstreamIcon, GamepadIcon, SpriteIcon, SparksIcon,
};

function formatSize(kb) {
  if (!kb || kb <= 0) return '—';
  if (kb >= 1024 * 1024) return (kb / 1024 / 1024).toFixed(1) + ' GiB';
  if (kb >= 1024)        return Math.round(kb / 1024) + ' MiB';
  return kb + ' KiB';
}

function findCategory(slug) {
  return packagesData.categories.find(c => c.slug === slug);
}

function findEdition(slug) {
  return packagesData.editions.find(e => e.slug === slug);
}

function Topbar() {
  return (
    <header className="topbar-wrap">
      <div className="shell">
        <div className="topbar">
          <a className="topbar-mark" href="/">
            <FoundryMark />
            <b>foundrylinux.org</b>
          </a>
          <nav className="topbar-nav">
            <a href="/#forge">The Forge</a>
            <a href="/#install">Install</a>
            <a href="/#editions">Editions</a>
            <a href="/packages">Packages</a>
            <a href="/#docs">Docs</a>
          </nav>
          <div className="topbar-version">
            <span className="dot" />FOUNDRY 26.04 · "ANVIL" · LTS
          </div>
        </div>
      </div>
    </header>
  );
}

function Hero() {
  const [copied, setCopied] = React.useState(false);
  const copy = () => {
    navigator.clipboard?.writeText("sudo apt install foundry-anvil");
    setCopied(true);
    setTimeout(() => setCopied(false), 1400);
  };

  return (
    <section className="hero">
      <div className="hero-deco left"><GearStackIcon /></div>

      <div className="shell">
        <div className="hero-eyebrow">the game maker's forge · est. 2026</div>

        <div className="hero-anvil">
          <BigAnvilLogo width={420} />
        </div>

        <h1 className="wordmark"
            data-material="chrome"
            data-font="big-shoulders"
            data-mark="foundry-linux">
          {["FOUNDRY", "LINUX"].map((l, i) => (
            <span key={i} className="wordmark-line">{l}</span>
          ))}
        </h1>

        <p className="hero-tagline">
          Forged for <em>game makers, level designers, and digital artists</em>{' '}
          of every kind — a Linux desktop with the tools of the trade
          struck into it from the first boot.
        </p>

        <div className="apt-command">
          <span className="prompt">$</span>
          <span className="cmd">sudo apt install foundry-anvil</span>
          <button className="copy" onClick={copy} title={copied ? "Copied!" : "Copy"}>
            <CopyIcon />
          </button>
        </div>

        <div className="hero-actions">
          <a className="cta cta-primary" href="https://iso.foundrylinux.org">
            <DownloadIcon />Download the ISO
          </a>
          <a className="cta cta-secondary" href="#install">
            Set up the repo<ArrowRightIcon />
          </a>
        </div>
      </div>
    </section>
  );
}

// ─────────────────────────────────────────────────────────────────────────────
// The Forge — six category headliner cards, auto-driven from packages-data.json
//
// Card slots are hand-curated (which category appears, in what order, with
// what icon). The numbers (package count, install command, headline size)
// are looked up from packages-data.json at SSR time so they never diverge
// from the live apt indexes.

const FORGE_CARDS = [
  // Top row — creative loops
  { categorySlug: 'worldfoundry-gdk' },
  { categorySlug: 'blender' },
  { categorySlug: 'retro-tools' },
  // Bottom row — rest of catalogue
  { categorySlug: 'emulators' },
  { categorySlug: 'audio-production' },
  { categorySlug: 'games' },
];

function Forge() {
  const totals = packagesData.upstream_summary;
  const inUniverse = (totals['in-ubuntu-universe'] || 0) + (totals['in-ubuntu-multiverse'] || 0);
  const openItps = totals['debian-itp'] || 0;
  const vendored = totals['vendored'] || 0;

  return (
    <section className="section" id="forge">
      <div className="shell">
        <div className="section-head">
          <div>
            <span className="section-num">№ 01 · The Forge</span>
            <h2 className="section-title">Struck into the iron.</h2>
          </div>
          <p className="section-blurb">
            We package the retro, game-dev, reverse-engineering, and
            authoring tools Ubuntu doesn&rsquo;t ship — and push the ones
            worth shipping upstream into Debian. Built for Ubuntu&nbsp;26.04.
            Resigned, rebuilt, and re-tested on every push.
          </p>
        </div>

        <div className="forge-grid">
          {FORGE_CARDS.map(({ categorySlug }) => {
            const cat = findCategory(categorySlug);
            if (!cat) {
              throw new Error(`Forge card references unknown category slug: ${categorySlug}`);
            }
            const Icon = ICONS[cat.icon] || SparksIcon;
            const pkgName = cat.metapackages[0];
            const installCmd = `sudo apt install ${pkgName}`;
            return (
              <div className="forge-cell" key={categorySlug}>
                <a className="forge-cell-link" href={`/packages#${categorySlug}`}>
                  <div className="forge-icon"><Icon size={64} /></div>
                  <span className="forge-role">{cat.role}</span>
                  <h3 className="forge-name">{cat.title}</h3>
                </a>
                <p className="forge-desc">{cat.blurb}</p>
                <div className="forge-meta">
                  <div className="forge-cmd-row">
                    <span className="forge-cmd-prefix">sudo apt install</span>
                    <span className="forge-cmd-pkg">
                      <span className="forge-pkg">{pkgName}</span>
                      <button className="forge-copy" data-copy-text={installCmd} aria-label="Copy install command"><CopyIcon /></button>
                    </span>
                  </div>
                  <span className="forge-stats">
                    {cat.package_count}&nbsp;pkgs · {formatSize(cat.installed_size_kb)}
                  </span>
                </div>
              </div>
            );
          })}
        </div>

        <div className="forge-footstrip">
          <span>
            <span className="forge-badge">{vendored}</span> vendored &nbsp;·&nbsp;
            <span className="forge-badge">{openItps}</span> open Debian&nbsp;ITP{openItps === 1 ? '' : 's'} &nbsp;·&nbsp;
            <span className="forge-badge forge-badge--accent">{inUniverse}</span> already in Ubuntu universe/multiverse
          </span>
          <a className="forge-catalogue" href="/packages">
            Browse the full catalogue<span className="forge-arrow" aria-hidden="true">›</span>
          </a>
        </div>
      </div>
    </section>
  );
}

// ─────────────────────────────────────────────────────────────────────────────
// Install / apt instructions

// Cards 02 and 04 stay in the middle row via map.
// Cards 01, 03, 05 are rendered individually with two-zone layouts.
const CHANNELS_MID = [
  {
    num: '02',
    label: 'Container',
    hook: 'Any host — Linux, macOS, or Windows via WSL.',
    cmd: 'distrobox create -i ghcr.io/foundry-linux/devbox:26.04',
    href: 'https://github.com/foundry-linux/foundry-devbox',
    inv: 'isolated',
  },
  {
    num: '04',
    label: 'Dual boot',
    hook: 'Keep Windows. Calamares offers "Install alongside" automatically.',
    cmd: 'foundry-anvil-latest-amd64.iso → alongside Windows',
    href: '#path-iso',
    inv: 'additive',
  },
];

function Install() {
  return (
    <section className="section" id="install">
      <div className="shell">
        <div className="section-head">
          <div>
            <span className="section-num">№ 02 · Install</span>
            <h2 className="section-title">Pick your<br />path.</h2>
          </div>
          <p className="section-blurb">
            Five ways in — lightest touch to full commitment. Coming from Windows? Start at{" "}
            <a href="#path-vm">Virtual machine</a> or{" "}
            <a href="#path-iso">Dual boot</a>.
          </p>
        </div>

        <div className="channels-grid">

          {/* Card 01 — full-width, two-zone: left code + right info */}
          <div className="channel-card channel-card--wide" id="path-apt" data-invasiveness="non-destructive">
            <div className="hero-left hero-left--code">
              <span className="hero-dl-label">shell · root</span>
              <div className="card-code">
                <div className="code-section">
                  <span className="step">① Add the Foundry archive</span>
                  <div className="code-cmd-row">
                    <code><span className="ember">curl</span> <span className="kw">-fsSL</span> <span className="str">https://foundrylinux.org/setup.sh</span> | <span className="ember">bash</span></code>
                    <button className="inline-copy-btn" data-copy-text="curl -fsSL https://foundrylinux.org/setup.sh | bash" aria-label="Copy"><CopyIcon /></button>
                  </div>
                </div>
                <div className="code-section">
                  <span className="step">② Install</span>
                  <div className="code-cmd-row">
                    <code><span className="ember">sudo apt install</span> foundry-core</code>
                    <button className="inline-copy-btn" data-copy-text="sudo apt install foundry-core" aria-label="Copy"><CopyIcon /></button>
                  </div>
                  <div className="code-alts">
                    <code><span className="cmt"># or pick a different edition:</span></code>
                    <code><span className="ember">sudo apt install</span> foundry-sprite <span className="cmt"># + heavy graphics + audio</span></code>
                    <code><span className="ember">sudo apt install</span> foundry-atelier <span className="cmt"># + everything (~15 GB)</span></code>
                  </div>
                </div>
              </div>
            </div>
            <div className="hero-right hero-right--info">
              <span className="channel-num">01</span>
              <span className="channel-label">Existing Ubuntu</span>
              <p className="channel-hook">Already on 26.04? Add the repo —&nbsp;no&nbsp;reinstall.</p>
              <span className="channel-inv">non-destructive</span>
            </div>
          </div>

          {/* Middle row: 02, 03 (VM downloads inline), 04 */}
          <a className="channel-card" href="https://github.com/foundry-linux/foundry-devbox" data-invasiveness="isolated">
            <span className="channel-num">02</span>
            <span className="channel-label">Container</span>
            <p className="channel-hook">Any host — Linux, macOS, or Windows via WSL.</p>
            <div className="channel-cmd-row">
              <code className="channel-cmd">distrobox create -i ghcr.io/foundry-linux/devbox:26.04</code>
              <button className="inline-copy-btn" data-copy-text="distrobox create -i ghcr.io/foundry-linux/devbox:26.04" aria-label="Copy" onClick={e => { e.preventDefault(); e.stopPropagation(); }}><CopyIcon /></button>
            </div>
            <span className="channel-inv">isolated</span>
          </a>

          <div className="channel-card channel-card--vm" id="path-vm" data-invasiveness="isolated">
            <span className="channel-num">03</span>
            <span className="channel-label">Virtual machine</span>
            <p className="channel-hook">Sandboxed — VirtualBox, VMware, or QEMU. Your OS unchanged.</p>
            <div className="card-vm-boxes">
              <a className="card-vm-box" href="https://iso.foundrylinux.org/foundry-anvil-latest-amd64.ova">
                <span className="card-vm-box-type">VirtualBox</span>
                <span className="card-vm-box-size">11 GB</span>
              </a>
              <a className="card-vm-box" href="https://iso.foundrylinux.org/foundry-anvil-latest-amd64.vmdk">
                <span className="card-vm-box-type">VMware</span>
                <span className="card-vm-box-size">11 GB</span>
              </a>
              <a className="card-vm-box" href="https://iso.foundrylinux.org/foundry-anvil-latest-amd64.qcow2">
                <span className="card-vm-box-type">QEMU / KVM</span>
                <span className="card-vm-box-size">5.1 GB</span>
              </a>
            </div>
            <span className="channel-inv">isolated</span>
          </div>

          <a className="channel-card" href="#path-iso" data-invasiveness="additive">
            <span className="channel-num">04</span>
            <span className="channel-label">Dual boot</span>
            <p className="channel-hook">Keep Windows. Boot from USB — the installer detects Windows and offers to shrink its partition automatically.</p>
            <code className="channel-cmd">foundry-anvil-latest-amd64.iso →&nbsp;alongside&nbsp;Windows</code>
            <span className="channel-inv">additive</span>
          </a>

          {/* Card 05 — hero: full-width, ember glow, embedded ISO downloads */}
          <div className="channel-card channel-card--hero" id="path-iso" data-invasiveness="replaces-os">
            <div className="hero-left">
              <span className="channel-num">05</span>
              <span className="channel-label">Fresh install</span>
              <p className="channel-hook">Replace everything. Full branded Kubuntu 26.04 desktop from first boot — bare metal or alongside Windows.</p>
              <span className="channel-inv">replaces-os</span>
            </div>
            <div className="hero-right">
              <span className="hero-dl-label">download iso · anvil edition</span>
              <a className="hero-dl-row" href="https://iso.foundrylinux.org/foundry-anvil-latest-amd64.iso">
                <span>foundry-anvil-latest-amd64.iso</span>
                <span className="hero-dl-size">~5 GB</span>
                <DownloadIcon />
              </a>
              <a className="hero-dl-row" href="https://iso.foundrylinux.org/foundry-atelier-latest-amd64.iso">
                <span>foundry-atelier-latest-amd64.iso</span>
                <span className="hero-dl-size">~15 GB</span>
                <DownloadIcon />
              </a>
              <p className="hero-dualboot">
                Keeping Windows?{" "}
                Boot from USB, and the installer will offer to shrink your Windows
                partition automatically — no manual partitioning required.{" "}
                <a className="dualboot-guide-link" href="https://help.ubuntu.com/community/WindowsDualBoot">Windows dual-boot guide <span className="dualboot-guide-arrow" aria-hidden="true">›</span></a>
              </p>
            </div>
          </div>
        </div>

        <div className="install-info">
          <div className="install-signed">
            <div className="install-signed-text install-side">
              <h3>Signed by the smith.</h3>
              <p>Every package in the Foundry archive is signed with our long-term maintainer key. Verify the fingerprint before adding the keyring to a production machine.</p>
            </div>
            <div className="fingerprint">
              <b>archive key · ed25519</b>
              <code>F0D9 7E1A · C5B2 4A87 · 6E3F 218D · 9C04 BB31</code>
            </div>
          </div>

          <div className="install-reqs">
            <h3>System requirements</h3>
            <div className="req-cards">
              <div className="req-card"><ReqCpuIcon size={28} /><span className="req-key">CPU</span><ul className="req-val"><li><FoundryMark size={10} />x86_64</li><li><FoundryMark size={10} />ARM64</li></ul></div>
              <div className="req-card"><ReqMemoryIcon size={28} /><span className="req-key">Memory</span><ul className="req-val"><li><FoundryMark size={10} />4 GB minimum</li><li><FoundryMark size={10} />8 GB recommended</li></ul></div>
              <div className="req-card"><ReqStorageIcon size={28} /><span className="req-key">Storage</span><ul className="req-val"><li><FoundryMark size={10} />20 GB free</li></ul></div>
              <div className="req-card"><ReqGpuIcon size={28} /><span className="req-key">GPU</span><ul className="req-val"><li><FoundryMark size={10} />Vulkan 1.2</li></ul></div>
              <div className="req-card"><ReqBaseIcon size={28} /><span className="req-key">Base</span><ul className="req-val"><li><FoundryMark size={10} />Kubuntu 26.04 LTS</li></ul></div>
            </div>
          </div>
        </div>
      </div>
    </section>
  );
}

// ─────────────────────────────────────────────────────────────────────────────
// Editions


function Editions() {
  const anvil   = findEdition('anvil');
  const sprite  = findEdition('sprite');
  const atelier = findEdition('atelier');
  const editionCards = [
    { ed: anvil,   icon: <AnvilEditionIcon /> },
    { ed: sprite,  icon: <SpriteIcon /> },
    { ed: atelier, icon: <GamepadIcon /> },
  ];
  return (
    <section className="section" id="editions">
      <div className="shell">
        <div className="section-head">
          <div>
            <span className="section-num">№ 03 · Editions</span>
            <h2 className="section-title">Three<br />marks.</h2>
          </div>
          <p className="section-blurb">
            One distribution, three nested casts. Atelier pulls in Sprite,
            which pulls in Anvil. Pick the largest one whose tools you
            actually want struck in from first boot.
          </p>
        </div>

        <div className="editions">
          {editionCards.map(({ ed, icon }) => (
            <div className="edition" key={ed.slug} data-flavor={ed.slug}>
              <div className="edition-icon">{icon}</div>
              <span className="edition-tag">Edition · 26.04</span>
              <h3 className="edition-name">{ed.title}</h3>
              <p className="edition-desc">{ed.blurb}</p>
              <div className="edition-pkg">
                <span>{ed.metapackage}</span>
                <span className="size">{ed.package_count}&nbsp;pkgs · {formatSize(ed.installed_size_kb)}</span>
                <button className="inline-copy-btn" data-copy-text={`sudo apt install ${ed.metapackage}`} aria-label="Copy install command"><CopyIcon /></button>
              </div>
            </div>
          ))}
        </div>
      </div>
    </section>
  );
}

// ─────────────────────────────────────────────────────────────────────────────
// Docs

const DOCS_LINKS = [
  {
    label: 'Source',
    href: 'https://github.com/foundry-linux',
    desc: 'github.com/foundry-linux',
  },
  {
    label: 'Issues & Bugs',
    href: 'https://github.com/foundry-linux/foundry-iso/issues',
    desc: 'github.com/…/issues',
  },
  {
    label: 'APT Repository',
    href: 'https://apt.foundrylinux.org',
    desc: 'apt.foundrylinux.org',
  },
  {
    label: 'Package Catalogue',
    href: '/packages',
    desc: 'foundrylinux.org/packages',
  },
];

function Docs() {
  return (
    <section className="section" id="docs">
      <div className="shell">
        <div className="section-head">
          <div>
            <span className="section-num">№ 04 · Docs</span>
            <h2 className="section-title">Find the<br />source.</h2>
          </div>
          <p className="section-blurb">
            Everything is in the open — source, issues, packages, and the
            signed apt repo. No forums, no wiki, no separate account.
          </p>
        </div>

        <div className="docs-links">
          {DOCS_LINKS.map(({ label, href, desc }) => (
            <a key={href} className="docs-link-card" href={href}>
              <span className="docs-link-label">{label}</span>
              <span className="docs-link-desc">{desc}</span>
              <ArrowRightIcon />
            </a>
          ))}
        </div>
      </div>
    </section>
  );
}

// ─────────────────────────────────────────────────────────────────────────────

function Foot() {
  return (
    <footer className="shell">
      <div className="foot">
        <div className="foot-mark">
          <FoundryMark size={28} />
          <span>FOUNDRY · LINUX</span>
        </div>
        <nav className="foot-links">
          <a href="/#docs">Docs</a>
          <a href="/packages">Packages</a>
          <a href="https://github.com/foundry-linux">Git</a>
          <a href="https://github.com/foundry-linux/foundry-iso/issues">Bugs</a>
        </nav>
        <div className="foot-meta">
          Released under GPLv3<br />
          Forged in the open · MMXXVI
        </div>
      </div>
    </footer>
  );
}

export { Topbar, Hero, Forge, Install, Editions, Docs, Foot, formatSize, findCategory, findEdition, ICONS };
