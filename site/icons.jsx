// Line-art icons — strokes only, currentColor.
// Drawn as silhouettes/line drawings to match the "everything line art" brief.

const I = {
  stroke: { fill: "none", stroke: "currentColor", strokeWidth: 1.4, strokeLinecap: "round", strokeLinejoin: "round" },
  strokeThin: { fill: "none", stroke: "currentColor", strokeWidth: 1, strokeLinecap: "round", strokeLinejoin: "round" },
  strokeThick: { fill: "none", stroke: "currentColor", strokeWidth: 1.8, strokeLinecap: "round", strokeLinejoin: "round" },
};

// Anvil — the centerpiece mark
const AnvilIcon = ({ size = 24 }) => (
  <svg viewBox="0 0 64 64" width={size} height={size} aria-hidden="true">
    <g {...I.stroke}>
      {/* anvil body */}
      <path d="M8 22 L56 22 L52 30 L46 30 L46 38 L18 38 L18 30 L12 30 Z" />
      {/* horn */}
      <path d="M8 22 L4 24 L4 28 L8 30" />
      {/* base */}
      <path d="M22 38 L20 48 L44 48 L42 38" />
      <path d="M16 48 L48 48 L48 54 L16 54 Z" />
      {/* top groove */}
      <line x1="14" y1="26" x2="50" y2="26" />
    </g>
  </svg>
);

// Hammer
const HammerIcon = ({ size = 24 }) => (
  <svg viewBox="0 0 64 64" width={size} height={size} aria-hidden="true">
    <g {...I.stroke}>
      <path d="M14 14 L34 14 L34 26 L14 26 Z" />
      <line x1="14" y1="18" x2="34" y2="18" />
      <line x1="14" y1="22" x2="34" y2="22" />
      <path d="M22 26 L28 26 L46 52 L40 56 Z" />
    </g>
  </svg>
);

// Sparks (radiating lines)
const SparksIcon = ({ size = 24 }) => (
  <svg viewBox="0 0 64 64" width={size} height={size} aria-hidden="true">
    <g {...I.strokeThin}>
      <circle cx="32" cy="32" r="2" />
      {Array.from({ length: 12 }).map((_, i) => {
        const a = (i / 12) * Math.PI * 2;
        const r1 = i % 2 === 0 ? 10 : 7;
        const r2 = i % 2 === 0 ? 22 : 14;
        return (
          <line key={i}
                x1={32 + Math.cos(a) * r1}
                y1={32 + Math.sin(a) * r1}
                x2={32 + Math.cos(a) * r2}
                y2={32 + Math.sin(a) * r2} />
        );
      })}
    </g>
  </svg>
);

// World Foundry GDK — globe + grid (engine)
const WorldFoundryIcon = ({ size = 64 }) => (
  <svg viewBox="0 0 64 64" width={size} height={size} aria-hidden="true">
    <g {...I.stroke}>
      <circle cx="32" cy="32" r="20" />
      <ellipse cx="32" cy="32" rx="20" ry="9" />
      <ellipse cx="32" cy="32" rx="9" ry="20" />
      <line x1="12" y1="32" x2="52" y2="32" />
      <line x1="32" y1="12" x2="32" y2="52" />
      {/* tick marks like a level grid */}
      <line x1="22" y1="14" x2="22" y2="18" />
      <line x1="42" y1="46" x2="42" y2="50" />
      <line x1="14" y1="22" x2="18" y2="22" />
      <line x1="46" y1="42" x2="50" y2="42" />
    </g>
  </svg>
);

// Blender — abstract polygon / wireframe cube
const BlenderIcon = ({ size = 64 }) => (
  <svg viewBox="0 0 64 64" width={size} height={size} aria-hidden="true">
    <g {...I.stroke}>
      {/* a wireframe cube in iso */}
      <polygon points="32,10 52,22 52,42 32,54 12,42 12,22" />
      <line x1="32" y1="10" x2="32" y2="32" />
      <line x1="12" y1="22" x2="32" y2="32" />
      <line x1="52" y1="22" x2="32" y2="32" />
      <line x1="32" y1="32" x2="32" y2="54" />
      {/* edge segments to suggest faceting */}
      <line x1="22" y1="16" x2="42" y2="16" opacity="0.5" />
      <line x1="22" y1="48" x2="42" y2="48" opacity="0.5" />
    </g>
  </svg>
);

