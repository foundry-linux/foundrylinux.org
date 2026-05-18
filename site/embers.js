(function () {
  'use strict';
  if (matchMedia('(prefers-reduced-motion: reduce)').matches) return;
  requestAnimationFrame(function () { requestAnimationFrame(init); });

  function init() {
    var wrap = document.querySelector('.topbar-wrap');
    if (!wrap) return;

    // Last child so it paints over the nav text — multiply then darkens white elements
    var canvas = document.createElement('canvas');
    canvas.style.cssText = 'position:absolute;inset:0;pointer-events:none;z-index:10;mix-blend-mode:multiply;';
    wrap.appendChild(canvas);
    var ctx = canvas.getContext('2d');

    function resize() { canvas.width = wrap.clientWidth; canvas.height = wrap.clientHeight; }
    resize();
    if (typeof ResizeObserver !== 'undefined') { new ResizeObserver(resize).observe(wrap); }
    else { window.addEventListener('resize', resize, { passive: true }); }

    var STATE = 'idle';
    var age = 0;
    var nextTrigger = 1 + Math.random() * 2;
    var dir = 1;

    var CROSS_DUR = 4.2;

    function smoothstep(t) { return t * t * (3 - 2 * t); }

    var last = null;
    function frame(ts) {
      requestAnimationFrame(frame);
      if (last === null) { last = ts; return; }
      var dt = Math.min((ts - last) / 1000, 0.05);
      last = ts;

      ctx.clearRect(0, 0, canvas.width, canvas.height);

      if (STATE === 'idle') {
        nextTrigger -= dt;
        if (nextTrigger <= 0) {
          STATE = 'cross';
          age = 0;
          dir = Math.random() < 0.5 ? 1 : -1;
        }
        return;
      }

      age += dt;
      if (age >= CROSS_DUR) {
        STATE = 'idle';
        nextTrigger = 11 + Math.random() * 15;
        return;
      }

      var W = canvas.width;
      var H = canvas.height;

      var blobW = W * 0.45;

      var t  = age / CROSS_DUR;
      var cx = dir === 1
        ? -blobW / 2 + t * (W + blobW)
        :  W + blobW / 2 - t * (W + blobW);
      var cy = H / 2;

      // Soft intensity envelope as blob enters / exits the bar
      var overlap = Math.min(cx + blobW / 2, W) - Math.max(cx - blobW / 2, 0);
      var intensity = smoothstep(Math.max(0, Math.min(1, overlap / (blobW * 0.55))));

      var maxAlpha = 0.55 * intensity;
      if (maxAlpha < 0.003) return;

      // Radial gradient: dark centre, transparent edge
      // With multiply blend, this dims (but doesn't black-out) white text it crosses
      var grad = ctx.createRadialGradient(cx, cy, 0, cx, cy, blobW / 2);
      grad.addColorStop(0,   'rgba(0,0,15,' + maxAlpha.toFixed(4) + ')');
      grad.addColorStop(0.5, 'rgba(0,0,12,' + (maxAlpha * 0.6).toFixed(4) + ')');
      grad.addColorStop(1,   'rgba(0,0,8,0)');
      ctx.fillStyle = grad;

      // Draw as tall ellipse (taller than bar so vertical falloff is gentle)
      ctx.save();
      ctx.scale(1, (H * 2.5) / blobW);
      ctx.beginPath();
      ctx.arc(cx, cy / ((H * 2.5) / blobW), blobW / 2, 0, Math.PI * 2);
      ctx.fill();
      ctx.restore();
    }

    requestAnimationFrame(frame);
  }
})();
