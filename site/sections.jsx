// Page sections — Hero, Forge, Install, Editions, Docs, Footer
import React from 'react';
import {
  FoundryMark, GearStackIcon, BigAnvilLogo, CopyIcon, DownloadIcon, ArrowRightIcon,
  WorldFoundryIcon, BlenderIcon, MameIcon, AssemblerIcon, LibvgmIcon, VgmstreamIcon,
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
          <div className="topbar-mark">
            <FoundryMark />
            <b>foundrylinux.org</b>
          </div>
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
        <div className="hero-eyebrow">a linux distribution · est. 2026</div>

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
          of every kind — an Ubuntu LTS desktop with the tools of the trade
          struck into it from the first boot.
        </p>

        <div className="apt-command">
          <span className="prompt">$</span>
          <span className="cmd">sudo apt install foundry-anvil</span>
          <button className="copy" onClick={copy}>
            <CopyIcon />{copied ? "COPIED" : "COPY"}
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
            const installCmd = `sudo apt install ${cat.metapackages[0]}`;
            return (
              <a className="forge-cell" key={categorySlug} href={`/packages#${categorySlug}`}>
                <div className="forge-icon"><Icon size={64} /></div>
                <span className="forge-role">{cat.role}</span>
                <h3 className="forge-name">{cat.title}</h3>
                <p className="forge-desc">{cat.blurb}</p>
                <div className="forge-meta">
                  <span className="forge-pkg">{installCmd}</span>
                  <span className="forge-stats">
                    {cat.package_count}&nbsp;pkgs · {formatSize(cat.installed_size_kb)}
                  </span>
                </div>
              </a>
            );
          })}
        </div>

        <div className="forge-footstrip">
          <span>
            <b>{vendored}</b> vendored &nbsp;·&nbsp;
            <b>{openItps}</b> open Debian&nbsp;ITP{openItps === 1 ? '' : 's'} &nbsp;·&nbsp;
            <b>{inUniverse}</b> already in Ubuntu universe/multiverse
          </span>
          <a className="forge-catalogue" href="/packages">
            Browse the full catalogue<ArrowRightIcon />
          </a>
        </div>
      </div>
    </section>
  );
}

// ─────────────────────────────────────────────────────────────────────────────
// Install / apt instructions