// MAME — arcade cabinet
const MameIcon = ({ size = 64 }) => (
  <svg viewBox="0 0 64 64" width={size} height={size} aria-hidden="true">
    <g {...I.stroke}>
      <path d="M18 8 L46 8 L46 14 L48 16 L48 50 L46 52 L46 58 L18 58 L18 52 L16 50 L16 16 L18 14 Z" />
      {/* screen */}
      <rect x="22" y="16" width="20" height="14" />
      {/* control panel slope */}
      <path d="M16 34 L48 34 L46 40 L18 40 Z" />
      {/* joystick + buttons */}
      <circle cx="24" cy="44" r="2" />
      <circle cx="32" cy="44" r="1.5" />
      <circle cx="38" cy="44" r="1.5" />
      {/* coin slot */}
      <line x1="28" y1="50" x2="36" y2="50" strokeWidth="2" />
    </g>
  </svg>
);

// f9dasm — disassembler / hex view
const F9dasmIcon = ({ size = 64 }) => (
  <svg viewBox="0 0 64 64" width={size} height={size} aria-hidden="true">
    <g {...I.stroke}>
      <rect x="10" y="14" width="44" height="36" />
      {/* address column divider */}
      <line x1="22" y1="14" x2="22" y2="50" />
      {/* hex rows */}
      <line x1="12" y1="22" x2="20" y2="22" strokeDasharray="2 2" />
      <line x1="12" y1="30" x2="20" y2="30" strokeDasharray="2 2" />
      <line x1="12" y1="38" x2="20" y2="38" strokeDasharray="2 2" />
      <line x1="12" y1="46" x2="20" y2="46" strokeDasharray="2 2" />
      {/* code lines */}
      <line x1="26" y1="22" x2="50" y2="22" />
      <line x1="26" y1="30" x2="44" y2="30" />
      <line x1="26" y1="38" x2="48" y2="38" />
      <line x1="26" y1="46" x2="40" y2="46" />
      {/* arrow */}
      <path d="M24 30 L26 30 M24 28 L26 30 L24 32" strokeWidth="1.2" />
    </g>
  </svg>
);

// 65ax — assembler / chip
const AssemblerIcon = ({ size = 64 }) => (
  <svg viewBox="0 0 64 64" width={size} height={size} aria-hidden="true">
    <g {...I.stroke}>
      {/* chip body */}
      <rect x="18" y="18" width="28" height="28" />
      <rect x="24" y="24" width="16" height="16" />
      {/* pins */}
      {[22, 28, 34, 40].map(y => (
        <g key={y}>
          <line x1="10" y1={y} x2="18" y2={y} />
          <line x1="46" y1={y} x2="54" y2={y} />
        </g>
      ))}
      {[22, 28, 34, 40].map(x => (
        <g key={x}>
          <line x1={x} y1="10" x2={x} y2="18" />
          <line x1={x} y1="46" x2={x} y2="54" />
        </g>
      ))}
      {/* notch marker */}
      <circle cx="22" cy="22" r="1.2" fill="currentColor" />
    </g>
  </svg>
);

// libvgm — waveform / sound chip
const LibvgmIcon = ({ size = 64 }) => (
  <svg viewBox="0 0 64 64" width={size} height={size} aria-hidden="true">
    <g {...I.stroke}>
      <rect x="8" y="20" width="48" height="24" />
      {/* waveform */}
      <path d="M12 32 L16 32 L18 24 L22 40 L26 24 L30 40 L34 28 L38 36 L42 30 L46 34 L52 32" />
      {/* outputs */}
      <line x1="14" y1="48" x2="14" y2="54" />
      <line x1="22" y1="48" x2="22" y2="54" />
      <line x1="42" y1="48" x2="42" y2="54" />
      <line x1="50" y1="48" x2="50" y2="54" />
      <line x1="14" y1="16" x2="14" y2="10" />
      <line x1="50" y1="16" x2="50" y2="10" />
    </g>
  </svg>
);

