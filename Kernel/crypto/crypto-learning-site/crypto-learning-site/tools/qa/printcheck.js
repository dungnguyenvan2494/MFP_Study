const { chromium, SITE: SITE_URL, OUT: QA_OUT } = require('./common');
(async () => {
  const b = await chromium.launch({ args: ['--no-sandbox'] });
  const p = await (await b.newContext({ viewport: { width: 1200, height: 900 } })).newPage();
  await p.goto(SITE_URL + '#ch-request');
  await p.waitForFunction(() => [...document.querySelectorAll('article.is-current .diagram-card')].every(c => c.getAttribute('data-rendered') === 'true'));
  const closedBefore = await p.$$eval('article.is-current details:not([open])', d => d.length);
  await p.evaluate(() => window.dispatchEvent(new Event('beforeprint')));
  const closedDuring = await p.$$eval('article.is-current details:not([open])', d => d.length);
  await p.emulateMedia({ media: 'print' });

  const sigOverflow = await p.$eval('article.is-current .card-struct pre', e => e.scrollWidth > e.clientWidth + 2);
  await p.locator('article.is-current .card-struct').screenshot({ path: QA_OUT + '/shots/print-struct.png' });
  await p.evaluate(() => window.dispatchEvent(new Event('afterprint')));
  const closedAfter = await p.$$eval('article.is-current details:not([open])', d => d.length);
  console.log(JSON.stringify({ closedBefore, closedDuring, closedAfter, sigOverflow }));
  await b.close();
})();
