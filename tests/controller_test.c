#include "../core/undergrowth.h"
#include "../core/controller.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static void init(void){ug_init(44100);ug_ui_init();}
int main(void){
 assert(!strcmp(ug_ui_meter(-1,0,0,0),"CPU -- / FM1 only"));
 assert(!strcmp(ug_ui_meter(42,98,3,0),"CPU 42% P98% X3 EXP"));
 assert(!strcmp(ug_ui_meter(120,120,0,1),"HOST 120% P120% X0"));
 assert(strstr(ug_ui_meter(1,2,100000,0),"X99999+"));
 for(int v=0;v<3;v++){
  init();ug_ui_select(0,v);ug_ui_button(9);
  ug_ui_key(0,1,-1);ug_ui_key(0,0,-1);assert(ug_step(v,0));
  ug_ui_button(10);ug_ui_key(0,1,-1);ug_ui_key(0,0,-1);assert(!ug_step(v,0));
  ug_ui_key(0,1,-1);ug_ui_tick(351);ug_ui_key(0,0,-1);assert(ug_ui_state(7)&&!ug_step(v,0));
  ug_ui_turn(3,5);assert(!ug_step(v,0));ug_ui_button(9);
  ug_ui_key(0,1,-1);ug_ui_key(0,-1,-1);assert(!ug_step(v,0));
 }
 init();assert(ug_ui_state(0)==6&&!ug_state(0));
 ug_ui_button(2);assert(ug_ui_state(0)==2&&(ug_ui_state(6)&4));ug_ui_turn(4,20);assert(ug_getparam(0,4)>.69f);
 ug_ui_turn(0,1);ug_ui_turn(3,-40);assert(ug_getvelocity(0,-1)>.599f&&ug_getvelocity(0,-1)<.601f);
 ug_ui_button(9);ug_ui_key(0,1,-1);ug_ui_key(0,0,-1);assert(ug_step(0,0)&&ug_lockmask(0,0)==127);assert(ug_getvelocity(0,0)==ug_getvelocity(0,-1));
 // A short tap toggles an authored step; a hold never toggles it.
 ug_ui_key(0,1,-1);ug_ui_tick(100);ug_ui_key(0,0,-1);assert(!ug_step(0,0));ug_undo();assert(ug_step(0,0));
 ug_ui_key(0,1,-1);ug_ui_tick(349);assert(!ug_ui_state(7));ug_ui_tick(2);assert(ug_ui_state(7));ug_ui_key(0,0,-1);assert(ug_step(0,0));
 ug_ui_refresh();assert(strstr(ug_ui_line(0),"TRIG LOCK"));
 ug_ui_turn(3,10);ug_ui_turn(4,10);ug_ui_turn(0,1);assert(ug_ui_state(1)==3);ug_ui_turn(4,-10);
 ug_ui_turn(0,1);assert(ug_ui_state(1)==2);ug_ui_turn(3,-20);
 ug_undo();assert(ug_lockvalue(0,0,0)==ug_getparam(0,0)&&ug_lockvalue(0,0,1)==ug_getparam(0,1));assert(ug_getvelocity(0,0)==ug_getvelocity(0,-1));
 // A rest can be edited without creating a hit. SEQ leaves the menu; SELECT has no competing pages.
 ug_ui_key(2,1,-1);ug_ui_key(2,0,-1);assert(!ug_step(0,1));ug_ui_turn(3,-20);assert(!ug_step(0,1));
 ug_ui_button(9);assert(!ug_ui_state(7));ug_ui_turn(0,1);assert(ug_ui_state(1)==0&&ug_ui_state(5)==1);
 ug_ui_key(2,1,-1);ug_ui_key(2,-1,-1);ug_ui_key(2,0,-1);assert(!ug_step(0,1));
 // Navigation cancels a pending tap, and immediate knob edits promote it to the menu.
 ug_ui_key(2,1,-1);ug_ui_button(2);ug_ui_key(2,0,-1);assert(!ug_step(0,1));
 ug_ui_button(9);ug_ui_key(0,1,-1);ug_ui_turn(3,1);ug_ui_key(0,0,-1);assert(ug_ui_state(7)&&ug_step(0,0));
 ug_ui_button(11);assert(ug_state(18));ug_ui_turn(3,1);assert(!ug_state(18)&&ug_state(17)==2);ug_ui_button(11);ug_ui_button(2);ug_ui_turn(3,10);ug_ui_button(10);ug_render(1);assert(ug_state(18));ug_ui_button(11);assert(!ug_state(18));ug_ui_button(10);
 ug_ui_select(0,2);ug_ui_button(2);ug_ui_turn(0,1);ug_ui_turn(3,7);assert(ug_draftvalue(0)==43);ug_ui_button(1);assert(ug_state(16)==-1);ug_ui_tick(181);assert(ug_state(16)==0);
 ug_ui_refresh();assert(strstr(ug_ui_line(2),"G2"));ug_ui_button(9);ug_ui_key(6,1,-1);ug_ui_key(6,0,-1);assert(ug_step(2,3)&&ug_notevalue(3,0)==43);
 for(int page=0;page<14;page++){if(page==10||page==11||page==1||page>11)continue;ug_ui_button(page);ug_ui_button(10);assert(ug_state(0));ug_ui_button(10);assert(!ug_state(0));}
 ug_ui_button(-1);ug_ui_turn(99,1);ug_ui_key(-1,1,1);ug_ui_key(27,1,1);ug_ui_select(0,999);assert(ug_ui_state(2)==2);
 for(int v=0;v<3;v++){ug_ui_select(0,v);for(int b=0;b<14;b++){ug_ui_button(b);for(int sub=0;sub<4;sub++){ug_ui_turn(0,1);ug_ui_refresh();for(int line=0;line<9;line++)assert(strlen(ug_ui_line(line))<64);}}}
 ug_panic();puts("PASS shared controller: pages/lights, stamp/velocity, held-edit Undo, rests, REC, ROOT audition, transport and bounded screen text");
}
