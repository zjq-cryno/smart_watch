#include "bluetoothTask.h"
#include "rtc.h"

uint8_t BlueTooth_Rx_Buffer[20];
EventGroupHandle_t xBluetoothEventGroup = NULL;

// 更新蓝牙图标显示
static void update_bluetooth_icon(bool is_connected)
{
    if (is_connected)
    {
        lv_obj_clear_flag(ui_ImgBlueTooth, LV_OBJ_FLAG_HIDDEN);
    }
    else
    {
        lv_obj_add_flag(ui_ImgBlueTooth, LV_OBJ_FLAG_HIDDEN);
    }
}

// 解析时间字符串并设置RTC
static void parse_and_set_time(char *time_str)
{		//20:30:15
    uint8_t hour = (time_str[0] - '0') * 10 + (time_str[1] - '0');
    uint8_t minute = (time_str[3] - '0') * 10 + (time_str[4] - '0');
    uint8_t second = (time_str[6] - '0') * 10 + (time_str[7] - '0');

    hour = ((hour / 10) << 4) | (hour % 10);
    minute = ((minute / 10) << 4) | (minute % 10);
    second = ((second / 10) << 4) | (second % 10);

    rtc_set_time(hour, minute, second);
}

// 解析日期字符串并设置RTC
static void parse_and_set_date(char *date_str)
{		//2024-05-21-5
    uint8_t year = (date_str[2] - '0') * 10 + (date_str[3] - '0');
    uint8_t month = (date_str[5] - '0') * 10 + (date_str[6] - '0');
    uint8_t day = (date_str[8] - '0') * 10 + (date_str[9] - '0');
    uint8_t weekday = (date_str[11] - '0');

    year = ((year / 10) << 4) | (year % 10);
    month = ((month / 10) << 4) | (month % 10);
    day = ((day / 10) << 4) | (day % 10);
    weekday = ((weekday / 10) << 4) | (weekday % 10);

    rtc_set_date(year, month, day, weekday);
}
//解析闹钟时间字符串并设置RTC
static void parse_and_set_alarmtime(char *alarmtime_str)
	{		//20:30:15-2
    uint8_t hour = (alarmtime_str[0] - '0') * 10 + (alarmtime_str[1] - '0');
    uint8_t minute = (alarmtime_str[3] - '0') * 10 + (alarmtime_str[4] - '0');
    uint8_t second = (alarmtime_str[6] - '0') * 10 + (alarmtime_str[7] - '0');
		uint8_t weekday = (alarmtime_str[9] - '0');
		
    hour = ((hour / 10) << 4) | (hour % 10);
    minute = ((minute / 10) << 4) | (minute % 10);
    second = ((second / 10) << 4) | (second % 10);
		weekday = ((weekday / 10) << 4) | (weekday % 10);
    rtc_set_alarmtime(hour, minute, second, weekday);
		
}

// 蓝牙状态监控任务
void bluetooth_status_task(void *pvParameters)
{
    BluetoothStatus status;
    TickType_t xLastWakeTime = xTaskGetTickCount();

    while (true)
    {
        vTaskDelayUntil(&xLastWakeTime, BT_CHECK_INTERVAL);

        status = Bluetooth_CheckStatus() ? BT_CONNECTED : BT_DISCONNECTED;
        xQueueOverwrite(xBluetoothStatusQueue, &status);
    }
}

// UI更新任务
void bluetooth_ui_update_task(void *pvParameters)
{
    BluetoothStatus status;

    while (true)
    {
        if (xQueueReceive(xBluetoothStatusQueue, &status, portMAX_DELAY) == pdTRUE)
        {
            update_bluetooth_icon(status == BT_CONNECTED);
        }
    }
}


// 蓝牙数据处理任务
void bluetooth_data_task(void *pvParameters)
{
    EventBits_t event_bit;

    while (true)
    {
        event_bit = xEventGroupWaitBits(xBluetoothEventGroup,
                                        BLUETOOTH_RX_BIT,
                                        pdTRUE, pdFALSE, portMAX_DELAY);

        printf("\r\nReceived data: %s\r\n", BlueTooth_Rx_Buffer);

				if(strlen(BlueTooth_Rx_Buffer) == 12)  //日期
				{
					parse_and_set_date(BlueTooth_Rx_Buffer);
					
				}
				if(strlen(BlueTooth_Rx_Buffer) == 8 )  //时间
				{
					parse_and_set_time(BlueTooth_Rx_Buffer);
					
				}
				if(strlen(BlueTooth_Rx_Buffer) == 10)  //设置闹钟时间
				{
					parse_and_set_alarmtime(BlueTooth_Rx_Buffer);
					
				}
				if(strlen(BlueTooth_Rx_Buffer) == 5 && strcmp(BlueTooth_Rx_Buffer, "alarm")==0)  //关闭闹钟时间
				{
					bluetooth_close_alarm();					
				}
    }
}

void bluetooth_close_alarm()
{
	LED0(1);
	RTC_ITConfig(RTC_IT_ALRA, DISABLE);
	/* Enable the alarm */
	RTC_AlarmCmd(RTC_Alarm_A, DISABLE);
	
	/* Enable RTC Alarm A Interrupt */
	RTC_ITConfig(RTC_IT_ALRA, DISABLE);
	
	/* Enable the alarm */
	RTC_AlarmCmd(RTC_Alarm_A, DISABLE);
	
	/* Enable RTC Alarm A Interrupt */
	RTC_ITConfig(RTC_IT_ALRA, DISABLE);
	
	/* Enable the alarm */
	RTC_AlarmCmd(RTC_Alarm_A, DISABLE);
	
	/* Clear the RTC Alarm Flag */
	RTC_ClearFlag(RTC_FLAG_ALRAF);
	
	/* Clear the EXTI Line 17 Pending bit (Connected internally to RTC Alarm) */
	EXTI_ClearITPendingBit(EXTI_Line17);
}

void bluetoothTask_init(void)
{
    xBluetoothEventGroup = xEventGroupCreate();
    xBluetoothStatusQueue = xQueueCreate(1, sizeof(BluetoothStatus));
}