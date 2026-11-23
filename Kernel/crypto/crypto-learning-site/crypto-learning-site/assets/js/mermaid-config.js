/* ==========================================================================
   mermaid-config.js — cấu hình Mermaid DUY NHẤT cho toàn site.
   Phiên bản Mermaid: 11.17.2 (assets/vendor/mermaid/mermaid.min.js).
   File này cũng được script kiểm tra (mmdc) đọc lại để render giống hệt trình duyệt.
   Chỉ dùng các khoá cấu hình mà Mermaid 11.x hỗ trợ.
   ========================================================================== */
(function (root) {
  'use strict';
  var config = {
    startOnLoad: false,
    securityLevel: 'strict',
    suppressErrorRendering: true,
    theme: 'base',
    fontFamily: 'system-ui, -apple-system, "Segoe UI", Roboto, "Noto Sans", Arial, sans-serif',
    themeVariables: {
      fontSize: '15px',
      primaryColor: '#E8EEFC',
      primaryBorderColor: '#1D4ED8',
      primaryTextColor: '#172033',
      secondaryColor: '#F1F5F9',
      tertiaryColor: '#FFFFFF',
      lineColor: '#475569',
      textColor: '#172033',
      edgeLabelBackground: '#FFFFFF',
      clusterBkg: '#F8FAFD',
      clusterBorder: '#94A3B8',
      actorBkg: '#E8EEFC',
      actorBorder: '#1D4ED8',
      actorTextColor: '#0F1B33',
      actorLineColor: '#94A3B8',
      signalColor: '#334155',
      signalTextColor: '#172033',
      labelBoxBkgColor: '#F1F5F9',
      labelBoxBorderColor: '#64748B',
      labelTextColor: '#172033',
      loopTextColor: '#172033',
      noteBkgColor: '#FEF7DC',
      noteBorderColor: '#B45309',
      noteTextColor: '#172033',
      activationBkgColor: '#DBEAFE',
      activationBorderColor: '#1D4ED8',
      sequenceNumberColor: '#FFFFFF'
    },
    sequence: {
      useMaxWidth: false,
      diagramMarginX: 24,
      diagramMarginY: 16,
      actorMargin: 80,
      width: 176,
      height: 54,
      boxMargin: 10,
      boxTextMargin: 6,
      noteMargin: 14,
      messageMargin: 46,
      messageAlign: 'center',
      mirrorActors: true,
      wrap: true,
      wrapPadding: 12,
      actorFontSize: 14,
      noteFontSize: 13,
      messageFontSize: 14
    },
    flowchart: {
      useMaxWidth: false,
      htmlLabels: true,
      curve: 'basis',
      nodeSpacing: 42,
      rankSpacing: 52,
      padding: 14,
      diagramPadding: 14,
      wrappingWidth: 230
    },
    class: { useMaxWidth: false },
    state: { useMaxWidth: false }
  };
  root.CRYPTO_SITE_MERMAID_CONFIG = config;
  if (typeof module !== 'undefined' && module.exports) { module.exports = config; }
})(typeof window !== 'undefined' ? window : globalThis);
