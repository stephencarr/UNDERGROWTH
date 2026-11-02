import assert from 'node:assert/strict';
import {ConfirmedUpdater, verifyPair} from '../web/confirmed-updater.js';
import {pack7} from '../web/vendor/fm1-base/fm1ota.js';

function midiPair(name, identity) {
  const sent=[];
  const input={id:name+'-in',name,state:'connected',open:async()=>{},onmidimessage:null};
  const output={id:name+'-out',name,state:'connected',open:async()=>{},send(bytes){
    sent.push([...bytes]);
    if(bytes[1]===0&&bytes[2]===0x32){
      const payload=[0,0x59,0x11,0,0,0,...Array.from(identity,c=>c.charCodeAt(0))];
      while(payload.length<34)payload.push(0);
      queueMicrotask(()=>input.onmidimessage?.({data:Uint8Array.from([0xF0,...pack7(payload),0xF7])}));
    }
  }};
  return {input,output,sent};
}
const a=midiPair('FM-1 A','FM-1_620000100'),b=midiPair('FM-1 B','FM-1_620000100');
const access={inputs:new Map([[b.input.id,b.input],[a.input.id,a.input]]),outputs:new Map([[b.output.id,b.output],[a.output.id,a.output]])};
let stagePair=null;
const bound=new ConfirmedUpdater(access,a,'FM-1_620000100',()=>{},async()=>{throw new Error('No transition confirmed');});
bound.stages=async dev=>{stagePair=dev.link.output;dev.link.close();};
await bound.install(new Uint8Array(), 'FM-1_731000102');
assert.equal(stagePair,a.output);assert.equal(b.sent.length,0,'Port order must not send even discovery queries to the unconfirmed pair');
access.inputs.delete(a.input.id);access.outputs.delete(a.output.id);
await assert.rejects(()=>bound.install(new Uint8Array(),'FM-1_731000102'),/disconnected/);
await assert.rejects(()=>bound.resume(new Uint8Array(),()=>{},'FM-1_731000102'),/disconnected/);
assert.equal(b.sent.length,0,'Disconnected target must not fall back to another FM1');
access.inputs.set(a.input.id,a.input);access.outputs.set(a.output.id,a.output);a.invalidated=true;
await assert.rejects(()=>verifyPair(access,a),/disconnected/);
assert.equal(b.sent.length,0);
console.log('PASS confirmed updater: exact pair despite reorder, disconnect/reconnect invalidation, install/resume never fall back');
