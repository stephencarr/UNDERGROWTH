/* Audible differences are checked after removing overall gain. */
#include "../core/undergrowth.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#define N 16384
static float reference[N],changed[N];
static void capture(int parameter,float value,float *out){
 ug_init(44100);ug_setparam(0,17,1);ug_setparam(0,15,.8f);ug_setparam(0,13,0);
 ug_setparam(0,parameter,value);ug_play(1);
 for(int n=0;n<44160;n+=128)ug_render(128);
 for(int n=0;n<N;n+=128){ug_render(128);for(int i=0;i<128;i++)out[n+i]=ug_buffer()[i*2];}
}
static double difference(void){double aa=0,bb=0,ab=0;for(int i=0;i<N;i++){assert(isfinite(reference[i])&&isfinite(changed[i]));aa+=reference[i]*reference[i];bb+=changed[i]*changed[i];ab+=reference[i]*changed[i];}assert(aa>.001&&bb>.001);return 1-ab*ab/(aa*bb);}
int main(void){
 for(int p=11;p<=13;p++){
  capture(p,0,reference);capture(p,1,changed);double d=difference();
  printf("Halo parameter %d gain-independent difference %.3f\n",p,d);assert(d>.25);
 }
 capture(19,0,reference);capture(19,1,changed);assert(difference()>.1);
 /* No dependence on external effects or percussion score. */
 ug_init(44100);ug_setparam(0,17,1);ug_setparam(0,15,.8f);ug_play(1);
 for(int i=0;i<200;i++)ug_render(128);
 assert(ug_state(8)==0);for(int s=0;s<16;s++)assert(!ug_step(0,s));
 ug_panic();ug_render(128);for(int i=0;i<256;i++)assert(ug_buffer()[i]==0);
 puts("PASS halo: pronounced Structure/Texture/Movement/Roughness without reverb, no score events, Panic");
}
