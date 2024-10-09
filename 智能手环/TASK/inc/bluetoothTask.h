#ifndef __BLUETOOTH_TASK_H__
#define __BLUETOOTH_TASK_H__

#include <stdio.h>
#include <stdbool.h>
#include "FreeRTOS.h"
#include "queue.h"
#include "event_groups.h"
#include "ui.h"

#define BT_CHECK_INTERVAL pdMS_TO_TICKS(1000)
#define BLUETOOTH_RX_BIT (1 << 0)

static QueueHandle_t xBluetoothStatusQueue = NULL;
extern EventGroupHandle_t xBluetoothEventGroup;

typedef enum
{
    BT_CONNECTED,
    BT_DISCONNECTED
} BluetoothStatus;

extern void bluetooth_status_task(void *pvParameters);
extern void bluetooth_ui_update_task(void *pvParameters);
extern void bluetooth_data_task(void *pvParameters);
extern void parse_and_set_time(char *alarmtime_str);
extern void parse_and_set_alarmtime(char *alarmtime_str);
extern void bluetooth_close_alarm();
#endif // __BLUETOOTH_TASK_H__
