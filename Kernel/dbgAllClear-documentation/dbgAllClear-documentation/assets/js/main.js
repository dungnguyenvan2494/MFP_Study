/* dbgAllClear Deep Dive — core UI: theme, navigation, TOC, code blocks, tabs, checklist, glossary */
(function () {
  'use strict';

  var root = document.documentElement;
  var store = {
    get: function (k) { try { return window.localStorage.getItem(k); } catch (e) { return null; } },
    set: function (k, v) { try { window.localStorage.setItem(k, v); } catch (e) { /* storage unavailable */ } }
  };
  window.DocStore = store;

  /* ---------------- theme ---------------- */
  function systemDark() { return window.matchMedia && window.matchMedia('(prefers-color-scheme: dark)').matches; }
  function isDark() {
    var t = root.getAttribute('data-theme');
    return t ? t === 'dark' : systemDark();
  }
  window.DocTheme = { isDark: isDark };
  var saved = store.get('dbg-doc-theme');
  if (saved === 'dark' || saved === 'light') root.setAttribute('data-theme', saved);

  function updateThemeBtn() {
    var btn = document.getElementById('theme-btn');
    if (!btn) return;
    btn.setAttribute('aria-label', isDark() ? 'Chuyển sang giao diện sáng' : 'Chuyển sang giao diện tối');
    btn.querySelector('.i-sun').style.display = isDark() ? 'block' : 'none';
    btn.querySelector('.i-moon').style.display = isDark() ? 'none' : 'block';
  }
  function toggleTheme() {
    var next = isDark() ? 'light' : 'dark';
    root.setAttribute('data-theme', next);
    store.set('dbg-doc-theme', next);
    updateThemeBtn();
    document.dispatchEvent(new CustomEvent('doc:themechange'));
  }

  /* ---------------- helpers ---------------- */
  function slug(s) {
    return s.toLowerCase().normalize('NFD').replace(/[̀-ͯ]/g, '').replace(/đ/g, 'd')
      .replace(/[^a-z0-9]+/g, '-').replace(/^-+|-+$/g, '').slice(0, 64);
  }
  function $all(sel, ctx) { return Array.prototype.slice.call((ctx || document).querySelectorAll(sel)); }
  function headText(h) {
    var c = h.cloneNode(true);
    var a = c.querySelector('.anchor');
    if (a) a.parentNode.removeChild(a);
    return c.textContent.replace(/\s+/g, ' ').trim();
  }
  window.DocHeadText = headText;

  /* ---------------- headings: ids + anchors ---------------- */
  function setupHeadings() {
    var used = {};
    $all('.content h2, .content h3, .content h4').forEach(function (h) {
      if (!h.id) {
        var base = slug(h.textContent) || 'section';
        var id = base, n = 2;
        while (used[id] || document.getElementById(id)) { id = base + '-' + n++; }
        h.id = id;
      }
      used[h.id] = true;
      if (!h.querySelector('.anchor')) {
        var a = document.createElement('a');
        a.className = 'anchor';
        a.href = '#' + h.id;
        a.setAttribute('aria-label', 'Liên kết tới mục này');
        a.textContent = '#';
        h.appendChild(a);
      }
    });
  }

  /* ---------------- sidebar ---------------- */
  function setupSidebar() {
    $all('.nav-group').forEach(function (g) {
      var btn = g.querySelector('.nav-group-btn');
      var key = 'dbg-doc-nav-' + g.getAttribute('data-key');
      if (store.get(key) === '1') { g.setAttribute('data-collapsed', 'true'); btn.setAttribute('aria-expanded', 'false'); }
      btn.addEventListener('click', function () {
        var collapsed = g.getAttribute('data-collapsed') === 'true';
        g.setAttribute('data-collapsed', collapsed ? 'false' : 'true');
        btn.setAttribute('aria-expanded', collapsed ? 'true' : 'false');
        store.set(key, collapsed ? '0' : '1');
      });
    });
    var menuBtn = document.getElementById('menu-btn');
    var overlay = document.getElementById('overlay');
    function closeNav() { document.body.classList.remove('nav-open'); menuBtn.setAttribute('aria-expanded', 'false'); }
    menuBtn.addEventListener('click', function () {
      var open = document.body.classList.toggle('nav-open');
      menuBtn.setAttribute('aria-expanded', open ? 'true' : 'false');
    });
    overlay.addEventListener('click', closeNav);
    $all('.sidebar a').forEach(function (a) { a.addEventListener('click', closeNav); });
    document.addEventListener('keydown', function (e) { if (e.key === 'Escape') closeNav(); });
  }

  /* ---------------- scroll spy, TOC, progress ---------------- */
  var navLinks = [], navTargets = [], chapters = [], tocEl, currentChapter = null, tocLinks = [];

  function buildToc(ch) {
    if (!tocEl || ch === currentChapter) return;
    currentChapter = ch;
    tocEl.innerHTML = '';
    tocLinks = [];
    if (!ch) return;
    var title = document.createElement('div');
    title.className = 'toc-title';
    title.textContent = 'Trong chương này';
    var chTitle = document.createElement('div');
    chTitle.className = 'toc-chapter';
    var h2 = ch.querySelector('h2, h1');
    chTitle.textContent = h2 ? headText(h2) : '';
    var ul = document.createElement('ul');
    $all('h3', ch).forEach(function (h) {
      var li = document.createElement('li');
      var a = document.createElement('a');
      a.href = '#' + h.id;
      a.textContent = headText(h);
      li.appendChild(a);
      ul.appendChild(li);
      tocLinks.push({ a: a, h: h });
    });
    tocEl.appendChild(title);
    tocEl.appendChild(chTitle);
    if (tocLinks.length) tocEl.appendChild(ul);
  }

  function onScroll() {
    var y = window.scrollY;
    var docH = document.documentElement.scrollHeight - window.innerHeight;
    var p = document.getElementById('progress');
    if (p) p.style.width = (docH > 0 ? Math.min(100, (y / docH) * 100) : 0) + '%';
    var tt = document.getElementById('to-top');
    if (tt) tt.classList.toggle('show', y > 900);

    var offset = 130;
    // chapter
    var ch = null;
    for (var i = 0; i < chapters.length; i++) {
      if (chapters[i].getBoundingClientRect().top - offset <= 0) ch = chapters[i]; else break;
    }
    buildToc(ch || chapters[0]);
    // sidebar active
    var active = -1;
    for (var j = 0; j < navTargets.length; j++) {
      if (navTargets[j] && navTargets[j].getBoundingClientRect().top - offset <= 0) active = j;
    }
    navLinks.forEach(function (a, k) { a.classList.toggle('active', k === active); });
    if (active >= 0) {
      var act = navLinks[active];
      var sb = document.querySelector('.sidebar');
      var r = act.getBoundingClientRect(), sr = sb.getBoundingClientRect();
      if (r.top < sr.top + 40 || r.bottom > sr.bottom - 40) sb.scrollTop += r.top - sr.top - sr.height / 2;
    }
    // toc active
    var tActive = -1;
    tocLinks.forEach(function (t, k) { if (t.h.getBoundingClientRect().top - offset <= 0) tActive = k; });
    tocLinks.forEach(function (t, k) { t.a.classList.toggle('active', k === tActive); });
  }

  function setupScrollSpy() {
    tocEl = document.getElementById('toc');
    chapters = $all('.content > .chapter, .content > .hero');
    navLinks = $all('.sidebar a[href^="#"]');
    navTargets = navLinks.map(function (a) { return document.getElementById(a.getAttribute('href').slice(1)); });
    // sort by document position so "active" picks the last passed target
    var pairs = navLinks.map(function (a, i) { return { a: a, t: navTargets[i] }; }).filter(function (p) { return p.t; });
    pairs.sort(function (x, y) { return x.t.compareDocumentPosition(y.t) & Node.DOCUMENT_POSITION_FOLLOWING ? -1 : 1; });
    navLinks = pairs.map(function (p) { return p.a; });
    navTargets = pairs.map(function (p) { return p.t; });
    var ticking = false;
    window.addEventListener('scroll', function () {
      if (!ticking) { ticking = true; requestAnimationFrame(function () { ticking = false; onScroll(); }); }
    }, { passive: true });
    window.addEventListener('resize', onScroll);
    onScroll();
  }

  /* ---------------- code blocks ---------------- */
  function escapeHtml(s) { return s.replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;'); }
  function parseList(v) {
    var out = {};
    (v || '').split(',').forEach(function (part) {
      part = part.trim();
      if (!part) return;
      var m = part.split('-');
      var a = parseInt(m[0], 10), b = m[1] ? parseInt(m[1], 10) : a;
      for (var k = a; k <= b; k++) out[k] = true;
    });
    return out;
  }
  function copyText(text, btn) {
    function done(ok) {
      var old = btn.getAttribute('data-label') || 'Copy';
      btn.textContent = ok ? 'Đã copy' : 'Lỗi copy';
      setTimeout(function () { btn.textContent = old; }, 1400);
    }
    if (navigator.clipboard && window.isSecureContext) {
      navigator.clipboard.writeText(text).then(function () { done(true); }, function () { fallback(); });
    } else { fallback(); }
    function fallback() {
      var ta = document.createElement('textarea');
      ta.value = text; ta.setAttribute('readonly', ''); ta.style.position = 'fixed'; ta.style.opacity = '0';
      document.body.appendChild(ta); ta.select();
      var ok = false;
      try { ok = document.execCommand('copy'); } catch (e) { ok = false; }
      document.body.removeChild(ta);
      done(ok);
    }
  }
  function setupCode() {
    $all('figure.code').forEach(function (fig) {
      var code = fig.querySelector('code');
      if (!code || code.getAttribute('data-done')) return;
      var raw = code.textContent.replace(/^\n/, '').replace(/\s+$/, '');
      var start = parseInt(fig.getAttribute('data-start') || '1', 10);
      var hl = parseList(fig.getAttribute('data-hl'));
      var warn = parseList(fig.getAttribute('data-warn'));
      var lang = (code.className.match(/language-(\w+)/) || [])[1] || 'cpp';
      var lines = raw.split('\n');
      var html = lines.map(function (line, i) {
        var n = start + i;
        var inner;
        if (window.hljs && window.hljs.getLanguage(lang)) {
          try { inner = window.hljs.highlight(line, { language: lang, ignoreIllegals: true }).value; }
          catch (e) { inner = escapeHtml(line); }
        } else { inner = escapeHtml(line); }
        var cls = 'ln' + (hl[n] ? ' hl' : '') + (warn[n] ? ' hl-warn' : '');
        var label = fig.hasAttribute('data-nonum') ? '' : String(n);
        return '<span class="' + cls + '" data-n="' + label + '">' + (inner || ' ') + '</span>';
      }).join('');
      code.innerHTML = html;
      code.setAttribute('data-done', '1');
      var btn = fig.querySelector('.copy-btn');
      if (btn) {
        btn.setAttribute('data-label', btn.textContent);
        btn.addEventListener('click', function () { copyText(raw, btn); });
      }
    });
  }

  /* ---------------- tabs ---------------- */
  function setupTabs() {
    $all('.tabs').forEach(function (tabs) {
      var buttons = $all('[role="tab"]', tabs);
      function select(btn, focus) {
        buttons.forEach(function (b) {
          var on = b === btn;
          b.setAttribute('aria-selected', on ? 'true' : 'false');
          b.tabIndex = on ? 0 : -1;
          var panel = document.getElementById(b.getAttribute('aria-controls'));
          if (panel) panel.hidden = !on;
        });
        if (focus) btn.focus();
        document.dispatchEvent(new CustomEvent('doc:tabchange'));
      }
      buttons.forEach(function (b, i) {
        b.addEventListener('click', function () { select(b, false); });
        b.addEventListener('keydown', function (e) {
          var k = e.key, idx = i;
          if (k === 'ArrowRight') idx = (i + 1) % buttons.length;
          else if (k === 'ArrowLeft') idx = (i - 1 + buttons.length) % buttons.length;
          else if (k === 'Home') idx = 0;
          else if (k === 'End') idx = buttons.length - 1;
          else return;
          e.preventDefault();
          select(buttons[idx], true);
        });
      });
    });
    // links that open a specific tab: <a data-open-tab="tab-id">
    $all('[data-open-tab]').forEach(function (a) {
      a.addEventListener('click', function () {
        var t = document.getElementById(a.getAttribute('data-open-tab'));
        if (t) t.click();
      });
    });
  }

  /* ---------------- checklist ---------------- */
  function setupChecklist() {
    var list = document.getElementById('debug-checklist');
    if (!list) return;
    var boxes = $all('input[type="checkbox"]', list);
    var meter = document.getElementById('ck-meter');
    var count = document.getElementById('ck-count');
    var state = {};
    try { state = JSON.parse(store.get('dbg-doc-checklist') || '{}') || {}; } catch (e) { state = {}; }
    function refresh() {
      var done = 0;
      boxes.forEach(function (b) {
        b.closest('li').classList.toggle('done', b.checked);
        if (b.checked) done++;
      });
      if (meter) meter.style.width = (boxes.length ? done / boxes.length * 100 : 0) + '%';
      if (count) count.textContent = done + ' / ' + boxes.length;
    }
    boxes.forEach(function (b) {
      if (state[b.id]) b.checked = true;
      b.addEventListener('change', function () {
        state[b.id] = b.checked;
        store.set('dbg-doc-checklist', JSON.stringify(state));
        refresh();
      });
    });
    var reset = document.getElementById('ck-reset');
    if (reset) reset.addEventListener('click', function () {
      boxes.forEach(function (b) { b.checked = false; });
      state = {};
      store.set('dbg-doc-checklist', '{}');
      refresh();
    });
    refresh();
  }

  /* ---------------- glossary filter ---------------- */
  function norm(s) { return s.toLowerCase().normalize('NFD').replace(/[̀-ͯ]/g, '').replace(/đ/g, 'd'); }
  window.DocNorm = norm;
  function setupGlossary() {
    var input = document.getElementById('glossary-filter');
    var table = document.getElementById('glossary-table');
    if (!input || !table) return;
    var rows = $all('tbody tr', table);
    var empty = document.getElementById('glossary-empty');
    input.addEventListener('input', function () {
      var q = norm(input.value.trim());
      var shown = 0;
      rows.forEach(function (r) {
        var ok = !q || norm(r.textContent).indexOf(q) !== -1;
        r.hidden = !ok;
        if (ok) shown++;
      });
      if (empty) empty.hidden = shown !== 0;
    });
  }

  /* ---------------- misc ---------------- */
  function setupMisc() {
    var tt = document.getElementById('to-top');
    if (tt) tt.addEventListener('click', function () { window.scrollTo({ top: 0, behavior: 'smooth' }); });
    var themeBtn = document.getElementById('theme-btn');
    if (themeBtn) themeBtn.addEventListener('click', toggleTheme);
    if (window.matchMedia) {
      var mq = window.matchMedia('(prefers-color-scheme: dark)');
      var h = function () { if (!root.getAttribute('data-theme')) { updateThemeBtn(); document.dispatchEvent(new CustomEvent('doc:themechange')); } };
      if (mq.addEventListener) mq.addEventListener('change', h); else if (mq.addListener) mq.addListener(h);
    }
    updateThemeBtn();
  }

  function flashTarget(el) {
    if (!el) return;
    el.classList.remove('flash');
    void el.offsetWidth;
    el.classList.add('flash');
  }
  window.DocFlash = flashTarget;

  document.addEventListener('DOMContentLoaded', function () {
    setupHeadings();
    setupSidebar();
    setupCode();
    setupTabs();
    setupChecklist();
    setupGlossary();
    setupMisc();
    setupScrollSpy();
    if (location.hash) {
      var t = document.getElementById(decodeURIComponent(location.hash.slice(1)));
      if (t) setTimeout(function () { t.scrollIntoView(); flashTarget(t); }, 60);
    }
  });
})();
