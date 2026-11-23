/* ==========================================================================
   diagrams.js — render Mermaid có kiểm soát + toolbar + modal phóng to.

   Cấu trúc HTML của một diagram card (xem diagrams/README.md):
     <figure class="diagram-card" id="dg-..." data-type="sequence" data-status="verified">
       <figcaption class="diagram-head">…tiêu đề, mục đích…</figcaption>
       <div class="diagram-canvas"><div class="diagram-render"></div></div>
       <script type="text/x-mermaid" class="diagram-source"> …Mermaid source… </script>
       <div class="diagram-notes">…chú thích, file/hàm liên quan…</div>
     </figure>
   Mermaid source là nguồn duy nhất; toolbar, xem source, copy, zoom đều đọc từ thẻ <script>.
   ========================================================================== */
(function () {
  'use strict';
  var CS = (window.CryptoSite = window.CryptoSite || {});
  var renderCounter = 0;
  var mermaidReady = false;
  var queue = Promise.resolve();   // render tuần tự: Mermaid không an toàn khi render song song

  function initMermaid() {
    if (mermaidReady) return true;
    if (!window.mermaid) return false;
    try {
      window.mermaid.initialize(window.CRYPTO_SITE_MERMAID_CONFIG || { startOnLoad: false });
      mermaidReady = true;
    } catch (e) {
      console.error('[diagrams] mermaid.initialize failed', e);
    }
    return mermaidReady;
  }

  function sourceOf(card) {
    var s = card.querySelector('script.diagram-source');
    return s ? s.textContent.replace(/^\s*\n/, '').replace(/\s+$/, '') + '\n' : '';
  }

  function titleOf(card) {
    var t = card.querySelector('.diagram-title');
    return t ? t.textContent.trim() : card.id;
  }

  function makeButton(label, icon, handler, extraAttrs) {
    var b = document.createElement('button');
    b.type = 'button';
    b.className = 'text-btn';
    b.innerHTML = '<span aria-hidden="true">' + icon + '</span> ' + label;
    b.addEventListener('click', handler);
    if (extraAttrs) Object.keys(extraAttrs).forEach(function (k) { b.setAttribute(k, extraAttrs[k]); });
    return b;
  }

  /** Thêm toolbar (phóng to / copy source / xem source) cho các card chưa chuẩn bị. */
  CS.prepareDiagrams = function (root) {
    var cards = (root || document).querySelectorAll('.diagram-card:not([data-prepared])');
    Array.prototype.forEach.call(cards, function (card) {
      card.setAttribute('data-prepared', 'true');
      var head = card.querySelector('.diagram-head');
      var bar = document.createElement('div');
      bar.className = 'diagram-toolbar';
      bar.setAttribute('role', 'toolbar');
      bar.setAttribute('aria-label', 'Công cụ sơ đồ: ' + titleOf(card));

      var srcView = document.createElement('div');
      srcView.className = 'diagram-source-view code-block';
      srcView.hidden = true;
      var pre = document.createElement('pre');
      var code = document.createElement('code');
      code.className = 'language-plaintext';
      code.textContent = sourceOf(card);
      pre.appendChild(code);
      srcView.appendChild(pre);
      srcView.id = card.id + '-source';

      bar.appendChild(makeButton('Phóng to', '⤢', function () { CS.openDiagramModal(card); }));
      bar.appendChild(makeButton('Sao chép Mermaid', '⧉', function () {
        CS.copyText(sourceOf(card)).then(function () { CS.toast('Đã sao chép Mermaid source'); },
          function () { CS.toast('Không sao chép được — hãy chọn thủ công trong "Xem source"'); });
      }));
      var toggle = makeButton('Xem source', '‹›', function () {
        srcView.hidden = !srcView.hidden;
        toggle.setAttribute('aria-expanded', String(!srcView.hidden));
        toggle.lastChild.textContent = srcView.hidden ? ' Xem source' : ' Ẩn source';
      }, { 'aria-expanded': 'false', 'aria-controls': srcView.id });
      bar.appendChild(toggle);
      head.insertAdjacentElement('afterend', bar);

      var canvas = card.querySelector('.diagram-canvas');
      canvas.insertAdjacentElement('afterend', srcView);
      var hint = document.createElement('div');
      hint.className = 'diagram-hint';
      hint.hidden = true;               // nội dung do fitInline() đặt sau khi render
      hint.setAttribute('aria-live', 'polite');
      srcView.insertAdjacentElement('afterend', hint);
    });
  };

  function showError(card, err) {
    var target = card.querySelector('.diagram-render');
    var msg = (err && (err.message || err.str || String(err))) || 'Lỗi không xác định';
    target.innerHTML = '<div class="diagram-error" role="alert"><strong>Không render được sơ đồ.</strong> ' +
      'Nội dung chương vẫn dùng được; xem Mermaid source bằng nút "Xem source".<pre>' +
      CS.escapeHtml(msg) + '</pre></div>';
    card.setAttribute('data-rendered', 'error');
    console.error('[diagrams] render failed:', card.id, err);
  }

  function renderOne(card) {
    if (card.getAttribute('data-rendered')) return Promise.resolve();
    var target = card.querySelector('.diagram-render');
    if (!initMermaid()) {
      showError(card, new Error('Không tải được thư viện Mermaid (assets/vendor/mermaid/mermaid.min.js).'));
      return Promise.resolve();
    }
    card.setAttribute('data-rendered', 'pending');
    var src = sourceOf(card);
    var id = 'mmd-' + (card.id || 'x') + '-' + (++renderCounter);
    return window.mermaid.parse(src).then(function () {
      return window.mermaid.render(id, src);
    }).then(function (res) {
      target.innerHTML = res.svg;
      var svg = target.querySelector('svg');
      if (svg) {
        svg.setAttribute('role', 'img');
        svg.setAttribute('aria-label', 'Sơ đồ: ' + titleOf(card));
        svg.removeAttribute('style');           // bỏ max-width do Mermaid gắn, để canvas tự cuộn
        var vb = svg.viewBox && svg.viewBox.baseVal;
        if (vb && vb.width) { svg.setAttribute('width', Math.ceil(vb.width)); svg.setAttribute('height', Math.ceil(vb.height)); }
      }
      if (res.bindFunctions) res.bindFunctions(target);
      card.setAttribute('data-rendered', 'true');
      fitInline(card);
    }).catch(function (err) {
      showError(card, err);
      var stray = document.getElementById('d' + id);  // phần tử tạm Mermaid có thể để lại
      if (stray && stray.parentNode) stray.parentNode.removeChild(stray);
    });
  }

  /* Sơ đồ rộng hơn khung: thu nhỏ vừa khung nếu chữ vẫn đọc được (≥ MIN_SCALE),
     nếu không thì giữ kích thước thật và cho cuộn ngang. Modal luôn dùng kích thước thật. */
  var MIN_SCALE = 0.72;          // khung rộng: dưới mức này giữ kích thước thật + cuộn ngang
  var NARROW_PX = 700;           // khung hẹp (điện thoại): luôn thu vừa khung để thấy toàn cảnh
  function fitInline(card) {
    var canvas = card.querySelector('.diagram-canvas');
    var svg = card.querySelector('.diagram-render svg');
    var hint = card.querySelector('.diagram-hint');
    var vb = svg && svg.viewBox && svg.viewBox.baseVal;
    if (!canvas || !vb || !vb.width || !canvas.clientWidth) return;
    var cs = window.getComputedStyle(canvas);
    var avail = canvas.clientWidth - parseFloat(cs.paddingLeft) - parseFloat(cs.paddingRight);
    var scale = Math.min(1, avail / vb.width);
    var mode = scale >= 1 ? 'natural' : ((scale >= MIN_SCALE || avail < NARROW_PX) ? 'scaled' : 'scroll');
    if (mode === 'scroll') scale = 1;
    svg.setAttribute('width', Math.floor(vb.width * scale));
    svg.setAttribute('height', Math.floor(vb.height * scale));
    card.setAttribute('data-fit', mode);
    if (hint) {
      hint.hidden = mode === 'natural';
      hint.textContent = mode === 'scaled'
        ? 'Đã thu nhỏ còn ' + Math.round(scale * 100) + '% để vừa khung — bấm "Phóng to" để xem chi tiết.'
        : 'Sơ đồ rộng hơn khung: cuộn ngang trong khung hoặc bấm "Phóng to" để xem toàn màn hình.';
    }
  }
  CS.refitDiagrams = function (root) {
    Array.prototype.forEach.call((root || document).querySelectorAll('.diagram-card[data-rendered="true"]'), fitInline);
  };
  var refitTimer = null;
  window.addEventListener('resize', function () {
    clearTimeout(refitTimer);
    refitTimer = setTimeout(function () { CS.refitDiagrams(document.querySelector('article.chapter.is-current')); }, 150);
  });

  /** Render tuần tự mọi diagram trong root (một chương). Trả về Promise hoàn tất. */
  CS.renderDiagrams = function (root) {
    CS.prepareDiagrams(root);
    var cards = Array.prototype.slice.call((root || document).querySelectorAll('.diagram-card'));
    cards.forEach(function (card) {
      queue = queue.then(function () { return renderOne(card); });
    });
    return queue;
  };

  /* ---------------- Modal phóng to ---------------- */
  var modal, modalBody, modalTitle, zoomLabel, currentSvg, naturalW = 0, naturalH = 0, scale = 1, lastFocus = null;

  function applyScale() {
    if (!currentSvg) return;
    currentSvg.setAttribute('width', Math.round(naturalW * scale));
    currentSvg.setAttribute('height', Math.round(naturalH * scale));
    zoomLabel.textContent = Math.round(scale * 100) + '%';
  }
  function fit() {
    if (!currentSvg) return;
    var availW = modalBody.clientWidth - 32, availH = modalBody.clientHeight - 32;
    // Màn hẹp: vừa chiều ngang rồi cuộn dọc (đọc được hơn); màn rộng: vừa cả khung.
    var s = window.innerWidth < 700 ? availW / naturalW : Math.min(availW / naturalW, availH / naturalH);
    scale = Math.max(0.2, Math.min(s, 2));
    applyScale();
  }
  function zoom(f) { scale = Math.max(0.2, Math.min(scale * f, 5)); applyScale(); }

  function ensureModal() {
    if (modal) return;
    modal = document.getElementById('diagram-modal');
    modalBody = modal.querySelector('.modal-body');
    modalTitle = modal.querySelector('.modal-title');
    zoomLabel = modal.querySelector('.zoom-level');
    modal.querySelector('[data-zoom="in"]').addEventListener('click', function () { zoom(1.25); });
    modal.querySelector('[data-zoom="out"]').addEventListener('click', function () { zoom(0.8); });
    modal.querySelector('[data-zoom="reset"]').addEventListener('click', function () { scale = 1; applyScale(); });
    modal.querySelector('[data-zoom="fit"]').addEventListener('click', fit);
    modal.querySelector('[data-modal-close]').addEventListener('click', CS.closeDiagramModal);
    modal.addEventListener('click', function (e) { if (e.target === modal) CS.closeDiagramModal(); });
    modal.addEventListener('keydown', function (e) {
      if (e.key === 'Escape') { e.preventDefault(); CS.closeDiagramModal(); }
      else if (e.key === '+' || e.key === '=') { zoom(1.25); }
      else if (e.key === '-') { zoom(0.8); }
      else if (e.key === 'Tab') {               // giữ focus trong modal
        var f = modal.querySelectorAll('button, [tabindex="0"]');
        var first = f[0], last = f[f.length - 1];
        if (e.shiftKey && document.activeElement === first) { e.preventDefault(); last.focus(); }
        else if (!e.shiftKey && document.activeElement === last) { e.preventDefault(); first.focus(); }
      }
    });
    // Kéo để di chuyển (pan)
    var drag = null;
    modalBody.addEventListener('mousedown', function (e) {
      drag = { x: e.clientX, y: e.clientY, l: modalBody.scrollLeft, t: modalBody.scrollTop };
      modalBody.classList.add('dragging');
    });
    window.addEventListener('mousemove', function (e) {
      if (!drag) return;
      modalBody.scrollLeft = drag.l - (e.clientX - drag.x);
      modalBody.scrollTop = drag.t - (e.clientY - drag.y);
    });
    window.addEventListener('mouseup', function () { drag = null; modalBody.classList.remove('dragging'); });
  }

  CS.openDiagramModal = function (card) {
    ensureModal();
    var svg = card.querySelector('.diagram-render svg');
    if (!svg) { CS.toast('Sơ đồ chưa render xong'); return; }
    lastFocus = document.activeElement;
    modalTitle.textContent = titleOf(card);
    modalBody.innerHTML = '';
    currentSvg = svg.cloneNode(true);
    var vb = currentSvg.viewBox && currentSvg.viewBox.baseVal;
    naturalW = (vb && vb.width) || svg.getBoundingClientRect().width;
    naturalH = (vb && vb.height) || svg.getBoundingClientRect().height;
    modalBody.appendChild(currentSvg);
    modal.hidden = false;
    document.body.style.overflow = 'hidden';
    fit();
    modal.querySelector('[data-modal-close]').focus();
  };

  CS.closeDiagramModal = function () {
    if (!modal || modal.hidden) return;
    modal.hidden = true;
    modalBody.innerHTML = '';
    currentSvg = null;
    document.body.style.overflow = '';
    if (lastFocus && lastFocus.focus) lastFocus.focus();
  };
})();
