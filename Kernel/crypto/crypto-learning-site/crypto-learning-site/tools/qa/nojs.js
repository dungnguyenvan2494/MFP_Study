const { chromium, SITE: SITE_URL, OUT: QA_OUT } = require('./common');
(async () => {
  const b = await chromium.launch({ args: ['--no-sandbox'] });
  const p = await (await b.newContext({ viewport: { width: 1280, height: 900 }, javaScriptEnabled: false })).newPage();
  await p.goto(SITE_URL + '#ch-aead');
  const r = await p.evaluate(() => ({
    visibleChapters: [...document.querySelectorAll('article.chapter')].filter(a => a.offsetHeight > 0).length,
    noscript: !!document.querySelector('noscript'),
    placeholders: document.querySelectorAll('.diagram-placeholder').length,
  }));
  await p.screenshot({ path: QA_OUT + '/shots/nojs.png' });
  console.log(JSON.stringify(r));
  await b.close();
})();