// vgmstream — flowing wave / audio stream
const VgmstreamIcon = ({ size = 64 }) => (
  <svg viewBox="0 0 64 64" width={size} height={size} aria-hidden="true">
    <g {...I.stroke}>
      {/* outer disc */}
      <circle cx="32" cy="32" r="22" />
      {/* inner waveform circle */}
      <circle cx="32" cy="32" r="4" />
      {/* spiral stream */}
      <path d="M32 14 Q44 18 48 32 Q44 46 32 50 Q20 46 16 32 Q20 18 32 14" opacity="0.6" />
      <path d="M32 20 Q40 24 42 32 Q40 40 32 44 Q24 40 22 32 Q24 24 32 20" opacity="0.4" />
      {/* tick marks */}
      <line x1="32" y1="6" x2="32" y2="10" />
      <line x1="32" y1="54" x2="32" y2="58" />
      <line x1="6" y1="32" x2="10" y2="32" />
      <line x1="54" y1="32" x2="58" y2="32" />
    </g>
  </svg>
);

// Gamepad — for editions
const GamepadIcon = ({ size = 42 }) => (
  <svg viewBox="0 0 64 64" width={size} height={size} aria-hidden="true">
    <g {...I.stroke}>
      <path d="M16 24 Q8 24 8 36 Q8 48 16 48 L20 44 L44 44 L48 48 Q56 48 56 36 Q56 24 48 24 Z" />
      <circle cx="20" cy="34" r="3" />
      <line x1="17" y1="34" x2="23" y2="34" />
      <line x1="20" y1="31" x2="20" y2="37" />
      <circle cx="42" cy="32" r="1.4" />
      <circle cx="46" cy="36" r="1.4" />
      <circle cx="38" cy="36" r="1.4" />
    </g>
  </svg>
);

// Diamond/anvil mark for ANVIL edition
const AnvilEditionIcon = ({ size = 42 }) => (
  <svg viewBox="0 0 64 64" width={size} height={size} aria-hidden="true">
    <g {...I.strokeThick}>
      <path d="M6 22 L58 22 L52 32 L46 32 L46 38 L18 38 L18 32 L12 32 Z" />
      <path d="M22 38 L20 50 L44 50 L42 38" />
      <path d="M14 50 L50 50 L50 56 L14 56 Z" />
      <line x1="8" y1="14" x2="6" y2="8" />
      <line x1="14" y1="14" x2="14" y2="6" />
      <line x1="20" y1="14" x2="22" y2="8" />
    </g>
  </svg>
);

// Pixel / sprite — for retro tools
const SpriteIcon = ({ size = 42 }) => (
  <svg viewBox="0 0 64 64" width={size} height={size} aria-hidden="true">
    <g {...I.stroke}>
      <rect x="10" y="10" width="44" height="44" />
      {[18, 26, 34, 42, 50].map(x => <line key={"v"+x} x1={x} y1="10" x2={x} y2="54" opacity="0.4" />)}
      {[18, 26, 34, 42, 50].map(y => <line key={"h"+y} x1="10" y1={y} x2="54" y2={y} opacity="0.4" />)}
      {/* sprite shape */}
      <rect x="26" y="18" width="12" height="4" fill="currentColor" stroke="none" />
      <rect x="22" y="22" width="20" height="8" fill="currentColor" stroke="none" />
      <rect x="18" y="30" width="28" height="4" fill="currentColor" stroke="none" />
      <rect x="22" y="34" width="6" height="4" fill="currentColor" stroke="none" />
      <rect x="36" y="34" width="6" height="4" fill="currentColor" stroke="none" />
      <rect x="18" y="38" width="6" height="4" fill="currentColor" stroke="none" />
      <rect x="40" y="38" width="6" height="4" fill="currentColor" stroke="none" />
    </g>
  </svg>
);

