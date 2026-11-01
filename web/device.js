// Original UNDERGROWTH panel adapter. Navigation, editing and screen text live in shared C.
import "./fm1-panel.js";
{
const lcd=document.getElementById('fm1-screen'),ctx=lcd.getContext('2d');
const buttons=['FX','SEL','ENV','LFO','EDIT','GLO','HOME','SAVE','ARP','SEQ','PLAY','REC','OCT-','OCT+'];
const encoders=['SELECT','ALGORITHM','PRESETS','KNOB1','KNOB2','KNOB3','KNOB4'];
let master=.65,last='';const ug=()=>window.UG,send=(type,...args)=>ug()?.command(type,...args);
const hardware=window.createFM1Panel({
 begin(kind){send('uiGesture',kind==='drag'?1:0);},end(){send('uiGesture',1);},
 turn(name,delta){if(name==='MASTER'){master=Math.max(0,Math.min(1,master+delta*.01));send('master',master);}else send('uiTurn',encoders.indexOf(name),delta);},
 button(name){send('uiButton',buttons.indexOf(name));},panic(){send('panic');},
 key(offset,down){send('uiKey',offset,+down,-1);}
});
window.addEventListener('blur',()=>{for(let k=0;k<27;k++)send('uiKey',k,0,-1);});
function draw(){const s=ug()?.state,p=s?.panel;const signature=JSON.stringify([p,s?.step,s?.playing,s?.voices.map(v=>v.steps.map(x=>x.on))]);if(signature===last)return;last=signature;
 const lines=p?.lines||['UNDERGROWTH','Enable audio above','Empty until you play','','','','','','SILENT'];
 ctx.fillStyle='#b8d0bd';ctx.fillRect(0,0,240,240);ctx.fillStyle='#142e23';ctx.font='bold 16px monospace';ctx.fillText(lines[0],8,24);ctx.fillRect(8,32,224,2);ctx.font='13px monospace';for(let i=1;i<=5;i++)ctx.fillText(lines[i],8,54+(i-1)*23);ctx.font='11px monospace';ctx.fillText(lines[6],8,181);ctx.fillText(lines[7],8,195);ctx.font='bold 11px monospace';ctx.fillText(lines[8],8,209);
 const v=p?.voice||0;for(let i=0;i<16;i++){ctx.fillStyle=s?.voices[v].steps[i].on?'#254e37':'#91ab97';ctx.fillRect(8+i*14,220,11,12);if(i===p?.selected||s?.playing&&s.step===i){ctx.strokeStyle=s?.playing&&s.step===i?'#fff':'#142e23';ctx.strokeRect(8+i*14,220,11,12);}}
 const readout=lines.filter(Boolean).join(' · ');document.getElementById('panel-readout').textContent=readout;lcd.setAttribute('aria-label',readout);
 for(const [name,b] of Object.entries(hardware.buttons)){const on=!!(p?.lights&(1<<buttons.indexOf(name)));b.g.classList.toggle('lit',on);b.hit.setAttribute('aria-pressed',String(on));}
 hardware.keys.forEach((k,i)=>{const note=29+i+(p?.octave||0)*12,outside=v===2&&p?.page!==9&&(note<24||note>60);k.g.style.opacity=outside?'.35':'1';k.hit.setAttribute('aria-disabled',String(outside));});
 hardware.whiteKeys.forEach((key,i)=>{hardware.keys[key].g.classList.toggle('lit',!!s?.voices[v].steps[i].on);hardware.keys[key].g.classList.toggle('held',!!s?.playing&&s.step===i);});
 document.getElementById('device-status').textContent=document.getElementById('audio-status').textContent;document.getElementById('device-audio').textContent=document.getElementById('enable-audio').textContent;
}
document.getElementById('device-audio').onclick=async()=>{await ug().enableAudio();last='';draw();};
document.getElementById('view-toggle').onclick=()=>{const studio=document.querySelector('main.instrument'),device=document.getElementById('device-view');studio.hidden=!studio.hidden;device.hidden=!device.hidden;document.getElementById('view-toggle').textContent=studio.hidden?'Expanded view':'Device view';};
setInterval(draw,50);draw();
}
