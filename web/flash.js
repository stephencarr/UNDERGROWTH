// UNDERGROWTH's experimental FM1 install flow. Transport and package parsing are pinned fm1-base MPL-2.0 modules.
import {productOf, logicalImage, checkUfw, ufwEntries} from './vendor/fm1-base/fm1pkg.js';
import {Updater} from './vendor/fm1-base/fm1ota.js';
import {pairs} from './vendor/fm1-base/fm1ports.js';
import {ConfirmedUpdater, pairPresent, verifyPair} from './confirmed-updater.js';

const FILE='firmware/releases/undergrowth-0.3.0-hw1.fwsc';
const SHA='e6b4d13a8dcca8d6387ef11f1b256e38118057dbdb6da4a1e3285024b7a17793';
const PRODUCT='FM-1_731000300';
const $=id=>document.getElementById(id);
let image=null,access=null,updater=null,device=null,target=null,writing=false,probing=false,transitionPair=null,installApproved=false;
const status=(id,message,error=false)=>{const el=$(id);el.textContent=message;if(error)el.dataset.error='';else delete el.dataset.error;};
const log=message=>{$('log').append(message+'\n');$('log').scrollTop=$('log').scrollHeight;};
const hex=bytes=>Array.from(bytes,b=>b.toString(16).padStart(2,'0')).join('');
const isLoader=id=>/^ota-/i.test(id.model);
const isFM1=id=>id.model==='FM-1'||(isLoader(id)&&id.model==='ota-FM-1');
const canInstall=()=>!!image&&!!device&&isFM1(device)&&!device.open&&!writing&&!probing&&!!target&&pairPresent(access,target);
const refresh=()=>{$('connect').disabled=!image||writing||probing;$('install').disabled=!canInstall();$('try-port').disabled=writing||probing;$('port').disabled=writing||probing;};

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
    status('package-status',`Verified UNDERGROWTH 0.3.0-hw1 · ${raw.length.toLocaleString()} bytes · ${PRODUCT}`);
    refresh();
  }catch(error){status('package-status',`Package check failed: ${error.message}`,true);}
}

function identified(found){
  const id=found.id;device=id;target={input:found.link.input,output:found.link.output};found.link.close();
  if(!isFM1(id))status('device-status',`Found ${id.text}, which is not the expected FM1 target. Installation is blocked.`,true);
  else if(id.open)status('device-status',`${id.text} uses M-VAVE Open firmware. This updater does not install UNDERGROWTH directly over it.`,true);
  else status('device-status',`Identified ${id.text} on ${target.input.name} → ${target.output.name}${isLoader(id)?' in update mode':''}. No firmware has been written.`);
  refresh();
}
function showOtherPorts(){
  const select=$('port');select.replaceChildren();
  for(const pair of pairs(access)){
    const option=document.createElement('option');option.value=pair.input.id;option.textContent=`${pair.input.name} → ${pair.output.name}`;select.append(option);
  }
  $('other-port').hidden=!select.options.length;
}
function stateChanged(){
  if(target&&!pairPresent(access,target)){
    target.invalidated=true;
    if(!writing){device=null;if($('confirm').open)$('confirm').close('cancel');status('device-status','The identified pair disconnected. Identify the FM1 again.',true);refresh();}
  }
  if(transitionPair&&!pairPresent(access,transitionPair)){transitionPair.invalidated=true;$('handover-continue').disabled=true;}
}
$('connect').addEventListener('click',async()=>{
  if(probing||writing)return;
  device=target=null;probing=true;refresh();status('device-status','Requesting MIDI access and identifying the FM1…');
  try{
    access?.removeEventListener?.('statechange',stateChanged);
    access=await navigator.requestMIDIAccess({sysex:true});access.addEventListener?.('statechange',stateChanged);
    updater=new Updater(access,log);
    const found=await updater.findAny();
    if(found){identified(found);$('other-port').hidden=true;}
    else{status('device-status','No FM1 answered on a usual port. Check the cable and MIDI access, or try the port your Mac shows.',true);showOtherPorts();}
  }catch(error){status('device-status',`MIDI access failed: ${error.message}`,true);}
  finally{probing=false;refresh();}
});
$('try-port').addEventListener('click',async()=>{
  if(!access||writing||probing)return;
  device=target=null;probing=true;refresh();
  try{
    const pair=pairs(access).find(p=>p.input.id===$('port').value);
    if(!pair)throw new Error('That MIDI pair is no longer connected.');
    status('device-status',`Asking ${pair.name} for its FM1 identity…`);
    identified(await verifyPair(access,pair));
  }catch(error){status('device-status',`Port check failed: ${error.message}`,true);}
  finally{probing=false;refresh();}
});

