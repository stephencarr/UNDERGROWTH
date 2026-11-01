// UNDERGROWTH's experimental FM1 install flow. Transport and package parsing are pinned fm1-base MPL-2.0 modules.
import {productOf, logicalImage, checkUfw, ufwEntries} from './vendor/fm1-base/fm1pkg.js';
import {Updater} from './vendor/fm1-base/fm1ota.js';

const FILE='firmware/releases/undergrowth-0.1.0-hw1.fwsc';
const SHA='33e1a2b8ad58970f01f6ac3dfbd425217f6dbecb67af3db4c8256a51fbd6c753';
const PRODUCT='FM-1_731000100';
const $=id=>document.getElementById(id);
let image=null,access=null,updater=null,device=null,writing=false;
const status=(id,message,error=false)=>{const el=$(id);el.textContent=message;if(error)el.dataset.error='';else delete el.dataset.error;};
const log=message=>{$('log').append(message+'\n');$('log').scrollTop=$('log').scrollHeight;};
const hex=bytes=>Array.from(bytes,b=>b.toString(16).padStart(2,'0')).join('');
const isLoader=id=>/^ota-/i.test(id.model);
const isFM1=id=>id.model==='FM-1'||(isLoader(id)&&id.model==='ota-FM-1');
const canInstall=()=>!!image&&!!device&&isFM1(device)&&!device.open&&!writing;
const refresh=()=>{$('connect').disabled=!image||writing;$('install').disabled=!canInstall();$('try-port').disabled=writing;};

if(!globalThis.isSecureContext||!navigator.requestMIDIAccess||!crypto.subtle){
  status('package-status','Open this page over HTTPS or localhost in desktop Chrome or Edge for Web MIDI and package verification.',true);
}else{
  try{
    const response=await fetch(FILE,{cache:'no-store'});
    if(!response.ok)throw new Error(`Firmware file: HTTP ${response.status}`);
    const raw=new Uint8Array(await response.arrayBuffer());
    const digest=hex(new Uint8Array(await crypto.subtle.digest('SHA-256',raw)));
    if(digest!==SHA)throw new Error('Firmware checksum does not match this page. No installation is available.');
    if(productOf(raw)!==PRODUCT)throw new Error('Firmware identity does not match the FM1 target.');
    const logical=logicalImage(raw);checkUfw(logical);
    const parts=ufwEntries(logical);
    if(parts[0]?.len!==0x93000||!parts[100])throw new Error('Firmware is missing expected FM1 sections.');
    image=logical;
    status('package-status',`Verified UNDERGROWTH 0.1.0-hw1 · ${raw.length.toLocaleString()} bytes · ${PRODUCT}`);
    refresh();
  }catch(error){status('package-status',`Package check failed: ${error.message}`,true);}
}

function identified(id){
  device=id;
  if(!isFM1(id))status('device-status',`Found ${id.text}, which is not the expected FM1 target. Installation is blocked.`,true);
  else if(id.open)status('device-status',`${id.text} uses M-VAVE Open firmware. This updater does not install UNDERGROWTH directly over it.`,true);
  else status('device-status',`Identified ${id.text}${isLoader(id)?' in update mode':''}. No firmware has been written.`);
  refresh();
}
function showOtherPorts(){
  const select=$('port');select.replaceChildren();
  for(const name of updater.ports()){
    const option=document.createElement('option');option.value=name;option.textContent=name;select.append(option);
  }
  $('other-port').hidden=!select.options.length;
}
$('connect').addEventListener('click',async()=>{
  device=null;refresh();status('device-status','Requesting MIDI access and identifying the FM1…');
  try{
    access=await navigator.requestMIDIAccess({sysex:true});
    updater=new Updater(access,log);
    const found=await updater.findAny();
    if(found){found.link.close();identified(found.id);$('other-port').hidden=true;}
    else{status('device-status','No FM1 answered on a usual port. Check the cable and MIDI access, or try the port your Mac shows.',true);showOtherPorts();}
  }catch(error){status('device-status',`MIDI access failed: ${error.message}`,true);}
});
$('try-port').addEventListener('click',async()=>{
  if(!updater||writing)return;
  const name=$('port').value;status('device-status',`Asking ${name} for its FM1 identity…`);
  try{const id=await updater.probe(name);if(id)identified(id);else status('device-status',`No FM1 identity answered on ${name}.`,true);}
  catch(error){status('device-status',`Port check failed: ${error.message}`,true);}
});

$('backup-ok').addEventListener('change',()=>{$('confirm-install').disabled=!$('backup-ok').checked;});
$('install').addEventListener('click',()=>{
  if(!canInstall())return;
  $('confirm-device').textContent=`Detected: ${device.text}. Target after installation: ${PRODUCT}.`;
  $('backup-ok').checked=false;$('confirm-install').disabled=true;$('confirm').showModal();
});
window.addEventListener('beforeunload',event=>{if(writing){event.preventDefault();event.returnValue='Firmware writing in progress.';}});
$('confirm').addEventListener('close',async()=>{
  if($('confirm').returnValue!=='install'||!$('backup-ok').checked||!canInstall())return;
  writing=true;refresh();$('other-port').hidden=true;$('progress').value=0;
  status('install-status','Starting firmware update. Keep the FM1 connected.');
  const entries=ufwEntries(image),loader=entries[100],flash=entries[0];
  const checkBytes=0x400+0x400+(loader?.len-0x20||0);
  const writeBytes=flash?.len>0x4000&&flash.len<=0x100000?0x400+0x20+flash.len+(flash.len-0x4000):image.length;
  const total=checkBytes+writeBytes;
  const step=(stage,_,bytes)=>{
    if(stage==='verify')$('progress').value=Math.min(99,100*Math.min(bytes,checkBytes)/total);
    if(stage==='write')$('progress').value=Math.min(99,100*(checkBytes+bytes)/total);
    const messages={start:'Preparing update loader',verify:'Checking package on FM1',loader:'Waiting for update loader',write:'Writing firmware',reboot:'Waiting for FM1 to restart',done:`Installed ${PRODUCT}`};
    if(messages[stage])status('install-status',`${messages[stage]}${stage==='verify'||stage==='write'?` · ${Math.round($('progress').value)}%`:''}`);
    if(stage!=='verify'&&stage!=='write')log(`${stage}${stage==='done'?` ${PRODUCT}`:''}`);
  };
  try{
    let done;
    if(isLoader(device)){
      done=await updater.resume(image,step,PRODUCT);
      if(!done)throw new Error('The update loader did not complete the write. Reconnect and retry.');
    }else{
      await updater.install(image,PRODUCT,step,SHA);
    }
    $('progress').value=100;
    status('install-status',`Done. FM1 reports ${PRODUCT}. UNDERGROWTH remains experimental until device testing is complete.`);
    device=null;
  }catch(error){
    log(`${error.code||'error'}: ${error.message}`);
    status('install-status',`Update stopped: ${error.message} Reconnect and identify the FM1 before retrying.`,true);
    device=null;
  }finally{writing=false;refresh();}
});
