/* Thin audio adapter. C owns synthesis, transport, score, locks and recovery. */
class UndergrowthProcessor extends AudioWorkletProcessor {
  constructor(options) {
    super();
    this.e = new WebAssembly.Instance(options.processorOptions.module, {}).exports;
    this.e.ug_init(sampleRate);
    this.buffer = new Float32Array(this.e.memory.buffer, this.e.ug_buffer(), 512);
    this.frames = 0;
    this.commands = {
      velocity:'ug_velocity', keyvelocity:'ug_keyvelocity', arm:'ug_arm', recmode:'ug_recmode', draftnote:'ug_draftnote', stamp:'ug_stamp', erase:'ug_erase', note: 'ug_note', key: 'ug_key', motion: 'ug_motion', groove: 'ug_groove', play: 'ug_play', hit: 'ug_hit', record: 'ug_record', tempo: 'ug_tempo', master: 'ug_master',
      toggle: 'ug_togglestep', param: 'ug_setparam', step: 'ug_setstep', lock: 'ug_setlock', clearlock: 'ug_clearlock',
      begin: 'ug_begin_edit', undo: 'ug_undo', clear: 'ug_clear', rejoin: 'ug_rejoin', panic: 'ug_panic'
    };
    this.port.onmessage = ({data}) => {
      if (data.type === 'snapshot') return this.snapshot();
      if(data.type==='editvelocity'){
        if(!Array.isArray(data.args)||data.args.length!==3||data.args.some(x=>!Number.isFinite(x)))return;
        if(this.e.ug_state(18)||this.e.ug_state(10)||this.e.ug_state(2))return;
        this.e.ug_begin_edit();this.e.ug_velocity(...data.args);this.snapshot();return;
      }
      if(data.type==='editparam'){
        if(!Array.isArray(data.args)||data.args.length!==3||data.args.some(x=>!Number.isFinite(x)))return;
        if(!this.e.ug_state(10)&&!this.e.ug_state(18))this.e.ug_begin_edit();
        this.e.ug_setparam(...data.args);this.snapshot();return;
      }
      const fn = this.commands[data.type];
      if (!fn || !Array.isArray(data.args) || data.args.length > 4 || data.args.some(x => !Number.isFinite(x))) return;
      this.e[fn](...data.args);
      this.snapshot();
    };
    this.snapshot();
  }
  snapshot() {
    const e=this.e;
    this.port.postMessage({type:'state', playing:e.ug_state(0), step:e.ug_state(1), recording:e.ug_state(2),
      rate:e.ug_state(3), tempo:e.ug_state(4), undo:!!e.ug_state(5),
      recordMode:e.ug_state(17),armed:!!e.ug_state(18),recordTrack:e.ug_state(19),draftNote:[0,1,2].map(p=>e.ug_draftvalue(p)),
      motion:!!e.ug_state(10), motionMask:[e.ug_state(11),e.ug_state(12),e.ug_state(15)],
      groove:[0,1].map(p=>e.ug_getgroove(p)),
      voices:[0,1,2].map(v=>({takeover:e.ug_state(v===2?13:6+v), count:e.ug_state(v===2?14:8+v),
        velocity:e.ug_getvelocity(v,-1), base:[0,1,2,3,4,5,6].map(p=>e.ug_getparam(v,p)), effective:[0,1,2,3,4,5,6].map(p=>e.ug_effective(v,p)),
        steps:Array.from({length:16},(_,s)=>({velocity:e.ug_getvelocity(v,s),note:e.ug_notevalue(s,0),gate:e.ug_notevalue(s,1),slide:!!e.ug_notevalue(s,2),on:!!e.ug_step(v,s),mask:e.ug_lockmask(v,s),lock:[0,1,2,3,4,5,6].map(p=>e.ug_lockvalue(v,s,p))}))}))});
  }
  process(inputs, outputs) {
    const channels=outputs[0]; if(!channels?.length)return true;
    const count=channels[0].length;
    // Render bounded slices; no JS audio synthesis or transport timing.
    for(let offset=0;offset<count;offset+=256){
      const n=Math.min(256,count-offset);this.e.ug_render(n);
      for(let c=0;c<channels.length;c++)for(let i=0;i<n;i++)channels[c][offset+i]=this.buffer[i*2+(c%2)];
    }
    this.frames+=count;
    if(this.frames>=sampleRate/24){this.frames=0;this.snapshot();}
    return true;
  }
}
registerProcessor('undergrowth',UndergrowthProcessor);
