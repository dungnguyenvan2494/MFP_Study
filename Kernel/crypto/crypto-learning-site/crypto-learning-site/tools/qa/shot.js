// usage: node shot.js <hash> <out.png> [width] [height] [theme] [fullPage]
// Loads the site via file://, waits for the chapter's diagrams, reports console errors,
// non-file requests and diagram render status; writes a screenshot.
const { chromium, SITE: SITE_URL, OUT: QA_OUT } = require('./common');
const SITE = SITE_URL;
(async () => {
  const [hash, out, w = '1440', h = '900', theme = 'light', full = '0'] = process.argv.slice(2);
  const browser = await chromium.launch({ args: ['--no-sandbox'] });
  const ctx = await browser.newContext({ viewport: { width: +w, height: +h }, colorScheme: theme === 'dark' ? 'dark' : 'light' });
  const page = await ctx.newPage();
  const errors = [], external = [];
  page.on('console', m => { if (m.type() === 'error' || m.type() === 'warning') errors.push(m.type() + ': ' + m.text()); });
  page.on('pageerror', e => errors.push('pageerror: ' + e.message));
  page.on('request', r => { if (!r.url().startsWith('file:') && !r.url().startsWith('data:')) external.push(r.url()); });
  await page.goto(SITE + (hash ? '#' + hash : ''));
  await page.waitForFunction(() => {
    const ch = document.querySelector('article.chapter.is-current');
    if (!ch) return false;
    return [...ch.querySelectorAll('.diagram-card')].every(c => ['true', 'error'].includes(c.getAttribute('data-rendered')));
  }, null, { timeout: 120000 });
  const info = await page.evaluate(() => {
    const ch = document.querySelector('article.chapter.is-current');
    const cards = [...ch.querySelectorAll('.diagram-card')];
    return {
      chapter: ch.id,
      diagrams: cards.map(c => {
        const svg = c.querySelector('.diagram-render svg');
        return c.id + ':' + c.getAttribute('data-rendered') + (svg ? ':' + svg.getAttribute('width') + 'x' + svg.getAttribute('height') : '');
      }),
      searchEntries: document.documentElement.getAttribute('data-search-entries'),
      docScrollW: document.documentElement.scrollWidth, vw: window.innerWidth,
    };
  });
  if (out) await page.screenshot({ path: out, fullPage: full === '1' });
  console.log(JSON.stringify({ ...info, errors, external }, null, 1));
  await browser.close();
})().catch(e => { console.error('FAILED', e); process.exit(1); });
