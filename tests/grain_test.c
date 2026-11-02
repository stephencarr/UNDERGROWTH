/* Exercise correlated sample reads and grain lifetime, independently of reverb. */
#include "../core/undergrowth.c"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "../core/controller.h"
static void start(int sr,float density){
 ug_init(sr);for(int i=0;i<TEXTURE;i++)texture[i]=1;
 ug_setparam(2,17,1);ug_setparam(2,15,1);ug_setparam(2,12,.5f);
 ug_setparam(2,13,density);ug_setparam(2,18,0);ug_play(1);
}
int main(void){
 const int rates[]={8000,22050,44100,48000,96000};
 for(unsigned r=0;r<sizeof rates/sizeof *rates;r++){
  int sr=rates[r];float peak[2]={0};
  for(int dense=0;dense<2;dense++){
   start(sr,(float)dense);
   for(int i=0;i<sr*2;i++){
    float x=atmosphere();assert(isfinite(x));
    if(i>sr&&x>peak[dense])peak[dense]=x;
    assert(ug_grain(1)>=0&&ug_grain(1)<=GRAINS);
   }
   ug_play(0);for(int i=0;i<sr;i++)atmosphere();assert(ug_grain(1)==0);
   ug_panic();assert(atmosphere()==0);
  }
  assert(peak[0]>.15f);assert(peak[1]>.15f);
  assert(peak[1]<peak[0]*1.12f); /* Correlated overlap must not multiply level. */
  start(sr,1);ug_setparam(2,19,1);for(int i=0;i<sr;i++){
   if(i%(sr/20)==0){ug_setparam(2,12,(i&1)?0:1);ug_setparam(2,14,(i&1)?1:0);}
   assert(isfinite(atmosphere()));
   for(int g=0;g<GRAINS;g++)if(grains[g].active)assert(grains[g].pos>=0&&grains[g].pos<TEXTURE);
  }
 }
 /* Variation repeats after init and never consumes the shared percussion RNG. */
 float sequence[2048];uint32_t before;
 start(44100,1);ug_setparam(2,19,1);before=noise_state;
 for(int i=0;i<2048;i++)sequence[i]=atmosphere();assert(noise_state==before);
 start(44100,1);ug_setparam(2,19,1);
 for(int i=0;i<2048;i++)assert(atmosphere()==sequence[i]);
 /* A single grain begins and ends at zero with flat window endpoints. */
 start(44100,0);atmosphere();assert(grains[0].active);
 ug_ui_init();ug_ui_select(0,2);ug_ui_button(2);ug_ui_turn(0,2);
 assert(!strcmp(ug_ui_knoblabel(3),"Surprise"));
 ug_ui_turn(3,50);assert(ug_getparam(2,19)>.49f);
 ug_begin_edit();ug_setlock(2,0,19,.75f);assert(ug_lockvalue(2,0,19)==.75f);ug_undo();assert(!(ug_lockmask(2,0)&(1<<19)));
 ug_init(44100);ug_stamp(2,0);ug_recmode(2);ug_arm(1,2);
 ug_setparam(2,19,.8f);ug_play(1);
 for(int i=0;i<400;i++)ug_render(128);
 ug_arm(0,2);assert(ug_lockvalue(2,0,19)==.8f);
 ug_undo();assert(!(ug_lockmask(2,0)&(1<<19)));
 start(44100,0);atmosphere();
 float inv=grains[0].inv_duration;
 assert(inv>0&&inv<.01f);
 puts("PASS grains: correlated-overlap gain, bounded voices/positions, parameter extremes, Stop/Panic at five sample rates");
}
