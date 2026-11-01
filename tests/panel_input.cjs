// Requires Playwright and Chromium. Run with TEST_URL and CHROMIUM_PATH as needed.
const {chromium}=require('playwright');
const assert=require('node:assert/strict');
(async()=>{
 const url=process.env.TEST_URL||'http://127.0.0.1:8000';
 const browser=await chromium.launch({executablePath:process.env.CHROMIUM_PATH||'/usr/bin/chromium',headless:true,
  ...(url.startsWith('https:')&&process.env.HTTPS_PROXY?{proxy:{server:process.env.HTTPS_PROXY}}:{}),args:['--no-sandbox']});
 try{
  for(const touch of [false,true]){
   const page=await browser.newPage({viewport:{width:1000,height:800},hasTouch:touch,isMobile:touch});
   const errors=[];page.on('pageerror',e=>errors.push(String(e)));
   await page.goto(url);
   const activate=loc=>touch?loc.tap():loc.click();
   await activate(page.locator('#device-audio'));await page.waitForFunction(()=>window.UG?.state);
   const button=name=>page.locator('#fm1-controls').getByRole('button',{name,exact:true});
   for(const expected of [1,0]){await activate(button('PLAY'));await page.waitForFunction(x=>UG.state.playing===x,expected);await page.waitForTimeout(150);assert.equal(await page.evaluate(()=>UG.state.playing),expected);}
   await activate(button('SEQ'));await page.waitForFunction(()=>document.getElementById('panel-readout').textContent.includes('SEQUENCE'));
   for(const expected of [1,0]){await activate(button('REC'));await page.waitForFunction(x=>UG.state.recording===x,expected);await page.waitForTimeout(150);assert.equal(await page.evaluate(()=>UG.state.recording),expected);}
   await button('PLAY').focus();await page.keyboard.press('Enter');await page.waitForFunction(()=>UG.state.playing===1);
   await page.keyboard.press('Space');await page.waitForFunction(()=>UG.state.playing===0);
   assert.deepEqual(errors,[]);await page.close();console.log(`PASS ${touch?'touch':'mouse'} panel buttons and keyboard: transport, SEQ, REC, no duplicate activation`);
  }
 }finally{await browser.close();}
})().catch(e=>{console.error(e);process.exitCode=1;});
