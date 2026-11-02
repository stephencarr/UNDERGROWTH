const {chromium}=require('playwright'),assert=require('node:assert/strict');
(async()=>{const browser=await chromium.launch({executablePath:'/usr/bin/chromium',headless:true,args:['--no-sandbox']});try{for(const touch of [false,true]){
 const page=await browser.newPage({hasTouch:touch,viewport:{width:1100,height:900}}),errors=[];page.on('pageerror',e=>errors.push(String(e)));
 await page.addInitScript(()=>{const Base=AudioContext;window.AudioContext=class extends Base{constructor(o){super(o);const dest=this.destination,a=this.createAnalyser();a.fftSize=2048;a.connect(dest);window.chordAnalyser=a;Object.defineProperty(this,'destination',{get:()=>a});}};});
 await page.goto(process.env.TEST_URL||'http://127.0.0.1:8000');
 const click=async el=>{await(touch?el.tap():el.click());await page.waitForTimeout(70);},btn=n=>page.locator('#fm1-controls').getByRole('button',{name:n,exact:true}),press=n=>click(btn(n));
 const turn=async(n,d)=>{await btn(n).focus();for(let i=0;i<Math.abs(d);i++)await page.keyboard.press(d>0?'ArrowUp':'ArrowDown');await page.waitForTimeout(80);};
 const peak=()=>page.evaluate(()=>{const a=new Float32Array(chordAnalyser.fftSize);chordAnalyser.getFloatTimeDomainData(a);return Math.max(...a.map(Math.abs));});
 await click(page.locator('#device-audio'));await page.waitForFunction(()=>UG.state?.voices.length===5);assert.equal(await peak(),0);
 await turn('ALGORITHM',4);await press('EDIT');await turn('SELECT',2);await turn('KNOB 1',1);await page.waitForFunction(()=>Math.abs(UG.state.voices[4].base[8]-1/7)<.001);
 await press('SEQ');await press('White key 1');await page.waitForFunction(()=>UG.state.voices[4].steps[0].on);const stored=await page.evaluate(()=>UG.state.voices[4].steps[0].lock[13]);
 await press('EDIT');await turn('SELECT',1);await turn('KNOB 2',3);assert.equal(await page.evaluate(()=>UG.state.voices[4].steps[0].lock[13]),stored);
 await press('SEQ');await press('PLAY');await page.waitForTimeout(350);assert(await peak()>.0001,'Expected real chord audio');
 await press('PLAY');await press('SEQ');await press('White key 1');await page.waitForTimeout(100);assert(await peak()>.0001,'Keyboard chord should sound');
 // Return to Steps before holding a rest to author its actual harmony.
 await press('SEQ');const key=btn('White key 2'),box=await key.boundingBox();
 if(touch){const cdp=await page.context().newCDPSession(page);await cdp.send('Input.dispatchTouchEvent',{type:'touchStart',touchPoints:[{x:box.x+box.width/2,y:box.y+box.height/2}]});await page.waitForTimeout(450);await cdp.send('Input.dispatchTouchEvent',{type:'touchEnd',touchPoints:[]});await cdp.detach();}
 else{await key.hover();await page.mouse.down();await page.waitForTimeout(450);await page.mouse.up();}
 await page.waitForFunction(()=>UG.state.panel.locks);assert.equal(await page.evaluate(()=>UG.state.voices[4].steps[1].on),false);await turn('SELECT',2);await turn('KNOB 1',2);await page.waitForFunction(()=>UG.state.voices[4].steps[1].mask&(1<<13));
 await press('SEQ');await press('HOME');await press('White key 5');await page.waitForFunction(()=>UG.state.voices[4].muted);await press('White key 5');
 await click(page.locator('#view-toggle'));await page.waitForFunction(()=>!document.querySelector('main.instrument').hidden);await click(page.locator('#voice-chord'));
 assert(await page.locator('#chord-param-8').isVisible());await page.locator('#chord-param-8').fill('4');await page.locator('#chord-param-8').dispatchEvent('input');await page.locator('#chord-param-8').dispatchEvent('change');await page.waitForFunction(()=>Math.abs(UG.state.voices[4].base[8]-4/7)<.001);
 await click(page.locator('[data-trigger="4"]'));await page.waitForTimeout(100);assert(await peak()>.0001);
 await page.keyboard.press('Escape');await page.waitForTimeout(100);assert.equal(await peak(),0);assert.deepEqual(errors,[]);
 await page.setViewportSize({width:360,height:780});assert(await page.locator('#voice-chord').isVisible());await page.close();console.log('PASS CHORD '+(touch?'touch':'mouse')+': fifth track, palette snapshot isolation, live/sequence audio, trig harmony, mix, Expanded controls, Panic');
}}finally{await browser.close();}})().catch(e=>{console.error(e);process.exitCode=1;});
