// Original UNDERGROWTH controller on the fm1-base MPL-2.0 panel.
import "./fm1-panel.js";
{
const lcd=document.getElementById('fm1-screen'),ctx=lcd.getContext('2d');
const titles=['KICK','CHATTER','ROOT'],modes=['','Hits','Motion','Both'];
let page='HOME',sub=0,octave=0,action=0,master=.65,gesture=null,gestureValue=0,lastDraw='',heldStep=-1,heldEdited=false,lastTurn=0;
const envMaps=[[2,4,5,6],[4,5,6,2],[2,4,5,6]],envLabels=[['Length','Damping','Drop','Response'],['Stretch','Fracture','Grain','Length'],['Release','Glide','Weight','Response']];
const noteLabel=n=>['C','C#','D','D#','E','F','F#','G','G#','A','A#','B'][n%12]+(Math.floor(n/12)-1);
const pageCount=()=>page==='SEQ'?4:['HOME','FX','ENV'].includes(page)?2:1;
const ug=()=>window.UG, clampPanel=(x,a,b)=>Math.max(a,Math.min(b,x));
function send(type,...args){ug().command(type,...args);}
function setPage(p){page=p;sub=0;action=0;gesture=null;draw();}
function macro(p,delta){const u=ug(),s=u.state;if(!s||s.armed&&s.recordTrack!==u.voice)return;
 const target=heldStep>=0?heldStep:page==='SEQ'&&sub===3?u.selected:-1;
 const lock=target>=0&&!s.armed&&!s.motion&&!s.recording,step=s.voices[u.voice].steps[target],key=`${lock?target:'draft'}:${p}`;
 const first=gesture!==key;
 if(first){gesture=key;gestureValue=lock&&(step.mask&(1<<p))?step.lock[p]:s.motion?s.voices[u.voice].effective[p]:s.voices[u.voice].base[p];if(lock&&(heldStep<0||!heldEdited)){send('begin');if(heldStep>=0)heldEdited=true;}}
 gestureValue=clampPanel(gestureValue+delta*.01,0,1);
 if(lock)send('lock',u.voice,target,p,gestureValue);else send(first?'editparam':'param',u.voice,p,gestureValue);
}
function turn(name,delta){if(!delta)return;const u=ug(),s=u.state;
 if(name==='ALGORITHM'){heldStep=-1;heldEdited=false;u.setVoice((u.voice+Math.sign(delta)+3)%3);gesture=null;draw();return;}
 if(name==='SELECT'){const count=pageCount();sub=(sub+Math.sign(delta)+count)%count;gesture=null;draw();return;}
 if(name==='PRESETS'){setPage('PRESETS');return;}
 if(!s)return;
 if(name==='MASTER'){master=clampPanel(master+delta*.01,0,1);send('master',master);draw();return;}
 const p=Number(name.slice(-1))-1;
 if(page==='REC'){if(p===0)send('recmode',clampPanel(s.recordMode+Math.sign(delta),1,3));return;}
 if(page==='HOME'&&sub===1||page==='SEQ'&&sub===1){action=(action+Math.sign(delta)+3)%3;draw();return;}
 if(page==='ENV'&&sub===0||page==='SEQ'&&sub===3){macro(envMaps[u.voice][p],delta);return;}
 if(page==='ENV'&&sub===1||page==='SEQ'&&sub===2){
  if(s.armed||s.motion||s.recording)return;
  const target=page==='SEQ'?u.selected:-1,st=target>=0?s.voices[u.voice].steps[target]:null;
  const velocityControl=u.voice===2?p===3:p===0;
  if(velocityControl){const key='velocity'+target,first=gesture!==key;if(first){gesture=key;gestureValue=st?st.velocity:s.voices[u.voice].velocity;}gestureValue=clampPanel(gestureValue+delta*.01,0,1);send(first?'editvelocity':'velocity',u.voice,target,gestureValue);return;}
  if(u.voice!==2||p>2)return;
  const key='note'+target+':'+p,first=gesture!==key,values=st?[st.note,st.gate,+st.slide]:[...s.draftNote];
  if(first){gesture=key;gestureValue=values[p];send('begin');}
  gestureValue=clampPanel(gestureValue+delta*(p===1?.05:1),p===0?24:p===1?.1:0,p===0?60:p===1?1.5:1);
  values[p]=gestureValue;send(st?'note':'draftnote',...(st?[target,...values]:values));return;
 }
 if(page==='GLO'){if(s.armed||s.motion)return;if(gesture!=='global'+p){send('begin');gesture='global'+p;gestureValue=p===0?s.tempo:s.groove[p-1];}if(p>2)return;gestureValue+=delta*(p===0?1:.01);gestureValue=clampPanel(gestureValue,p===0?40:0,p===0?220:1);send(p===0?'tempo':'groove',...(p===0?[gestureValue]:[p-1,gestureValue]));return;}
 if(page==='FX'){if(sub===0&&p===0)macro(3,delta);return;}
 if(['HOME','EDIT','SEQ'].includes(page))macro(p,delta);
}
function press(name){const u=ug(),s=u.state;gesture=null;
 if(name==='PLAY')send('play',-1);
 else if(name==='REC'){setPage('REC');if(s)send('arm',-1,u.voice);}
 else if(name==='SEL'){
  if(page==='ENV'&&sub===1||page==='SEQ'&&sub===2){
   if(s&&!s.armed&&!s.motion&&!s.recording){const st=page==='SEQ'?s.voices[u.voice].steps[u.selected]:null;
    if(u.voice===2){const n=st?st.note:s.draftNote[0];send('keyvelocity',n,1,st?st.velocity:s.voices[2].velocity);setTimeout(()=>send('key',n,0),180);}
    else send('hit',u.voice,st?st.velocity:-1);
   }
  }
  else if(page==='REC'){if(s)send('arm',-1,u.voice);}
  else if(page==='SEQ'&&sub===1){if(s&&!s.armed&&!s.motion&&!s.recording)send(['stamp','erase','undo'][action],...(action===2?[]:[u.voice,u.selected]));}
  else if(page==='HOME'&&sub===1)send(['undo','rejoin','panic'][action]);
  else{u.setVoice((u.voice+1)%3);}
 }
 else if(name==='OCT-')octave=clampPanel(octave-1,-1,1);
 else if(name==='OCT+')octave=clampPanel(octave+1,-1,1);
 else setPage(name);draw();
}
const heldPanel=new Map();
const hardware=window.createFM1Panel({begin(kind){const now=performance.now();if(kind==='drag'||now-lastTurn>300)gesture=null;lastTurn=now;},end(){gesture=null;},turn,button:press,panic(){send('panic');},key(offset,down,index){
 const u=ug(),s=u.state;
 if(!down){if(index===heldStep){heldStep=-1;heldEdited=false;}if(heldPanel.has(offset)){send('key',heldPanel.get(offset),0);heldPanel.delete(offset);}gesture=null;return;}if(!s)return;
 if(page==='SEQ'&&index>=0&&!(s.armed&&s.recording)){
  u.selectStep(index);heldStep=index;heldEdited=false;
  if(sub===0&&!s.armed&&!s.recording&&!s.motion&&!s.voices[u.voice].steps[index].on){send('stamp',u.voice,index);heldEdited=true;}
  draw();return;
 }
 if(s.armed&&s.recordTrack!==u.voice)return;
 if(u.voice===2){const note=29+offset+octave*12;if(note<24||note>60)return;if(page==='ENV'&&sub===1&&!s.armed&&!s.motion&&!s.recording){send('begin');send('draftnote',note,s.draftNote[1],s.draftNote[2]);}heldPanel.set(offset,note);send('key',note,1);}else send('hit',u.voice,-1);
}});
window.addEventListener('blur',()=>{for(const note of heldPanel.values())send('key',note,0);heldPanel.clear();heldStep=-1;});
function draw(){const u=ug();if(!u)return;const s=u.state,v=u.voice,st=s?.voices[v].steps[u.selected],title=titles[v];let lines=[];
 if(!s)lines=['Enable audio above','Empty until you play'];
 else if(page==='REC')lines=[`1 Mode: ${modes[s.recordMode]}`,s.armed?`Armed: ${titles[s.recordTrack]}`:'Disarmed',s.playing?'Transport running':'PLAY to start',s.armed?'REC / SEL: finish':'REC / SEL: arm','Mode change finishes take'];
 else if(page==='ENV'&&sub===1||page==='SEQ'&&sub===2){const target=page==='SEQ'?st:null;
  const n=target?target.note:s.draftNote[0],gate=target?target.gate:s.draftNote[1],slide=target?target.slide:s.draftNote[2],velocity=target?target.velocity:s.voices[v].velocity;
  lines=[target?`STEP ${u.selected+1} ${target.on?'ON':'REST'}`:'DRAFT · SEL audition',...(v===2?[`1 Pitch: ${noteLabel(n)}`,`2 Gate: ${Math.round(gate*100)}%`,`3 Slide: ${slide?'ON':'OFF'}`,`4 Velocity: ${Math.round(velocity*100)}%`]:[`1 Velocity: ${Math.round(velocity*100)}%`,'SEL: audition hit'])];
 }
 else if(page==='ENV'&&sub===0||page==='SEQ'&&sub===3){const target=heldStep>=0?heldStep:page==='SEQ'?u.selected:-1,step=target>=0?s.voices[v].steps[target]:null;
  lines=[target>=0?`ENV LOCK · STEP ${target+1}`:s.motion?'ENV · MOTION':'ENV · TRACK DRAFT',...envMaps[v].map((p,i)=>`${i+1} ${envLabels[v][i]}: ${Math.round((step&&(step.mask&(1<<p))?step.lock[p]:s.motion?s.voices[v].effective[p]:s.voices[v].base[p])*100)}%`)];
 }
 else if(page==='GLO')lines=[`1 Tempo: ${s.tempo}`,`2 Swing: ${(50+s.groove[0]*100/6).toFixed(1)}%`,`3 Human: ${Math.round(s.groove[1]*100)}%`];
 else if(page==='SEQ'&&sub===1)lines=[`STEP ${u.selected+1} ${st.on?'ON':'REST'}`,['Stamp draft','Erase step','Undo / redo'].map((x,i)=>(i===action?'> ':'  ')+x).join('|'),'KNOB1 choose; SEL apply'].flatMap(x=>x.split('|'));
 else if(page==='HOME'&&sub===1)lines=['Recovery',...['Undo / redo','Rejoin locks','Panic'].map((x,i)=>(i===action?'> ':'  ')+x),'KNOB1 choose; SEL apply'];
 else if(page==='FX')lines=sub===0?[`TRACK ${title}`,`1 Send: ${Math.round((s.motion&&(!s.armed||s.recordTrack===v)?s.voices[v].effective[3]:s.voices[v].base[3])*100)}%`,'Motion / step locks apply','SELECT: global bus']:['GLOBAL DELAY','Dark 1/8-note delay','Feedback fixed at 52%','Bus automation: next slice'];
 else if(['HOME','EDIT','SEQ'].includes(page))lines=[heldStep>=0?`LOCK STEP ${heldStep+1}`:s.motion?'MOTION VALUES':page==='SEQ'?`STEP ${u.selected+1} ${st.on?'ON':'REST'} · DRAFT`:'DRAFT SOUND',...['Body','Edge','Length','Send'].map((label,p)=>`${p+1} ${label}: ${Math.round((heldStep>=0&&(st.mask&(1<<p))?st.lock[p]:s.motion&&(!s.armed||s.recordTrack===v)?s.voices[v].effective[p]:s.voices[v].base[p])*100)}`)];
 else lines=[`${page} not implemented`,page==='SAVE'?'Reload clears this session':'Reserved hardware page'];
 const role=(page==='ENV'&&sub===1||page==='SEQ'&&sub===2)?(s?.armed?'Finish REC to edit notes':'SEL: audition · no record'):page==='SEQ'&&!(s?.armed&&s?.recording)?(s?.armed||sub>0?'Keys: select steps':'Keys: stamp / hold-lock'):'Keys: live instrument';
 const footer=s?`${s.playing?'PLAY':'STOP'} ${s.tempo} BPM`+(s.armed?` REC ${modes[s.recordMode]} ${titles[s.recordTrack]}`:''):'SILENT';
 const signature=JSON.stringify([page,sub,title,lines,footer,s?.step,octave,u.selected,role]);if(signature===lastDraw)return;lastDraw=signature;
 ctx.fillStyle='#b8d0bd';ctx.fillRect(0,0,240,240);ctx.fillStyle='#142e23';ctx.font='bold 16px monospace';ctx.fillText(title+' / '+(page==='SEQ'?'SEQUENCE':page),8,24);ctx.fillRect(8,32,224,2);ctx.font='13px monospace';lines.forEach((l,i)=>ctx.fillText(l,8,54+i*23));ctx.font='11px monospace';ctx.fillText(role,8,181);ctx.fillText(pageCount()>1?'SELECT '+(sub+1)+'/'+pageCount()+' '+(page==='SEQ'?['Sound','Actions','Notes / vel','ENV locks'][sub]:page==='ENV'?['Environment','Draft notes / vel'][sub]:page==='HOME'?['Sound','Recovery'][sub]:['Track','Global'][sub]):'ALGORITHM: track',8,195);ctx.font='bold 11px monospace';ctx.fillText(footer,8,209);
 for(let i=0;i<16;i++){ctx.fillStyle=s?.voices[v].steps[i].on?'#254e37':'#91ab97';ctx.fillRect(8+i*14,220,11,12);if(i===u.selected||s?.playing&&s.step===i){ctx.strokeStyle=s?.playing&&s.step===i?'#fff':'#142e23';ctx.strokeRect(8+i*14,220,11,12);}}
 document.getElementById('panel-readout').textContent=title+' · '+(page==='SEQ'?'SEQUENCE':page)+' · '+lines.join(' · ')+' · '+role;
 lcd.setAttribute('aria-label',document.getElementById('panel-readout').textContent);
 for(const [name,b] of Object.entries(hardware.buttons)){const on=(name===page&&name!=='REC')||(name==='PLAY'&&s?.playing)||(name==='REC'&&s?.armed);b.g.classList.toggle('lit',!!on);b.hit.setAttribute('aria-pressed',String(!!on));}
 hardware.keys.forEach((k,i)=>{const outside=v===2&&page!=='SEQ'&&(29+i+octave*12<24||29+i+octave*12>60);k.g.style.opacity=outside?'.35':'1';k.hit.setAttribute('aria-disabled',String(outside));});
 hardware.whiteKeys.forEach((key,i)=>{hardware.keys[key].g.classList.toggle('lit',!!s?.voices[v].steps[i].on);hardware.keys[key].g.classList.toggle('held',!!s?.playing&&s.step===i);});
 document.getElementById('device-status').textContent=document.getElementById('audio-status').textContent;
 document.getElementById('device-audio').textContent=document.getElementById('enable-audio').textContent;
 hardware.knobs.MASTER.hit.setAttribute('aria-valuenow',Math.round(master*100));
}
document.getElementById('device-audio').onclick=async()=>{await ug().enableAudio();draw();};
document.getElementById('view-toggle').onclick=()=>{const studio=document.querySelector('main.instrument'),device=document.getElementById('device-view');studio.hidden=!studio.hidden;device.hidden=!device.hidden;document.getElementById('view-toggle').textContent=studio.hidden?'Expanded view':'Device view';};
setInterval(draw,50);draw();

}
