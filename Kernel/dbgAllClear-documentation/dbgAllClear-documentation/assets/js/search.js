/* dbgAllClear Deep Dive — full-text search over headings and their content */
(function () {
  'use strict';

  var index = [];
  var input, box, activeIdx = -1, results = [];

  function norm(s) { return s.toLowerCase().normalize('NFD').replace(/[̀-ͯ]/g, '').replace(/đ/g, 'd'); }
  function headingText(h) {
    if (window.DocHeadText) return window.DocHeadText(h);
    return h.textContent.replace(/#\s*$/, '').trim();
  }

  function build() {
    var heads = Array.prototype.slice.call(document.querySelectorAll('.content h1[id], .content h2[id], .content h3[id], .content h4[id]'));
    heads.forEach(function (h, i) {
      var level = parseInt(h.tagName.slice(1), 10);
      var text = '';
      var n = h.nextElementSibling;
      while (n && !/^H[1-4]$/.test(n.tagName)) {
        if (n.tagName !== 'SCRIPT') {
          var clone = n.cloneNode(true);
          Array.prototype.forEach.call(clone.querySelectorAll('script, .diagram-body'), function (x) { x.parentNode.removeChild(x); });
          text += ' ' + clone.textContent;
        }
        n = n.nextElementSibling;
      }
      // include mermaid source text (function names inside diagrams)
      var chapter = h.closest('.chapter, .hero');
      var chTitle = '';
      if (chapter) {
        var h2 = chapter.querySelector('h2, h1');
        if (h2) chTitle = headingText(h2);
      }
      text = text.replace(/\s+/g, ' ').trim();
      index.push({
        id: h.id, title: headingText(h), level: level, chapter: chTitle,
        text: text, nTitle: norm(headingText(h)), nText: norm(text)
      });
    });
  }

  function escapeHtml(s) { return s.replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;'); }

  function snippet(entry, terms) {
    var t = entry.text, nt = entry.nText;
    var pos = -1;
    for (var i = 0; i < terms.length; i++) { pos = nt.indexOf(terms[i]); if (pos !== -1) break; }
    if (pos === -1) return escapeHtml(t.slice(0, 120));
    var start = Math.max(0, pos - 40);
    var frag = t.slice(start, start + 140);
    var nfrag = nt.slice(start, start + 140);
    // highlight terms (positions identical between t and nt: normalization keeps length for Vietnamese after NFD strip? not always) —
    // to stay safe we highlight on the normalized index but only when lengths match
    var out = escapeHtml(frag);
    if (frag.length === nfrag.length) {
      var marks = [];
      terms.forEach(function (term) {
        var p = nfrag.indexOf(term);
        while (p !== -1) { marks.push([p, p + term.length]); p = nfrag.indexOf(term, p + term.length); }
      });
      marks.sort(function (a, b) { return a[0] - b[0]; });
      var res = '', last = 0;
      marks.forEach(function (m) {
        if (m[0] < last) return;
        res += escapeHtml(frag.slice(last, m[0])) + '<mark>' + escapeHtml(frag.slice(m[0], m[1])) + '</mark>';
        last = m[1];
      });
      res += escapeHtml(frag.slice(last));
      out = res;
    }
    return (start > 0 ? '…' : '') + out + '…';
  }

  function search(q) {
    var terms = norm(q).split(/\s+/).filter(Boolean);
    if (!terms.length) return [];
    var scored = [];
    index.forEach(function (e) {
      var score = 0, all = true;
      terms.forEach(function (t) {
        var inTitle = e.nTitle.indexOf(t) !== -1;
        var count = 0, p = e.nText.indexOf(t);
        while (p !== -1 && count < 20) { count++; p = e.nText.indexOf(t, p + t.length); }
        if (!inTitle && !count) all = false;
        score += (inTitle ? 12 : 0) + count + (e.nTitle === t ? 10 : 0);
      });
      if (all && score > 0) scored.push({ e: e, s: score - e.level * 0.1 });
    });
    scored.sort(function (a, b) { return b.s - a.s; });
    return scored.slice(0, 14).map(function (x) { return x.e; });
  }

  function render(q) {
    results = search(q);
    activeIdx = -1;
    box.innerHTML = '';
    if (!q.trim()) { box.classList.remove('open'); input.setAttribute('aria-expanded', 'false'); return; }
    if (!results.length) {
      box.innerHTML = '<div class="search-empty">Không tìm thấy kết quả cho “' + escapeHtml(q) + '”.</div>';
    } else {
      var terms = norm(q).split(/\s+/).filter(Boolean);
      results.forEach(function (e, i) {
        var b = document.createElement('button');
        b.type = 'button';
        b.setAttribute('role', 'option');
        b.id = 'sr-' + i;
        b.innerHTML = '<span class="r-ctx">' + escapeHtml(e.chapter) + '</span><div class="r-title">' + escapeHtml(e.title) +
          '</div><span class="r-snip">' + snippet(e, terms) + '</span>';
        b.addEventListener('click', function () { go(e); });
        box.appendChild(b);
      });
    }
    box.classList.add('open');
    input.setAttribute('aria-expanded', 'true');
  }

  function go(e) {
    var el = document.getElementById(e.id);
    box.classList.remove('open');
    input.setAttribute('aria-expanded', 'false');
    if (!el) return;
    var tabPanel = el.closest('.tabpanel');
    if (tabPanel && tabPanel.hidden) {
      var tab = document.querySelector('[aria-controls="' + tabPanel.id + '"]');
      if (tab) tab.click();
    }
    var acc = el.closest('details');
    if (acc) acc.open = true;
    history.replaceState(null, '', '#' + e.id);
    el.scrollIntoView({ behavior: 'smooth', block: 'start' });
    if (window.DocFlash) window.DocFlash(el);
  }

  function move(d) {
    var btns = box.querySelectorAll('button');
    if (!btns.length) return;
    activeIdx = (activeIdx + d + btns.length) % btns.length;
    Array.prototype.forEach.call(btns, function (b, i) { b.classList.toggle('active', i === activeIdx); });
    btns[activeIdx].scrollIntoView({ block: 'nearest' });
    input.setAttribute('aria-activedescendant', btns[activeIdx].id);
  }

  function init() {
    input = document.getElementById('search-input');
    box = document.getElementById('search-results');
    if (!input || !box) return;
    build();
    var timer;
    input.addEventListener('input', function () { clearTimeout(timer); timer = setTimeout(function () { render(input.value); }, 90); });
    input.addEventListener('focus', function () { if (input.value.trim()) render(input.value); });
    input.addEventListener('keydown', function (ev) {
      if (ev.key === 'ArrowDown') { ev.preventDefault(); move(1); }
      else if (ev.key === 'ArrowUp') { ev.preventDefault(); move(-1); }
      else if (ev.key === 'Enter') {
        ev.preventDefault();
        var pick = results[activeIdx >= 0 ? activeIdx : 0];
        if (pick) go(pick);
      } else if (ev.key === 'Escape') { box.classList.remove('open'); input.blur(); }
    });
    document.addEventListener('click', function (ev) {
      if (!ev.target.closest('.search')) box.classList.remove('open');
    });
    document.addEventListener('keydown', function (ev) {
      var tag = (ev.target.tagName || '').toLowerCase();
      if (ev.key === '/' && tag !== 'input' && tag !== 'textarea') { ev.preventDefault(); input.focus(); input.select(); }
    });
  }

  // Scripts are loaded with `defer`, so DOMContentLoaded has not fired yet; main.js assigns heading ids first.
  if (document.readyState === 'complete') init(); else document.addEventListener('DOMContentLoaded', init);
})();
