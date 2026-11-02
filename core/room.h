/* C adaptation of Mutable Instruments Clouds reverb.h / fx_engine.h,
 * eurorack f4cfa7697095a28d292bff758beb51deb49b5a86.
 * Copyright 2014 Emilie Gillet. MIT: ../LICENSES/Mutable-Instruments-MIT.txt.
 * Changes: explicit float circular lines, sample-rate scaling, tank modulation,
 * bounded decay mapping, aux return/width, silence bypass and Panic integration.
 * Griesinger/Dattorro topology: 4 input APs, cross-coupled 2x(2 AP + delay).
 */
// Copyright 2014 Emilie Gillet.
//
// Author: Emilie Gillet (emilie.o.gillet@gmail.com)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
// See http://creativecommons.org/licenses/MIT/ for more information.
//

static void room_clear(void){
 zero(room_buffer,sizeof room_buffer);zero(room.low,sizeof room.low);
 zero(room.position,sizeof room.position);zero(room.phase,sizeof room.phase);room.active=0;
}
static float room_read(int line,float delay){
 int pos=room.position[line];float at=pos-delay;
 if(at<0)at+=room.length[line];
 int a=(int)at,b=a+1;if(b>=room.length[line])b=0;
 float x=room_buffer[room.offset[line]+a];
 return x+(room_buffer[room.offset[line]+b]-x)*(at-a);
}
static void room_write(int line,float x){
 room_buffer[room.offset[line]+room.position[line]]=clamp(x,-8,8);
 if(++room.position[line]>=room.length[line])room.position[line]=0;
}
/* Same accumulator/previous-read allpass operation as Clouds FxEngine. */
static float room_ap(int line,float x,float coefficient){
 float delayed=room_read(line,(float)(room.length[line]-2));
 float written=x+coefficient*delayed;
 room_write(line,written);return delayed-coefficient*written;
}
static void room_render(float input,float *left,float *right){
 for(int p=0;p<5;p++)room.now[p]+=(score.fx[p]-room.now[p])*room.smoothing;
 *left=*right=0;
 if(score.fx[0]==0&&room.now[0]<.00001f){if(room.active)room_clear();room.now[0]=0;return;}
 room.active=1;
 const float diffusion=.625f;
 /* Useful room already at mid travel; long wash at the top, never freeze.
  * Two feedback traversals per complete tank loop. */
 float feedback=.62f+.34f*room.now[1];
 float damping=(.92f-.85f*room.now[2]);
 /* Keep the low-pass time constant approximately invariant with sample rate. */
 damping=damping/(damping+(1-damping)*(rate/32000.0f));
 float x=clamp(input,-2,2)*.35f;
 for(int i=0;i<4;i++)x=room_ap(i,x,diffusion);
 room.phase[0]+=.5f/rate;room.phase[1]+=.3f/rate;
 for(int i=0;i<2;i++)if(room.phase[i]>=1)room.phase[i]-=1;
 float scale=rate/32000.0f;
 float a=room_read(9,(4680+100*sine(room.phase[1]))*scale)*feedback+x;
 float b=room_read(6,(3330+50*sine(room.phase[0]))*scale)*feedback+x;
 room.low[0]+=(a-room.low[0])*damping;room.low[1]+=(b-room.low[1])*damping;
 for(int i=0;i<2;i++)if(room.low[i]<1e-12f&&room.low[i]>-1e-12f)room.low[i]=0;
 a=room_ap(4,room.low[0],-diffusion);a=room_ap(5,a,diffusion);
 b=room_ap(7,room.low[1],diffusion);b=room_ap(8,b,-diffusion);
 room_write(6,a);room_write(9,b);
 float middle=(a+b)*.5f,spread=(a-b)*.5f*room.now[3];
 *left=(middle+spread)*room.now[0]*2.8f;*right=(middle-spread)*room.now[0]*2.8f;
}
