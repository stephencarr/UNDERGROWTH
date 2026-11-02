/* White-box impulse response: diffusion and decay, independent of voice RMS. */
#include "../core/undergrowth.c"
#include <assert.h>
#include <stdio.h>
#include <math.h>
int main(void){for(int r=8000;r<=96000;r+=(r==8000?36100:r==44100?3900:48000))for(int d=0;d<3;d++){
 ug_init(r);ug_setfx(0,1);ug_setfx(1,(float[]){.25f,.55f,1}[d]);float l,h;for(int i=0;i<r;i++)room_render(0,&l,&h);
 double bins[10]={0};int populated=0,total=0;float peak=0;
 for(int i=0;i<r*10;i++){room_render(i==0?1:0,&l,&h);float e=l*l+h*h;bins[i/r]+=e;if(i>r/5&&i<r){total++;populated+=e>1e-12f;}if(fabsf(l)>peak)peak=fabsf(l);}
 assert((double)populated/total>.99);assert(bins[1]>bins[0]*.01);assert(bins[9]<bins[1]);assert(peak<.5f);
 printf("%d decay %.2f occupancy .2-1s %.5f peak %.5f energy",r,(float[]){.25f,.55f,1}[d],(double)populated/total,peak);for(int i=0;i<10;i++)printf(" %.6g",bins[i]);puts("");
}}
