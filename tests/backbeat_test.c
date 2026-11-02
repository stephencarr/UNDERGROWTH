#include "../core/undergrowth.h"
#include "../core/controller.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
static void render(int n){while(n){int k=n>128?128:n;ug_render(k);for(int i=0;i<k*2;i++)assert(isfinite(ug_buffer()[i])&&fabsf(ug_buffer()[i])<=.95001f);n-=k;}}
static double energy(int sr,int p,float x,float velocity,int late){
 ug_init(sr);ug_setparam(3,3,0);ug_setparam(3,p,x);ug_setparam(3,6,1);render(sr/10);ug_hit(3,velocity);
 double sum=0;for(int t=0;t<sr;t+=128){ug_render(128);for(int i=0;i<128;i++){float a=ug_buffer()[i*2];assert(isfinite(a));if(t+i>=late)sum+=a*a;}}return sum;
}
int main(void){for(int sr=44100;sr<=48000;sr+=3900){
 ug_init(sr);assert(ug_state(23)==4);render(sr);assert(ug_buffer()[0]==0);ug_play(1);render(sr);assert(!ug_state(21));ug_panic();
 double loud=energy(sr,1,.8f,1,0),soft=energy(sr,1,.8f,.2f,0);assert(loud>1&&loud>soft*10);
 double short_tail=energy(sr,2,0,1,sr/10),long_tail=energy(sr,2,1,1,sr/10);assert(long_tail>short_tail*10);
 double damp=energy(sr,4,1,1,0),ring=energy(sr,4,0,1,0);assert(ring>damp*2);
 // Fully noisy, short, damped settings expose the late clap pulses from Spread.
 double pulse[2];for(int spread=0;spread<2;spread++){
  ug_init(sr);ug_setparam(3,1,1);ug_setparam(3,2,0);ug_setparam(3,3,0);ug_setparam(3,4,1);ug_setparam(3,5,(float)spread);ug_setparam(3,6,1);render(sr/10);ug_hit(3,1);pulse[spread]=0;
  for(int t=0;t<sr/3;t+=128){ug_render(128);for(int i=0;i<128;i++)if(t+i>sr/30){float a=ug_buffer()[i*2];pulse[spread]+=a*a;}}
 }assert(pulse[1]>pulse[0]*10);
 ug_init(sr);ug_ui_init();ug_ui_select(0,3);ug_ui_button(2);ug_ui_refresh();assert(ug_ui_state(2)==3);ug_ui_button(9);ug_ui_key(0,1,-1);ug_ui_key(0,0,-1);assert(ug_step(3,0)&&ug_lockmask(3,0)==127);assert(!ug_step(0,0)&&!ug_step(1,0)&&!ug_step(2,0));
 ug_ui_key(0,1,-1);ug_ui_tick(351);ug_ui_key(0,0,-1);ug_ui_edit(5,.9f);ug_ui_button(9);ug_ui_key(0,1,-1);ug_ui_key(0,0,-1);assert(!ug_step(3,0)&&fabsf(ug_lockvalue(3,0,5)-.9f)<.001f);ug_ui_key(0,1,-1);ug_ui_key(0,0,-1);assert(ug_step(3,0));
 ug_recmode(2);ug_arm(1,3);ug_setparam(3,4,.1f);ug_play(1);render(1);assert(ug_state(21)==1&&fabsf(ug_lockvalue(3,0,4)-.1f)<.001f);assert(!ug_step(3,1));ug_arm(0,3);ug_play(0);ug_undo();assert(fabsf(ug_lockvalue(3,0,4)-.5f)<.001f);
 ug_panic();render(sr);for(int i=0;i<128;i++)assert(ug_buffer()[i]==0);
 // All four organisms run together under dense patterns, then Panic clears tails.
 ug_init(sr);ug_master(1);for(int v=0;v<4;v++){for(int p=0;p<7;p++)ug_setparam(v,p,1);for(int s=0;s<16;s++)ug_setstep(v,s,1);}ug_play(1);render(sr*4);ug_panic();render(sr);assert(ug_buffer()[0]==0);
 }puts("PASS Backbeat: silence, velocity, length/damping ranges, clap Spread, independent fourth lane, trig preservation, scoped ENV motion/Undo, dense mix bounds and Panic at 44.1/48 kHz");}
