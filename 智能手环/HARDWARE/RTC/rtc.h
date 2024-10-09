#ifndef __RTC_H__
#define __RTC_H__
#include "stdint.h"
#include <stdio.h>
#include "led.h"
#include <stdbool.h>
#include "FreeRTOS.h"
#include "timers.h"
#include "event_groups.h"

extern bool alarm_flag;
extern bool alarm_close_flag;

extern void rtc_init();
extern void rtc_set_time(uint8_t hour, uint8_t minute, uint8_t second);
extern void rtc_set_date(int year, int month, int date, int weekday);
extern void rtc_set_alarmtime(uint8_t hour, uint8_t minute, uint8_t second, int weekday);


#endif
