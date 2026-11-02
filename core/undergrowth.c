/* UNDERGROWTH musical core, 2026. Clouds-derived reverb: see room.h and LICENSING.md.
 * One source for native tests, WebAssembly, and fm1-base app adapter.
 * No allocation, system calls, IO, wall-clock scheduling or random composition.
 */
#include "undergrowth.h"
#define NVOICE 4
#define NSTEPS 16
#define NPARAM 20
#define MAX_BLOCK 256
#define DELAY 22050

typedef struct { unsigned char on, authored; uint32_t mask; float lock[NPARAM]; int note; float velocity, gate; unsigned char slide; } Step;
typedef struct { Step steps[NVOICE][NSTEPS]; float base[NVOICE][NPARAM]; float swing, human, velocity[NVOICE], fx[5]; Step draft_note; } Score;
typedef struct {
 float phase, env, sweep, noise_lp, vel, dna_decay;
 float params[NPARAM];
 int age, burst_left, burst_wait;
} Voice;
static Score score, saved;
static int saved_valid;
static int motion, motion_saved;
static int rec_mode=1, take_active, rec_track=-1;
static uint32_t motion_mask[NVOICE];
static Voice voices[NVOICE];
#if defined(UG_FM1) && !defined(PLAT_HOST)
#define UG_POOL __attribute__((section(".pool")))
#else
#define UG_POOL
#endif
static float delaybuf[DELAY] UG_POOL;
static float delay_lp, output[MAX_BLOCK*2];
/* Rate-scaled Clouds tank. Float storage avoids quantized-tail limit cycles. */
#define ROOM_SAMPLES 49200
static float room_buffer[ROOM_SAMPLES] UG_POOL;
static struct { int offset[10], length[10], position[10]; float low[2], phase[2], now[5], smoothing; int active; } room;
static void room_clear(void);

static float master_target, master_now;
static float levels[NVOICE],level_now[NVOICE],dry_now[NVOICE],cloud_dry;
static unsigned char muted[NVOICE];
static Step clipboard;
static int clipboard_voice;
static int audition_step[NVOICE];
static float manual[NVOICE][NPARAM];
static uint32_t takeover[NVOICE];
static uint32_t record_skip[NVOICE]; /* Only newly overdubbed future hits: heard live, skip once. */
static int rate, tempo, playing, recording, step_index, delaypos, first;
static uint32_t tickphase, noise_state, trigger_count[NVOICE], sample_clock, last_trigger[NVOICE];
static int pending_chatter;
static float pending_velocity;
static float root_freq, root_target;
/* One bounded collision cloud per Backbeat hit; no independent musical events. */
static struct {float mode, metal, low, band, held, clock, grain, smooth[6];} backbeat;
static int root_gate, root_live=-1;
static struct { float mod_phase, feedback, sub_phase; } root_fm;
#define GRAINS 8
#define TEXTURE 4096
/* Original, deterministic synthetic sample; generated once, never in the render path. */
static float texture[TEXTURE] UG_POOL;
static struct { float pos, age, duration, speed; int active; } grains[GRAINS];
static float grain_cursor, grain_level;
static int grain_wait;
static unsigned grain_serial;

