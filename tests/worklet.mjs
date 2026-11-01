import fs from 'node:fs';import vm from 'node:vm';import assert from 'node:assert/strict';
const module=await WebAssembly.compile(fs.readFileSync('web/core.wasm'));
let Processor;const states=[];
const sandbox={sampleRate:48000,WebAssembly,Float32Array,Array,Math,Number,
 AudioWorkletProcessor:class {constructor(){this.port={postMessage:m=>states.push(m)};}},
 registerProcessor:(name,c)=>{assert.equal(name,'undergrowth');Processor=c;}};
vm.runInNewContext(fs.readFileSync('web/worklet.js','utf8'),sandbox);
const p=new Processor({processorOptions:{module}});
const cmd=(type,...args)=>p.port.onmessage({data:{type,args}});
const block=()=>{const out=[new Float32Array(128),new Float32Array(128)];assert(p.process([], [out]));return out;};
const silent=()=>block().every(a=>a.every(x=>x===0));
assert(silent());cmd('play',1);for(let i=0;i<20;i++)assert(silent());
cmd('toggle',0,0);cmd('toggle',0,0);assert.equal(states.at(-1).voices[0].steps[0].on,false);
cmd('begin');cmd('toggle',0,0);cmd('undo');assert.equal(states.at(-1).voices[0].steps[0].on,false);
cmd('lock',1,2,0,.85);assert.equal(states.at(-1).voices[1].steps[2].on,false);
cmd('hit',1,1);assert(block().some(a=>a.some(x=>Math.abs(x)>0)));
cmd('param',0,0,NaN);cmd('unknown',1);cmd('panic');assert(silent());assert.equal(states.at(-1).playing,0);
cmd('play',-1);assert.equal(states.at(-1).playing,1);cmd('play',-1);assert.equal(states.at(-1).playing,0);
cmd('record',-1);assert.equal(states.at(-1).recording,1);cmd('record',-1);assert.equal(states.at(-1).recording,0);
console.log('PASS AudioWorklet adapter in Node harness: 48kHz, silent init/empty transport, actual WASM render, command validation, rapid toggles, undo, locks preserve rests, Panic, toggled transport/record. Not browser-host execution.');

cmd('panic');cmd('clear');cmd('step',2,0,1);cmd('motion',1);cmd('editparam',2,0,.7);cmd('play',1);block();cmd('snapshot');assert.equal(states.at(-1).motion,true);assert(states.at(-1).voices[2].steps[0].mask&1);cmd('motion',0);cmd('undo');assert.equal(states.at(-1).voices[2].steps[0].mask,0);console.log('PASS ordered arm + immediate slider edit retains motion and whole-take Undo.');
