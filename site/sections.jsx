// Page sections — Hero, Forge, Install, Editions, Footer
import React from 'react';
import {
  FoundryMark, GearStackIcon, BigAnvilLogo, CopyIcon, DownloadIcon, ArrowRightIcon,
  WorldFoundryIcon, BlenderIcon, MameIcon, AssemblerIcon, LibvgmIcon, VgmstreamIcon,
  AnvilEditionIcon, SpriteIcon, GamepadIcon, SparksIcon,
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
            <a href="#forge">The Forge</a>
            <a href="#install">Install</a>
            <a href="#editions">Editions</a>
            <a href="/packages">Packages</a>
            <a href="#docs">Docs</a>
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

function Install() {
  return (
    <section className="section" id="install">
      <div className="shell">
        <div className="section-head">
          <div>
            <span className="section-num">№ 02 · Install</span>
            <h2 className="section-title">Add the forge<br />to your sources.</h2>
          </div>
          <p className="section-blurb">
            Already running Ubuntu 26.04 LTS or any apt-compatible derivative?
            Add our signed repository and pull the Foundry kit piece by piece.
          </p>
        </div>

        <div className="install-wrap">
          <div className="install-left">
            <div className="codeblock">
              <div className="codeblock-head">
                <span>shell · root</span>
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
                <span className="ember">sudo apt install</span> foundry-atelier    <span className="cmt"># + everything (~10 GB)</span>
              </pre>
            </div>

            <div className="download-vms" id="download">
              <div className="codeblock-head">
                <span>download · iso</span>
                <DownloadIcon />
              </div>
              <ul className="vm-list">
                <li className="vm-row">
                  <span className="vm-type">Anvil</span>
                  <span className="vm-file">foundry-anvil-latest-amd64.iso</span>
                  <span className="vm-size">~5 GB</span>
                  <a className="vm-dl" href="https://iso.foundrylinux.org/foundry-anvil-latest-amd64.iso" aria-label="Download Anvil ISO"><DownloadIcon /></a>
                </li>
                <li className="vm-row">
                  <span className="vm-type">Atelier</span>
                  <span className="vm-file">foundry-atelier-latest-amd64.iso</span>
                  <span className="vm-size">~10 GB</span>
                  <a className="vm-dl" href="https://iso.foundrylinux.org/foundry-atelier-latest-amd64.iso" aria-label="Download Atelier ISO"><DownloadIcon /></a>
                </li>
              </ul>
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
              The full ISO is 4.8 GB.
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

function Foot() {
  return (
    <footer className="shell">
      <div className="foot">
        <div className="foot-mark">
          <FoundryMark size={28} />
          <span>FOUNDRY · LINUX</span>
        </div>
        <nav className="foot-links">
          <a href="#docs">Docs</a>
          <a href="/packages">Packages</a>
          <a href="#mirrors">Mirrors</a>
          <a href="#irc">IRC</a>
          <a href="#git">Git</a>
          <a href="#bugs">Bugs</a>
        </nav>
        <div className="foot-meta">
          Released under GPLv3<br />
          Forged in the open · MMXXVI
        </div>
      </div>
    </footer>
  );
}

export { Topbar, Hero, Forge, Install, Editions, Foot, formatSize, findCategory, findEdition, ICONS };
