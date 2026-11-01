/* Original UNDERGROWTH musical core, 2026. No donor DSP copied.
 * One source for native tests, WebAssembly, and fm1-base app adapter.
 * No allocation, system calls, IO, wall-clock scheduling or random composition.
 */
#include "undergrowth.h"
#define NVOICE 3
#define NSTEPS 16
#define NPARAM 7
#define MAX_BLOCK 256
#define DELAY 22050

typedef struct { unsigned char on, mask; float lock[NPARAM]; int note; float velocity, gate; unsigned char slide; } Step;
typedef struct { Step steps[NVOICE][NSTEPS]; float base[NVOICE][NPARAM]; float swing, human, velocity[NVOICE]; Step draft_note; } Score;
typedef struct {
 float phase, env, sweep, noise_lp, vel, dna_decay;
 float params[NPARAM];
 int age, burst_left, burst_wait;
} Voice;
static Score score, saved;
static int saved_valid;
static int motion, motion_saved;
static int rec_mode=1, take_active, rec_track=-1;
static unsigned char motion_mask[NVOICE];
static Voice voices[NVOICE];
#if defined(UG_FM1) && !defined(PLAT_HOST)
#define UG_POOL __attribute__((section(".pool")))
#else
#define UG_POOL
#endif
static float delaybuf[DELAY] UG_POOL;
static float delay_lp, output[MAX_BLOCK*2];
static float master_target, master_now;
static float manual[NVOICE][NPARAM];
static unsigned char takeover[NVOICE];
static uint32_t record_skip[NVOICE]; /* Only newly overdubbed future hits: heard live, skip once. */
static int rate, tempo, playing, recording, step_index, delaypos, first;
static uint32_t tickphase, noise_state, trigger_count[NVOICE], sample_clock, last_trigger[NVOICE];
static int pending_chatter;
static float pending_velocity;
static float root_freq, root_target;
static int root_gate, root_live=-1;
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
 zero(delaybuf,sizeof delaybuf);zero(output,sizeof output);zero(takeover,sizeof takeover);
 zero(manual,sizeof manual);zero(trigger_count,sizeof trigger_count);zero(record_skip,sizeof record_skip);
 sample_clock=0;zero(last_trigger,sizeof last_trigger);
 root_freq=root_target=65.406392f;root_gate=0;root_live=-1;
 for(int s=0;s<NSTEPS;s++){score.steps[2][s].note=36;score.steps[2][s].gate=.75f;}
 for(int v=0;v<NVOICE;v++){score.velocity[v]=1;for(int st=0;st<NSTEPS;st++)score.steps[v][st].velocity=1;score.base[v][4]=.5f;score.base[v][5]=.5f;score.base[v][6]=.5f;}
 score.draft_note.note=36;score.draft_note.gate=.75f;
 rec_mode=1;take_active=0;rec_track=-1;
 motion=motion_saved=0;zero(motion_mask,sizeof motion_mask);
 rate=sample_rate>=8000&&sample_rate<=96000?sample_rate:44100;
 tempo=110;playing=recording=first=0;step_index=-1;tickphase=0;delaypos=0;delay_lp=0;
 master_target=0.65f;master_now=0;noise_state=0x51d734a1; saved_valid=0;pending_chatter=-1;
 score.base[0][0]=0.35f;score.base[0][1]=0.28f;score.base[0][2]=0.40f;score.base[0][3]=0.0f;
 score.base[2][0]=.65f;score.base[2][1]=.25f;score.base[2][2]=.35f;score.base[2][3]=0;
 score.base[1][0]=0.45f;score.base[1][1]=0.50f;score.base[1][2]=0.24f;score.base[1][3]=0.16f;
}
void ug_begin_edit(void) {ug_arm(0,0);ug_motion(0);saved=score;saved_valid=1;}
int ug_undo(void) {ug_arm(0,0);ug_motion(0);if(!saved_valid)return 0;Score swap=score;score=saved;saved=swap;ug_rejoin();zero(record_skip,sizeof record_skip);pending_chatter=-1;return 1;}
void ug_rejoin(void) {ug_arm(0,0);ug_motion(0);zero(takeover,sizeof takeover);}
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
void ug_draftnote(int note,float gate,int slide){score.draft_note.note=note<24?24:note>60?60:note;score.draft_note.gate=clamp(gate,.1f,1.5f);score.draft_note.slide=slide?1:0;}
float ug_draftvalue(int p){return p==0?score.draft_note.note:p==1?score.draft_note.gate:p==2?score.draft_note.slide:0;}
/* Velocity is authored event data, separate from macro-lock masks. -1 means draft. */
void ug_velocity(int v,int step,float value){
 if(!valid_voice(v)||step < -1||step>=NSTEPS||take_active||motion||recording)return;
 if(step<0)score.velocity[v]=clamp(value,0,1);else score.steps[v][step].velocity=clamp(value,0,1);
}
float ug_getvelocity(int v,int step){return !valid_voice(v)||step < -1||step>=NSTEPS?0:step<0?score.velocity[v]:score.steps[v][step].velocity;}
void ug_stamp(int v,int step){
 if(!valid_voice(v)||!valid_step(step)||recording||motion)return;
 ug_begin_edit();Step *st=&score.steps[v][step];st->on=1;st->velocity=score.velocity[v];st->mask=(1u<<NPARAM)-1;
 for(int p=0;p<NPARAM;p++)st->lock[p]=score.base[v][p];
 if(v==2){st->note=score.draft_note.note;st->gate=score.draft_note.gate;st->slide=score.draft_note.slide;}
 record_skip[v]&=~(1u<<step);zero(takeover,sizeof takeover);
}
void ug_erase(int v,int step){if(!valid_voice(v)||!valid_step(step)||recording||motion)return;if(!score.steps[v][step].on&&!score.steps[v][step].mask)return;ug_begin_edit();zero(&score.steps[v][step],sizeof(Step));score.steps[v][step].velocity=1;if(v==2){score.steps[v][step].note=36;score.steps[v][step].gate=.75f;}record_skip[v]&=~(1u<<step);}
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
void ug_master(float gain) {master_target=clamp(gain,0,1);}
void ug_setparam(int v,int p,float value) {
 if(!valid_voice(v)||!valid_param(p)|| (take_active&&rec_track!=v))return;
 if(take_active&&!motion)take_snapshot();
 value=clamp(value,0,1);
 if(motion&&recording_voice(v)){motion_mask[v]|=(1u<<p);manual[v][p]=value;takeover[v]|=(1u<<p);return;}
 score.base[v][p]=value;
 if(playing){takeover[v]|=(1u<<p);manual[v][p]=value;}
}
float ug_getparam(int v,int p) {return valid_voice(v)&&valid_param(p)?score.base[v][p]:0;}
float ug_effective(int v,int p) {
 if(!valid_voice(v)||!valid_param(p))return 0;
 if(takeover[v]&(1u<<p))return manual[v][p];
 if(playing&&valid_step(step_index)&&(score.steps[v][step_index].mask&(1u<<p)))return score.steps[v][step_index].lock[p];
 return score.base[v][p];
}
void ug_setstep(int v,int s,int on) {if(valid_voice(v)&&valid_step(s)){score.steps[v][s].on=on?1:0;record_skip[v]&=~(1u<<s);}}
void ug_togglestep(int v,int s) {if(valid_voice(v)&&valid_step(s)){score.steps[v][s].on=!score.steps[v][s].on;record_skip[v]&=~(1u<<s);}}
int ug_step(int v,int s) {return valid_voice(v)&&valid_step(s)?score.steps[v][s].on:0;}
void ug_setlock(int v,int s,int p,float value){
 if(!valid_voice(v)||!valid_step(s)||!valid_param(p))return;
 score.steps[v][s].mask|=(1u<<p);score.steps[v][s].lock[p]=clamp(value,0,1);
}
void ug_clearlock(int v,int s){if(valid_voice(v)&&valid_step(s))score.steps[v][s].mask=0;}
int ug_lockmask(int v,int s){return valid_voice(v)&&valid_step(s)?score.steps[v][s].mask:0;}
float ug_lockvalue(int v,int s,int p){return valid_voice(v)&&valid_step(s)&&valid_param(p)?score.steps[v][s].lock[p]:0;}
void ug_note(int s,int note,float gate,int slide){if(!valid_step(s))return;score.steps[2][s].note=note<24?24:note>60?60:note;score.steps[2][s].gate=clamp(gate,.1f,1.5f);score.steps[2][s].slide=slide?1:0;}
float ug_notevalue(int s,int p){if(!valid_step(s))return 0;Step *q=&score.steps[2][s];return p==0?q->note:p==1?q->gate:p==2?q->slide:0;}
static void trigger(int v,float velocity,int sequenced){
 if(sequenced)capture_motion(v,step_index);

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
  Step *st=&score.steps[2][sequenced?step_index:0];
  int glide=sequenced&&st->slide&&q->env>.001f;
  root_target=note_frequency(sequenced?st->note:root_live>=0?root_live:36);
  if(!glide){root_freq=root_target;q->phase=0;q->age=0;}
  root_gate=sequenced?(int)((float)step_duration()/(tempo*4)*st->gate):-1;
  q->env=glide?q->env:1;q->vel=clamp(velocity,0,1);trigger_count[v]++;return;
 }
 q->phase=0;q->env=1;q->sweep=1;q->vel=clamp(velocity,0,1);q->age=0;
 q->burst_left=v==1?(int)clamp(1+q->params[5]*11+pressure,1,13):0;
 q->burst_wait=(int)(rate*(.003f+q->params[4]*q->params[4]*.14f)*q->dna_decay);
 trigger_count[v]++;
}
void ug_keyvelocity(int note,int down,float velocity){
 velocity=velocity<0?score.velocity[2]:clamp(velocity,0,1);
 if(note<24||note>60)return;
 if(!down){if(root_live==note){root_gate=0;root_live=-1;}return;}
 root_live=note;
 if(recording&&playing&&recording_voice(2)){int nearest=step_index<0?0:step_index;if(tickphase>step_duration()/2)nearest=(nearest+1)%NSTEPS;
  take_snapshot();if(!score.steps[2][nearest].on&&(step_index<0||nearest!=step_index))record_skip[2]|=1u<<nearest;
  score.steps[2][nearest].on=1;score.steps[2][nearest].note=note;score.steps[2][nearest].velocity=velocity;
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
   score.steps[v][nearest].on=1;score.steps[v][nearest].velocity=velocity;
  }
  capture_motion(v,nearest);
 }
 if(v==1)pending_chatter=-1; /* A live retrigger supersedes a queued soft hit. */
 trigger(v,velocity,0);
}
void ug_clear(void){
 int changed=0;for(int v=0;v<NVOICE;v++)for(int s=0;s<NSTEPS;s++)if(score.steps[v][s].on||score.steps[v][s].mask)changed=1;
 if(changed){ug_begin_edit();zero(score.steps,sizeof score.steps);for(int v=0;v<NVOICE;v++)for(int st=0;st<NSTEPS;st++)score.steps[v][st].velocity=1;for(int s=0;s<NSTEPS;s++){score.steps[2][s].note=36;score.steps[2][s].gate=.75f;}}ug_panic();
}
void ug_panic(void){ug_play(0);zero(voices,sizeof voices);zero(delaybuf,sizeof delaybuf);delay_lp=0;zero(output,sizeof output);}
static void advance(void){
 step_index=(step_index+1)%NSTEPS;
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
void ug_render(int frames){
 if(frames<0)frames=0;
 if(frames>MAX_BLOCK)frames=MAX_BLOCK;
 int delaylen=(int)((float)rate*30.0f/(float)tempo);if(delaylen>=DELAY)delaylen=DELAY-1;if(delaylen<1)delaylen=1;
 for(int i=0;i<frames;i++){
  if(playing){if(first){advance();first=0;}else{tickphase+=(uint32_t)(tempo*4);if(tickphase>=step_duration()){tickphase-=step_duration();advance();}}}
  if(playing&&pending_chatter>0&&--pending_chatter==0){trigger(1,pending_velocity,1);pending_chatter=-1;}
  float dry=0,send=0;
  for(int v=0;v<NVOICE;v++){
   Voice *q=&voices[v];if(q->env<0.000015f&&q->burst_left==0){q->env=0;continue;}
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
    float p2=q->phase*2;while(p2>=1)p2-=1;
    float p3=q->phase*3;while(p3>=1)p3-=1;
    float p5=q->phase*5;while(p5>=1)p5-=1;
    float tone=sine(q->phase)*(0.65f+body*.35f)+edge*(sine(p2)*.35f+sine(p3)*.22f+sine(p5)*.12f);
    if(root_gate>0)root_gate--;
    if(root_gate==0)q->env*=1.0f-1.0f/(rate*(.008f+length*.30f));
    sample=tone*q->env*.42f*(.65f+e5*.45f);
   }else{
    if(q->burst_left>0&&--q->burst_wait<=0){q->env=0.85f;q->age=0;q->burst_left--;q->burst_wait=(int)(rate*(.003f+e4*e4*.14f)*q->dna_decay*(.92f+(q->burst_left%3)*.08f));}
    float n=noise();q->noise_lp+=(n-q->noise_lp)*(.025f+e6*.70f);
    q->phase+=(280.0f+body*2200.0f)/(float)rate;if(q->phase>=1)q->phase-=1;
    sample=(sine(q->phase)*(0.70f-edge*0.45f)+q->noise_lp*(0.18f+edge*0.72f))*q->env*0.40f;
    q->env*=1.0f-1.0f/(rate*(.001f+(1-e6)*.025f)*(.3f+length*2.3f)*q->dna_decay);
   }
   /* Response has a strong direct effect on onset; DNA adds only a small pressure colour. */
   float attack_time=v==1?.0002f+(1-e6)*.001f:.0003f+(1-e6)*(1-e6)*(v==2?.04f:.012f);
   float attack=clamp((float)q->age/(rate*attack_time),0,1);q->age++;sample*=q->vel*attack;
   dry+=sample;send+=sample*ug_effective(v,3);
  }
  int read=delaypos-delaylen;if(read<0)read+=DELAY;
  delay_lp+=(delaybuf[read]-delay_lp)*0.13f;
  float write=send+delay_lp*0.52f;
  delaybuf[delaypos]=clamp(write,-1.0f,1.0f);delaypos=(delaypos+1)%DELAY;
  if(delay_lp<0.0000001f&&delay_lp>-0.0000001f)delay_lp=0;
  master_now+=(master_target-master_now)*0.002f;
  float mixed=(dry+delay_lp*0.75f)*master_now;
  mixed=clamp(mixed,-0.95f,0.95f);
  output[2*i]=mixed;output[2*i+1]=mixed;sample_clock++;
 }
}
float *ug_buffer(void){return output;}
int ug_state(int key){switch(key){case 0:return playing;case 1:return step_index;case 2:return recording;case 3:return rate;case 4:return tempo;case 5:return saved_valid;case 6:return takeover[0];case 7:return takeover[1];case 8:return trigger_count[0];case 9:return trigger_count[1];case 10:return motion;case 11:return motion_mask[0];case 12:return motion_mask[1];case 13:return takeover[2];case 14:return trigger_count[2];case 15:return motion_mask[2];case 16:return root_gate;case 17:return rec_mode;case 18:return take_active;case 19:return rec_track;default:return 0;}}
int ug_state_bytes(void){return sizeof(score)+sizeof(saved)+sizeof(voices)+sizeof(delaybuf)+sizeof(output)+sizeof(manual)+128;}
