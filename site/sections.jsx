// Page sections — Hero, Kit, Install, Editions, Footer
import React from 'react';
import {
  FoundryMark, GearStackIcon, BigAnvilLogo, CopyIcon, DownloadIcon, ArrowRightIcon,
  WorldFoundryIcon, BlenderIcon, MameIcon, F9dasmIcon, AssemblerIcon, LibvgmIcon, VgmstreamIcon,
  AnvilEditionIcon, SpriteIcon, GamepadIcon, SparksIcon,
} from './icons.jsx';

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
            <a href="#kit">The Kit</a>
            <a href="#install">Install</a>
            <a href="#editions">Editions</a>
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
          <a className="cta cta-primary" href="#download">
            <DownloadIcon />Download foundry-26.04.iso
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
// The Kit — installed tools

const KIT = [
  {
    icon: WorldFoundryIcon,
    name: "World Foundry GDK",
    role: "Game Development Kit",
    desc: "Cross-platform engine and scripting toolchain. Author once; ship to 6502, 68k, ARM, x86 — and the modern desktop.",
    pkg: "world-foundry",
    version: "4.2.1",
  },
  {
    icon: BlenderIcon,
    name: "Blender",
    role: "3D Modeling · Animation",
    desc: "Full pipeline preconfigured with Wacom drivers, EEVEE Next, and the Foundry preset library for game-asset workflows.",
    pkg: "blender-foundry",
    version: "4.3",
  },
  {
    icon: MameIcon,
    name: "MAME",
    role: "Arcade Platform",
    desc: "Multiple Arcade Machine Emulator — reference hardware, debugger, and dev rig for nearly every arcade board ever made.",
    pkg: "mame",
    version: "0.272",
  },
  {
    icon: F9dasmIcon,
    name: "f9dasm",
    role: "Disassembler",
    desc: "Free 6800 / 6809 / 6309 / 68HC11 disassembler. Annotated output, symbol tables, and Motorola S-record support.",
    pkg: "f9dasm",
    version: "2.94",
  },
  {
    icon: AssemblerIcon,
    name: "65ax",
    role: "Assembler Suite",
    desc: "Cross-assembler for 6502, 65C02, 65816, and the W65C02 family. Macros, conditional assembly, listing output.",
    pkg: "65ax",
    version: "1.8.0",
  },
  {
    icon: LibvgmIcon,
    name: "libvgm",
    role: "Audio Library",
    desc: "Video Game Music library — emulators for YM2612, SN76489, NES APU, SID, and dozens more vintage sound chips.",
    pkg: "libvgm",
    version: "0.5.2",
  },
  {
    icon: VgmstreamIcon,
    name: "vgmstream",
    role: "Audio Extractor",
    desc: "Plays and extracts streamed audio from hundreds of game container formats. Plugin support for foobar, Audacious, deadbeef.",
    pkg: "vgmstream",
    version: "r1980",
  },
];

function Kit() {
  return (
    <section className="section" id="kit">
      <div className="shell">
        <div className="section-head">
          <div>
            <span className="section-num">№ 01 · The Kit</span>
            <h2 className="section-title">Struck into the iron.</h2>
          </div>
          <p className="section-blurb">
            Every Foundry install ships with the working tools of game
            development and digital art — preconfigured, signed, and
            updated through the same <code>apt</code> channels as the OS.
          </p>
        </div>

        <div className="kit-grid">
          {KIT.map((k) => {
            const Icon = k.icon;
            return (
              <div className="kit-cell" key={k.pkg}>
                <div className="kit-icon"><Icon size={64} /></div>
                <span className="kit-role">{k.role}</span>
                <h3 className="kit-name">{k.name}</h3>
                <p className="kit-desc">{k.desc}</p>
                <div className="kit-meta">
                  <span>{k.pkg}</span>
                  <span>v{k.version}</span>
                </div>
              </div>
            );
          })}
          <div className="kit-cell" style={{ display: "flex", alignItems: "center", justifyContent: "center", textAlign: "center" }}>
            <div>
              <div style={{ opacity: 0.5, marginBottom: 18 }}><SparksIcon size={48} /></div>
              <div style={{ fontFamily: "var(--font-mono)", fontSize: 11, letterSpacing: "0.2em", textTransform: "uppercase", color: "var(--ink-soft)", marginBottom: 10 }}>
                + 1,400 more
              </div>
              <p style={{ fontSize: 13, color: "var(--ink-soft)", margin: 0 }}>
                Krita, Aseprite, Godot, Tiled,<br />
                ScummVM, OpenMSX, OpenTTD,<br />
                Audacity, ffmpeg, ImageMagick...
              </p>
            </div>
          </div>
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
          <div className="codeblock">
            <div className="codeblock-head">
              <span>shell · root</span>
              <div className="dots"><i /><i /><i /></div>
            </div>
            <pre>
<span className="step">① Add the Foundry archive</span>
<span className="ember">curl</span> <span className="kw">-fsSL</span> <span className="str">https://foundrylinux.org/setup.sh</span> | <span className="ember">bash</span>

<span className="step">② Install</span>
<span className="ember">sudo apt install</span> foundry-anvil

<span className="cmt"># or pick a different edition:</span>
<span className="ember">sudo apt install</span> foundry-sprite     <span className="cmt"># + heavy graphics + audio</span>
<span className="ember">sudo apt install</span> foundry-atelier    <span className="cmt"># + everything (~10 GB)</span>
            </pre>
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
  return (
    <section className="section" id="editions">
      <div className="shell">
        <div className="section-head">
          <div>
            <span className="section-num">№ 03 · Editions</span>
            <h2 className="section-title">Three<br />marks.</h2>
          </div>
          <p className="section-blurb">
            One distribution, three casts. Same kernel, same archive — different
            sets of tools struck in from the first boot.
          </p>
        </div>

        <div className="editions">
          <div className="edition" data-flavor="anvil">
            <div className="edition-icon"><AnvilEditionIcon /></div>
            <span className="edition-tag">Edition · 26.04</span>
            <h3 className="edition-name">Anvil</h3>
            <p className="edition-desc">
              The full kit. World Foundry GDK, Blender, MAME, the assembler
              suite, the audio libraries — everything ready on first boot.
            </p>
            <div className="edition-pkg">
              <span>foundry-anvil</span>
              <span className="size">4.8 GB</span>
            </div>
          </div>

          <div className="edition" data-flavor="sprite">
            <div className="edition-icon"><SpriteIcon /></div>
            <span className="edition-tag">Edition · 26.04</span>
            <h3 className="edition-name">Sprite</h3>
            <p className="edition-desc">
              Retro-first. 65ax, f9dasm, libvgm, vgmstream, MAME, Aseprite,
              Tiled, ScummVM. For the 8-bit and 16-bit homebrew scene.
            </p>
            <div className="edition-pkg">
              <span>foundry-sprite</span>
              <span className="size">2.6 GB</span>
            </div>
          </div>

          <div className="edition" data-flavor="atelier">
            <div className="edition-icon"><GamepadIcon /></div>
            <span className="edition-tag">Edition · 26.04</span>
            <h3 className="edition-name">Atelier</h3>
            <p className="edition-desc">
              Artist-leaning. Blender, Krita, GIMP, Inkscape, Natron, OpenToonz,
              Wacom tuning, color-managed pipeline. Light on engines.
            </p>
            <div className="edition-pkg">
              <span>foundry-atelier</span>
              <span className="size">3.4 GB</span>
            </div>
          </div>
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
          <a href="#packages">Packages</a>
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

export { Topbar, Hero, Kit, Install, Editions, Foot };
