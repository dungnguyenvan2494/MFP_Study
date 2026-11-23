/* dbgAllClear Deep Dive — Mermaid rendering (lazy, theme-aware) + zoom dialog */
(function () {
  'use strict';

  var counter = 0;
  var queue = Promise.resolve();
  var rendered = [];

  function themeConfig() {
    var dark = window.DocTheme && window.DocTheme.isDark();
    return {
      startOnLoad: false,
      securityLevel: 'strict',
      theme: dark ? 'dark' : 'default',
      fontFamily: 'Inter, system-ui, -apple-system, Segoe UI, sans-serif',
      themeVariables: dark ? {
        background: '#111827', primaryColor: '#1E293B', primaryTextColor: '#E5E7EB',
        lineColor: '#94A3B8', primaryBorderColor: '#475569', fontSize: '14px'
      } : {
        background: '#FFFFFF', primaryColor: '#F1F5F9', primaryTextColor: '#172033',
        lineColor: '#64748B', primaryBorderColor: '#94A3B8', fontSize: '14px'
      },
      flowchart: { htmlLabels: true, curve: 'basis', useMaxWidth: true, nodeSpacing: 36, rankSpacing: 44, padding: 10 },
      sequence: { useMaxWidth: true, mirrorActors: false, boxMargin: 8, messageMargin: 32, wrap: true, width: 170 },
      state: { useMaxWidth: true }
    };
  }

  function fallback(body, src, msg) {
    body.classList.remove('loading');
    body.innerHTML = '';
    var wrap = document.createElement('div');
    wrap.className = 'diagram-fallback';
    var p = document.createElement('p');
    p.className = 'caption';
    p.textContent = msg;
    var pre = document.createElement('pre');
    pre.textContent = src;
    wrap.appendChild(p);
    wrap.appendChild(pre);
    body.appendChild(wrap);
  }

  // The diagram source lives in <script class="mmd"> inside .diagram-body; rendering replaces the body,
  // so the source is cached on the element first (needed for theme re-renders).
  function sourceOf(fig) {
    if (fig.__mmdSrc === undefined) {
      var srcEl = fig.querySelector('script.mmd');
      fig.__mmdSrc = srcEl ? srcEl.textContent.trim() : '';
    }
    return fig.__mmdSrc;
  }

  function renderOne(fig) {
    var body = fig.querySelector('.diagram-body');
    var src = sourceOf(fig);
    if (!src || !body) return Promise.resolve();
    if (!window.mermaid) {
      fallback(body, src, 'Không tải được thư viện Mermaid (assets/vendor/mermaid.min.js). Hiển thị mã nguồn sơ đồ:');
      return Promise.resolve();
    }
    var id = 'mmd-' + (++counter);
    return window.mermaid.render(id, src).then(function (res) {
      body.classList.remove('loading');
      body.innerHTML = res.svg;
      var svg = body.querySelector('svg');
      if (svg) { svg.setAttribute('role', 'img'); svg.setAttribute('aria-label', fig.getAttribute('data-title') || 'Sơ đồ'); }
      fig.setAttribute('data-rendered', '1');
      if (rendered.indexOf(fig) === -1) rendered.push(fig);
    }).catch(function (err) {
      var stray = document.getElementById('d' + id);
      if (stray && stray.parentNode) stray.parentNode.removeChild(stray);
      fallback(body, src, 'Lỗi render sơ đồ: ' + (err && err.message ? err.message : err));
    });
  }

  function enqueue(fig) {
    if (fig.getAttribute('data-queued')) return;
    fig.setAttribute('data-queued', '1');
    queue = queue.then(function () { return renderOne(fig); });
  }

  function buildChrome(fig) {
    var title = fig.getAttribute('data-title') || 'Sơ đồ';
    var bar = document.createElement('div');
    bar.className = 'diagram-toolbar';
    var t = document.createElement('span');
    t.className = 't';
    t.textContent = title;
    var zoom = document.createElement('button');
    zoom.type = 'button';
    zoom.textContent = 'Phóng to';
    zoom.setAttribute('aria-label', 'Phóng to sơ đồ: ' + title);
    zoom.addEventListener('click', function () { openZoom(fig); });
    bar.appendChild(t);
    bar.appendChild(zoom);
    fig.insertBefore(bar, fig.firstChild);
    var body = fig.querySelector('.diagram-body');
    if (body) {
      body.classList.add('loading');
      body.addEventListener('dblclick', function () { openZoom(fig); });
    }
  }

  /* -------- zoom dialog -------- */
  var dlg, stage, scale = 1, baseW = 0;
  function applyScale() {
    var svg = stage.querySelector('svg');
    if (!svg) return;
    svg.style.width = Math.round(baseW * scale) + 'px';
    svg.style.maxWidth = 'none';
    var lbl = document.getElementById('zoom-level');
    if (lbl) lbl.textContent = Math.round(scale * 100) + '%';
  }
  function openZoom(fig) {
    var svg = fig.querySelector('.diagram-body svg');
    if (!svg || !dlg) return;
    stage.innerHTML = '';
    var clone = svg.cloneNode(true);
    clone.removeAttribute('height');
    stage.appendChild(clone);
    var vb = (svg.getAttribute('viewBox') || '').split(/\s+/);
    var rect = svg.getBoundingClientRect();
    baseW = vb.length === 4 ? parseFloat(vb[2]) : rect.width;
    var baseH = vb.length === 4 ? parseFloat(vb[3]) : rect.height;
    document.getElementById('zoom-title').textContent = fig.getAttribute('data-title') || 'Sơ đồ';
    if (typeof dlg.showModal === 'function') dlg.showModal(); else dlg.setAttribute('open', '');
    // fit the whole diagram into the stage (width and height), within sane bounds
    // wide diagrams: fit the width; tall diagrams: fit the height but never below 100% (scroll instead)
    var fitW = (stage.clientWidth - 40) / Math.max(baseW, 1);
    var fitH = (stage.clientHeight - 40) / Math.max(baseH, 1);
    scale = Math.min(2, Math.max(0.5, Math.min(fitW, Math.max(fitH, 1))));
    applyScale();
  }
  function setupZoom() {
    dlg = document.getElementById('zoom-dialog');
    if (!dlg) return;
    stage = dlg.querySelector('.zoom-stage');
    dlg.querySelector('[data-z="in"]').addEventListener('click', function () { scale = Math.min(6, scale * 1.25); applyScale(); });
    dlg.querySelector('[data-z="out"]').addEventListener('click', function () { scale = Math.max(0.3, scale / 1.25); applyScale(); });
    dlg.querySelector('[data-z="reset"]').addEventListener('click', function () { scale = 1; applyScale(); });
    dlg.querySelector('[data-z="close"]').addEventListener('click', function () { dlg.close ? dlg.close() : dlg.removeAttribute('open'); });
    dlg.addEventListener('click', function (e) { if (e.target === dlg) dlg.close(); });
  }

  function init() {
    var figs = Array.prototype.slice.call(document.querySelectorAll('figure.diagram'));
    figs.forEach(function (f) { sourceOf(f); buildChrome(f); });
    setupZoom();
    if (window.mermaid) {
      try { window.mermaid.initialize(themeConfig()); } catch (e) { /* ignore */ }
    }
    if ('IntersectionObserver' in window) {
      var io = new IntersectionObserver(function (entries) {
        entries.forEach(function (en) {
          if (en.isIntersecting) { enqueue(en.target); io.unobserve(en.target); }
        });
      }, { rootMargin: '600px 0px' });
      figs.forEach(function (f) { io.observe(f); });
    } else {
      figs.forEach(enqueue);
    }
    document.addEventListener('doc:themechange', function () {
      if (!window.mermaid) return;
      try { window.mermaid.initialize(themeConfig()); } catch (e) { /* ignore */ }
      rendered.slice().forEach(function (fig) {
        fig.removeAttribute('data-queued');
        var body = fig.querySelector('.diagram-body');
        if (body) { body.innerHTML = ''; body.classList.add('loading'); }
        enqueue(fig);
      });
    });
  }

  window.DocDiagrams = { renderAllNow: function () {
    Array.prototype.forEach.call(document.querySelectorAll('figure.diagram'), enqueue);
    return queue.then(function () { return queue; });
  } };

  if (document.readyState === 'loading') document.addEventListener('DOMContentLoaded', init); else init();
})();
