---
name: feedback-color-swatches
description: "Always include inline color swatches when presenting colors (hex, RGB, or by name)"
metadata: 
  node_type: memory
  type: feedback
  originSessionId: 62e6a4c3-be50-4a20-85ec-c25ea402f2d3
---

Always include a rendered color swatch alongside any color value — hex, RGB, or even a color name alone. Use an inline HTML span:

```html
<span style="display:inline-block;width:12px;height:12px;background:#ff5b1a;border-radius:2px;vertical-align:middle"></span>
```

Place the swatch immediately before or after the hex value in tables and prose.

**Why:** User explicitly requested this as a standing rule after presenting a color token table without swatches.

**How to apply:** Any time a color (by hex, RGB, name, or CSS variable) is shown in a response, table, or doc — add the swatch. Sweep existing docs when this rule is first established.

**Placement rule for md-to-pdf.sh rendering:** Swatches must be outside all backtick code spans. The pattern `` `code` <span...></span> `#hex` `` is safe. Swatches accidentally placed inside a code span (e.g. `` `gradient <span...></span>` ``) will render as escaped HTML. The script now protects `<span>` elements before the code-span regex, but correct placement is still required. See fix: `python-tui-lib` commit `1766121`.

## Foundry Linux design tokens

| Token | Swatch | Hex | RGB | Role |
|---|---|---|---|---|
| `--color-surface` | <span style="display:inline-block;width:12px;height:12px;background:#0a0a0a;border-radius:2px;vertical-align:middle;border:1px solid #333"></span> | `#0a0a0a` | 10, 10, 10 | Window / panel background |
| `--color-surface-raised` | <span style="display:inline-block;width:12px;height:12px;background:#1a1a1a;border-radius:2px;vertical-align:middle;border:1px solid #333"></span> | `#1a1a1a` | 26, 26, 26 | Button / raised surface |
| `--color-view-bg` | <span style="display:inline-block;width:12px;height:12px;background:#0d0d0d;border-radius:2px;vertical-align:middle;border:1px solid #333"></span> | `#0d0d0d` | 13, 13, 13 | Content areas |
| `--color-on-surface` | <span style="display:inline-block;width:12px;height:12px;background:#f7f7f7;border-radius:2px;vertical-align:middle;border:1px solid #333"></span> | `#f7f7f7` | 247, 247, 247 | Primary text |
| `--color-muted` | <span style="display:inline-block;width:12px;height:12px;background:#909090;border-radius:2px;vertical-align:middle;border:1px solid #333"></span> | `#909090` | 144, 144, 144 | Inactive / placeholder text |
| `--color-accent` | <span style="display:inline-block;width:12px;height:12px;background:#ff5b1a;border-radius:2px;vertical-align:middle"></span> | `#ff5b1a` | 255, 91, 26 | Focus rings, selection bg, links |
| `--color-accent-dark` | <span style="display:inline-block;width:12px;height:12px;background:#c8410a;border-radius:2px;vertical-align:middle"></span> | `#c8410a` | 200, 65, 10 | Alt/hover selection |
| `--color-border` | <span style="display:inline-block;width:12px;height:12px;background:#141414;border-radius:2px;vertical-align:middle;border:1px solid #333"></span> | `#141414` | 20, 20, 20 | Alt backgrounds, borders |
