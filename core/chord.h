/* Original UNDERGROWTH chord synthesis, 2026. Four tones, two bounded tail banks.
 * No donor DSP, allocation, samples, IO or independent sequencing. */
static struct ChordBank {float phase[4],freq[4],tone[4],env,attack,low,filter,last,previous,fade,vel;int gate,age,active;} chord[2];
static float chord_smooth[7];
static void chord_clear(void){zero(chord,sizeof chord);chord_live=-1;}
static void chord_stop(void){for(int b=0;b<2;b++)chord[b].gate=0;chord_live=-1;}
static void chord_init(void){
 static const int intervals[8][3]={{4,7,12},{3,7,12},{4,7,10},{4,7,11},{3,7,10},{5,7,12},{7,12,19},{3,6,9}};
 chord_clear();zero(chord_smooth,sizeof chord_smooth);
 float *base=score.base[4];base[0]=.35f;base[1]=.35f;base[2]=.3f;base[4]=.01f;base[5]=.2f;base[6]=.55f;base[7]=1.0f/3;base[11]=.4f;
 for(int i=0;i<8;i++)for(int k=0;k<3;k++)score.palette[i][k]=(intervals[i][k]-1)/23.0f;
 for(int k=0;k<3;k++)base[13+k]=score.palette[0][k];
 for(int p=0;p<7;p++)chord_smooth[p]=base[p];
}
static void chord_trigger(float velocity,int mode){
 if(!chord[0].active&&!chord[1].active)for(int p=0;p<7;p++)chord_smooth[p]=ug_effective(4,p);
 int b=!chord[0].active?0:!chord[1].active?1:chord[0].env<=chord[1].env?0:1;
 int live=chord_live;float previous=chord[b].active?chord[b].last:0;chord_stop();
 struct ChordBank *q=&chord[b];zero(q,sizeof *q);q->active=1;q->env=1;q->vel=clamp(velocity,0,1);q->previous=previous;q->fade=1;
 int root=mode||live<0?36+(int)(ug_effective(4,7)*36+.5f):live;
 /* chord_stop releases old banks but must not discard the caller's live root. */
 int inv=(int)(ug_effective(4,9)*3+.5f);float spread=ug_effective(4,10);
 for(int k=0;k<4;k++){
  int interval=k?1+(int)(ug_effective(4,12+k)*23+.5f):0;
  int note=root+interval+(k<inv?12:0)+(k==1||k==3?(int)(spread*2+.5f)*12:0);
  while(note>96)note-=12;
  float freq=note_frequency(note);while(freq>rate*.22f){freq*=.5f;note-=12;}
  q->tone[k]=note;q->freq[k]=freq;q->phase[k]=k*.17f;
 }
 /* Octave folding can collapse user-authored doubled tones at range limits.
  * Keep two distinct pitches without changing their pitch classes. */
 if(q->tone[0]==q->tone[1]&&q->tone[0]==q->tone[2]&&q->tone[0]==q->tone[3]){q->tone[3]-=12;q->freq[3]*=.5f;}
 q->gate=(mode||live<0)?(int)((float)rate*60/(tempo*4)*(.1f+ug_effective(4,11)*1.4f)):-1;
}
void ug_chordkey(int note,int down,float velocity){
 if(note<36||note>72)return;
 if(!down){if(chord_live==note)chord_stop();return;}
 velocity=velocity<0?score.velocity[4]:clamp(velocity,0,1);
 if(recording&&playing&&recording_voice(4)){
  int step=step_index<0?0:step_index;if(tickphase>step_duration()/2)step=(step+1)%NSTEPS;
  take_snapshot();Step *st=&score.steps[4][step];
  if(!st->on){st->mask=61431u;for(int p=0;p<NPARAM;p++)st->lock[p]=score.base[4][p];if(step_index<0||step!=step_index)record_skip[4]|=1u<<step;}
  st->on=st->authored=1;st->velocity=velocity;
  const int harmony[8]={7,8,9,10,11,13,14,15};
  for(int i=0;i<8;i++){int p=harmony[i];st->mask|=1u<<p;st->lock[p]=score.base[4][p];}
  st->lock[7]=(note-36)/36.0f;capture_motion(4,step);
 }
 audition_step[4]=-1;chord_live=note;trigger(4,velocity,0);chord_live=note;
}
float ug_chordtone(int bank,int field){if(bank<0||bank>1)return 0;return field==0?chord[bank].active:field>=1&&field<=4?chord[bank].tone[field-1]:0;}
static float chord_blep(float t,float dt){if(t<dt){t/=dt;return t+t-t*t-1;}if(t>1-dt){t=(t-1)/dt;return t*t+t+t+1;}return 0;}
static float chord_render(void){
 if(!chord[0].active&&!chord[1].active)return 0;
 for(int p=0;p<7;p++)chord_smooth[p]+=(ug_effective(4,p)-chord_smooth[p])/(rate*.003f);
 float colour=chord_smooth[1],length=chord_smooth[2],attack=chord_smooth[4],release=chord_smooth[5],sweep=chord_smooth[6],out=0;
 for(int b=0;b<2;b++)if(chord[b].active){struct ChordBank *q=&chord[b];
  if(q->gate>0)q->gate--;if(q->gate==0)q->env*=1-1/(rate*(.03f+release*release*4));
  else if(length<.98f)q->env*=1-1/(rate*(.035f+length*length*6));
  q->attack=clamp(q->age/(rate*(.001f+attack*attack*2)),0,1);if(q->age<rate*3)q->age++;
  float sum=0;for(int k=0;k<4;k++){float dt=q->freq[k]/rate,t=q->phase[k]+dt;if(t>=1)t-=1;q->phase[k]=t;
   float saw=2*t-1-chord_blep(t,dt),u=t+.5f;if(u>=1)u-=1;
   float square=(t<.5f?1:-1)+chord_blep(t,dt)-chord_blep(u,dt);
   float warm=sine(t),bright=saw*(1-colour)+square*colour;
   sum+=warm*(1-colour)+bright*colour;
  }
  float cutoff=180+chord_smooth[0]*chord_smooth[0]*10000+sweep*sweep*q->env*5000;
  float coef=clamp(cutoff/(rate+cutoff),.002f,.45f);q->low+=(sum-q->low)*coef;q->filter+=(q->low-q->filter)*coef;
  float sample=q->filter*q->env*q->attack*q->vel*.14f;
  if(q->fade>0){sample=sample*(1-q->fade)+q->previous*q->fade;q->fade-=1/(rate*.005f);}
  q->last=sample;out+=sample;
  if(q->env<.00001f&&q->fade<=0){q->active=0;q->last=0;}
 }
 return out;
}
