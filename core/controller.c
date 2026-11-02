/* Original UNDERGROWTH control logic, shared by the browser and FM1. No platform IO. */
#include "controller.h"
#include "undergrowth.h"
static int page,sub,voice,selected,octave,action,held,edited,gesture;
static float value,gesture_age,audition_ms;
static float press_ms[16];
static unsigned char presses[16];
static int lock_menu,keyboard;
static int remembered[4][15],step_memory[4],record_return;
static char message[64];static float message_ms;
static unsigned char key_down[27];
static void tell(const char *text){int n=0;while(text[n]&&n<63){message[n]=text[n];n++;}message[n]=0;message_ms=1600;}
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
static int count(void){if(page==0&&!lock_menu)return 3;if(voice!=2&&page==2&&!lock_menu)return 5;if(voice==2){if(lock_menu)return 6;if(page==4)return 3;if(page==2)return 3;}return lock_menu?5:page==9?1:page==6?3:page==0||page==2||page==4?2:1;}
static int busy(void){return ug_state(18)||ug_state(10)||ug_state(2);}
static int note_page(void){if(voice==2&&page==9&&!lock_menu&&!busy())return 1;return (((page==2&&voice!=2)||page==4)&&sub==1)||(lock_menu&&sub==2);}
static int key_mode(void){return page==6?2:page==9&&!(voice==2&&keyboard)&&!(ug_state(18)&&ug_state(2))?1:0;}
static const int blacks[11]={1,3,5,8,10,13,15,17,20,22,25};
void ug_ui_init(void){page=6;sub=voice=selected=octave=action=edited=0;held=gesture=audition_note=-1;lock_menu=keyboard=0;record_return=4;message_ms=0;message[0]=0;cancel_presses();value=gesture_age=audition_ms=0;for(int i=0;i<27;i++){notes[i]=-1;key_down[i]=0;}for(int v=0;v<4;v++){step_memory[v]=0;for(int p=0;p<15;p++)remembered[v][p]=0;}}
void ug_ui_gesture(int reset){if(reset||gesture_age>300)gesture=-1;gesture_age=0;}
void ug_ui_select(int field,int n){if(field==0&&n>=0&&n<4&&n!=voice){if(!lock_menu)remembered[voice][page]=sub;step_memory[voice]=selected;voice=n;selected=step_memory[n];cancel_presses();edited=lock_menu=0;sub=remembered[voice][page];}if(field==1&&n>=0&&n<16){select_step(n);}gesture=-1;}
static int pending_other(int key){for(int i=0;i<16;i++)if(i!=key&&presses[i]==1&&press_ms[i]<350)return 1;return 0;}
static void open_locks(void){ug_rejoin();lock_menu=1;if(held>=0)presses[held]=2;sub=0;gesture=-1;edited=0;}
static void snapshot(void){if(!lock_menu||!edited){ug_begin_edit();if(lock_menu)edited=1;}}
void ug_ui_record_mode(int mode){int armed=ug_state(18),track=ug_state(19);ug_recmode(mode);if(armed)ug_arm(1,track);tell("Mode set / REC finishes take");}
void ug_ui_action(int a){
 if(a<0||a>5)return;
 if(a!=2&&busy()){tell("Finish REC for this action");return;}
 cancel_presses();gesture=-1;
 if(a==0){if(ug_undo())tell("Undo / redo: score restored");else tell("Nothing to undo yet");edited=0;return;}
 if(a==1){ug_rejoin();tell("Rejoined stored locks");return;}
 if(a==2){ug_panic();tell("Panic: score kept, silent");return;}
 if(a==3||a==4){
  if(page!=9){tell("Open SEQ to copy or paste");return;}
  if(a==3){if(ug_stepcopy(voice,selected))tell("Step copied incl. locks");return;}
  if(ug_clipboard_voice()!=voice){tell(ug_clipboard_voice()<0?"Copy a step first":"Paste on the source track");return;}
  snapshot();ug_steppaste(voice,selected);tell("Step pasted; Undo available");return;
 }
 int target=page==9?selected:-1;float vel=ug_getvelocity(voice,target);
 if(target>=0){audition_note=-1;ug_audition(voice,target);}
 else if(voice==2){audition_note=(int)ug_draftvalue(0);ug_keyvelocity(audition_note,1,vel);audition_ms=180;}
 else ug_hit(voice,vel);
 tell(ug_getmute(voice)?"Track muted; HOME to unmute":"Audition only / no recording");
}
void ug_ui_edit(int p,float x){
 if(!lock_menu||busy()||p<0||(p>11&&(p<20||p>=40)))return;
 if(p>=8&&p<=10&&voice!=2)return;
 snapshot();gesture=-1;
 if(p>=20&&p<40)ug_setlock(voice,selected,p-20,bound(x,0,1));
 else if(p<7)ug_setlock(voice,selected,p,bound(x,0,1));
 else if(p==7)ug_velocity(voice,selected,x);
 else if(p==11)ug_clearlock(voice,selected);
 else {int n=(int)ug_notevalue(selected,0),s=(int)ug_notevalue(selected,2);float g=ug_notevalue(selected,1);if(p==8)n=(int)x;else if(p==9)g=x;else s=x!=0;ug_note(selected,n,g,s);}
}
static const char *root_names[20]={"Tone","Colour","Decay","Synth send","Glide","Weight","Attack","Structure","Intensity","Feedback","Motion","Position","Grain size","Density","Pitch","Level","ENV send","Enabled","Scan","Surprise"};
static const char *ambient_names[4][9]={
 {"Harmonics","Texture","Movement","Pitch","Level","ENV send","Enabled","Rate","Crunch"},
 {"Colour","Pulse","Movement","Range","Level","ENV send","Enabled","Rate","Roughness"},
 {"Position","Grain size","Density","Pitch","Level","ENV send","Enabled","Scan","Surprise"},
 {"Vowel","Resonance","Movement","Pitch","Level","ENV send","Enabled","Rate","Breath"}};
