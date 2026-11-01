/* UNDERGROWTH rhythm slice. Original project code, 2026. */
#ifndef UNDERGROWTH_H
#define UNDERGROWTH_H
#include <stdint.h>
void ug_init(int sample_rate);
void ug_render(int frames);
float *ug_buffer(void);
void ug_play(int enabled);
void ug_hit(int voice, float velocity);
void ug_record(int enabled);
void ug_tempo(int bpm);
void ug_master(float gain);
void ug_groove(int parameter,float value);
float ug_getgroove(int parameter);
void ug_setparam(int voice,int parameter,float value);
float ug_getparam(int voice,int parameter);
float ug_effective(int voice,int parameter);
void ug_setstep(int voice,int step,int enabled);
int ug_step(int voice,int step);
void ug_togglestep(int voice,int step);
void ug_setlock(int voice,int step,int parameter,float value);
void ug_clearlock(int voice,int step);
int ug_lockmask(int voice,int step);
float ug_lockvalue(int voice,int step,int parameter);
void ug_begin_edit(void);
int ug_undo(void);
void ug_clear(void);
void ug_rejoin(void);
void ug_panic(void);
int ug_state(int key);
int ug_state_bytes(void);
#endif
