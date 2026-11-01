#include "../core/undergrowth.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
static void frames(int n){while(n){int k=n>256?256:n;ug_render(k);n-=k;}}
static int count(int v){int n=0;for(int i=0;i<16;i++)n+=ug_step(v,i);return n;}
static int near(float a,float b){return fabsf(a-b)<.0001f;}
int main(void){for(int rate=44100;rate<=48000;rate+=3900){
 ug_init(rate);ug_setparam(0,0,.2f);ug_stamp(0,0);assert(ug_lockmask(0,0)==127);
 ug_setparam(0,0,.8f);ug_play(1);frames(1);ug_rejoin();assert(near(ug_effective(0,0),.2f));
 ug_stamp(0,4);assert(near(ug_lockvalue(0,4,0),.8f));assert(near(ug_lockvalue(0,0,0),.2f));
 ug_undo();assert(!ug_step(0,4));ug_undo();assert(ug_step(0,4));
 ug_erase(0,4);assert(!ug_step(0,4)&&!ug_lockmask(0,4));ug_undo();assert(ug_lockmask(0,4)==127);
 ug_play(0);ug_draftnote(43,.5f,1);ug_stamp(2,3);assert(ug_notevalue(3,0)==43&&near(ug_notevalue(3,1),.5f)&&ug_notevalue(3,2)==1);
 ug_init(rate);ug_recmode(3);ug_arm(1,0);ug_play(1);frames(1);ug_setparam(0,1,.9f);ug_hit(0,1);frames(rate/4);ug_hit(0,1);
 assert(count(0)==2&&ug_lockmask(0,0)==2);ug_hit(1,1);assert(count(1)==0);
 ug_stamp(0,7);assert(!ug_step(0,7));ug_arm(0,0);assert(!ug_state(2)&&!ug_state(10));
 ug_undo();assert(count(0)==0);ug_undo();assert(count(0)==2&&near(ug_lockvalue(0,0,1),.9f));
 ug_play(0);ug_recmode(2);ug_arm(1,0);ug_setparam(0,2,.3f);ug_play(1);frames(rate*2);ug_arm(0,0);
 assert(count(0)==2&&ug_lockmask(0,0)==6);ug_undo();assert(ug_lockmask(0,0)==2);
 ug_recmode(1);ug_arm(1,2);ug_draftnote(48,.4f,1);ug_key(48,1);assert(count(2)==1);ug_key(48,0);ug_play(0);assert(!ug_state(18));ug_undo();assert(!count(2));
 ug_arm(1,0);ug_recmode(2);assert(!ug_state(18));ug_arm(1,0);ug_panic();assert(!ug_state(18));
 }puts("PASS workflow: draft stamping, ROOT draft, erase, scoped Hits/Motion/Both, take Undo, transport at 44.1/48kHz");}
