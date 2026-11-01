/* Original UNDERGROWTH shared panel controller. Hardware IDs follow fm1-base plat.h. */
#ifndef UG_CONTROLLER_H
#define UG_CONTROLLER_H
void ug_ui_init(void);
void ug_ui_button(int button);
void ug_ui_turn(int encoder,int delta);
void ug_ui_key(int offset,int down,float velocity);
void ug_ui_select(int field,int value);
void ug_ui_gesture(int reset);
void ug_ui_tick(float milliseconds);
int ug_ui_state(int field);
void ug_ui_refresh(void);
const char *ug_ui_line(int line);
#endif
