import fs from 'node:fs';import {execFileSync} from 'node:child_process';import assert from 'node:assert/strict';
const native=execFileSync('./build/render',{maxBuffer:8*1024*1024});
const {instance}=await WebAssembly.instantiate(fs.readFileSync('web/core.wasm'),{});const e=instance.exports;
e.ug_init(44100);e.ug_groove(0,.65);e.ug_groove(1,.8);e.ug_tempo(110);for(let s=0;s<16;s+=4)e.ug_setstep(0,s,1);for(let s=2;s<16;s+=3)e.ug_setstep(1,s,1);
e.ug_setlock(1,5,0,.8);e.ug_setlock(1,11,2,.5);e.ug_play(1);
const out=Buffer.alloc(native.length);for(let block=0;block<2048;block++){
 if(block===200)e.ug_hit(1,.75);if(block===500){e.ug_setparam(0,0,.7);e.ug_setparam(1,3,.8);}if(block===700)e.ug_rejoin();if(block===1200){e.ug_groove(0,0);e.ug_groove(1,0);}if(block===1700)e.ug_play(0);
 e.ug_render(128);Buffer.from(e.memory.buffer,e.ug_buffer(),1024).copy(out,block*1024);
}
assert.deepEqual(out,native);console.log(`PASS native/WASM bit-identical: ${out.length/8} stereo frames, 2048 blocks; shared score, swing/Human, live hits, locks, manual takeover, rejoin, stop/tails. WASM memory ${e.memory.buffer.byteLength} bytes (not device RAM usage).`);
fs.writeFileSync('build/demo.f32',out);
