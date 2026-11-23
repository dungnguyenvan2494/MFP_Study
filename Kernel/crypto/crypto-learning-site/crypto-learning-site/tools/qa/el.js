// usage: node el.js <hash> <selector> <out.png> [width] [theme]
const { chromium, SITE: SITE_URL, OUT: QA_OUT } = require('./common');
const SITE = SITE_URL;
(async () => {
  const [hash, sel, out, w = '1440', theme = 'light'] = process.argv.slice(2);
  const browser = await chromium.launch({ args: ['--no-sandbox'] });
  const page = await (await browser.newContext({ viewport: { width: +w, height: 900 }, colorScheme: theme })).newPage();
  await page.goto(SITE + '#' + hash);
  await page.waitForFunction(() => [...document.querySelectorAll('article.chapter.is-current .diagram-card')].every(c => ['true','error'].includes(c.getAttribute('data-rendered'))), null, { timeout: 120000 });
  const el = page.locator(sel).first();
  await el.scrollIntoViewIfNeeded();
  await el.screenshot({ path: out });
  await browser.close();
})().catch(e => { console.error('FAILED', e.message); process.exit(1); });
