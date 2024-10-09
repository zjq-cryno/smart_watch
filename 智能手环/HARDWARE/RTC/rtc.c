#include "rtc.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_pwr.h"
#include "stm32f4xx_rtc.h"


static RTC_TimeTypeDef  RTC_TimeStructure;
static RTC_InitTypeDef  RTC_InitStructure;
static RTC_AlarmTypeDef RTC_AlarmStructure;
static NVIC_InitTypeDef  NVIC_InitStructure;
static EXTI_InitTypeDef  EXTI_InitStructure;
static RTC_DateTypeDef RTC_DateStructure;

bool alarm_close_flag = false;

void rtc_init()
{
    // 初始化RTC RTC_DateStructure
    RTC_InitTypeDef RTC_InitStructure;

    /* Enable the PWR clock ,使能电源时钟*/
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);

    /* Allow access to RTC ，允许访问RTC*/
    PWR_BackupAccessCmd(ENABLE);

    /* 使能LSI*/
    RCC_LSICmd(ENABLE);

    /* 检查该LSI是否有效*/
    while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET);

    /* 选择LSI作为RTC的硬件时钟源*/
    RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);

    /* ck_spre(1Hz) = RTCCLK(LSI) /(uwAsynchPrediv + 1)/(uwSynchPrediv + 1)*/
    /* Enable the RTC Clock ，使能RTC时钟*/
    RCC_RTCCLKCmd(ENABLE);

    /* Wait for RTC APB registers synchronisation ，等待RTC相关寄存器就绪*/
    RTC_WaitForSynchro();

    /* Configure the RTC data register and RTC prescaler，配置RTC数据寄存器与RTC的分频值 */
    RTC_InitStructure.RTC_AsynchPrediv = 0x7F;            // 异步分频系数
    RTC_InitStructure.RTC_SynchPrediv = 0xF9;             // 同步分频系数
    RTC_InitStructure.RTC_HourFormat = RTC_HourFormat_24; // 24小时格式
    RTC_Init(&RTC_InitStructure);
}

void rtc_set_time(uint8_t hour, uint8_t minute, uint8_t second)
{
    RTC_TimeTypeDef RTC_TimeStructure;
    RTC_TimeStructure.RTC_Hours = hour;
    RTC_TimeStructure.RTC_Minutes = minute;
    RTC_TimeStructure.RTC_Seconds = second;
    RTC_SetTime(RTC_Format_BCD, &RTC_TimeStructure);
	
	/* Wait for RTC APB registers synchronisation ，等待RTC相关寄存器就绪*/
    RTC_WaitForSynchro();

    return;
}


void rtc_set_date(int year, int month, int date, int weekday)
{
    RTC_DateTypeDef RTC_DateStructure;
    RTC_DateStructure.RTC_Year = year;
    RTC_DateStructure.RTC_Month = month;
    RTC_DateStructure.RTC_Date = date;
    RTC_DateStructure.RTC_WeekDay = weekday;
    RTC_SetDate(RTC_Format_BCD, &RTC_DateStructure);
		
		/* Wait for RTC APB registers synchronisation ，等待RTC相关寄存器就绪*/
    RTC_WaitForSynchro();
		
    return;
}

void rtc_set_alarmtime(uint8_t hour, uint8_t minute, uint8_t second, int weekday)
{
	
	/* Enable RTC Alarm A Interrupt */
	//设置闹钟必须先关闭闹钟
	RTC_ITConfig(RTC_IT_ALRA, DISABLE);
	/* Enable the alarm */
	RTC_AlarmCmd(RTC_Alarm_A, DISABLE);
	
	/* Set the alarm 05h:20min:30s */
	if(hour<0x12)
		RTC_AlarmStructure.RTC_AlarmTime.RTC_H12     = RTC_H12_AM;
	else
		RTC_AlarmStructure.RTC_AlarmTime.RTC_H12     = RTC_H12_PM;
	RTC_AlarmStructure.RTC_AlarmTime.RTC_Hours   = hour;
	RTC_AlarmStructure.RTC_AlarmTime.RTC_Minutes = minute;
	RTC_AlarmStructure.RTC_AlarmTime.RTC_Seconds = second;
	RTC_AlarmStructure.RTC_AlarmDateWeekDay = weekday;//日期或者星期几闹
	RTC_AlarmStructure.RTC_AlarmDateWeekDaySel = RTC_AlarmDateWeekDaySel_Date;//按天开始闹钟
	RTC_AlarmStructure.RTC_AlarmMask = RTC_AlarmMask_DateWeekDay;//屏蔽日期和星期
	
	/* Configure the RTC Alarm A register */
	RTC_SetAlarm(RTC_Format_BCD, RTC_Alarm_A, &RTC_AlarmStructure);
	
	/* Enable RTC Alarm A Interrupt */
	RTC_ITConfig(RTC_IT_ALRA, ENABLE);
	
	/* Enable the alarm */
	RTC_AlarmCmd(RTC_Alarm_A, ENABLE);
	
	/* Enable RTC Alarm A Interrupt */
	RTC_ITConfig(RTC_IT_ALRA, ENABLE);
	
	/* Enable the alarm */
	RTC_AlarmCmd(RTC_Alarm_A, ENABLE);
	
	/* Clear the RTC Alarm Flag */
	RTC_ClearFlag(RTC_FLAG_ALRAF);
	
	/* Clear the EXTI Line 17 Pending bit (Connected internally to RTC Alarm) */
	EXTI_ClearITPendingBit(EXTI_Line17);
	
 /* RTC Alarm A Interrupt Configuration */
	/* EXTI configuration *******************************************************/
	EXTI_ClearITPendingBit(EXTI_Line17);
	EXTI_InitStructure.EXTI_Line = EXTI_Line17;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);
	
	/* Enable the RTC Alarm Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = RTC_Alarm_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

}

//闹钟中断服务函数 
void RTC_Alarm_IRQHandler(void)
{

//	if(RTC_GetITStatus(RTC_IT_ALRA) != RESET)
//   {
//			LED0(0);
//		 
//			RTC_ClearITPendingBit(RTC_IT_ALRA);
//			EXTI_ClearITPendingBit(EXTI_Line17);
//   } 

	
	//判断中断标志位是否置1
	if(EXTI_GetITStatus(EXTI_Line17) == SET)
	{
		//判断是否为闹钟A的中断
		if(RTC_GetFlagStatus(RTC_FLAG_ALRAF) == SET)
		{
			
			LED0(0);
			
			RTC_ClearFlag(RTC_FLAG_ALRAF);
		}
		EXTI_ClearITPendingBit(EXTI_Line17);
	}
}