// The loader/restarted firmware may enumerate as a new MIDI pair. Never guess which device it is.
async function confirmNextPair(filter,milliseconds){
  const dialog=$('handover'),select=$('handover-port');let checkedIdentity=null,active=true,choice=0;
  transitionPair=null;dialog.returnValue='';$('handover-continue').disabled=true;
  $('handover-status').textContent='Select the same physical FM1 after it reappears, then identify its new MIDI pair.';
  const populate=()=>{
    choice++;transitionPair=null;checkedIdentity=null;$('handover-continue').disabled=true;select.replaceChildren();
    for(const pair of pairs(access)){const option=document.createElement('option');option.value=pair.input.id;option.textContent=`${pair.input.name} → ${pair.output.name}`;select.append(option);}
  };
  const identify=async()=>{
    const request=++choice;transitionPair=null;checkedIdentity=null;$('handover-continue').disabled=true;$('handover-identify').disabled=true;
    try{
      const pair=pairs(access).find(p=>p.input.id===select.value);
      if(!pair)throw new Error('No connected MIDI pair selected. Refresh ports after the FM1 reappears.');
      const found=await verifyPair(access,pair,id=>isFM1(id)&&!id.open&&filter(id));found.link.close();
      if(!active||!dialog.open||request!==choice)return;
      transitionPair=pair;checkedIdentity=found.id.text;
      $('handover-status').textContent=`Identified ${checkedIdentity} on ${pair.input.name} → ${pair.output.name}. Confirm this is the same physical FM1.`;
      $('handover-continue').disabled=false;
    }catch(error){if(active&&request===choice)$('handover-status').textContent=error.message;}
    finally{if(active)$('handover-identify').disabled=false;}
  };
  populate();$('handover-refresh').onclick=populate;$('handover-identify').onclick=identify;select.onchange=()=>{choice++;transitionPair=null;checkedIdentity=null;$('handover-continue').disabled=true;};
  $('handover-identify').disabled=false;dialog.showModal();
  const timeout=setTimeout(()=>dialog.close('timeout'),Math.max(milliseconds,180000));
  await new Promise(resolve=>dialog.addEventListener('close',resolve,{once:true}));clearTimeout(timeout);active=false;choice++;
  const pair=transitionPair;transitionPair=null;
  if(dialog.returnValue!=='continue'||!pair||!checkedIdentity)throw new Error('Device handover canceled. Identify the FM1 again before retrying.');
  return verifyPair(access,pair,id=>id.text===checkedIdentity&&filter(id));
}

$('backup-ok').addEventListener('change',()=>{$('confirm-install').disabled=!$('backup-ok').checked;});
$('install').addEventListener('click',()=>{
  if(!canInstall())return;
  $('confirm-device').textContent=`Detected: ${device.text} on ${target.input.name} → ${target.output.name}. Target after installation: ${PRODUCT}.`;
  installApproved=false;$('confirm').returnValue='';$('backup-ok').checked=false;$('confirm-install').disabled=true;$('confirm').showModal();
});
$('confirm').querySelector('form').addEventListener('submit',event=>{installApproved=event.submitter?.id==='confirm-install'&&$('backup-ok').checked&&canInstall();});
$('confirm').addEventListener('cancel',()=>{installApproved=false;$('confirm').returnValue='cancel';});
$('handover').addEventListener('cancel',()=>{$('handover').returnValue='cancel';});
window.addEventListener('beforeunload',event=>{if(writing){event.preventDefault();event.returnValue='Firmware writing in progress.';}});
$('confirm').addEventListener('close',async()=>{
  const approved=installApproved;installApproved=false;
  if(!approved||$('confirm').returnValue!=='install'||!$('backup-ok').checked||!canInstall())return;
  const bound=new ConfirmedUpdater(access,target,device.text,log,confirmNextPair);
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
      done=await bound.resume(image,step,PRODUCT);
      if(!done)throw new Error('The update loader did not complete the write. Reconnect and retry.');
    }else{
      await bound.install(image,PRODUCT,step,SHA);
    }
    $('progress').value=100;
    status('install-status',`Done. FM1 reports ${PRODUCT}. UNDERGROWTH remains experimental until device testing is complete.`);
    device=target=null;
  }catch(error){
    log(`${error.code||'error'}: ${error.message}`);
    status('install-status',`Update stopped: ${error.message} Reconnect and identify the FM1 before retrying.`,true);
    device=target=null;
  }finally{writing=false;refresh();}
});
