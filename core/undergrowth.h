/* UNDERGROWTH rhythm slice. Original project code, 2026. */
#ifndef UNDERGROWTH_H
#define UNDERGROWTH_H
#include <stdint.h>
void ug_init(int sample_rate);
void ug_render(int frames);
/* Grain display telemetry: cursor 0, active count 1, grain positions 2..9. */
float ug_grain(int field);
float *ug_buffer(void);
void ug_play(int enabled);
void ug_hit(int voice, float velocity);
void ug_record(int enabled);
void ug_motion(int enabled);
void ug_arm(int enabled,int track);
void ug_recmode(int mode);
void ug_draftnote(int note,float gate,int slide);
float ug_draftvalue(int parameter);
void ug_stamp(int voice,int step);
void ug_toggle_authored(int voice,int step);
void ug_erase(int voice,int step);
void ug_note(int step,int note,float gate,int slide);
float ug_notevalue(int step,int parameter);
/* CHORD roots C2-C5; harmony is captured from four-tone palette parameters. */
void ug_chordkey(int note,int down,float velocity);
/* Sounding chord telemetry: bank 0/1, fields 0 active, 1..4 MIDI tones. */
float ug_chordtone(int bank,int field);
void ug_key(int note,int down);
void ug_keyvelocity(int note,int down,float velocity);
void ug_velocity(int voice,int step,float velocity);
float ug_getvelocity(int voice,int step);
void ug_tempo(int bpm);
void ug_master(float gain);
/* Shared FX returns/settings: reverb level/decay/damping/width, delay level.
 * Global score values; not owned by any instrument trig/motion lane. */
void ug_setfx(int parameter,float value);
float ug_getfx(int parameter);
/* Session performance mix: outside trig locks, motion and authored-score Undo. */
void ug_level(int voice,float gain);
float ug_getlevel(int voice);
void ug_mute(int voice,int enabled);
int ug_getmute(int voice);
/* Typed step clipboard. Paste changes data only; caller owns the Undo boundary. */
int ug_stepcopy(int voice,int step);
int ug_steppaste(int voice,int step);
int ug_clipboard_voice(void);
/* Hear an authored trig (including a rest) without changing score or transport. */
void ug_audition(int voice,int step);
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
