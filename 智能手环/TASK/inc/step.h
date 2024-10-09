#ifndef __STEP_H__
#define __STEP_H__

#include "FreeRTOS.h"
#include "semphr.h"
#include "event_groups.h"

#define SEDENTARY_CHECK_INTERVAL_TICKS (pdMS_TO_TICKS(60000)) // 检查久坐状态的时间间隔：1分钟
#define STEP_THRESHOLD 5                                      // 判定为久坐状态的步数阈值
#define UI_UPDATE_INTERVAL_TICKS (pdMS_TO_TICKS(1000))        // UI更新时间间隔：1秒
#define SEDENTARY_EVENT_BIT (1 << 0)                          // 久坐事件对应的事件位

extern SemaphoreHandle_t xStepMutex;       // 用于保护步数读取的互斥量
extern EventGroupHandle_t xStepEventGroup; // 用于通知久坐事件的事件组

extern void sedentary_handler_task(void *pvParameters);   // 处理久坐事件的任务
extern void sedentary_ui_update_task(void *pvParameters); // 更新UI的任务
extern void sedentary_monitor_task(void *pvParameters);   // 监控久坐事件的任务
extern void step_monitor_task(void *pvParameters);

#endif
