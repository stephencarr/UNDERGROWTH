#include "../core/undergrowth.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
static void render(int n){while(n){int k=n>256?256:n;ug_render(k);for(int i=0;i<2*k;i++)assert(isfinite(ug_buffer()[i])&&fabsf(ug_buffer()[i])<=.95001f);n-=k;}}
static double measure(int rate,int voice,int parameter,float value,int tempo,float velocity,int *last){
 ug_init(rate);ug_tempo(tempo);ug_setparam(voice,3,0);ug_setparam(voice,parameter,value);render(rate/10);
 if(voice==2){ug_keyvelocity(36,1,velocity);render(rate/10);ug_key(36,0);}else ug_hit(voice,velocity);
 double energy=0;*last=0;
 for(int t=0;t<rate*4;t+=128){ug_render(128);for(int i=0;i<128;i++){float x=ug_buffer()[i*2];assert(isfinite(x)&&fabsf(x)<=.95001f);energy+=x*x;if(fabsf(x)>.0001f)*last=t+i;}}
 return energy;
}
int main(void){for(int rate=44100;rate<=48000;rate+=3900){
 int a,b;double lo,hi;
 measure(rate,1,4,0,110,1,&a);measure(rate,1,4,1,110,1,&b);assert(b>a*3);
 int fast,slow;measure(rate,1,4,1,220,1,&fast);measure(rate,1,4,1,40,1,&slow);assert(slow>fast&&slow<fast*1.2);
 lo=measure(rate,1,5,0,110,1,&a);hi=measure(rate,1,5,1,110,1,&b);assert(hi>lo*2);
 lo=measure(rate,1,6,1,110,1,&a);hi=measure(rate,1,6,0,110,1,&b);assert(hi>lo*2);
 lo=measure(rate,0,4,1,110,1,&a);hi=measure(rate,0,4,0,110,1,&b);assert(hi>lo*2);
 lo=measure(rate,2,2,0,110,1,&a);hi=measure(rate,2,2,1,110,1,&b);assert(hi>lo*5);
 for(int v=0;v<3;v++){
  ug_init(rate);assert(ug_getvelocity(v,-1)==1);ug_velocity(v,-1,.35f);ug_setparam(v,4,.8f);ug_stamp(v,0);
  assert(ug_getvelocity(v,0)==.35f&&ug_lockmask(v,0)==(v==2?2039:127)&&ug_lockvalue(v,0,4)==.8f);
  ug_begin_edit();ug_velocity(v,0,.9f);ug_undo();assert(ug_getvelocity(v,0)==.35f);ug_undo();assert(ug_getvelocity(v,0)==.9f);
  ug_recmode(2);ug_arm(1,v);ug_setparam(v,6,.1f);ug_play(1);render(rate*3);ug_arm(0,v);
  assert(ug_lockvalue(v,0,6)==.1f&&!ug_step(v,1)&&ug_getvelocity(v,0)==.9f);ug_undo();assert(ug_lockvalue(v,0,6)==.5f);
  ug_play(0);ug_clear();ug_recmode(1);ug_arm(1,v);ug_play(1);render(1);
  if(v==2)ug_keyvelocity(43,1,.23f);else ug_hit(v,.23f);
  assert(fabsf(ug_getvelocity(v,0)-.23f)<.001f);ug_arm(0,v);ug_undo();assert(!ug_step(v,0));
  ug_init(rate);ug_velocity(v,-1,0);ug_stamp(v,0);ug_play(1);ug_render(256);for(int i=0;i<512;i++)assert(ug_buffer()[i]==0);
 }
 ug_init(rate);ug_velocity(1,-1,.4f);ug_stamp(1,0);ug_groove(1,1);ug_play(1);render(rate*5);assert(ug_getvelocity(1,0)==.4f);ug_panic();ug_render(256);for(int i=0;i<512;i++)assert(ug_buffer()[i]==0);
 /* Ordinary taps use the draft, including overdub over an existing event. */
 ug_init(rate);ug_velocity(0,-1,.3f);ug_stamp(0,0);ug_recmode(1);ug_arm(1,0);ug_play(1);render(1);ug_hit(0,.7f);ug_arm(0,0);assert(ug_getvelocity(0,0)==.7f);ug_undo();assert(ug_getvelocity(0,0)==.3f);
 /* Adding steps on another lane must preserve a playing track's ENV takeover. */
 for(int source=0;source<4;source++)for(int target=0;target<4;target++)if(source!=target){
  ug_init(rate);ug_stamp(source,0);ug_stamp(source,4);ug_play(1);render(1);
  for(int p=4;p<7;p++)ug_setparam(source,p,.95f);
  ug_toggle_authored(target,0);ug_toggle_authored(target,4);
  for(int i=0;i<16;i++){
   for(int p=4;p<7;p++)assert(fabsf(ug_effective(source,p)-.95f)<.001f);
   render(rate/8);
  }
  assert(ug_lockvalue(source,0,4)==.5f); /* Takeover must not rewrite authored locks. */
  ug_play(0);ug_play(1);render(1);ug_rejoin();
  assert(fabsf(ug_effective(source,4)-.95f)>.1f);
 }
 /* Extreme controls and repeated retriggers remain bounded. */
 ug_init(rate);for(int v=0;v<3;v++)for(int p=0;p<7;p++)ug_setparam(v,p,1);for(int i=0;i<40;i++){ug_hit(0,1);ug_hit(1,1);ug_keyvelocity(60,1,1);render(rate/20);}ug_panic();render(rate);
 }puts("PASS ENV/velocity: pronounced ranges, bounded tempo DNA, trig snapshots, scoped motion, recording, Human preservation, Undo, silence and stress at 44.1/48 kHz");}
