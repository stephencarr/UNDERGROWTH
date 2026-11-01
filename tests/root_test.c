#include "../core/undergrowth.h"
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
static float render(int n){float peak=0;while(n>0){int k=n>256?256:n;ug_render(k);for(int i=0;i<k*2;i++){float x=ug_buffer()[i];assert(isfinite(x)&&fabsf(x)<=.95001f);if(fabsf(x)>peak)peak=fabsf(x);}n-=k;}return peak;}
static int crossings(int rate,int note){ug_init(rate);ug_setparam(2,1,0);ug_key(note,1);render(rate/10);float prev=0;int count=0;for(int i=0;i<rate;i++){ug_render(1);float x=ug_buffer()[0];if(prev<0&&x>=0)count++;prev=x;}ug_key(note,0);return count;}
int main(void){for(int rate=44100;rate<=48000;rate+=3900){
 assert(abs(crossings(rate,36)-65)<=1);assert(abs(crossings(rate,48)-131)<=1);
 ug_init(rate);assert(render(rate)==0);ug_play(1);assert(render(rate)==0);ug_play(0);
 ug_key(36,1);assert(render(rate/10)>.01f);ug_key(40,1);ug_key(36,0);assert(ug_state(16)==-1);ug_key(40,0);assert(ug_state(16)==0);render(rate*3);assert(render(rate)==0);
 ug_key(36,1);ug_panic();assert(render(rate)==0);
 ug_init(rate);ug_tempo(120);ug_setstep(0,0,1);ug_setstep(2,0,1);ug_setstep(2,4,1);ug_note(0,36,.5f,0);ug_note(4,43,1.5f,1);ug_play(1);render(1);assert(ug_state(14)==1&&ug_state(8)==1);render(rate/10);assert(ug_state(16)==0);render(rate/2);assert(ug_state(14)==2);ug_play(0);assert(ug_state(16)==0);
 ug_init(rate);ug_setstep(2,0,1);ug_note(0,40,.8f,1);ug_begin_edit();ug_note(0,48,.4f,0);ug_undo();assert(ug_notevalue(0,0)==40&&ug_notevalue(0,2)==1);
 ug_setlock(2,0,1,.9f);ug_motion(1);ug_setparam(2,0,.3f);ug_play(1);render(1);assert(ug_lockvalue(2,0,0)==.3f&&ug_lockvalue(2,0,1)==.9f);ug_motion(0);assert(ug_state(13)==0);ug_undo();assert(ug_lockmask(2,0)==2);
 ug_init(rate);ug_play(1);ug_record(1);render(1);ug_key(45,1);assert(ug_step(2,0)&&ug_notevalue(0,0)==45);ug_undo();assert(!ug_step(2,0));ug_clear();assert(ug_notevalue(0,0)==36&&ug_notevalue(0,1)==.75f);
 /* Slide changes the transition audio; stale live key-up cannot cut a sequenced note. */
 float transition[256];int different=0;
 for(int slide=0;slide<2;slide++){
  ug_init(rate);ug_tempo(120);ug_setstep(2,0,1);ug_setstep(2,1,1);ug_note(0,36,1.5f,0);ug_note(1,48,1,slide);ug_play(1);render(rate/8);ug_render(128);
  for(int i=0;i<256;i++){if(!slide)transition[i]=ug_buffer()[i];else if(transition[i]!=ug_buffer()[i])different=1;}
 }assert(different);
 ug_init(rate);ug_setstep(2,0,1);ug_key(36,1);ug_play(1);render(1);ug_key(36,0);assert(ug_state(16)>0);
 ug_note(-1,99,1,0);ug_note(0,999,9,1);assert(ug_notevalue(0,0)==60&&ug_notevalue(0,1)==1.5f);
 }puts("PASS ROOT tuning, silence, held/released keys, gate, independent lane, note Undo, motion/locks, note overdub, clear/defaults at 44.1/48 kHz.");}
