/* Original UNDERGROWTH atmosphere voices. Fixed state, no samples or allocation.
 * Parameters 11..19: colour, texture, movement, pitch, level, send, enable, rate, character. */
static void atmosphere_reset(void){
 zero(ambient,sizeof ambient);zero(&halo,sizeof halo);
 for(int v=0;v<NVOICE;v++){
  ambient[v].rng=0x713b21u+(uint32_t)v*0x1f531u;
  for(int p=0;p<9;p++)ambient[v].now[p]=score.base[v][11+p];
 }
}
static float atmosphere_noise(int v){
 uint32_t x=ambient[v].rng;x^=x<<13;x^=x>>17;x^=x<<5;ambient[v].rng=x;
 return (float)(x&65535u)/32767.5f-1;
}
/* A continuously excited resonant field: Structure changes modal relationships,
 * Texture changes the material's excitation, Movement redistributes its energy.
 * Coefficients/targets refresh in bounded groups and interpolate between them. */
static float resonant_halo(Ambient *a,float hz,float structure,float texture,float movement,float roughness){
 static const float harmonic[6]={1,1.5f,2,3,4,6};
 static const float metallic[6]={1,1.414214f,2.314f,3.73f,5.19f,7.53f};
 if(halo.update--<=0){
  halo.update=31;
  for(int i=0;i<6;i++){
   float ratio=harmonic[i]+(metallic[i]-harmonic[i])*structure;
   float frequency=hz*2*ratio;
   /* Fade modes approaching the low-rate stability limit instead of piling
    * several resonances onto one clamped frequency. */
   float audible=clamp((rate*.18f-frequency)/(rate*.05f),0,1);
   float fc=clamp(frequency,20,rate*.14f);
   halo.target[i]=2*sine(fc/(2*rate));
   float phase=a->phase[0]+i*.163f;phase-=(int)phase;
   float swell=.5f+.5f*sine(phase);swell*=swell;
   halo.gain_target[i]=audible*(1-movement+movement*(.08f+.92f*swell));
  }
 }
 float n=atmosphere_noise(0);
 halo.air+=(n-halo.air)*clamp(1800.0f/rate,0,.8f);
 halo.friction+=(n-halo.friction)*clamp(35.0f/rate,0,.2f);
 float air=halo.air*.35f;
 float friction=(n-halo.air)*(.1f+(halo.friction<0?-halo.friction:halo.friction)*3);
 /* Sparse rough energy is part of a continuous excitation, not a score event. */
 float crack=clamp((n*n-.72f)*4,0,1)*(n<0?-.5f:.5f);
 float source=texture<.5f?air+(friction-air)*(texture*2):friction+(crack-friction)*(texture*2-1);
 float driven=source*(1+roughness*8);source=driven/(1+(driven<0?-driven:driven));
 float sum=0,smoothing=1.0f/(rate*.01f),damping=.025f+roughness*.07f;
 for(int i=0;i<6;i++){
  halo.coefficient[i]+=(halo.target[i]-halo.coefficient[i])*smoothing;
  halo.gain[i]+=(halo.gain_target[i]-halo.gain[i])*smoothing;
  float f=halo.coefficient[i];
  halo.band[i]+=f*(source-halo.low[i]-damping*halo.band[i]);
  halo.low[i]+=f*halo.band[i];
  halo.low[i]=clamp(halo.low[i],-4,4);halo.band[i]=clamp(halo.band[i],-4,4);
  sum+=halo.band[i]*halo.gain[i];
 }
 return sum*1.8f;
}
static float simple_atmosphere(int v){
 Ambient *a=&ambient[v];float target=playing&&score.base[v][17]>.5f?ug_effective(v,15):0;
 a->level+=(target-a->level)/(rate*.02f);
 if(!target&&a->level<.00001f){a->level=0;return 0;}
 for(int p=0;p<9;p++)if(p!=6)a->now[p]+=(ug_effective(v,11+p)-a->now[p])/(rate*.02f);
 float colour=a->now[0],texture=a->now[1],motion=a->now[2],character=a->now[8];
 float hz=note_frequency(24+(int)(a->now[3]*36+.5f));
 a->phase[0]+=(.03f+a->now[7]*a->now[7]*4)/rate;if(a->phase[0]>=1)a->phase[0]-=1;
 float lfo=sine(a->phase[0]),x=0;
 if(v==0){x=resonant_halo(a,hz,colour,texture,motion,character);}
 else if(v==1){
  /* Coloured noise, with independent timbral motion and pulse depth. */
  float n=atmosphere_noise(v);
  a->clock+=(1-character*.97f)*.45f;if(a->clock>=1){a->clock-=1;a->held=n;}
  n+=(a->held-n)*character;
  float cutoff=clamp(100+colour*colour*12000+motion*lfo*3000+hz,40,rate*.18f);
  a->low[0]+=(n-a->low[0])*(cutoff/rate);
  a->low[1]+=(a->low[0]-a->low[1])*.01f;
  x=(a->low[0]*(1-colour)+(n-a->low[0])*colour)*.9f;
  x*=1-texture*.8f+texture*.8f*(.5f+.5f*lfo);
 }else{
  a->phase[1]+=hz*(1+motion*lfo*.004f)/rate;if(a->phase[1]>=1)a->phase[1]-=1;
  float phase=a->phase[1],fundamental=sine(phase);
   /* Two smoothly morphed vowel resonances; coefficient capped at low rates. */
   static const float vowels[5][2]={{800,1150},{400,2000},{350,2300},{450,800},{325,700}};
   float vowel=clamp(colour*4+motion*lfo*.6f,0,4);int index=(int)vowel;
   int next=index<4?index+1:index;float fraction=vowel-index;
   /* Band-limited saw excites upper vowel resonances even at bass pitches.
    * A six-partial source alone starves the second formant of energy. */
   float dt=hz*(1+motion*lfo*.004f)/rate,blep=0;
   if(phase<dt){float t=phase/dt;blep=t+t-t*t-1;}
   else if(phase>1-dt){float t=(phase-1)/dt;blep=t*t+t+t+1;}
   float saw=2*phase-1-blep;
   float source=(fundamental*.2f+saw*.5f)*(1-character*.7f)+atmosphere_noise(v)*character*.25f;
   x=0;
   for(int b=0;b<2;b++){
    float fc=vowels[index][b]+(vowels[next][b]-vowels[index][b])*fraction;
    fc=clamp(fc,40,rate*.1f);float f=2*sine(fc/(2*rate)),damping=.5f-texture*.35f;
    a->band[b]+=f*(source-a->low[b]-damping*a->band[b]);
    a->low[b]+=f*a->band[b];
    a->band[b]=clamp(a->band[b],-4,4);a->low[b]=clamp(a->low[b],-4,4);
    x+=a->band[b]*(1.4f-texture*.72f);
   }
 }
 return x*a->level*.3f;
}