// BIG ANVIL — bold graphic silhouette, symmetric, half-lit / half-in-shadow.
// Strong flat shapes, no gradients, no detail noise.
const BigAnvilLogo = ({ width = 420 }) => {
  // Symmetric three-stack profile: wide face, narrow waist, wider base.
  // viewBox 400×320; vertical axis of symmetry at x=200.
  const anvilPath =
    "M 64 40 L 336 40 L 336 116 L 252 116 L 252 198 L 360 198 L 360 282 L 40 282 L 40 198 L 148 198 L 148 116 L 64 116 Z";

  // hardy hole (square) + pritchel hole (round) centered on the face top
  const hardyX = 184, hardyY = 60, hardyW = 18;
  const pritchelX = 232, pritchelY = 69, pritchelR = 6;

  return (
    <svg viewBox="0 0 400 320" width={width} height={width * 320 / 400} aria-hidden="true">
      <defs>
        <clipPath id="anvil-lit"><rect x="0" y="0" width="200" height="320" /></clipPath>
        <clipPath id="anvil-shadow"><rect x="200" y="0" width="200" height="320" /></clipPath>
      </defs>

      {/* drop shadow on the ground */}
      <ellipse cx="200" cy="298" rx="180" ry="6" fill="#000" opacity="0.7" />
      <ellipse cx="200" cy="300" rx="220" ry="3" fill="#000" opacity="0.5" />

      {/* LIT HALF — bright silver flat fill */}
      <g clipPath="url(#anvil-lit)">
        <path d={anvilPath} fill="#ededed" />
        {/* punch the holes only on the lit side (left half cuts both) */}
        <rect x={hardyX} y={hardyY} width={hardyW} height={hardyW} fill="#000" />
        <circle cx={pritchelX} cy={pritchelY} r={pritchelR} fill="#000" />
      </g>

      {/* SHADOW HALF — hairline silver outline only, body falls into the dark */}
      <g clipPath="url(#anvil-shadow)">
        <path
          d={anvilPath}
          fill="none"
          stroke="#7a7a7a"
          strokeWidth="1.5"
          strokeLinejoin="miter"
        />
        {/* faint trace of the holes on the shadow side for completion */}
        <rect x={hardyX} y={hardyY} width={hardyW} height={hardyW} fill="none" stroke="#3a3a3a" strokeWidth="1" />
        <circle cx={pritchelX} cy={pritchelY} r={pritchelR} fill="none" stroke="#3a3a3a" strokeWidth="1" />
      </g>

      {/* The seam — sharp light/dark transition down the center.
          A 1px brighter highlight on the lit side suggests the catch of light. */}
      <line x1="200" y1="40"  x2="200" y2="116" stroke="#ffffff" strokeWidth="1" opacity="0.6" />
      <line x1="200" y1="198" x2="200" y2="282" stroke="#ffffff" strokeWidth="1" opacity="0.6" />
      <line x1="200" y1="116" x2="200" y2="198" stroke="#ffffff" strokeWidth="1" opacity="0.4" />
    </svg>
  );
};

// Small mark for topbar / footer
const FoundryMark = ({ size = 22 }) => (
  <svg viewBox="0 0 32 32" width={size} height={size} aria-hidden="true">
    <g fill="none" stroke="currentColor" strokeWidth="1.4" strokeLinecap="round" strokeLinejoin="round">
      {/* mini anvil */}
      <path d="M4 12 L28 12 L26 16 L22 16 L22 20 L10 20 L10 16 L6 16 Z" />
      <path d="M12 20 L11 24 L21 24 L20 20" />
      <path d="M9 24 L23 24 L23 27 L9 27 Z" />
      {/* spark */}
      <line x1="16" y1="4" x2="16" y2="8" />
      <line x1="11" y1="6" x2="13" y2="9" />
      <line x1="21" y1="6" x2="19" y2="9" />
    </g>
  </svg>
);

// Download arrow
const DownloadIcon = () => (
  <svg viewBox="0 0 16 16" fill="none" stroke="currentColor" strokeWidth="1.4" strokeLinecap="round" strokeLinejoin="round">
    <path d="M8 2 L8 11 M4 7 L8 11 L12 7 M3 13 L13 13" />
  </svg>
);

const CopyIcon = () => (
  <svg viewBox="0 0 16 16" fill="none" stroke="currentColor" strokeWidth="1.4" strokeLinecap="round" strokeLinejoin="round">
    <rect x="5" y="5" width="9" height="9" />
    <path d="M5 11 L2 11 L2 2 L11 2 L11 5" />
  </svg>
);

const ArrowRightIcon = () => (
  <svg viewBox="0 0 16 16" fill="none" stroke="currentColor" strokeWidth="1.4" strokeLinecap="round" strokeLinejoin="round">
    <path d="M3 8 L13 8 M9 4 L13 8 L9 12" />
  </svg>
);

