function Topbar({ wordmark }) {
  return /* @__PURE__ */ React.createElement("header", { className: "topbar-wrap" }, /* @__PURE__ */ React.createElement("div", { className: "shell" }, /* @__PURE__ */ React.createElement("div", { className: "topbar" }, /* @__PURE__ */ React.createElement("div", { className: "topbar-mark" }, /* @__PURE__ */ React.createElement(FoundryMark, null), /* @__PURE__ */ React.createElement("b", null, "foundrylinux.org")), /* @__PURE__ */ React.createElement("nav", { className: "topbar-nav" }, /* @__PURE__ */ React.createElement("a", { href: "#kit" }, "The Kit"), /* @__PURE__ */ React.createElement("a", { href: "#install" }, "Install"), /* @__PURE__ */ React.createElement("a", { href: "#editions" }, "Editions"), /* @__PURE__ */ React.createElement("a", { href: "#docs" }, "Docs")), /* @__PURE__ */ React.createElement("div", { className: "topbar-version" }, /* @__PURE__ */ React.createElement("span", { className: "dot" }), 'FOUNDRY 26.04 \xB7 "ANVIL" \xB7 LTS'))));
}
function Hero({ material, font, wordmark }) {
  const [copied, setCopied] = React.useState(false);
  const copy = () => {
    navigator.clipboard?.writeText("sudo apt install foundry-base");
    setCopied(true);
    setTimeout(() => setCopied(false), 1400);
  };
  const lines = wordmark === "foundry" ? ["FOUNDRY"] : wordmark === "anvil" ? ["ANVIL"] : ["FOUNDRY", "LINUX"];
  return /* @__PURE__ */ React.createElement("section", { className: "hero" }, /* @__PURE__ */ React.createElement("div", { className: "hero-deco left" }, /* @__PURE__ */ React.createElement(GearStackIcon, null)), /* @__PURE__ */ React.createElement("div", { className: "shell" }, /* @__PURE__ */ React.createElement("div", { className: "hero-eyebrow" }, "a linux distribution \xB7 est. 2026"), /* @__PURE__ */ React.createElement("div", { className: "hero-anvil" }, /* @__PURE__ */ React.createElement(BigAnvilLogo, { width: 420 })), /* @__PURE__ */ React.createElement(
    "h1",
    {
      className: "wordmark",
      "data-material": material,
      "data-font": font,
      "data-mark": wordmark
    },
    lines.map((l, i) => /* @__PURE__ */ React.createElement("span", { key: i, className: "wordmark-line" }, l))
  ), /* @__PURE__ */ React.createElement("p", { className: "hero-tagline" }, "Forged for ", /* @__PURE__ */ React.createElement("em", null, "game makers, level designers, and digital artists"), " ", "of every kind \u2014 an Ubuntu LTS desktop with the tools of the trade struck into it from the first boot."), /* @__PURE__ */ React.createElement("div", { className: "apt-command" }, /* @__PURE__ */ React.createElement("span", { className: "prompt" }, "$"), /* @__PURE__ */ React.createElement("span", { className: "cmd" }, "sudo apt install foundry-base"), /* @__PURE__ */ React.createElement("button", { className: "copy", onClick: copy }, /* @__PURE__ */ React.createElement(CopyIcon, null), copied ? "COPIED" : "COPY")), /* @__PURE__ */ React.createElement("div", { className: "hero-actions" }, /* @__PURE__ */ React.createElement("a", { className: "cta cta-primary", href: "#download" }, /* @__PURE__ */ React.createElement(DownloadIcon, null), "Download foundry-26.04.iso"), /* @__PURE__ */ React.createElement("a", { className: "cta cta-secondary", href: "#install" }, "Set up the repo", /* @__PURE__ */ React.createElement(ArrowRightIcon, null)))));
}
const KIT = [
  {
    icon: WorldFoundryIcon,
    name: "World Foundry GDK",
    role: "Game Development Kit",
    desc: "Cross-platform engine and scripting toolchain. Author once; ship to 6502, 68k, ARM, x86 \u2014 and the modern desktop.",
    pkg: "world-foundry",
    version: "4.2.1"
  },
  {
    icon: BlenderIcon,
    name: "Blender",
    role: "3D Modeling \xB7 Animation",
    desc: "Full pipeline preconfigured with Wacom drivers, EEVEE Next, and the Foundry preset library for game-asset workflows.",
    pkg: "blender-foundry",
    version: "4.3"
  },
  {
    icon: MameIcon,
    name: "MAME",
    role: "Arcade Platform",
    desc: "Multiple Arcade Machine Emulator \u2014 reference hardware, debugger, and dev rig for nearly every arcade board ever made.",
    pkg: "mame",
    version: "0.272"
  },
  {
    icon: F9dasmIcon,
    name: "f9dasm",
    role: "Disassembler",
    desc: "Free 6800 / 6809 / 6309 / 68HC11 disassembler. Annotated output, symbol tables, and Motorola S-record support.",
    pkg: "f9dasm",
    version: "2.94"
  },
  {
    icon: AssemblerIcon,
    name: "65ax",
    role: "Assembler Suite",
    desc: "Cross-assembler for 6502, 65C02, 65816, and the W65C02 family. Macros, conditional assembly, listing output.",
    pkg: "65ax",
    version: "1.8.0"
  },
  {
    icon: LibvgmIcon,
    name: "libvgm",
    role: "Audio Library",
    desc: "Video Game Music library \u2014 emulators for YM2612, SN76489, NES APU, SID, and dozens more vintage sound chips.",
    pkg: "libvgm",
    version: "0.5.2"
  },
  {
    icon: VgmstreamIcon,
    name: "vgmstream",
    role: "Audio Extractor",
    desc: "Plays and extracts streamed audio from hundreds of game container formats. Plugin support for foobar, Audacious, deadbeef.",
    pkg: "vgmstream",
    version: "r1980"
  }
];
function Kit() {
  return /* @__PURE__ */ React.createElement("section", { className: "section", id: "kit" }, /* @__PURE__ */ React.createElement("div", { className: "shell" }, /* @__PURE__ */ React.createElement("div", { className: "section-head" }, /* @__PURE__ */ React.createElement("div", null, /* @__PURE__ */ React.createElement("span", { className: "section-num" }, "\u2116 01 \xB7 The Kit"), /* @__PURE__ */ React.createElement("h2", { className: "section-title" }, "Struck into the iron.")), /* @__PURE__ */ React.createElement("p", { className: "section-blurb" }, "Every Foundry install ships with the working tools of game development and digital art \u2014 preconfigured, signed, and updated through the same ", /* @__PURE__ */ React.createElement("code", null, "apt"), " channels as the OS.")), /* @__PURE__ */ React.createElement("div", { className: "kit-grid" }, KIT.map((k) => {
    const Icon = k.icon;
    return /* @__PURE__ */ React.createElement("div", { className: "kit-cell", key: k.pkg }, /* @__PURE__ */ React.createElement("div", { className: "kit-icon" }, /* @__PURE__ */ React.createElement(Icon, { size: 64 })), /* @__PURE__ */ React.createElement("span", { className: "kit-role" }, k.role), /* @__PURE__ */ React.createElement("h3", { className: "kit-name" }, k.name), /* @__PURE__ */ React.createElement("p", { className: "kit-desc" }, k.desc), /* @__PURE__ */ React.createElement("div", { className: "kit-meta" }, /* @__PURE__ */ React.createElement("span", null, k.pkg), /* @__PURE__ */ React.createElement("span", null, "v", k.version)));
  }), /* @__PURE__ */ React.createElement("div", { className: "kit-cell", style: { display: "flex", alignItems: "center", justifyContent: "center", textAlign: "center" } }, /* @__PURE__ */ React.createElement("div", null, /* @__PURE__ */ React.createElement("div", { style: { opacity: 0.5, marginBottom: 18 } }, /* @__PURE__ */ React.createElement(SparksIcon, { size: 48 })), /* @__PURE__ */ React.createElement("div", { style: { fontFamily: "var(--font-mono)", fontSize: 11, letterSpacing: "0.2em", textTransform: "uppercase", color: "var(--ink-soft)", marginBottom: 10 } }, "+ 1,400 more"), /* @__PURE__ */ React.createElement("p", { style: { fontSize: 13, color: "var(--ink-soft)", margin: 0 } }, "Krita, Aseprite, Godot, Tiled,", /* @__PURE__ */ React.createElement("br", null), "ScummVM, OpenMSX, OpenTTD,", /* @__PURE__ */ React.createElement("br", null), "Audacity, ffmpeg, ImageMagick..."))))));
}
function Install() {
  return /* @__PURE__ */ React.createElement("section", { className: "section", id: "install" }, /* @__PURE__ */ React.createElement("div", { className: "shell" }, /* @__PURE__ */ React.createElement("div", { className: "section-head" }, /* @__PURE__ */ React.createElement("div", null, /* @__PURE__ */ React.createElement("span", { className: "section-num" }, "\u2116 02 \xB7 Install"), /* @__PURE__ */ React.createElement("h2", { className: "section-title" }, "Add the forge", /* @__PURE__ */ React.createElement("br", null), "to your sources.")), /* @__PURE__ */ React.createElement("p", { className: "section-blurb" }, "Already running Ubuntu 26.04 LTS or any apt-compatible derivative? Add our signed repository and pull the Foundry kit piece by piece.")), /* @__PURE__ */ React.createElement("div", { className: "install-wrap" }, /* @__PURE__ */ React.createElement("div", { className: "codeblock" }, /* @__PURE__ */ React.createElement("div", { className: "codeblock-head" }, /* @__PURE__ */ React.createElement("span", null, "shell \xB7 root"), /* @__PURE__ */ React.createElement("div", { className: "dots" }, /* @__PURE__ */ React.createElement("i", null), /* @__PURE__ */ React.createElement("i", null), /* @__PURE__ */ React.createElement("i", null))), /* @__PURE__ */ React.createElement("pre", null, /* @__PURE__ */ React.createElement("span", { className: "step" }, "\u2460 Trust the keyring"), /* @__PURE__ */ React.createElement("span", { className: "ember" }, "curl"), " ", /* @__PURE__ */ React.createElement("span", { className: "kw" }, "-fsSL"), " ", /* @__PURE__ */ React.createElement("span", { className: "str" }, "https://apt.foundrylinux.org/keyring.gpg"), " \\ | ", /* @__PURE__ */ React.createElement("span", { className: "ember" }, "sudo tee"), " /etc/apt/keyrings/foundry.gpg > /dev/null", /* @__PURE__ */ React.createElement("span", { className: "step" }, "\u2461 Wire up the repository"), /* @__PURE__ */ React.createElement("span", { className: "ember" }, "echo"), " ", /* @__PURE__ */ React.createElement("span", { className: "str" }, '"deb [signed-by=/etc/apt/keyrings/foundry.gpg] \\ https://apt.foundrylinux.org stable main"'), " \\ | ", /* @__PURE__ */ React.createElement("span", { className: "ember" }, "sudo tee"), " /etc/apt/sources.list.d/foundry.list", /* @__PURE__ */ React.createElement("span", { className: "step" }, "\u2462 Update and install"), /* @__PURE__ */ React.createElement("span", { className: "ember" }, "sudo apt update"), " && ", /* @__PURE__ */ React.createElement("span", { className: "ember" }, "sudo apt install"), " foundry-base", /* @__PURE__ */ React.createElement("span", { className: "cmt" }, "# or pick \xE0 la carte:"), /* @__PURE__ */ React.createElement("span", { className: "ember" }, "sudo apt install"), " world-foundry blender-foundry mame 65ax libvgm")), /* @__PURE__ */ React.createElement("div", { className: "install-side" }, /* @__PURE__ */ React.createElement("h3", null, "Signed by the smith."), /* @__PURE__ */ React.createElement("p", null, "Every package in the Foundry archive is signed with our long-term maintainer key. Verify the fingerprint before adding the keyring to a production machine."), /* @__PURE__ */ React.createElement("div", { className: "fingerprint" }, /* @__PURE__ */ React.createElement("b", null, "archive key \xB7 ed25519"), /* @__PURE__ */ React.createElement("code", null, "F0D9 7E1A \xB7 C5B2 4A87 \xB7 6E3F 218D \xB7 9C04 BB31")), /* @__PURE__ */ React.createElement("h3", { style: { marginTop: 32 } }, "System requirements"), /* @__PURE__ */ React.createElement("p", null, "The full ISO is 4.8 GB."), /* @__PURE__ */ React.createElement("ul", { className: "requirements" }, /* @__PURE__ */ React.createElement("li", null, /* @__PURE__ */ React.createElement("span", { className: "req-key" }, "CPU"), /* @__PURE__ */ React.createElement("span", null, "x86_64 \xB7 ARM64")), /* @__PURE__ */ React.createElement("li", null, /* @__PURE__ */ React.createElement("span", { className: "req-key" }, "Memory"), /* @__PURE__ */ React.createElement("span", null, "4 GB minimum \xB7 8 GB recommended")), /* @__PURE__ */ React.createElement("li", null, /* @__PURE__ */ React.createElement("span", { className: "req-key" }, "Storage"), /* @__PURE__ */ React.createElement("span", null, "20 GB free")), /* @__PURE__ */ React.createElement("li", null, /* @__PURE__ */ React.createElement("span", { className: "req-key" }, "GPU"), /* @__PURE__ */ React.createElement("span", null, "Vulkan 1.2 capable")), /* @__PURE__ */ React.createElement("li", null, /* @__PURE__ */ React.createElement("span", { className: "req-key" }, "Base"), /* @__PURE__ */ React.createElement("span", null, "Ubuntu 26.04 LTS")))))));
}
function Editions() {
  return /* @__PURE__ */ React.createElement("section", { className: "section", id: "editions" }, /* @__PURE__ */ React.createElement("div", { className: "shell" }, /* @__PURE__ */ React.createElement("div", { className: "section-head" }, /* @__PURE__ */ React.createElement("div", null, /* @__PURE__ */ React.createElement("span", { className: "section-num" }, "\u2116 03 \xB7 Editions"), /* @__PURE__ */ React.createElement("h2", { className: "section-title" }, "Three", /* @__PURE__ */ React.createElement("br", null), "pours.")), /* @__PURE__ */ React.createElement("p", { className: "section-blurb" }, "One distribution, three shapes. Same kernel, same archive \u2014 different sets of tools struck in from the first boot.")), /* @__PURE__ */ React.createElement("div", { className: "editions" }, /* @__PURE__ */ React.createElement("div", { className: "edition", "data-flavor": "anvil" }, /* @__PURE__ */ React.createElement("div", { className: "edition-icon" }, /* @__PURE__ */ React.createElement(AnvilEditionIcon, null)), /* @__PURE__ */ React.createElement("span", { className: "edition-tag" }, "Edition \xB7 26.04"), /* @__PURE__ */ React.createElement("h3", { className: "edition-name" }, "Anvil"), /* @__PURE__ */ React.createElement("p", { className: "edition-desc" }, "The full kit. World Foundry GDK, Blender, MAME, the assembler suite, the audio libraries \u2014 everything ready on first boot."), /* @__PURE__ */ React.createElement("div", { className: "edition-pkg" }, /* @__PURE__ */ React.createElement("span", null, "foundry-anvil"), /* @__PURE__ */ React.createElement("span", { className: "size" }, "4.8 GB"))), /* @__PURE__ */ React.createElement("div", { className: "edition", "data-flavor": "sprite" }, /* @__PURE__ */ React.createElement("div", { className: "edition-icon" }, /* @__PURE__ */ React.createElement(SpriteIcon, null)), /* @__PURE__ */ React.createElement("span", { className: "edition-tag" }, "Edition \xB7 26.04"), /* @__PURE__ */ React.createElement("h3", { className: "edition-name" }, "Sprite"), /* @__PURE__ */ React.createElement("p", { className: "edition-desc" }, "Retro-first. 65ax, f9dasm, libvgm, vgmstream, MAME, Aseprite, Tiled, ScummVM. For the 8-bit and 16-bit homebrew scene."), /* @__PURE__ */ React.createElement("div", { className: "edition-pkg" }, /* @__PURE__ */ React.createElement("span", null, "foundry-sprite"), /* @__PURE__ */ React.createElement("span", { className: "size" }, "2.6 GB"))), /* @__PURE__ */ React.createElement("div", { className: "edition", "data-flavor": "atelier" }, /* @__PURE__ */ React.createElement("div", { className: "edition-icon" }, /* @__PURE__ */ React.createElement(GamepadIcon, null)), /* @__PURE__ */ React.createElement("span", { className: "edition-tag" }, "Edition \xB7 26.04"), /* @__PURE__ */ React.createElement("h3", { className: "edition-name" }, "Atelier"), /* @__PURE__ */ React.createElement("p", { className: "edition-desc" }, "Artist-leaning. Blender, Krita, GIMP, Inkscape, Natron, OpenToonz, Wacom tuning, color-managed pipeline. Light on engines."), /* @__PURE__ */ React.createElement("div", { className: "edition-pkg" }, /* @__PURE__ */ React.createElement("span", null, "foundry-atelier"), /* @__PURE__ */ React.createElement("span", { className: "size" }, "3.4 GB"))))));
}
function Foot() {
  return /* @__PURE__ */ React.createElement("footer", { className: "shell" }, /* @__PURE__ */ React.createElement("div", { className: "foot" }, /* @__PURE__ */ React.createElement("div", { className: "foot-mark" }, /* @__PURE__ */ React.createElement(FoundryMark, { size: 28 }), /* @__PURE__ */ React.createElement("span", null, "FOUNDRY \xB7 LINUX")), /* @__PURE__ */ React.createElement("nav", { className: "foot-links" }, /* @__PURE__ */ React.createElement("a", { href: "#docs" }, "Docs"), /* @__PURE__ */ React.createElement("a", { href: "#packages" }, "Packages"), /* @__PURE__ */ React.createElement("a", { href: "#mirrors" }, "Mirrors"), /* @__PURE__ */ React.createElement("a", { href: "#irc" }, "IRC"), /* @__PURE__ */ React.createElement("a", { href: "#git" }, "Git"), /* @__PURE__ */ React.createElement("a", { href: "#bugs" }, "Bugs")), /* @__PURE__ */ React.createElement("div", { className: "foot-meta" }, "Released under GPLv3", /* @__PURE__ */ React.createElement("br", null), "Forged in the open \xB7 MMXXVI")));
}
Object.assign(window, { Topbar, Hero, Kit, Install, Editions, Foot });
