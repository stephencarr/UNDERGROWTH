#include "../core/undergrowth.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
static float render(int n){float peak=0;while(n>0){int k=n>128?128:n;ug_render(k);float *b=ug_buffer();for(int i=0;i<k*2;i++){assert(isfinite(b[i]));assert(fabsf(b[i])<=0.95001f);if(fabsf(b[i])>peak)peak=fabsf(b[i]);}n-=k;}return peak;}
int main(void){
 for(int sr=44100;sr<=48000;sr+=3900){
 ug_init(sr);assert(ug_state(0)==0);assert(ug_state(1)==-1);assert(render(sr)==0);ug_play(1);assert(render(sr)==0);
 ug_setparam(0,0,1);assert(render(sr)==0);ug_play(0);
 ug_hit(0,1);assert(render(sr)>0.1f);ug_panic();assert(render(sr)==0);
 ug_hit(1,1);assert(render(sr)>0.01f);ug_panic();assert(render(sr)==0);
 ug_tempo(120);ug_setstep(0,0,1);ug_setstep(0,4,1);ug_setstep(0,8,1);ug_setstep(0,12,1);
 ug_play(1);render(sr*2);assert(ug_state(8)==5); /* 1 earlier live + 4 sequence hits */
 ug_play(0);int count=ug_state(8);render(sr);assert(ug_state(8)==count);
 ug_init(sr);ug_begin_edit();ug_setstep(1,3,1);assert(ug_step(1,3));assert(ug_undo());assert(!ug_step(1,3));assert(ug_undo());assert(ug_step(1,3));
 ug_setparam(0,0,.3f);ug_setlock(0,0,0,.8f);assert(!ug_step(0,0));ug_play(1);render(1);assert(ug_effective(0,0)==.8f);ug_setparam(0,0,.1f);assert(ug_effective(0,0)==.1f);ug_rejoin();assert(ug_effective(0,0)==.8f);render(sr);assert(ug_effective(0,0)==.1f);
 ug_panic();ug_record(1);ug_hit(1,1);assert(!ug_step(1,0)); /* stopped hits not recorded */
 ug_play(1);ug_record(1);render(1);ug_hit(1,1);assert(ug_step(1,0));ug_clear();for(int v=0;v<2;v++)for(int s=0;s<16;s++){assert(!ug_step(v,s));assert(!ug_lockmask(v,s));}assert(render(sr)==0);assert(ug_undo());assert(ug_step(1,0));
 ug_init(sr);ug_tempo(220);for(int v=0;v<2;v++){for(int p=0;p<4;p++)ug_setparam(v,p,1);for(int s=0;s<16;s++)ug_setstep(v,s,1);}ug_master(1);ug_play(1);assert(render(sr*12)>0.1f);ug_panic();assert(render(sr)==0);
 ug_init(sr);ug_tempo(120);ug_play(1);ug_record(1);render(sr/10);ug_hit(0,1);assert(ug_state(8)==1);render(sr/30);assert(ug_state(8)==1);render(sr*2);assert(ug_state(8)==2); /* no first-pass flam, repeats next loop */
 ug_init(sr);ug_tempo(120);ug_setstep(0,1,1);ug_play(1);ug_record(1);render(sr/10);ug_hit(0,1);render(sr/30);assert(ug_state(8)==2); /* existing authored hit retained */
 ug_init(sr);ug_begin_edit();ug_setstep(0,0,1);ug_play(1);ug_record(1);render(1);ug_hit(0,1);ug_undo();assert(!ug_step(0,0)); /* no-op record preserves prior edit */
 ug_begin_edit();ug_setstep(0,0,1);ug_clear();ug_clear();ug_undo();assert(ug_step(0,0)); /* repeated clear preserves recovery */
 ug_setparam(-1,0,1);ug_setstep(9,999,1);ug_setlock(0,-1,0,1);assert(!ug_step(9,999));assert(ug_getparam(-1,0)==0);
 }
 printf("PASS: silent boot/empty Play, two voices, sample-clock sequencing, stop/panic, undo/redo, locks/manual/rejoin, inactive locks, quantized hit overdub, clear+undo, overload bounds, invalid indices; 44.1/48kHz. Core static state %d bytes.\n",ug_state_bytes());return 0;
}
