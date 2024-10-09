#include "screenControls.h"

extern bool is_screen_off;

static EventGroupHandle_t screenEventGroup = NULL; // 事件组句柄
static TimerHandle_t screenOffTimer = NULL;        // 屏幕关闭定时器句柄

// 活动检测函数，由其他任务调用
void touch_activity_detected(void)
{
    xEventGroupSetBitsFromISR(screenEventGroup, SCREEN_ON_BIT, NULL);
}

// 屏幕关闭定时器回调函数
static void screen_off_timer_callback(TimerHandle_t xTimer)
{
    if (!is_screen_off)
    {
        xEventGroupSetBits(screenEventGroup, SCREEN_OFF_BIT);
    }
}

// 屏幕控制初始化函数
void screen_control_init(void)
{
    screenEventGroup = xEventGroupCreate();
    if (screenEventGroup == NULL)
    {
        // 错误处理：无法创建事件组
        printf("\r\nFailed to create screen event group\r\n");
        return;
    }

    screenOffTimer = xTimerCreate("ScreenOffTimer", pdMS_TO_TICKS(SCREEN_OFF_TIMEOUT_MS),
                                  pdFALSE, 0, screen_off_timer_callback);
    if (screenOffTimer == NULL)
    {
        // 错误处理：无法创建定时器
        printf("\r\nFailed to create screen off timer\r\n");
        return;
    }

    xTimerStart(screenOffTimer, 0);
}

// 屏幕开关控制函数
static void control_screen(bool turn_on)
{
    if (turn_on && is_screen_off)
    {
        // 屏幕打开
        printf("\r\nScreen turned on\r\n");

        lcd_display_on(true);
        is_screen_off = false;
        xTimerReset(screenOffTimer, portMAX_DELAY);
    }
    else if (!turn_on && !is_screen_off)
    {
        // 屏幕关闭
        printf("\r\nScreen turned off\r\n");

        lcd_display_on(false);
        is_screen_off = true;
    }
    else if ((!is_screen_off) && turn_on)
        xTimerReset(screenOffTimer, portMAX_DELAY);
}

// 屏幕事件处理任务
void screen_event_task(void *pvParameters)
{
    EventBits_t bits;
    while (1)
    {
        bits = xEventGroupWaitBits(screenEventGroup,
                                   SCREEN_ON_BIT | SCREEN_OFF_BIT,
                                   pdTRUE,
                                   pdFALSE,
                                   portMAX_DELAY);

        if (bits & SCREEN_ON_BIT)
        {
            control_screen(true);
        }
        if (bits & SCREEN_OFF_BIT)
        {
            control_screen(false);
        }
    }
}
