const {chromium}=require('playwright'),assert=require('node:assert/strict');
(async()=>{const browser=await chromium.launch({executablePath:'/usr/bin/chromium',headless:true,args:['--no-sandbox']});try{for(const touch of [false,true]){
 const page=await browser.newPage({hasTouch:touch,viewport:{width:1100,height:850}}),errors=[];page.on('pageerror',e=>errors.push(String(e)));
 await page.addInitScript(()=>{const Base=AudioContext;window.AudioContext=class extends Base{constructor(opts){super(opts);const dest=this.destination,a=this.createAnalyser();a.fftSize=2048;a.connect(dest);window.atmoAnalyser=a;Object.defineProperty(this,'destination',{get:()=>a});}};});
 await page.goto(process.env.TEST_URL||'http://127.0.0.1:8000');
 const click=async el=>{await(touch?el.tap():el.click());await page.waitForTimeout(80);};const btn=n=>page.locator('#fm1-controls').getByRole('button',{name:n,exact:true});
 const turn=async(n,d)=>{await btn(n).focus();for(let i=0;i<Math.abs(d);i++)await page.keyboard.press(d>0?'ArrowUp':'ArrowDown');await page.waitForTimeout(80);};
 const peak=()=>page.evaluate(()=>{const a=new Float32Array(atmoAnalyser.fftSize);atmoAnalyser.getFloatTimeDomainData(a);return Math.max(...a.map(Math.abs));});
 await click(page.locator('#device-audio'));await page.waitForFunction(()=>window.UG?.state);assert.equal(await peak(),0);
 for(const [voice,label] of [[0,'Harmonics'],[1,'Colour'],[3,'Vowel']]){
  await page.evaluate(v=>{UG.command('panic');UG.command('uiSelect',0,v);},voice);await click(btn('ENV'));await turn('SELECT',2);
  await page.waitForFunction(label=>UG.state.panel.knobLabels.includes(label),label);
  await turn('SELECT',1);await turn('KNOB 3',1);await click(btn('PLAY'));await page.waitForTimeout(400);
  assert(await peak()>1e-5);assert(await page.evaluate(v=>UG.state.voices[v].steps.every(s=>!s.on),voice));
  await click(btn('FX'));await turn('KNOB 2',40);assert(await page.evaluate(v=>UG.state.voices[v].base[16]>.39,voice));
  await click(page.locator('#view-toggle'));const input=page.locator('#atmo-param-11');assert(await input.isVisible());
  await input.evaluate(el=>{el.value=.8;el.dispatchEvent(new Event('input',{bubbles:true}));el.dispatchEvent(new Event('change',{bubbles:true}));});
  await page.waitForFunction(v=>UG.state.voices[v].base[11]>.79,voice);
  await click(page.locator('#view-toggle'));await page.evaluate(()=>UG.command('panic'));await page.waitForTimeout(250);assert(await peak()<1e-7);
 }
 assert.deepEqual(errors,[]);await page.close();console.log('PASS '+(touch?'touch':'mouse')+' atmospheres: harmonic/noise/formant, explicit enable, empty-score audio, separate send, Expanded edit and Panic');
}}finally{await browser.close();}})().catch(e=>{console.error(e);process.exitCode=1;});
