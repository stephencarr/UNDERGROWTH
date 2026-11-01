/* Original UNDERGROWTH FM1 application contract. */
#pragma once
#include <stdint.h>
#include "plat.h"
#define APP_NAME "UNDERGROWTH"
#define APP_SPLASH_RGB RGB(90,140,92)
#define APP_BLOCK 256u
#define APP_TOUCH
#define APP_CANVAS_PX (240u * 24u)
enum { OBJ_PANEL, OBJ_NOBJ };
#define APP_STORE_MAP {0xDC000u}
void app_init(void);
void app_render(int32_t *lr,uint32_t n,uint32_t master_q12);
void app_panic(void);
void app_ui_init(void);
void app_frame(void);
void app_input_only(void);
void app_say(const char *a,const char *b);
#define APP_HOST_EXPECT
int app_expect(const char *what,int *got);
