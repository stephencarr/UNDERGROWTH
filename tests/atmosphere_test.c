#include "../core/undergrowth.h"
#include "../core/controller.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
static double render(int frames){double sum=0;while(frames>0){int n=frames>128?128:frames;ug_render(n);for(int i=0;i<n*2;i++){float x=ug_buffer()[i];assert(isfinite(x)&&fabsf(x)<=.95001f);sum+=x*x;}frames-=n;}return sum;}
static void start(int sr,int v){ug_init(sr);ug_setparam(v,17,1);ug_play(1);}
int main(void){const int rates[]={8000,44100,48000,96000};
 for(unsigned r=0;r<4;r++)for(int v=0;v<4;v++)if(v!=2){int sr=rates[r];
  ug_init(sr);assert(render(sr/4)==0);ug_play(1);assert(render(sr/4)==0);
  ug_setparam(v,17,1);assert(render(sr)>.01);
  ug_mute(v,1);render(sr);assert(render(sr/4)<1e-7);ug_mute(v,0);
  assert(render(sr/4)>.001);ug_play(0);render(sr);assert(render(sr/4)<1e-7);
  ug_panic();assert(render(256)==0);
  for(int corner=0;corner<2;corner++){
   start(sr,v);for(int p=11;p<20;p++)if(p!=17)ug_setparam(v,p,(float)corner);
   ug_setparam(v,15,1);assert(render(sr*2)>=0);ug_panic();assert(render(256)==0);
  }
 }
 double energy[4]={0};for(int v=0;v<4;v++)if(v!=2){
  start(44100,v);energy[v]=render(44100);
  ug_play(0);float level=ug_getparam(v,15),send=.65f;ug_setparam(v,16,send);ug_stamp(v,0);
  assert(!(ug_lockmask(v,0)&(1<<15)));assert(!(ug_lockmask(v,0)&(1<<16)));
  ug_play(1);render(128);assert(ug_effective(v,15)==level&&ug_effective(v,16)==send);
  ug_play(0);ug_begin_edit();ug_setparam(v,17,0);ug_undo();assert(ug_getparam(v,17)==1);
  ug_recmode(2);ug_arm(1,v);ug_setparam(v,17,0);ug_arm(0,v);ug_undo();assert(ug_getparam(v,17)==1);
  ug_setlock(v,0,17,0);assert(!(ug_lockmask(v,0)&(1<<17)));
  ug_recmode(2);ug_arm(1,v);ug_setparam(v,11,.8f);ug_play(1);render(44100);ug_arm(0,v);
  assert(ug_lockvalue(v,0,11)==.8f);ug_undo();assert(!(ug_lockmask(v,0)&(1<<11)));
  ug_init(44100);ug_ui_init();ug_ui_select(0,v);ug_ui_button(2);ug_ui_turn(0,2);
  assert(strcmp(ug_ui_knoblabel(3),"Body"));ug_ui_turn(0,1);assert(!strcmp(ug_ui_knoblabel(5),"Enabled"));
  ug_ui_turn(5,1);assert(ug_getparam(v,17)==1);
  ug_ui_button(0);assert(!strcmp(ug_ui_knoblabel(4),"ENV send"));ug_ui_turn(4,50);assert(ug_getparam(v,16)>.49f);
 }
 assert(fabs(energy[0]-energy[1])>.01&&fabs(energy[1]-energy[3])>.01);
 for(int p=11;p<20;p++)if(p!=15&&p!=16&&p!=17){for(int v=0;v<4;v++)if(v!=2){start(44100,v);ug_setparam(v,p,0);double a=render(44100);start(44100,v);ug_setparam(v,p,1);double b=render(44100);assert(fabs(a-b)>1e-6);}}
 ug_init(44100);for(int v=0;v<4;v++){ug_setparam(v,17,1);ug_setparam(v,16,1);for(int s=0;s<16;s++)ug_setstep(v,s,1);}ug_setfx(0,1);ug_play(1);render(44100*5);ug_panic();assert(render(256)==0);
 puts("PASS atmospheres: silent startup, empty-score drones, ranges/corners, mute/Stop/Panic, independent ownership, enable Undo, motion/Undo and controller access");
}