const CHANNELS = [
  {
    num: '01',
    label: 'Existing Ubuntu',
    hook: 'Already on 26.04? Add the repo — no reinstall.',
    cmd: 'sudo apt install foundry-anvil',
    href: '#path-apt',
    inv: 'non-destructive',
  },
  {
    num: '02',
    label: 'Container',
    hook: 'Any host — Linux, macOS, or Windows via WSL.',
    cmd: 'distrobox create -i ghcr.io/foundry-linux/devbox:26.04',
    href: 'https://github.com/foundry-linux/foundry-devbox',
    inv: 'isolated',
  },
  {
    num: '03',
    label: 'Virtual machine',
    hook: 'Sandboxed — VirtualBox, VMware, or QEMU. Your OS unchanged.',
    cmd: 'foundry-anvil-latest-amd64.{ova,vmdk,qcow2}',
    href: '#path-vm',
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
  {
    num: '05',
    label: 'Fresh install',
    hook: 'Replace everything. Full branded desktop from first boot.',
    cmd: 'foundry-anvil-latest-amd64.iso → erase disk',
    href: '#path-iso',
    inv: 'replaces-os',
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
            Five ways in — ordered from least to most disruptive to your
            current setup. Coming from Windows? Start at{" "}
            <a href="#path-vm">Virtual machine</a> or{" "}
            <a href="#path-iso">Dual boot</a>.
          </p>
        </div>

        <div className="channels-grid">
          {CHANNELS.map(({ num, label, hook, cmd, href, inv }) => (
            <a key={num} className="channel-card" href={href} data-invasiveness={inv}>
              <span className="channel-num">{num}</span>
              <span className="channel-label">{label}</span>
              <p className="channel-hook">{hook}</p>
              <code className="channel-cmd">{cmd}</code>
              <span className="channel-inv">{inv}</span>
            </a>
          ))}
        </div>

        <div className="install-wrap">
          <div className="install-left">

            <div className="codeblock" id="path-apt">
              <div className="codeblock-head">
                <span><span className="path-tag">path 01 · existing ubuntu</span> shell · root</span>
                <div className="dots"><i /><i /><i /></div>
              </div>
              <pre>
                <span className="step">① Add the Foundry archive</span>{"\n"}
                <span className="ember">curl</span> <span className="kw">-fsSL</span> <span className="str">https://foundrylinux.org/setup.sh</span> | <span className="ember">bash</span>{"\n"}
                {"\n"}
                <span className="step">② Install</span>{"\n"}
                <span className="ember">sudo apt install</span> foundry-anvil{"\n"}
                {"\n"}
                <span className="cmt"># or pick a different edition:</span>{"\n"}
                <span className="ember">sudo apt install</span> foundry-sprite     <span className="cmt"># + heavy graphics + audio</span>{"\n"}
                <span className="ember">sudo apt install</span> foundry-atelier    <span className="cmt"># + everything (~15 GB)</span>
              </pre>
            </div>

            <div className="download-vms" id="path-vm">
              <div className="codeblock-head">
                <span><span className="path-tag">path 03 · virtual machine</span> download · vms</span>
                <DownloadIcon />
              </div>
              <ul className="vm-list">
                <li className="vm-row">
                  <span className="vm-type">VirtualBox</span>
                  <span className="vm-file">foundry-anvil-latest-amd64.ova</span>
                  <span className="vm-size">11 GB</span>
                  <a className="vm-dl" href="https://iso.foundrylinux.org/foundry-anvil-latest-amd64.ova" aria-label="Download Anvil OVA"><DownloadIcon /></a>
                </li>
                <li className="vm-row">
                  <span className="vm-type">VMware</span>
                  <span className="vm-file">foundry-anvil-latest-amd64.vmdk</span>
                  <span className="vm-size">11 GB</span>
                  <a className="vm-dl" href="https://iso.foundrylinux.org/foundry-anvil-latest-amd64.vmdk" aria-label="Download Anvil VMDK"><DownloadIcon /></a>
                </li>
                <li className="vm-row">
                  <span className="vm-type">QEMU / KVM</span>
                  <span className="vm-file">foundry-anvil-latest-amd64.qcow2</span>
                  <span className="vm-size">5.1 GB</span>
                  <a className="vm-dl" href="https://iso.foundrylinux.org/foundry-anvil-latest-amd64.qcow2" aria-label="Download Anvil qcow2"><DownloadIcon /></a>
                </li>
              </ul>
            </div>

            <div className="download-vms" id="path-iso">
              <div className="codeblock-head">
                <span><span className="path-tag">path 04–05 · dual boot / fresh install</span> download · iso</span>
                <DownloadIcon />
              </div>
              <ul className="vm-list">
                <li className="vm-row">
                  <span className="vm-type">Anvil</span>
                  <span className="vm-file">foundry-anvil-latest-amd64.iso</span>
                  <span className="vm-size">~5 GB</span>
                  <a className="vm-dl" href="https://iso.foundrylinux.org/foundry-anvil-latest-amd64.iso" aria-label="Download Anvil ISO"><DownloadIcon /></a>
                  <a className="vm-dl" href="https://iso.foundrylinux.org/foundry-anvil-latest-amd64.iso.torrent" aria-label="Anvil torrent"><TorrentIcon /></a>
                  <a className="vm-dl" href="magnet:?xt=urn:btih:4475708175bc2ef9f9c764f406a43679221147ed&dn=foundry-anvil-0.9.0-amd64&ws=https%3A%2F%2Fiso.foundrylinux.org%2Ffoundry-anvil-latest-amd64.iso&tr=http%3A%2F%2Ftracker.openbittorrent.com%3A80%2Fannounce&tr=http%3A%2F%2Ftracker.opentracker.info%3A80%2Fannounce" aria-label="Anvil magnet"><MagnetIcon /></a>
                </li>
                <li className="vm-row">
                  <span className="vm-type">Atelier</span>
                  <span className="vm-file">foundry-atelier-latest-amd64.iso</span>
                  <span className="vm-size">~15 GB</span>
                  <a className="vm-dl" href="https://iso.foundrylinux.org/foundry-atelier-latest-amd64.iso" aria-label="Download Atelier ISO"><DownloadIcon /></a>
                  <a className="vm-dl" href="https://iso.foundrylinux.org/foundry-atelier-latest-amd64.iso.torrent" aria-label="Atelier torrent"><TorrentIcon /></a>
                  <a className="vm-dl" href="magnet:?xt=urn:btih:bfb49155b514b79b36e544887f8b87af25d63bc6&dn=foundry-atelier-0.9.0-amd64&ws=https%3A%2F%2Fiso.foundrylinux.org%2Ffoundry-atelier-latest-amd64.iso&tr=http%3A%2F%2Ftracker.openbittorrent.com%3A80%2Fannounce&tr=http%3A%2F%2Ftracker.opentracker.info%3A80%2Fannounce" aria-label="Atelier magnet"><MagnetIcon /></a>
                </li>
              </ul>
              <div className="dualboot-callout">
                <span className="dualboot-label">dual boot with windows</span>
                <p>
                  Boot the ISO, choose <em>Install alongside Windows Boot Manager</em> in
                  the Calamares installer — it detects Windows and offers to shrink its
                  partition automatically. No manual partitioning required. Give Foundry
                  Linux at least 60 GB.{" "}
                  <a href="https://ubuntu.com/tutorials/install-ubuntu-desktop#1-overview">
                    Ubuntu dual-boot guide →
                  </a>
                </p>
              </div>
            </div>

          </div>

          <div className="install-side">
            <h3>Signed by the smith.</h3>
            <p>
              Every package in the Foundry archive is signed with our long-term
              maintainer key. Verify the fingerprint before adding the keyring
              to a production machine.
            </p>

            <div className="fingerprint">
              <b>archive key · ed25519</b>
              <code>F0D9 7E1A · C5B2 4A87 · 6E3F 218D · 9C04 BB31</code>
            </div>

            <h3 style={{ marginTop: 32 }}>System requirements</h3>
            <p>
              The ISO installs a full Kubuntu 26.04 (Plasma 6) desktop with
              the Foundry kit struck in. Calamares installer — guided or
              automatic partitioning. Works bare-metal or in a VM.
            </p>
            <ul className="requirements">
              <li><span className="req-key">CPU</span><span>x86_64 · ARM64</span></li>
              <li><span className="req-key">Memory</span><span>4 GB minimum · 8 GB recommended</span></li>
              <li><span className="req-key">Storage</span><span>20 GB free</span></li>
              <li><span className="req-key">GPU</span><span>Vulkan 1.2 capable</span></li>
              <li><span className="req-key">Base</span><span>Ubuntu 26.04 LTS</span></li>
            </ul>
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
