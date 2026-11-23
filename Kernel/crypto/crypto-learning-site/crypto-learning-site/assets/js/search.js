/* ==========================================================================
   search.js — tìm kiếm toàn văn phía client, không cần backend.
   Chỉ mục được dựng từ DOM khi tải trang ⇒ sửa HTML là chỉ mục tự cập nhật.
   - Không phân biệt hoa/thường, bỏ dấu tiếng Việt (CryptoSite.fold).
   - Mọi từ khoá phải xuất hiện (AND); điểm cao hơn cho tiêu đề/tên định danh.
   - Phím: "/" để focus, ↑/↓ chọn, Enter mở, Esc đóng.
   ========================================================================== */
(function () {
  'use strict';
  var CS = (window.CryptoSite = window.CryptoSite || {});
  var index = [];
  var TYPE_LABEL = {
    chapter: 'Chương', section: 'Mục', diagram: 'Sơ đồ', fn: 'Hàm', struct: 'Struct',
    term: 'Thuật ngữ', file: 'File', content: 'Nội dung'
  };
  var TYPE_WEIGHT = { chapter: 60, fn: 55, struct: 55, term: 50, diagram: 45, file: 45, section: 40, content: 0 };
  var MAX_RESULTS = 40;

  function text(el) { return el ? el.textContent.replace(/\s+/g, ' ').trim() : ''; }

  function nearestHeadingId(el, chapter) {
    // Tìm heading h2/h3 gần nhất phía trước el trong cùng chương → làm đích điều hướng.
    var all = chapter.querySelectorAll('h2[id], h3[id], .diagram-card[id], .card[id]');
    var best = chapter.id;
    for (var i = 0; i < all.length; i++) {
      var pos = all[i].compareDocumentPosition(el);
      if (all[i] === el || all[i].contains(el)) { best = all[i].id; break; }
      if (pos & Node.DOCUMENT_POSITION_FOLLOWING) best = all[i].id; else break;
    }
    return best;
  }

  function add(type, title, body, target, chapter, el) {
    var hay = title + ' ' + body;
    index.push({
      type: type, title: title, body: body, target: target, el: el || null,
      chapterTitle: chapter ? chapter.getAttribute('data-title') : '',
      fTitle: CS.fold(title), fBody: CS.fold(body), fAll: CS.fold(hay)
    });
  }

  CS.buildSearchIndex = function () {
    index = [];
    var chapters = document.querySelectorAll('article.chapter');
    Array.prototype.forEach.call(chapters, function (ch) {
      add('chapter', ch.getAttribute('data-title') || text(ch.querySelector('h1')),
        text(ch.querySelector('.chapter-lead')), ch.id, ch, ch);
      Array.prototype.forEach.call(ch.querySelectorAll('h2[id], h3[id]'), function (h) {
        var next = h.nextElementSibling;
        add('section', text(h).replace(/#$/, ''), next ? text(next).slice(0, 220) : '', h.id, ch, h);
      });
      Array.prototype.forEach.call(ch.querySelectorAll('.diagram-card[id]'), function (d) {
        var src = d.querySelector('script.diagram-source');
        add('diagram', text(d.querySelector('.diagram-title')),
          text(d.querySelector('.diagram-purpose')) + ' ' + text(d.querySelector('.diagram-notes')) + ' ' + (src ? src.textContent : '') + ' ' + d.id,
          d.id, ch, d);
      });
      Array.prototype.forEach.call(ch.querySelectorAll('.card-fn[id], .card-struct[id]'), function (c) {
        add(c.classList.contains('card-fn') ? 'fn' : 'struct', text(c.querySelector('.card-name')),
          text(c.querySelector('.card-loc')) + ' ' + text(c.querySelector('dl')), c.id, ch, c);
      });
      Array.prototype.forEach.call(ch.querySelectorAll('.term[id]'), function (t) {
        add('term', text(t.querySelector('.term-en')) + ' — ' + text(t.querySelector('.term-vi')),
          text(t.querySelector('.term-body')), t.id, ch, t);
      });
      Array.prototype.forEach.call(ch.querySelectorAll('.tree-file[id]'), function (f) {
        add('file', text(f.querySelector('.f-name')), text(f.querySelector('.f-desc')) + ' ' + text(f.querySelector('.f-tags')), f.id, ch, f);
      });
      // Nội dung thường: đoạn văn, mục danh sách, hàng bảng, khối code
      var blocks = ch.querySelectorAll('p, li, tr, pre');
      Array.prototype.forEach.call(blocks, function (b) {
        if (b.closest('.diagram-card, .card-fn, .card-struct, .term, .tree-file, nav, .chapter-header')) return;
        var t = text(b);
        if (t.length < 12) return;
        add('content', '', t.slice(0, 600), nearestHeadingId(b, ch), ch, b);
      });
    });
    return index.length;
  };

  function score(entry, terms) {
    var s = TYPE_WEIGHT[entry.type] || 0;
    for (var i = 0; i < terms.length; i++) {
      var t = terms[i];
      if (entry.fAll.indexOf(t) === -1) return -1;
      if (entry.fTitle === t) s += 120;
      else if (entry.fTitle.indexOf(t) === 0) s += 60;
      else if (entry.fTitle.indexOf(t) !== -1) s += 35;
      else s += 5;
    }
    return s;
  }

  /** Bọc từ khoá bằng <mark> trên chuỗi GỐC dựa vào vị trí tìm thấy trên chuỗi đã fold. */
  function highlight(original, folded, terms, maxLen) {
    var first = -1;
    terms.forEach(function (t) { var p = folded.indexOf(t); if (p !== -1 && (first === -1 || p < first)) first = p; });
    var start = 0, end = original.length;
    if (maxLen && original.length > maxLen) {
      start = Math.max(0, first - Math.floor(maxLen / 3));
      end = Math.min(original.length, start + maxLen);
    }
    var o = original.slice(start, end), f = folded.slice(start, end);
    var marks = [];
    terms.forEach(function (t) {
      if (!t) return;
      var p = f.indexOf(t);
      while (p !== -1) { marks.push([p, p + t.length]); p = f.indexOf(t, p + t.length); }
    });
    marks.sort(function (a, b) { return a[0] - b[0]; });
    var out = '', pos = 0;
    marks.forEach(function (m) {
      if (m[0] < pos) return;
      out += CS.escapeHtml(o.slice(pos, m[0])) + '<mark>' + CS.escapeHtml(o.slice(m[0], m[1])) + '</mark>';
      pos = m[1];
    });
    out += CS.escapeHtml(o.slice(pos));
    return (start > 0 ? '… ' : '') + out + (end < original.length ? ' …' : '');
  }

  CS.search = function (query) {
    var terms = CS.fold(query).split(/\s+/).filter(Boolean);
    if (!terms.length) return { terms: terms, results: [] };
    var res = [];
    index.forEach(function (e) {
      var sc = score(e, terms);
      if (sc >= 0) res.push({ e: e, s: sc });
    });
    res.sort(function (a, b) { return b.s - a.s; });
    return { terms: terms, results: res };
  };

  /* ---------------- UI ---------------- */
  CS.initSearchUI = function () {
    var input = document.getElementById('search-input');
    var box = document.getElementById('search-results');
    if (!input || !box) return;
    var items = [], sel = -1, timer = null;

    function close() { box.hidden = true; input.setAttribute('aria-expanded', 'false'); sel = -1; }
    function select(i) {
      items.forEach(function (it, k) { it.setAttribute('aria-selected', k === i ? 'true' : 'false'); });
      sel = i;
      if (items[i]) items[i].scrollIntoView({ block: 'nearest' });
    }
    function go(entry) {
      close();
      input.blur();
      if (CS.navigateTo) CS.navigateTo(entry.target, entry.el);
    }
    function run() {
      var q = input.value.trim();
      if (q.length < 2) { close(); return; }
      var r = CS.search(q);
      var list = r.results.slice(0, MAX_RESULTS);
      var html = '<div class="search-meta">' + r.results.length + ' kết quả' +
        (r.results.length > MAX_RESULTS ? ' (hiển thị ' + MAX_RESULTS + ' kết quả tốt nhất)' : '') + '</div>';
      list.forEach(function (x, i) {
        var e = x.e;
        var title = e.title ? highlight(e.title, e.fTitle, r.terms) : CS.escapeHtml(e.chapterTitle);
        var snippet = e.body ? highlight(e.body, e.fBody, r.terms, 170) : '';
        html += '<a class="search-item" role="option" id="sr-' + i + '" href="#' + e.target + '" data-i="' + i + '" aria-selected="false">' +
          '<span class="search-item-head"><span class="badge">' + TYPE_LABEL[e.type] + '</span>' +
          '<span class="search-item-title">' + title + '</span>' +
          '<span class="search-item-where">' + CS.escapeHtml(e.chapterTitle) + '</span></span>' +
          (snippet ? '<span class="search-item-snippet">' + snippet + '</span>' : '') + '</a>';
      });
      if (!list.length) html += '<div class="search-meta">Không tìm thấy. Thử từ khoá ngắn hơn hoặc không dấu.</div>';
      box.innerHTML = html;
      box.hidden = false;
      input.setAttribute('aria-expanded', 'true');
      items = Array.prototype.slice.call(box.querySelectorAll('.search-item'));
      items.forEach(function (a) {
        a.addEventListener('click', function (ev) { ev.preventDefault(); go(list[+a.getAttribute('data-i')].e); });
      });
      sel = -1;
      box._list = list;
    }
    input.addEventListener('input', function () { clearTimeout(timer); timer = setTimeout(run, 120); });
    input.addEventListener('focus', function () { if (input.value.trim().length >= 2) run(); });
    input.addEventListener('keydown', function (e) {
      if (e.key === 'ArrowDown') { e.preventDefault(); if (items.length) select(Math.min(sel + 1, items.length - 1)); }
      else if (e.key === 'ArrowUp') { e.preventDefault(); if (items.length) select(Math.max(sel - 1, 0)); }
      else if (e.key === 'Enter') {
        e.preventDefault();
        var list = box._list || [];
        if (list.length) go(list[sel >= 0 ? sel : 0].e);
      } else if (e.key === 'Escape') { close(); input.blur(); }
    });
    document.addEventListener('click', function (e) { if (!e.target.closest('.search')) close(); });
    document.addEventListener('keydown', function (e) {
      if (e.key === '/' && !/^(INPUT|TEXTAREA|SELECT)$/.test(document.activeElement.tagName)) {
        e.preventDefault(); input.focus(); input.select();
      }
    });
  };
})();
