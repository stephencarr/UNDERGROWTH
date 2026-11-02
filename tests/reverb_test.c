#include "../core/undergrowth.h"
#include "../core/controller.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <time.h>
static double stereo;
static double render(int frames){double energy=0;while(frames>0){int n=frames>256?256:frames;ug_render(n);for(int i=0;i<n;i++){float a=ug_buffer()[2*i],b=ug_buffer()[2*i+1];assert(isfinite(a)&&isfinite(b)&&fabsf(a)<=.95001f&&fabsf(b)<=.95001f);energy+=a*a+b*b;stereo+=fabsf(a-b);}frames-=n;}return energy;}
static double tail(int rate,float wet,float send,float decay,float width){ug_init(rate);ug_setfx(0,wet);ug_setfx(1,decay);ug_setfx(3,width);ug_setfx(4,0);ug_setparam(0,2,0);ug_setparam(0,4,1);ug_setparam(0,3,send);ug_hit(0,1);render(rate/2);return render(rate);}

/* Full send/return must produce a clearly comparable wet layer on both sources. */
static float reference[44100];
static void presence_source(int layer,int wet){ug_init(44100);ug_setfx(0,wet);ug_setfx(4,0);if(layer){ug_setparam(2,17,1);ug_setparam(2,16,1);ug_play(1);}else{ug_setparam(2,3,1);ug_key(36,1);}render(44100);}
static void wet_presence(int layer){
 presence_source(layer,0);for(int at=0;at<44100;){int n=44100-at;if(n>256)n=256;ug_render(n);for(int i=0;i<n;i++)reference[at+i]=ug_buffer()[2*i];at+=n;}
 presence_source(layer,1);double dry=0,wet=0;for(int at=0;at<44100;){int n=44100-at;if(n>256)n=256;ug_render(n);for(int i=0;i<n;i++){float d=reference[at+i],w=ug_buffer()[2*i];dry+=d*d;wet+=w*w;}at+=n;}
 printf("Full-return balance %s: wet/dry %.2f dB\n",layer?"ENV":"ROOT",10*log10(wet/dry));assert(wet>dry*.15&&wet<dry*4);
}
int main(void){
 wet_presence(0);wet_presence(1);
 /* A settled full send + full Reverb removes the direct attack before the
  * first room reflection. A zero-send track must retain its direct attack. */
 ug_init(44100);ug_setfx(0,1);ug_setfx(4,0);ug_setparam(0,3,1);render(4410);ug_hit(0,1);assert(render(882)<1e-6);
 ug_panic();ug_setparam(0,3,0);render(4410);ug_hit(0,1);assert(render(882)>.01);
 const int rates[4]={8000,44100,48000,96000};for(int r=0;r<4;r++){int rate=rates[r];
  ug_init(rate);ug_setfx(0,1);assert(render(rate)==0);ug_play(1);assert(render(rate)==0);
  double dry=tail(rate,0,1,1,1),no_send=tail(rate,1,0,1,1);assert(dry<1e-8&&no_send<1e-8);
  double short_tail=tail(rate,1,1,0,1);stereo=0;double long_tail=tail(rate,1,1,1,1);assert(long_tail>short_tail*2&&long_tail>1e-5&&stereo>1e-5);
  render(rate*20);assert(render(rate)<1e-7);ug_panic();assert(render(256)==0);
  tail(rate,1,1,1,0);stereo=0;render(rate/2);assert(stereo<1e-6);
  ug_init(rate);ug_setfx(0,1);ug_setfx(1,1);ug_setfx(4,0);ug_setparam(2,17,1);ug_setparam(2,16,1);ug_play(1);render(rate);ug_play(0);render(rate/2);assert(render(rate)>1e-6);ug_panic();assert(render(256)==0);
  ug_init(rate);ug_setfx(0,1);ug_setfx(1,1);ug_setfx(2,1);for(int v=0;v<4;v++)for(int p=0;p<7;p++)ug_setparam(v,p,1);
  for(int i=0;i<32;i++){for(int v=0;v<4;v++)ug_hit(v,1);render(rate/16);}ug_setfx(0,0);render(rate);ug_panic();ug_setfx(0,1);assert(render(rate)==0);
 }
 ug_init(44100);ug_ui_init();ug_ui_button(0);ug_ui_turn(0,1);ug_ui_turn(3,60);assert(fabsf(ug_getfx(0)-.6f)<.001f);ug_undo();assert(ug_getfx(0)==0);ug_undo();assert(fabsf(ug_getfx(0)-.6f)<.001f);
 ug_ui_button(11);ug_ui_button(0);ug_ui_turn(0,1);float before=ug_getfx(1);ug_ui_turn(4,10);assert(ug_state(18)&&ug_getfx(1)==before);ug_ui_button(11);
 puts("PASS reverb: stereo tails, decay/width, dry and send isolation, ENV send, silence, extreme 8/96kHz bounds, Panic/bypass clearing, global Undo and REC guard");
 for(int enabled=0;enabled<2;enabled++){ug_init(44100);ug_setparam(2,17,1);ug_setparam(2,13,1);ug_setparam(2,12,1);ug_setparam(2,16,1);ug_setfx(0,enabled);for(int v=0;v<4;v++)for(int st=0;st<16;st++)ug_setstep(v,st,1);ug_play(1);clock_t start=clock();render(441000);printf("HOST comparison: grains ON, reverb %s, 10s rendered in %.4f CPU seconds (not FM1 CPU)\n",enabled?"ON":"OFF",(double)(clock()-start)/CLOCKS_PER_SEC);}
}