// Decorative scene — hammer striking anvil, sparks flying
const ForgeSceneIcon = () => (
  <svg viewBox="0 0 200 200" aria-hidden="true">
    <g fill="none" stroke="currentColor" strokeWidth="1" strokeLinecap="round" strokeLinejoin="round">
      {/* anvil */}
      <path d="M40 110 L160 110 L150 128 L130 128 L130 140 L70 140 L70 128 L50 128 Z" />
      <path d="M75 140 L70 165 L130 165 L125 140" />
      <path d="M55 165 L145 165 L145 178 L55 178 Z" />
      {/* hammer in motion */}
      <g transform="rotate(-25 100 75)">
        <rect x="80" y="40" width="40" height="22" />
        <rect x="120" y="46" width="6" height="10" />
        <rect x="74" y="46" width="6" height="10" />
        <path d="M100 62 L100 90" strokeWidth="3" />
      </g>
      {/* sparks */}
      <g strokeWidth="0.8">
        {[
          [80, 100, -30, 12], [120, 100, 30, 12],
          [70, 95, -45, 18], [130, 95, 45, 18],
          [90, 88, -20, 22], [110, 88, 20, 22],
          [100, 82, 0, 26],
          [60, 105, -55, 14], [140, 105, 55, 14],
        ].map(([x, y, ang, len], i) => {
          const r = ang * Math.PI / 180;
          return <line key={i} x1={x} y1={y} x2={x + Math.cos(r - Math.PI/2) * len} y2={y + Math.sin(r - Math.PI/2) * len} />;
        })}
        {[[88, 78], [112, 78], [100, 70], [74, 90], [126, 90]].map(([x, y], i) => (
          <circle key={"c"+i} cx={x} cy={y} r="0.8" fill="currentColor" />
        ))}
      </g>
    </g>
  </svg>
);

// Decorative left-side gear / cog stack
const GearStackIcon = () => (
  <svg viewBox="0 0 200 200" aria-hidden="true">
    <g fill="none" stroke="currentColor" strokeWidth="1" strokeLinecap="round" strokeLinejoin="round">
      {/* gear 1 */}
      <g transform="translate(60 60)">
        <circle r="36" />
        <circle r="22" />
        <circle r="8" />
        {Array.from({ length: 12 }).map((_, i) => {
          const a = (i / 12) * Math.PI * 2;
          return <line key={i} x1={Math.cos(a) * 36} y1={Math.sin(a) * 36} x2={Math.cos(a) * 44} y2={Math.sin(a) * 44} strokeWidth="1.5" />;
        })}
      </g>
      {/* gear 2 (smaller, offset) */}
      <g transform="translate(140 130)">
        <circle r="26" />
        <circle r="14" />
        <circle r="5" />
        {Array.from({ length: 10 }).map((_, i) => {
          const a = (i / 10) * Math.PI * 2;
          return <line key={i} x1={Math.cos(a) * 26} y1={Math.sin(a) * 26} x2={Math.cos(a) * 32} y2={Math.sin(a) * 32} strokeWidth="1.5" />;
        })}
      </g>
    </g>
  </svg>
);

const TorrentIcon = () => (
  <svg viewBox="0 0 16 16" width="14" height="14" fill="none" stroke="currentColor" strokeWidth="1.5" strokeLinecap="round" strokeLinejoin="round" aria-hidden="true">
    <path d="M8 2 C4.7 2 2 4.7 2 8 C2 11.3 4.7 14 8 14 C11.3 14 14 11.3 14 8 C14 6.1 13.1 4.4 11.7 3.3" />
    <path d="M8 5 L8 11 M5.5 8.5 L8 11 L10.5 8.5" />
  </svg>
);

const MagnetIcon = () => (
  <svg viewBox="0 0 16 16" width="14" height="14" fill="none" stroke="currentColor" strokeWidth="1.5" strokeLinecap="round" strokeLinejoin="round" aria-hidden="true">
    <path d="M3 3 L3 9 C3 11.8 5.2 14 8 14 C10.8 14 13 11.8 13 9 L13 3" />
    <line x1="3" y1="3" x2="6" y2="3" />
    <line x1="13" y1="3" x2="10" y2="3" />
    <line x1="6" y1="3" x2="6" y2="7" />
    <line x1="10" y1="3" x2="10" y2="7" />
  </svg>
);

