#include "../core/undergrowth.h"
#include <stdio.h>
#include "../core/controller.h"
int main(void){
 ug_init(44100);ug_ui_init();ug_groove(0,.65f);ug_groove(1,.8f);ug_tempo(110);for(int s=0;s<16;s+=4)ug_setstep(0,s,1);for(int s=2;s<16;s+=3)ug_setstep(1,s,1);
 ug_setstep(2,0,1);ug_setstep(2,1,1);ug_note(0,36,1.5f,0);ug_note(1,43,.75f,1);ug_setlock(2,1,1,.8f);ug_setlock(1,5,0,.8f);ug_setlock(1,11,2,.5f);ug_setparam(1,4,.85f);ug_setparam(1,5,.75f);ug_setlock(1,5,6,.9f);ug_velocity(0,0,.5f);ug_velocity(1,2,.35f);ug_velocity(2,1,.6f);ug_play(1);
 for(int block=0;block<2048;block++){if(block==50){ug_ui_turn(1,1);ug_ui_button(2);ug_ui_turn(3,4);}if(block==200)ug_hit(1,.75f);if(block==350)ug_motion(1);if(block==500){ug_setparam(0,0,.7f);ug_setparam(1,3,.8f);}if(block==550)ug_setparam(1,4,.15f);if(block==650)ug_motion(0);if(block==700)ug_rejoin();if(block==1200){ug_groove(0,0);ug_groove(1,0);}if(block==1700)ug_play(0);ug_render(128);fwrite(ug_buffer(),sizeof(float),256,stdout);}return 0;
}
