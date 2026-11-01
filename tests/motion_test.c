#include "../core/undergrowth.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
static void render(int n){while(n>0){int k=n>256?256:n;ug_render(k);for(int i=0;i<k*2;i++)assert(isfinite(ug_buffer()[i]));n-=k;}}
int main(void){
 for(int rate=44100;rate<=48000;rate+=3900){
  ug_init(rate);ug_tempo(120);ug_setstep(0,0,1);ug_setstep(0,4,1);ug_setstep(1,4,1);
  ug_setlock(0,4,3,.6f);ug_setlock(0,0,0,.9f);float base=ug_getparam(0,0);
  ug_motion(1);ug_setparam(0,0,.2f);assert(ug_getparam(0,0)==base);
  assert(ug_lockvalue(0,0,0)==.9f);ug_play(1);render(1);assert(ug_lockvalue(0,0,0)==.2f);
  render(rate/8);assert(!ug_lockmask(0,1)); /* rests stay untouched */
  ug_setparam(0,0,.8f);render(rate/2);assert(ug_lockvalue(0,4,0)==.8f);
  assert(ug_lockvalue(0,4,3)==.6f);assert(!ug_lockmask(1,4));
  ug_motion(0);assert(!ug_state(10));assert(!ug_state(6));assert(ug_getparam(0,0)==base);
  ug_play(0);ug_play(1);render(1);assert(ug_effective(0,0)==.2f); /* locks govern after Finish */
  assert(ug_undo());assert(ug_lockvalue(0,0,0)==.9f);assert(ug_lockmask(0,4)==8);assert(ug_lockvalue(0,4,3)==.6f);
  assert(ug_step(0,0)&&ug_step(0,4)&&ug_step(1,4));assert(ug_undo());assert(ug_lockvalue(0,4,0)==.8f);
  /* An armed take with no moved controls preserves locks and prior Undo. */
  ug_init(rate);ug_begin_edit();ug_setstep(0,0,1);ug_motion(1);ug_play(1);render(rate);ug_motion(0);assert(!ug_lockmask(0,0));ug_undo();assert(!ug_step(0,0));
  /* No hits: sliders/live taps do not write locks or replace previous Undo. */
  ug_init(rate);ug_begin_edit();ug_setlock(1,2,3,.7f);ug_motion(1);ug_setparam(1,1,.8f);ug_play(1);ug_hit(1,1);render(rate);ug_play(0);
  assert(ug_lockmask(1,2)==8);for(int s=0;s<16;s++)assert(!ug_step(1,s));ug_undo();assert(!ug_lockmask(1,2));
  /* Modes are exclusive; Stop/Panic/Undo safely end a take. */
  ug_record(1);ug_motion(1);assert(!ug_state(2)&&ug_state(10));ug_record(1);assert(!ug_state(10)&&ug_state(2));
  ug_motion(1);ug_panic();assert(!ug_state(10)&&!ug_state(2)&&!ug_state(6));
  /* Chatter with swing/Human captures once per authored event; untouched locks survive. */
  ug_init(rate);ug_tempo(120);ug_groove(0,1);ug_groove(1,1);ug_setstep(1,1,1);ug_setlock(1,1,2,.3f);
  ug_motion(1);ug_setparam(1,0,.6f);ug_setparam(1,3,.4f);ug_play(1);render(rate/5);
  assert(ug_lockmask(1,1)==13);assert(ug_lockvalue(1,1,0)==.6f);assert(ug_lockvalue(1,1,2)==.3f);assert(ug_lockvalue(1,1,3)==.4f);
  ug_undo();assert(!ug_state(10));assert(ug_lockmask(1,1)==4);render(rate);assert(ug_lockmask(1,1)==4);
  /* Human delays the actual trigger: capture the latest value when it sounds. */
  ug_init(rate);ug_groove(1,1);ug_setstep(1,0,1);ug_motion(1);ug_setparam(1,0,.2f);ug_play(1);render(1);
  assert(!ug_lockmask(1,0));ug_setparam(1,0,.8f);render(rate/100);assert(ug_lockvalue(1,0,0)==.8f);
 }
 puts("PASS motion: per-trigger overdub, rests/base/untouched locks preserved, whole-take Undo/redo, empty/no-op takes, modes, Stop/Panic, swung Human chatter at 44.1/48 kHz.");return 0;
}
