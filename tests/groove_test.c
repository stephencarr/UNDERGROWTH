#include "../core/undergrowth.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
static void render(int n){while(n>0){int k=n>256?256:n;ug_render(k);for(int i=0;i<k*2;i++)assert(isfinite(ug_buffer()[i]));n-=k;}}
int main(void){
 for(int rate=44100;rate<=48000;rate+=3900){
  /* Max swing: 2:1 subdivisions with exact pair/loop length, no tempo drift. */
  ug_init(rate);ug_tempo(120);ug_groove(0,1);ug_play(1);
  int position=0;
  for(int s=0;s<32;s++){
   int duration=(s&1)?rate/12:rate/6;
   render(1);assert(ug_state(1)==s%16);
   render(duration-1);assert(ug_state(1)==s%16);position+=duration;
  }
  assert(position==rate*4);
  /* Human alone never creates events, even at extremes. */
  ug_init(rate);ug_groove(1,1);ug_play(1);render(rate*3);assert(ug_state(8)==0&&ug_state(9)==0);
  ug_panic();ug_setstep(0,0,1);ug_setstep(1,0,1);ug_play(1);render(1);
  assert(ug_state(8)==1);assert(ug_state(9)==0); /* kick firm, chatter pending */
  render(rate/100);assert(ug_state(9)==1);
  ug_panic();ug_play(1);render(1);ug_play(0);render(rate/100);assert(ug_state(9)==1); /* Stop cancels pending */
  ug_play(1);render(1);ug_hit(1,1);int hits=ug_state(9);render(rate/100);assert(ug_state(9)==hits); /* live tap cancels pending */
  /* Recording quantizes around swung boundaries, not straight sixteenths. */
  ug_init(rate);ug_tempo(120);ug_groove(0,1);ug_play(1);ug_record(1);render(rate*7/100);ug_hit(0,1);
  assert(ug_step(0,0));assert(!ug_step(0,1));
  ug_clear();ug_play(1);ug_record(1);render(rate/10);ug_hit(0,1);assert(ug_step(0,1));
  int before=ug_state(8);render(rate/10);assert(ug_state(8)==before); /* no first-pass flam */
  /* Groove is authored state: one-edit Undo/redo, score preserved. */
  ug_init(rate);ug_setstep(0,4,1);ug_begin_edit();ug_groove(0,.7f);ug_groove(1,.8f);
  assert(ug_undo());assert(ug_getgroove(0)==0&&ug_getgroove(1)==0&&ug_step(0,4));
  assert(ug_undo());assert(ug_getgroove(0)==.7f&&ug_getgroove(1)==.8f);
  ug_groove(0,-1);ug_groove(1,2);assert(ug_getgroove(0)==0&&ug_getgroove(1)==1);
  /* Each step's timing feel repeats, independent of loop count. */
  ug_init(rate);ug_tempo(120);ug_groove(1,1);ug_setstep(1,0,1);ug_play(1);
  int onset=-1;for(int i=0;i<rate/100;i++){render(1);if(ug_state(9)){onset=i;break;}}
  assert(onset>=0&&onset<=rate*8/1000);
  render(rate*2-onset-1);assert(ug_state(9)==1);render(onset);assert(ug_state(9)==1);render(1);assert(ug_state(9)==2);
 }
 puts("PASS groove: swing pair/loop timing, Human rests/kick protection/repeatability, Stop/live cancellation, swung recording, Undo/redo at 44.1/48kHz.");return 0;
}
