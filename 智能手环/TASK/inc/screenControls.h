#ifndef __f_screenControls__
#define __f_screenControls__

#include <stdio.h>
#include <stdbool.h>
#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"

#define SCREEN_OFF_TIMEOUT_MS 30000 // 30秒
#define CHECK_INTERVAL_MS 1000      // 1秒

#define SCREEN_ON_BIT (1 << 0)
#define SCREEN_OFF_BIT (1 << 1)

extern void touch_activity_detected(void);
extern void screen_control_init(void);
extern void screen_event_task(void *pvParameters);

#endif
