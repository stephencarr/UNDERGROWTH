const {chromium}=require('playwright'),assert=require('node:assert/strict');
(async()=>{const browser=await chromium.launch({executablePath:'/usr/bin/chromium',headless:true,args:['--no-sandbox']});try{for(const touch of [false,true]){
 const page=await browser.newPage({viewport:{width:1000,height:850},hasTouch:touch,isMobile:touch});const errors=[];page.on('pageerror',e=>errors.push(String(e)));
 await page.goto(process.env.TEST_URL||'http://127.0.0.1:8000');const activate=loc=>touch?loc.tap():loc.click(),button=name=>page.locator('#fm1-controls').getByRole('button',{name,exact:true});
 await activate(page.locator('#device-audio'));await page.waitForFunction(()=>UG.state?.voices.length===4);
 await page.keyboard.press('4');await page.waitForFunction(()=>UG.state.panel.voice===3);
 await activate(button('ENV'));await page.waitForFunction(()=>document.getElementById('panel-readout').textContent.includes('Spread'));
 await activate(button('SEQ'));await activate(button('White key 5'));await page.waitForFunction(()=>UG.state.voices[3].steps[4].on);
 await activate(button('White key 5'));await page.waitForFunction(()=>!UG.state.voices[3].steps[4].on);await activate(button('White key 5'));
 await button('White key 5').click({delay:450});await page.waitForFunction(()=>UG.state.panel.locks);
 await button('KNOB 1').dispatchEvent('wheel',{deltaY:-100});await page.waitForFunction(()=>UG.state.voices[3].steps[4].lock[0]>.55);
 assert(!(await page.evaluate(()=>UG.state.voices.slice(0,3).some(v=>v.steps[4].on))));
 await activate(button('SEQ'));await activate(button('PLAY'));await page.waitForFunction(()=>UG.state.voices[3].count>0);await activate(button('PLAY'));
 await activate(page.locator('#view-toggle'));await activate(page.locator('#voice-backbeat'));const count=await page.evaluate(()=>UG.state.voices[3].count);
 await activate(page.getByRole('button',{name:'Trigger backbeat',exact:true}));await page.waitForFunction(n=>UG.state.voices[3].count>n,count);
 assert.equal(await page.locator('#selected-voice').textContent(),'Backbeat');
 for(const width of [320,375,680]){await page.setViewportSize({width,height:850});assert(await page.evaluate(()=>document.documentElement.scrollWidth<=innerWidth));}
 assert.deepEqual(errors,[]);await page.close();console.log('PASS '+(touch?'touch':'mouse')+' Backbeat: fourth lane, ENV, toggle/hold lock, independent sequence/playback, expanded live pad and narrow layout');
}}finally{await browser.close();}})().catch(e=>{console.error(e);process.exitCode=1;});
