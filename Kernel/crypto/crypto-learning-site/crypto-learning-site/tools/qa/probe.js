const { chromium, SITE: SITE_URL, OUT: QA_OUT } = require('./common');
(async () => {
  const [hash, w='1440', expr] = process.argv.slice(2);
  const b = await chromium.launch({ args: ['--no-sandbox'] });
  const p = await (await b.newContext({ viewport: { width: +w, height: 900 } })).newPage();
  await p.goto(SITE_URL + '#' + hash);
  await p.waitForTimeout(1500);
  console.log(JSON.stringify(await p.evaluate(expr), null, 1));
  await b.close();
})();
