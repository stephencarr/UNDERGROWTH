#include "../core/undergrowth.h"
#include <stdio.h>
int main(void){
 ug_init(44100);ug_groove(0,.65f);ug_groove(1,.8f);ug_tempo(110);for(int s=0;s<16;s+=4)ug_setstep(0,s,1);for(int s=2;s<16;s+=3)ug_setstep(1,s,1);
 ug_setlock(1,5,0,.8f);ug_setlock(1,11,2,.5f);ug_play(1);
 for(int block=0;block<2048;block++){if(block==200)ug_hit(1,.75f);if(block==500){ug_setparam(0,0,.7f);ug_setparam(1,3,.8f);}if(block==700)ug_rejoin();if(block==1200){ug_groove(0,0);ug_groove(1,0);}if(block==1700)ug_play(0);ug_render(128);fwrite(ug_buffer(),sizeof(float),256,stdout);}return 0;
}
