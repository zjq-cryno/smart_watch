#ifndef __TEMHUM_H__
#define __TEMHUM_H__

#include <stdint.h>
#include <stdbool.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#define DHT11_QUEUE_LENGTH 5
#define DHT11_POLL_DELAY pdMS_TO_TICKS(2000) // 2秒

typedef struct
{
    uint8_t temperature_int;
    uint8_t temperature_dec;
    uint8_t humidity_int;
    uint8_t humidity_dec;
} dht11_data_t;

extern void TemHum_init(void);
extern void TemHum_ui_update_task(void *pvParameters); // 更新ui
extern void TemHum__sensor_task(void *pvParameters);   // DHT11数据采集任务

#endif // __TEMHUM_H__
