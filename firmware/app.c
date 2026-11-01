/* Original UNDERGROWTH FM1 adapter. C core/controller own all musical state.
 * Main loop enqueues input; the audio interrupt consumes it. No flash writes in this app. */
#include "undergrowth.h"
#include "controller.h"
#define UG_QUEUE 64u
typedef struct { int type,a,b; float value; } UgEvent;
static volatile uint32_t ug_head,ug_tail,ug_overflow,ug_dropped;
static UgEvent ug_events[UG_QUEUE];
static uint32_t ug_keys,ug_buttons,ug_home_since,ug_home_fired,ug_snapshot_clock;
typedef struct { char lines[9][64];uint32_t buttons,keys,dimkeys; } UgScreen;
static UgScreen ug_screen,ug_drawn;
static int ug_drawn_valid;
static void ug_enqueue(int type,int a,int b,float value){
 uint32_t f=plat_lock(),next=(ug_head+1u)%UG_QUEUE;
 if(next==ug_tail){ug_overflow=1;ug_dropped++;}else{ug_events[ug_head]=(UgEvent){type,a,b,value};ug_head=next;}
 plat_unlock(f);
}
static void ug_input(void){
 touch_poll();uint32_t keys=touch_keys(),buttons=plat_buttons(),now=plat_ms();
 for(int i=0;i<27;i++){uint32_t bit=1u<<i;if((keys^ug_keys)&bit)ug_enqueue(2,i,(keys&bit)!=0,(float)touch_vel(i)/127.0f);}
 for(int b=0;b<NB;b++)if((buttons&(1u<<b))&&!(ug_buttons&(1u<<b)))ug_enqueue(0,b,0,0);
 if((buttons&(1u<<B_HOME))&&!(ug_buttons&(1u<<B_HOME))){ug_home_since=now;ug_home_fired=0;}
 if((buttons&(1u<<B_HOME))&&!ug_home_fired&&now-ug_home_since>=650){ug_enqueue(3,0,0,0);ug_home_fired=1;}
 for(int e=0;e<NE;e++){int d=plat_enc(e);if(d)ug_enqueue(1,e,d,0);}
 ug_keys=keys;ug_buttons=buttons;
}
void app_input_only(void){ug_input();}
void app_init(void){ug_init(44100);ug_ui_init();}
void app_panic(void){uint32_t f=plat_lock();ug_tail=ug_head;ug_panic();plat_unlock(f);}
void app_render(int32_t *lr,uint32_t n,uint32_t master_q12){
 if(ug_overflow){ug_tail=ug_head;ug_overflow=0;ug_panic();ug_ui_init();}
 for(unsigned count=0;count<UG_QUEUE&&ug_tail!=ug_head;count++){
  UgEvent e=ug_events[ug_tail];ug_tail=(ug_tail+1u)%UG_QUEUE;
  if(e.type==0)ug_ui_button(e.a);else if(e.type==1)ug_ui_turn(e.a,e.b);else if(e.type==2)ug_ui_key(e.a,e.b,e.value);else ug_panic();
 }
 uint32_t packet;int port;
 for(int i=0;i<32&&plat_midi_in(&packet,&port);i++){
  int status=(packet>>8)&255,note=(packet>>16)&127,vel=(packet>>24)&127,ch=status&15;
  if((status&240)==144&&vel){if(ch<2)ug_hit(ch,vel/127.0f);else if(ch==2)ug_keyvelocity(note,1,vel/127.0f);}
  else if(((status&240)==128||((status&240)==144&&!vel))&&ch==2)ug_key(note,0);
  else if((status&240)==176&&(note==120||note==123))ug_panic();
 }
 ug_master((float)master_q12/4096.0f);
 for(uint32_t off=0;off<n;off+=256){uint32_t frames=n-off;if(frames>256)frames=256;ug_ui_tick(frames*(1000.0f/44100.0f));ug_render((int)frames);float *audio=ug_buffer();for(uint32_t i=0;i<frames*2;i++)lr[off*2+i]=(int32_t)(audio[i]*8388607.0f);}
 if((ug_snapshot_clock++&3u)==0){
  ug_ui_refresh();for(int i=0;i<9;i++){const char *src=ug_ui_line(i);for(int j=0;j<64;j++){ug_screen.lines[i][j]=src[j];if(!src[j])break;}}
  ug_screen.buttons=(uint32_t)ug_ui_state(6);ug_screen.keys=ug_screen.dimkeys=0;
  static const uint8_t white[16]=PLAT_WHITE_KEYS;int v=ug_ui_state(2);
  for(int i=0;i<16;i++){if(ug_step(v,i))ug_screen.keys|=1u<<white[i];if(ug_state(0)&&ug_state(1)==i)ug_screen.dimkeys|=1u<<white[i];}
 }
}
void app_ui_init(void){ug_drawn_valid=0;touch_on(1);lcd_fill(0,0,240,240,RGB(184,208,189));}
void app_say(const char *a,const char *b){cv_begin(240,24,RGB(184,208,189));cv_text(3,3,&FONT_XS,a?a:"",RGB(20,46,35));cv_blit(0,0);cv_begin(240,24,RGB(184,208,189));cv_text(3,3,&FONT_XS,b?b:"",RGB(20,46,35));cv_blit(0,24);}
void app_frame(void){
 ug_input();UgScreen screen;uint32_t f=plat_lock();screen=ug_screen;plat_unlock(f);
 if(!ug_drawn_valid||memcmp(screen.lines,ug_drawn.lines,sizeof screen.lines)){
 ug_drawn=screen;ug_drawn_valid=1;
 for(int i=0;i<9;i++){cv_begin(240,24,RGB(184,208,189));cv_text(4,4,&FONT_XS,screen.lines[i],RGB(20,46,35));cv_blit(0,i*24);}
 cv_begin(240,24,RGB(184,208,189));cv_text(4,4,&FONT_XS,"EXPERIMENTAL / SESSION ONLY",RGB(20,46,35));cv_blit(0,216);
 }
 plat_lights(screen.buttons,screen.keys|screen.dimkeys,0,0,0);
}
#ifdef PLAT_HOST
int app_expect(const char *what,int *got){
 if(!strcmp(what,"page"))*got=ug_ui_state(0);else if(!strcmp(what,"voice"))*got=ug_ui_state(2);else if(!strcmp(what,"sub"))*got=ug_ui_state(1);else if(!strcmp(what,"step"))*got=ug_ui_state(3);else if(!strcmp(what,"playing"))*got=ug_state(0);else if(!strcmp(what,"armed"))*got=ug_state(18);else if(!strcmp(what,"hit"))*got=ug_step(ug_ui_state(2),ug_ui_state(3));else if(!strcmp(what,"velocity"))*got=(int)(ug_getvelocity(ug_ui_state(2),ug_ui_state(3))*100+.5f);else if(!strcmp(what,"dropped"))*got=(int)ug_dropped;else return 0;return 1;
}
#endif
