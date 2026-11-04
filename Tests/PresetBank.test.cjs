const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');
const http = require('node:http');
const { chromium } = require(process.env.PLAYWRIGHT_MODULE || 'playwright');
(async () => {
  const root = path.join(__dirname, '..');
  const data = JSON.parse(fs.readFileSync(path.join(root, 'Resources/presets.json')));
  assert.equal(data.presets.length, 100);
  assert.equal(new Set(data.presets.map(p => p.id)).size, 100);
  assert.equal(new Set(data.presets.map(p => p.name)).size, 100);
  for (const tag of ['Male Vocal', 'Female Vocal', 'Acoustic', 'Acoustic Guitar', 'Electric Guitar', 'Electric Bass', 'Piano', 'Strings', 'Brass & Winds', 'Synth', 'Percussion', 'Best Of', 'Dark', 'Light']) {
    assert(data.presets.some(p => p.tags.includes(tag)), `Missing category ${tag}`);
  }
  const server = http.createServer((q, r) => { r.setHeader('Content-Type', 'text/html'); r.end(fs.readFileSync(path.join(root, 'voxbox.html'))); });
  await new Promise(resolve => server.listen(0, '127.0.0.1', resolve));
  let browser;
  try { browser = await chromium.launch({ executablePath: process.env.CHROMIUM || undefined, args: ['--no-sandbox'] });
    const page = await browser.newPage();
    const errors = [];
    page.on('pageerror', e => errors.push(e.message));
    await page.goto('http://127.0.0.1:' + server.address().port);
    const embedded = await page.evaluate(() => voxbox.getPresets().filter(p => p.factory));
    assert.equal(embedded.length, 100);
    assert.deepEqual(embedded.map(p => [p.id, p.name, p.settings, p.tags]), data.presets.map(p => [p.id, p.name, p.settings, p.tags]));
    await page.locator('#preset').click();
    if (await page.locator('#clearTags').isVisible()) await page.locator('#clearTags').click();
    for (const p of data.presets) {
      assert.equal(Object.keys(p.settings).length, 25, p.name);
      for (const spec of data.parameters) {
        const value = p.settings[spec.id];
        assert(Number.isFinite(value) && value >= spec.min && value <= spec.max, `${p.name}: ${spec.id}`);
        if (spec.step) assert(Math.abs((value - spec.min) / spec.step - Math.round((value - spec.min) / spec.step)) < 1e-6);
      }
      await page.locator('#presetSearch').fill(p.name);
      await page.getByText(p.name, { exact: true }).click();
      const actual = await page.evaluate(() => voxbox.getState());
      for (const [id, expected] of Object.entries(p.settings)) assert(Math.abs(actual[id] - expected) < 0.001, `${p.name}: ${id} expected ${expected}, got ${actual[id]}`);
    }
    for (const tag of ['Male Vocal', 'Female Vocal', 'Acoustic Guitar', 'Best Of', 'Dark', 'Light']) {
      await page.locator('#presetSearch').fill('');
      if (await page.locator('#clearTags').isVisible()) await page.locator('#clearTags').click();
      await page.getByRole('button', {name: tag, exact: true}).click();
      assert.deepEqual(await page.locator('.preset-row').allTextContents(), data.presets.filter(p => p.tags.includes(tag)).map(p => p.name));
    }
    assert.deepEqual(errors, []);
    console.log('PASS: 100 presets × 25 controls selected through the UI; native/HTML snapshots match; instrument and style filters.');
  } finally { if (browser) await browser.close(); await new Promise(resolve => server.close(resolve)); }
})().catch(e => { console.error(e); process.exitCode = 1; });