static float note_frequency(int note){
 static const float hz[12]={32.703196f,34.647829f,36.708096f,38.890873f,41.203445f,43.653529f,46.249303f,48.999429f,51.913087f,55.0f,58.270470f,61.735413f};
 int n=note-24;float f=hz[n%12];for(int o=0;o<n/12;o++)f*=2;return f;
}
/* Durations alternate long/short; the pair always spans two straight steps. */
static uint32_t step_duration(void) {
 uint32_t offset=(uint32_t)(rate*20*score.swing);
 return (uint32_t)(rate*60)+((step_index&1)?-offset:offset);
}
static float clamp(float x,float a,float b) {return x<a?a:x>b?b:x;}
static int valid_voice(int v) {return v>=0&&v<NVOICE;}
static int valid_param(int p) {return p>=0&&p<NPARAM;}
static int valid_step(int s) {return s>=0&&s<NSTEPS;}
static void zero(void *ptr, unsigned n) {unsigned char *p=ptr; while(n--) *p++=0;}
static float sine(float phase) {
 /* Range-reduced odd polynomial, deterministic native/WASM with FMA disabled. */
 float x=(phase<0.5f?phase:phase-1.0f)*6.28318530718f;
 float xx=x*x;
 return x*(1.0f+xx*(-0.1666666667f+xx*(0.0083333333f+xx*(-0.0001984127f+xx*0.0000027557f))));
}
static float noise(void) {
 noise_state^=noise_state<<13;noise_state^=noise_state>>17;noise_state^=noise_state<<5;
 return (float)(noise_state&65535u)/32767.5f-1.0f;
}
void ug_init(int sample_rate) {
 zero(&score,sizeof score); zero(&saved,sizeof saved); zero(voices,sizeof voices);
 zero(delaybuf,sizeof delaybuf);zero(&room,sizeof room);room_clear();zero(output,sizeof output);zero(takeover,sizeof takeover);
 zero(manual,sizeof manual);zero(trigger_count,sizeof trigger_count);zero(record_skip,sizeof record_skip);
 zero(muted,sizeof muted);clipboard_voice=-1;zero(&clipboard,sizeof clipboard);
 cloud_dry=1;for(int v=0;v<NVOICE;v++){dry_now[v]=1;levels[v]=level_now[v]=1;audition_step[v]=-1;}
 sample_clock=0;zero(last_trigger,sizeof last_trigger);
 zero(&backbeat,sizeof backbeat);zero(&root_fm,sizeof root_fm);zero(grains,sizeof grains);grain_cursor=grain_level=0;grain_wait=0;grain_serial=0;root_freq=root_target=65.406392f;root_gate=0;root_live=-1;
 for(int s=0;s<NSTEPS;s++){score.steps[2][s].note=36;score.steps[2][s].gate=.75f;}
 for(int v=0;v<NVOICE;v++){score.velocity[v]=1;for(int st=0;st<NSTEPS;st++)score.steps[v][st].velocity=1;score.base[v][4]=.5f;score.base[v][5]=.5f;score.base[v][6]=.5f;}
 score.base[2][7]=.25f;score.base[2][8]=.25f;score.base[2][9]=.1f;score.base[2][10]=.25f;
 score.base[2][11]=.3f;score.base[2][12]=.4f;score.base[2][13]=.4f;score.base[2][14]=1.0f/3;score.base[2][15]=.3f;score.base[2][18]=.15f;
 score.draft_note.note=36;score.draft_note.gate=.75f;
 rec_mode=1;take_active=0;rec_track=-1;
 motion=motion_saved=0;zero(motion_mask,sizeof motion_mask);
 rate=sample_rate>=8000&&sample_rate<=96000?sample_rate:44100;
 const int times[10]={113,162,241,399,1653,2038,3411,1913,1663,4782};
 room.smoothing=1.0f/(rate*.005f);
 int room_offset=0;for(int i=0;i<10;i++){room.offset[i]=room_offset;room.length[i]=((times[i]*rate+16000)/32000)+2;room_offset+=room.length[i];}
 score.fx[0]=0;score.fx[1]=.55f;score.fx[2]=.45f;score.fx[3]=.8f;score.fx[4]=.75f;
 for(int p=0;p<5;p++)room.now[p]=score.fx[p];
 tempo=110;playing=recording=first=0;step_index=-1;tickphase=0;delaypos=0;delay_lp=0;
 master_target=0.65f;master_now=0;noise_state=0x51d734a1; saved_valid=0;pending_chatter=-1;
 score.base[0][0]=0.35f;score.base[0][1]=0.28f;score.base[0][2]=0.40f;score.base[0][3]=0.0f;
 score.base[2][0]=.65f;score.base[2][1]=.25f;score.base[2][2]=.35f;score.base[2][3]=0;
 score.base[3][0]=.55f;score.base[3][1]=.45f;score.base[3][2]=.35f;score.base[3][3]=.08f;score.base[3][5]=.15f;score.base[3][6]=.7f;
 /* Woven, pitched/noisy texture at 22050 Hz, with a periodic seam. */
 float low=0;for(int i=0;i<TEXTURE;i++){low+=(noise()-low)*.08f;float t=(float)i/TEXTURE,phase=i*(65.406392f/22050.0f);
  texture[i]=(sine(phase-(int)phase)*.45f+sine(phase*3-(int)(phase*3))*.18f+low*.4f)*4*t*(1-t);}
 score.base[1][0]=0.45f;score.base[1][1]=0.50f;score.base[1][2]=0.24f;score.base[1][3]=0.16f;
}
void ug_begin_edit(void) {ug_arm(0,0);ug_motion(0);saved=score;saved_valid=1;}
int ug_undo(void) {ug_arm(0,0);ug_motion(0);if(!saved_valid)return 0;Score swap=score;score=saved;saved=swap;ug_rejoin();zero(record_skip,sizeof record_skip);pending_chatter=-1;return 1;}
void ug_rejoin(void) {ug_arm(0,0);ug_motion(0);zero(takeover,sizeof takeover);for(int v=0;v<NVOICE;v++)audition_step[v]=-1;}
void ug_motion(int enabled) {
 if(enabled<0)enabled=!motion;
 if(take_active)ug_arm(0,0);
 if(enabled&&!motion){
  motion=1;motion_saved=0;recording=0;
  zero(motion_mask,sizeof motion_mask);zero(takeover,sizeof takeover);
 }else if(!enabled&&motion){
  for(int v=0;v<NVOICE;v++)takeover[v]&=~motion_mask[v];
  motion=0;zero(motion_mask,sizeof motion_mask);
 }
}
void ug_arm(int enabled,int track){
 if(enabled<0)enabled=!take_active;
 if(enabled){
  if(!valid_voice(track))return;
  if(take_active)return;
  ug_motion(0);recording=0;zero(takeover,sizeof takeover);
  take_active=1;rec_track=track;motion_saved=0;
  recording=(rec_mode&1)!=0;motion=(rec_mode&2)!=0;zero(motion_mask,sizeof motion_mask);
 }else if(take_active){
  take_active=0;recording=motion=0;rec_track=-1;zero(takeover,sizeof takeover);zero(motion_mask,sizeof motion_mask);
 }
}
void ug_recmode(int mode){if(mode<1||mode>3||mode==rec_mode)return;ug_arm(0,0);ug_motion(0);recording=0;rec_mode=mode;}
static int recording_voice(int v){return !take_active||rec_track==v;}
static void take_snapshot(void){if(take_active){if(!motion_saved){saved=score;saved_valid=1;motion_saved=1;}}else ug_begin_edit();}
void ug_level(int v,float gain){if(valid_voice(v))levels[v]=clamp(gain,0,1);}
float ug_getlevel(int v){return valid_voice(v)?levels[v]:0;}
void ug_mute(int v,int enabled){if(valid_voice(v))muted[v]=enabled<0?!muted[v]:enabled!=0;}
int ug_getmute(int v){return valid_voice(v)?muted[v]:0;}
int ug_clipboard_voice(void){return clipboard_voice;}
int ug_stepcopy(int v,int s){if(!valid_voice(v)||!valid_step(s)||take_active||motion||recording)return 0;clipboard=score.steps[v][s];clipboard_voice=v;return 1;}
int ug_steppaste(int v,int s){if(!valid_voice(v)||!valid_step(s)||clipboard_voice!=v||take_active||motion||recording)return 0;score.steps[v][s]=clipboard;record_skip[v]&=~(1u<<s);return 1;}
void ug_draftnote(int note,float gate,int slide){score.draft_note.note=note<24?24:note>60?60:note;score.draft_note.gate=clamp(gate,.1f,1.5f);score.draft_note.slide=slide?1:0;}
float ug_draftvalue(int p){return p==0?score.draft_note.note:p==1?score.draft_note.gate:p==2?score.draft_note.slide:0;}
/* Velocity is authored event data, separate from macro-lock masks. -1 means draft. */
void ug_velocity(int v,int step,float value){
 if(!valid_voice(v)||step < -1||step>=NSTEPS||take_active||motion||recording)return;
 if(step<0)score.velocity[v]=clamp(value,0,1);else {score.steps[v][step].velocity=clamp(value,0,1);score.steps[v][step].authored=1;}
}
float ug_getvelocity(int v,int step){return !valid_voice(v)||step < -1||step>=NSTEPS?0:step<0?score.velocity[v]:score.steps[v][step].velocity;}
void ug_stamp(int v,int step){
 if(!valid_voice(v)||!valid_step(step)||recording||motion)return;
 ug_begin_edit();Step *st=&score.steps[v][step];st->authored=1;st->on=1;st->velocity=score.velocity[v];st->mask=v==2?2039u:127u; /* ROOT sound/character only: sends and ENV need deliberate locks. */
 for(int p=0;p<NPARAM;p++)st->lock[p]=score.base[v][p];
 if(v==2){st->note=score.draft_note.note;st->gate=score.draft_note.gate;st->slide=score.draft_note.slide;}
 record_skip[v]&=~(1u<<step);takeover[v]=0;
}
void ug_toggle_authored(int v,int step){
 if(!valid_voice(v)||!valid_step(step)||take_active||recording||motion)return;
 Step *st=&score.steps[v][step];
 if(!st->authored&&!st->on){ug_stamp(v,step);return;}
 ug_begin_edit();st->on=!st->on;record_skip[v]&=~(1u<<step);
}
void ug_erase(int v,int step){if(!valid_voice(v)||!valid_step(step)||recording||motion)return;if(!score.steps[v][step].authored&&!score.steps[v][step].on&&!score.steps[v][step].mask)return;ug_begin_edit();zero(&score.steps[v][step],sizeof(Step));score.steps[v][step].velocity=1;if(v==2){score.steps[v][step].note=36;score.steps[v][step].gate=.75f;}record_skip[v]&=~(1u<<step);}
static void capture_motion(int v,int step){
 if(!motion||!recording_voice(v))return;
 Step *s=&score.steps[v][step];
 for(int p=0;p<NPARAM;p++)if(motion_mask[v]&(1u<<p)){
  if(!(s->mask&(1u<<p))||s->lock[p]!=manual[v][p]){
   if(!motion_saved){saved=score;saved_valid=1;motion_saved=1;}
   s->mask|=(1u<<p);s->lock[p]=manual[v][p];
  }
 }
}
void ug_record(int enabled) {if(enabled<0)enabled=!recording;ug_arm(0,0);if(enabled)ug_motion(0);recording=enabled?1:0;}
void ug_play(int enabled) {
 if(enabled<0)enabled=!playing;
 if(enabled&&!playing){playing=1;first=1;step_index=-1;tickphase=0;zero(record_skip,sizeof record_skip);}
 else if(!enabled){ug_arm(0,0);root_gate=0;root_live=-1;ug_motion(0);pending_chatter=-1;playing=0;recording=0;step_index=-1;first=0;ug_rejoin();zero(record_skip,sizeof record_skip);}
}
void ug_tempo(int bpm) {tempo=bpm<40?40:bpm>220?220:bpm;}
void ug_groove(int parameter,float value) {
 if(parameter==0)score.swing=clamp(value,0,1);
 if(parameter==1)score.human=clamp(value,0,1);
}
float ug_getgroove(int parameter) {return parameter==0?score.swing:parameter==1?score.human:0;}
void ug_setfx(int p,float value){if(p>=0&&p<5&&!take_active&&!recording&&!motion)score.fx[p]=clamp(value,0,1);}
float ug_getfx(int p){return p>=0&&p<5?score.fx[p]:0;}
void ug_master(float gain) {master_target=clamp(gain,0,1);}
void ug_setparam(int v,int p,float value) {
 if(!valid_voice(v)||!valid_param(p)|| (take_active&&rec_track!=v))return;
 audition_step[v]=-1;
 if(take_active&&!motion)take_snapshot();
 value=clamp(value,0,1);
 /* Drone enable is a track switch, never a passing trig or motion lock. */
 if(v==2&&p==17){score.base[v][p]=value;return;}
 if(motion&&recording_voice(v)){motion_mask[v]|=(1u<<p);manual[v][p]=value;takeover[v]|=(1u<<p);return;}
 score.base[v][p]=value;
 if(playing){takeover[v]|=(1u<<p);manual[v][p]=value;}
}
float ug_getparam(int v,int p) {return valid_voice(v)&&valid_param(p)?score.base[v][p]:0;}
float ug_effective(int v,int p) {
 if(!valid_voice(v)||!valid_param(p))return 0;
 if(audition_step[v]>=0){Step *s=&score.steps[v][audition_step[v]];return s->mask&(1u<<p)?s->lock[p]:score.base[v][p];}
 if(takeover[v]&(1u<<p))return manual[v][p];
 if(playing&&valid_step(step_index)&&(score.steps[v][step_index].mask&(1u<<p)))return score.steps[v][step_index].lock[p];
 return score.base[v][p];
}
void ug_setstep(int v,int s,int on) {if(valid_voice(v)&&valid_step(s)){score.steps[v][s].authored=1;score.steps[v][s].on=on?1:0;record_skip[v]&=~(1u<<s);}}
void ug_togglestep(int v,int s) {if(valid_voice(v)&&valid_step(s)){score.steps[v][s].authored=1;score.steps[v][s].on=!score.steps[v][s].on;record_skip[v]&=~(1u<<s);}}
int ug_step(int v,int s) {return valid_voice(v)&&valid_step(s)?score.steps[v][s].on:0;}
void ug_setlock(int v,int s,int p,float value){
 if(!valid_voice(v)||!valid_step(s)||!valid_param(p)||p==17)return;
 score.steps[v][s].authored=1;score.steps[v][s].mask|=(1u<<p);score.steps[v][s].lock[p]=clamp(value,0,1);
}
void ug_clearlock(int v,int s){if(valid_voice(v)&&valid_step(s))score.steps[v][s].mask=0;}
int ug_lockmask(int v,int s){return valid_voice(v)&&valid_step(s)?score.steps[v][s].mask:0;}
float ug_lockvalue(int v,int s,int p){return valid_voice(v)&&valid_step(s)&&valid_param(p)?score.steps[v][s].lock[p]:0;}
void ug_note(int s,int note,float gate,int slide){if(!valid_step(s))return;score.steps[2][s].authored=1;score.steps[2][s].note=note<24?24:note>60?60:note;score.steps[2][s].gate=clamp(gate,.1f,1.5f);score.steps[2][s].slide=slide?1:0;}
float ug_notevalue(int s,int p){if(!valid_step(s))return 0;Step *q=&score.steps[2][s];return p==0?q->note:p==1?q->gate:p==2?q->slide:0;}
static void trigger(int v,float velocity,int sequenced){
 if(sequenced==1)capture_motion(v,step_index);
 if(!sequenced)audition_step[v]=-1;

 Voice *q=&voices[v];
 /* DNA is a small, deterministic interpretation. Never write it back into the score. */
 float pressure=clamp(velocity,0,1)-.5f;
 q->dna_decay=1.0f+clamp((110.0f-tempo)/110.0f,-1,1)*.06f;
 if(v==0&&trigger_count[v]&&sample_clock-last_trigger[v]<(uint32_t)(rate/8))q->dna_decay*=.96f;
 last_trigger[v]=sample_clock;
 /* The short attack fades retriggers in, avoiding a hard initial sample jump. */
 for(int p=0;p<NPARAM;p++)q->params[p]=ug_effective(v,p);
 if(v==2){
  if(sequenced)root_live=-1; /* A sequenced note replaces the live-key gate owner. */
  Step *st=&score.steps[2][sequenced==2?audition_step[2]:sequenced?step_index:0];
  int glide=sequenced&&st->slide&&q->env>.001f;
  root_target=note_frequency(sequenced?st->note:root_live>=0?root_live:36);
  if(!glide){root_freq=root_target;q->phase=0;q->age=0;}
  root_gate=sequenced?(int)((sequenced==2?(float)rate*60:(float)step_duration())/(tempo*4)*st->gate):-1;
  q->env=glide?q->env:1;q->vel=clamp(velocity,0,1);trigger_count[v]++;return;
 }
 q->phase=0;q->env=1;q->sweep=1;q->vel=clamp(velocity,0,1);q->age=0;
 q->burst_left=v==1?(int)clamp(1+q->params[5]*11+pressure,1,13):0;
 q->burst_wait=(int)(rate*(.003f+q->params[4]*q->params[4]*.14f)*q->dna_decay);
 if(v==3){
  zero(&backbeat,sizeof backbeat);q->noise_lp=0;backbeat.grain=1;
  for(int p=0;p<6;p++)backbeat.smooth[p]=q->params[p<3?p:p+1];
  float shaker=clamp(1-q->params[0]*2,0,1),clap=clamp(q->params[0]*2-1,0,1),spread=q->params[5];
  q->burst_left=(int)(shaker*(3+spread*18)+(1-shaker)*spread*(4+clap*3));
  q->burst_wait=(int)(rate*((.0015f+spread*spread*.014f)*shaker+(.005f+spread*spread*.045f)*(1-shaker)));
 }
 trigger_count[v]++;
}
void ug_audition(int v,int s){if(!valid_voice(v)||!valid_step(s)||take_active||motion||recording)return;audition_step[v]=s;trigger(v,score.steps[v][s].velocity,2);}
void ug_keyvelocity(int note,int down,float velocity){
 velocity=velocity<0?score.velocity[2]:clamp(velocity,0,1);
 if(note<24||note>60)return;
 if(!down){if(root_live==note){root_gate=0;root_live=-1;}return;}
 root_live=note;
 if(recording&&playing&&recording_voice(2)){int nearest=step_index<0?0:step_index;if(tickphase>step_duration()/2)nearest=(nearest+1)%NSTEPS;
  take_snapshot();if(!score.steps[2][nearest].on&&(step_index<0||nearest!=step_index))record_skip[2]|=1u<<nearest;
  score.steps[2][nearest].authored=1;score.steps[2][nearest].on=1;score.steps[2][nearest].note=note;score.steps[2][nearest].velocity=velocity;
  if(take_active){score.steps[2][nearest].gate=score.draft_note.gate;score.steps[2][nearest].slide=score.draft_note.slide;}
  capture_motion(2,nearest);
 }
 trigger(2,velocity,0);
}
void ug_key(int note,int down){ug_keyvelocity(note,down,-1);}
void ug_hit(int v,float velocity){
 if(!valid_voice(v))return;
 velocity=velocity<0?score.velocity[v]:clamp(velocity,0,1);
 if(recording&&playing&&recording_voice(v)){
  int nearest=step_index<0?0:step_index;
  if(tickphase>step_duration()/2)nearest=(nearest+1)%NSTEPS;
  if(!score.steps[v][nearest].on||score.steps[v][nearest].velocity!=velocity){
   take_snapshot();
   if(!score.steps[v][nearest].on&&(step_index<0||nearest!=step_index))record_skip[v]|=(1u<<nearest);
   score.steps[v][nearest].authored=1;score.steps[v][nearest].on=1;score.steps[v][nearest].velocity=velocity;
  }
  capture_motion(v,nearest);
 }
 if(v==1)pending_chatter=-1; /* A live retrigger supersedes a queued soft hit. */
 trigger(v,velocity,0);
}
void ug_clear(void){
 int changed=0;for(int v=0;v<NVOICE;v++)for(int s=0;s<NSTEPS;s++)if(score.steps[v][s].authored||score.steps[v][s].on||score.steps[v][s].mask)changed=1;
 if(changed){ug_begin_edit();zero(score.steps,sizeof score.steps);for(int v=0;v<NVOICE;v++)for(int st=0;st<NSTEPS;st++)score.steps[v][st].velocity=1;for(int s=0;s<NSTEPS;s++){score.steps[2][s].note=36;score.steps[2][s].gate=.75f;}}ug_panic();
}
void ug_panic(void){ug_play(0);room_clear();zero(&backbeat,sizeof backbeat);zero(&root_fm,sizeof root_fm);zero(grains,sizeof grains);grain_level=0;grain_wait=0;zero(voices,sizeof voices);zero(delaybuf,sizeof delaybuf);delay_lp=0;zero(output,sizeof output);}
static void advance(void){
 step_index=(step_index+1)%NSTEPS;
 for(int v=0;v<NVOICE;v++)audition_step[v]=-1;
 for(int v=0;v<NVOICE;v++){
  uint32_t bit=1u<<step_index;
  if(score.steps[v][step_index].on&&!(record_skip[v]&bit)){
   if(v==1&&score.human>0){
    /* Fixed per-step feel, independent of DSP noise and loop count. Never invent events. */
    uint32_t feel=(uint32_t)(step_index+1)*2654435761u;
    pending_chatter=(int)(rate*0.008f*score.human*(float)(1u+(feel&255u))/256.0f);
    pending_velocity=score.steps[v][step_index].velocity*(1.0f-score.human*(float)((feel>>8)&255u)*(0.12f/255.0f));
    if(pending_chatter==0){trigger(v,pending_velocity,1);pending_chatter=-1;}
   }else trigger(v,score.steps[v][step_index].velocity,1);
  }
  record_skip[v]&=~bit;
 }
}
/* A finite eight-grain cloud. Level zero/off skips spawning and settles to no work. */
static float atmosphere(void){
 float target=playing&&score.base[2][17]>.5f?ug_effective(2,15):0;
 grain_level+=(target-grain_level)/(rate*.01f);
 if(target==0&&grain_level<.00001f){zero(grains,sizeof grains);grain_level=0;return 0;}
 float size=ug_effective(2,12),density=ug_effective(2,13);
 if(target>0&&--grain_wait<=0){
  for(int j=0;j<GRAINS;j++)if(!grains[j].active){
   int note=24+(int)(ug_effective(2,14)*36+.5f);
   grains[j].active=1;grains[j].age=0;grains[j].duration=rate*(.015f+size*size*.22f);
   grains[j].pos=ug_effective(2,11)*(TEXTURE-1)+grain_cursor;while(grains[j].pos>=TEXTURE)grains[j].pos-=TEXTURE;
   grains[j].speed=(22050.0f/rate)*(note_frequency(note)/65.406392f);grain_serial++;break;
  }
  grain_wait=(int)(rate*(.015f+size*size*.22f)/(1+density*7));if(grain_wait<1)grain_wait=1;
 }
 grain_cursor+=ug_effective(2,18)*22050.0f/rate*.1f;if(grain_cursor>=TEXTURE)grain_cursor-=TEXTURE;
 float sum=0;for(int j=0;j<GRAINS;j++)if(grains[j].active){
  int a=(int)grains[j].pos,b=(a+1)%TEXTURE;float f=grains[j].pos-a,t=grains[j].age/grains[j].duration;
  sum+=(texture[a]+(texture[b]-texture[a])*f)*4*t*(1-t);
  grains[j].pos+=grains[j].speed;while(grains[j].pos>=TEXTURE)grains[j].pos-=TEXTURE;
  if(++grains[j].age>=grains[j].duration)grains[j].active=0;
 }
 return sum*grain_level*.18f;
}
float ug_grain(int field){
 if(field==0){float p=grain_cursor+ug_effective(2,11)*(TEXTURE-1);if(p>=TEXTURE)p-=TEXTURE;return p/TEXTURE;}
 if(field==1){int n=0;for(int j=0;j<GRAINS;j++)n+=grains[j].active;return n;}
 if(field>=10&&field<74)return texture[(field-10)*(TEXTURE/64)];
 if(field>=2&&field<2+GRAINS)return grains[field-2].active?grains[field-2].pos/TEXTURE:-1;
 return 0;
}
#include "room.h"
void ug_render(int frames){
 if(frames<0)frames=0;
 if(frames>MAX_BLOCK)frames=MAX_BLOCK;
 int delaylen=(int)((float)rate*30.0f/(float)tempo);if(delaylen>=DELAY)delaylen=DELAY-1;if(delaylen<1)delaylen=1;
 for(int i=0;i<frames;i++){
  if(playing){if(first){advance();first=0;}else{tickphase+=(uint32_t)(tempo*4);if(tickphase>=step_duration()){tickphase-=step_duration();advance();}}}
  if(playing&&pending_chatter>0&&--pending_chatter==0){trigger(1,pending_velocity,1);pending_chatter=-1;}
  float dry=0,send=0;
  /* The top of Reverb washes sent layers fully wet without boosting/clipping
   * the dry attack. Unsent layers keep their direct sound. */
  float wash=room.now[0]*room.now[0];wash*=wash;
  for(int v=0;v<NVOICE;v++){
   level_now[v]+=((muted[v]?0:levels[v])-level_now[v])*(1.0f/(rate*.005f));
   float layer_send=ug_effective(v,3);dry_now[v]+=(1-wash*layer_send-dry_now[v])*room.smoothing;
   Voice *q=&voices[v];if(q->env<0.000015f&&q->burst_left==0&&(v!=3||q->sweep<0.000015f)){q->env=0;continue;}
   float body=ug_effective(v,0),edge=ug_effective(v,1),length=ug_effective(v,2);
   float e4=ug_effective(v,4),e5=ug_effective(v,5),e6=ug_effective(v,6);
   float pressure=(q->vel-.5f)*e6;
   if(v!=1)edge=clamp(edge+pressure*.12f,0,1);
   float sample=0;
   if(v==0){
    float freq=36.0f+body*64.0f+q->sweep*q->sweep*(10.0f+e5*320.0f)*(0.9f+edge*.2f);
    q->phase+=freq/(float)rate;if(q->phase>=1)q->phase-=1;
    sample=sine(q->phase)*q->env+noise()*edge*0.18f*q->sweep*q->sweep*q->sweep;
    q->sweep*=1.0f-1.0f/(rate*0.018f);
    q->env*=1.0f-1.0f/(rate*(0.035f+length*0.25f)*(1.8f-e4*1.5f)*q->dna_decay);
    sample*=0.65f;
   }else if(v==2){
    root_freq+=(root_target-root_freq)*(1.0f/(rate*(.002f+e4*e4*.20f)));
    q->phase+=root_freq/rate;if(q->phase>=1)q->phase-=1;
    float structure=ug_effective(2,7),intensity=ug_effective(2,8),feedback=ug_effective(2,9),motion=ug_effective(2,10);
    /* Fixed carrier/modulator topology: DNA owns operators; macros shape relationships. */
    float ratio=1+(int)(structure*5.999f);
    root_fm.mod_phase+=root_freq*ratio/rate;root_fm.mod_phase-=(int)root_fm.mod_phase;
    float mp=root_fm.mod_phase+root_fm.feedback*feedback*.35f;mp-=(int)mp;if(mp<0)mp+=1;
    float mod=sine(mp);root_fm.feedback=mod;
    float evolution=1-motion+motion/(1+(float)q->age/(rate*.08f));
    float cp=q->phase+mod*intensity*intensity*.45f*evolution;cp-=(int)cp;if(cp<0)cp+=1;
    float tone=sine(cp)*(1-edge)+sine(q->phase*2-(int)(q->phase*2))*edge*.65f;
    root_fm.sub_phase+=root_freq*.5f/rate;root_fm.sub_phase-=(int)root_fm.sub_phase;
    tone=tone*(.35f+body*.65f)+sine(root_fm.sub_phase)*(1-body)*.65f;
    if(root_gate>0)root_gate--;
    if(root_gate==0)q->env*=1.0f-1.0f/(rate*(.008f+length*.30f));
    sample=tone*q->env*.42f*(.65f+e5*.45f);
   }else if(v==3){
    /* Continuous shaker -> snare -> clap blend. Smooth timbre changes inside a hit,
     * but keep the score/draft and authored lock values exact. */
    float target[6]={body,edge,length,e4,e5,e6};
    for(int p=0;p<6;p++)backbeat.smooth[p]+=(target[p]-backbeat.smooth[p])*(1.0f/(rate*.002f));
    body=backbeat.smooth[0];edge=backbeat.smooth[1];length=backbeat.smooth[2];e4=backbeat.smooth[3];e5=backbeat.smooth[4];e6=backbeat.smooth[5];
    float shaker=clamp(1-body*2,0,1),clap=clamp(body*2-1,0,1),snare=1-shaker-clap;
    if(q->burst_left>0&&--q->burst_wait<=0){
     backbeat.grain=.65f+noise()*.3f;q->sweep=.8f+backbeat.grain*.15f;q->burst_left--;
     float spacing=(.0015f+e5*e5*.014f)*shaker+(.005f+e5*e5*.045f)*(1-shaker);
     q->burst_wait=(int)(rate*spacing*(.75f+backbeat.grain*.4f));
    }
    float hz=150.0f+body*190.0f+q->sweep*q->sweep*(25+edge*120);
    q->phase+=hz/rate;if(q->phase>=1)q->phase-=1;
    backbeat.mode+=hz*(2.19f+edge*.73f)/rate;if(backbeat.mode>=1)backbeat.mode-=1;
    backbeat.metal+=clamp((2200+edge*5100+backbeat.grain*800)/rate,0,.42f);if(backbeat.metal>=1)backbeat.metal-=1;
    float n=noise(),crush=clamp((edge-.6f)*2.5f,0,1);
    backbeat.clock+=clamp((18000-crush*15500)/rate,0,1);
    if(backbeat.clock>=1){backbeat.clock-=1;backbeat.held=n;}
    n+=(backbeat.held-n)*crush;
    /* Difference of two stable low-passes: airy high-pass grains through
     * papery band-limited clap. Damping darkens without killing the grain cloud. */
    float lower=900+clap*1000+edge*1400;
    backbeat.low+=(n-backbeat.low)*clamp(lower/rate,0,.8f);
    backbeat.band+=(n-backbeat.band)*clamp((lower+600+(1-e4)*(4500+edge*12000))/rate,0,.9f);
    float wire=(backbeat.band-backbeat.low)*(2.4f+shaker*.9f);
    float skin=sine(q->phase)*.72f+sine(backbeat.mode)*.28f;
    float metal=sine(backbeat.metal)*sine(backbeat.mode);
    float cloud=wire+metal*crush*(.15f+shaker*.4f);
    /* At the edge, short thresholded impacts break up the noise into crackle. */
    float crack=clamp((wire*wire-.12f)*6,0,1);
    cloud*=1-crush*.7f+crush*crack*1.3f;
    float driven=cloud*(1+crush*5);cloud=driven/(1+(driven<0?-driven:driven));
    sample=(skin*q->env*snare*(.7f-edge*.35f)+cloud*(q->sweep*(.85f+shaker*.4f+clap*.5f)+q->env*(snare*.35f+clap*.16f)))*.65f;
    q->env*=1.0f-1.0f/(rate*(.02f+length*.45f)*(1.6f-e4*1.3f)*q->dna_decay);
    float grain_decay=(.0006f+length*.004f)*shaker+(.009f+length*.17f)*snare+(.003f+length*.045f)*clap;
    q->sweep*=1.0f-1.0f/(rate*grain_decay*(1.4f-e4*1.1f)*q->dna_decay);
   }else{
    if(q->burst_left>0&&--q->burst_wait<=0){q->env=0.85f;q->age=0;q->burst_left--;q->burst_wait=(int)(rate*(.003f+e4*e4*.14f)*q->dna_decay*(.92f+(q->burst_left%3)*.08f));}
    float n=noise();q->noise_lp+=(n-q->noise_lp)*(.025f+e6*.70f);
    q->phase+=(280.0f+body*2200.0f)/(float)rate;if(q->phase>=1)q->phase-=1;
    sample=(sine(q->phase)*(0.70f-edge*0.45f)+q->noise_lp*(0.18f+edge*0.72f))*q->env*0.40f;
    q->env*=1.0f-1.0f/(rate*(.001f+(1-e6)*.025f)*(.3f+length*2.3f)*q->dna_decay);
   }
   /* Response has a strong direct effect on onset; DNA adds only a small pressure colour. */
   float attack_time=v==2?.0003f+e6*e6*.04f:v==1?.0002f+(1-e6)*.001f:.0003f+(1-e6)*(1-e6)*(v==2?.04f:.012f);
   float attack=clamp((float)q->age/(rate*attack_time),0,1);q->age++;sample*=q->vel*attack;
   sample*=level_now[v];dry+=sample*dry_now[v];send+=sample*layer_send;
  }
  float cloud_send=ug_effective(2,16);cloud_dry+=(1-wash*cloud_send-cloud_dry)*room.smoothing;
  float cloud=atmosphere()*level_now[2];dry+=cloud*cloud_dry;send+=cloud*cloud_send;
  int read=delaypos-delaylen;if(read<0)read+=DELAY;
  delay_lp+=(delaybuf[read]-delay_lp)*0.13f;
  float write=send+delay_lp*0.52f;
  delaybuf[delaypos]=clamp(write,-1.0f,1.0f);delaypos=(delaypos+1)%DELAY;
  if(delay_lp<0.0000001f&&delay_lp>-0.0000001f)delay_lp=0;
  master_now+=(master_target-master_now)*0.002f;
  float room_left,room_right;room_render(send,&room_left,&room_right);
  float mixed=dry+delay_lp*room.now[4]*(1-wash);
  output[2*i]=clamp((mixed+room_left)*master_now,-.95f,.95f);
  output[2*i+1]=clamp((mixed+room_right)*master_now,-.95f,.95f);sample_clock++;
 }
}
float *ug_buffer(void){return output;}
int ug_state(int key){switch(key){case 0:return playing;case 1:return step_index;case 2:return recording;case 3:return rate;case 4:return tempo;case 5:return saved_valid;case 6:return takeover[0];case 7:return takeover[1];case 8:return trigger_count[0];case 9:return trigger_count[1];case 10:return motion;case 11:return motion_mask[0];case 12:return motion_mask[1];case 13:return takeover[2];case 14:return trigger_count[2];case 15:return motion_mask[2];case 16:return root_gate;case 17:return rec_mode;case 18:return take_active;case 19:return rec_track;case 20:return takeover[3];case 21:return trigger_count[3];case 22:return motion_mask[3];case 23:return NVOICE;default:return 0;}}
int ug_state_bytes(void){return sizeof(score)+sizeof(saved)+sizeof(voices)+sizeof(delaybuf)+sizeof(output)+sizeof(manual)+sizeof(texture)+sizeof(grains)+sizeof(root_fm)+sizeof(room_buffer)+sizeof(room)+128;}
