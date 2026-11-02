#include "../core/undergrowth.h"
#include "../core/controller.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
static double render(int n){double e=0;while(n>0){int b=n>256?256:n;ug_render(b);for(int i=0;i<b*2;i++){float x=ug_buffer()[i];assert(isfinite(x)&&fabsf(x)<=.95001f);e+=x*x;}n-=b;}return e;}
static int active(void){return (int)(ug_chordtone(0,0)+ug_chordtone(1,0));}
static void sound_character(void){
 ug_init(44100);ug_setparam(4,0,1);ug_setparam(4,1,0);ug_setparam(4,2,1);ug_setparam(4,6,0);ug_chordkey(48,1,1);render(4410);
 static float samples[8192];for(int i=0;i<8192;){int n=8192-i;if(n>256)n=256;ug_render(n);for(int k=0;k<n;k++)samples[i++]=ug_buffer()[k*2];}
 double powers[5];const int notes[5]={48,52,55,60,49};
 for(int j=0;j<5;j++){double re=0,im=0,hz=440*pow(2,(notes[j]-69)/12.0);for(int i=0;i<8192;i++){double w=.5-.5*cos(6.28318530718*i/8191),p=6.28318530718*hz*i/44100;re+=samples[i]*w*cos(p);im+=samples[i]*w*sin(p);}powers[j]=re*re+im*im;}
 for(int j=0;j<4;j++)assert(powers[j]>powers[4]*10); /* Actual spectrum contains all four tones. */
 ug_init(44100);ug_setparam(4,2,1);ug_setparam(4,4,1);ug_chordkey(48,1,1);double early=render(4410);render(44100);double late=render(4410);assert(late>early*20);
 ug_setparam(4,5,1);ug_chordkey(48,0,0);render(44100);assert(render(4410)>.001);ug_panic();assert(render(256)==0);
 /* A one-shot audition ends even at maximum sustained Decay. */
 ug_init(44100);ug_setparam(4,2,1);ug_hit(4,1);render(44100*6);assert(active()==0);
}
int main(void){
 sound_character();
 for(int r=8000;r<=96000;r+=(r==8000?36100:r==44100?3900:48000)){
  ug_init(r);assert(ug_state(23)==5&&render(r)==0);ug_play(1);assert(render(r)==0);ug_play(0);
  ug_chordkey(48,1,1);assert(active()==1);assert(ug_chordtone(0,1)==48&&ug_chordtone(0,2)==52&&ug_chordtone(0,3)==55);
  double live=render(r/4);assert(live>.001);ug_chordkey(48,0,0);render(r*6);assert(active()==0);ug_panic();assert(render(256)==0);
  /* Every palette/voicing corner remains chordal, bounded and playable. */
  for(int slot=0;slot<8;slot++)for(int inv=0;inv<4;inv++){
   ug_panic();ug_setparam(4,8,slot/7.0f);ug_setparam(4,9,inv/3.0f);ug_setparam(4,10,1);ug_chordkey(72,1,1);assert(active()<=2);
   int b=ug_chordtone(0,0)?0:1;assert(ug_chordtone(b,1)!=ug_chordtone(b,2)||ug_chordtone(b,1)!=ug_chordtone(b,3));render(r/100);
  }
  ug_panic();ug_setparam(4,9,1);ug_setparam(4,10,1);for(int p=13;p<16;p++)ug_setparam(4,p,1);ug_chordkey(72,1,1);
  assert(ug_chordtone(0,1)!=ug_chordtone(0,2)||ug_chordtone(0,1)!=ug_chordtone(0,3)||ug_chordtone(0,1)!=ug_chordtone(0,4));
  for(int i=0;i<40;i++){ug_chordkey(48+i%12,1,1);render(32);assert(active()<=2);}
  ug_play(0);render(r*6);assert(active()==0);ug_panic();assert(render(256)==0);
 }
 ug_init(44100);ug_setparam(4,8,1.0f/7);ug_stamp(4,0);float minor=ug_lockvalue(4,0,13);assert(fabsf(minor-2.0f/23)<.00001f);
 ug_begin_edit();ug_setparam(4,13,8.0f/23);assert(ug_lockvalue(4,0,13)==minor);ug_undo();assert(ug_getparam(4,13)==minor);
 ug_setparam(4,8,0);assert(fabsf(ug_getparam(4,13)-3.0f/23)<.00001f);ug_play(1);render(1);assert(ug_chordtone(0,2)==51);ug_play(0);
 /* Slot selection in a trig captures its actual definition; later palette edits don't rewrite it. */
 ug_setlock(4,0,8,0);float major=ug_lockvalue(4,0,13);ug_setparam(4,13,10.0f/23);assert(ug_lockvalue(4,0,13)==major);
 assert(ug_stepcopy(4,0));ug_begin_edit();assert(ug_steppaste(4,4));assert(ug_lockmask(4,4)==ug_lockmask(4,0));assert(!ug_steppaste(2,4));ug_undo();assert(!ug_step(4,4));
 ug_setlock(4,1,13,.8f);assert(!ug_step(4,1));ug_audition(4,1);assert(active()>0);render(44100);ug_panic();assert(render(256)==0);
 /* Hits/Both record the actual root and harmony. Motion alone leaves rests alone. */
 ug_init(44100);ug_recmode(3);ug_arm(1,4);ug_play(1);render(1);ug_setparam(4,1,.9f);ug_chordkey(55,1,.4f);ug_arm(0,4);assert(ug_step(4,0));assert(fabsf(ug_lockvalue(4,0,7)-19.0f/36)<.00001f);assert(ug_lockvalue(4,0,1)==.9f);assert(ug_getvelocity(4,0)==.4f);ug_undo();assert(!ug_step(4,0));ug_panic();
 ug_stamp(4,0);ug_recmode(2);ug_arm(1,4);ug_setparam(4,4,.6f);ug_play(1);render(44100);ug_arm(0,4);assert(ug_lockvalue(4,0,4)==.6f);assert(!ug_step(4,1));ug_undo();assert(ug_lockvalue(4,0,4)<.02f);
 /* Real panel paths: fifth-track selection, draft, step/harmony/palette pages and mixer. */
 ug_init(44100);ug_ui_init();ug_ui_turn(1,4);assert(ug_ui_state(2)==4);ug_ui_button(4);ug_ui_turn(0,2);ug_ui_turn(3,1);assert(fabsf(ug_getparam(4,8)-1.0f/7)<.0001f);
 ug_ui_button(9);ug_ui_key(0,1,1);ug_ui_key(0,0,1);assert(ug_step(4,0));ug_ui_turn(3,7);assert(fabsf(ug_lockvalue(4,0,7)-19.0f/36)<.0001f);
 ug_ui_key(2,1,1);ug_ui_tick(351);ug_ui_key(2,0,1);ug_ui_turn(0,2);ug_ui_turn(3,1);assert(!ug_step(4,1));assert(ug_lockmask(4,1)&(1<<13));
 ug_ui_button(6);ug_ui_key(7,1,1);ug_ui_key(7,0,1);assert(ug_getmute(4));ug_ui_turn(0,3);ug_ui_turn(3,-30);assert(fabsf(ug_getlevel(4)-.7f)<.001f);
 puts("PASS CHORD: four-tone harmony, 8 editable slots, voicing/range, immutable trig definitions, copy/Undo/rests, scoped hits/motion, panel and mixer, silence/Panic and finite 8-96kHz tails");
 ug_init(44100);for(int v=0;v<5;v++)for(int st=0;st<16;st++)ug_stamp(v,st);ug_setparam(2,17,1);ug_setparam(2,16,1);ug_setparam(4,3,1);ug_setfx(0,1);ug_play(1);clock_t begin=clock();render(441000);printf("HOST dense 5-track/grains/reverb: 10s in %.4fs CPU (not device headroom)\n",(double)(clock()-begin)/CLOCKS_PER_SEC);
}
