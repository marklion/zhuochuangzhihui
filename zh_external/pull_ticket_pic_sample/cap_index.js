var arg = process.argv

const puppeteer = require('puppeteer');
(async () => {
    const browser = await puppeteer.launch({
        defaultViewport: {
            width: 390, height: 844
        }, args: ['--no-sandbox', '--disable-setuid-sandbox']
    });
    const page = await browser.newPage();
    await page.goto('https://www.d8sis.cn/pa_web/ticket/' + arg[2]);
    const image = await page.waitForSelector('#ticket_picture');
    await new Promise(r => setTimeout(r, 3000));

    await image.screenshot({
        path: '/home/pptruser/example.png',
    });
    await browser.close();
})();