const {chromium}=require('playwright'),assert=require('node:assert/strict');
(async()=>{const browser=await chromium.launch({executablePath:'/usr/bin/chromium',headless:true,args:['--no-sandbox']});
try{for(const touch of [false,true]){
 const page=await browser.newPage({viewport:{width:1100,height:850},hasTouch:touch,isMobile:touch});const errors=[];page.on('pageerror',e=>errors.push(String(e)));
 await page.addInitScript(()=>{const Base=AudioContext;window.AudioContext=class extends Base{constructor(opts){super(opts);const dest=this.destination,a=this.createAnalyser();a.fftSize=2048;a.connect(dest);window.envAnalyser=a;Object.defineProperty(this,'destination',{get:()=>a});}};});
 await page.goto(process.env.TEST_URL||'http://127.0.0.1:8000');const wait=n=>page.waitForTimeout(n);
 const button=name=>page.locator('#fm1-controls').getByRole('button',{name:name.replace(/^KNOB(\d)$/,'KNOB $1'),exact:true});
 const click=async loc=>{await(touch?loc.tap():loc.click());await wait(70);},press=name=>click(button(name));
 const turn=async(name,n=1)=>{await button(name).focus();for(let i=0;i<Math.abs(n);i++)await page.keyboard.press(n>0?'ArrowUp':'ArrowDown');await wait(80);};
 const snap=()=>page.evaluate(()=>UG.state),read=()=>page.locator('#panel-readout').textContent();
 const peak=()=>page.evaluate(()=>{const a=new Float32Array(envAnalyser.fftSize);envAnalyser.getFloatTimeDomainData(a);return Math.max(...a.map(Math.abs));});
 await click(page.locator('#device-audio'));await page.waitForFunction(()=>UG.state);assert.equal(await peak(),0);
 await press('ENV');assert((await read()).includes('Damping'));await turn('KNOB2',20);assert(Math.abs((await snap()).voices[0].base[4]-.7)<.001);
 await turn('SELECT');await turn('KNOB1',-40);assert(Math.abs((await snap()).voices[0].velocity-.6)<.001);
 await press('SEQ');await press('White key 1');let s=await snap();assert(Math.abs(s.voices[0].steps[0].velocity-.6)<.001);assert(Math.abs(s.voices[0].steps[0].lock[4]-.7)<.001);
 await turn('SELECT',2);await turn('KNOB1',-20);assert(Math.abs((await snap()).voices[0].steps[0].velocity-.4)<.001);assert((await read()).includes('STEP 1'));
 await turn('SELECT');await turn('KNOB2',-20);assert(Math.abs((await snap()).voices[0].steps[0].lock[4]-.5)<.001);
 await turn('ALGORITHM');await press('ENV');assert((await read()).includes('Stretch'));await turn('KNOB1',25);await press('SEQ');await press('White key 1');
 // Motion records the new ENV slots, and Undo restores their authored values.
 await press('REC');await turn('KNOB1');await press('REC');await press('ENV');await turn('KNOB1',-40);await press('PLAY');
 await page.waitForFunction(()=>Math.abs(UG.state.voices[1].steps[0].lock[4]-.35)<.001);await press('REC');await press('PLAY');
 await press('HOME');await turn('SELECT');await press('SEL');assert(Math.abs((await snap()).voices[1].steps[0].lock[4]-.75)<.001);
 // Browser waveform persists longer for stretched fragments; remove shared delay first.
 await press('EDIT');await turn('KNOB4',-100);await press('ENV');await turn('KNOB1',-100);await press('White key 1');await wait(700);const compressed=await peak();
 await turn('KNOB1',100);await press('White key 1');await wait(700);let stretched=await peak();for(let i=0;i<12;i++){await wait(15);stretched=Math.max(stretched,await peak());}assert(stretched>.0001&&stretched>compressed*3,`stretch response ${compressed}/${stretched}`);
 await page.keyboard.press('Escape');await wait(100);assert.equal(await peak(),0);
 await turn('ALGORITHM');await press('ENV');assert((await read()).includes('Release'));await turn('SELECT');assert((await read()).includes('C2'));
 await press('White key 1');assert.equal((await snap()).draftNote[0],29);assert((await read()).includes('F1'));
 await turn('KNOB1',7);await turn('KNOB2',-5);await turn('KNOB3',1);await turn('KNOB4',-30);
 const before=(await snap()).voices[2].steps;await press('SEL');assert(await peak()>.0001);assert.deepEqual((await snap()).voices[2].steps,before);
 await press('SEQ');await press('White key 4');s=await snap();assert.equal(s.voices[2].steps[3].note,36);assert(Math.abs(s.voices[2].steps[3].velocity-.7)<.001);
 await turn('SELECT',2);await turn('KNOB1',7);assert.equal((await snap()).voices[2].steps[3].note,43);assert.equal((await snap()).draftNote[0],36);
 await page.keyboard.press('Escape');await wait(100);await page.screenshot({path:`/tmp/undergrowth-env-${touch?'touch':'mouse'}.png`,fullPage:true});
 await page.click('#view-toggle');await page.click('#voice-chatter');await page.check('#lock-mode');
 const slider=async(id,val)=>{await page.locator('#'+id).evaluate((el,value)=>{el.value=value;el.dispatchEvent(new Event('input',{bubbles:true}));el.dispatchEvent(new Event('change',{bubbles:true}));},String(val));await wait(80);};
 await page.click('#step-0');await slider('env-4',.9);await slider('velocity',.2);s=await snap();assert(Math.abs(s.voices[1].steps[0].lock[4]-.9)<.001);assert(Math.abs(s.voices[1].steps[0].velocity-.2)<.001);
 await page.click('#undo');await wait(80);assert.equal((await snap()).voices[1].steps[0].velocity,1);
 await page.uncheck('#lock-mode');await page.click('#motion');await slider('env-6',.25);await page.click('#play');await page.waitForFunction(()=>Math.abs(UG.state.voices[1].steps[0].lock[6]-.25)<.001);await page.click('#panic');await wait(100);assert.equal(await peak(),0);
 for(const width of [320,375,680]){await page.setViewportSize({width,height:850});assert(await page.evaluate(()=>document.documentElement.scrollWidth<=innerWidth));}
 assert.deepEqual(errors,[]);await page.close();console.log('PASS '+(touch?'touch':'mouse')+' ENV: velocity stamps/locks, environment locks/motion/Undo, Chatter stretch audio, ROOT pitch entry/audition, narrow layout');
}}finally{await browser.close();}})().catch(e=>{console.error(e);process.exitCode=1;});
