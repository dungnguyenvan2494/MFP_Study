// Full-site QA: open every chapter via its hash, wait for all diagrams, and report
// console errors, failed diagrams, external requests, horizontal overflow,
// duplicate ids, dangling #links, empty TOC, search index size.
// usage: node full.js [width] [height] [theme]   (writes qa-report-<w>.json next to this file)
const { chromium, SITE: SITE_URL, OUT: QA_OUT } = require('./common');
const path = require('path');
const fs = require('fs');
const SITE = SITE_URL;
(async () => {
  const [w = '1440', h = '900', theme = 'light'] = process.argv.slice(2);
  const browser = await chromium.launch({ args: ['--no-sandbox'] });
  const ctx = await browser.newContext({ viewport: { width: +w, height: +h }, colorScheme: theme });
  const page = await ctx.newPage();
  const consoleMsgs = [], external = [];
  page.on('console', m => { if (['error', 'warning'].includes(m.type())) consoleMsgs.push(m.type() + ': ' + m.text()); });
  page.on('pageerror', e => consoleMsgs.push('pageerror: ' + e.message));
  page.on('request', r => { const u = r.url(); if (!u.startsWith('file:') && !u.startsWith('data:') && !u.startsWith('blob:')) external.push(u); });
  const t0 = Date.now();
  await page.goto(SITE);
  await page.waitForSelector('article.chapter.is-current');
  const loadMs = Date.now() - t0;
  const statics = await page.evaluate(() => {
    const ids = {}; const dups = [];
    document.querySelectorAll('[id]').forEach(e => { if (ids[e.id]) dups.push(e.id); ids[e.id] = 1; });
    const dangling = [];
    document.querySelectorAll('a[href^="#"]').forEach(a => {
      const id = decodeURIComponent(a.getAttribute('href').slice(1));
      if (id && !document.getElementById(id)) dangling.push(id);
    });
    const ext = [...document.querySelectorAll('a[href^="http"]')].map(a => a.href);
    return {
      chapters: [...document.querySelectorAll('article.chapter')].map(c => c.id),
      dups: [...new Set(dups)], dangling: [...new Set(dangling)], externalLinks: ext,
      searchEntries: document.documentElement.getAttribute('data-search-entries'),
      scripts: [...document.scripts].map(s => s.src).filter(Boolean),
    };
  });
  const perChapter = [];
  for (const id of statics.chapters) {
    await page.evaluate(i => { location.hash = i; }, id);
    await page.waitForFunction(i => {
      const ch = document.getElementById(i);
      if (!ch || !ch.classList.contains('is-current')) return false;
      return [...ch.querySelectorAll('.diagram-card')].every(c => ['true', 'error'].includes(c.getAttribute('data-rendered')));
    }, id, { timeout: 180000 });
    const info = await page.evaluate(i => {
      const ch = document.getElementById(i);
      const cards = [...ch.querySelectorAll('.diagram-card')];
      return {
        id: i,
        diagrams: cards.length,
        failed: cards.filter(c => c.getAttribute('data-rendered') !== 'true').map(c => c.id),
        fit: cards.reduce((a, c) => { const f = c.getAttribute('data-fit') || '?'; a[f] = (a[f] || 0) + 1; return a; }, {}),
        toc: document.querySelectorAll('#toc-list li').length,
        overflowX: document.documentElement.scrollWidth > window.innerWidth + 1,
        wideEls: [...ch.querySelectorAll('*')].filter(e => {
          const r = e.getBoundingClientRect();
          return r.right > window.innerWidth + 1 && !e.closest('.diagram-canvas, .table-wrap, pre, .code-block');
        }).slice(0, 3).map(e => e.tagName + '.' + e.className),
        breadcrumbs: document.querySelector('#breadcrumbs').textContent.trim().slice(0, 80),
        pager: ch.querySelectorAll('.pager a').length,
      };
    }, id);
    perChapter.push(info);
  }
  const report = { viewport: `${w}x${h}`, theme, loadMs, ...statics, external: [...new Set(external)], console: consoleMsgs, perChapter };
  fs.writeFileSync(path.join(QA_OUT, `qa-report-${w}.json`), JSON.stringify(report, null, 1));
  const totalD = perChapter.reduce((a, c) => a + c.diagrams, 0);
  const failed = perChapter.flatMap(c => c.failed);
  console.log(`viewport ${w}x${h} ${theme}: chapters=${perChapter.length} diagrams=${totalD} failed=${failed.length} ${failed.join(',')}`);
  console.log(`console=${consoleMsgs.length} external=${report.external.length} dupIds=${statics.dups.length} dangling=${statics.dangling.length} search=${statics.searchEntries} load=${loadMs}ms`);
  console.log('overflow chapters:', perChapter.filter(c => c.overflowX || c.wideEls.length).map(c => c.id + ' ' + c.wideEls.join('|')).join('; ') || 'none');
  console.log('empty toc:', perChapter.filter(c => !c.toc).map(c => c.id).join(',') || 'none');
  if (consoleMsgs.length) console.log(consoleMsgs.slice(0, 10).join('\n'));
  if (statics.dups.length) console.log('dups', statics.dups.slice(0, 20));
  if (statics.dangling.length) console.log('dangling', statics.dangling.slice(0, 20));
  await browser.close();
})().catch(e => { console.error('FAILED', e); process.exit(1); });
