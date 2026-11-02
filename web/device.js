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
const grainView=document.createElement('canvas');grainView.width=640;grainView.height=110;grainView.style.cssText='width:100%;max-width:640px;display:block;margin:12px auto';grainView.setAttribute('role','img');grainView.setAttribute('aria-label','ROOT atmosphere sample, scan position and active grains');document.getElementById('panel-readout').after(grainView);
window.addEventListener('blur' ,()=>{for(let k=0;k<27;k++)send('uiKey',k,-1,-1);});
function draw(){const s=ug()?.state,p=s?.panel;const signature=JSON.stringify([p,s?.step,s?.playing,s?.voices.map(v=>v.steps.map(x=>x.on))]);if(signature===last)return;last=signature;
 const lines=p?.lines||['UNDERGROWTH','Enable audio above','Empty until you play','','','','','','SILENT'];
 ctx.fillStyle='#b8d0bd';ctx.fillRect(0,0,240,240);ctx.fillStyle='#142e23';ctx.font='bold 16px monospace';ctx.fillText(lines[0],8,24);ctx.fillRect(8,32,224,2);ctx.font='13px monospace';for(let i=1;i<=5;i++)ctx.fillText(lines[i],8,54+(i-1)*23);ctx.font='11px monospace';ctx.fillText(lines[6],8,181);ctx.fillText(lines[7],8,195);ctx.font='bold 11px monospace';ctx.fillText(lines[8],8,209);
 const v=p?.voice||0;for(let i=0;i<16;i++){ctx.fillStyle=s?.voices[v].steps[i].on?'#254e37':'#91ab97';ctx.fillRect(8+i*14,216,11,7);if(i===p?.selected||s?.playing&&s.step===i){ctx.strokeStyle=s?.playing&&s.step===i?'#fff':'#142e23';ctx.strokeRect(8+i*14,216,11,7);}}
 grainView.hidden=p?.voice!==2||p?.page!==2;
 if(!grainView.hidden&&p?.grains){const g=p.grains,c=grainView.getContext('2d');c.fillStyle='#142e23';c.fillRect(0,0,640,110);c.strokeStyle='#91ab97';c.beginPath();for(let i=0;i<64;i++){const x=10+i*620/63,y=53-g[10+i]*65;i?c.lineTo(x,y):c.moveTo(x,y);}c.stroke();c.strokeStyle='#e1bd6d';c.beginPath();c.moveTo(10+g[0]*620,8);c.lineTo(10+g[0]*620,90);c.stroke();c.fillStyle='#b8d0bd';for(let i=2;i<10;i++)if(g[i]>=0)c.fillRect(10+g[i]*620,75,4,12);c.font='12px monospace';c.fillText('ROOT / woven texture / '+g[1]+' of 8 grains',10,103);}
 const meter=p?.meter||'CPU -- / FM1 only';ctx.fillStyle='#142e23';ctx.font='10px monospace';ctx.fillText(meter,8,237);
 const readout=lines.filter(Boolean).join(' · ')+' · '+meter;document.getElementById('panel-readout').textContent=readout;lcd.setAttribute('aria-label',readout);
 for(const [name,b] of Object.entries(hardware.buttons)){const on=!!(p?.lights&(1<<buttons.indexOf(name)));b.g.classList.toggle('lit',on);b.hit.setAttribute('aria-pressed',String(on));}
 hardware.keys.forEach((k,i)=>{const label=p?.keyLabels?.[i]||'',outside=label==='OUT';k.g.style.opacity=outside?'.35':'1';k.hit.setAttribute('aria-disabled',String(outside));k.hit.setAttribute('aria-description',label);k.hit.title=label;k.caption.textContent=label;k.g.classList.toggle('lit',!!(p?.keyLights&(1<<i)));k.g.classList.toggle('latched',!!(p?.keyPlayhead&(1<<i)));k.hit.setAttribute('aria-pressed',String(!!(p?.keyLights&(1<<i))));});
 for(let i=0;i<7;i++){const k=hardware.knobs[encoders[i]],label=p?.knobLabels?.[i]||encoders[i];k.hit.setAttribute('aria-description',label);k.hit.title=label;}
 document.getElementById('panel-task').textContent=lines[0];document.getElementById('panel-context').textContent=[lines[1],lines[6],lines[7]].filter(Boolean).join(' · ');
 document.getElementById('panel-transport').textContent=lines[8];
 document.getElementById('device-status').textContent=document.getElementById('audio-status').textContent;document.getElementById('device-audio').textContent=document.getElementById('enable-audio').textContent;
}
document.getElementById('device-audio').onclick=async()=>{await ug().enableAudio();last='';draw();};
document.getElementById('view-toggle').onclick=()=>{const studio=document.querySelector('main.instrument'),device=document.getElementById('device-view');studio.hidden=!studio.hidden;device.hidden=!device.hidden;document.getElementById('view-toggle').textContent=studio.hidden?'Expanded view':'Device view';};
setInterval(draw,50);draw();
}
