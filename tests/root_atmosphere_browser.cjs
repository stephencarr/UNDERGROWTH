const {chromium}=require('playwright'),assert=require('node:assert/strict');
(async()=>{const browser=await chromium.launch({executablePath:'/usr/bin/chromium',headless:true,args:['--no-sandbox']});try{for(const touch of [false,true]){
const page=await browser.newPage({hasTouch:touch,viewport:{width:1100,height:850}}),errors=[];page.on('pageerror',e=>errors.push(String(e)));
await page.goto(process.env.TEST_URL||'http://127.0.0.1:8000');const click=async el=>{await(touch?el.tap():el.click());await page.waitForTimeout(100);};const btn=n=>page.locator('#fm1-controls').getByRole('button',{name:n,exact:true});
await click(page.locator('#device-audio'));await page.waitForFunction(()=>window.UG?.state);await btn('ALGORITHM').focus();await page.keyboard.press('ArrowUp');await page.keyboard.press('ArrowUp');await page.waitForFunction(()=>UG.state.panel.voice===2);
await click(btn('SEQ'));await click(btn('White key 1'));await btn('KNOB 1').focus();for(let i=0;i<7;i++)await page.keyboard.press('ArrowUp');await page.waitForFunction(()=>UG.state.voices[2].steps[0].note===43);
await click(btn('SEQ'));await page.waitForFunction(()=>UG.state.panel.keyMode===0);await click(btn('White key 5'));await page.waitForFunction(()=>UG.state.voices[2].steps[0].note===36);
await click(btn('ENV'));await page.evaluate(()=>{UG.command('param',2,17,1);UG.command('param',2,13,1);UG.command('play',1);});await page.waitForFunction(()=>UG.state.panel.grains[1]>0);assert(await page.locator('canvas[aria-label^="ROOT atmosphere"]').isVisible());
await btn('SELECT').focus();await page.keyboard.press('ArrowUp');await page.keyboard.press('ArrowUp');
await page.waitForFunction(()=>UG.state.panel.knobLabels.includes('Surprise'));
await btn('KNOB 1').focus();for(let i=0;i<50;i++)await page.keyboard.press('ArrowUp');
await page.waitForFunction(()=>UG.state.voices[2].base[19]>.49);
await click(btn('FX'));await page.waitForFunction(()=>UG.state.panel.knobLabels.includes('ENV send'));await btn('KNOB 2').focus();for(let i=0;i<40;i++)await page.keyboard.press('ArrowUp');await page.waitForFunction(()=>UG.state.voices[2].base[16]>.39);assert.equal(await page.evaluate(()=>UG.state.voices[2].base[3]),0);
await page.evaluate(()=>UG.command('panic'));await page.waitForFunction(()=>UG.state.panel.grains[1]===0);assert.deepEqual(errors,[]);await page.close();console.log('PASS ROOT '+(touch?'touch':'mouse')+': direct step pitch, keyboard assignment, active grain display, independent sends and Panic');
}}finally{await browser.close();}})().catch(e=>{console.error(e);process.exitCode=1;});
