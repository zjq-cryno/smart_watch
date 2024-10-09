#ifndef __RUNMODE_H__
#define __RUNMODE_H__

// 定义事件位
#define RUNMODE_TIMER_PERIOD_MS 1000 // 1秒

#include <stdio.h>
#include "FreeRTOS.h"
#include "timers.h"
#include "event_groups.h"
#include <stdbool.h>

extern uint32_t t;
extern unsigned long  step_count;
extern unsigned long  step_count_last;
extern uint32_t sedentary_event;
extern bool step_flag;

extern void mpu6050_runmode_task(void *pvParameter);
extern void Send_stepcnt_task(void *pvParameter);
extern void runmode_timer_create(void *pvParameter);
extern void Update_StepNum_ui(void *pvParameter);

#endif