static int ambient_page(void){return voice!=2&&((page==2&&sub>=2&&!lock_menu)||(lock_menu&&(sub==7||sub==5)));}
static int ambient_param(int knob){
 if((lock_menu&&sub==7)||(!lock_menu&&sub==2))return 11+knob;
 if(lock_menu&&sub==5)return (int[4]){15,18,19,16}[knob];
 if(!lock_menu&&sub==3)return (int[4]){15,18,17,16}[knob];
 return knob==0?19:-1;
}
static int root_param(int knob){
 if(lock_menu){if(sub==4)return 7+knob;if(sub==3)return 11+knob;if(sub==5)return (int[4]){15,18,19,16}[knob];if(sub==6)return knob==0?3:knob==1?16:-1;}
 if(page==2&&sub==2)return knob==0?19:-1;
 if(page==2)return sub==0?11+knob:(int[4]){15,18,17,16}[knob];
 if(page==0)return sub==0?(knob==0?3:knob==1?16:-1):-1;
 if(page==4&&sub==2)return 7+knob;
 return (int[4]){0,1,2,6}[knob];
}
static void macro(int p,int d){
 if(ug_state(18)&&ug_state(19)!=voice)return;
 int target=lock_menu?selected:held>=0?held:page==9&&sub==3?selected:-1,lock=target>=0&&!busy(),key=100+(lock?target+1:0)*32+p;
 int first=gesture!=key;
 if(first){gesture=key;value=lock&&(ug_lockmask(voice,target)&(1<<p))?ug_lockvalue(voice,target,p):(ug_state(10)||(!lock&&(p==16||(voice==2&&p==3))&&ug_state(0)))?ug_effective(voice,p):ug_getparam(voice,p);
  if(lock){if(lock_menu)snapshot();else if(held<0||!edited){ug_begin_edit();if(held>=0)edited=1;}}else if(!ug_state(18)&&!ug_state(10))ug_begin_edit();}
 value=p==17?(d>0?1:0):p==14?bound(((int)(value*36+.5f)+d)/36.0f,0,1):bound(value+d*.01f,0,1);
 if(lock)ug_setlock(voice,target,p,value);else ug_setparam(voice,p,value);
}
void ug_ui_turn(int e,int d){
 if(e<0||e>6||!d)return;ug_ui_gesture(0);
 if(held>=0&&presses[held]==1&&!busy()){for(int i=0;i<16;i++)if(i!=held&&presses[i]==1)return;open_locks();}
 if(e==0){if(lock_menu){const int order[6]={0,4,3,5,6,2};int n=voice==2?6:5,i=0;const int old[5]={0,3,2,7,5};while(i<n&&(voice==2?order[i]:old[i])!=sub)i++;sub=voice==2?order[(i+d%n+n)%n]:old[(i+d%n+n)%n];gesture=-1;return;}int n=count();sub=(sub+d%n+n)%n;remembered[voice][page]=sub;gesture=-1;return;}
 if(e==1){ug_ui_select(0,(voice+d%4+4)%4);return;}
 if(e==2){lock_menu=0;cancel_presses();page=14;sub=0;gesture=-1;return;}
 int p=e-3;
 if(lock_menu&&busy())return;
 if(page==11){if(!p)ug_ui_record_mode((int)bound(ug_state(17)+d,1,3));return;}
 if(page==6&&sub==1){action=(action+(d>0?1:-1)+3)%3;return;}
 if(page==6&&sub==2)return;
 if(page==6){ug_level(p,bound(ug_getlevel(p)+d*.01f,0,1));return;}
 if(page==0&&sub&&!lock_menu){if(busy())return;int rp=sub==1?p:4;if(sub==2&&p)return;int key=900+rp;if(gesture!=key){gesture=key;value=ug_getfx(rp);ug_begin_edit();}value=bound(value+d*.01f,0,1);ug_setfx(rp,value);return;}
 if(ambient_page()){int ap=ambient_param(p);if(ap>=0)macro(ap,d);return;}
 if(voice!=2&&page==0&&!sub&&!lock_menu){if(p<2)macro(p?16:3,d);return;}
 if(voice==2&&!note_page()&&(page==2||page==4||(page==0&&!sub)||lock_menu)){int rp=root_param(p);if(rp>=0)macro(rp,d);return;}
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
 if(b==10){ug_play(-1);return;}
 if(b==11){if(ug_state(18)){ug_arm(0,voice);tell("Take finished / Undo ready");if(page==11){page=record_return;sub=remembered[voice][page];}}else{if(!lock_menu)remembered[voice][page]=sub;record_return=page==11?4:page;page=11;sub=0;ug_arm(1,voice);}lock_menu=edited=0;return;}
 if(b==12||b==13){octave=(int)bound(octave+(b==13?1:-1),-1,1);return;}
 if(b==1){
  if(page==11){ug_ui_button(11);return;}
  if(page==6&&sub==1){ug_ui_action(action);return;}
  ug_ui_action(5);return;
 }
 if(!lock_menu)remembered[voice][page]=sub;
 int again=page==b&&!lock_menu;if(b==9&&voice==2&&!lock_menu){keyboard=again?!keyboard:0;}else if(b!=9)keyboard=0;lock_menu=edited=0;page=b;sub=b==6?0:remembered[voice][b];
 if(again&&b!=6&&count()>1)sub=(sub+1)%count();remembered[voice][b]=sub;action=0;
}
void ug_ui_key(int offset,int down,float velocity){
 if(offset<0||offset>=27)return;int index=-1;for(int i=0;i<16;i++)if(whites[i]==offset)index=i;
 if(down<=0){
  key_down[offset]=0;
  if(index>=0&&presses[index]){
   if(down==0&&presses[index]==1&&press_ms[index]<350&&!busy()&&page==9)ug_toggle_authored(voice,index);
   presses[index]=0;press_ms[index]=0;
   if(index==held)held=-1;
  }
  if(notes[offset]>=0){ug_key(notes[offset],0);notes[offset]=-1;}gesture=-1;return;
 }
 if(key_down[offset])return;key_down[offset]=1;
 int mode=key_mode();
 if(mode){
  int black=-1;for(int i=0;i<11;i++)if(blacks[i]==offset)black=i;
  if(black>=0){
   if(black<4){ug_ui_select(0,black);return;}
   if(black==4||black==5){if(mode==1)ug_ui_action(black==4?3:4);else tell("Open SEQ to copy / paste");return;}
   if(black>=6&&black<=8){ug_ui_action(black==6?0:black==7?1:5);return;}
   if(lock_menu)ug_ui_turn(0,black==9?-1:1);else tell("Hold step for lock pages");return;
  }
  if(mode==2){if(index>=0&&index<4){ug_mute(index,-1);tell(ug_getmute(index)?"Track muted / score kept":"Track unmuted");}return;}
 }
 if(page==9&&index>=0&&!(voice==2&&keyboard)&&!(ug_state(18)&&ug_state(2))){
  if(presses[index])return;
  select_step(index);held=index;press_ms[index]=0;presses[index]=!lock_menu&&!busy()?1:2;return;
 }
 if(ug_state(18)&&ug_state(19)!=voice)return;
 if(voice==2){int note=29+offset+octave*12;if(note<24||note>60)return;
  if(note_page()&&!busy()){if(page==9){snapshot();ug_note(selected,note,ug_notevalue(selected,1),(int)ug_notevalue(selected,2));}else{ug_begin_edit();ug_draftnote(note,ug_draftvalue(1),(int)ug_draftvalue(2));}}
  notes[offset]=note;audition_note=-1;ug_keyvelocity(note,1,velocity);
 }else ug_hit(voice,velocity);
}
void ug_ui_tick(float ms){
 gesture_age+=ms;
 if(message_ms>0){message_ms-=ms;if(message_ms<=0)message[0]=0;}
 int due=-1;
 for(int i=0;i<16;i++)if(presses[i]==1){press_ms[i]+=ms;if(press_ms[i]>=350&&due<0)due=i;}
 // Finish all potential short taps before starting a lock-menu Undo transaction.
 if(due>=0&&!busy()&&page==9&&!pending_other(due)){
  for(int i=0;i<16;i++)if(presses[i]==1&&press_ms[i]>=350)presses[i]=2;
  if(!lock_menu){select_step(due);held=due;open_locks();}
 }
 if(audition_note>=0){audition_ms-=ms;if(audition_ms<=0){ug_key(audition_note,0);audition_note=-1;}}
}
int ug_ui_state(int f){switch(f){case 0:return page;case 1:return sub;case 2:return voice;case 3:return selected;case 4:return octave;case 5:return count();case 7:return lock_menu;case 10:return key_mode();case 11:return ug_clipboard_voice();case 8:{unsigned bits=0;int mode=key_mode();if(mode){bits|=1u<<blacks[voice];for(int i=0;i<16;i++)if(mode==1?ug_step(voice,i):i<4&&!ug_getmute(i))bits|=1u<<whites[i];}else for(int i=0;i<27;i++)if(key_down[i])bits|=1u<<i;return (int)bits;}case 9:return key_mode()==1&&ug_state(0)&&ug_state(1)>=0?1u<<whites[ug_state(1)]:0;case 6:{int b=page<14&&page!=11?1<<page:0;if(ug_state(0))b|=1<<10;if(ug_state(18)||page==11)b|=1<<11;return b;}default:return 0;}}
static void add(char *s,const char *t){int n=0;while(s[n]&&n<63)n++;while(*t&&n<63)s[n++]=*t++;s[n]=0;}
static void number(char *s,int n){char t[16];int i=0;if(n<0){add(s,"-");n=-n;}do{t[i++]='0'+n%10;n/=10;}while(n&&i<15);while(i){char c[2]={t[--i],0};add(s,c);}}
static void named(char *s,int row,const char *name,int n,const char *unit){number(s,row);add(s," ");add(s,name);add(s,": ");number(s,n);add(s,unit);}
const char *ug_ui_knoblabel(int e){
 static const char *normal[4]={"Body","Edge","Length","Send"};
 if(e==0)return "SELECT: page";if(e==1)return "ALGORITHM: track";if(e==2)return "PRESETS: unavailable";if(e<3||e>6)return "";
 int p=e-3;if(ambient_page()){int ap=ambient_param(p);return ap<0?"Unused":ambient_names[voice][ap-11];}if(page==6)return sub==0?voices[p]:sub==1&&p==0?"Choose recovery":"Unused";
 if(page==11)return p==0?"Record mode":"Unused";
 if(note_page())return voice==2?(const char *[4]){"Pitch","Gate","Slide","Velocity"}[p]:p==0?"Velocity":"Unused";
 if(voice==2&&(page==2||page==4||(page==0&&!sub)||lock_menu)){int rp=root_param(p);return rp>=0?root_names[rp]:"Unused";}
 if((page==2&&!sub)||(lock_menu&&sub==3))return labels[voice][p];
 if(page==0)return sub==1?(const char *[4]){"Reverb","Decay","Damping","Width"}[p]:sub==2?(p==0?"Delay return":"Unused"):p==0?"Send":p==1?"ENV send":"Unused";
 if(page==5)return (const char *[4]){"Tempo","Swing","Human","Unused"}[p];
 return page==4||page==9?normal[p]:"Unused";
}
const char *ug_ui_keylabel(int offset){
 static char text[64];text[0]=0;if(offset<0||offset>=27)return text;
 int white=-1,black=-1;for(int i=0;i<16;i++)if(whites[i]==offset)white=i;for(int i=0;i<11;i++)if(blacks[i]==offset)black=i;
 int mode=key_mode();
 if(mode&&black>=0){const char *names[11]={"KICK","CHAT","ROOT","BACK","COPY","PASTE","UNDO","REJOIN","HEAR","PAGE-","PAGE+"};return names[black];}
 if(mode==2){if(white>=0&&white<4){add(text,ug_getmute(white)?"MUTED":"ON");}else add(text,"--");return text;}
 if(mode==1){if(white<9)add(text,"0");number(text,white+1);return text;}
 if(voice!=2)return voices[voice];
 int note=29+offset+octave*12;if(note<24||note>60)return "OUT";
 const char *names[12]={"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"};add(text,names[note%12]);number(text,note/12-1);return text;
}
void ug_ui_refresh(void){
 for(int i=0;i<9;i++)lines[i][0]=0;
 add(lines[0],voices[voice]);add(lines[0]," / ");add(lines[0],lock_menu?"TRIG LOCK":pages[page]);
 if(ug_getmute(voice))add(lines[0]," [M]");
 if(page==11){add(lines[1],"1 Mode: ");add(lines[1],modes[ug_state(17)]);add(lines[2],ug_state(18)?"Armed: ":"Disarmed");if(ug_state(18))add(lines[2],voices[ug_state(19)]);add(lines[3],ug_state(0)?"Recording / keys play live":"PLAY to start recording");add(lines[4],"EDIT / ENV / FX: perform");add(lines[5],"Mode change = next take");}
 else if(note_page()){
  int target=page==9?selected:-1;add(lines[1],target>=0?"STEP ":"DRAFT");if(target>=0){number(lines[1],target+1);add(lines[1],ug_step(voice,target)?" ON":" REST");}
  if(voice==2){const char *nn[12]={"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"};int n=(int)(target>=0?ug_notevalue(target,0):ug_draftvalue(0));add(lines[2],"1 Pitch: ");add(lines[2],nn[n%12]);number(lines[2],n/12-1);named(lines[3],2,"Gate",(int)((target>=0?ug_notevalue(target,1):ug_draftvalue(1))*100+.5f),"%");add(lines[4],"3 Slide: ");add(lines[4],(target>=0?ug_notevalue(target,2):ug_draftvalue(2))?"ON":"OFF");named(lines[5],4,"Velocity",(int)(ug_getvelocity(voice,target)*100+.5f),"%");}
  else named(lines[2],1,"Velocity",(int)(ug_getvelocity(voice,target)*100+.5f),"%");
 }
 else if(page==6&&sub==1){const char *a[3]={"Undo / redo","Rejoin locks","Panic"};add(lines[1],"Recovery");for(int i=0;i<3;i++){add(lines[i+2],i==action?"> ":"  ");add(lines[i+2],a[i]);}add(lines[5],"KNOB1 choose; SEL apply");}
 else if(page==6&&sub==2){add(lines[1],"UPPER KEYS in HOME / SEQ");add(lines[2],"1-4 tracks 5copy 6paste");add(lines[3],"7undo 8rejoin 9hear");add(lines[4],"10/11 lock page -/+");add(lines[5],"Copy/paste in SEQ only");}
 else if(page==6){add(lines[1],"MIX / knobs: track levels");for(int v=0;v<4;v++){named(lines[v+2],v+1,voices[v],(int)(ug_getlevel(v)*100+.5f),"%");if(ug_getmute(v))add(lines[v+2]," MUTE");}}
 else if(page==5){named(lines[1],1,"Tempo",ug_state(4),"");named(lines[2],2,"Swing",50+(int)(ug_getgroove(0)*100/6+.5f),"%");named(lines[3],3,"Human",(int)(ug_getgroove(1)*100+.5f),"%");}
 else if(ambient_page()){
  static const char *kind[4]={"HARMONIC DRONE","MODULATED NOISE","GRAINS","MORPHING FORMANT"};
  add(lines[1],lock_menu?"STEP / ":"DRAFT / ");add(lines[1],kind[voice]);
  for(int i=0;i<4;i++){int p=ambient_param(i);if(p<0)continue;
   float x=lock_menu&&(ug_lockmask(voice,selected)&(1<<p))?ug_lockvalue(voice,selected,p):ug_state(10)?ug_effective(voice,p):ug_getparam(voice,p);
   if(p==17){number(lines[i+2],i+1);add(lines[i+2]," Enabled: ");add(lines[i+2],x>.5f?"ON":"OFF");}
   else named(lines[i+2],i+1,ambient_names[voice][p-11],(int)(x*100+.5f),"%");
  }
 }
 else if(voice==2&&(page==2||page==4||(page==0&&!sub)||lock_menu)){
  int target=lock_menu?selected:-1;add(lines[1],target>=0?"STEP ":ug_state(10)?"MOTION / ":page==0&&ug_state(0)?"LIVE / ":"DRAFT / ");if(target>=0){number(lines[1],target+1);add(lines[1]," / ");}
  add(lines[1],page==2&&sub==2?"GRAIN VARIATION":((lock_menu&&sub==4)||(page==4&&sub==2))?"CHARACTER":((lock_menu&&sub==3)||(page==2&&sub==0))?"ATMOSPHERE":((lock_menu&&sub==5)||(page==2&&sub==1))?"ATM MIX":(page==0||(lock_menu&&sub==6))?"FX SENDS":"SOUND");
  for(int i=0;i<4;i++){int p=root_param(i);if(p<0)continue;float x=target>=0&&(ug_lockmask(voice,target)&(1<<p))?ug_lockvalue(voice,target,p):(ug_state(10)||(target<0&&(p==3||p==16)&&ug_state(0)))?ug_effective(voice,p):ug_getparam(voice,p);
   if(p==17){number(lines[i+2],i+1);add(lines[i+2]," Enabled: ");add(lines[i+2],x>.5f?"ON":"OFF");}
   else if(p==14){const char *nn[12]={"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"};int n=24+(int)(x*36+.5f);number(lines[i+2],i+1);add(lines[i+2]," Pitch: ");add(lines[i+2],nn[n%12]);number(lines[i+2],n/12-1);}
   else named(lines[i+2],i+1,root_names[p],(int)(x*100+.5f),"%");
   if(target>=0&&(ug_lockmask(voice,target)&(1<<p)))add(lines[i+2]," *");
  }
 }
 else if(page==0){if(sub==1){add(lines[1],"GLOBAL REVERB");const char *names[4]={"Reverb","Decay","Damping","Width"};for(int i=0;i<4;i++)named(lines[i+2],i+1,names[i],(int)(ug_getfx(i)*100+.5f),"%");}else if(sub==2){add(lines[1],"GLOBAL DELAY");named(lines[2],1,"Delay return",(int)(ug_getfx(4)*100+.5f),"%");add(lines[3],"Dark 1/8-note delay");add(lines[4],"Feedback fixed at 52%");}else{add(lines[1],"TRACK FX SEND");named(lines[2],1,"Send",(int)((ug_state(10)?ug_effective(voice,3):ug_getparam(voice,3))*100+.5f),"%");named(lines[3],2,"ENV send",(int)((ug_state(0)?ug_effective(voice,16):ug_getparam(voice,16))*100+.5f),"%");add(lines[4],"Motion / step locks apply");}}

 else if(page==2||page==4||page==9){int isenv=page==2||(lock_menu&&sub==3),target=lock_menu?selected:held>=0?held:-1;add(lines[1],target>=0?"STEP ":ug_state(10)?"MOTION VALUES":"TRACK DRAFT");if(target>=0){number(lines[1],target+1);add(lines[1],ug_step(voice,target)?" ON":" REST");}else if(page==9){add(lines[1]," / step ");number(lines[1],selected+1);}const char *normal[4]={"Body","Edge","Length","Send"};for(int i=0;i<4;i++){int p=isenv?env[voice][i]:i;float x=target>=0&&(ug_lockmask(voice,target)&(1<<p))?ug_lockvalue(voice,target,p):ug_state(10)?ug_effective(voice,p):ug_getparam(voice,p);named(lines[i+2],i+1,isenv?labels[voice][i]:normal[i],(int)(x*100+.5f),"%");if(target>=0&&(ug_lockmask(voice,target)&(1<<p)))add(lines[i+2]," *");}}
 else{add(lines[1],pages[page]);add(lines[2],"Not implemented");add(lines[3],page==7?"Session only":"Reserved hardware page");}
 add(lines[6],message[0]?message:lock_menu?"Keys select / SEQ closes":page==6?"Lower 1-4: mute / unmute":page==11?"REC / SEL: finish + return":note_page()?(busy()?"Finish REC to edit notes":"Keys: pitch / SEL: hear"):key_mode()==1?busy()?"Keys select / no new hits":"Tap: on/off; hold: locks":"Keys: live / SEL: audition");
 if(page==0&&sub==1&&!message[0]){lines[6][0]=0;add(lines[6],busy()?"Finish REC for global FX":"Raise track / ENV send to hear");}
 if(voice==2&&page==9&&!lock_menu&&!message[0]){lines[6][0]=0;add(lines[6],keyboard?"KEYBOARD: assign / REC: play":"STEPS / SEQ: keyboard");}
 if(voice==2&&page==2&&!message[0]){char strip[28];for(int i=0;i<26;i++)strip[i]='-';strip[26]=0;for(int i=2;i<10;i++){float x=ug_grain(i);if(x>=0)strip[(int)(x*25)]='o';}strip[(int)(ug_grain(0)*25)]='|';lines[6][0]=0;add(lines[6],strip);}
 if(count()>1){
  int ordinal=sub+1;const char *label=" Track";
  if(lock_menu){
   static const int root_order[8]={1,0,6,3,2,4,5,0},percussion_order[8]={1,0,3,2,0,5,0,4};
   ordinal=sub>=0&&sub<8?(voice==2?root_order[sub]:percussion_order[sub]):1;
   label=sub==0?" Sound":sub==2?" Notes / vel":sub==3?(voice==2?" Atmosphere":" Percussion ENV"):sub==4?" Character":sub==5?" Atmos mix":sub==6?" Sends":sub==7?" Atmosphere":" Track";
  }else if(page==2){
   label=voice==2?(sub==0?" Atmosphere":sub==1?" Atmos mix":" Grain variation"):(sub==0?" Percussion ENV":sub==1?" Velocity":sub==2?" Atmosphere":sub==3?" Atmos mix":" Character");
  }else if(page==4)label=sub==0?" Sound":sub==1?" Notes / vel":" Character";
  else if(page==6)label=sub==0?" Mix":sub==1?" Recovery":" Key guide";
  else if(page==0)label=sub==0?" Sends":sub==1?" Reverb":" Delay";
  add(lines[7],"SELECT ");number(lines[7],ordinal);add(lines[7],"/");number(lines[7],count());add(lines[7],label);
 }else add(lines[7],key_mode()==1?"Upper: tracks + tools":"ALGORITHM: track");
 add(lines[8],ug_state(0)?"PLAY ":"STOP ");number(lines[8],ug_state(4));add(lines[8]," BPM");if(ug_state(18)){add(lines[8]," REC ");add(lines[8],modes[ug_state(17)]);add(lines[8]," ");add(lines[8],voices[ug_state(19)]);}
}
const char *ug_ui_line(int n){return n>=0&&n<9?lines[n]:"";}

const char *ug_ui_meter(int cpu,int peak,int xruns,int host){
 static char text[64];text[0]=0;
 if(cpu<0){add(text,"CPU -- / FM1 only");return text;}
 add(text,host?"HOST ":"CPU ");number(text,(int)bound(cpu,0,999));add(text,"% P");number(text,(int)bound(peak,0,999));add(text,"% X");number(text,(int)bound(xruns,0,99999));if(xruns>99999)add(text,"+");if(!host)add(text," EXP");return text;
}
