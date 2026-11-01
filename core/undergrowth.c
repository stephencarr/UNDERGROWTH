/* Original UNDERGROWTH musical core, 2026. No donor DSP copied.
 * One source for native tests, WebAssembly, and fm1-base app adapter.
 * No allocation, system calls, IO, wall-clock scheduling or random composition.
 */
#include "undergrowth.h"
#define NVOICE 2
#define NSTEPS 16
#define NPARAM 4
#define MAX_BLOCK 256
#define DELAY 22050

typedef struct { unsigned char on, mask; float lock[NPARAM]; } Step;
typedef struct { Step steps[NVOICE][NSTEPS]; float base[NVOICE][NPARAM]; float swing, human; } Score;
typedef struct {
 float phase, env, sweep, noise_lp, vel;
 float params[NPARAM];
 int age, burst_left, burst_wait;
} Voice;
static Score score, saved;
static int saved_valid;
static int motion, motion_saved;
static unsigned char motion_mask[NVOICE];
static Voice voices[NVOICE];
static float delaybuf[DELAY], delay_lp, output[MAX_BLOCK*2];
static float master_target, master_now;
static float manual[NVOICE][NPARAM];
static unsigned char takeover[NVOICE];
static uint32_t record_skip[NVOICE]; /* Only newly overdubbed future hits: heard live, skip once. */
static int rate, tempo, playing, recording, step_index, delaypos, first;
static uint32_t tickphase, noise_state, trigger_count[NVOICE];
static int pending_chatter;
static float pending_velocity;
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
 motion=motion_saved=0;zero(motion_mask,sizeof motion_mask);
 rate=sample_rate>=8000&&sample_rate<=96000?sample_rate:44100;
 tempo=110;playing=recording=first=0;step_index=-1;tickphase=0;delaypos=0;delay_lp=0;
 master_target=0.65f;master_now=0;noise_state=0x51d734a1; saved_valid=0;pending_chatter=-1;
 score.base[0][0]=0.35f;score.base[0][1]=0.28f;score.base[0][2]=0.40f;score.base[0][3]=0.0f;
 score.base[1][0]=0.45f;score.base[1][1]=0.50f;score.base[1][2]=0.24f;score.base[1][3]=0.16f;
}
void ug_begin_edit(void) {ug_motion(0);saved=score;saved_valid=1;}
int ug_undo(void) {ug_motion(0);if(!saved_valid)return 0;Score swap=score;score=saved;saved=swap;ug_rejoin();zero(record_skip,sizeof record_skip);pending_chatter=-1;return 1;}
void ug_rejoin(void) {ug_motion(0);zero(takeover,sizeof takeover);}
void ug_motion(int enabled) {
 if(enabled<0)enabled=!motion;
 if(enabled&&!motion){
  motion=1;motion_saved=0;recording=0;
  zero(motion_mask,sizeof motion_mask);zero(takeover,sizeof takeover);
 }else if(!enabled&&motion){
  for(int v=0;v<NVOICE;v++)takeover[v]&=~motion_mask[v];
  motion=0;zero(motion_mask,sizeof motion_mask);
 }
}
void ug_record(int enabled) {if(enabled!=0)ug_motion(0);recording=enabled<0?!recording:enabled?1:0;}
void ug_play(int enabled) {
 if(enabled<0)enabled=!playing;
 if(enabled&&!playing){playing=1;first=1;step_index=-1;tickphase=0;zero(record_skip,sizeof record_skip);}
 else if(!enabled){ug_motion(0);pending_chatter=-1;playing=0;recording=0;step_index=-1;first=0;ug_rejoin();zero(record_skip,sizeof record_skip);}
}
void ug_tempo(int bpm) {tempo=bpm<40?40:bpm>220?220:bpm;}
void ug_groove(int parameter,float value) {
 if(parameter==0)score.swing=clamp(value,0,1);
 if(parameter==1)score.human=clamp(value,0,1);
}
float ug_getgroove(int parameter) {return parameter==0?score.swing:parameter==1?score.human:0;}
void ug_master(float gain) {master_target=clamp(gain,0,1);}
void ug_setparam(int v,int p,float value) {
 if(!valid_voice(v)||!valid_param(p))return;
 value=clamp(value,0,1);
 if(motion){motion_mask[v]|=(1u<<p);manual[v][p]=value;takeover[v]|=(1u<<p);return;}
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
static void trigger(int v,float velocity,int sequenced){
   if(sequenced&&motion){
    Step *s=&score.steps[v][step_index];
    for(int p=0;p<NPARAM;p++)if(motion_mask[v]&(1u<<p)){
     if(!(s->mask&(1u<<p))||s->lock[p]!=manual[v][p]){
      /* Snapshot once, at first actual write; preserve no-op takes and all untouched locks. */
      if(!motion_saved){saved=score;saved_valid=1;motion_saved=1;}
      s->mask|=(1u<<p);s->lock[p]=manual[v][p];
     }
    }
   }

 Voice *q=&voices[v];
 /* The short attack fades retriggers in, avoiding a hard initial sample jump. */
 for(int p=0;p<NPARAM;p++)q->params[p]=ug_effective(v,p);
 q->phase=0;q->env=1;q->sweep=1;q->vel=clamp(velocity,0,1);q->age=0;
 q->burst_left=v==1?(int)(q->params[0]*4.99f):0;
 q->burst_wait=(int)(rate*(0.013f+q->params[2]*0.030f));
 trigger_count[v]++;
}
void ug_hit(int v,float velocity){
 if(!valid_voice(v))return;
 if(recording&&playing){
  int nearest=step_index<0?0:step_index;
  if(tickphase>step_duration()/2)nearest=(nearest+1)%NSTEPS;
  if(!score.steps[v][nearest].on){
   ug_begin_edit();
   if(step_index<0||nearest!=step_index)record_skip[v]|=(1u<<nearest);
   score.steps[v][nearest].on=1;
  }
 }
 if(v==1)pending_chatter=-1; /* A live retrigger supersedes a queued soft hit. */
 trigger(v,velocity,0);
}
void ug_clear(void){
 int changed=0;for(int v=0;v<NVOICE;v++)for(int s=0;s<NSTEPS;s++)if(score.steps[v][s].on||score.steps[v][s].mask)changed=1;
 if(changed){ug_begin_edit();zero(score.steps,sizeof score.steps);}ug_panic();
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
    pending_velocity=1.0f-score.human*(float)((feel>>8)&255u)*(0.12f/255.0f);
    if(pending_chatter==0){trigger(v,pending_velocity,1);pending_chatter=-1;}
   }else trigger(v,1.0f,1);
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
   float sample=0;
   if(v==0){
    float freq=36.0f+body*64.0f+q->sweep*q->sweep*(40.0f+edge*210.0f);
    q->phase+=freq/(float)rate;if(q->phase>=1)q->phase-=1;
    sample=sine(q->phase)*q->env+noise()*edge*0.18f*q->sweep*q->sweep*q->sweep;
    q->sweep*=1.0f-1.0f/(rate*0.018f);
    q->env*=1.0f-1.0f/(rate*(0.035f+length*0.25f));
    sample*=0.65f;
   }else{
    if(q->burst_left>0&&--q->burst_wait<=0){q->env=0.85f;q->age=0;q->burst_left--;q->burst_wait=(int)(rate*(0.012f+length*0.026f));}
    float n=noise();q->noise_lp+=(n-q->noise_lp)*(0.04f+edge*0.65f);
    q->phase+=(280.0f+body*2200.0f)/(float)rate;if(q->phase>=1)q->phase-=1;
    sample=(sine(q->phase)*(0.70f-edge*0.45f)+q->noise_lp*(0.18f+edge*0.72f))*q->env*0.40f;
    q->env*=1.0f-1.0f/(rate*(0.003f+length*0.075f));
   }
   float attack=clamp((float)q->age/(rate*0.0008f),0,1);q->age++;sample*=q->vel*attack;
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
  output[2*i]=mixed;output[2*i+1]=mixed;
 }
}
float *ug_buffer(void){return output;}
int ug_state(int key){switch(key){case 0:return playing;case 1:return step_index;case 2:return recording;case 3:return rate;case 4:return tempo;case 5:return saved_valid;case 6:return takeover[0];case 7:return takeover[1];case 8:return trigger_count[0];case 9:return trigger_count[1];case 10:return motion;case 11:return motion_mask[0];case 12:return motion_mask[1];default:return 0;}}
int ug_state_bytes(void){return sizeof(score)+sizeof(saved)+sizeof(voices)+sizeof(delaybuf)+sizeof(output)+sizeof(manual)+128;}
