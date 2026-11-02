const {chromium}=require('playwright'),assert=require('node:assert/strict');
(async()=>{const browser=await chromium.launch({executablePath:'/usr/bin/chromium',headless:true,args:['--no-sandbox']});try{
const page=await browser.newPage();
await page.addInitScript(()=>{
 const pack7=data=>{const out=[];let acc=0,bits=0;for(const byte of data){acc|=byte<<bits;bits+=8;while(bits>=7){out.push(acc&127);acc>>>=7;bits-=7;}}if(bits)out.push(acc&127);return out;};
 const access={inputs:new Map(),outputs:new Map(),addEventListener:()=>{},removeEventListener:()=>{}};window.sent=[];
 function pair(name,identity){
  const input={id:name+'-in',name,state:'connected',open:async()=>{},onmidimessage:null};
  let upgrading=false;
  const output={id:name+'-out',name,state:'connected',open:async()=>{},send:data=>{
   window.sent.push({name,data:[...data]});
   if(data.length===10&&data[1]===0&&data[2]===0x32){const bytes=[0,0x59,0x11,0,0,0,...Array.from(identity,c=>c.charCodeAt(0))];while(bytes.length<34)bytes.push(0);queueMicrotask(()=>input.onmidimessage?.({data:Uint8Array.from([0xF0,...pack7(bytes),0xF7])}));}
   else if(data[1]===0x22&&data[2]===0x24){
    upgrading=true;const bytes=[0,0x59,0x30,0,0,0,0,0,0,0,0xE0,0,0,0];bytes.push(~bytes.slice(6).reduce((a,b)=>a+b,0)&255);
    queueMicrotask(()=>input.onmidimessage?.({data:Uint8Array.from([0xF0,...pack7(bytes),0xF7])}));
   }else if(upgrading&&data.length>12&&name==='FM-1 initial'){
    upgrading=false;input.state=output.state='disconnected';access.inputs.delete(input.id);access.outputs.delete(output.id);
    const loader=pair('FM-1 loader','ota-FM-1_100');access.inputs.set(loader.input.id,loader.input);access.outputs.set(loader.output.id,loader.output);
   }
  }};return {input,output};
 }
 const initial=pair('FM-1 initial','FM-1_620000100');access.inputs.set(initial.input.id,initial.input);access.outputs.set(initial.output.id,initial.output);
 navigator.requestMIDIAccess=async()=>access;
});
await page.goto((process.env.TEST_URL||'http://127.0.0.1:8000')+'/flash.html');
await page.locator('#connect').click();await page.locator('#install').click();await page.locator('#backup-ok').check();await page.locator('#confirm-install').click();
await page.waitForFunction(()=>document.getElementById('handover').open);
assert.equal(await page.evaluate(()=>sent.some(x=>x.name==='FM-1 loader'&&x.data[1]===0x22)),false);
await page.locator('#handover-identify').click();await page.waitForFunction(()=>!document.getElementById('handover-continue').disabled);
await page.keyboard.press('Escape');await page.getByText('Update stopped:',{exact:false}).waitFor();
assert.equal(await page.evaluate(()=>sent.some(x=>x.name==='FM-1 loader'&&x.data[1]===0x22)),false);
console.log('PASS simulated USB re-enumeration: loader needs explicit identification/confirmation; Escape sends no loader update');
}finally{await browser.close();}})().catch(e=>{console.error(e);process.exitCode=1;});
