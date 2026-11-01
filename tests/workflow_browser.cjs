const {chromium}=require('playwright');
const assert=require('node:assert/strict');
(async()=>{
 const browser=await chromium.launch({executablePath:'/usr/bin/chromium',headless:true,args:['--no-sandbox']});
 try{for(const touch of [false,true]){
 const page=await browser.newPage({viewport:{width:1000,height:850},hasTouch:touch,isMobile:touch});
 const errors=[];page.on('pageerror',e=>errors.push(String(e)));
 await page.goto(process.env.TEST_URL||'http://127.0.0.1:8000');
 const activate=loc=>touch?loc.tap():loc.click(),button=name=>page.locator('#fm1-controls').getByRole('button',{name:name.replace(/^KNOB(\d)$/,'KNOB $1'),exact:true});
 const press=async name=>{await activate(button(name));await page.waitForTimeout(60);};
 const turn=async(name,key='ArrowUp',n=1)=>{await button(name).focus();for(let i=0;i<n;i++)await page.keyboard.press(key);await page.waitForTimeout(80);};
 const state=()=>page.evaluate(()=>UG.state),step=n=>press('White key '+n);
 await activate(page.locator('#device-audio'));await page.waitForFunction(()=>UG.state);
 await press('EDIT');await turn('KNOB1','ArrowUp',15);const draft=(await state()).voices[0].base[0];
 await press('SEQ');for(const n of [1,5,9,13])await step(n);
 await page.waitForFunction(()=>UG.state.voices[0].steps[12].on);
 let s=await state();for(const i of [0,4,8,12]){assert.equal(s.voices[0].steps[i].mask,127);assert.equal(s.voices[0].steps[i].lock[0],draft);}
 await turn('KNOB1','ArrowUp',20);await step(7);await page.waitForTimeout(80);s=await state();assert(s.voices[0].steps[6].lock[0]>draft+.19);assert.equal(s.voices[0].steps[0].lock[0],draft);
 await step(1);assert((await state()).voices[0].steps[0].on);
 // Hold a selected key, turn a sound knob, then release: only that step changes.
 await button('White key 1').focus();await page.keyboard.down('Space');await button('KNOB2').dispatchEvent('wheel',{deltaY:-100});await page.keyboard.up('Space');await button('White key 1').dispatchEvent('keyup',{code:'Space'});await page.waitForTimeout(80);
 s=await state();assert(s.voices[0].steps[0].lock[1]>s.voices[0].steps[4].lock[1]);
 await turn('SELECT');await turn('KNOB1');await press('SEL');await page.waitForFunction(()=>!UG.state.voices[0].steps[0].on);
 await turn('KNOB1');await press('SEL');await page.waitForFunction(()=>UG.state.voices[0].steps[0].on);
 // Playing a different lock must not contaminate the draft stamped on a new step.
 await press('PLAY');await press('SEQ');await step(3);await page.waitForTimeout(80);s=await state();assert.equal(s.voices[0].steps[2].lock[0],s.voices[0].base[0]);await press('PLAY');
 // Motion take, recorded through EDIT and FX, whole-take undo from HOME recovery.
 const before=(await state()).voices[0].steps;
 await press('REC');await turn('KNOB1');assert.equal((await state()).armed,false);await press('REC');await press('EDIT');await turn('KNOB1','ArrowDown',20);await press('FX');await turn('KNOB1','ArrowUp',25);await press('PLAY');
 await page.waitForFunction(()=>Math.abs(UG.state.voices[0].steps[0].lock[3]-.25)<.001);
 await press('REC');assert.equal((await state()).motion,false);await press('PLAY');
 await press('HOME');await turn('SELECT');
 await press('SEL');await page.waitForTimeout(80);assert.deepEqual((await state()).voices[0].steps,before);
 // Both records new hits and touched locks; other track remains unrecorded.
 await press('REC');await turn('KNOB1');await press('REC');await press('EDIT');await turn('KNOB2','ArrowUp',10);await press('PLAY');await step(2);await page.waitForTimeout(80);
 assert((await state()).voices[0].steps.some(x=>x.on&&(x.mask&2)));
 await turn('ALGORITHM');assert.equal((await state()).recordTrack,0);await step(1);await page.waitForTimeout(80);assert(!(await state()).voices[1].steps.some(x=>x.on));await press('REC');await press('PLAY');
 // ROOT stamps draft pitch/gate/slide with the same lower keys.
 await turn('ALGORITHM');await press('ENV');await turn('SELECT');await turn('KNOB1','ArrowUp',7);await turn('KNOB2','ArrowDown',5);await turn('KNOB3');await press('SEQ');await step(4);await page.waitForTimeout(80);
 s=await state();assert.equal(s.voices[2].steps[3].note,43);assert(Math.abs(s.voices[2].steps[3].gate-.5)<.001);assert(s.voices[2].steps[3].slide);
 for(const name of ['HOME','FX','SEL','ENV','LFO','EDIT','GLO','SAVE','ARP','SEQ']){await press(name);await press('PLAY');await page.waitForFunction(()=>UG.state.playing);await press('PLAY');await page.waitForFunction(()=>!UG.state.playing);}
 for(const width of [320,375,680]){await page.setViewportSize({width,height:850});assert(await page.evaluate(()=>document.documentElement.scrollWidth<=innerWidth));}
 assert.deepEqual(errors,[]);await page.close();console.log('PASS '+(touch?'touch':'mouse')+' workflow: draft stamps, selection, held locks, erase/Undo, REC modes, FX motion, ROOT, all-page transport, narrow layout');
 }}finally{await browser.close();}
})().catch(e=>{console.error(e);process.exitCode=1;});
