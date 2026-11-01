const $=id=>document.getElementById(id);
const names=['Kick','Chatter'];
const params=['body','edge','length','send'];
let context,node,state,voice=0,selected=0,starting=null,dragging=null;
const musical=[...document.querySelectorAll('button,input')].filter(el=>!['enable-audio','voice-kick','voice-chatter','lock-mode'].includes(el.id));
for(const el of musical)el.disabled=true;
function command(type,...args){if(node)node.port.postMessage({type,args});}
function status(text){$('audio-status').textContent=text;}
async function enableAudio(){
 if(context?.state==='running')return true;
 if(starting)return starting;
 starting=(async()=>{
  $('enable-audio').disabled=true;status('Preparing audio…');
  try{
   if(!window.isSecureContext)throw new Error('Use localhost or HTTPS for browser audio.');
   if(!context){
    context=new AudioContext({sampleRate:44100,latencyHint:'interactive'});
    if(context.sampleRate<8000||context.sampleRate>96000)throw new Error('Unsupported audio sample rate.');
    const response=await fetch('core.wasm');if(!response.ok)throw new Error('Could not load core.wasm. Serve the complete web folder.');
    const module=await WebAssembly.compile(await response.arrayBuffer());
    await context.audioWorklet.addModule('worklet.js');
    node=new AudioWorkletNode(context,'undergrowth',{numberOfInputs:0,numberOfOutputs:1,outputChannelCount:[2],processorOptions:{module}});
    node.port.onmessage=({data})=>{if(data.type==='state'){state=data;paint();}};
    node.onprocessorerror=()=>{status('Audio stopped unexpectedly. Reload to restart safely.');node.disconnect();for(const el of musical)el.disabled=true;};
    node.connect(context.destination);
    context.onstatechange=()=>{if(context.state!=='running'){command('panic');status('Audio paused. Enable audio to continue.');$('enable-audio').disabled=false;$('enable-audio').textContent='Resume audio';}};
   }
   await context.resume();
   for(const el of musical)el.disabled=false;
   $('enable-audio').textContent='Audio enabled';status(`Audio ready · ${(context.sampleRate/1000).toFixed(1)} kHz · silent until you play`);
   command('snapshot');return true;
  }catch(error){
   if(context){try{await context.close();}catch{}context=null;node=null;}
   $('enable-audio').disabled=false;$('enable-audio').textContent='Retry audio';status(error.message);return false;
  }finally{starting=null;}
 })();return starting;
}
$('enable-audio').onclick=enableAudio;
function setVoice(v){voice=v;dragging=null;paint();}
for(const b of document.querySelectorAll('[data-voice]'))b.onclick=()=>setVoice(Number(b.dataset.voice));
function hit(v){command('hit',v,1);const pad=document.querySelector(`[data-trigger="${v}"]`);pad.classList.add('active');setTimeout(()=>pad.classList.remove('active'),90);}
for(const b of document.querySelectorAll('[data-trigger]'))b.onclick=()=>hit(Number(b.dataset.trigger));
$('play').onclick=()=>command('play',1);
$('stop').onclick=()=>command('play',0);
$('panic').onclick=()=>command('panic');
$('tempo').onchange=()=>command('tempo',Math.round(Number($('tempo').value)||110));
$('master').oninput=()=>command('master',Number($('master').value));
$('record').onclick=()=>command('record',-1);
$('rejoin').onclick=()=>command('rejoin');
$('undo').onclick=()=>command('undo');
$('clear-pattern').onclick=()=>command('clear');
$('clear-lock').onclick=()=>{command('begin');command('clearlock',voice,selected);};
$('lock-mode').onchange=()=>{dragging=null;paint();};
for(const b of document.querySelectorAll('[data-step]'))b.onclick=()=>{
 if(!state)return;const s=Number(b.dataset.step);
 if($('lock-mode').checked){selected=s;paint();}
 else{command('begin');command('toggle',voice,s);}
};
for(const [p,id] of params.entries()){
 const el=$(id);
 const begin=()=>{if(dragging!==id){command('begin');dragging=id;}};
 el.oninput=()=>{
  begin();const val=Number(el.value);$(`${id}-value`).textContent=Math.round(val*100);
  if($('lock-mode').checked)command('lock',voice,selected,p,val);else command('param',voice,p,val);
 };
 el.onchange=()=>{dragging=null;};el.addEventListener('blur',()=>dragging=null);
}
function paint(){
 for(const b of document.querySelectorAll('[data-voice]')){const on=Number(b.dataset.voice)===voice;b.classList.toggle('active',on);b.setAttribute('aria-pressed',String(on));}
 $('selected-voice').textContent=names[voice];
 const lock=$('lock-mode').checked;
 if(!state){$('edit-status').textContent='Enable audio to begin. Your canvas is empty.';return;}
 const v=state.voices[voice],st=v.steps[selected];
 $('transport-status').textContent=state.playing?'Playing':'Stopped';
 $('step-status').textContent=state.step>=0?`${String(state.step+1).padStart(2,'0')} / 16`:'— / 16';
 $('play').classList.toggle('active',!!state.playing);$('play').setAttribute('aria-pressed',String(!!state.playing));
 $('record').classList.toggle('active',!!state.recording);$('record').setAttribute('aria-pressed',String(!!state.recording));
 $('record').textContent=state.recording?'Recording hits':'Record hits';
 $('undo').disabled=!state.undo;$('clear-lock').disabled=!lock||!st.mask;
 if(document.activeElement!==$('tempo'))$('tempo').value=state.tempo;
 $('rejoin').disabled=!(state.voices[0].takeover||state.voices[1].takeover);
 for(const b of document.querySelectorAll('[data-step]')){
  const s=Number(b.dataset.step),item=v.steps[s];
  b.classList.toggle('active',item.on);b.classList.toggle('playing',state.playing&&state.step===s);
  b.classList.toggle('selected',lock&&selected===s);b.classList.toggle('locked',!!item.mask);
  b.setAttribute('aria-pressed',String(item.on));
  b.setAttribute('aria-label',`${names[voice]} step ${s+1}, ${item.on?'on':'off'}${item.mask?', parameter locks':''}${lock&&selected===s?', selected for editing':''}`);
 }
 for(const [p,id] of params.entries())if(dragging!==id){
  const val=lock&&(st.mask&(1<<p))?st.lock[p]:v.base[p];$(id).value=val;$(`${id}-value`).textContent=Math.round(val*100);
 }
 $('edit-status').textContent=lock?`LOCK · step ${selected+1} · knobs write locks${st.on?'':' · this step stays silent until enabled'}`:
 v.takeover?'LIVE · your changed knobs override locks. Rejoin restores lock priority.':
 state.playing&&v.steps[state.step]?.mask?'LOCK · stored step values are sounding.':'BASE · knobs shape this voice. Enable Edit locks to shape a single step.';
}
document.addEventListener('keydown',e=>{
 if(e.code==='Escape'&&node){e.preventDefault();command('panic');return;}
 if(e.repeat||e.ctrlKey||e.metaKey||e.altKey||['INPUT','TEXTAREA','SELECT'].includes(e.target.tagName))return;
 if(e.code==='Digit1'){setVoice(0);return;}if(e.code==='Digit2'){setVoice(1);return;}
 if(!node)return;
 if(e.code==='Space'&&e.target.tagName!=='BUTTON'){e.preventDefault();command('play',-1);}
 if(e.code==='KeyA'||e.code==='KeyZ')hit(0);
 if(e.code==='KeyS'||e.code==='KeyX')hit(1);

});
document.addEventListener('visibilitychange',()=>{if(document.hidden)command('panic');});
window.addEventListener('pagehide',()=>{command('panic');context?.suspend();});
paint();
