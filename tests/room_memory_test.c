/* Compact hardware arena must preserve the 44.1 kHz impulse bit-for-bit. */
#include "../core/undergrowth.c"
#include <assert.h>
#include <stdio.h>
int main(void){
 ug_init(44100);int used=0;for(int i=0;i<10;i++)used+=room.length[i];
 assert(used<=ROOM_SAMPLES);
#ifdef UG_FM1
 assert(sizeof room_buffer<=91000);
#endif
 fprintf(stderr,"PASS reverb arena: %d/%d samples, %zu bytes\n",used,ROOM_SAMPLES,sizeof room_buffer);
 ug_setfx(0,1);ug_setfx(1,1);
 for(int i=0;i<44100*4;i++){float out[2];room_render(i==0?1:0,out,out+1);fwrite(out,sizeof(float),2,stdout);}
 return 0;
}