// System requirement icons — same stroke style, 32×32 viewBox for smaller use
const ReqCpuIcon = ({ size = 32 }) => (
  <svg viewBox="0 0 32 32" width={size} height={size} aria-hidden="true">
    <g {...I.stroke}>
      <rect x="10" y="10" width="12" height="12" />
      <rect x="13" y="13" width="6" height="6" />
      <line x1="13" y1="10" x2="13" y2="6" /><line x1="16" y1="10" x2="16" y2="6" /><line x1="19" y1="10" x2="19" y2="6" />
      <line x1="13" y1="22" x2="13" y2="26" /><line x1="16" y1="22" x2="16" y2="26" /><line x1="19" y1="22" x2="19" y2="26" />
      <line x1="10" y1="13" x2="6" y2="13" /><line x1="10" y1="16" x2="6" y2="16" /><line x1="10" y1="19" x2="6" y2="19" />
      <line x1="22" y1="13" x2="26" y2="13" /><line x1="22" y1="16" x2="26" y2="16" /><line x1="22" y1="19" x2="26" y2="19" />
    </g>
  </svg>
);

const ReqMemoryIcon = ({ size = 32 }) => (
  <svg viewBox="0 0 32 32" width={size} height={size} aria-hidden="true">
    <g {...I.stroke}>
      <rect x="4" y="10" width="24" height="12" />
      <line x1="8" y1="10" x2="8" y2="22" />
      <line x1="12" y1="10" x2="12" y2="22" />
      <line x1="16" y1="10" x2="16" y2="22" />
      <line x1="20" y1="10" x2="20" y2="22" />
      <line x1="24" y1="10" x2="24" y2="22" />
      <line x1="8" y1="22" x2="8" y2="25" />
      <line x1="12" y1="22" x2="12" y2="25" />
      <line x1="20" y1="22" x2="20" y2="25" />
      <line x1="24" y1="22" x2="24" y2="25" />
    </g>
  </svg>
);

const ReqStorageIcon = ({ size = 32 }) => (
  <svg viewBox="0 0 32 32" width={size} height={size} aria-hidden="true">
    <g {...I.stroke}>
      <rect x="5" y="8" width="22" height="16" rx="2" />
      <line x1="5" y1="19" x2="27" y2="19" />
      <circle cx="22" cy="22" r="1.5" />
      <line x1="8" y1="13" x2="16" y2="13" />
      <line x1="8" y1="16" x2="13" y2="16" />
    </g>
  </svg>
);

const ReqGpuIcon = ({ size = 32 }) => (
  <svg viewBox="0 0 32 32" width={size} height={size} aria-hidden="true">
    <g {...I.stroke}>
      <rect x="4" y="11" width="20" height="11" rx="1" />
      <line x1="24" y1="14" x2="28" y2="14" />
      <line x1="24" y1="18" x2="28" y2="18" />
      <line x1="8" y1="22" x2="8" y2="25" />
      <line x1="12" y1="22" x2="12" y2="25" />
      <line x1="16" y1="22" x2="16" y2="25" />
      <line x1="20" y1="22" x2="20" y2="25" />
      <rect x="9" y="14" width="10" height="5" />
    </g>
  </svg>
);

const ReqBaseIcon = ({ size = 32 }) => (
  <svg viewBox="0 0 32 32" width={size} height={size} aria-hidden="true">
    <g {...I.stroke}>
      <polygon points="16,5 28,12 28,20 16,27 4,20 4,12" />
      <polygon points="16,11 22,14 22,18 16,21 10,18 10,14" />
      <line x1="16" y1="5" x2="16" y2="11" />
      <line x1="28" y1="12" x2="22" y2="14" />
      <line x1="28" y1="20" x2="22" y2="18" />
      <line x1="16" y1="27" x2="16" y2="21" />
      <line x1="4" y1="20" x2="10" y2="18" />
      <line x1="4" y1="12" x2="10" y2="14" />
    </g>
  </svg>
);

export {
  AnvilIcon, HammerIcon, SparksIcon,
  WorldFoundryIcon, BlenderIcon, MameIcon, F9dasmIcon, AssemblerIcon, LibvgmIcon, VgmstreamIcon,
  GamepadIcon, AnvilEditionIcon, SpriteIcon,
  BigAnvilLogo, FoundryMark, DownloadIcon, CopyIcon, ArrowRightIcon,
  ForgeSceneIcon, GearStackIcon,
  TorrentIcon, MagnetIcon,
  ReqCpuIcon, ReqMemoryIcon, ReqStorageIcon, ReqGpuIcon, ReqBaseIcon,
};
