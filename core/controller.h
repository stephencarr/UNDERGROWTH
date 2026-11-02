/* Original UNDERGROWTH shared panel controller. Hardware IDs follow fm1-base plat.h. */
#ifndef UG_CONTROLLER_H
#define UG_CONTROLLER_H
void ug_ui_init(void);
void ug_ui_button(int button);
/* Absolute menu edits: macros 0..6, velocity 7, ROOT pitch/gate/slide 8..10, clear locks 11. */
void ug_ui_edit(int parameter,float value);
void ug_ui_turn(int encoder,int delta);
void ug_ui_key(int offset,int down,float velocity);
void ug_ui_select(int field,int value);
void ug_ui_gesture(int reset);
void ug_ui_tick(float milliseconds);
int ug_ui_state(int field);
void ug_ui_refresh(void);
/* Adapter-supplied diagnostics. Negative cpu means unavailable, never an estimate. */
const char *ug_ui_meter(int cpu,int peak,int xruns,int host);
const char *ug_ui_line(int line);
#endif
