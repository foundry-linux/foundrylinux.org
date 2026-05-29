// copy.js — clipboard wiring for all copy buttons on foundrylinux.org
// Buttons carry data-copy="<selector>" to copy that element's textContent,
// or data-copy-text="<literal>" to copy a literal string directly.
// No dependencies.
(function () {
  'use strict';

  function copyText(text, btn) {
    var done = function () {
      btn.setAttribute('data-copied', '1');
      setTimeout(function () { btn.removeAttribute('data-copied'); }, 1500);
    };
    if (navigator.clipboard && navigator.clipboard.writeText) {
      navigator.clipboard.writeText(text).then(done).catch(function () {
        fallback(text); done();
      });
    } else {
      fallback(text); done();
    }
  }

  function fallback(text) {
    var ta = document.createElement('textarea');
    ta.value = text;
    ta.setAttribute('readonly', '');
    ta.style.cssText = 'position:fixed;opacity:0';
    document.body.appendChild(ta);
    ta.select();
    try { document.execCommand('copy'); } catch (e) {}
    document.body.removeChild(ta);
  }

  document.addEventListener('DOMContentLoaded', function () {
    document.querySelectorAll('[data-copy], [data-copy-text]').forEach(function (btn) {
      btn.addEventListener('click', function (e) {
        e.preventDefault();
        e.stopPropagation();
        var literal = btn.getAttribute('data-copy-text');
        if (literal) { copyText(literal, btn); return; }
        var target = document.querySelector(btn.getAttribute('data-copy'));
        if (!target) return;
        copyText((target.textContent || '').trim(), btn);
      });
    });
  });
})();
