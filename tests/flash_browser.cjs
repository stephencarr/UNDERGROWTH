const {chromium}=require('playwright');
const assert=require('node:assert/strict');
(async()=>{
 const browser=await chromium.launch({executablePath:process.env.CHROMIUM_PATH||'/usr/bin/chromium',headless:true,args:['--no-sandbox']});
 const base=process.env.TEST_URL||'http://127.0.0.1:8000';
 try{
  const page=await browser.newPage();const errors=[];page.on('pageerror',e=>errors.push(String(e)));
  await page.addInitScript(()=>{navigator.requestMIDIAccess=async()=>{throw new Error('Test denial');};});
  await page.goto(base+'/flash.html');
  await page.getByText('Verified UNDERGROWTH 0.4.3-hw1').waitFor();
  assert.equal(await page.locator('#install').isDisabled(),true);
  await page.locator('#connect').click();
  await page.getByText('MIDI access failed: Test denial').waitFor();
  assert.equal(await page.locator('#install').isDisabled(),true);
  assert.deepEqual(errors,[]);
  await page.close();
  const identified=await browser.newPage();
  await identified.addInitScript(()=>{
   window.__sends=[];
   const pack7=data=>{const out=[];let acc=0,bits=0;for(const byte of data){acc|=byte<<bits;bits+=8;while(bits>=7){out.push(acc&127);acc>>>=7;bits-=7;}}if(bits)out.push(acc&127);return out;};
   window.__identity='FM-1_620000100';
   const input={id:'in',name:'FM-1 Test',state:'connected',open:async()=>{},onmidimessage:null};
   const output={id:'out',name:'FM-1 Test',state:'connected',open:async()=>{},send:data=>{
    window.__sends.push(Array.from(data));
    if(data[0]===0xF0&&data[1]===0&&data[2]===0x32){
     const bytes=[0,0x59,0x11,0,0,0,...Array.from(window.__identity,c=>c.charCodeAt(0))];while(bytes.length<34)bytes.push(0);
     setTimeout(()=>input.onmidimessage?.({data:new Uint8Array([0xF0,...pack7(bytes),0xF7])}),0);
    }
   }};
   navigator.requestMIDIAccess=async()=>({inputs:new Map([['in',input]]),outputs:new Map([['out',output]])});
  });
  await identified.goto(base+'/flash.html');
  await identified.getByText('Verified UNDERGROWTH 0.4.3-hw1').waitFor();
  await identified.locator('#connect').click();
  await identified.getByText('Identified FM-1_620000100').waitFor();
  await identified.locator('#install').click();
  assert.equal(await identified.locator('#confirm-install').isDisabled(),true);
  await identified.locator('#backup-ok').check();
  assert.equal(await identified.locator('#confirm-install').isEnabled(),true);
  await identified.locator('#confirm button[value="cancel"]').click();
  assert.equal(await identified.evaluate(()=>window.__sends.some(d=>d[1]===0x22&&d[2]===0x24)),false);
  // A retry must discard the prior install result, including when Escape closes it.
  await identified.evaluate(()=>document.getElementById('confirm').returnValue='install');
  await identified.locator('#install').click();await identified.locator('#backup-ok').check();await identified.keyboard.press('Escape');
  assert.equal(await identified.evaluate(()=>window.__sends.some(d=>d[1]===0x22&&d[2]===0x24)),false);
  // Recheck the confirmed identity on the same pair before any update command.
  await identified.locator('#install').click();await identified.locator('#backup-ok').check();
  await identified.evaluate(()=>window.__identity='FM-1_620000101');await identified.locator('#confirm-install').click();
  await identified.getByText('Update stopped:',{exact:false}).waitFor();
  assert.equal(await identified.evaluate(()=>window.__sends.some(d=>d[1]===0x22&&d[2]===0x24)),false);
  await identified.close();
  const damaged=await browser.newPage();
  await damaged.route('**/firmware/releases/undergrowth-0.4.3-hw1.fwsc',async route=>{
   const response=await route.fetch();const body=await response.body();body[body.length-1]^=1;
   await route.fulfill({response,body});
  });
  await damaged.goto(base+'/flash.html');
  await damaged.getByText('Firmware checksum does not match this page').waitFor();
  assert.equal(await damaged.locator('#connect').isDisabled(),true);
  assert.equal(await damaged.locator('#install').isDisabled(),true);
  console.log('PASS Chrome flasher: verified package, FM1 identity and confirmation, no write on cancel, denied MIDI, corrupt package blocked');
 }finally{await browser.close();}
})().catch(e=>{console.error(e);process.exitCode=1;});
