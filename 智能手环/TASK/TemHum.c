#include "TemHum.h"
#include "ui.h"
#include "dht11.h"

QueueHandle_t xDHT11Queue = NULL;
SemaphoreHandle_t xTemHumMutex = NULL;

void TemHum_init(void)
{
    // 创建队列
    xDHT11Queue = xQueueCreate(10, sizeof(dht11_data_t));
    configASSERT(xDHT11Queue != NULL);

    // 创建互斥锁
    xTemHumMutex = xSemaphoreCreateMutex();
    configASSERT(xTemHumMutex != NULL);
}

// DHT11数据采集任务
void TemHum__sensor_task(void *pvParameters)
{
    uint8_t buff[5];
    dht11_data_t data;
    TickType_t xLastWakeTime = xTaskGetTickCount();

    while (true)
    {
        vTaskDelayUntil(&xLastWakeTime, DHT11_POLL_DELAY);

        if (xSemaphoreTake(xTemHumMutex, portMAX_DELAY) == pdTRUE)
        {
            uint8_t rec = dht11_read(buff);
            xSemaphoreGive(xTemHumMutex);

            if (rec == 0) // 数据读取成功
            {
                data.humidity_int = buff[0];
                data.humidity_dec = buff[1];
                data.temperature_int = buff[2];
                data.temperature_dec = buff[3];

                // 发送数据到队列
                xQueueSend(xDHT11Queue, &data, 0);
            }
            else
            {
                // TODO: 错误处理，例如记录日志或重试
            }
        }
    }
}

// UI更新任务
void TemHum_ui_update_task(void *pvParameters)
{
    dht11_data_t data;
    char str[10];

    while (true)
    {
        if (xQueueReceive(xDHT11Queue, &data, portMAX_DELAY) == pdPASS)
        {
            // 更新温度UI
            snprintf(str, sizeof(str), "%d.%d", data.temperature_int, data.temperature_dec);
            lv_label_set_text(ui_LabelTemperatureNum, str);
            lv_arc_set_value(ui_ArcTemperature, data.temperature_int);

            // 更新湿度UI
            snprintf(str, sizeof(str), "%d.%d", data.humidity_int, data.humidity_dec);
            lv_label_set_text(ui_LabelHumidityNum, str);
            lv_arc_set_value(ui_ArcHumidity, data.humidity_int);
        }
    }
}
