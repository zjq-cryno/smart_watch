
#ifndef __INCLUDES_H__
#define __INCLUDES_H__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "stm32f4xx.h"
#include "stm32f4xx_it.h"
#include "sys.h"
#include "usart.h"
#include "delay.h"
#include "led.h"
#include "beep.h"
#include "key.h"
#include "tim.h"
#include "rtc.h"
#include "dht11.h"
#include "MPU6050.h"
#include "Bluetooth.h"
#include "inv_mpu_dmp_motion_driver.h"

#include "pic_logo.h"
#include "tft.h"
#include "bmp.h"
#include "touch.h"
#include "lvgl.h"
#include "lv_port_disp.h"
#include "lv_port_indev.h"
#include "lv_demo_widgets.h"
#include "ui.h"
#include "ui_helpers.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "event_groups.h"



extern GPIO_InitTypeDef GPIO_InitStructure;
extern NVIC_InitTypeDef NVIC_InitStructure;
extern SPI_InitTypeDef SPI_InitStructure;

extern lv_disp_drv_t *g_disp_drvp;
extern uint8_t BlueTooth_Rx_Buffer[20];

#endif
