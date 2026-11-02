#include "../core/undergrowth.h"
#include "../core/controller.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <time.h>
static double render(int frames){double e=0;while(frames>0){int n=frames>128?128:frames;ug_render(n);for(int i=0;i<n*2;i++){float x=ug_buffer()[i];assert(isfinite(x)&&fabsf(x)<=.95001f);e+=x*x;}frames-=n;}return e;}
static void tap(int k){ug_ui_key(k,1,1);ug_ui_key(k,0,1);}
static double sound(int param,float value){ug_init(44100);ug_setparam(2,param,value);ug_key(36,1);return render(22050);}
int main(void){
 for(int rate=8000;rate<=96000;rate+=88000){
  ug_init(rate);assert(render(rate)==0);ug_play(1);assert(render(rate)==0);
  ug_setparam(2,17,1);assert(render(rate)>.01);assert(ug_grain(1)>0&&ug_grain(1)<=8);
  ug_mute(2,1);render(rate);assert(render(rate)<.00001);ug_mute(2,0);
  ug_play(0);render(rate);assert(ug_grain(1)==0);ug_panic();assert(render(256)==0);
  for(int p=7;p<19;p++)ug_setparam(2,p,1);ug_play(1);render(rate*2);assert(ug_grain(1)<=8);ug_panic();assert(render(256)==0);
 }
 for(int p=7;p<=10;p++){double a=sound(p,0),b=sound(p,1);assert(fabs(a-b)>.01);}
 ug_init(44100);ug_stamp(2,0);ug_stamp(2,4);assert(!(ug_lockmask(2,0)&(1<<17)));ug_setlock(2,0,8,.9f);ug_setlock(2,0,9,.8f);ug_setlock(2,0,16,.7f);
 assert(ug_lockvalue(2,4,8)==.25f&&ug_getparam(2,8)==.25f);ug_play(1);render(1);assert(ug_effective(2,8)==.9f&&ug_effective(2,16)==.7f);
 ug_play(0);ug_recmode(2);ug_arm(1,2);ug_setparam(2,18,.9f);ug_play(1);render(44100);ug_arm(0,2);assert(ug_lockvalue(2,0,18)==.9f);ug_undo();assert(ug_lockvalue(2,0,18)==.15f);
 /* Layer sends route independently: dry grain output is identical until delay arrives. */
 ug_init(44100);ug_setparam(2,17,1);ug_play(1);double dry=render(44100);ug_play(0);double drytail=render(44100);
 ug_init(44100);ug_setparam(2,17,1);ug_setparam(2,16,1);ug_play(1);double wet=render(44100);ug_play(0);double wettail=render(44100);assert(fabs(dry-wet)>.01&&wettail>drytail);
 /* Shared panel: direct step notes, explicit keyboard assignment, Character locks. */
 ug_init(44100);ug_ui_init();ug_ui_select(0,2);ug_ui_button(9);tap(0);ug_ui_turn(3,7);assert(ug_notevalue(0,0)==43);
 ug_ui_button(9);assert(ug_ui_state(10)==0);tap(7);assert(ug_notevalue(0,0)==36&&ug_step(2,0));ug_ui_button(9);assert(ug_ui_state(10)==1);
 ug_ui_key(0,1,1);ug_ui_tick(351);ug_ui_key(0,0,1);ug_ui_turn(0,1);ug_ui_turn(3,20);assert(ug_lockvalue(2,0,7)>.44f&&ug_getparam(2,7)==.25f);
 ug_ui_edit(29,.8f);assert(ug_lockvalue(2,0,9)==.8f);ug_ui_button(9);ug_undo();assert(ug_lockvalue(2,0,7)==.25f);
 ug_ui_button(2);ug_ui_turn(0,1);ug_ui_turn(5,1);assert(ug_getparam(2,17)==1);ug_ui_turn(5,-1);assert(ug_getparam(2,17)==0);
 puts("PASS ROOT Character/grains: bounded cloud, explicit enable, stop/panic/mute, FM variation, high-bit locks/motion/Undo, separate sends, direct notes and keyboard assignment");
 /* Desktop comparison only. Not FM1 deadline utilisation. */
 for(int enabled=0;enabled<=1;enabled++){
  ug_init(44100);for(int v=0;v<4;v++)for(int st=0;st<16;st++)ug_setstep(v,st,1);
  ug_setparam(2,13,1);ug_setparam(2,12,1);ug_setparam(2,17,enabled);ug_play(1);
  clock_t start=clock();render(44100*10);double elapsed=(double)(clock()-start)/CLOCKS_PER_SEC;
  printf("HOST comparison: grains %s, 10 s audio rendered in %.4f CPU seconds (%.3f%% of real time; not FM1 CPU)\n",enabled?"ON":"OFF",elapsed,elapsed*10);
 }
}
