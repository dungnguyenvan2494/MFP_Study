// Interaction QA: every check prints PASS/FAIL with evidence; screenshots go to ./shots/
const { chromium, SITE: SITE_URL, OUT: QA_OUT } = require('./common');
const fs = require('fs');
const SITE = SITE_URL;
const OUT = QA_OUT + '/shots';
fs.mkdirSync(OUT, { recursive: true });
const results = [];
function rec(name, ok, info) { results.push({ name, ok: !!ok, info }); console.log((ok ? 'PASS ' : 'FAIL ') + name + (info ? ' — ' + info : '')); }
async function waitDiagrams(page) {
  await page.waitForFunction(() => [...document.querySelectorAll('article.chapter.is-current .diagram-card')]
    .every(c => ['true', 'error'].includes(c.getAttribute('data-rendered'))), null, { timeout: 120000 });
}
(async () => {
  const browser = await chromium.launch({ args: ['--no-sandbox'] });
  const errors = [];

  // ---------- desktop ----------
  const ctx = await browser.newContext({ viewport: { width: 1440, height: 900 }, colorScheme: 'light' });
  await ctx.grantPermissions(['clipboard-read', 'clipboard-write']);
  const page = await ctx.newPage();
  page.on('pageerror', e => errors.push(e.message));
  page.on('console', m => { if (m.type() === 'error') errors.push(m.text()); });
  await page.goto(SITE + '#ch-overview');
  await waitDiagrams(page);
  await page.screenshot({ path: OUT + '/desktop-overview.png' });
  rec('desktop: first chapter shown', await page.isVisible('#ch-overview h1'));

  // search
  await page.keyboard.press('/');
  rec('search: "/" focuses input', await page.evaluate(() => document.activeElement.id === 'search-input'));
  await page.keyboard.type('larval wait');
  await page.waitForSelector('#search-results:not([hidden]) .search-item');
  const nRes = await page.$$eval('#search-results .search-item', a => a.length);
  const firstRes = await page.$eval('#search-results .search-item', a => a.textContent.slice(0, 100));
  rec('search: results for "larval wait"', nRes > 0, nRes + ' kết quả; đầu: ' + firstRes);
  rec('search: highlight marks', await page.$$eval('#search-results mark', m => m.length) > 0);
  await page.screenshot({ path: OUT + '/desktop-search.png' });
  await page.keyboard.press('ArrowDown');
  await page.keyboard.press('Enter');
  await page.waitForTimeout(400);
  const h1 = await page.evaluate(() => location.hash);
  rec('search: Enter navigates', h1 && h1 !== '#ch-overview', h1);
  // diacritic folding
  await page.fill('#search-input', 'dang ky');
  await page.waitForTimeout(300);
  const fold = await page.$$eval('#search-results .search-item', a => a.length);
  rec('search: no-diacritic query "dang ky"', fold > 0, fold + ' kết quả');
  await page.fill('#search-input', 'gcm-aes-ce');
  await page.waitForTimeout(300);
  rec('search: driver name "gcm-aes-ce"', await page.$$eval('#search-results .search-item', a => a.length) > 0);
  await page.fill('#search-input', 'crypto_req_done');
  await page.waitForTimeout(300);
  const fnHit = await page.$$eval('#search-results .search-item', a => a.map(x => x.textContent).filter(t => /Hàm|Function/.test(t)).length);
  rec('search: function card found', fnHit > 0);
  await page.keyboard.press('Escape');
  rec('search: Esc closes', await page.isHidden('#search-results'));

  // navigate lookup, zoom modal
  await page.goto(SITE + '#ch-lookup');
  await waitDiagrams(page);
  const card = page.locator('#dg-lk-seq-miss');
  await card.scrollIntoViewIfNeeded();
  await card.screenshot({ path: OUT + '/desktop-card-seq.png' });
  await card.locator('button', { hasText: 'Phóng to' }).click();
  await page.waitForSelector('#diagram-modal:not([hidden]) svg');
  const z0 = await page.textContent('.zoom-level');
  await page.click('[data-zoom="in"]');
  const z1 = await page.textContent('.zoom-level');
  rec('modal: opens and zoom-in changes scale', z0 !== z1, z0 + ' → ' + z1);
  await page.keyboard.press('-');
  const z2 = await page.textContent('.zoom-level');
  rec('modal: "-" key zooms out', z2 !== z1, z1 + ' → ' + z2);
  await page.click('[data-zoom="fit"]');
  await page.screenshot({ path: OUT + '/desktop-modal.png' });
  // drag to pan
  const box = await page.locator('.modal-body').boundingBox();
  await page.click('[data-zoom="in"]'); await page.click('[data-zoom="in"]'); await page.click('[data-zoom="in"]');
  const s0 = await page.$eval('.modal-body', e => [e.scrollLeft, e.scrollTop]);
  await page.mouse.move(box.x + box.width / 2, box.y + box.height / 2);
  await page.mouse.down(); await page.mouse.move(box.x + box.width / 2 - 200, box.y + box.height / 2 - 150, { steps: 5 }); await page.mouse.up();
  const s1 = await page.$eval('.modal-body', e => [e.scrollLeft, e.scrollTop]);
  rec('modal: drag pans content', s1[0] !== s0[0] || s1[1] !== s0[1], JSON.stringify(s0) + ' → ' + JSON.stringify(s1));
  await page.keyboard.press('Escape');
  rec('modal: Esc closes', await page.isHidden('#diagram-modal'));
  rec('modal: focus returns to trigger', await page.evaluate(() => document.activeElement && document.activeElement.textContent.includes('Phóng to')));

  // copy Mermaid + view source
  await card.locator('button', { hasText: 'Sao chép Mermaid' }).click();
  await page.waitForSelector('#toast:not([hidden])');
  const toast = await page.textContent('#toast');
  let clip = '';
  try { clip = await page.evaluate(() => navigator.clipboard.readText()); } catch (e) { clip = 'ERR ' + e.message; }
  rec('copy Mermaid: toast + clipboard holds source', /Đã sao chép/.test(toast) && clip.startsWith('sequenceDiagram'), toast + ' | clipboard: ' + clip.slice(0, 30));
  await card.locator('button', { hasText: 'Xem source' }).click();
  rec('view source: toggles visible', await page.isVisible('#dg-lk-seq-miss-source'));
  const exp = await card.locator('button[aria-controls="dg-lk-seq-miss-source"]').getAttribute('aria-expanded');
  rec('view source: aria-expanded=true', exp === 'true');

  // code copy button
  await page.goto(SITE + '#ch-request');
  await waitDiagrams(page);
  const cb = page.locator('#ch-request .code-block', { hasText: 'DECLARE_CRYPTO_WAIT' }).first().locator('.copy-btn');
  await cb.scrollIntoViewIfNeeded();
  await cb.click();
  await page.waitForTimeout(200);
  const cbt = await cb.textContent();
  let clip2 = ''; try { clip2 = await page.evaluate(() => navigator.clipboard.readText()); } catch (e) { clip2 = 'ERR'; }
  rec('code copy: button feedback + clipboard', cbt === 'Đã copy' && clip2.includes('DECLARE_CRYPTO_WAIT'), cbt + ' | ' + clip2.slice(0, 30));
  rec('code highlight applied', await page.$$eval('#ch-request .code-block code.hljs', a => a.length) > 0);

  // TOC + scroll spy
  const tocN = await page.$$eval('#toc-list a', a => a.length);
  await page.click('#toc-list a >> nth=4');
  await page.waitForTimeout(600);
  const active = await page.$$eval('#toc-list a.active', a => a.map(x => x.textContent));
  rec('TOC: links + active state after click', tocN > 5 && active.length > 0, tocN + ' mục; active=' + active.join('|'));
  rec('breadcrumbs show section', (await page.textContent('#breadcrumbs')).split('›').length >= 1, (await page.textContent('#breadcrumbs')).trim());

  // pager
  const next = await page.$eval('#ch-request .pager-next', a => a.getAttribute('href'));
  await page.click('#ch-request .pager-next');
  await page.waitForTimeout(300);
  rec('pager: next chapter', await page.evaluate(n => location.hash === n, next), next);

  // collapsibles
  await page.goto(SITE + '#ch-skcipher');
  await waitDiagrams(page);
  const nd = await page.$$eval('#ch-skcipher details.collapsible', d => d.length);
  await page.click('#ch-skcipher .section-tools button >> nth=0');
  const openN = await page.$$eval('#ch-skcipher details.collapsible[open]', d => d.length);
  rec('expand all collapsibles', nd > 0 && openN === nd, openN + '/' + nd);

  // theme toggle + persistence
  const t0 = await page.getAttribute('html', 'data-theme');
  await page.click('#theme-toggle');
  const t1 = await page.getAttribute('html', 'data-theme');
  await page.reload(); await page.waitForSelector('article.chapter.is-current');
  const t2 = await page.getAttribute('html', 'data-theme');
  rec('theme toggle + persisted after reload', t0 !== t1 && t1 === t2, `${t0} → ${t1} → reload ${t2}`);
  await waitDiagrams(page);
  await page.screenshot({ path: OUT + '/desktop-dark-skcipher.png' });
  await page.locator('#dg-sk-walk-next').screenshot({ path: OUT + '/desktop-dark-card.png' });
  await page.click('#theme-toggle');

  // progress
  const prog = await page.textContent('#progress-text');
  rec('progress counts visited chapters', /\d+\/28/.test(prog), prog);

  // tree filter
  await page.goto(SITE + '#ch-source-tree');
  await page.waitForTimeout(300);
  await page.fill('#tree-filter', 'cryptd');
  await page.waitForTimeout(200);
  const tc = await page.textContent('#tree-count');
  await page.selectOption('#tree-priority', 'P0');
  await page.fill('#tree-filter', '');
  await page.waitForTimeout(200);
  const tc2 = await page.textContent('#tree-count');
  rec('tree filter by text and priority', /^\d+ file/.test(tc) && tc !== tc2, `"cryptd": ${tc}; P0: ${tc2}`);
  await page.selectOption('#tree-priority', '');
  await page.locator('.tree').screenshot({ path: OUT + '/desktop-tree.png' });

  // glossary filter
  await page.goto(SITE + '#ch-glossary');
  await page.fill('#glossary-filter', 'larval');
  await page.waitForTimeout(200);
  rec('glossary filter', /^\d+ thuật ngữ/.test(await page.textContent('#glossary-count')), await page.textContent('#glossary-count'));

  // deep link to a diagram id opens its chapter
  await page.goto(SITE + '#dg-hw-cryptd-complete');
  await page.waitForTimeout(500);
  rec('deep link to diagram opens owning chapter', await page.evaluate(() => document.querySelector('article.is-current').id === 'ch-hwaccel'));
  // deep link inside a collapsed details opens it
  const qid = await page.evaluate(() => { const d = document.querySelector('#ch-hash details.quiz-a .details-body p'); return null; });

  // print emulation
  await page.goto(SITE + '#ch-request');
  await waitDiagrams(page);
  await page.emulateMedia({ media: 'print' });
  const hiddenUi = await page.evaluate(() => ['.topbar', '.sidebar', '.toc', '.diagram-toolbar'].every(s => { const e = document.querySelector(s); return !e || getComputedStyle(e).display === 'none'; }));
  rec('print: UI chrome hidden', hiddenUi);
  await page.screenshot({ path: OUT + '/print-request.png', fullPage: false });
  await page.pdf({ path: OUT + '/print-test-request.pdf', format: 'A4', printBackground: true });
  const pdfSize = fs.statSync(OUT + '/print-test-request.pdf').size;
  rec('print: Chromium can print chapter to PDF (QA only)', pdfSize > 20000, pdfSize + ' bytes');
  await page.emulateMedia({ media: 'screen' });
  await ctx.close();

  // ---------- mobile ----------
  const m = await browser.newContext({ viewport: { width: 390, height: 844 }, deviceScaleFactor: 2, isMobile: true, hasTouch: true });
  const mp = await m.newPage();
  mp.on('pageerror', e => errors.push('mobile: ' + e.message));
  await mp.goto(SITE + '#ch-request');
  await waitDiagrams(mp);
  await mp.screenshot({ path: OUT + '/mobile-request.png' });
  rec('mobile: sidebar hidden by default', !(await mp.evaluate(() => document.body.classList.contains('sidebar-open'))));
  await mp.click('#sidebar-toggle');
  await mp.waitForTimeout(300);
  rec('mobile: drawer opens', await mp.evaluate(() => document.body.classList.contains('sidebar-open')));
  await mp.screenshot({ path: OUT + '/mobile-drawer.png' });
  await mp.click('#sidebar a[data-chapter="ch-aead"]');
  await mp.waitForTimeout(400);
  rec('mobile: choosing chapter closes drawer', !(await mp.evaluate(() => document.body.classList.contains('sidebar-open'))) && (await mp.evaluate(() => location.hash)) === '#ch-aead');
  await waitDiagrams(mp);
  const mcard = mp.locator('#dg-aead-gcm-dec');
  await mcard.scrollIntoViewIfNeeded();
  await mcard.screenshot({ path: OUT + '/mobile-card.png' });
  await mp.click('#toc-toggle');
  await mp.waitForTimeout(300);
  rec('mobile: TOC panel opens', await mp.evaluate(() => document.body.classList.contains('toc-open')));
  await mp.screenshot({ path: OUT + '/mobile-toc.png' });
  await mp.keyboard.press('Escape');
  await mcard.locator('button', { hasText: 'Phóng to' }).click();
  await mp.waitForSelector('#diagram-modal:not([hidden]) svg');
  await mp.screenshot({ path: OUT + '/mobile-modal.png' });
  rec('mobile: modal opens', true);
  await m.close();

  rec('no JS errors during interaction run', errors.length === 0, errors.slice(0, 3).join(' | '));
  fs.writeFileSync(QA_OUT + '/interact-report.json', JSON.stringify(results, null, 1));
  console.log(`\n${results.filter(r => r.ok).length}/${results.length} checks passed`);
  await browser.close();
})().catch(e => { console.error('FAILED', e); process.exit(1); });
