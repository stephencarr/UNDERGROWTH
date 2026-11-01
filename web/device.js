// Original UNDERGROWTH menu/control mapping on the fm1-base MPL-2.0 panel.
import "./fm1-panel.js";
{
const panel=document.getElementById('fm1-panel'),lcd=document.getElementById('fm1-screen'),ctx=lcd.getContext('2d');
const pages=['SOUND','SEQUENCE','LOCKS','GROOVE','MOTION','FX','RECOVERY','SAVE'];
let page=0,octave=0,recovery=0,master=.65,gesture=null,gestureValue=0,lastDraw='';
const ug=()=>window.UG;
const clampPanel=(x,a,b)=>Math.max(a,Math.min(b,x));
function send(type,...args){ug().command(type,...args);}
function setPage(p){page=p;gesture=null;draw();}
function voiceNext(delta){ug().setVoice((ug().voice+delta+3)%3);draw();}
function editStart(key){if(gesture!==key){send('begin');gesture=key;}}
function macro(p,delta){const u=ug(),s=u.state;if(!s)return;const v=u.voice,st=s.voices[v].steps[u.selected];
 const key=(page===2?'lock':'macro')+p,first=gesture!==key;
 if(first){gesture=key;gestureValue=page===2&&(st.mask&(1<<p))?st.lock[p]:s.voices[v].effective[p];if(page===2)send('begin');}
 gestureValue=clampPanel(gestureValue+delta*.01,0,1);
 if(page===2)send('lock',v,u.selected,p,gestureValue);else send(first?'editparam':'param',v,p,gestureValue);
}
function noteEdit(p,delta){const u=ug(),s=u.state;if(!s||s.motion)return;const st=s.voices[u.voice].steps[u.selected];editStart('note'+p);
 if(u.voice!==2){if(p===0)send('step',u.voice,u.selected,delta>0?1:0);return;}
 if(p===0)send('step',2,u.selected,delta>0?1:0);
 else send('note',u.selected,clampPanel(st.note+(p===1?delta:0),24,60),clampPanel(st.gate+(p===2?delta*.05:0),.1,1.5),p===3?(delta>0?1:0):+st.slide);
}
function turn(name,delta){if(!delta)return;const u=ug(),s=u.state;
 if(name==='SELECT'){u.selectStep((u.selected+delta%16+16)%16);draw();return;}
 if(name==='PRESETS'){voiceNext(Math.sign(delta));return;}
 if(name==='ALGORITHM'){setPage((page+Math.sign(delta)+pages.length)%pages.length);return;}
 if(!s)return;
 if(name==='MASTER'){master=clampPanel(master+delta*.01,0,1);send('master',master);draw();return;}
 const p=Number(name.slice(-1))-1;
 if(page===1)noteEdit(p,delta);
 else if(page===3){if(s.motion)return;editStart('global'+p);if(p===0)send('tempo',s.tempo+delta);else if(p<3)send('groove',p-1,clampPanel(s.groove[p-1]+delta*.01,0,1));}
 else if(page===6){recovery=delta>0?1:0;draw();}
 else if(page===5){if(p===0)macro(3,delta);}
 else if(page===0||page===2||page===4)macro(p,delta);
}
function press(name){const u=ug(),s=u.state;gesture=null;
 if(name==='HOME')setPage(0);
 else if(name==='SEL'){if(page===6&&s){send(recovery?'clear':'undo');recovery=0;}else voiceNext(1);}
 else if(name==='PLAY STOP')send('play',-1);
 else if(name==='REC'){if(page===4){send('motion',-1);}else send('record',-1);}
 else if(name==='OCT-')octave=clampPanel(octave-1,-1,1);
 else if(name==='OCT+')octave=clampPanel(octave+1,-1,1);
 else setPage(({FX:5,ENV:1,LFO:4,EDIT:2,GLO:3,SAVE:7,ARP:6,SEQ:1})[name]??0);
 if(name==='EDIT')send('rejoin');draw();
}
const heldPanel=new Map();
const hardware=window.createFM1Panel({begin(){gesture=null;},end(){gesture=null;},turn,button(name){press(name==='PLAY'?'PLAY STOP':name);},panic(){send('panic');},key(offset,down,index){
 const u=ug(),s=u.state;if(!down){if(heldPanel.has(offset)){send('key',heldPanel.get(offset),0);heldPanel.delete(offset);}return;}if(!s)return;
 if((page===1||page===2)&&index>=0){u.selectStep(index);if(page===1&&!s.motion&&u.voice!==2){send('begin');send('toggle',u.voice,index);}draw();return;}
 if(u.voice===2){const note=29+offset+octave*12;if(note<24||note>60)return;heldPanel.set(offset,note);send('key',note,1);}else send('hit',u.voice,1);
}});
window.addEventListener('blur',()=>{for(const note of heldPanel.values())send('key',note,0);heldPanel.clear();});
function draw(){const u=ug();if(!u)return;const s=u.state,v=u.voice,st=s?.voices[v].steps[u.selected];const title=['KICK','CHATTER','ROOT'][v];
 let lines=[];
 if(!s)lines=['Enable audio above','Empty until you play'];
 else if(page===1)lines=[`STEP ${u.selected+1}  ${st.on?'ON':'REST'}`,`1 Hit: ${st.on?'ON':'OFF'}`,`2 Note: ${['C','C#','D','D#','E','F','F#','G','G#','A','A#','B'][st.note%12]}${Math.floor(st.note/12)-1}`,`3 Gate: ${Math.round(st.gate*100)}%`,`4 Slide: ${st.slide?'ON':'OFF'}`];
 else if(page===3)lines=[`1 Tempo: ${s.tempo}`,`2 Swing: ${(50+s.groove[0]*100/6).toFixed(1)}%`,`3 Human: ${Math.round(s.groove[1]*100)}%`];
 else if(page===6)lines=[(recovery===0?'> ':'  ')+'Undo / redo',(recovery===1?'> ':'  ')+'Clear all lanes','KNOB1 choose · SEL apply'];
 else if(page===7)lines=['Save not implemented','Session only','Reload clears your work'];
 else if(page===5)lines=[`1 Send: ${Math.round(s.voices[v].effective[3]*100)}%`,'Dark 1/8-note delay','Feedback fixed at 52%'];
 else{const labels=['Body','Edge','Length','Send'];lines=labels.map((label,p)=>`${p+1} ${label}: ${Math.round((page===2&&(st.mask&(1<<p))?st.lock[p]:s.voices[v].effective[p])*100)}${page===2&&(st.mask&(1<<p))?' *':''}`);if(page===2)lines.unshift(`STEP ${u.selected+1} ${st.on?'ON':'REST'}`);if(page===4)lines.unshift(s.motion?'REC MOTION · REC finish':'REC to arm motion');}
 const footer=s?(s.motion?'MOTION REC':s.recording?'HIT REC':s.playing?'PLAY':'STOP')+`  ${s.tempo} BPM`:'SILENT';
 const signature=JSON.stringify([page,title,lines,footer,s?.step,octave]);if(signature===lastDraw)return;lastDraw=signature;
 ctx.fillStyle='#b8d0bd';ctx.fillRect(0,0,240,240);ctx.fillStyle='#142e23';ctx.font='bold 17px monospace';ctx.fillText(title+' / '+pages[page],8,24);ctx.fillRect(8,32,224,2);ctx.font='13px monospace';lines.forEach((l,i)=>ctx.fillText(l,8,56+i*23));ctx.font='bold 12px monospace';ctx.fillText(footer,8,205);
 for(let i=0;i<16;i++){ctx.fillStyle=s?.voices[v].steps[i].on?'#254e37':'#91ab97';ctx.fillRect(8+i*14,218,11,12);if(s?.playing&&s.step===i){ctx.strokeStyle='#fff';ctx.strokeRect(8+i*14,218,11,12);}}
 document.getElementById('panel-readout').textContent=title+' · '+pages[page]+' · '+lines.join(' · ');lcd.setAttribute('aria-label',title+' '+pages[page]+': '+lines.join(', '));
 hardware.keys.forEach((k,i)=>{const outside=v===2&&page!==1&&page!==2&&(29+i+octave*12<24||29+i+octave*12>60);k.g.style.opacity=outside?'.35':'1';k.hit.setAttribute('aria-disabled',String(outside));});
 hardware.whiteKeys.forEach((key,i)=>{hardware.keys[key].g.classList.toggle('lit',!!s?.voices[v].steps[i].on);hardware.keys[key].g.classList.toggle('held',!!s?.playing&&s.step===i);});
 document.getElementById('device-status').textContent=document.getElementById('audio-status').textContent;
 document.getElementById('device-audio').textContent=document.getElementById('enable-audio').textContent;
 hardware.knobs.MASTER.hit.setAttribute('aria-valuenow',Math.round(master*100));
}
document.getElementById('device-audio').onclick=async()=>{await ug().enableAudio();draw();};
document.getElementById('view-toggle').onclick=()=>{const studio=document.querySelector('main.instrument'),device=document.getElementById('device-view');studio.hidden=!studio.hidden;device.hidden=!device.hidden;document.getElementById('view-toggle').textContent=studio.hidden?'Expanded view':'Device view';};
setInterval(draw,50);draw();

}
