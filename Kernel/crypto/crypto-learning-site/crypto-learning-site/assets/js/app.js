/* ==========================================================================
   app.js — điều hướng & tương tác chung.
   - Router theo URL hash: #<id chương> hoặc #<id mục/sơ đồ/card> (anchor HTML thuần).
   - Sidebar (thu gọn trên desktop, drawer trên mobile), TOC phải, breadcrumbs,
     trước/sau, tiến độ (localStorage, chỉ trên trình duyệt hiện tại), theme sáng/tối,
     nút copy code, mở/đóng toàn bộ mục thu gọn, back-to-top, bộ lọc source tree & glossary.
   Thứ tự nạp: util.js → mermaid-config.js → diagrams.js → search.js → app.js
   ========================================================================== */
(function () {
  'use strict';
  var CS = (window.CryptoSite = window.CryptoSite || {});
  var chapters = [];          // [{id, title, group, el, link}]
  var current = null;
  var tocObserver = null;
  var VISITED_KEY = 'cryptoSite.visited.v1';
  var THEME_KEY = 'cryptoSite.theme.v1';

  /* ---------------- helpers ---------------- */
  function $(sel, root) { return (root || document).querySelector(sel); }
  function $all(sel, root) { return Array.prototype.slice.call((root || document).querySelectorAll(sel)); }
  function isMobile() { return window.matchMedia('(max-width: 960px)').matches; }
  function isNarrow() { return window.matchMedia('(max-width: 1280px)').matches; }

  /* ---------------- theme ---------------- */
  function applyTheme(t) {
    document.documentElement.setAttribute('data-theme', t);
    var btn = $('#theme-toggle');
    if (btn) {
      btn.setAttribute('aria-pressed', t === 'dark' ? 'true' : 'false');
      btn.setAttribute('aria-label', t === 'dark' ? 'Chuyển sang giao diện sáng' : 'Chuyển sang giao diện tối');
      btn.title = btn.getAttribute('aria-label');
    }
  }
  function initTheme() {
    var saved = CS.store.get(THEME_KEY, null);
    var prefersDark = window.matchMedia && window.matchMedia('(prefers-color-scheme: dark)').matches;
    applyTheme(saved || (prefersDark ? 'dark' : 'light'));
    $('#theme-toggle').addEventListener('click', function () {
      var next = document.documentElement.getAttribute('data-theme') === 'dark' ? 'light' : 'dark';
      applyTheme(next);
      CS.store.set(THEME_KEY, next);
    });
  }

  /* ---------------- sidebar & toc panels ---------------- */
  function setSidebar(open) {
    var btn = $('#sidebar-toggle');
    if (isMobile()) {
      document.body.classList.toggle('sidebar-open', open);
      document.body.classList.remove('sidebar-collapsed');
    } else {
      document.body.classList.toggle('sidebar-collapsed', !open);
      document.body.classList.remove('sidebar-open');
    }
    btn.setAttribute('aria-expanded', String(open));
    if (open && isMobile()) { var a = $('#sidebar a[aria-current="page"]') || $('#sidebar a'); if (a) a.focus(); }
  }
  function sidebarIsOpen() {
    return isMobile() ? document.body.classList.contains('sidebar-open') : !document.body.classList.contains('sidebar-collapsed');
  }
  function setToc(open) {
    document.body.classList.toggle('toc-open', open);
    $('#toc-toggle').setAttribute('aria-expanded', String(open));
  }
  function initPanels() {
    $('#sidebar-toggle').addEventListener('click', function () { setSidebar(!sidebarIsOpen()); });
    $('#toc-toggle').addEventListener('click', function () { setToc(!document.body.classList.contains('toc-open')); });
    $('#sidebar-backdrop').addEventListener('click', function () { setSidebar(false); setToc(false); });
    document.addEventListener('keydown', function (e) {
      if (e.key !== 'Escape') return;
      if (isMobile() && document.body.classList.contains('sidebar-open')) { setSidebar(false); $('#sidebar-toggle').focus(); }
      if (document.body.classList.contains('toc-open')) { setToc(false); }
    });
    $('#sidebar-toggle').setAttribute('aria-expanded', String(!isMobile()));
  }

  /* ---------------- progress (localStorage, chỉ trình duyệt hiện tại) ---------------- */
  function updateProgress() {
    var visited = CS.store.get(VISITED_KEY, []);
    chapters.forEach(function (c) { c.link.classList.toggle('visited', visited.indexOf(c.id) !== -1); });
    var n = chapters.filter(function (c) { return visited.indexOf(c.id) !== -1; }).length;
    var pct = chapters.length ? Math.round(n * 100 / chapters.length) : 0;
    $('#progress-text').textContent = n + '/' + chapters.length + ' chương đã mở';
    $('#progress-bar').style.width = pct + '%';
    $('#progress-bar').parentNode.setAttribute('aria-valuenow', String(pct));
  }
  function markVisited(id) {
    var visited = CS.store.get(VISITED_KEY, []);
    if (visited.indexOf(id) === -1) { visited.push(id); CS.store.set(VISITED_KEY, visited); }
    updateProgress();
  }

  /* ---------------- TOC (phải) ---------------- */
  function buildToc(ch) {
    var list = $('#toc-list');
    list.innerHTML = '';
    if (tocObserver) tocObserver.disconnect();
    var heads = $all('h2[id], h3[id]', ch);
    heads.forEach(function (h) {
      var li = document.createElement('li');
      li.className = h.tagName === 'H3' ? 'lvl-3' : 'lvl-2';
      var a = document.createElement('a');
      a.href = '#' + h.id;
      a.textContent = (h.getAttribute('data-toc') || h.textContent).replace(/#$/, '').trim();
      a.addEventListener('click', function () { if (isNarrow()) setToc(false); });
      li.appendChild(a);
      list.appendChild(li);
    });
    if (!('IntersectionObserver' in window) || !heads.length) return;
    var links = $all('a', list);
    tocObserver = new IntersectionObserver(function (entries) {
      entries.forEach(function (en) {
        if (!en.isIntersecting) return;
        var id = en.target.id;
        links.forEach(function (l) { l.classList.toggle('active', l.getAttribute('href') === '#' + id); });
      });
    }, { rootMargin: '-64px 0px -70% 0px' });
    heads.forEach(function (h) { tocObserver.observe(h); });
  }

  /* ---------------- breadcrumbs & pager ---------------- */
  function buildBreadcrumbs(ch, targetEl) {
    var c = chapters.filter(function (x) { return x.id === ch.id; })[0];
    var parts = ['<li><a href="#' + chapters[0].id + '">Trang chủ</a></li>',
      '<li>' + CS.escapeHtml(c.group) + '</li>',
      '<li><a href="#' + c.id + '">' + CS.escapeHtml(c.title) + '</a></li>'];
    if (targetEl && targetEl !== ch && /^H[23]$/.test(targetEl.tagName)) {
      parts.push('<li aria-current="location">' + CS.escapeHtml(targetEl.textContent.replace(/#$/, '').trim()) + '</li>');
    }
    $('#breadcrumbs').innerHTML = '<ol>' + parts.join('') + '</ol>';
  }
  function buildPager(ch) {
    var i = chapters.map(function (c) { return c.id; }).indexOf(ch.id);
    var pager = $('.pager', ch);
    if (!pager) return;
    var prev = chapters[i - 1], next = chapters[i + 1], html = '';
    if (prev) html += '<a class="pager-prev" href="#' + prev.id + '"><span class="pager-dir">← Chương trước</span>' + CS.escapeHtml(prev.title) + '</a>';
    if (next) html += '<a class="pager-next" href="#' + next.id + '"><span class="pager-dir">Chương sau →</span>' + CS.escapeHtml(next.title) + '</a>';
    pager.innerHTML = html;
  }

  /* ---------------- code blocks: copy + highlight ---------------- */
  function enhanceCode(root) {
    $all('.code-block:not([data-enhanced])', root).forEach(function (blk) {
      blk.setAttribute('data-enhanced', 'true');
      var code = $('code', blk);
      if (!code) return;
      if (window.hljs && !code.classList.contains('language-plaintext') && !code.classList.contains('nohighlight')) {
        try { window.hljs.highlightElement(code); } catch (e) { /* ngôn ngữ không hỗ trợ: giữ nguyên */ }
      }
      var btn = document.createElement('button');
      btn.type = 'button';
      btn.className = 'copy-btn';
      btn.textContent = 'Copy';
      btn.setAttribute('aria-label', 'Sao chép đoạn code');
      btn.addEventListener('click', function () {
        CS.copyText(code.textContent).then(function () {
          btn.textContent = 'Đã copy'; btn.classList.add('copied');
          setTimeout(function () { btn.textContent = 'Copy'; btn.classList.remove('copied'); }, 1600);
        }, function () { CS.toast('Không sao chép được — hãy chọn văn bản thủ công'); });
      });
      blk.appendChild(btn);
    });
  }

  /* ---------------- expand/collapse all ---------------- */
  function addSectionTools(ch) {
    if ($('.section-tools', ch)) return;
    var ds = $all('details.collapsible', ch);
    if (!ds.length) return;
    var tools = document.createElement('div');
    tools.className = 'section-tools';
    var openBtn = document.createElement('button');
    openBtn.type = 'button'; openBtn.className = 'text-btn'; openBtn.textContent = 'Mở tất cả mục thu gọn (' + ds.length + ')';
    openBtn.addEventListener('click', function () { ds.forEach(function (d) { d.open = true; }); });
    var closeBtn = document.createElement('button');
    closeBtn.type = 'button'; closeBtn.className = 'text-btn'; closeBtn.textContent = 'Đóng tất cả';
    closeBtn.addEventListener('click', function () { ds.forEach(function (d) { d.open = false; }); });
    tools.appendChild(openBtn); tools.appendChild(closeBtn);
    var header = $('.chapter-header', ch);
    header.insertAdjacentElement('afterend', tools);
  }

  /* ---------------- router ---------------- */
  function chapterOf(el) { return el ? el.closest('article.chapter') : null; }

  function showChapter(ch, targetEl) {
    if (current !== ch) {
      if (current) current.classList.remove('is-current');
      ch.classList.add('is-current');
      current = ch;
      chapters.forEach(function (c) {
        if (c.id === ch.id) c.link.setAttribute('aria-current', 'page'); else c.link.removeAttribute('aria-current');
      });
      document.title = ch.getAttribute('data-title') + ' — Linux Kernel Crypto (5.10)';
      buildToc(ch);
      buildPager(ch);
      addSectionTools(ch);
      enhanceCode(ch);
      CS.renderDiagrams(ch);
      markVisited(ch.id);
      var link = chapters.filter(function (c) { return c.id === ch.id; })[0].link;
      link.scrollIntoView({ block: 'nearest' });
    }
    buildBreadcrumbs(ch, targetEl);
  }

  function route() {
    var id = decodeURIComponent((location.hash || '').slice(1));
    var el = id ? document.getElementById(id) : null;
    var ch = chapterOf(el) || (chapters[0] && chapters[0].el);
    if (!ch) return;
    if (el && el.tagName === 'DETAILS') el.open = true;
    var parentDetails = el && el.closest('details');
    while (parentDetails) { parentDetails.open = true; parentDetails = parentDetails.parentElement.closest('details'); }
    showChapter(ch, el);
    if (isMobile()) setSidebar(false);
    if (isNarrow()) setToc(false);
    requestAnimationFrame(function () {
      if (el && el !== ch) el.scrollIntoView({ block: 'start' });
      else window.scrollTo(0, 0);
    });
  }

  /** Dùng bởi search: điều hướng tới id, sau đó cuộn tới phần tử cụ thể và nháy nền. */
  CS.navigateTo = function (id, el) {
    if (location.hash !== '#' + id) location.hash = id; else route();
    if (el) {
      setTimeout(function () {
        el.scrollIntoView({ block: 'center' });
        el.style.transition = 'background-color .8s';
        var old = el.style.backgroundColor;
        el.style.backgroundColor = 'var(--c-highlight)';
        setTimeout(function () { el.style.backgroundColor = old; }, 1200);
      }, 60);
    }
  };

  /* ---------------- source tree & glossary filters ---------------- */
  function initTreeFilter() {
    var input = $('#tree-filter'), sel = $('#tree-priority');
    if (!input) return;
    function apply() {
      var q = CS.fold(input.value.trim()), pr = sel ? sel.value : '';
      var shown = 0;
      $all('.tree-file').forEach(function (f) {
        var ok = (!q || CS.fold(f.textContent).indexOf(q) !== -1) && (!pr || f.getAttribute('data-priority') === pr);
        f.classList.toggle('hidden-by-filter', !ok);
        if (ok) shown++;
      });
      $all('.tree details').forEach(function (d) {
        var visible = $all('.tree-file:not(.hidden-by-filter)', d).length;
        d.hidden = visible === 0;
        if (q || pr) d.open = visible > 0;
      });
      $('#tree-count').textContent = shown + ' file khớp';
    }
    input.addEventListener('input', apply);
    if (sel) sel.addEventListener('change', apply);
    $('#tree-expand').addEventListener('click', function () { $all('.tree details').forEach(function (d) { d.open = true; }); });
    $('#tree-collapse').addEventListener('click', function () { $all('.tree details').forEach(function (d) { d.open = false; }); });
    apply();
  }
  function initGlossaryFilter() {
    var input = $('#glossary-filter');
    if (!input) return;
    input.addEventListener('input', function () {
      var q = CS.fold(input.value.trim()), n = 0;
      $all('.term').forEach(function (t) {
        var ok = !q || CS.fold(t.textContent).indexOf(q) !== -1;
        t.classList.toggle('hidden-by-filter', !ok);
        if (ok) n++;
      });
      $('#glossary-count').textContent = n + ' thuật ngữ';
    });
  }

  /* ---------------- back to top, print ---------------- */
  function initMisc() {
    var btt = $('#back-to-top');
    window.addEventListener('scroll', function () { btt.hidden = window.scrollY < 600; }, { passive: true });
    btt.addEventListener('click', function () { window.scrollTo({ top: 0 }); $('#main').focus({ preventScroll: true }); });
    $('#print-btn').addEventListener('click', function () {
      CS.renderDiagrams(current).then(function () { window.print(); });
    });
    // In: mở tạm mọi khối thu gọn của chương hiện tại (Chromium không in nội dung <details> đóng),
    // khôi phục sau khi in. Dùng sự kiện beforeprint/afterprint nên cũng áp dụng cho Ctrl+P.
    var reopened = [];
    window.addEventListener('beforeprint', function () {
      reopened = current ? $all('details:not([open])', current) : [];
      reopened.forEach(function (d) { d.open = true; });
    });
    window.addEventListener('afterprint', function () {
      reopened.forEach(function (d) { d.open = false; });
      reopened = [];
    });
    $('#progress-reset').addEventListener('click', function () { CS.store.set(VISITED_KEY, []); markVisited(current.id); });
  }

  /* ---------------- init ---------------- */
  function init() {
    chapters = $all('#sidebar a[data-chapter]').map(function (a) {
      var el = document.getElementById(a.getAttribute('data-chapter'));
      return { id: el.id, title: el.getAttribute('data-title'), group: el.getAttribute('data-group'), el: el, link: a };
    });
    initTheme();
    initPanels();
    initTreeFilter();
    initGlossaryFilter();
    initMisc();
    var n = CS.buildSearchIndex();
    CS.initSearchUI();
    document.documentElement.setAttribute('data-search-entries', String(n));
    window.addEventListener('hashchange', route);
    window.addEventListener('resize', function () {
      if (!isMobile()) document.body.classList.remove('sidebar-open');
    });
    route();
  }

  if (document.readyState === 'loading') document.addEventListener('DOMContentLoaded', init); else init();
})();
