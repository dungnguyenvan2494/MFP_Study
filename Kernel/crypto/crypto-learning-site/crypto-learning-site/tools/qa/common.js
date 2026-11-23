// Shared by the QA scripts: Playwright (local install first, then the global one used when
// this site was built) and the file:// URL of the site next to this folder.
const path = require('path');
const fs = require('fs');
let pw;
try { pw = require('playwright'); } catch (e) { pw = require('/opt/node22/lib/node_modules/playwright'); }
const OUT = process.env.QA_OUT || path.resolve(__dirname, '..', '.work', 'qa');
fs.mkdirSync(path.join(OUT, 'shots'), { recursive: true });
module.exports = {
  chromium: pw.chromium,
  SITE: 'file://' + path.resolve(__dirname, '..', '..', 'index.html'),
  OUT,
};
