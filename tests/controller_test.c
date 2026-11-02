#include "../core/undergrowth.h"
#include "../core/controller.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static void init(void){ug_init(44100);ug_ui_init();}
static void tap(int key){ug_ui_key(key,1,-1);ug_ui_key(key,0,-1);}
static void hold(int key){ug_ui_key(key,1,-1);ug_ui_tick(351);ug_ui_key(key,0,-1);}
static void review_regressions(void){
 // Notes, velocity and locks edited on a rest survive activation and subsequent off/on toggles.
 init();ug_ui_select(0,2);ug_ui_button(9);hold(0);ug_ui_turn(3,15);
 float body=ug_lockvalue(2,0,0);ug_ui_turn(0,1);ug_ui_turn(4,10);float glide=ug_lockvalue(2,0,4);
 ug_ui_turn(0,1);ug_ui_turn(3,7);ug_ui_turn(4,-4);ug_ui_turn(5,1);ug_ui_turn(6,-25);ug_ui_button(9);
 for(int n=0;n<3;n++){tap(0);assert(ug_notevalue(0,0)==43);assert(ug_notevalue(0,2)==1);assert(ug_getvelocity(2,0)>.749f&&ug_getvelocity(2,0)<.751f);assert(ug_lockvalue(2,0,0)==body&&ug_lockvalue(2,0,4)==glide);}
 assert(ug_step(2,0));tap(2);assert(ug_step(2,1)&&ug_notevalue(1,0)==ug_draftvalue(0));
 // Overlapping taps commit independently, including a canceled second touch.
 for(int v=0;v<3;v++){
  init();ug_ui_select(0,v);ug_ui_button(9);ug_ui_key(0,1,-1);ug_ui_tick(50);ug_ui_key(2,1,-1);ug_ui_tick(50);ug_ui_key(0,0,-1);ug_ui_key(2,0,-1);assert(ug_step(v,0)&&ug_step(v,1));
  ug_ui_key(4,1,-1);ug_ui_key(6,1,-1);ug_ui_key(6,-1,-1);ug_ui_key(4,0,-1);assert(ug_step(v,2)&&!ug_step(v,3));
 }
 // Reselecting the same target must not replace the whole-menu Undo snapshot.
 init();ug_ui_button(9);tap(0);hold(0);float b=ug_lockvalue(0,0,0),e=ug_lockvalue(0,0,1);ug_ui_edit(0,b+.1f);tap(0);ug_ui_turn(4,20);ug_ui_button(9);ug_undo();assert(ug_lockvalue(0,0,0)==b&&ug_lockvalue(0,0,1)==e);
 // A tap already pending when another key reaches hold age resolves before menu editing begins.
 init();ug_ui_button(9);tap(0);float original=ug_lockvalue(0,0,0);
 ug_ui_key(0,1,-1);ug_ui_tick(260);ug_ui_key(2,1,-1);ug_ui_tick(91);assert(!ug_ui_state(7));
 ug_ui_key(2,0,-1);ug_ui_tick(1);assert(ug_ui_state(7)&&ug_step(0,1));ug_ui_edit(0,.8f);ug_ui_key(0,0,-1);ug_ui_button(9);ug_undo();assert(ug_lockvalue(0,0,0)==original&&ug_step(0,1));
 // Opening lock editing releases manual takeover, so the edit can be heard while playing.
 ug_play(1);ug_render(1);ug_setparam(0,0,.9f);assert(ug_effective(0,0)==.9f);hold(0);assert(!ug_state(6));ug_ui_turn(3,5);assert(ug_effective(0,0)==ug_lockvalue(0,0,0));
}
int main(void){
 review_regressions();
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
