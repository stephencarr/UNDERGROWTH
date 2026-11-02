const {chromium}=require('playwright'),assert=require('node:assert/strict');
(async()=>{const browser=await chromium.launch({executablePath:'/usr/bin/chromium',headless:true,args:['--no-sandbox']});try{for(const touch of [false,true]){
 const page=await browser.newPage({viewport:{width:1100,height:1000},hasTouch:touch,isMobile:touch});const errors=[];page.on('pageerror',e=>errors.push(String(e)));
 await page.goto(process.env.TEST_URL||'http://127.0.0.1:8000');
 const activate=async loc=>{await(touch?loc.tap():loc.click());await page.waitForTimeout(70);},button=name=>page.locator('#fm1-controls').getByRole('button',{name,exact:true});
 const press=name=>activate(button(name));const state=()=>page.evaluate(()=>UG.state);
 const turn=async(name,key='ArrowUp',n=1)=>{await button(name).focus();for(let i=0;i<n;i++)await page.keyboard.press(key);await page.waitForTimeout(80);};
 const hold=async loc=>{if(!touch){await loc.click({delay:450});return;}const b=await loc.boundingBox(),cdp=await page.context().newCDPSession(page);await cdp.send('Input.dispatchTouchEvent',{type:'touchStart',touchPoints:[{x:b.x+b.width/2,y:b.y+b.height/2}]});await page.waitForTimeout(450);await cdp.send('Input.dispatchTouchEvent',{type:'touchEnd',touchPoints:[]});await cdp.detach();};
 await activate(page.locator('#device-audio'));await page.waitForFunction(()=>UG.state?.panel.keyMode===2);
 assert.equal(await button('KNOB 3').getAttribute('aria-description'),'ROOT');await turn('KNOB 1','ArrowDown',25);assert(Math.abs((await state()).voices[0].level-.75)<.001);
 await press('White key 1');assert((await state()).voices[0].muted);assert(!(await state()).voices[0].steps.some(s=>s.on));await press('White key 1');
 await press('SEQ');await press('White key 1');await hold(button('White key 1'));await page.waitForFunction(()=>UG.state.panel.locks);await turn('KNOB 2','ArrowUp',12);await press('SEQ');
 const source=(await state()).voices[0].steps[0];await press('Black key 5');await hold(button('White key 5'));await page.waitForFunction(()=>UG.state.panel.selected===4&&UG.state.panel.locks);await press('Black key 6');assert.deepEqual((await state()).voices[0].steps[4],source);
 await press('Black key 7');assert(!(await state()).voices[0].steps[4].on);await press('Black key 7');assert((await state()).voices[0].steps[4].on);
 const hits=(await state()).voices[0].count;await press('Black key 9');assert((await state()).voices[0].count>hits);assert.equal((await state()).playing,0);
 await press('Black key 3');assert.equal((await state()).panel.voice,2);await press('Black key 6');assert(!(await state()).voices[2].steps.some(s=>s.on));assert((await page.locator('#panel-context').textContent()).includes('source track'));
 await press('Black key 1');assert.equal((await state()).panel.selected,4);
 await press('REC');await turn('KNOB 1');assert((await state()).armed&&(await state()).recordMode===2);await press('ENV');await press('REC');assert(!(await state()).armed);assert.equal((await state()).panel.page,2);
 await activate(page.locator('#view-toggle'));await page.locator('#mute-2').waitFor();await activate(page.locator('#mute-2'));assert((await state()).voices[2].muted);await activate(page.locator('#mute-2'));
 await page.locator('#record-mode').selectOption('3');await activate(page.locator('#record'));assert((await state()).armed&&(await state()).recordMode===3);await activate(page.locator('#voice-root'));assert.equal((await state()).recordTrack,0);await activate(page.locator('#record'));assert(!(await state()).armed);
 for(const width of [320,375,680]){await page.setViewportSize({width,height:900});assert(await page.evaluate(()=>document.documentElement.scrollWidth<=innerWidth));}
 await page.setViewportSize({width:1100,height:1000});await activate(page.locator('#view-toggle'));await page.screenshot({path:`/tmp/undergrowth-hardware-ux-${touch?'touch':'mouse'}.png`,fullPage:true});
 assert.deepEqual(errors,[]);await page.close();console.log('PASS '+(touch?'touch':'mouse')+' hardware UX: mixer and labels/lights, full-trig copy/paste/Undo, heard rest, wrong-track refusal, track memory, take mode/owner/finish, shared Expanded mix/recording and narrow layout');
}}finally{await browser.close();}})().catch(e=>{console.error(e);process.exitCode=1;});
