/* Original UNDERGROWTH control logic, shared by the browser and FM1. No platform IO. */
#include "controller.h"
#include "undergrowth.h"
static int page,sub,voice,selected,octave,action,held,edited,gesture;
static float value,gesture_age,audition_ms;
static float press_ms[16];
static unsigned char presses[16];
static int lock_menu;
static void cancel_presses(void){for(int i=0;i<16;i++){presses[i]=0;press_ms[i]=0;}held=-1;}
static void select_step(int n){if(selected!=n)edited=0;selected=n;}
static int audition_note,notes[27];
static char lines[9][64];
static const int whites[16]={0,2,4,6,7,9,11,12,14,16,18,19,21,23,24,26};
static const int env[4][4]={{2,4,5,6},{4,5,6,2},{2,4,5,6},{2,4,5,6}};
static const char *labels[4][4]={{"Length","Damping","Drop","Response"},{"Stretch","Fracture","Grain","Length"},{"Release","Glide","Weight","Response"},{"Length","Damping","Spread","Response"}};
static const char *voices[4]={"KICK","CHATTER","ROOT","BACKBEAT"};
static const char *pages[15]={"FX","SEL","ENV","LFO","EDIT","GLO","HOME","SAVE","ARP","SEQUENCE","PLAY","REC","OCT-","OCT+","PRESETS"};
static const char *modes[4]={"","Hits","Motion","Both"};
static float bound(float x,float a,float b){return x<a?a:x>b?b:x;}
static int count(void){return lock_menu?3:page==9?1:page==6||page==0||page==2?2:1;}
static int busy(void){return ug_state(18)||ug_state(10)||ug_state(2);}
static int note_page(void){return (page==2&&sub==1)||(lock_menu&&sub==2);}
void ug_ui_init(void){page=6;sub=voice=selected=octave=action=edited=0;held=gesture=audition_note=-1;lock_menu=0;cancel_presses();value=gesture_age=audition_ms=0;for(int i=0;i<27;i++)notes[i]=-1;}
void ug_ui_gesture(int reset){if(reset||gesture_age>300)gesture=-1;gesture_age=0;}
void ug_ui_select(int field,int n){if(field==0&&n>=0&&n<4){voice=n;cancel_presses();edited=lock_menu=0;sub=0;}if(field==1&&n>=0&&n<16){select_step(n);}gesture=-1;}
static int pending_other(int key){for(int i=0;i<16;i++)if(i!=key&&presses[i]==1&&press_ms[i]<350)return 1;return 0;}
static void open_locks(void){ug_rejoin();lock_menu=1;if(held>=0)presses[held]=2;sub=0;gesture=-1;edited=0;}
static void snapshot(void){if(!lock_menu||!edited){ug_begin_edit();if(lock_menu)edited=1;}}
void ug_ui_edit(int p,float x){
 if(!lock_menu||busy()||p<0||p>11)return;
 if(p>=8&&p<=10&&voice!=2)return;
 snapshot();gesture=-1;
 if(p<7)ug_setlock(voice,selected,p,bound(x,0,1));
 else if(p==7)ug_velocity(voice,selected,x);
 else if(p==11)ug_clearlock(voice,selected);
 else {int n=(int)ug_notevalue(selected,0),s=(int)ug_notevalue(selected,2);float g=ug_notevalue(selected,1);if(p==8)n=(int)x;else if(p==9)g=x;else s=x!=0;ug_note(selected,n,g,s);}
}
static void macro(int p,int d){
 if(ug_state(18)&&ug_state(19)!=voice)return;
 int target=lock_menu?selected:held>=0?held:page==9&&sub==3?selected:-1,lock=target>=0&&!busy(),key=100+(lock?target+1:0)*8+p;
 int first=gesture!=key;
 if(first){gesture=key;value=lock&&(ug_lockmask(voice,target)&(1<<p))?ug_lockvalue(voice,target,p):ug_state(10)?ug_effective(voice,p):ug_getparam(voice,p);
  if(lock){if(lock_menu)snapshot();else if(held<0||!edited){ug_begin_edit();if(held>=0)edited=1;}}else if(!ug_state(18)&&!ug_state(10))ug_begin_edit();}
 value=bound(value+d*.01f,0,1);
 if(lock)ug_setlock(voice,target,p,value);else ug_setparam(voice,p,value);
}
void ug_ui_turn(int e,int d){
 if(e<0||e>6||!d)return;ug_ui_gesture(0);
 if(held>=0&&presses[held]==1&&!busy()){for(int i=0;i<16;i++)if(i!=held&&presses[i]==1)return;open_locks();}
 if(e==0){if(lock_menu){sub=sub==0?(d>0?3:2):sub==3?(d>0?2:0):(d>0?0:3);gesture=-1;return;}int n=count();sub=(sub+(d>0?1:-1)+n)%n;gesture=-1;return;}
 if(e==1){ug_ui_select(0,(voice+(d>0?1:-1)+4)%4);return;}
 if(e==2){lock_menu=0;cancel_presses();page=14;sub=0;gesture=-1;return;}
 int p=e-3;
 if(lock_menu&&busy())return;
 if(page==11){if(!p)ug_recmode((int)bound(ug_state(17)+(d>0?1:-1),1,3));return;}
 if(page==6&&sub==1){action=(action+(d>0?1:-1)+3)%3;return;}
 if((page==2&&!sub)||(lock_menu&&sub==3)){macro(env[voice][p],d);return;}
 if(note_page()){
  if(busy())return;int target=page==9?selected:-1,key=300+(target+1)*4+p;
  if((voice==2&&p==3)||(voice!=2&&!p)){
   if(gesture!=key){gesture=key;value=ug_getvelocity(voice,target);snapshot();}
   value=bound(value+d*.01f,0,1);ug_velocity(voice,target,value);return;
  }
  if(voice!=2||p>2)return;
  float n[3];for(int i=0;i<3;i++)n[i]=target>=0?ug_notevalue(target,i):ug_draftvalue(i);
  if(gesture!=key){gesture=key;value=n[p];snapshot();}
  value=bound(value+d*(p==1?.05f:1),p==0?24:p==1?.1f:0,p==0?60:p==1?1.5f:1);n[p]=value;
  if(target>=0)ug_note(target,(int)n[0],n[1],(int)n[2]);else ug_draftnote((int)n[0],n[1],(int)n[2]);return;
 }
 if(page==5){if(busy()||p>2)return;int key=500+p;if(gesture!=key){gesture=key;value=!p?ug_state(4):ug_getgroove(p-1);ug_begin_edit();}value=bound(value+d*(!p?1:.01f),!p?40:0,!p?220:1);if(!p)ug_tempo((int)value);else ug_groove(p-1,value);return;}
 if(page==0){if(!sub&&!p)macro(3,d);return;}
 if(page==6||page==4||page==9)macro(p,d);
}
void ug_ui_button(int b){
 if(b<0||b>13)return;gesture=-1;cancel_presses();
 if(b!=1&&b!=10&&b!=12&&b!=13){lock_menu=0;edited=0;}
 if(b==10){ug_play(-1);return;}
 if(b==11){page=11;sub=action=0;ug_arm(-1,voice);return;}
 if(b==12||b==13){octave=(int)bound(octave+(b==13?1:-1),-1,1);return;}
 if(b==1){
  if(note_page()||lock_menu){
   if(busy())return;int target=page==9?selected:-1;float vel=ug_getvelocity(voice,target);
   if(voice==2){audition_note=(int)(target>=0?ug_notevalue(target,0):ug_draftvalue(0));ug_keyvelocity(audition_note,1,vel);audition_ms=180;}
   else ug_hit(voice,vel);return;
  }
  if(page==11){ug_arm(-1,voice);return;}

  if(page==6&&sub==1){if(!action)ug_undo();else if(action==1)ug_rejoin();else ug_panic();return;}
  ug_ui_select(0,(voice+1)%4);return;
 }
 page=b;sub=action=0;
}
void ug_ui_key(int offset,int down,float velocity){
 if(offset<0||offset>=27)return;int index=-1;for(int i=0;i<16;i++)if(whites[i]==offset)index=i;
 if(down<=0){
  if(index>=0&&presses[index]){
   if(down==0&&presses[index]==1&&press_ms[index]<350&&!busy()&&page==9)ug_toggle_authored(voice,index);
   presses[index]=0;press_ms[index]=0;
   if(index==held)held=-1;
  }
  if(notes[offset]>=0){ug_key(notes[offset],0);notes[offset]=-1;}gesture=-1;return;
 }
 if(page==9&&index>=0&&!(ug_state(18)&&ug_state(2))){
  if(presses[index])return;
  select_step(index);held=index;press_ms[index]=0;presses[index]=!lock_menu&&!busy()?1:2;return;
 }
 if(ug_state(18)&&ug_state(19)!=voice)return;
 if(voice==2){int note=29+offset+octave*12;if(note<24||note>60)return;
  if(page==2&&sub==1&&!busy()){ug_begin_edit();ug_draftnote(note,ug_draftvalue(1),(int)ug_draftvalue(2));}
  notes[offset]=note;audition_note=-1;ug_keyvelocity(note,1,velocity);
 }else ug_hit(voice,velocity);
}
void ug_ui_tick(float ms){
 gesture_age+=ms;
 int due=-1;
 for(int i=0;i<16;i++)if(presses[i]==1){press_ms[i]+=ms;if(press_ms[i]>=350&&due<0)due=i;}
 // Finish all potential short taps before starting a lock-menu Undo transaction.
 if(due>=0&&!busy()&&page==9&&!pending_other(due)){
  for(int i=0;i<16;i++)if(presses[i]==1&&press_ms[i]>=350)presses[i]=2;
  if(!lock_menu){select_step(due);held=due;open_locks();}
 }
 if(audition_note>=0){audition_ms-=ms;if(audition_ms<=0){ug_key(audition_note,0);audition_note=-1;}}
}
int ug_ui_state(int f){switch(f){case 0:return page;case 1:return sub;case 2:return voice;case 3:return selected;case 4:return octave;case 5:return count();case 7:return lock_menu;case 6:{int b=page<14&&page!=11?1<<page:0;if(ug_state(0))b|=1<<10;if(ug_state(18))b|=1<<11;return b;}default:return 0;}}
static void add(char *s,const char *t){int n=0;while(s[n]&&n<63)n++;while(*t&&n<63)s[n++]=*t++;s[n]=0;}
static void number(char *s,int n){char t[16];int i=0;if(n<0){add(s,"-");n=-n;}do{t[i++]='0'+n%10;n/=10;}while(n&&i<15);while(i){char c[2]={t[--i],0};add(s,c);}}
static void named(char *s,int row,const char *name,int n,const char *unit){number(s,row);add(s," ");add(s,name);add(s,": ");number(s,n);add(s,unit);}
void ug_ui_refresh(void){
 for(int i=0;i<9;i++)lines[i][0]=0;
 add(lines[0],voices[voice]);add(lines[0]," / ");add(lines[0],lock_menu?"TRIG LOCK":pages[page]);
 if(page==11){add(lines[1],"1 Mode: ");add(lines[1],modes[ug_state(17)]);add(lines[2],ug_state(18)?"Armed: ":"Disarmed");if(ug_state(18))add(lines[2],voices[ug_state(19)]);add(lines[3],ug_state(0)?"Transport running":"PLAY to start");add(lines[4],ug_state(18)?"REC / SEL: finish":"REC / SEL: arm");add(lines[5],"Mode change finishes take");}
 else if(note_page()){
  int target=page==9?selected:-1;add(lines[1],target>=0?"STEP ":"DRAFT");if(target>=0){number(lines[1],target+1);add(lines[1],ug_step(voice,target)?" ON":" REST");}
  if(voice==2){const char *nn[12]={"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"};int n=(int)(target>=0?ug_notevalue(target,0):ug_draftvalue(0));add(lines[2],"1 Pitch: ");add(lines[2],nn[n%12]);number(lines[2],n/12-1);named(lines[3],2,"Gate",(int)((target>=0?ug_notevalue(target,1):ug_draftvalue(1))*100+.5f),"%");add(lines[4],"3 Slide: ");add(lines[4],(target>=0?ug_notevalue(target,2):ug_draftvalue(2))?"ON":"OFF");named(lines[5],4,"Velocity",(int)(ug_getvelocity(voice,target)*100+.5f),"%");}
  else named(lines[2],1,"Velocity",(int)(ug_getvelocity(voice,target)*100+.5f),"%");
 }
 else if(page==6&&sub==1){const char *a[3]={"Undo / redo","Rejoin locks","Panic"};add(lines[1],"Recovery");for(int i=0;i<3;i++){add(lines[i+2],i==action?"> ":"  ");add(lines[i+2],a[i]);}add(lines[5],"KNOB1 choose; SEL apply");}
 else if(page==5){named(lines[1],1,"Tempo",ug_state(4),"");named(lines[2],2,"Swing",50+(int)(ug_getgroove(0)*100/6+.5f),"%");named(lines[3],3,"Human",(int)(ug_getgroove(1)*100+.5f),"%");}
 else if(page==0){if(sub){add(lines[1],"GLOBAL DELAY");add(lines[2],"Dark 1/8-note delay");add(lines[3],"Feedback fixed at 52%");add(lines[4],"Bus automation: deferred");}else{add(lines[1],"TRACK SEND");named(lines[2],1,"Send",(int)((ug_state(10)?ug_effective(voice,3):ug_getparam(voice,3))*100+.5f),"%");add(lines[3],"Motion / step locks apply");}}
 else if(page==2||page==6||page==4||page==9){int isenv=page==2||(lock_menu&&sub==3),target=lock_menu?selected:held>=0?held:page==9&&sub==3?selected:-1;add(lines[1],target>=0?"LOCK STEP ":ug_state(10)?"MOTION VALUES":"TRACK DRAFT");if(target>=0)number(lines[1],target+1);const char *normal[4]={"Body","Edge","Length","Send"};for(int i=0;i<4;i++){int p=isenv?env[voice][i]:i;float x=target>=0&&(ug_lockmask(voice,target)&(1<<p))?ug_lockvalue(voice,target,p):ug_state(10)?ug_effective(voice,p):ug_getparam(voice,p);named(lines[i+2],i+1,isenv?labels[voice][i]:normal[i],(int)(x*100+.5f),"%");}}
 else{add(lines[1],pages[page]);add(lines[2],"Not implemented");add(lines[3],page==7?"Session only":"Reserved hardware page");}
 add(lines[6],lock_menu?"Keys: select; SEQ: back":note_page()?(busy()?"Finish REC to edit notes":"SEL: audition / no record"):page==9&&!(ug_state(18)&&ug_state(2))?sub||busy()?"Keys: select steps":"Tap: on/off; hold: locks":"Keys: live instrument");
 if(count()>1){add(lines[7],"SELECT ");number(lines[7],lock_menu?(sub==0?1:sub==3?2:3):sub+1);add(lines[7],"/");number(lines[7],count());const char *seq[4]={" Sound",""," Notes / vel"," ENV locks"};add(lines[7],page==9?seq[sub]:page==2?sub?" Draft notes / vel":" Environment":page==6?sub?" Recovery":" Sound":sub?" Global":" Track");}else add(lines[7],"ALGORITHM: track");
 add(lines[8],ug_state(0)?"PLAY ":"STOP ");number(lines[8],ug_state(4));add(lines[8]," BPM");if(ug_state(18)){add(lines[8]," REC ");add(lines[8],modes[ug_state(17)]);add(lines[8]," ");add(lines[8],voices[ug_state(19)]);}
}
const char *ug_ui_line(int n){return n>=0&&n<9?lines[n]:"";}

const char *ug_ui_meter(int cpu,int peak,int xruns,int host){
 static char text[64];text[0]=0;
 if(cpu<0){add(text,"CPU -- / FM1 only");return text;}
 add(text,host?"HOST ":"CPU ");number(text,(int)bound(cpu,0,999));add(text,"% P");number(text,(int)bound(peak,0,999));add(text,"% X");number(text,(int)bound(xruns,0,99999));if(xruns>99999)add(text,"+");if(!host)add(text," EXP");return text;
}
