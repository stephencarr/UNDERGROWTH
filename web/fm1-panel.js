// SPDX-License-Identifier: MPL-2.0
// Copyright (C) 2026 Charles Vestal
// Adapted for UNDERGROWTH from fm1-base web/emu/index.html at 02f422c7d9de121ca099bb2fdebe5c035f59e8ad
// Changes: extracted panel, application callbacks, no firmware-specific transport.
window.createFM1Panel=function(callbacks){
const NS = "http://www.w3.org/2000/svg";
const BTN = { FX: 0, SEL: 1, ENV: 2, LFO: 3, EDIT: 4, GLO: 5, HOME: 6, SAVE: 7, ARP: 8, SEQ: 9, PLAY: 10, REC: 11, "OCT-": 12, "OCT+": 13 };
const ENC = { SELECT: 0, ALGORITHM: 1, PRESETS: 2, KNOB1: 3, KNOB2: 4, KNOB3: 5, KNOB4: 6 };
const WHITE_K = [0, 2, 4, 6, 7, 9, 11, 12, 14, 16, 18, 19, 21, 23, 24, 26];
const BLACK_K = [1, 3, 5, 8, 10, 13, 15, 17, 20, 22, 25];
const BLACK_AFTER = [0, 1, 2, 4, 5, 7, 8, 9, 11, 12, 14];
const BODY = "#3a3d42", TRAY = "#26282c", KEY_W = "#242827", KEY_B = "#242827", KNOB = "#1f2124", BTNC = "#2e3135", LABEL = "#e8e8ea";
const $ = id=>document.getElementById(id);
const svg = $("fm1-svg");
function el(tag, attrs, parent = svg) {
  const e = document.createElementNS(NS, tag);
  for (const [k, v] of Object.entries(attrs)) e.setAttribute(k, v);
  parent.appendChild(e);
  return e;
}
function text(parent, x, y, s, size, fill = LABEL, weight = 600) {
  const t = el("text", { x, y, "text-anchor": "middle", "font-family": "'Barlow Semi Condensed', 'Arial Narrow', Arial, sans-serif",
                         "font-size": size, "font-weight": weight, fill, "letter-spacing": ".05em", class: "btn", "pointer-events": "none" }, parent);
  t.textContent = s;
  return t;
}

// ---------------------------------------------------------------- the panel (a picture: aria-hidden)
el("rect", { x: 4, y: 4, width: 992, height: 632, rx: 44, fill: BODY });
const knobs = {};
function knob(x, y, r, label, role, name) {
  const g = el("g", { class: "ctl knob" });
  el("circle", { cx: x, cy: y, r: r + 5, fill: TRAY }, g);
  el("circle", { cx: x, cy: y, r, fill: KNOB, stroke: "#44474c", "stroke-width": 2 }, g);
  const ind = el("line", { x1: x, y1: y - r + 5, x2: x, y2: y - r + 15, stroke: LABEL, "stroke-width": 3, "stroke-linecap": "round" }, g);
  text(g, x, y - r - 12, label, 17);
  el("circle", { cx: x, cy: y, r: r + 14, fill: "transparent" }, g);
  knobs[label] = { g, ind, x, y, angle: 0, role, name: name || label };
  return knobs[label];
}
knob(105, 100, 24, "MASTER", "master");
knob(215, 100, 24, "SELECT", ENC.SELECT);
knob(105, 205, 24, "PRESETS", ENC.PRESETS);
knob(215, 205, 24, "ALGORITHM", ENC.ALGORITHM);
for (let i = 0; i < 4; i++) knob(610 + i * 104, 100, 22, `KNOB${i + 1}`, ENC[`KNOB${i + 1}`], `KNOB ${i + 1}`);

const buttons = {};
function button(x, y, w, h, label, two) {
  const g = el("g", { class: "ctl" });
  el("rect", { x, y, width: w, height: h, rx: 9, fill: BTNC, stroke: "#4a4d52", "stroke-width": 1.5, class: "btnface" }, g);
  if (two) { text(g, x + w / 2, y + h / 2 - 1, label, 12); text(g, x + w / 2, y + h / 2 + 12, two, 12); }
  else text(g, x + w / 2, y + h / 2 + 5, label, 15);
  buttons[label] = { g, bit: BTN[label], name: { "OCT-": "OCT minus", "OCT+": "OCT plus" }[label] || label, light: "" };
}
el("rect", { x: 58, y: 258, width: 210, height: 58, rx: 14, fill: TRAY });
button(72, 268, 82, 38, "OCT-"); button(170, 268, 82, 38, "OCT+");
el("rect", { x: 300, y: 44, width: 236, height: 236, rx: 26, fill: "#111214" });
el("rect", { x: 568, y: 160, width: 400, height: 140, rx: 18, fill: TRAY });
const ROW1 = ["FX", "SEL", "ENV", "LFO", "EDIT", "GLO"], ROW2 = ["HOME", "SAVE", "ARP", "SEQ", "PLAY", "REC"];
for (let i = 0; i < 6; i++) button(582 + i * 63, 174, 54, 48, ROW1[i]);          // row by row: the reading order
for (let i = 0; i < 6; i++) button(582 + i * 63, 236, 54, 48, ROW2[i], ROW2[i] === "PLAY" ? "STOP" : undefined);

const keys = [];
(function keybed(x0, y0, w, h) {
  el("rect", { x: x0, y: y0, width: w, height: h, rx: 18, fill: TRAY });
  const pitch = (w - 24) / 16, kw = pitch * 0.78;
  BLACK_AFTER.forEach((a, k) => {                                                   // the black keys: the upper row
    const g = el("g", { class: "ctl" });
    el("rect", { x: x0 + 12 + (a + 1) * pitch - kw / 2, y: y0 + h * 0.06, width: kw, height: h * 0.33, rx: kw / 2, fill: KEY_B, class: "keyface black" }, g);
    keys[BLACK_K[k]] = { g, name: `Black key ${k + 1}`, light: "" };
  });
  for (let i = 0; i < 16; i++) {
    const g = el("g", { class: "ctl" });
    el("rect", { x: x0 + 12 + i * pitch + (pitch - kw) / 2, y: y0 + h * 0.43, width: kw, height: h * 0.52, rx: kw / 2, fill: KEY_W, class: "keyface white" }, g);
    keys[WHITE_K[i]] = { g, name: `White key ${i + 1}` };
  }
})(36, 340, 928, 270);

// UNDERGROWTH adaptation: match the photographed dark keybed and white key markers.
keys.forEach(k=>{const b=k.g.getBBox();el('line',{x1:b.x+b.width/2,y1:b.y+10,x2:b.x+b.width/2,y2:b.y+b.height*.5,stroke:'#e0e4dd','stroke-width':4,'stroke-linecap':'round'},k.g);});
// ---------------------------------------------------------------- the controls
// The drawing is only a picture (pointer-events: none). Each control gets an invisible HTML target over it,
// which is also what the keyboard and a screen reader use: browsers (iOS Safari above all) honour touch-action
// and touchstart on HTML elements but not on SVG ones, so the controls keep their touches while a finger
// anywhere else on the device scrolls the page natively.
const controls = $("fm1-controls");
function group(label) {
  const d = document.createElement("div");
  d.setAttribute("role", "group");
  d.setAttribute("aria-label", label);
  controls.appendChild(d);
  return d;
}
function hit(c, parent, attrs, cls) {
  const b = c.g.getBBox(), d = document.createElement("div");
  d.className = "hit" + (cls ? " " + cls : "");
  Object.assign(d.style, { left: `${b.x / 10}%`, top: `${b.y / 6.4}%`, width: `${b.width / 10}%`, height: `${b.height / 6.4}%` });
  d.tabIndex = 0;
  for (const [k, v] of Object.entries(attrs)) if (v != null) d.setAttribute(k, v);
  if (attrs["aria-description"]) d.title = attrs["aria-description"];
  d.addEventListener("touchstart", (e) => e.preventDefault(), { passive: false });
  // a click focuses the control (without the ring), so the panel's keyboard shortcuts follow the mouse
  d.addEventListener("pointerdown", () => d.focus({ preventScroll: true }));
  parent.appendChild(d);
  c.hit = d;
}
const gKnobs = group("Knobs"), gButtons = group("Buttons"), gBlack = group("Black keys"), gWhite = group("White keys");
let master = 4096*.65;
const masterPct = () => Math.round(master * 100 / 4096);
for (const [label, k] of Object.entries(knobs)) {
  if (k.role === "master")      // the volume has a value: a slider
    hit(k, gKnobs, { role: "slider", "aria-label": "MASTER", "aria-description": "Volume", "aria-orientation": "vertical",
                     "aria-valuemin": 0, "aria-valuemax": 100, "aria-valuenow": masterPct(), "aria-valuetext": `${masterPct()} percent` }, "knob");
  else                          // an endless encoder has none: the screen says what it changed
    hit(k, gKnobs, { role: "button", "aria-label": k.name,
                     "aria-description": "Turn it with the arrow keys, Page Up and Page Down for big steps" }, "knob");
}
for (const lbl of ["OCT-", "OCT+", ...ROW1, ...ROW2])
  hit(buttons[lbl], gButtons, { role: "button", "aria-label": buttons[lbl].name, "aria-pressed": "false"});
BLACK_K.forEach((n) => hit(keys[n], gBlack, { role: "button", "aria-label": keys[n].name, "aria-pressed": "false" }));
WHITE_K.forEach((n) => hit(keys[n], gWhite, { role: "button", "aria-label": keys[n].name, "aria-pressed": "false" }));


function drawKnob(k){k.ind.setAttribute('transform',`rotate(${k.angle} ${k.x} ${k.y})`);}
function turn(k,n){if(!n)return;if(k.role==='master'){master=Math.max(0,Math.min(4096,master+n*40.96));k.angle=-135+270*master/4096;k.hit.setAttribute('aria-valuenow',masterPct());k.hit.setAttribute('aria-valuetext',masterPct()+' percent');}else k.angle=(k.angle+n*18)%360;drawKnob(k);const name=Object.keys(knobs).find(name=>knobs[name]===k);callbacks.turn(name,n);}
for (const k of Object.values(knobs)) {
  if (k.role === "master") { k.angle = -135 + 270 * master / 4096; drawKnob(k); }
  let drag = null, wheel = 0;
  k.hit.addEventListener("pointerdown", (e) => {
    e.preventDefault();
    k.hit.setPointerCapture(e.pointerId);
    callbacks.begin('drag');
    drag = { y: e.clientY, acc: 0 };
  });
  k.hit.addEventListener("pointermove", (e) => {
    if (!drag) return;
    drag.acc += drag.y - e.clientY;
    drag.y = e.clientY;
    const step = k.role === "master" ? 3 : 7;
    const n = Math.trunc(drag.acc / step);
    if (n) { drag.acc -= n * step; turn(k, n); }
  });
  const end = () => { drag = null; callbacks.end(); };
  k.hit.addEventListener("pointerup", end);
  k.hit.addEventListener("pointercancel", end);
  k.hit.addEventListener("wheel", (e) => {
    e.preventDefault();callbacks.begin('wheel');
    const dy = e.deltaMode === 1 ? e.deltaY * 33 : e.deltaY;
    if (Math.abs(dy) >= 50) { turn(k, dy < 0 ? 1 : -1); wheel = 0; return; }   // a mouse wheel notch: one detent
    wheel -= dy;                                                                   // a trackpad: by distance
    const n = Math.trunc(wheel / 25);
    if (n) { wheel -= n * 25; turn(k, n); }
  }, { passive: false });
  k.hit.addEventListener("keydown", (e) => {
    if (e.metaKey || e.ctrlKey || e.altKey) return;
    callbacks.begin('key');
    const big = k.role === "master" ? 8 : 5;
    const n = { ArrowUp: 1, ArrowRight: 1, ArrowDown: -1, ArrowLeft: -1, PageUp: big, PageDown: -big }[e.key];
    if (n) turn(k, n);
    else if (k.role === "master" && (e.key === "Home" || e.key === "End")) turn(k, e.key === "End" ? 64 : -64);
    else return;
    e.preventDefault(); e.stopPropagation();
  });
}


for(const [name,b] of Object.entries(buttons)){
 // The template suppresses touchstart's compatibility click. Handle the physical
 // press directly; keep click only for assistive activation without a press.
 let directAt=-Infinity;
 const activate=()=>{directAt=performance.now();callbacks.button(name);};
 b.hit.addEventListener('pointerdown',e=>{e.preventDefault();b.hit.setPointerCapture(e.pointerId);b.g.classList.add('held');activate();});
 for(const event of ['pointerup','pointercancel','lostpointercapture','blur'])b.hit.addEventListener(event,()=>b.g.classList.remove('held'));
 b.hit.addEventListener('click',()=>{if(performance.now()-directAt>500)activate();});
 b.hit.addEventListener('keydown',e=>{if((e.code==='Enter'||e.code==='Space')&&!e.repeat){e.preventDefault();e.stopPropagation();activate();}});
 if(name==='HOME'){let timer;const clear=()=>clearTimeout(timer);b.hit.addEventListener('pointerdown',()=>timer=setTimeout(()=>callbacks.panic(),650));for(const e of ['pointerup','pointercancel','lostpointercapture','pointerleave'])b.hit.addEventListener(e,clear);}
}
keys.forEach((k,i)=>{
 const down=()=>{k.g.classList.add('held');callbacks.key(i,true,WHITE_K.indexOf(i));};
 const up=()=>{k.g.classList.remove('held');callbacks.key(i,false,WHITE_K.indexOf(i));};
 k.hit.addEventListener('pointerdown',e=>{e.preventDefault();k.hit.setPointerCapture(e.pointerId);down();});
 for(const e of ['pointerup','pointercancel','lostpointercapture','blur'])k.hit.addEventListener(e,up);
 k.hit.addEventListener('keydown',e=>{if((e.code==='Enter'||e.code==='Space')&&!e.repeat){e.preventDefault();e.stopPropagation();down();}});
 k.hit.addEventListener('keyup',e=>{if(e.code==='Enter'||e.code==='Space'){e.preventDefault();e.stopPropagation();up();}});
});
return {knobs,buttons,keys,whiteKeys:WHITE_K};

};
