const {chromium}=require('playwright'),assert=require('node:assert/strict');
(async()=>{const browser=await chromium.launch({executablePath:'/usr/bin/chromium',headless:true,args:['--no-sandbox']});try{for(const touch of [false,true]){
 const page=await browser.newPage({hasTouch:touch,viewport:{width:1100,height:850}}),errors=[];page.on('pageerror',e=>errors.push(String(e)));
 await page.goto(process.env.TEST_URL||'http://127.0.0.1:8000');
 const click=async el=>{await(touch?el.tap():el.click());await page.waitForTimeout(70);},button=n=>page.locator('#fm1-controls').getByRole('button',{name:n,exact:true}),press=n=>click(button(n));
 const turn=async(n,d)=>{await button(n).focus();for(let i=0;i<Math.abs(d);i++)await page.keyboard.press(d>0?'ArrowUp':'ArrowDown');await page.waitForTimeout(70);};
 await click(page.locator('#device-audio'));await page.waitForFunction(()=>UG.state?.voices.length===5);await turn('ALGORITHM',1);await press('SEQ');await press('White key 1');await press('White key 5');
 const old=await page.evaluate(()=>UG.state.voices[1].steps[0].lock[0]);await press('PLAY');await press('EDIT');await turn('KNOB 1',25);await turn('KNOB 2',-20);
 const draft=await page.evaluate(()=>UG.state.voices[1].base.slice(0,2));await press('SEQ');await press('White key 9');
 await page.waitForFunction(()=>UG.state.voices[1].steps[8].on);
 for(let i=0;i<32;i++){const v=await page.evaluate(()=>UG.state.voices[1]);for(let p=0;p<2;p++){assert(Math.abs(v.effective[p]-draft[p])<.001,'Stamp must retain the live sound');assert(Math.abs(v.steps[8].lock[p]-draft[p])<.001,'New hit captures draft');}assert.equal(v.steps[0].lock[0],old);await page.waitForTimeout(80);}
 await press('Black key 8');await page.waitForFunction(()=>UG.state.voices[1].takeover===0);await press('PLAY');await press('PLAY');await page.waitForFunction(()=>UG.state.step===0&&Math.abs(UG.state.voices[1].effective[0]-UG.state.voices[1].steps[0].lock[0])<.001);
 await page.keyboard.press('Escape');assert.deepEqual(errors,[]);await page.close();console.log('PASS '+(touch?'touch':'mouse')+' same-track stamp: draft capture, live override across sequence, old locks preserved, explicit Rejoin and restart');
}}finally{await browser.close();}})().catch(e=>{console.error(e);process.exitCode=1;});
