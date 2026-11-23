/* ==========================================================================
   util.js — tiện ích dùng chung cho toàn site.
   Tất cả module gắn vào namespace window.CryptoSite để tránh biến toàn cục rời rạc.
   ========================================================================== */
(function () {
  'use strict';
  var CS = (window.CryptoSite = window.CryptoSite || {});

  /** Bỏ dấu tiếng Việt + chữ thường, GIỮ NGUYÊN độ dài chuỗi (map 1 ký tự → 1 ký tự)
   *  để có thể dùng vị trí trên chuỗi đã chuẩn hoá để highlight chuỗi gốc. */
  function foldChar(ch) {
    var lower = ch.toLowerCase();
    if (lower.length !== 1) return ch;
    if (lower === 'đ') return 'd';
    var base = lower.normalize('NFD').charAt(0);
    return base || lower;
  }
  CS.fold = function (text) {
    var out = '';
    for (var i = 0; i < text.length; i++) out += foldChar(text.charAt(i));
    return out;
  };

  CS.escapeHtml = function (s) {
    return String(s).replace(/[&<>"']/g, function (c) {
      return { '&': '&amp;', '<': '&lt;', '>': '&gt;', '"': '&quot;', "'": '&#39;' }[c];
    });
  };

  /** Toast ngắn ở cuối màn hình (thông báo đã copy, lỗi...). */
  var toastTimer = null;
  CS.toast = function (msg) {
    var el = document.getElementById('toast');
    if (!el) return;
    el.textContent = msg;
    el.hidden = false;
    clearTimeout(toastTimer);
    toastTimer = setTimeout(function () { el.hidden = true; }, 2200);
  };

  /** Sao chép văn bản: dùng Clipboard API, nếu không có (file://, trình duyệt cũ) thì fallback textarea. */
  CS.copyText = function (text) {
    function fallback() {
      return new Promise(function (resolve, reject) {
        var ta = document.createElement('textarea');
        ta.value = text;
        ta.setAttribute('readonly', '');
        ta.style.position = 'fixed';
        ta.style.top = '-1000px';
        document.body.appendChild(ta);
        ta.select();
        var ok = false;
        try { ok = document.execCommand('copy'); } catch (e) { ok = false; }
        document.body.removeChild(ta);
        if (ok) { resolve(); } else { reject(new Error('copy failed')); }
      });
    }
    if (navigator.clipboard && window.isSecureContext) {
      return navigator.clipboard.writeText(text).catch(fallback);
    }
    return fallback();
  };

  /** localStorage an toàn (private mode / bị chặn → không ném lỗi). */
  CS.store = {
    get: function (key, dflt) {
      try { var v = localStorage.getItem(key); return v === null ? dflt : JSON.parse(v); } catch (e) { return dflt; }
    },
    set: function (key, value) {
      try { localStorage.setItem(key, JSON.stringify(value)); return true; } catch (e) { return false; }
    }
  };
})();
