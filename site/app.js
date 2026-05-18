const TWEAK_DEFAULTS = (
  /*EDITMODE-BEGIN*/
  {
    "material": "chrome",
    "font": "big-shoulders",
    "wordmark": "foundry-linux",
    "background": "pure",
    "accent": "#ff5b1a"
  }
);
function hexToRgba(hex, a) {
  const h = hex.replace("#", "");
  const n = parseInt(h.length === 3 ? h.split("").map((c) => c + c).join("") : h, 16);
  return `rgba(${n >> 16 & 255}, ${n >> 8 & 255}, ${n & 255}, ${a})`;
}
function applyAccent(hex) {
  const r = document.documentElement.style;
  r.setProperty("--accent", hex);
  r.setProperty("--accent-soft", hexToRgba(hex, 0.18));
  r.setProperty("--accent-glow", hexToRgba(hex, 0.5));
}
function applyBg(mode) {
  document.body.setAttribute("data-bg", mode);
}
function applyFont(font) {
  const map = {
    "big-shoulders": '"Big Shoulders Display", "Impact", sans-serif',
    "cinzel": '"Cinzel", serif',
    "major-mono": '"Major Mono Display", monospace',
    "black-ops": '"Black Ops One", system-ui'
  };
  document.documentElement.style.setProperty("--font-wordmark", map[font] || map["big-shoulders"]);
}
function App() {
  const [t, setTweak] = useTweaks(TWEAK_DEFAULTS);
  React.useEffect(() => {
    applyAccent(t.accent);
  }, [t.accent]);
  React.useEffect(() => {
    applyBg(t.background);
  }, [t.background]);
  React.useEffect(() => {
    applyFont(t.font);
  }, [t.font]);
  return /* @__PURE__ */ React.createElement(React.Fragment, null, /* @__PURE__ */ React.createElement(Topbar, null), /* @__PURE__ */ React.createElement("main", null, /* @__PURE__ */ React.createElement(Hero, { material: t.material, font: t.font, wordmark: t.wordmark }), /* @__PURE__ */ React.createElement(Kit, null), /* @__PURE__ */ React.createElement(Install, null), /* @__PURE__ */ React.createElement(Editions, null)), /* @__PURE__ */ React.createElement(Foot, null), /* @__PURE__ */ React.createElement(TweaksPanel, { title: "Tweaks" }, /* @__PURE__ */ React.createElement(TweakSection, { label: "Wordmark" }, /* @__PURE__ */ React.createElement(
    TweakRadio,
    {
      label: "Material",
      value: t.material,
      options: [
        { value: "chrome", label: "Chrome" },
        { value: "brushed", label: "Brushed" },
        { value: "hammered", label: "Hammered" }
      ],
      onChange: (v) => setTweak("material", v)
    }
  ), /* @__PURE__ */ React.createElement(
    TweakSelect,
    {
      label: "Typeface",
      value: t.font,
      options: [
        { value: "big-shoulders", label: "Big Shoulders (industrial)" },
        { value: "cinzel", label: "Cinzel (engraved)" },
        { value: "black-ops", label: "Black Ops (stencil)" },
        { value: "major-mono", label: "Major Mono (technical)" }
      ],
      onChange: (v) => setTweak("font", v)
    }
  ), /* @__PURE__ */ React.createElement(
    TweakRadio,
    {
      label: "Mark",
      value: t.wordmark,
      options: [
        { value: "foundry-linux", label: "FOUNDRY LINUX" },
        { value: "foundry", label: "FOUNDRY" },
        { value: "anvil", label: "ANVIL" }
      ],
      onChange: (v) => setTweak("wordmark", v)
    }
  )), /* @__PURE__ */ React.createElement(TweakSection, { label: "Surface" }, /* @__PURE__ */ React.createElement(
    TweakRadio,
    {
      label: "Background",
      value: t.background,
      options: [
        { value: "pure", label: "Pure" },
        { value: "gradient", label: "Vignette" },
        { value: "textured", label: "Textured" }
      ],
      onChange: (v) => setTweak("background", v)
    }
  ), /* @__PURE__ */ React.createElement(
    TweakColor,
    {
      label: "Accent",
      value: t.accent,
      options: ["#ff5b1a", "#d4a23a", "#2e9fff", "#ec3b3b", "#bcbcbc"],
      onChange: (v) => setTweak("accent", v)
    }
  ))));
}
ReactDOM.createRoot(document.getElementById("root")).render(/* @__PURE__ */ React.createElement(App, null));
