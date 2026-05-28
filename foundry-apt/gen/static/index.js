// index.js — progressive enhancement for the apt.foundrylinux.org package index.
// No dependencies. Page is fully functional without JS (links work, table is
// readable, install snippet is visible). This script layers on:
//   - filter box: hides non-matching rows, updates live count
//   - sortable columns: Name / Version / Description
//   - copy-to-clipboard: on the quick-install pre block
//
// Adapted from the worldfoundry.org apt index.js.

(function () {
  "use strict";

  // ── copy buttons ────────────────────────────────────────────────────
  // Any <button data-copy="<id>"> copies the textContent of element #<id>.
  function bindCopy() {
    document.querySelectorAll("[data-copy]").forEach(function (btn) {
      btn.addEventListener("click", function () {
        var target = document.getElementById(btn.getAttribute("data-copy"));
        if (!target) return;
        var text = (target.textContent || "").trim();
        var done = function () {
          var prevTitle = btn.getAttribute("title") || "Copy";
          btn.setAttribute("title", "Copied!");
          btn.setAttribute("data-copied", "1");
          setTimeout(function () {
            btn.setAttribute("title", prevTitle);
            btn.removeAttribute("data-copied");
          }, 1400);
        };
        if (navigator.clipboard && navigator.clipboard.writeText) {
          navigator.clipboard.writeText(text).then(done).catch(function () {
            fallbackCopy(text); done();
          });
        } else {
          fallbackCopy(text); done();
        }
      });
    });
  }

  function fallbackCopy(text) {
    var ta = document.createElement("textarea");
    ta.value = text;
    ta.setAttribute("readonly", "");
    ta.style.cssText = "position:fixed;opacity:0";
    document.body.appendChild(ta);
    ta.select();
    try { document.execCommand("copy"); } catch (e) {}
    document.body.removeChild(ta);
  }

  // ── filter ──────────────────────────────────────────────────────────
  // Matches against data-name and data-desc on each <tr>.
  function bindFilter() {
    var input = document.getElementById("filter-q");
    var clear = document.querySelector(".filter-clear");
    var table = document.querySelector(".listing-table");
    var count = document.querySelector("[data-count]");
    if (!input || !table || !count) return;

    var tbody   = table.querySelector("tbody");
    var rows    = Array.prototype.slice.call(tbody.querySelectorAll("tr"));
    var origHTML = count.innerHTML;

    function apply() {
      var q = (input.value || "").trim().toLowerCase();
      var n = 0;
      rows.forEach(function (r) {
        var match = !q ||
          (r.getAttribute("data-name") || "").indexOf(q) !== -1 ||
          (r.getAttribute("data-desc") || "").indexOf(q) !== -1;
        r.hidden = !match;
        if (match) n++;
      });
      if (clear) clear.hidden = !q;
      count.innerHTML = q
        ? "<b>" + n + "</b> " + (n === 1 ? "package" : "packages") +
          ' <span style="color:var(--ink-soft)">· filtered <em>' + escapeHtml(q) + "</em></span>"
        : origHTML;
    }

    input.addEventListener("input", apply);
    if (clear) {
      clear.addEventListener("click", function () {
        input.value = "";
        input.focus();
        apply();
      });
    }
  }

  function escapeHtml(s) {
    return String(s).replace(/[&<>"']/g, function (c) {
      return ({ "&": "&amp;", "<": "&lt;", ">": "&gt;", '"': "&quot;", "'": "&#39;" })[c];
    });
  }

  // ── column sort ─────────────────────────────────────────────────────
  // <th data-sort="name|ver|desc"> triggers a stable string sort.
  // Each <tr> carries data-name, data-ver, data-desc for the comparator.
  function bindSort() {
    var table = document.querySelector(".listing-table");
    if (!table) return;
    var tbody = table.querySelector("tbody");
    var ths   = table.querySelectorAll("thead th[data-sort]");
    var state = { key: "name", dir: "asc" };

    function setIndicators() {
      ths.forEach(function (th) {
        var ind = th.querySelector(".sort-ind");
        if (!ind) return;
        if (th.getAttribute("data-sort") === state.key) {
          ind.textContent = state.dir === "asc" ? "▲" : "▼";
          ind.style.color = "var(--accent)";
        } else {
          ind.textContent = "↕";
          ind.style.color = "var(--ink-faint)";
        }
      });
    }

    function sortRows() {
      var rows = Array.prototype.slice.call(tbody.querySelectorAll("tr"));
      var key = state.key, dir = state.dir === "asc" ? 1 : -1;
      rows.sort(function (a, b) {
        var av = (a.getAttribute("data-" + key) || "").toLowerCase();
        var bv = (b.getAttribute("data-" + key) || "").toLowerCase();
        if (av < bv) return -1 * dir;
        if (av > bv) return  1 * dir;
        return 0;
      });
      var frag = document.createDocumentFragment();
      rows.forEach(function (r) { frag.appendChild(r); });
      tbody.appendChild(frag);
    }

    ths.forEach(function (th) {
      th.style.cursor = "pointer";
      th.addEventListener("click", function () {
        var key = th.getAttribute("data-sort");
        state.dir = state.key === key && state.dir === "asc" ? "desc" : "asc";
        state.key = key;
        setIndicators();
        sortRows();
      });
    });

    setIndicators();
  }

  // ── go ──────────────────────────────────────────────────────────────
  function ready(fn) {
    if (document.readyState !== "loading") fn();
    else document.addEventListener("DOMContentLoaded", fn);
  }

  ready(function () {
    bindCopy();
    bindFilter();
    bindSort();
  });
})();
