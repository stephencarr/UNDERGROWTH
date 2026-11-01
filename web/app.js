const $=id=>document.getElementById(id);
const names=['Kick','Chatter','ROOT'];
const params=['body','edge','length','send','env-4','env-5','env-6'];
const environmentLabels=[['Damping','Drop','Response'],['Stretch','Fracture','Grain'],['Glide','Weight','Response']];
const noteName=n=>['C','C♯','D','D♯','E','F','F♯','G','G♯','A','A♯','B'][n%12]+(Math.floor(n/12)-1);
for(const b of document.querySelectorAll('[data-step]')){const label=document.createElement('span');label.className='note-label';b.append(label);}
let context,node,state,voice=0,selected=0,starting=null,dragging=null;
const musical=[...document.querySelector('main.instrument').querySelectorAll('button,input,select')].filter(el=>!['enable-audio','voice-kick','voice-chatter','voice-root','lock-mode'].includes(el.id));
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
function chooseLockStep(){
 if(!state)return;
 const steps=state.voices[voice].steps;
 if(!steps[selected].on){const first=steps.findIndex(s=>s.on);if(first>=0)selected=first;}
}
function setVoice(v){voice=v;dragging=null;if($('lock-mode').checked&&!state?.armed){chooseLockStep();command('rejoin');}paint();}
for(const b of document.querySelectorAll('[data-voice]'))b.onclick=()=>setVoice(Number(b.dataset.voice));
function hit(v){command('hit',v,-1);const pad=document.querySelector(`[data-trigger="${v}"]`);pad.classList.add('active');setTimeout(()=>pad.classList.remove('active'),90);}
for(const b of document.querySelectorAll('[data-trigger]'))b.onclick=()=>hit(Number(b.dataset.trigger));
$('play').onclick=()=>command('play',1);
$('stop').onclick=()=>command('play',0);
$('panic').onclick=()=>command('panic');
$('tempo').onchange=()=>command('tempo',Math.round(Number($('tempo').value)||110));
$('master').oninput=()=>command('master',Number($('master').value));
$('record').onclick=()=>command('record',-1);
$('motion').onclick=()=>{dragging=null;$('lock-mode').checked=false;command('motion',-1);paint();};
$('rejoin').onclick=()=>command('rejoin');
$('undo').onclick=()=>command('undo');
$('clear-pattern').onclick=()=>command('clear');
$('clear-lock').onclick=()=>{command('begin');command('clearlock',voice,selected);};
$('lock-mode').onchange=()=>{dragging=null;if($('lock-mode').checked){chooseLockStep();command('rejoin');}paint();};
$('lock-hit').onclick=()=>{command('begin');command('toggle',voice,selected);};
$('lock-play').onclick=()=>command('play',1);
for(const b of document.querySelectorAll('[data-step]'))b.onclick=()=>{
 if(!state)return;const s=Number(b.dataset.step);
 if(voice===2||$('lock-mode').checked){selected=s;paint();}
 else{command('begin');command('toggle',voice,s);}
};
for(const [p,id] of params.entries()){
 const el=$(id);
 const begin=()=>{const first=dragging!==id;dragging=id;return first;};
 el.oninput=()=>{
  const first=begin();const val=Number(el.value);$(`${id}-value`).textContent=Math.round(val*100);
  if($('lock-mode').checked){if(first)command('begin');command('lock',voice,selected,p,val);}else command(first?'editparam':'param',voice,p,val);
 };
 el.onchange=()=>{dragging=null;};el.addEventListener('blur',()=>dragging=null);
}
$('velocity').oninput=()=>{if(!state||state.armed||state.motion||state.recording)return;const first=dragging!=='velocity';dragging='velocity';command(first?'editvelocity':'velocity',voice,$('lock-mode').checked?selected:-1,Number($('velocity').value));};
$('velocity').onchange=()=>dragging=null;
$('velocity').addEventListener('blur',()=>dragging=null);
const grooveText=(id,value)=>id==='swing'?`${(50+value*100/6).toFixed(1)}%`:`${Math.round(value*100)}%`;
for(const [p,id] of ['swing','human'].entries()){
 const el=$(id);
 el.oninput=()=>{if(dragging!==id){command('begin');dragging=id;}const val=Number(el.value);$(`${id}-value`).textContent=grooveText(id,val);command('groove',p,val);};
 el.onchange=()=>{dragging=null;};el.addEventListener('blur',()=>dragging=null);
}
function paint(){
 for(const b of document.querySelectorAll('[data-voice]')){const on=Number(b.dataset.voice)===voice;b.classList.toggle('active',on);b.setAttribute('aria-pressed',String(on));}
 $('selected-voice').textContent=names[voice];
 const lock=$('lock-mode').checked;
 $('lock-editor').hidden=!lock;
 $('root-editor').hidden=voice!==2;
 $('sequence-help').textContent=voice===2?'Choose a step, enable Note on, then set pitch, gate and slide. Unchecked steps are rests.':'Tap steps to plant hits. Turn a knob to shape the selected voice.';
 if(!state){$('edit-status').textContent='Enable audio to begin. Your canvas is empty.';return;}
 for(const [p,id] of ['swing','human'].entries())if(dragging!==id){$(id).value=state.groove[p];$(`${id}-value`).textContent=grooveText(id,state.groove[p]);}
 const v=state.voices[voice],st=v.steps[selected];
 for(let i=0;i<3;i++)$('env-'+(i+4)+'-label').textContent=environmentLabels[voice][i];
 $('velocity-label').textContent=lock?`Step ${selected+1} velocity`:'Draft velocity';
 $('velocity').disabled=state.armed||state.motion||!!state.recording;
 if(dragging!=='velocity'){$('velocity').value=lock?st.velocity:v.velocity;$('velocity-value').textContent=Math.round(Number($('velocity').value)*100);}
 if(voice===2){
  $('root-step-label').textContent=`ROOT · step ${selected+1} · ${st.on?'note':'rest'}`;
  $('root-on').checked=st.on;$('root-note').value=st.note;$('root-slide').checked=st.slide;
  if(dragging!=='root-gate')$('root-gate').value=st.gate;
  $('root-gate-value').textContent=Math.round(st.gate*100)+'%';
 }
 for(const id of ['root-on','root-note','root-gate','root-slide'])$(id).disabled=state.motion;
 const locked=params.filter((_,p)=>st.mask&(1<<p)).map(id=>id.startsWith('env-')?environmentLabels[voice][Number(id.slice(-1))-4]:id[0].toUpperCase()+id.slice(1));
 $('lock-target').textContent=`${names[voice]} · step ${selected+1} · ${st.on?'hit on':'NO HIT — enable it to hear this step'}${locked.length?' · Locked: '+locked.join(', '):' · no locks yet'}`;
 $('lock-hit').textContent=st.on?'Disable hit':'Enable hit';$('lock-hit').setAttribute('aria-pressed',String(st.on));
 $('lock-play').disabled=!!state.playing;$('lock-play').textContent=state.playing?'Loop playing':'Play loop';
 for(const [p,id] of params.entries()){
  $(id).closest('label').classList.toggle('has-lock',lock&&!!(st.mask&(1<<p)));
  const controlName=p<4?id:environmentLabels[voice][p-4];$(id).setAttribute('aria-label',lock?`${names[voice]} step ${selected+1} ${controlName}${st.mask&(1<<p)?', locked':''}`:controlName);
 }
 $('transport-status').textContent=state.playing?'Playing':'Stopped';
 $('step-status').textContent=state.step>=0?`${String(state.step+1).padStart(2,'0')} / 16`:'— / 16';
 $('play').classList.toggle('active',!!state.playing);$('play').setAttribute('aria-pressed',String(!!state.playing));
 $('record').classList.toggle('active',!!state.recording);$('record').setAttribute('aria-pressed',String(!!state.recording));
 $('record').textContent=state.recording?'Recording hits':'Record hits';
 $('motion').classList.toggle('active',state.motion);$('motion').setAttribute('aria-pressed',String(state.motion));
 $('motion').textContent=state.motion?'Finish motion':'Record motion';
 const touched=params.filter((_,p)=>state.motionMask[voice]&(1<<p)).map(id=>id.startsWith('env-')?environmentLabels[voice][Number(id.slice(-1))-4]:id);
 $('motion-help').textContent=state.motion?`${state.playing?'RECORDING':'ARMED'} · ${names[voice]} · ${!v.steps.some(s=>s.on)?'No hits in this voice yet. Finish motion and plant some steps first.':touched.length?'Capturing '+touched.join(', ')+' on existing hits.':'Move a sound slider to begin capturing.'} Finish motion or Stop to hear the recorded locks. Undo restores the whole take.`:
 'Motion overdub: arm Record motion, play your rhythm, then move a sound slider. Each existing hit captures that value as a step lock.';
 $('lock-mode').disabled=state.motion;$('clear-pattern').disabled=state.motion;
 for(const id of ['swing','human'])$(id).disabled=state.motion;
 $('undo').disabled=!state.undo;$('clear-lock').disabled=!lock||!st.mask;
 if(document.activeElement!==$('tempo'))$('tempo').value=state.tempo;
 $('rejoin').disabled=state.motion||!state.voices.some(v=>v.takeover);
 for(const b of document.querySelectorAll('[data-step]')){
  const s=Number(b.dataset.step),item=v.steps[s];b.disabled=state.motion;
  b.querySelector('.note-label').textContent=voice===2&&item.on?noteName(item.note):'';
  b.classList.toggle('active',item.on);b.classList.toggle('playing',state.playing&&state.step===s);
  b.classList.toggle('selected',(lock||voice===2)&&selected===s);b.classList.toggle('locked',!!item.mask);
  b.setAttribute('aria-pressed',String(item.on));
  b.setAttribute('aria-label',`${names[voice]} step ${s+1}, ${item.on?'on':'off'}${voice===2&&item.on?', '+noteName(item.note):''}${item.mask?', parameter locks':''}${lock&&selected===s?', selected for editing':''}`);
 }
 for(const [p,id] of params.entries())if(dragging!==id){
  const val=lock&&(st.mask&(1<<p))?st.lock[p]:lock?v.base[p]:state.motion||state.playing?v.effective[p]:v.base[p];$(id).value=val;$(`${id}-value`).textContent=Math.round(val*100);
 }
 $('edit-status').textContent=state.motion?`MOTION · ${names[voice]} · sound sliders overdub locks on existing hits; other parameters stay intact.`:lock?`LOCK EDIT · ${names[voice]} step ${selected+1} · ${st.on?(state.playing?'changes sound when this step plays':'press Play loop to hear your edits'):'no hit here — use Enable hit to hear it'}`:
 v.takeover?'LIVE · your changed knobs override locks. Rejoin restores lock priority.':
 state.playing&&v.steps[state.step]?.mask?'LOCK · stored step values are sounding.':'BASE · knobs shape this voice. Enable Edit locks to shape a single step.';
}
const rootKeys={KeyQ:36,KeyW:38,KeyE:40,KeyR:41,KeyT:43,KeyY:45,KeyU:47,KeyI:48};
const heldKeys=new Set();
for(const b of document.querySelectorAll('[data-note]')){
 b.onpointerdown=e=>{if(b.disabled)return;e.preventDefault();b.setPointerCapture(e.pointerId);setVoice(2);command('key',Number(b.dataset.note),1);};
 const release=()=>command('key',Number(b.dataset.note),0);
 b.onpointerup=release;b.onpointercancel=release;b.onlostpointercapture=release;
 b.onclick=e=>{if(e.detail===0){setVoice(2);command('key',Number(b.dataset.note),1);setTimeout(release,180);}};
}
const noteEdit=()=>command('note',selected,Number($('root-note').value),Number($('root-gate').value),$('root-slide').checked?1:0);
$('root-on').onchange=()=>{command('begin');command('step',2,selected,$('root-on').checked?1:0);};
for(const id of ['root-note','root-slide'])$(id).onchange=()=>{command('begin');noteEdit();};
$('root-gate').oninput=()=>{if(dragging!=='root-gate'){command('begin');dragging='root-gate';}noteEdit();};
$('root-gate').onchange=()=>dragging=null;
document.addEventListener('keyup',e=>{if(heldKeys.has(e.code)){command('key',rootKeys[e.code],0);heldKeys.delete(e.code);}});
window.addEventListener('blur',()=>{for(const key of heldKeys)command('key',rootKeys[key],0);heldKeys.clear();});
document.addEventListener('keydown',e=>{
 if(e.code==='Escape'&&node){e.preventDefault();command('panic');return;}
 if(e.repeat||e.ctrlKey||e.metaKey||e.altKey||['INPUT','TEXTAREA','SELECT'].includes(e.target.tagName))return;
 if(e.code==='Digit3'){setVoice(2);return;}if(e.code==='Digit1'){setVoice(0);return;}if(e.code==='Digit2'){setVoice(1);return;}
 if(!node)return;
 if(rootKeys[e.code]){e.preventDefault();heldKeys.add(e.code);setVoice(2);command('key',rootKeys[e.code],1);return;}
 if(e.code==='Space'&&e.target.tagName!=='BUTTON'){e.preventDefault();command('play',-1);}
 if(e.code==='KeyA'||e.code==='KeyZ')hit(0);
 if(e.code==='KeyS'||e.code==='KeyX')hit(1);

});
document.addEventListener('visibilitychange',()=>{if(document.hidden)command('panic');});
window.addEventListener('pagehide',()=>{command('panic');context?.suspend();});
paint();

// Device-shell bridge: the existing worklet remains the sole audio/sequencing owner.
window.UG={command,enableAudio,setVoice,paint,get state(){return state;},get voice(){return voice;},get selected(){return selected;},selectStep(s){selected=s;paint();}};
